#include <algorithm>
#include <cassert>
#include <cstring>
#include <exception>
#if _MSC_VER
#include "msinttypes/stdint.h"
#else
#include <stdint.h>
#endif
#include "double-conversion/double-conversion.h"
#include "resultfilename.h"
#include "test.h"
#include "timer.h"
#include <cstdio>
#include <cstdlib>
#include <random>
#include <iostream>
#include <immintrin.h>
#include <cpuid.h>

const unsigned kVerifyRandomCount = 100000;
const unsigned kIterations = 2048 * 64;
const unsigned kRandomCount = 2048 * 64;// equal to kIterations
const unsigned kTrial = 10;
const int kMaxDigits = 17;//can not change;

std::random_device rd; // 用于获取种子
std::mt19937_64 gen(rd()); // 以随机设备 rd 初始化 Mersenne Twister 生成器

std::string cpuName;

class Random {
public:
  Random(unsigned seed = 0) : mSeed(seed) {}

  uint64_t operator()() {
    // mSeed = UINT64_C(6364136223846793005) * mSeed +
    // UINT64_C(1442695040888963407); mSeed = (uint64_t)(gen());
    // mSeed = (gen() & 0x800FFFFFFFFFFFFF) | (((gen() % 200) + (1023 - 100)) <<
    // 52);
    mSeed = gen();
    return mSeed;
  }

private:
  uint64_t mSeed;
};

static size_t VerifyValue(double value, char *(*f)(double, char *)) {
  char buffer[1024];
  for (int i = 0; i < 1024; ++i)
    buffer[i] = 0;
  char *str = f(value, buffer);
  // double-conversion returns correct result.
#if 0
  char *end = nullptr;
  const double roundtrip = strtod(str, &end);
  const size_t processed = end ? end - str : strnlen(str, sizeof(str));
#else
  using namespace double_conversion;
  StringToDoubleConverter converter(
      StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0.0, 0.0, NULL, NULL);
  int processed = 0;
  double roundtrip = converter.StringToDouble(str, 1024, &processed);
#endif
  size_t len = strlen(str);
  if (len != (size_t)processed) {
    printf("Warning: some extra character %g -> '%s'\n", value, str);
    //throw std::exception();
  }
  if (value != roundtrip) {
    printf(": roundtrip fail %.17g -> '%s' -> %.17g\n", value, str, roundtrip);
    throw std::exception();
  }
  return len;
}

static size_t VerifyValue(double *value, char *(*f)(double *, char *)) {
  char buffer[1024];
  for (int i = 0; i < 1024; ++i)
    buffer[i] = 0;
  char *str = f(value, buffer);
  // double-conversion returns correct result.
#if 0
  char *end = nullptr;
  const double roundtrip = strtod(str, &end);
  const size_t processed = end ? end - str : strnlen(str, sizeof(str));
#else
  using namespace double_conversion;
  StringToDoubleConverter converter(
      StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0.0, 0.0, NULL, NULL);
  int processed = 0;
  double roundtrip = converter.StringToDouble(str, 1024, &processed);
#endif
  //size_t len = strlen(str);
  char* ptr = str;
  //while(*ptr!=' ' && *ptr!='\n' && *ptr!='\0' && *ptr!='\t') ++ptr;
  while(*ptr=='+' ||*ptr=='-' || (*ptr>='0' && *ptr<='9') || *ptr=='.'|| *ptr=='e') ++ptr;
  size_t len = ptr - str;
  if (len != (size_t)processed) {
    printf("Warning: some extra character %g -> '%s'\n", value[0], str);
    throw std::exception();
  }
  if (value[0] != roundtrip) {
    printf(": roundtrip fail %.17g -> '%s' -> %.17g\n", value[0], str,
           roundtrip);
    throw std::exception();
  }
  return len;
}

