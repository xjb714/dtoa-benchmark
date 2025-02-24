#include"Grisu_Exact/fp_to_chars.h"
#include"Grisu_Exact/grisu_exact.h"
#include"test.h"


char* dtoa_grisu_exact(double value,char* buffer){
    jkj::fp_to_chars(value, buffer,
			jkj::grisu_exact_rounding_modes::nearest_to_even{},
			jkj::grisu_exact_correct_rounding::tie_to_even{});
	return buffer;
}

REGISTER_TEST(grisu_exact);