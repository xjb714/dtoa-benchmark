#pragma once

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class Timer {
public:
  Timer() : start_(), end_() {}

  void Start() { QueryPerformanceCounter(&start_); }

  void Stop() { QueryPerformanceCounter(&end_); }

  double GetElapsedMilliseconds() {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    return (end_.QuadPart - start_.QuadPart) * 1000.0 / freq.QuadPart;
  }

private:
  LARGE_INTEGER start_;
  LARGE_INTEGER end_;
};

// Undefine Windows bad macros
#undef min
#undef max

#else

#include <sys/time.h>
#include <chrono>

class Timer {
public:
  Timer() : start_(), end_() {}

  void Start() { 
    //gettimeofday(&start_, NULL);
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    time1 = nanos;
  }

  void Stop() {
    //gettimeofday(&end_, NULL); 
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    time2 = nanos;
    //duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
  }

  double GetElapsedMilliseconds() {
    return (end_.tv_sec - start_.tv_sec) * 1000.0 +
           (end_.tv_usec - start_.tv_usec) / 1000.0 ;
    //return duration_ns * 1e-6;
  }
  double GetElapsedMicroseconds() {
    return (end_.tv_sec - start_.tv_sec) * 1e6 +
           (end_.tv_usec - start_.tv_usec) ;
    //return duration_ns * 1e-3;
  }

  double GetElapsedNanoseconds() {
    return (double)(time2 - time1);
    //return (double)duration_ns;
  }

private:
  struct timeval start_;
  struct timeval end_;
  //std::chrono::_V2::system_clock::time_point t1,t2;
  long long time1,time2;
  long long duration_ns;
};

#endif