static size_t VerifyValue(double *value, char **(*f)(double *, char **)) {
  char buffer[1024];
  for (int i = 0; i < 1024; ++i)
    buffer[i] = 0;
  char *buf[32];
  for (int i = 0; i < 32; ++i) {
    buf[i] = &buffer[i * 32];
  }
  char **str = f(value, (char **)buf);
  // double-conversion returns correct result.
#if 0
  char *end = nullptr;
  const double roundtrip = strtod(str, &end);
  const size_t processed = end ? end - str : strnlen(str, sizeof(str));
#else
  using namespace double_conversion;
  StringToDoubleConverter converter(
      StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0.0, 0.0, NULL, NULL);
  int processed = 0;
  double roundtrip = converter.StringToDouble(str[0], 1024, &processed);
#endif
  size_t len = strlen(str[0]);
  if (len != (size_t)processed) {
    printf("Warning: some extra character %g -> '%s'\n", value[0], str[0]);
    throw std::exception();
  }
  if (value[0] != roundtrip) {
    printf(": roundtrip fail %.17g -> '%s' -> %.17g\n", value[0], str[0],
           roundtrip);
    throw std::exception();
  }
  return len;
}

static void Verify(const Case *it) {
  printf("Verifying %s...", it->fname);
  fflush(nullptr);
  

  auto VerifySingleValue = [&it](double value) -> int {
    double value_array[32];
    if (*it->dtoa) 
    {
      return VerifyValue(value, it->dtoa);
    } else 
    if (*it->dtoa_batch_seq) 
    {
      value_array[0] = value;
      return VerifyValue(value_array, it->dtoa_batch_seq);
    } else 
    if (*it->dtoa_batch_split) 
    {
      value_array[0] = value;
      return VerifyValue(value_array, it->dtoa_batch_split);
    }
    return 0;
  };
  double E100=1e100;
  long long E100_plus1 = *(long long*)&E100 - 1;
  long long E100_add1 = *(long long*)&E100 + 1;
  double E100_p1 = *(double *)&E100_plus1;
  double E100_a1 = *(double *)&E100_add1;
  double test_data[] = {0,-0,
                        0.1,-0.1,
                        0.12,-0.12,
                        0.123,-0.123,
                        0.1234,-0.1234,
                        1.2345,-1.2345,
                        1.0 / 3.0,-1.0/3.0,
                        2.0 / 3.0,-2.0/3.0,
                        10.0 / 3.0,-10.0/3.0,
                        20.0 / 3.0,-20.0/3.0,
                        1e5,-1e5,1e-5,-1e-5,
                        1e10,-1e10,1e-10,-1e-10,
                        1e100,-1e100,1e-100,-1e-100,
                        1e200,-1e200,1e-200,-1e-200,
                        1e300,-1e300,1e-300,-1e-300,
                        1e99,-1e99,1e-99,-1e-99,
                        E100_p1, -E100_p1,
                        E100_a1, -E100_a1,
                        std::numeric_limits<double>::min(),-std::numeric_limits<double>::min(),
                        std::numeric_limits<double>::max(),-std::numeric_limits<double>::max(),
                        std::numeric_limits<double>::denorm_min(),-std::numeric_limits<double>::denorm_min()
                        };

  for (int i = 0; i < (sizeof(test_data) / sizeof(double)); ++i) {
    VerifySingleValue(test_data[i]);
  }
  // Boundary and simple cases
  // VerifyValue(0, it->dtoa);
  // VerifyValue(0.1, it->dtoa);
  // VerifyValue(0.12, it->dtoa);
  // VerifyValue(0.123, it->dtoa);
  // VerifyValue(0.1234, it->dtoa);
  // VerifyValue(1.2345, it->dtoa);
  // VerifyValue(1.0 / 3.0, it->dtoa);
  // VerifyValue(2.0 / 3.0, it->dtoa);
  // VerifyValue(10.0 / 3.0, it->dtoa);
  // VerifyValue(20.0 / 3.0, it->dtoa);
  // VerifyValue(std::numeric_limits<double>::min(), it->dtoa);
  // VerifyValue(std::numeric_limits<double>::max(), it->dtoa);
  // VerifyValue(std::numeric_limits<double>::denorm_min(), it->dtoa);

  Random rnd;
  union {
    double d;
    uint64_t u;
  } u;

  u.u = UINT64_C(0x345E0FFED391517E);
  // VerifyValue(u.d, it->dtoa);
  VerifySingleValue(u.d);
  u.u = UINT64_C(0xF6EA6C767640CD71);
  // VerifyValue(u.d, it->dtoa);
  VerifySingleValue(u.d);

  uint64_t lenSum = 0;
  size_t lenMax = 0;
  for (unsigned i = 0; i < kVerifyRandomCount; i++) {
    do
      u.u = rnd();
    while (isnan(u.d) || isinf(u.d));
    // size_t len = VerifyValue(u.d, it->dtoa);
    size_t len = VerifySingleValue(u.d);
    lenSum += len;
    lenMax = std::max(lenMax, len);
  }

  double lenAvg = double(lenSum) / kVerifyRandomCount;
  printf(" OK. Length Avg = %2.3f, Max = %d\n", lenAvg, (int)lenMax);
  fflush(nullptr);
}

