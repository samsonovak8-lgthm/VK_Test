cmake_minimum_required(VERSION 3.14)
project(metrics_logger)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_subdirectory(src)

add_executable(metrics_demo main.cpp)
target_link_libraries(metrics_demo metrics_logger pthread)

enable_testing()
add_subdirectory(tests)
