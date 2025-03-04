<!-- Required extensions: pymdownx.betterem, pymdownx.tilde, pymdownx.emoji, pymdownx.tasklist, pymdownx.superfences -->

# dtoa Benchmark

Copyright(c) 2019-2020 Leonid Yuriev <leo@yuriev.ru>,
Copyright(c) 2014 Milo Yip <miloyip@gmail.com>

Due to anonymity requirements, the author email address is not listed.

## Preamble

This fork of the benchmark was created to demonstrate the performance superiority of my new `dtoa()` implementation over others.

All the source code can be found in the **src/d2sci** directory.

Because the number of double values is too large to be fully tested, there may be bugs. If there is any problem with the running results, please give feedback.
There may be clerical errors in the relevant notes, please understand.

## Introduction

This benchmark evaluates the performance of conversion from double precision IEEE-754 floating point (`double`) to ASCII string. The function prototype is:

~~~~~~~~cpp
void dtoa(double value, char* buffer);
~~~~~~~~

The character string result **must** be convertible to the original value **exactly** via some correct implementation of `strtod()`, i.e. roundtrip convertible.

Note that `dtoa()` is *not* a standard function in C and C++.

All new APIs are as follows:
~~~~~~~~cpp
extern "C"{
    int d2sci_sse(double value, char *buffer);// print 1 double to 1 buffer ; digit 17;sse instruction
    int d2sci_avx512(double value,char* buffer);// print 1 double to 1 buffer ; digit 17;avx512 instruction 
    int d2sci_avx512_lut(double value,char* buffer);// print 1 double to 1 buffer ; digit 17;avx512 instruction + look up table
    int d2sci_lut(double value,char* buffer);// print 1 double to 1 buffer ; digit 17;look up table
    int d2sci(double value,char* buffer);// same as d2sci_avx512_lut
    //performance 
    //d2sci = d2sci_avx512_lut > d2sci_avx512 > d2sci_sse > d2sci_lut

    int d2sci_32(double* value,char* buffer);// print 32 double to 1 buffer ; digit 17 ; avx512 instruction
    int d2sci_32_init();//
    void d2sci_32v(double* value,char** buffer);// print 32 double to 32 buffer ; digit 17 ; avx512 instruction
    void d2s_32v(double* value,char** buffer);// print 32 double to 32 buffer ; shortest significant result;avx512 instruction
    int d2s_avx512(double value, char *buffer);//print 1 double to 1 buffer ; shortest significant result;avx512 instruction
    int d2s_sse(double value, char *buffer);//print 1 double to 1 buffer ; shortest significant result;sse instruction
}
~~~~~~~~

## API Introduction
Some API need CPU support AVX512 instruction set.


1. Print 1 double values to 1 buffers.
   The print result of double value contains 17 digits.
   return write buffer length;
   All functions have the same output.
~~~~~~~~cpp
int d2sci           (double value, char* buffer);//need avx512 instruction set,same as d2sci_avx512_lut
int d2sci_sse       (double value, char* buffer);//need sse instruction set
int d2sci_avx512    (double value, char* buffer);//need avx512 instruction set
int d2sci_avx512_lut(double value, char* buffer);//need avx512 instruction set
int d2sci_lut       (double value, char* buffer);//use more look up table
~~~~~~~~
~~~~~~~~cpp
//example
double v=1.3;
char buf[32];
int len = d2sci(v,buf);
//int len = d2sci_sse(v,buf);
printf("len=%d,%s",len,buf);//len=22,1.3000000000000000e+00
//special value: when v is 1e-323 print 0.9881312916824931e-323
~~~~~~~~
2. Continuously print 32 double values to the buffer.
The print result of each double value contains 17 digits.
return write buffer length;
~~~~~~~~cpp
int d2sci_32(double* value, char* buffer);
~~~~~~~~
~~~~~~~~cpp
//example
    double v32[32];
    for(int i=0;i<32;++i) v32[i] = (i+1)*sin(1);
    char buffer_1024[1024];
    int len = d2sci_32(v32,buffer_1024);
    printf("len=%d\n%s",len,buffer_1024);