void VerifyAll() {
  const TestList &tests = TestManager::Instance().GetTests();

  for (TestList::const_iterator itr = tests.begin(); itr != tests.end();
       ++itr) {
    if (strcmp((*itr)->fname, "null") != 0) { // skip null
      try {
        Verify(*itr);
      } catch (...) {
      }
    }
  }
}

//------------------------------------------------------------------------------

void BenchSequential(Case *it, FILE *fp) {
  printf("Benchmarking  sequential %s...", it->fname);
  //fflush(nullptr);
  //fprintf(fp, "\n%s", it->fname);

  Random rnd;
  char buffer[2048];
  it->reset();
  static_assert(kIterations % 32 == 0, "WTF");
  int64_t start = 1;
  for (int digit = 1; digit <= kMaxDigits; digit++) {
    int64_t end = start * 10;
    double duration = std::numeric_limits<double>::max();

    if (it->dtoa) {
      double *tmp_arr = (double *)malloc(kIterations * sizeof(double));
      for (unsigned trial = 0; trial < kTrial; trial++) {
        int64_t v = start + int64_t(rnd()) % start;
        double sign = 1;
        for (unsigned i = 0; i < kIterations; i++) {
          tmp_arr[i] = v * sign;
          sign = -sign;
          v += 1;
          if (v >= end)
            v = start;
        }
        Timer timer;
        timer.Start();
        for (unsigned iteration = 0; iteration < kIterations; iteration++) {
          it->dtoa(tmp_arr[iteration], buffer);
        }
        timer.Stop();
        duration = std::min(duration, timer.GetElapsedNanoseconds());
      }
      free(tmp_arr);
    } else if (it->dtoa_batch_seq) {
      double *tmp_arr = (double *)malloc(kIterations * sizeof(double));
      for (unsigned trial = 0; trial < kTrial; trial++) {
        int64_t v = start + int64_t(rnd()) % start;
        double sign = 1;
        for (unsigned i = 0; i < kIterations; i++) {
          tmp_arr[i] = v * sign;
          sign = -sign;
          v += 1;
          if (v >= end)
            v = start;
        }
        Timer timer;
        timer.Start();
        for (unsigned iteration = 0; iteration < kIterations; iteration += 32) {
          it->dtoa_batch_seq(&tmp_arr[iteration], buffer);
        }
        timer.Stop();
        duration = std::min(duration, timer.GetElapsedNanoseconds());
      }
      free(tmp_arr);
    } else if (it->dtoa_batch_split) {
      char *buf[32];
      for (int i = 0; i < 32; ++i)
        buf[i] = &buffer[i * 32];
      double *tmp_arr = (double *)malloc(kIterations * sizeof(double));
      for (unsigned trial = 0; trial < kTrial; trial++) {
        int64_t v = start + int64_t(rnd()) % start;
        double sign = 1;
        for (unsigned i = 0; i < kIterations; i++) {
          tmp_arr[i] = v * sign;
          sign = -sign;
          v += 1;
          if (v >= end)
            v = start;
        }
        Timer timer;
        timer.Start();
        for (unsigned iteration = 0; iteration < kIterations; iteration += 32) {
          it->dtoa_batch_split(&tmp_arr[iteration], buf);
        }
        timer.Stop();
        duration = std::min(duration, timer.GetElapsedNanoseconds());
      }
      free(tmp_arr);
    }

    duration *= 1.0 / kIterations; // convert to nano second per operation
    it->account(duration);
    //fprintf(fp, "sequential,%s,%d,%f\n", it->fname, digit, duration);
    //fprintf(fp, ",%f",duration);
    start = end;
  }
  printf(" Done\n");
  //fflush(nullptr);
}

