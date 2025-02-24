#include"schubfach/schubfach_64.h"
#include"test.h"


char* dtoa_schubfach(double value,char* buffer){
	//d2sci(value,buffer);
    char* end_buffer = schubfach::Dtoa(buffer,value);
    *end_buffer = '\0';
	return buffer;
}

REGISTER_TEST(schubfach);