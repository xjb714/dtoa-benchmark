#include <immintrin.h>
#include "d2sci_table.h"
template <int Precision = 16, int All_double = 32>
inline void my_dou_to_sci_avx512_pure9(double *value, char **buffer)
{
    const int group = 8;                        // avx512 = 8 double
    const int group_count = All_double / group; // 4 = 32/8
#define FORI for (int i = 0; i < group_count; ++i)
#define FORJ for (int j = 0; j < group; ++j)
#define FORJ4 for (int j = 0; j < 4; j++)
#define FORJ2 for (int j = 0; j < 2; j++)
    using u64 = unsigned long long;
    using i64 = long long;
    __m512i value_abs_i8[group_count]; // abs(value)
    __m512i value_f_8[group_count];
    __m512i value_e2_8[group_count];
    __m512i value_e10_8[group_count];
    __m512i value_write_ptr_8[group_count];
    i64 e10[All_double];
    char *value_write_ptr[All_double];
    __m512i value_i_8[group_count];
    __m512i ieee754_exp11_8[group_count];
    __mmask8 cmp_result_8[group_count];
    __m512i tmp_8[group_count];
    __m512i clz_8[group_count];
    __m512i high9_8[group_count], low8_8[group_count];
    __m512i pow10_f_8[group_count];
    __m512i a[group_count], b[group_count], c[group_count], d[group_count];
    __m512i ac[group_count], ad[group_count], bc[group_count];
    __m512i res_f_8[group_count], num0_rest_8[group_count], num0_rest_8_1[group_count];
    __m512i e10_temp_8[group_count];
    __m512i nres_e_8[group_count];
    __mmask8 cmp_lt_E16_8[group_count];
    const int one_zero = 0; // set 1 also right , set 0 can be faster than 1;
    FORI value_i_8[i] = _mm512_loadu_epi64(&value[i * group]);
    FORI value_abs_i8[i] = _mm512_and_epi64(value_i_8[i], _mm512_set1_epi64(((1ull << 63) - 1)));
    // FORI value_abs_i8[i] = _mm512_andnot_epi64(_mm512_set1_epi64(((1ull << 63))), value_i_8[i]);
    FORI ieee754_exp11_8[i] = _mm512_srli_epi64((value_abs_i8[i]), 52);
    FORI clz_8[i] = _mm512_max_epi64(_mm512_lzcnt_epi64((value_abs_i8[i])), _mm512_set1_epi64(11));
    FORI cmp_result_8[i] = _mm512_cmpge_epi64_mask(ieee754_exp11_8[i], _mm512_set1_epi64(1));                   // !=0  equal  >=1
    FORI tmp_8[i] = _mm512_add_epi64(_mm512_sub_epi64(ieee754_exp11_8[i], clz_8[i]), _mm512_set1_epi64(-1011)); //(ieee754_exp11_8[i] + 1 - 1023 + 11 - clz_8);
    FORI value_e2_8[i] = _mm512_mask_sub_epi64(tmp_8[i], cmp_result_8[i], tmp_8[i], _mm512_set1_epi64(1));
    FORI e10_temp_8[i] = _mm512_srai_epi64(_mm512_mullo_epi64(value_e2_8[i], _mm512_set1_epi64(78913)), 18); // = floor(value_e2*log10(2))
    FORI value_e10_8[i] = _mm512_add_epi64(e10_temp_8[i], _mm512_set1_epi64(one_zero));                      // e10_temp
    const u64 *powers_ptr = &powers_ten_reverse[343 + 1 - one_zero - Precision];                             // 343 - 15
    FORI pow10_f_8[i] = _mm512_i64gather_epi64(value_e10_8[i], powers_ptr, sizeof(u64));                     // very low speed instruction;
    FORI nres_e_8[i] = _mm512_sub_epi64(_mm512_sub_epi64(_mm512_set1_epi64(62), value_e2_8[i]),
                                        _mm512_srai_epi64(_mm512_mullo_epi64(
                                                              _mm512_sub_epi64(_mm512_set1_epi64(Precision + one_zero - 1),
                                                                               value_e10_8[i]),
                                                              _mm512_set1_epi64(1741647)),
                                                          19)); // -(e2-63 + ((16-e10)*1741647>>19)-63 + 64) = 62 - e2 - ((16-e10)*1741647>>19) , (num*1741647)>>19 = floor(num * log2(10))
    FORI value_write_ptr_8[i] = _mm512_add_epi64(_mm512_loadu_epi64(&buffer[i * group]), _mm512_srli_epi64(value_i_8[i], 63));
    FORI _mm512_storeu_epi64(&value_write_ptr[i * group], value_write_ptr_8[i]);
    FORI value_f_8[i] = _mm512_mask_or_epi64(_mm512_sllv_epi64((value_abs_i8[i]), clz_8[i]), cmp_result_8[i],
                                             _mm512_sllv_epi64((value_abs_i8[i]), clz_8[i]), _mm512_set1_epi64(1ull << 63));
    FORI a[i] = _mm512_srli_epi64(value_f_8[i], 32);
    FORI c[i] = _mm512_srli_epi64(pow10_f_8[i], 32);
    FORI ac[i] = _mm512_mul_epu32(a[i], c[i]);
    FORI ad[i] = _mm512_mul_epu32(a[i], pow10_f_8[i]); // d = pow10_f_8 & M32
    FORI bc[i] = _mm512_mul_epu32(value_f_8[i], c[i]); // b = value_f_8 & M32
    FORI res_f_8[i] = _mm512_add_epi64(_mm512_add_epi64(ac[i], _mm512_srli_epi64(ad[i], 32)),
                                       _mm512_srli_epi64(bc[i], 32));                                                                                  // ac + (ad>>32) + (bc>>32) ;
    FORI num0_rest_8[i] = _mm512_srlv_epi64(_mm512_add_epi64(res_f_8[i], _mm512_rolv_epi64(_mm512_set1_epi64(1ull << 63), nres_e_8[i])), nres_e_8[i]); // round
    FORI num0_rest_8_1[i] = _mm512_srlv_epi64(_mm512_add_epi64(_mm512_srli_epi64(res_f_8[i], 1), _mm512_srli_epi64(res_f_8[i], 3)) +
                                                  _mm512_rolv_epi64(_mm512_set1_epi64(1ull << 63), _mm512_sub_epi64(nres_e_8[i], _mm512_set1_epi64(4))),
                                              _mm512_sub_epi64(nres_e_8[i], _mm512_set1_epi64(4)));
    FORI cmp_lt_E16_8[i] = _mm512_cmplt_epi64_mask(num0_rest_8[i], _mm512_set1_epi64((u64)1e16)); // < 1e16
    FORI value_e10_8[i] = _mm512_mask_sub_epi64(value_e10_8[i], cmp_lt_E16_8[i], value_e10_8[i], _mm512_set1_epi64(1));
    FORI num0_rest_8[i] = _mm512_mask_blend_epi64(cmp_lt_E16_8[i], num0_rest_8[i], num0_rest_8_1[i]);
    FORI _mm512_storeu_epi64(&e10[i * group], value_e10_8[i]); // value = 0 , e10 result = e-324;
    // FORI _mm512_storeu_epi64(&e10[i * group], _mm512_mask_blend_epi64(_mm512_cmpeq_epi64_mask((value_abs_i8[i]), _mm512_set1_epi64(0)), value_e10_8[i], _mm512_set1_epi64(0))); // 0=0e0
    FORI high9_8[i] = _mm512_cvttpd_epi64(_mm512_mul_pd(_mm512_cvt_roundepi64_pd(num0_rest_8[i], _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC),          // round down
                                                        _mm512_set1_pd(1e-8 /* _10en[-Precision + 8 + 324] */)));                                     // 1e17/1e8 = 9 digit
    FORI low8_8[i] = _mm512_sub_epi64(num0_rest_8[i], _mm512_mullo_epi64(high9_8[i], _mm512_set1_epi64(1e8 /*int64_t(_10en[Precision - 8 + 324]*/))); // 1e17%1e8 = 8 digit
    FORI FORJ buffer[i * group + j][0] = '-';
    const u64 *exp_ptr = &exp_result3[324 + 1 - one_zero];
    for (int i = 0; i < group_count / 2; ++i)
    {
        const __m512i DIGIT_ZERO_8 = _mm512_set1_epi64(0x3030303030303030ull); // '0' = 0x30 = 48
        // const __m512i M24_16 = _mm512_set1_epi32((1 << 24) - 1); // low 24 bit in 32 bit
        const __m512i L8_16 = _mm512_set1_epi32(0xFF000000u); // high 8 bit in 32bit
        __m512i num1[2], num_low16[4],  num1234_8[4], num5678_8[4], num_tmp[4];
        __m512i num5678_1234_merge_8[4];
        __m512i num_final[4] = {_mm512_set1_epi32(0)};
        u64 num1_print[8 * 2];      // "1."
        u64 num_low16_print[8 * 4]; // low16 digtit

        //FORJ2 num1[j] = _mm512_srli_epi64(_mm512_mullo_epi64(high9_8[i * 2 + j], _mm512_set1_epi64(1441151881)), 57);    // 1441151881 = 2**57 / 1e8
        FORJ2 num1[j] = _mm512_srli_epi64(_mm512_mul_epu32(high9_8[i * 2 + j], _mm512_set1_epi64(1441151881)), 57);    // 1441151881 = 2**57 / 1e8
        FORJ2 _mm512_storeu_epi64(&num1_print[j * 8], _mm512_or_epi64(num1[j], _mm512_set1_epi64('.' * 256 + '0')));     // num1 | ('.' * 256 + '0')
        //FORJ2 num_low16[j] = _mm512_sub_epi64(high9_8[i * 2 + j], _mm512_mullo_epi64(num1[j], _mm512_set1_epi64(1e8)));  // num_2_9 = high9 - num1*1e8
        FORJ2 num_low16[j] = _mm512_sub_epi64(high9_8[i * 2 + j], _mm512_mul_epu32(num1[j], _mm512_set1_epi64(1e8)));  // num_2_9 = high9 - num1*1e8
        FORJ2 num_low16[2 + j] = low8_8[i * 2 + j];                                                                      // num_10_17 = low8
        //FORJ4 num1234_8[j] = _mm512_srli_epi64(_mm512_mullo_epi64(num_low16[j], _mm512_set1_epi64(28147497672)), 48); // num1234 = num_low8 / 1e4 = num_low8 * 28147497672 >> 48
        FORJ4 num1234_8[j] = _mm512_srli_epi64(_mm512_mul_epu32(num_low16[j], _mm512_set1_epi64(109951163)), 40); // num1234 = num_low8 / 1e4 = num_low8 * 109951163 >> 40
        //FORJ4 num5678_8[j] = _mm512_sub_epi64(num_low16[j], _mm512_mullo_epi64(num1234_8[j], _mm512_set1_epi64(10000))); // num5678 = num_low8 - num1234*1e4
        FORJ4 num5678_8[j] = _mm512_sub_epi64(num_low16[j], _mm512_mul_epu32(num1234_8[j], _mm512_set1_epi64(10000))); // num5678 = num_low8 - num1234*1e4
        FORJ4 num5678_1234_merge_8[j] = _mm512_or_epi64(_mm512_slli_epi64(num5678_8[j], 32), num1234_8[j]);
        // FORJ4 num5678_1234_merge_8[j] = _mm512_mask_shuffle_epi32(num1234_8[j], (__mmask16)0b1010101010101010, num5678_8[j], _MM_PERM_CAAA);//another way
        FORJ4 num_tmp[j] = _mm512_mullo_epi32(num5678_1234_merge_8[j], _mm512_set1_epi32(16778)); // num1234 / 1e3 = num1234 * 16778 >> 24
        for (int k = 0; k < 4; k++)
        {
            FORJ4 num_final[j] = _mm512_or_epi32(num_final[j], _mm512_srli_epi32(_mm512_and_epi32(num_tmp[j], L8_16), 24 - k * 8));
            FORJ4 num_tmp[j] = _mm512_mullo_epi32(_mm512_andnot_epi32(L8_16, num_tmp[j]), _mm512_set1_epi32(10)); // ((!L8) & num_tmp) * 10
            // FORJ4 num_tmp[j] = _mm512_mullo_epi32( _mm512_and_epi32( M24_16 , num_tmp[j] ) , _mm512_set1_epi32(10) );//another way
        }
        FORJ4 _mm512_storeu_epi64(&num_low16_print[j * 8], _mm512_or_epi64(num_final[j], DIGIT_ZERO_8));
        for (int j = 0; j < group; ++j)
        {
            char *buf_ptr1 = value_write_ptr[i * 2 * group + j];
            *(u64 *)(buf_ptr1) = num1_print[j];                                                                                  // write 8 byte but only low 2 byte use;
            _mm_storeu_si128((__m128i *)(buf_ptr1 + 2), _mm_set_epi64x(num_low16_print[2 * 8 + j], num_low16_print[0 * 8 + j])); // write 16byte
            *(u64 *)(buf_ptr1 + 18) = exp_ptr[e10[2 * i * group + j]];                                                           // e10 print result

            char *buf_ptr2 = value_write_ptr[(i * 2 + 1) * group + j];
            *(u64 *)(buf_ptr2) = num1_print[8 + j];
            _mm_storeu_si128((__m128i *)(buf_ptr2 + 2), _mm_set_epi64x(num_low16_print[3 * 8 + j], num_low16_print[1 * 8 + j])); // write 16byte
            *(u64 *)(buf_ptr2 + 18) = exp_ptr[e10[(2 * i + 1) * group + j]];
        }
    }
    //if(0)FORI FORJ if (value[i * group + j] == 0)*(short*)&value_write_ptr[i * group + j] = *(short*)&"0\0";  
}
extern "C" void d2sci_32v(double *value, char **buffer) // compile by clang++
{
    my_dou_to_sci_avx512_pure9<16, 8 * 4>(value, buffer);
}