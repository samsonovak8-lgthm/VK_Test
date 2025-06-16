#include "metrics_logger.hpp"
#include <iomanip>
#include <ctime>
#include <sstream>
#include <type_traits>

MetricsLogger::MetricsLogger(const std::string& filename, std::chrono::milliseconds interval)
    : out_file_(filename, std::ios::out | std::ios::app), flush_interval_(interval), stop_flag_(false)
{
    worker_ = std::thread([this] { this->workerLoop(); });
}

MetricsLogger::~MetricsLogger() {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        stop_flag_ = true;
    }
    cv_.notify_all();
    if (worker_.joinable()) {
        worker_.join();
    }
}

std::string MetricsLogger::currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto tt = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%dT%H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

void MetricsLogger::workerLoop() {
    std::unique_lock<std::mutex> lock(mtx_);
    while (!stop_flag_) {
        cv_.wait_for(lock, flush_interval_);

        if (!latest_values_.empty()) {
            std::string timestamp = currentTimestamp();
            out_file_ << timestamp;
            for (const auto& [name, value] : latest_values_) {
                out_file_ << " \"" << name << "\" " << value;
            }
            out_file_ << std::endl;
            out_file_.flush();
            latest_values_.clear();
        }
    }

    if (!latest_values_.empty()) {
        std::string timestamp = currentTimestamp();
        out_file_ << timestamp;
        for (const auto& [name, value] : latest_values_) {
            out_file_ << " \"" << name << "\" " << value;
        }
        out_file_ << std::endl;
        out_file_.flush();
        latest_values_.clear();
    }
}