//print result
len=768
8.4147098480789650e-01 
1.6829419696157930e+00 
2.5244129544236892e+00 
3.3658839392315860e+00 
4.2073549240394827e+00 
5.0488259088473785e+00 
5.8902968936552753e+00 
6.7317678784631720e+00 
7.5732388632710687e+00 
8.4147098480789654e+00 
9.2561808328868622e+00 
1.0097651817694757e+01 
1.0939122802502654e+01 
1.1780593787310551e+01 
1.2622064772118447e+01 
1.3463535756926344e+01 
1.4305006741734241e+01 
1.5146477726542138e+01 
1.5987948711350034e+01 
1.6829419696157930e+01 
1.7670890680965825e+01 
1.8512361665773724e+01 
1.9353832650581619e+01 
2.0195303635389514e+01 
2.1036774620197412e+01 
2.1878245605005307e+01 
2.2719716589813206e+01 
2.3561187574621101e+01 
2.4402658559428999e+01 
2.5244129544236894e+01 
2.6085600529044793e+01 
2.6927071513852688e+01
//Nan or inf will not be printed when the input value is nan or inf.
~~~~~~~~



3. Print 32 double values to 32 buffers.
The print result of each double value contains 17 digits.
~~~~~~~~cpp
void d2sci_32v(double* value, char** buffer);
~~~~~~~~
~~~~~~~~cpp
//example
    double v32[32];
    for(int i=0;i<32;++i) v32[i] = (i+1)*sin(1);
    char buffer_1024[1024];
    char* buf_32[32];
    for(int i=0;i<32;++i) buf_32[i]=&buffer_1024[i*32];
    d2sci_32v(v32,buf_32);
    for(int i=0;i<32;++i) printf("i=%d,%.16le %s\n",i,v32[i],buf_32[i]);
//print result 
i= 0,8.4147098480789650e-01 8.4147098480789650e-01
i= 1,1.6829419696157930e+00 1.6829419696157930e+00
i= 2,2.5244129544236893e+00 2.5244129544236893e+00
i= 3,3.3658839392315860e+00 3.3658839392315860e+00
i= 4,4.2073549240394827e+00 4.2073549240394827e+00
i= 5,5.0488259088473786e+00 5.0488259088473786e+00
i= 6,5.8902968936552753e+00 5.8902968936552753e+00
i= 7,6.7317678784631720e+00 6.7317678784631720e+00
i= 8,7.5732388632710688e+00 7.5732388632710688e+00
i= 9,8.4147098480789655e+00 8.4147098480789655e+00
i=10,9.2561808328868622e+00 9.2561808328868622e+00
i=11,1.0097651817694757e+01 1.0097651817694757e+01
i=12,1.0939122802502654e+01 1.0939122802502654e+01
i=13,1.1780593787310551e+01 1.1780593787310551e+01
i=14,1.2622064772118447e+01 1.2622064772118447e+01
i=15,1.3463535756926344e+01 1.3463535756926344e+01
i=16,1.4305006741734241e+01 1.4305006741734241e+01
i=17,1.5146477726542138e+01 1.5146477726542138e+01
i=18,1.5987948711350034e+01 1.5987948711350034e+01
i=19,1.6829419696157931e+01 1.6829419696157931e+01
i=20,1.7670890680965826e+01 1.7670890680965826e+01
i=21,1.8512361665773724e+01 1.8512361665773724e+01
i=22,1.9353832650581619e+01 1.9353832650581619e+01
i=23,2.0195303635389514e+01 2.0195303635389514e+01
i=24,2.1036774620197413e+01 2.1036774620197413e+01
i=25,2.1878245605005308e+01 2.1878245605005308e+01
i=26,2.2719716589813206e+01 2.2719716589813206e+01
i=27,2.3561187574621101e+01 2.3561187574621101e+01
i=28,2.4402658559429000e+01 2.4402658559429000e+01
i=29,2.5244129544236895e+01 2.5244129544236895e+01
i=30,2.6085600529044793e+01 2.6085600529044793e+01
i=31,2.6927071513852688e+01 2.6927071513852688e+01
//Nan or inf will not be printed when the input value is nan or inf.
~~~~~~~~
4. Print 32 double values to 32 buffers.
this API base on Schubfach Algorithm;AVX-512 implementation.
The print result of each double value contains the shortest significant digit.
~~~~~~~~cpp
void d2s_32v(double* value, char** buffer);
~~~~~~~~
~~~~~~~~cpp
//example
    double v32[32];
    for(int i=0;i<32;++i) v32[i] = (i+1)*sin(1);
    char buffer_1024[1024];
    char* buf_32[32];
    for(int i=0;i<32;++i) buf_32[i]=&buffer_1024[i*32];
    d2s_32v(v32,buf_32);
    for(int i=0;i<32;++i) printf("i=%2d,%.17lg %s\n",i,v32[i],buf_32[i]);
