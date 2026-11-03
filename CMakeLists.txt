cmake_minimum_required(VERSION 3.10)
project(AIC_ImageCompression CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(cpp/include)

# Targets
add_executable(encoder cpp/src/encoder.cpp)
add_executable(decoder cpp/src/decoder.cpp)

# Output binaries directly into cpp/src/ for runtime pickup
set_target_properties(encoder PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src")
set_target_properties(decoder PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cpp/src")
