#include <format>
#include <string>
#include "test.h"

char *dtoa_format(double value, char *const buffer) {
  static std::string str ;
  str = std::format("{:.17g}",value);
  return const_cast<char*>(str.c_str());
}

REGISTER_TEST(format);