#include"errol/errol.h"
#include"test.h"

// int errol0_dtoa(double val, char *buf);
// int errol1_dtoa(double val, char *buf, bool *opt);
// int errol2_dtoa(double val, char *buf, bool *opt);
// int errol3_dtoa(double val, char *buf);
// int errol3u_dtoa(double val, char *buf);
// int errol4_dtoa(double val, char *buf);
// int errol4u_dtoa(double val, char *buf);
// int errol_int(double val, char *buf);
// int errol_fixed(double val, char *buf);

char* dtoa_errol(double value,char* buffer){
    char * buf_start = buffer;
    bool opt = true;
    //this func can not pass the test; so do not use
    int exp10 = errol4_dtoa(value, buf_start);
    int len = strlen(buffer);
    //printf("len=%d\n",len);
    //if(value == 0.1 )printf("e10=%d\n", exp10);
    //printf("buf_start=%p,buffer=%p\n",buf_start+len,buffer);
    sprintf(buf_start+len,"e%d",exp10);
    //errol_fixed(value, buffer);
	return buffer;

}

// not pass the test 
// REGISTER_TEST(errol);