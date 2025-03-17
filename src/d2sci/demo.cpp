#include "d2sci.h"
#include <cmath>
#include <iostream>

void test_d2sci(){
    double v=1.3;
    char buf[32];
    int len = d2sci(v,buf);//d2sci_sse(v,buf);
    printf("len=%d,%s",len,buf);
}
void test_d2sci_32v(){
    double v32[32];
    for(int i=0;i<32;++i) v32[i] = (i+1)*sin(1);
    char buffer_1024[1024];
    char* buf_32[32];
    for(int i=0;i<32;++i) buf_32[i]=&buffer_1024[i*32];
    d2sci_32v(v32,buf_32);
    for(int i=0;i<32;++i) printf("i=%2d,%.16le %s\n",i,v32[i],buf_32[i]);
}
void test_d2sci_32(){
    double v32[32];
    for(int i=0;i<32;++i) v32[i] = (i+1)*sin(1);
    char buffer_1024[1024];
    int len = d2sci_32(v32,buffer_1024);
    printf("len=%d\n%s",len,buffer_1024);
}
void test_d2s_32v(){
    double v32[32];
    for(int i=0;i<32;++i) v32[i] = (i+1)*sin(1);
    char buffer_1024[1024];
    char* buf_32[32];
    for(int i=0;i<32;++i) buf_32[i]=&buffer_1024[i*32];
    d2s_32v(v32,buf_32);
    for(int i=0;i<32;++i) printf("i=%2d,%.17lg %s\n",i,v32[i],buf_32[i]);
}
void test_d2s()
{
    double v[8] = {1e-123,-0.0123,1.23,123,123.45,10000,1e8,1.23e100};
    char buf[32];
    for(int i=0;i<8;++i){
        int len = d2s_sse(v[i],buf);//d2s_avx512(v[i],buf);
        printf("len=%d,%s\n",len,buf);
    }
}
int main(){
    // test_d2sci();
    // test_d2sci_32v();
    // test_d2sci_32();
    // test_d2s_32v();
    test_d2s();
    printf("\n");
    return 0;
}