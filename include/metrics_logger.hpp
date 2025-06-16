#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <chrono>
#include <thread>

class MetricsLogger {
public:
    MetricsLogger(const std::string& filename,
                  std::chrono::milliseconds interval = std::chrono::milliseconds(1000));
    ~MetricsLogger();

    template <typename T>
    void addMetric(const std::string& name, T value);

private:
    std::ofstream out_file_;
    std::chrono::milliseconds flush_interval_;
    std::unordered_map<std::string, std::string> latest_values_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::thread worker_;
    bool stop_flag_;

    std::string currentTimestamp();
    void workerLoop();
};


#include "metrics_logger.tpp"
