#include <immintrin.h>
#include "d2sci_table.h"
template <int Precision = 16, int All_double = 32, unsigned int Num_per_row = 1, int is_init = 0>
inline int my_dou_to_sci_avx512_pure2_opt2(const double *value, char *buffer_s)
{
    const int group = 8;						// avx512 = 8 double
	const int group_count = All_double / group; // 4=32/8
#define FORI for(int i=0;i<group_count;++i)
	using u64 = unsigned long long;
	using i64 = long long;
	static u64 call_num = 0; // 调用次数
	static u64 line_num = 1; // now print pos in which line ; line_num >= 1 
	if (is_init)
	{
		call_num = 0;
		line_num = 1;
		return 0;
	}
	static char is_first = 1;
	const int arr_size = (Num_per_row <= All_double) ? (All_double + Num_per_row - 1) : (All_double * 2);
	static i64 is_end[arr_size];
	int start_is_end;
	if (Num_per_row != 0)
	{
		if (is_first)
		{ // only run once
			is_first = 0;
			if (Num_per_row <= All_double)
			{
				for (int i = 0; i < arr_size; ++i)
				{
					// is_end[i] = ((long long)((((i + 1) % Num_per_row) == 0) ? '\n' : ' ')) << (8 * 5);
					is_end[i] = ((long long)((((i + 1) % Num_per_row) == 0) ? ('\n' - '\n') : (' ' - '\n'))) << (8 * 5);
				}
			}
			else
			{
				for (int i = 0; i < arr_size; ++i)
				{
					// is_end[i] = ((long long)(' '))<<(8*5);
					is_end[i] = ((long long)(' ' - '\n')) << (8 * 5);
				}
				// is_end[All_double] = ((long long)('\n'))<<(8*5);
				is_end[All_double] = ((long long)('\n' - '\n')) << (8 * 5);
			}
		}
		if (Num_per_row <= All_double)
		{
			start_is_end = (call_num * All_double) % Num_per_row;
		}
		else
		{
			int tmp = ((call_num + 1) * All_double) - (line_num * Num_per_row);
			start_is_end = (tmp + 1 > 0) ? (tmp + 1) : 0; // max(tmp,-1)+1 or max(tmp+1,0)
			line_num += (tmp >= 0);
		}
	}
	call_num++;

	// -------------------------------------------
	// avx512 implementation ; powered by Xiang JB.
	// -------------------------------------------
	__m512d value_abs_d8[group_count];
	__m512i value_i8[group_count];
	__m512i value_abs_i8[group_count];
	__m512i ieee754_exp11_8[group_count];
	__m512i value_f_8[group_count];
	__m512i value_e_8[group_count];
	__m512i value_e2_8[group_count];
	__m512i value_e10_8[group_count];
	__m512i value_e10_abs_8[group_count];
	i64 e10[All_double];
	__m512i e10_temp[group_count];
	i64 is_neg[All_double];
	__mmask8 cmp_result_8[group_count];		// uchar
	__m512i tmp[group_count];
	__m512i clz_8[group_count];
	__m512i nres_e_8[group_count];
	__m512i high9_8[group_count], low8_8[group_count];
	__m512i pow10_f_8[group_count];
	__m512i a[group_count], b[group_count], c[group_count], d[group_count];
	__m512i ac[group_count], ad[group_count], bc[group_count];
	__m512i res_f_8[group_count], num0_rest_8[group_count], num0_rest_8_1[group_count];
	__mmask8 cmp_lt_E16_8[group_count];
	const int one_zero = 0;

	FORI value_i8[i] = _mm512_loadu_epi64(&value[i * group]);
	FORI value_abs_d8[i] = _mm512_castsi512_pd(_mm512_and_epi64(value_i8[i], _mm512_set1_epi64(((1ull << 63) - 1))));
	FORI _mm512_storeu_epi64(&is_neg[i * group], _mm512_srli_epi64(value_i8[i], 63));
	FORI value_abs_i8[i] = _mm512_castpd_si512(value_abs_d8[i]); // abs,int64
	FORI ieee754_exp11_8[i] = _mm512_srli_epi64(_mm512_castpd_si512(value_abs_d8[i]), 52);
	FORI clz_8[i] = _mm512_max_epi64(_mm512_lzcnt_epi64(_mm512_castpd_si512(value_abs_d8[i])), _mm512_set1_epi64(11));
	FORI cmp_result_8[i] = _mm512_cmpge_epi64_mask(ieee754_exp11_8[i], _mm512_set1_epi64(1));
	FORI tmp[i] = _mm512_add_epi64(_mm512_sub_epi64(ieee754_exp11_8[i], clz_8[i]), _mm512_set1_epi64(-1011)); //(ieee754_exp11_8[i] + 1 - 1023 + 11 - clz_8);
	FORI value_e2_8[i] = _mm512_mask_sub_epi64(tmp[i], cmp_result_8[i], tmp[i], _mm512_set1_epi64(1));
	buffer_s[0] = '-';
	FORI e10_temp[i] = _mm512_srai_epi64(_mm512_mullo_epi64(value_e2_8[i], _mm512_set1_epi64(78913)), 18);
	FORI value_e10_8[i] = _mm512_add_epi64(e10_temp[i], _mm512_set1_epi64(one_zero));
	const u64 *powers_ptr = &powers_ten_reverse[343 + (1 - one_zero) - Precision];
	FORI pow10_f_8[i] = _mm512_i64gather_epi64(value_e10_8[i], &powers_ten_reverse[343 + (1 - one_zero) - Precision], sizeof(u64));
	FORI nres_e_8[i] = _mm512_sub_epi64(_mm512_sub_epi64(_mm512_set1_epi64(62), value_e2_8[i]),
										_mm512_srai_epi64(_mm512_mullo_epi64(
															  _mm512_sub_epi64(_mm512_set1_epi64(Precision - (1 - one_zero)),
																			   value_e10_8[i]),
															  _mm512_set1_epi64(1741647)),
														  19));
	// // -(e2-63 + ((16-e10)*1741647>>19)-63 + 64) = 62 - e2 - ((16-e10)*1741647>>19) ; (num*1741647)>>19 = floor(num * log2(10))
	FORI value_f_8[i] = _mm512_mask_or_epi64(_mm512_sllv_epi64(_mm512_castpd_si512(value_abs_d8[i]), clz_8[i]), cmp_result_8[i],
											 _mm512_sllv_epi64(_mm512_castpd_si512(value_abs_d8[i]), clz_8[i]), _mm512_set1_epi64(1ull << 63));
	FORI a[i] = _mm512_srli_epi64(value_f_8[i], 32);
	FORI c[i] = _mm512_srli_epi64(pow10_f_8[i], 32);
	FORI ac[i] = _mm512_mul_epu32(a[i], c[i]);
	FORI ad[i] = _mm512_mul_epu32(a[i], pow10_f_8[i]);// d = pow10_f_8 & M32
	FORI bc[i] = _mm512_mul_epu32(value_f_8[i], c[i]);// b = value_f_8 & M32
    FORI res_f_8[i] = _mm512_add_epi64(_mm512_add_epi64(ac[i], _mm512_srli_epi64(ad[i], 32)),
                                       _mm512_srli_epi64(bc[i], 32));// ac+ (ad>>32) + (bc>>32) ;
	FORI num0_rest_8[i] = _mm512_srlv_epi64(_mm512_add_epi64(res_f_8[i], _mm512_rolv_epi64(_mm512_set1_epi64(1ull << 63), nres_e_8[i])), nres_e_8[i]); // round
	FORI num0_rest_8_1[i] = _mm512_srlv_epi64(_mm512_add_epi64(_mm512_srli_epi64(res_f_8[i], 1), _mm512_srli_epi64(res_f_8[i], 3)), _mm512_sub_epi64(nres_e_8[i], _mm512_set1_epi64(4)));
	FORI cmp_lt_E16_8[i] = _mm512_cmplt_epi64_mask(num0_rest_8[i], _mm512_set1_epi64((u64)1e16));
	FORI value_e10_8[i] = _mm512_mask_sub_epi64(value_e10_8[i], cmp_lt_E16_8[i], value_e10_8[i], _mm512_set1_epi64(1));
	FORI num0_rest_8[i] = _mm512_mask_blend_epi64(cmp_lt_E16_8[i], num0_rest_8[i], num0_rest_8_1[i]);
	FORI _mm512_storeu_epi64(&e10[i * group],value_e10_8[i]);
	//FORI _mm512_storeu_epi64(&e10[i * group], _mm512_mask_blend_epi64(_mm512_cmpeq_epi64_mask((value_abs_i8[i]), _mm512_set1_epi64(0)), value_e10_8[i], _mm512_set1_epi64(0)));
	FORI high9_8[i] = _mm512_cvttpd_epi64(_mm512_mul_pd(_mm512_cvt_roundepi64_pd(num0_rest_8[i], _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC),		   // round down
														_mm512_set1_pd(1e-8 /*_10en[-Precision + 8 + 324]*/)));										   // 1e17/1e8 = 9 digit
	FORI low8_8[i] = _mm512_sub_epi64(num0_rest_8[i], _mm512_mullo_epi64(high9_8[i], _mm512_set1_epi64(1e8 /* int64_t(_10en[Precision - 8 + 324]*/))); // 1e17%1e8 = 8 digit
	char *buf_ptr = buffer_s;
	const u64 *exp_ptr = &seq_exp_result3[324 + 1 - one_zero];
	const i64 *is_end_ptr = &is_end[start_is_end];
	for (int i = 0; i < group_count / 2; ++i)
    {
#define FORJ4 for (int j = 0; j < 4; j++)
#define FORJ2 for (int j = 0; j < 2; j++)

        const __m512i DIGIT_ZERO_8 = _mm512_set1_epi64(0x3030303030303030ull); // '0' = 0x30 = 48
        const __m512i M24_16 = _mm512_set1_epi32((1 << 24) - 1); // low 24 bit in 32 bit
        const __m512i L8_16 = _mm512_set1_epi32(0xFF000000u);    // high 8 bit in 32bit

        __m512i num1[2];
        __m512i num_low16[4];
        __m512i num1234_8[4];
        __m512i num5678_8[4];
        __m512i num5678_1234_merge_8[4];
        __m512i num_tmp[4];
        __m512i num_final[4] = {_mm512_set1_epi32(0)};
        __m512i tz_low16[4];
        __m512i tz_8[2];
        u64 num1_print[8*2];// "1."
        u64 num_low16_print[8 * 4];// low16 digtit
        u64 offset[8*2];//18-tz-(tz==16)

        FORJ2 num1[j] = _mm512_srli_epi64(_mm512_mullo_epi64(high9_8[i * 2 + j], _mm512_set1_epi64(1441151881)), 57);// 1441151881 = 2**57 / 1e8
        FORJ2 _mm512_storeu_epi64(&num1_print[j*8], _mm512_or_epi64(num1[j], _mm512_set1_epi64('.' * 256 + '0')));// num1 | ('.' * 256 + '0')
        FORJ2 num_low16[j] = _mm512_sub_epi64(high9_8[i * 2 + j], _mm512_mullo_epi64(num1[j], _mm512_set1_epi64(1e8)));// num_2_9 = high9 - num1*1e8
        FORJ2 num_low16[2+j] = low8_8[i * 2 + j];//num_10_17 = low8

        FORJ4 num1234_8[j] = _mm512_srli_epi64(_mm512_mullo_epi64(num_low16[j], _mm512_set1_epi64(28147497672ull)), 48);// num1234 = num_low8 / 1e4 = num_low8 * 28147497672 >> 48
        FORJ4 num5678_8[j] = _mm512_sub_epi64(num_low16[j], _mm512_mullo_epi64(num1234_8[j], _mm512_set1_epi64(10000)));// num5678 = num_low8 - num1234*1e4
        FORJ4 num5678_1234_merge_8[j] = _mm512_or_epi64(_mm512_slli_epi64(num5678_8[j], 32), num1234_8[j]);
        //FORJ4 num5678_1234_merge_8[j] = _mm512_mask_shuffle_epi32(num1234_8[j], (__mmask16)0b1010101010101010, num5678_8[j], _MM_PERM_CAAA);//another way
        FORJ4 num_tmp[j] = _mm512_mullo_epi32(num5678_1234_merge_8[j], _mm512_set1_epi32(16778)); // num1234 / 1e3 = num1234 * 16778 >> 24
        for (int k = 0; k < 4; k++)
        {
            FORJ4 num_final[j] = _mm512_or_epi32(num_final[j], _mm512_srli_epi32(_mm512_and_epi32(num_tmp[j], L8_16), 24 - k * 8));
            FORJ4 num_tmp[j] = _mm512_mullo_epi32(_mm512_andnot_epi32(L8_16, num_tmp[j]), _mm512_set1_epi32(10));// ((!L8) & num_tmp) * 10
            //FORJ4 num_tmp[j] = _mm512_mullo_epi32( _mm512_and_epi32( M24_16 , num_tmp[j] ) , _mm512_set1_epi32(10) );
        }
        FORJ4 _mm512_storeu_epi64(&num_low16_print[j * 8], _mm512_or_epi64(num_final[j], DIGIT_ZERO_8));
        const i64 diff = (((i64)(' ' - '\n')) << (8 * 5));
#pragma unroll(group)
		for (int j = 0; j < group; j++)
		{
			int idx = 2 * i * group + j;
			buf_ptr += is_neg[idx];
			*(u64 *)(buf_ptr) = num1_print[j];
			_mm_storeu_si128((__m128i *)(buf_ptr + 2), _mm_set_epi64x(num_low16_print[2 * 8 + j], num_low16_print[0 * 8 + j])); // write 16byte
			i64 write_e10_result = exp_ptr[e10[idx]]; // \n
			if (Num_per_row == 0)// Num_per_row is constant value;
			{
				write_e10_result += diff;// " "
			}
			else if (Num_per_row == 1)
			{
				// "\n"
				// do nothing
			}
			else if (((Num_per_row & (Num_per_row - 1)) == 0) && (Num_per_row < All_double)) // 2,4,8,16
			{
				//
				write_e10_result += (( (idx + 1) & (Num_per_row - 1)) == 0) ? 0 : diff;
			}
			else if (Num_per_row < All_double)
			{
				// is end
				write_e10_result += is_end_ptr[idx];
			}
			else if (Num_per_row == All_double)//32
			{
				if (idx != All_double)
					write_e10_result += diff;
			}
			else// >32
			{
				write_e10_result += is_end_ptr[idx];
			}
			*(i64 *)( buf_ptr + 18 ) = write_e10_result;
			buf_ptr += 24;
		}
#pragma unroll(group)
		for (int j = 0; j < group; j++)
		{
			int idx = (2 * i + 1) * group + j;
			buf_ptr += is_neg[idx];
			*(u64 *)(buf_ptr) = num1_print[8+j];
			_mm_storeu_si128((__m128i *)(buf_ptr + 2), _mm_set_epi64x(num_low16_print[3 * 8 + j], num_low16_print[1 * 8 + j])); // write 16byte
			i64 write_e10_result = exp_ptr[e10[idx]]; // \n
			if (Num_per_row == 0)
			{
				// " "
				write_e10_result += diff;
			}
			else if (Num_per_row == 1)
			{
				// "\n"
				// do nothing
			}
			else if (((Num_per_row & (Num_per_row - 1)) == 0) && (Num_per_row < All_double))//2,4,8,16
			{
				//
				write_e10_result += (( (idx + 1) & (Num_per_row - 1)) == 0) ? 0 : diff;
			}
			else if (Num_per_row < All_double)
			{
				// is end
				write_e10_result += is_end_ptr[idx];
			}
			else if (Num_per_row == All_double)
			{
				if (idx != All_double - 1)
					write_e10_result += diff;
			}
			else
			{
				write_e10_result += is_end_ptr[idx];
			}
			*(i64 *)(buf_ptr + 18) = write_e10_result;
			buf_ptr += 24;
		}
    }
	return buf_ptr - buffer_s; // length
}
extern "C"
{
    int d2sci_32(double *value, char *buffer)//icx
	{
		return my_dou_to_sci_avx512_pure2_opt2<16, 32, 1, 0>(value, buffer);
	}
	int d2sci_32_init()
	{
		return my_dou_to_sci_avx512_pure2_opt2<16, 32, 1, 1>(NULL, NULL);
	}
}