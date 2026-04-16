

add_library(pcr_proc child_proc_posix.cpp)
add_library(pcr::proc ALIAS pcr_proc)

target_include_directories(pcr_proc PUBLIC ${PCR_ROOT_DIR})

target_compile_features(pcr_proc PUBLIC cxx_std_20)
target_compile_options(pcr_proc PRIVATE -Wall -Wextra -Wpedantic)