//print result
i= 0,0.8414709848078965 8.414709848078965e-01
i= 1,1.682941969615793 1.682941969615793e+00
i= 2,2.5244129544236893 2.5244129544236893e+00
i= 3,3.365883939231586 3.365883939231586e+00
i= 4,4.2073549240394827 4.207354924039483e+00
i= 5,5.0488259088473786 5.048825908847379e+00
i= 6,5.8902968936552753 5.890296893655275e+00
i= 7,6.731767878463172 6.731767878463172e+00
i= 8,7.5732388632710688 7.573238863271069e+00
i= 9,8.4147098480789655 8.414709848078965e+00
i=10,9.2561808328868622 9.256180832886862e+00
i=11,10.097651817694757 1.0097651817694757e+01
i=12,10.939122802502654 1.0939122802502654e+01
i=13,11.780593787310551 1.178059378731055e+01
i=14,12.622064772118447 1.2622064772118447e+01
i=15,13.463535756926344 1.3463535756926344e+01
i=16,14.305006741734241 1.430500674173424e+01
i=17,15.146477726542138 1.5146477726542138e+01
i=18,15.987948711350034 1.5987948711350034e+01
i=19,16.829419696157931 1.682941969615793e+01
i=20,17.670890680965826 1.7670890680965826e+01
i=21,18.512361665773724 1.8512361665773724e+01
i=22,19.353832650581619 1.935383265058162e+01
i=23,20.195303635389514 2.0195303635389514e+01
i=24,21.036774620197413 2.1036774620197413e+01
i=25,21.878245605005308 2.1878245605005308e+01
i=26,22.719716589813206 2.2719716589813206e+01
i=27,23.561187574621101 2.35611875746211e+01
i=28,24.402658559429 2.4402658559429e+01
i=29,25.244129544236895 2.5244129544236895e+01
i=30,26.085600529044793 2.6085600529044793e+01
i=31,26.927071513852688 2.6927071513852688e+01
~~~~~~~~
5. Print 1 double values to 1 buffers.
this API base on Schubfach Algorithm;
The print result of each double value contains the shortest significant digit.
These two functions have the same output.
return write buffer length;

~~~~~~~~cpp
int d2s_avx512(double value, char* buffer);//need avx512 instruction
int d2s_sse   (double value, char* buffer);//need sse instruction
    // To increase that readability of the result
    // input double v range and print result ;
    // range       : double v ->   print result
    // 1. v<1e-7   : 1.23e-8  ->   1.23e-08  ,  1.23e-11 -> 1.23e-11  , 1.23e-103  -> 1.23e-103
    // 2. [1e-7,1) : 1.23e-3  ->   0.00123   ,  1.23e-1  -> 0.123     , 0.0123     -> 0.0123
    // 3. [1,1e9)  : 1.23e2   ->   123       ,  1.23e5   -> 12300       123.45     -> 123.45
    // 4. >=1e9    : 1.23e10  ->   1.23e+10  ,  1.23e9   -> 1.23e+09  , 1.23e103   -> 1.23e+103
    // Print integer results when the absolute value of input double is an integer between 0 and 2**53-1. 
