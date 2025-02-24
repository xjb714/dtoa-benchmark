#include <charconv>
#include "test.h"

char *dtoa_to_chars(double value, char *const buffer) {
  //stbsp_sprintf(buffer, "%.17g", value);
  std::to_chars(buffer,buffer+32,value,std::chars_format::scientific,17);
  return buffer;
}

REGISTER_TEST(to_chars);