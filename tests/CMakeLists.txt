include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/release-1.12.1.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(test_logger test_logger.cpp)

target_link_libraries(test_logger
    gtest_main
    metrics_logger     
    pthread
)

target_include_directories(test_logger
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(test_logger)