~~~~~~~~
~~~~~~~~cpp
//example
    double v[8] = {1e-123,-0.0123,1.23,123,123.45,10000,1e8,1.23e100};
    char buf[32];
    for(int i=0;i<8;++i){
        int len = d2s_avx512(v[i],buf);//or d2s_sse(v[i],buf);
        printf("len=%d,%s\n",len,buf);
    }
//print result
len=6,1e-123
len=7,-0.0123
len=4,1.23
len=3,123
len=6,123.45
len=5,10000
len=9,100000000
len=9,1.23e+100
~~~~~~~~
## Procedure

Firstly the program verifies the correctness of all algorithm algorithmimplementations.

There are performance test results in three data modes.  

#### 1.Sequential
Consecutive positive and negative integer values.
#### 2.Random
**Random** case for benchmark is carried out:
* Each iteration generates **2048*64=131072** random `double` values, filtered out `+/-inf` and `nan`.
* Convert these generated numbers into ASCII.
* Iterations are run 10 times taking the minimum time.
#### 3.RandomDigit

**RandomDigit** case for benchmark is carried out:

* Each digit generates **2048*64=131072** random `double` values, filtered out `+/-inf` and `nan`. Then convert them to limited precision (1 to 17 decimal digits in significand).

* Convert these generated numbers into ASCII.

* Each digit group is run 10 times and the minimum time is taken. 

## Build and Run
[1] software environment：**Linux , compiler support c++20**;
example : (1) ubuntu 24.04 ; (2) g++ 13.3 , icpx 2025.0.4 ,clang 18;
[2] hardware environment：**CPU support AVX-512**
example :
(1) intel xeon processor ;
(2) intel 11th Gen core ; i9 11900 ;
(3) AMD Zen4 or Zen5 ; AMD R7 7840H , AMD R7 7950X , AMD R9 9950X.

