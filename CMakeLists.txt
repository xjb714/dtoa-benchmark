﻿cmake_minimum_required(VERSION 3.8.2)
cmake_policy(SET CMP0054 NEW)
if(POLICY CMP0075)
  cmake_policy(SET CMP0075 NEW)
endif()
if(POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)

project(d2a-benchmark C CXX)

# set(CMAKE_CXX_COMPILER g++-14)
# set(CMAKE_CXX_COMPILER clang++)
# set(CMAKE_CXX_COMPILER icpx)

add_compile_options(-O3 -mavx512f -march=native -funroll-loops)

#
# Set default build type to Release. This is to ease a User's life.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
    FORCE)
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPERCASE)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(SOURCES
  src/stb_test.cpp src/stb/stb_sprintf.h

  src/double-conversion/bignum.cc
  src/double-conversion/bignum-dtoa.cc
  src/double-conversion/bignum-dtoa.h
  src/double-conversion/bignum.h
  src/double-conversion/cached-powers.cc
  src/double-conversion/cached-powers.h
  src/double-conversion/diy-fp.h
  src/double-conversion/double-conversion.h
  src/double-conversion/double-to-string.cc
  src/double-conversion/double-to-string.h
  src/double-conversion/fast-dtoa.cc
  src/double-conversion/fast-dtoa.h
  src/double-conversion/fixed-dtoa.cc
  src/double-conversion/fixed-dtoa.h
  src/double-conversion/ieee.h
  src/double-conversion/string-to-double.cc
  src/double-conversion/string-to-double.h
  src/double-conversion/strtod.cc
  src/double-conversion/strtod.h
  src/double-conversion/utils.h
  src/doubleconvtest.cpp

  src/emyg/emyg_dtoa.c
  src/emyg/emyg_dtoa.h
  src/emygtest.cpp

  #src/d2sci/d2sci.cpp
  src/d2sci/d2sci.h
  src/d2sci_test.cpp

  src/erthink/erthink.h
  src/erthink/erthink_arch.h
  src/erthink/erthink_bswap.h
  src/erthink/erthink_byteorder.h
  src/erthink/erthink_carryadd.h
  src/erthink/erthink_clz.h++
  src/erthink/erthink_d2a.h++
  src/erthink/erthink_defs.h
  src/erthink/erthink_intrin.h
  src/erthink/erthink_misc.h++
  src/erthink/erthink_mul.h
  src/erthink/erthink_rot.h
  src/erthink/erthink_u2a.h++
  src/erthink_test.cpp

  src/ryu/common.h
  src/ryu/d2fixed.c
  src/ryu/d2fixed_full_table.h
  src/ryu/d2s.c
  src/ryu/d2s_full_table.h
  src/ryu/d2s_intrinsics.h
  src/ryu/d2s_small_table.h
  src/ryu/digit_table.h
  src/ryu/f2s.c
  src/ryu/f2s_full_table.h
  src/ryu/generic_128.c
  src/ryu/generic_128.h
  src/ryu/ryu_generic_128.h
  src/ryu/ryu.h
  src/ryu/ryu_parse.h
  src/ryu/s2d.c
  src/ryu_test.cpp

  src/floaxie/atof.h
  src/floaxie/bit_ops.h
  src/floaxie/cached_power.h
  src/floaxie/conversion_status.h
  src/floaxie/default_fallback.h
  src/floaxie/diy_fp.h
  src/floaxie/fraction.h
  src/floaxie/ftoa.h
  src/floaxie/grisu.h
  src/floaxie/huge_val.h
  src/floaxie/integer_of_size.h
  src/floaxie/k_comp.h
  src/floaxie/krosh.h
  src/floaxie/memwrap.h
  src/floaxie/powers_ten.h
  src/floaxie/powers_ten_double.h
  src/floaxie/powers_ten_single.h
  src/floaxie/prettify.h
  src/floaxie/print.h
  src/floaxie/static_pow.h
  src/floaxie/type_punning_cast.h
  src/floaxietest.cpp

  src/fmt/chrono.h
  src/fmt/color.h
  src/fmt/compile.h
  src/fmt/core.h
  src/fmt/format.h
  src/fmt/format-inl.h
  src/fmt/locale.h
  src/fmt/os.h
  src/fmt/ostream.h
  src/fmt/posix.h
  src/fmt/printf.h
  src/fmt/ranges.h
  src/fmt/format.cc
  src/fmt/os.cc
  src/fmttest.cpp

  src/fpconv/fpconv.c
  src/fpconv/fpconv.h
  src/fpconv/license
  src/fpconv/powers.h
  src/fpconvtest.cpp

  src/gay/dtoa.c
  src/gay/g_fmt.c
  src/gaytest.cpp

  src/grisu/diy_fp.h
  src/grisu/double.h
  src/grisu/fast_exponent.h
  src/grisu/grisu2.h
  src/grisu/grisu2b_59_56.c
  src/grisu/k_comp.h
  src/grisu/powers.h
  src/grisu/powers_ten_round64.h
  src/grisu/prettify.h
  src/grisu2btest.cpp

  src/Grisu_Exact/fp_to_chars.h
  src/Grisu_Exact/fp_to_chars.cpp
  src/Grisu_Exact/grisu_exact.h
  src/grisu_exact_test.cpp

  src/dragonbox/dragonbox.h
  src/dragonbox/dragonbox.cc
  src/dragonbox_test.cpp

  src/schubfach/schubfach_64.cc
  src/schubfach_test.cpp
  
  src/dragon4/Dragon4.cpp
  src/dragon4/PrintFloat.cpp
  src/dragon4/Math.cpp
  src/dragon4_test.cpp

  # src/errol/errol.c
  # src/errol_test.cpp

  src/std_format_test.cpp

  src/milo/dtoa_milo.h src/milotest.cpp
  src/msinttypes/inttypes.h src/msinttypes/stdint.h

  src/ostringstream.cpp
  src/ostrstream.cpp
  src/sprintf.cpp
  src/to_chars_test.cpp

  src/resultfilename.h
  src/test.h
  src/timer.h
  src/null.cpp
  src/main.cpp
  )

add_executable(d2a-benchmark ${SOURCES})

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_definitions(d2a-benchmark PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()



if(NOT DEFINED CXX_STANDARD)
  list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_17 HAS_CXX17)
  list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_14 HAS_CXX14)
  list(FIND CMAKE_CXX_COMPILE_FEATURES cxx_std_11 HAS_CXX11)

  if(NOT HAS_CXX17 LESS 0)
    set(CXX_STANDARD 17)
  elseif(NOT HAS_CXX14 LESS 0)
    set(CXX_STANDARD 14)
  elseif(NOT HAS_CXX11 LESS 0)
    set(CXX_STANDARD 11)
  endif()
endif()
if(DEFINED CXX_STANDARD)
  message(STATUS "Use C++${CXX_STANDARD}")
  target_compile_features(d2a-benchmark PRIVATE cxx_std_11)
endif()

target_link_directories(d2a-benchmark PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}//src//d2sci")
target_link_libraries(d2a-benchmark d2sci2)