//------------------------------------------------------------------------------

class RandomData {
public:
  static double *GetData() {
    static RandomData singleton;
    return singleton.mData;
  }

private:
  RandomData() : mData(new double[kRandomCount]) {
    Random rnd;
    union {
      double d;
      uint64_t u;
    } u;

    for (size_t i = 0; i < kRandomCount; i++) {
      do
        u.u = rnd();
      while (isnan(u.d) | isinf(u.d));
      mData[i] = u.d;
    }
  }

  ~RandomData() { delete[] mData; }

  double *mData;
};

void BenchRandom(Case *it, FILE *fp) {
  printf("Benchmarking      random %s...", it->fname);
  fflush(nullptr);

  char buffer[1024]; // contain 32 double
  it->reset();
  double *data = RandomData::GetData();

  double duration = std::numeric_limits<double>::max();
  static_assert(kIterations % kRandomCount == 0, "Oops");
  static_assert(kRandomCount % 32 == 0, "Oops");
  if (it->dtoa) {
    for (unsigned trial = 0; trial < kTrial; trial++) { // 42
      Timer timer;
      timer.Start();
      for (unsigned iteration = 0; iteration < kIterations / kRandomCount;
           iteration++)
        for (size_t i = 0; i < kRandomCount; i++)
          it->dtoa(data[i], buffer);
      timer.Stop();
      duration = std::min(duration, timer.GetElapsedNanoseconds());
    }
  } else if (it->dtoa_batch_seq) {
    for (unsigned trial = 0; trial < kTrial; trial++) {
      Timer timer;
      timer.Start();
      for (unsigned iteration = 0; iteration < kIterations / kRandomCount;
           iteration++)
        for (size_t i = 0; i < kRandomCount; i += 32)
          it->dtoa_batch_seq(&data[i], buffer);
      timer.Stop();
      duration = std::min(duration, timer.GetElapsedNanoseconds());
    }
  } else if (it->dtoa_batch_split) {
    char *buf[32];
    for (int i = 0; i < 32; ++i)
      buf[i] = &buffer[i * 32];
    for (unsigned trial = 0; trial < kTrial; trial++) {
      Timer timer;
      timer.Start();
      for (unsigned iteration = 0; iteration < (kIterations / kRandomCount);
           iteration++)
        for (size_t i = 0; i < kRandomCount; i += 32)
          it->dtoa_batch_split(&data[i], buf);
      timer.Stop();
      duration = std::min(duration, timer.GetElapsedNanoseconds());
    }
  }

  duration *= 1.0 / kIterations; // convert to nano second per operation
  it->account(duration);
  //fprintf(fp, "random,%s,%f\n", it->fname, duration);

  printf(" Done\n");
  fflush(nullptr);
}

//------------------------------------------------------------------------------

