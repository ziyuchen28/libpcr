
add_library(pcr_stream
    pipe_stream.cpp
    socket_stream.cpp
)
add_library(pcr::stream ALIAS pcr_stream)

target_include_directories(pcr_stream PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(pcr_stream PUBLIC cxx_std_20)
target_compile_options(pcr_stream PRIVATE -Wall -Wextra -Wpedantic)
