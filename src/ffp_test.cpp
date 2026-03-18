#include"ffp/ffp.h"
#include"test.h"

char* dtoa_ffp(double value,char* buffer){
	ffp64(value,buffer);
    return buffer;
}


REGISTER_TEST(ffp);