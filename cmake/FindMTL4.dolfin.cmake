# - Try to find MTL4
# Once done this will define
#
#  MTL4_FOUND        - system has found MTL4
#  MTL4_INCLUDE_DIR  - the MTL4 include directory

set(MTL4_FOUND 0)

message(STATUS "Checking for package 'MTL4'")

# Check for header file
find_path(MTL4_INCLUDE_DIR boost/numeric/mtl/mtl.hpp
  $ENV{MTL4_DIR}
  /usr/local/include
  /usr/include
  DOC "Directory where the MTL4 header is located"
  )

# Try compiling and running test program
if(MTL4_INCLUDE_DIR)

  # Set flags for building test program
  set(CMAKE_REQUIRED_INCLUDES ${MTL4_INCLUDE_DIR})

  # Build and run test program
  include(CheckCXXSourceRuns)
  check_cxx_source_runs("
#include <iostream>
#include <boost/numeric/mtl/mtl.hpp>
int main()
{
  mtl::dense_vector<double> x(10);
  int size = mtl::num_rows(x);
  return 0;
}
" MTL4_TEST_RUNS)

  if(NOT MTL4_TEST_RUNS)
    message("   Unable to run test program for package 'MTL4'")
  endif(NOT MTL4_TEST_RUNS)

endif(MTL4_INCLUDE_DIR)

# Report results of test
if(MTL4_TEST_RUNS)
  message("   Found package 'MTL4'")
  set(MTL4_FOUND 1)
else(MTL4_TEST_RUNS)
  message("   Unable to configure package 'MTL4'")
endif(MTL4_TEST_RUNS)