class RandomDigitData {
public:
  static double *GetData(int digit) {
    assert(digit >= 1 && digit <= kMaxDigits);
    static RandomDigitData singleton;
    return singleton.mData + (digit - 1) * kRandomCount;
  }

private:
  RandomDigitData() : mData(new double[kMaxDigits * kRandomCount]) {
    Random rnd;
    union {
      double d;
      uint64_t u;
    } u;

    double *p = mData;
    for (int digit = 1; digit <= kMaxDigits; digit++) {
      for (size_t i = 0; i < kRandomCount; i++) {
        do
          u.u = rnd();
        while (isnan(u.d) || isinf(u.d));

        // Convert to string with limited digits, and convert it back.
        char buffer[256];
        sprintf(buffer, "%.*g", digit, u.d);
#if 0
        char *end = nullptr;
        const double roundtrip =strtod(buffer, &end);
        const size_t processed = end ? end - buffer : strnlen(buffer, sizeof(buffer));
#else
        using namespace double_conversion;
        StringToDoubleConverter converter(
            StringToDoubleConverter::ALLOW_TRAILING_JUNK, 0.0, 0.0, NULL, NULL);
        int processed = 0;
        double roundtrip = converter.StringToDouble(buffer, 256, &processed);
#endif

        *p++ = roundtrip;
      }
    }
  }

  ~RandomDigitData() { delete[] mData; }

  double *mData;
};

void BenchRandomDigit(Case *it, FILE *fp) {
  printf("Benchmarking randomdigit %s...", it->fname);
  fflush(nullptr);
  //fprintf(fp, "\n%s", it->fname);

  char buffer[1024];
  it->reset();
  static_assert(kIterations % kRandomCount == 0, "Oops");
  static_assert(kRandomCount % 32 == 0, "WTF");

  for (int digit = 1; digit <= kMaxDigits; digit++) {
    double *data = RandomDigitData::GetData(digit);
    double duration = std::numeric_limits<double>::max();

    if (it->dtoa) {
      for (unsigned trial = 0; trial < kTrial; trial++) {
        Timer timer;
        timer.Start();
        for (unsigned iteration = 0; iteration < kIterations / kRandomCount;
             iteration++) {
          for (size_t i = 0; i < kRandomCount; i++) {
            it->dtoa(data[i], buffer);
          }
        }
        timer.Stop();
        duration = std::min(duration, timer.GetElapsedNanoseconds());
      }
    } else if (it->dtoa_batch_seq) {
      for (unsigned trial = 0; trial < kTrial; trial++) {
        Timer timer;
        timer.Start();
        for (unsigned iteration = 0; iteration < kIterations / kRandomCount;
             iteration++) {
          for (size_t i = 0; i < kRandomCount; i += 32) {
            it->dtoa_batch_seq(&data[i], buffer);
          }
        }
        timer.Stop();
        duration = std::min(duration, timer.GetElapsedNanoseconds());
      }
    } else if (it->dtoa_batch_split) {
      char *buf[32];
      for (int i = 0; i < 32; ++i)
        buf[i] = &buffer[i * 32];
      for (unsigned trial = 0; trial < kTrial; trial++) { // 42
        Timer timer;
        timer.Start();
        for (unsigned iteration = 0; iteration < kIterations / kRandomCount;
             iteration++) {
          for (size_t i = 0; i < kRandomCount; i += 32) {
            it->dtoa_batch_split(&data[i], buf);
          }
        }
        timer.Stop();
        duration = std::min(duration, timer.GetElapsedNanoseconds());
      }
    }

    duration *= 1.0 / kIterations; // convert to nano second per operation
    it->account(duration);
    //fprintf(fp, "randomdigit,%s,%d,%f\n", it->fname, digit, duration);
    //fprintf(fp, ",%f", duration);
  }
  printf(" Done\n");
  fflush(nullptr);
}

//------------------------------------------------------------------------------

void Bench(Case *it, FILE *fp) {
  // BenchSequential(it, fp);
  // BenchRandom(it, fp);
  // BenchRandomDigit(it, fp);
}

