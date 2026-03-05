cmake_minimum_required(VERSION 3.24)
project(prism VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(PROJECT_IS_TOP_LEVEL)
  option(PRISM_BUILD_TESTS "Build prism tests" ON)
else()
  option(PRISM_BUILD_TESTS "Build prism tests" OFF)
endif()

add_subdirectory(libs/proc)

# Future libs:
# add_subdirectory(libs/metrics)
# add_subdirectory(libs/channel)
# add_subdirectory(libs/framing)
# add_subdirectory(libs/rpc)

if(PRISM_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
