extern "C"
{
    int d2sci_sse(double value, char *buffer);// print 1 double to 1 buffer ; digit 17;sse instruction
    int d2sci(double value,char* buffer);// print 1 double to 1 buffer ; digit 17;avx512 instruction
    int d2sci_32(double* value,char* buffer);// print 32 double to 1 buffer ; digit 17 ; avx512 instruction
    int d2sci_32_init();// 
    void d2sci_32v(double* value,char** buffer);// print 32 double to 32 buffer ; digit 17 ; avx512 instruction
    void d2s_32v(double* value,char** buffer);// print 32 double to 32 buffer ; shortest significant result;avx512 instruction
    int d2s_avx512(double value, char *buffer);//print 1 double to 1 buffer ; shortest significant result;avx512 instruction
    int d2s_sse(double value, char *buffer);//print 1 double to 1 buffer ; shortest significant result;sse instruction
}