The steps to compile and run are as follows:
1. Obtain [cmake](https://cmake.org/download/)
2. `cd dtoa-benchmark`
3. `mkdir build`
4. `cd build`
5. `cmake ..`
6. `make -j`
7. `./dtoa-benchmark`
8. The result appears in the running program directory.


## Results

All the source code of **d2sci** is compiled into a static library **libd2sci2.a** through **icpx 2025.0.4** and **clang++ 18.1.3**.

The software runs in the following environment:
(1)AMD R9 9950X
(2)Ubuntu 24.04
(3)icpx 2025.0.4(The results compiled by this compiler have the best performance)

The running results are as follows:

#### 1. Sequential

Function      |  Min ns |  RMS ns  |  Max ns |   Sum ns  | Speedup |
:-------------|--------:|---------:|--------:|----------:|--------:|
null          |   1.065 |     1.067 |    1.071 |    18.147 | ×97.564 |
d2sci_32      |   1.199 |     1.204 |    1.208 |    20.475 | ×86.471 |
d2sci_32v     |   1.196 |     1.208 |    1.212 |    20.529 | ×86.241 |
d2s_32v       |   2.346 |     2.364 |    2.377 |    40.186 | ×44.057 |
d2s_avx512    |   1.775 |     2.957 |    4.322 |    48.389 | ×36.589 |
d2s_sse       |   1.931 |     3.011 |    4.200 |    49.608 | ×35.690 |
d2sci_avx512_lut|   3.125 |     3.136 |    3.144 |    53.305 | ×33.214 |
d2sci_avx512  |   3.542 |     3.556 |    3.566 |    60.444 | ×29.291 |
d2sci_sse     |   3.886 |     3.897 |    3.916 |    66.252 | ×26.723 |
d2sci_lut     |   4.296 |     4.309 |    4.336 |    73.252 | ×24.170 |
erthink       |   3.137 |     5.232 |    7.390 |    85.053 | ×20.816 |
dragonbox     |   4.146 |     5.223 |    6.353 |    87.837 | ×20.156 |
schubfach     |   4.188 |     5.474 |    6.751 |    91.706 | ×19.306 |
ryu           |   4.751 |     6.399 |    7.590 |   107.827 | ×16.420 |
erthink_shodan|   6.346 |    11.594 |   16.321 |   189.279 | ×9.354  |
grisu_exact   |  10.853 |    11.440 |   11.927 |   194.369 | ×9.109  |
emyg          |   6.550 |    13.981 |   18.850 |   231.434 | ×7.650  |
milo          |  11.950 |    18.848 |   23.580 |   315.487 | ×5.612  |
floaxie       |  14.718 |    21.244 |   27.126 |   356.188 | ×4.971  |
fmt           |  14.593 |    21.349 |   25.840 |   359.679 | ×4.922  |
to_chars      |  20.117 |    23.502 |   25.372 |   398.985 | ×4.437  |
doubleconv    |  17.181 |    24.522 |   30.160 |   412.305 | ×4.294  |
grisu2        |  17.401 |    25.041 |   29.866 |   421.024 | ×4.205  |
fpconv        |  17.485 |    30.218 |   39.755 |   499.712 | ×3.543  |
stb           |  30.737 |    34.813 |   38.485 |   590.688 | ×2.997  |
format        |  60.923 |    64.794 |   69.422 |  1100.333 | ×1.609  |
dragon4       |  22.786 |   104.860 |  168.424 |  1615.349 | ×1.096  |
sprintf       |  64.202 |   107.486 |  153.510 |  1770.487 | ×1.000  |
ostrstream    | 123.378 |   172.399 |  225.709 |  2888.738 | ×0.613  |
ostringstream | 115.242 |   176.904 |  264.126 |  2942.899 | ×0.602  |

#### 2. Random
The following are results measured by `Random` testcase.
The speedup is based on `sprintf`'s _Sum_ values.
Time is the average value of printing a single double value.
Function      |   Sum ns  | Speedup |
:-------------|----------:|--------:|
null          |     1.106 | ×250.285 |
d2sci_32      |     1.244 | ×222.384 |
d2sci_32v     |     1.255 | ×220.556 |
d2s_32v       |     2.477 | ×111.741 |
d2sci_avx512_lut|     3.296 | ×83.957 |
d2sci_avx512  |     3.748 | ×73.845 |
d2sci_sse     |     4.099 | ×67.518 |
d2sci_lut     |     4.534 | ×61.039 |
d2s_avx512    |    10.641 | ×26.007 |
d2s_sse       |    10.857 | ×25.491 |
dragonbox     |    18.341 | ×15.089 |
schubfach     |    20.939 | ×13.217 |
ryu           |    25.292 | ×10.942 |
grisu_exact   |    26.534 | ×10.430 |
floaxie       |    29.781 | ×9.293  |
erthink       |    30.003 | ×9.224  |
erthink_shodan|    32.920 | ×8.407  |
emyg          |    46.102 | ×6.003  |
grisu2        |    50.210 | ×5.512  |
to_chars      |    51.354 | ×5.389  |
milo          |    54.160 | ×5.110  |
fmt           |    55.126 | ×5.020  |
stb           |    55.964 | ×4.945  |
fpconv        |    58.811 | ×4.706  |
doubleconv    |    61.243 | ×4.519  |
format        |    89.519 | ×3.092  |
sprintf       |   276.751 | ×1.000  |
ostrstream    |   356.084 | ×0.777  |
ostringstream |   392.724 | ×0.705  |
dragon4       |   582.747 | ×0.475  |


#### 3. RandomDigit
The following are results measured by `RandomDigit` testcase.
The speedup is based on `sprintf`'s _Sum_ values.
Time is the average value of printing a single double value.
Function      |  Min ns |  RMS ns  |  Max ns |   Sum ns  | Speedup |
:-------------|--------:|---------:|--------:|----------:|--------:|
null          |   1.113 |     1.131 |    1.247 |    19.210 | ×226.579 |
d2sci_32v     |   1.266 |     1.267 |    1.269 |    21.542 | ×202.056 |
d2sci_32      |   1.249 |     1.268 |    1.277 |    21.561 | ×201.879 |
d2s_32v       |   2.460 |     2.494 |    2.512 |    42.395 | ×102.670 |
d2sci_avx512_lut|   3.288 |     3.304 |    3.324 |    56.161 | ×77.504 |
d2sci_avx512  |   3.745 |     3.762 |    3.792 |    63.955 | ×68.058 |
d2sci_sse     |   4.094 |     4.108 |    4.121 |    69.840 | ×62.323 |
d2sci_lut     |   4.512 |     4.530 |    4.565 |    77.007 | ×56.523 |
d2s_avx512    |  10.698 |    10.767 |   10.849 |   183.036 | ×23.780 |
d2s_sse       |  10.843 |    10.905 |   11.009 |   185.383 | ×23.479 |
dragonbox     |  10.998 |    13.292 |   18.586 |   224.172 | ×19.417 |
schubfach     |  12.411 |    14.578 |   19.204 |   246.487 | ×17.659 |
floaxie       |  13.546 |    22.808 |   29.610 |   378.276 | ×11.507 |
erthink_shodan|  18.991 |    23.285 |   33.007 |   392.356 | ×11.094 |
erthink       |  19.903 |    23.970 |   30.122 |   405.033 | ×10.746 |
grisu_exact   |  24.571 |    25.343 |   26.702 |   430.728 | ×10.105 |
ryu           |  24.163 |    29.153 |   33.886 |   492.545 | ×8.837  |
emyg          |  32.307 |    40.927 |   47.269 |   693.229 | ×6.279  |
grisu2        |  34.908 |    41.516 |   50.512 |   702.151 | ×6.199  |
milo          |  29.044 |    43.273 |   55.044 |   726.322 | ×5.993  |
fmt           |  33.700 |    43.589 |   55.147 |   736.072 | ×5.913  |
doubleconv    |  35.051 |    48.577 |   60.974 |   819.179 | ×5.313  |
fpconv        |  42.804 |    50.401 |   58.940 |   854.342 | ×5.095  |
to_chars      |  50.101 |    50.860 |   52.198 |   864.529 | ×5.035  |
stb           |  53.285 |    55.175 |   56.037 |   937.914 | ×4.641  |
format        |  88.883 |    91.972 |   94.056 |  1563.392 | ×2.784  |
sprintf       | 232.795 |   256.283 |  269.735 |  4352.681 | ×1.000  |
ostrstream    | 316.879 |   339.209 |  354.910 |  5763.944 | ×0.755  |
dragon4       | 132.041 |   389.714 |  583.975 |  6185.312 | ×0.704  |
ostringstream | 344.971 |   370.825 |  389.556 |  6299.091 | ×0.691  |

![results/Random_result_9950x.png](results/Random_result_9950x.png)



## Implementations in descending order of speed

The following ranking results are from **2.Random** test results.
Because some functions perform very closely, the sorting results may not match the test results.

Function      | Description
--------------|-----------
d2sci_32v | Base on AVX-512, print 32 double to 32 buffer.
d2sci_32  | Base on AVX-512, print 32 double to 1 buffer.
d2s_32v   | Base on Schubfach and AVX-512, print 32 double to 32 buffer.
d2sci_avx512_lut     | Base on AVX-512, print 1 double to 1 buffer.(Same as d2sci)
d2sci_avx512     | Base on AVX-512, print 1 double to 1 buffer.
d2sci_sse | Base on sse, print 1 double to 1 buffer.
d2sci_lut | use more look up table, print 1 double to 1 buffer.
d2s_avx512 | Base on Schubfach and AVX-512, print 1 double to 1 buffer.
d2s_sse | Base on Schubfach and sse, print 1 double to 1 buffer.
[dragonbox](https://github.com/jk-jeon/dragonbox) | Junekey Jeon's CPP implementation.
[schubfach](https://github.com/abolz/Drachennest) | Raffaello Giulietti's algorithm.Origin source code from https://github.com/c4f7fcce9cb06515/Schubfach
[ryu](https://github.com/ulfjack/ryu) | Ulf Adams's [Ryū algorithm](https://dl.acm.org/citation.cfm?id=3192369).
[grisu_exact](https://github.com/jk-jeon/Grisu-Exact) | Junekey Jeon's CPP implementation.
[floaxie](https://github.com/aclex/floaxie) | Alexey Chernov's Grisu2 implementation.
[erthink](https://github.com/erthink/erthink/blob/master/erthink_d2a.h) | Leonid Yuriev's Grisu-based C++ implementation.
[erthink_shodan](https://github.com/erthink/erthink/blob/master/erthink_d2a.h) | Leonid Yuriev's Grisu-based C++ implementation.
[emyg](https://github.com/miloyip/dtoa-benchmark/blob/master/src/emyg/emyg_dtoa.c) | C version of Milo Yip's Grisu2 implementation by Doug Currie.
[grisu2](http://florian.loitsch.com/publications/bench.tar.gz?attredirects=0) | Florian Loitsch's Grisu2 C implementation [1].
to_chars | std::to_chars();
[stb](https://github.com/nothings/stb)         | Jeff Roberts's & Sean Barrett's snprintf() implementation.
[milo](https://github.com/miloyip/dtoa-benchmark/blob/master/src/milo/dtoa_milo.h) | Milo Yip's Grisu2 C++ implementation for RapidJSON.
[fmt](https://github.com/fmtlib/fmt) | Victor Zverovich's Grisu2 implementation for `{fmt}` C++ library.
[fpconv](https://github.com/night-shift/fpconv) | Andreas Samoljuk's Grisu2 C implementation.
[doubleconv](https://code.google.com/p/double-conversion/) |  C++ implementation extracted from Google's V8 JavaScript Engine with `EcmaScriptConverter().ToShortest()` (based on Grisu3, fall back to slower bignum algorithm when Grisu3 failed to produce shortest implementation).
format | C++20 stdlib, `std::format("{:.17g}")`
sprintf       | `sprintf()` in C standard library with `"%.17g"` format.
ostrstream    | traditional `std::ostrstream` in C++ standard library with `setprecision(17)`.
ostringstream | traditional `std::ostringstream` in C++ standard library with `setprecision(17)`.
[dragon4](https://github.com/ahaldane/Dragon4) | A copy of Ryan Juckett's Dragon4 implementations, see http://www.ryanjuckett.com/programming/printing-floating-point-numbers/ with modifications for IEEE unbiased rounding and a bugfix.
~~[gay](http://www.netlib.org/fp/)~~ | David M. Gay's `dtoa()` C implementation. **Disabled** because of invalid results and/or SIGSEGV.
~~[Errol](https://github.com/marcandrysco/Errol)~~ | Marcandrysco's `dtoa()` C implementation. **Disabled** because of endless loop and/or invalid results and/or SIGSEGV.
## FAQ

1. How to add an implementation?

   You may clone an existing implementation file, then modify it and add to `CMakeLists.txt`.
   Re-run `cmake` to re-configure and re-build benchmark.
   Note that it will automatically register to the benchmark by macro `REGISTER_TEST(name)`.

   **Making pull request of new implementations is welcome.**

2. Why not converting `double` to `std::string`?

   It may introduce heap allocation, which is a big overhead. User can easily wrap these low-level functions to return `std::string`, if needed.

3. Why fast `dtoa()` functions is needed?

   They are a very common operations in writing data in text format. The standard way of `sprintf()`, `std::stringstream`, often provides poor performance. 

## References

[1] Loitsch, Florian. ["Printing floating-point numbers quickly and accurately with integers."](http://florian.loitsch.com/publications/dtoa-pldi2010.pdf) ACM Sigplan Notices 45.6 (2010): 233-243.

To be supplemented...

## Related Benchmarks and Discussions

* [Printing Floating-Point Numbers](http://www.ryanjuckett.com/programming/printing-floating-point-numbers/)
