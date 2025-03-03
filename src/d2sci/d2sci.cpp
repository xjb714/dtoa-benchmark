#include <immintrin.h>
#include "d2sci_table.h"
static const unsigned int kDiv10000 = 0xd1b71759;
static const unsigned int kDiv10000Vector[4] = {kDiv10000, kDiv10000, kDiv10000, kDiv10000};
static const unsigned int k10000Vector[4] = {10000, 10000, 10000, 10000};
static const unsigned short kDivPowersVector[8] = {8389, 5243, 13108, 32768, 8389, 5243, 13108, 32768}; // 10^3, 10^2, 10^1, 10^0
static const unsigned short kShiftPowersVector[8] = {
    1 << (16 - (23 + 2 - 16)),
    1 << (16 - (19 + 2 - 16)),
    1 << (16 - 1 - 2),
    1 << (15),
    1 << (16 - (23 + 2 - 16)),
    1 << (16 - (19 + 2 - 16)),
    1 << (16 - 1 - 2),
    1 << (15)};
static const unsigned short k10Vector[8] = {10, 10, 10, 10, 10, 10, 10, 10};
inline __m128i Convert8DigitsSSE2(unsigned int value)
{
    // assert(value <= 99999999);

    // abcd, efgh = abcdefgh divmod 10000
    const __m128i abcdefgh = _mm_cvtsi32_si128(value);
    const __m128i abcd = _mm_srli_epi64(_mm_mul_epu32(abcdefgh, reinterpret_cast<const __m128i *>(kDiv10000Vector)[0]), 45);
    const __m128i efgh = _mm_sub_epi32(abcdefgh, _mm_mul_epu32(abcd, reinterpret_cast<const __m128i *>(k10000Vector)[0]));

    // v1 = [ abcd, efgh, 0, 0, 0, 0, 0, 0 ]
    const __m128i v1 = _mm_unpacklo_epi16(abcd, efgh);

    // v1a = v1 * 4 = [ abcd * 4, efgh * 4, 0, 0, 0, 0, 0, 0 ]
    const __m128i v1a = _mm_slli_epi64(v1, 2);

    // v2 = [ abcd * 4, abcd * 4, abcd * 4, abcd * 4, efgh * 4, efgh * 4, efgh * 4, efgh * 4 ]
    const __m128i v2a = _mm_unpacklo_epi16(v1a, v1a);
    const __m128i v2 = _mm_unpacklo_epi32(v2a, v2a);

    // v4 = v2 div 10^3, 10^2, 10^1, 10^0 = [ a, ab, abc, abcd, e, ef, efg, efgh ]
    const __m128i v3 = _mm_mulhi_epu16(v2, reinterpret_cast<const __m128i *>(kDivPowersVector)[0]);
    const __m128i v4 = _mm_mulhi_epu16(v3, reinterpret_cast<const __m128i *>(kShiftPowersVector)[0]);

    // v5 = v4 * 10 = [ a0, ab0, abc0, abcd0, e0, ef0, efg0, efgh0 ]
    const __m128i v5 = _mm_mullo_epi16(v4, reinterpret_cast<const __m128i *>(k10Vector)[0]);

    // v6 = v5 << 16 = [ 0, a0, ab0, abc0, 0, e0, ef0, efg0 ]
    const __m128i v6 = _mm_slli_epi64(v5, 16);

    // v7 = v4 - v6 = { a, b, c, d, e, f, g, h }
    const __m128i v7 = _mm_sub_epi16(v4, v6);

    return v7;
}

