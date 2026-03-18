#include "ffp.cpp"
#include <cmath>
#include <iostream>
void test_ffp(){
    double v32[32];
    for(int i=0;i<32;++i) v32[i] = (i+1)*sin(1);
    char buf[32];
    for(int i=0;i<32;++i) {
        ffp64(v32[i],buf);
        printf("i=%2d,%.16le %s\n",i,v32[i],buf);
    }
}
void test_ffp_input(){
    int i=0;
    while(i++ < 30){
        printf("input a double number:");
        double v;
        std::cin>> v;
        char buf[32];
        memset(buf,0,sizeof(buf));
        ffp64(v,buf);
        printf("i=%2d,%.16le %s\n",i,v,buf);
    }
}
int main(){
    test_ffp();
    test_ffp_input();
    return 0;
}