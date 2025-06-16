#include "../include/metrics_logger.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <thread>

using namespace std::chrono_literals;

TEST(MetricsLoggerTest, WriteAndFlushMetrics) {
    const std::string test_file = "test_output.txt";
    {
        MetricsLogger logger(test_file, std::chrono::milliseconds(500));
        logger.addMetric("CPU", 1.23);
        logger.addMetric("HTTP RPS", 99);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::ifstream file(test_file);
    ASSERT_TRUE(file.is_open());

    std::string line;
    std::getline(file, line);
    EXPECT_TRUE(line.find("\"CPU\"") != std::string::npos);
    EXPECT_TRUE(line.find("\"HTTP RPS\"") != std::string::npos);

    file.close();
    std::filesystem::remove(test_file);
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

TEST(MetricsLoggerTest, SingleMetricFlush) {
    const std::string file = "test_output_1.txt";
    {
        MetricsLogger logger(file, 500ms);
        logger.addMetric("CPU", 1.23);
        std::this_thread::sleep_for(1s);
    }
    auto content = readFile(file);
    EXPECT_NE(content.find("\"CPU\" 1.23"), std::string::npos);
    std::filesystem::remove(file);
}

TEST(MetricsLoggerTest, MultipleMetricsInSameFlush) {
    const std::string file = "test_output_2.txt";
    {
        MetricsLogger logger(file, 500ms);
        logger.addMetric("CPU", 1.23);
        logger.addMetric("HTTP RPS", 42);
        std::this_thread::sleep_for(1s);
    }
    auto content = readFile(file);
    EXPECT_NE(content.find("\"CPU\" 1.23"), std::string::npos);
    EXPECT_NE(content.find("\"HTTP RPS\" 42"), std::string::npos);
    std::filesystem::remove(file);
}

TEST(MetricsLoggerTest, OverwriteSameMetricBeforeFlush) {
    const std::string file = "test_output_3.txt";
    {
        MetricsLogger logger(file, 500ms);
        logger.addMetric("CPU", 1.0);
        std::this_thread::sleep_for(100ms);
        logger.addMetric("CPU", 2.0);
        std::this_thread::sleep_for(1s);
    }
    auto content = readFile(file);
    EXPECT_EQ(content.find("1.0"), std::string::npos);
    EXPECT_EQ(content.find("2.0"), std::string::npos);
    std::filesystem::remove(file);
}

TEST(MetricsLoggerTest, ThreadedAccessDoesNotCrash) {
    const std::string file = "test_output_4.txt";
    {
        MetricsLogger logger(file, 300ms);
        std::vector<std::thread> threads;

        for (int i = 0; i < 5; ++i) {
            threads.emplace_back([&logger, i]() {
                for (int j = 0; j < 10; ++j) {
                    logger.addMetric("Thread" + std::to_string(i), j);
                    std::this_thread::sleep_for(50ms);
                }
            });
        }

        for (auto& t : threads) t.join();
        std::this_thread::sleep_for(1s);
    }

    auto content = readFile(file);
    for (int i = 0; i < 5; ++i) {
        EXPECT_NE(content.find("Thread" + std::to_string(i)), std::string::npos);
    }
    std::filesystem::remove(file);
}

TEST(MetricsLoggerTest, EmptyLoggerDoesNotCrash) {
    const std::string file = "test_output_5.txt";
    {
        MetricsLogger logger(file, 300ms);
        std::this_thread::sleep_for(600ms);
    }
    std::ifstream f(file);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    EXPECT_TRUE(content.empty());
    std::filesystem::remove(file);
}

TEST(MetricsLoggerTest, SpecialCharactersInMetricName) {
    const std::string file = "test_output_6.txt";
    {
        MetricsLogger logger(file, 500ms);
        logger.addMetric("CPU[core:0]", 0.95);
        logger.addMetric("disk/io", 123);
        std::this_thread::sleep_for(1s);
    }
    auto content = readFile(file);
    EXPECT_NE(content.find("\"CPU[core:0]\" 0.95"), std::string::npos);
    EXPECT_NE(content.find("\"disk/io\" 123"), std::string::npos);
    std::filesystem::remove(file);
}