template <int Precision = 16, int alg = 2>
inline int d2sci_impl(double value, char *buffer)
{
    using ull = unsigned long long;
    using ll = long long;
    buffer[0] = '-';
    ull value_u64 = (*(ull *)&value);                   // as u64
    ull index = value_u64 >> 63;                        // is neg
    ull value_abs_u64 = value_u64 & ((1ull << 63) - 1); // abs u64
    ll ieee754_exp11 = (value_abs_u64 >> 52);           // ieee_exp
    char *ptr = (char *)&buffer[index];
    if (ieee754_exp11 == 0x7ff) // nan or inf
    {
        *(int *)ptr = (value_abs_u64 == (0x7ffull << 52)) ? *(int *)"inf" : *(int *)"nan";
        return index + 3;
    }
    ull f;
    ll e2;
    if (ieee754_exp11 != 0) [[likely]]
    {
        f = (value_u64 << 11) | ((1ull) << 63);
        e2 = ieee754_exp11 - 1023ll;
    }
    else
    {
        if (value_abs_u64 == 0)
        {
            *(short *)ptr = *(short *)"0";
            return index + 1;
        }
        ull clz = _lzcnt_u64(value_abs_u64);
        f = (value_abs_u64) << clz;
        e2 = -1011 - clz;
    }
    ll e10_tmp = ((e2 * 78913) >> 18); // == floor(e2*log10(2))
    const ull *_10en_ptr = (ull *)&_10en[1 + 324];
    // ll e10 = e10_tmp + (value_abs_u64 >= _10en_ptr[e10_tmp]) - (value_abs_u64 == 2);
    ll e10 = e10_tmp + (value_abs_u64 >= _10en_ptr[e10_tmp]); // _10en_ptr[e10_tmp] = pow(10, e10_tmp+1)
    const ull *power_ptr = &powers_ten_reverse[343 - Precision];
    ull pow10_f = power_ptr[e10]; // error 1 : 2**(-64) 
    // long long nres_e = 62 - e2 - (((Precision - e10) * 1741647) >> 19);
    ll nres_e1 = 61 - e2 - (((Precision - e10) * 1741647) >> 19);
    ull mul_res_f;
    _mulx_u64(f, pow10_f, (ull *)&mul_res_f);
    // ull num0_rest = (mul_res_f + (1ull << (nres_e - 1))) >> (nres_e); // 1e16<= num0_rest < 1e17
    ull num0_rest_mul2 = (mul_res_f >> (nres_e1)) + 1; // error 2 max : 0.5/1e16

    // way 1 slow than way2
    // way 1: 4.38ns  ,
    // way 2: 3.89ns  , test result from AMD R7 7840H
    // way 2 need more lookup table ( 4KB )
    ull length;
    if (alg == 1) // avx512
    {
        // way 1
        ll high9 = num0_rest_mul2 / (ll)(2e8); // 1e8 <= high9 < 1e9
        ll high1 = num0_rest_mul2 / (ll)(2e16);
        length = index | 22;
        ull num0_rest = (num0_rest_mul2) >> 1;
        ll low8 = num0_rest - high9 * (ll)(1e8); // 0<= low8 < 1e8
        int high2_9 = high9 - high1 * (ll)(1e8); // 0<= high2_9 < 1e8
        __m256i l8_4r = _mm256_set1_epi64x(low8);
        __m512i h9_4r = _mm512_castsi256_si512(_mm256_set1_epi64x(high2_9));
        length += ((e10 >= 100) | (e10 <= -100));
        __m512i n8r = _mm512_inserti64x4(h9_4r, l8_4r, 1);
        const ull ONE = 0xFFFFFFFFFFFFFFFFull;
        const ull m8 = 180143986;       //>>> 2**54/1e8
        const ull m6 = 18014398510;     //>>> 2**54/1e6
        const ull m4 = 1801439850949;   //>>> 2**54/1e4
        const ull m2 = 180143985094820; //>>> 2**54/1e2
        const __m512i mr = _mm512_set_epi64(m8, m6, m4, m2, m8, m6, m4, m2);
        const ull M54 = (1ull << 54) - 1;
        const ull M8 = 0xff00;
        const __m512i M54_8_all = _mm512_set1_epi64(M54);
        const __m512i M8_8_2 = _mm512_set1_epi64(M8);
        const __m512i t10r = _mm512_set1_epi64(10);
        __m512i tmp_8_0 = _mm512_mullo_epi64(n8r, mr);
        __m512i tmp_8_1 = _mm512_and_epi64(tmp_8_0, M54_8_all);
        __m512i tmp_8_2 = _mm512_mullo_epi64(tmp_8_1, t10r);
        __m512i tmp_8_3_t = _mm512_and_epi64(tmp_8_2, M54_8_all);
        __m512i tmp_8_3 = _mm512_mullo_epi64(tmp_8_3_t, t10r);
        __m512i tmp_8_1_print = _mm512_srli_epi64(tmp_8_2, 54);
        __m512i tmp_8_2_print = _mm512_and_epi64(_mm512_srli_epi64(tmp_8_3, (54 - 8)), M8_8_2);
        __m512i tmp_8_3_print = _mm512_set1_epi64(0x3030) | tmp_8_1_print | tmp_8_2_print;
        const short idx[8] = {12, 8, 4, 0, 28, 24, 20, 16}; // 16byte
        const __m512i idxr_epi16 = _mm512_castsi128_si512(_mm_loadu_epi64(idx));
        __m512i num_8_print_finalr = _mm512_permutexvar_epi16(idxr_epi16, tmp_8_3_print); // avx512bw
        *(short *)ptr = high1 | ('.' * 256 + '0');
        _mm_storeu_si128((__m128i *)(ptr + 2), _mm512_extracti32x4_epi32(num_8_print_finalr, 0));
    }
    if (alg == 2)                               // avx512+lut
    {                                           // way 2
        ull high9 = num0_rest_mul2 / (ll)(2e8); // 1e8 <= high9 < 1e9
        __m512i h9_4r = _mm512_castsi256_si512(_mm256_set1_epi64x(high9));
        ull num0_rest = (num0_rest_mul2) >> 1;
        ull low8 = num0_rest - high9 * (ll)(1e8); // 0<= low8 < 1e8
        ull length = index | 22;
        __m256i l8_4r = _mm256_set1_epi64x(low8);
        __m512i n8r = _mm512_inserti64x4(h9_4r, l8_4r, 1);
        length += ((e10 >= 100) | (e10 <= -100));
        const ull ONE = 0xFFFFFFFFFFFFFFFFull;
        const ull m8 = 180143986;       //>>> 2**54/1e8
        const ull m6 = 18014398510;     //>>> 2**54/1e6
        const ull m4 = 1801439850949;   //>>> 2**54/1e4
        const ull m2 = 180143985094820; //>>> 2**54/1e2
        const __m512i mr = _mm512_set_epi64(m8, m6, m4, m2, m6, m4, m2, m6);
        const ull M54 = (1ull << 54) - 1;
        const ull M8 = 0xff00;
        const __m512i M54_8_all = _mm512_set1_epi64(M54);
        const __m512i M8_8_2 = _mm512_set_epi64(M8, M8, M8, M8, M8, M8, M8, M8);
        const ll t10[8] = {1, 10, 10, 10, 10, 10, 10, 10};
        const __m512i t10r = _mm512_loadu_epi64(t10);
        __m512i tmp_8_0 = _mm512_mullo_epi64(n8r, mr);
        __m512i tmp_8_1 = _mm512_mask_and_epi64(tmp_8_0, 0b11111110, tmp_8_0, M54_8_all);
        __m512i tmp_8_2 = _mm512_mullo_epi64(tmp_8_1, t10r);
        __m512i tmp_8_3_t = _mm512_and_epi64(tmp_8_2, M54_8_all);
        __m512i tmp_8_3 = _mm512_mullo_epi64(tmp_8_3_t, t10r);
        __m512i tmp_8_1_print = _mm512_srli_epi64(tmp_8_2, 54);
        __m512i tmp_8_2_print = _mm512_and_epi64(_mm512_srli_epi64(tmp_8_3, (54 - 8)), M8_8_2);
        __m512i tmp_8_3_print = _mm512_set1_epi64(0x3030) | tmp_8_1_print | tmp_8_2_print;
        const short idx[8] = {12, 8, 4, 28, 24, 20, 16, (short)NULL};
        const __m512i idxr_epi16 = _mm512_castsi128_si512(_mm_loadu_epi64(idx));
        __m512i num_8_print_finalr = _mm512_permutexvar_epi16(idxr_epi16, tmp_8_3_print);               // avx512bw
        _mm_storeu_si128((__m128i *)(ptr + 4), _mm512_extracti32x4_epi32(num_8_print_finalr, 0));       // 1,0 ; low128bit:xmm ; mov xmm to memory
        *(int *)ptr = digit_000_999[_mm_extract_epi64(_mm512_extracti32x4_epi32(tmp_8_1_print, 0), 0)]; // result_8[0],xmm; 0.00 -> 9.99
        const ll *exp_ptr = &exp_result3[324];
        *(ll *)(ptr + 18) = exp_ptr[e10];
        return length; // write char length
    }
    if (alg == 3) // lut
    {
        const ull M50 = (1ull << 50) - 1;
        ll high9 = num0_rest_mul2 / (ll)(2e8); // 1e8 <= high9 < 1e9
        length = index | 22;
        ull num0_rest = (num0_rest_mul2) >> 1;
        ull low8 = num0_rest - high9 * (ll)(1e8); // 0<= low8 < 1e8
        length += ((e10 >= 100) | (e10 <= -100));

        ull num123_456789 = high9 * 1125899907ull;
        ull num123 = num123_456789 >> 50;
        ull num456_789 = (num123_456789 & M50) * 1000ull;
        ull num456 = num456_789 >> 50;
        ull num789 = ((num456_789 & M50) * 1000ull) >> 50;

        *(int *)&buffer[index + 0] = digit_000_999[num123]; // 4
        *(int *)&buffer[index + 4] = digit1000e[num456];    // 3
        *(int *)&buffer[index + 7] = digit1000e[num789];    // 3

        ull num12_345678 = low8 * 1125899907ull;
        ull num12 = num12_345678 >> 50;
        ull num345_678 = (num12_345678 & M50) * 1000ull;
        ull num345 = num345_678 >> 50;
        ull num678 = ((num345_678 & M50) * 1000ull) >> 50;

        *(short *)&buffer[index + 10] = digit100[num12];  // 2
        *(int *)&buffer[index + 12] = digit1000e[num345]; // 3
        *(int *)&buffer[index + 15] = digit1000e[num678]; // 3
    }
    const ll *exp_ptr = &exp_result3[324];
    *(ll *)(ptr + 18) = exp_ptr[e10];
    return length; // write char length
}

