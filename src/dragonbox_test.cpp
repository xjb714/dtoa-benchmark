#include"dragonbox/dragonbox.h"
#include"test.h"


char* dtoa_dragonbox(double value,char* buffer){
    char * end_buffer = dragonbox::Dtoa(buffer,value);
    *end_buffer = '\0';
    //printf("value = %.16le buffer = %s\n",value,buffer);
    return buffer;
}

REGISTER_TEST(dragonbox);