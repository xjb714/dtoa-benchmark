#include"dragon4/PrintFloat.h"
#include"test.h"


char* dtoa_dragon4(double value,char* buffer){
    PrintFloat64(buffer, 1024, value, PrintFloatFormat_Scientific, -1);
	return buffer;
}

REGISTER_TEST(dragon4);