template <int Precision = 16>
inline int d2sci_sse_impl(double value, char *buffer)
{
    using ull = unsigned long long;
    using ll = long long;
    buffer[0] = '-';
    ull value_u64 = (*(ull *)&value);                   // as u64
    ull index = value_u64 >> 63;                        // is neg
    ull value_abs_u64 = value_u64 & ((1ull << 63) - 1); // abs u64
    ll ieee754_exp11 = (value_abs_u64 >> 52);           // ieee_exp
    char *ptr = (char *)&buffer[index];
    if (ieee754_exp11 == 0x7ff) // nan or inf
    {
        *(int *)ptr = (value_abs_u64 == (0x7ffull << 52)) ? *(int *)"inf" : *(int *)"nan";
        return index + 3;
    }
    ull f;
    ll e2;
    if (ieee754_exp11 != 0) [[likely]]
    {
        f = (value_u64 << 11) | ((1ull) << 63);
        e2 = ieee754_exp11 - 1023ll;
    }
    else
    {
        if (value_abs_u64 == 0)
        {
            *(short *)ptr = *(short *)"0";
            return index + 1;
        }
        ull clz = _lzcnt_u64(value_abs_u64);
        f = (value_abs_u64) << clz;
        e2 = -1011 - clz;
    }
    ll e10_tmp = ((e2 * 78913) >> 18); // == floor(e2*log10(2))
    const ull *_10en_ptr = (ull *)&_10en[1 + 324];
    // ll e10 = e10_tmp + (value_abs_u64 >= _10en_ptr[e10_tmp]) - (value_abs_u64 == 2);
    ll e10 = e10_tmp + (value_abs_u64 >= _10en_ptr[e10_tmp]);    // _10en_ptr[e10_tmp] = pow(10, e10_tmp+1)
    const ull *power_ptr = &powers_ten_reverse[343 - Precision]; // Precision = 16
    ull pow10_f = power_ptr[e10];                                // 误差来源 1
    // long long nres_e = 62 - e2 - (((Precision - e10) * 1741647) >> 19);
    ll nres_e1 = 61 - e2 - (((Precision - e10) * 1741647) >> 19);
    ull mul_res_f;
    _mulx_u64(f, pow10_f, (ull *)&mul_res_f);
    // ull num0_rest = (mul_res_f + (1ull << (nres_e - 1))) >> (nres_e); // 1e16<= num0_rest < 1e17
    ull num0_rest_mul2 = (mul_res_f >> (nres_e1)) + 1; // 误差来源 2
    ull high9 = num0_rest_mul2 / (ull)(2e8);           // 1e8 <= high9 < 1e9
    ull high1 = num0_rest_mul2 / (ull)(2e16);
    ull length = index | 22;
    ull num0_rest = (num0_rest_mul2) >> 1;
    ull low8 = num0_rest - high9 * (ull)(1e8); // 0<= low8 < 1e8
    ull high2_9 = high9 - high1 * (ull)(1e8);  // 0<= high2_9 < 1e8
    const __m128i a0 = Convert8DigitsSSE2((unsigned int)high2_9);
    const __m128i a1 = Convert8DigitsSSE2((unsigned int)low8);
    const __m128i va = _mm_or_si128(_mm_packus_epi16(a0, a1), _mm_set1_epi8('0'));
    length += ((e10 >= 100) | (e10 <= -100));
    *(short *)ptr = high1 | ('.' * 256 + '0');
    _mm_storeu_si128((__m128i *)(ptr + 2), va);
    const ll *exp_ptr = &exp_result3[324];
    *(ll *)(ptr + 18) = exp_ptr[e10];
    return length; // write char length
}
extern "C" int d2sci(double value, char *buffer)
{
    return d2sci_impl<16, 2>(value, buffer);
}
extern "C" int d2sci_avx512_lut(double value, char *buffer)
{
    return d2sci_impl<16>(value, buffer);
}
extern "C" int d2sci_avx512(double value, char *buffer)
{
    return d2sci_impl<16, 1>(value, buffer);
}
extern "C" int d2sci_lut(double value, char *buffer)
{
    return d2sci_impl<16, 3>(value, buffer);
}
extern "C" int d2sci_sse(double value, char *buffer)
{
    return d2sci_sse_impl<16>(value, buffer);
}