void BenchAll() {
  // doublery to write to /result path, where template.php exists
  //FILE * fp = fopen("Sequential_" RESULT_FILENAME, "w");
  // if ((fp = fopen("../../result/template.php", "r")) != NULL) {
  //   fclose(fp);
  //   fp = fopen("../../result/" RESULT_FILENAME, "w");
  // } else if ((fp = fopen("../result/template.php", "r")) != NULL) {
  //   fclose(fp);
  //   fp = fopen("../result/" RESULT_FILENAME, "w");
  // } else{
  //   fp = fopen("Sequential_" RESULT_FILENAME, "w");
  // }


  // seq num
  {
    FILE * fp = fopen( "Sequential_" RESULT_FILENAME , "w");
    fputs((const char*)cpuName.c_str(),fp);//print CPU name
    fprintf(fp, "\nType:Sequential \nDigit:1 to 17 \nTime:ns\n\n");
    fprintf(fp, "Function,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17");
    for (auto it : TestManager::Instance().GetTests())
    {
      BenchSequential(it, fp);
    }
    TestManager::Instance().PrintScores(fp,&Case::sum);
    fclose(fp);
  }

  // random
  {
    FILE * fp = fopen("Random_" RESULT_FILENAME, "w");
    fputs((const char*)cpuName.c_str(),fp );
    fprintf(fp, "\nType:Random\n");
    fprintf(fp, "Function,Time(ns)");
    for (auto it : TestManager::Instance().GetTests())
    {
      BenchRandom(it, fp);
    }
    TestManager::Instance().PrintScores(fp,&Case::sum);
    fclose(fp);
  }

  // random digit
  {
    FILE * fp = fopen("RandomDigit_" RESULT_FILENAME, "w");
    fputs((const char*)cpuName.c_str(),fp );
    fprintf(fp, "\nType:RandomDigit \nDigit:1 to 17 \nTime:ns\n\n");
    fprintf(fp, "Function,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17");
    for (auto it : TestManager::Instance().GetTests())
    {
      BenchRandomDigit(it, fp);
    }
    TestManager::Instance().PrintScores(fp,&Case::sum);
    fclose(fp);
  }




  
}

void TestManager::Sort() {
  std::sort(mTests.begin(), mTests.end(),
            [](const Case *a, const Case *b) { return *a < *b; });
}

void TestManager::PrintScores(FILE*fp,Score score, bool SkipWorseThanBaseline) const {
  double baseline = 0;
  for (const auto it : mTests)
    if (it->baseline) {
      baseline = it->*score;
      break;
    }

  bool single_column = true;
  std::vector<const Case *> vector;
  for (const auto it : mTests) {
    if (SkipWorseThanBaseline && baseline && baseline <= it->*score)
      continue;
    if (it->count > 1)
      single_column = false;
    vector.push_back(it);
  }

  std::sort(
      vector.begin(), vector.end(),
      [score](const Case *a, const Case *b) { return a->*score < b->*score; });

  printf("Function      %s|   Sum ns  | Speedup |\n",
         single_column ? "" : "|  Min ns |  RMS ns  |  Max ns ");
  printf(":-------------%s|----------:|--------:|\n",
         single_column ? "" : "|--------:|---------:|--------:");

  for (const auto it : vector) {
    fprintf(fp,"\n%s",it->fname);
    for(int i = 0; i < it->count;++i){
      fprintf(fp,",%f",it->digit17_score[i]);
    }
    printf("%-14s|", it->fname);
    if (!single_column)
      printf("%8.3f | %9.3f | %8.3f |", it->min, it->rms, it->max);
    printf("%10.3f | ×%-6.3f |\n", it->sum,
           (baseline ? baseline : 1.0) / it->*score);
  }
}
void GetCPUInfo() {
    char cpuString[0x40]={0}; // 存放CPU信息字符串
    // 获取 CPU 名称
    unsigned int *ptr = (unsigned int*)cpuString;
    __cpuid(0x80000002,ptr[0],ptr[1],ptr[2],ptr[3]);
    __cpuid(0x80000003,ptr[4],ptr[5],ptr[6],ptr[7]);
    __cpuid(0x80000004,ptr[8],ptr[9],ptr[10],ptr[11]);
    cpuString[0x40-1] = '\0'; // 确保字符串结束
    std::cout << "CPU Name: " << cpuString << std::endl;
    cpuName = std::string(cpuString);
}
int main() {
  TestManager::Instance().Sort();
  GetCPUInfo();
  VerifyAll();
  BenchAll();
  //TestManager::Instance().PrintScores(&Case::sum);
}
