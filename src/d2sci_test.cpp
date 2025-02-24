#include"d2sci/d2sci.h"
#include"test.h"

char* dtoa_d2sci(double value,char* buffer){
	d2sci(value,buffer);
	return buffer;
}
char* dtoa_d2sci_sse(double value,char* buffer){
	d2sci_sse(value,buffer);
	return buffer;
}
char* dtoa_d2s_avx512(double value,char* buffer){
	d2s_avx512(value,buffer);
	return buffer;
}
char* dtoa_d2s_sse(double value,char* buffer){
	d2s_sse(value,buffer);
	return buffer;
}
char* dtoa_d2sci_32(double* value,char* buffer){
	d2sci_32(value,buffer);
	return buffer;
}

char** dtoa_d2sci_32v(double* value,char** buffer){
	d2sci_32v(value,buffer);
	return buffer;
}
char** dtoa_d2s_32v(double* value,char** buffer){
	d2s_32v(value,buffer);
	return buffer;
}
REGISTER_TEST(d2sci);
REGISTER_TEST(d2sci_sse);
REGISTER_TEST(d2sci_32);
REGISTER_TEST(d2sci_32v);
REGISTER_TEST(d2s_32v);
REGISTER_TEST(d2s_sse);
REGISTER_TEST(d2s_avx512);