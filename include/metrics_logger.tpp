#pragma once
#include <sstream>
#include <type_traits>

template <typename T>
void MetricsLogger::addMetric(const std::string& name, T value) {
    std::ostringstream oss;
    if constexpr (std::is_floating_point_v<T>) {
        oss << value;
    } else {
        oss << value;
    }

    {
        std::lock_guard<std::mutex> lock(mtx_);
        latest_values_[name] = oss.str();
    }
    cv_.notify_all();
}
