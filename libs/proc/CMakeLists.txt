

add_library(prism_proc src/child_proc_posix.cpp)
add_library(prism::proc ALIAS prism_proc)

target_include_directories(prism_proc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(prism_proc PUBLIC cxx_std_20)
target_compile_options(prism_proc PRIVATE -Wall -Wextra -Wpedantic)
