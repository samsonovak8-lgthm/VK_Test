#include "metrics_logger.hpp"
#include <thread>
#include <vector>
#include <random>
#include <iostream>

void simulateMetric(MetricsLogger& logger, const std::string& name, int count, int delay_ms) {
    std::default_random_engine rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.1, 2.0);

    for (int i = 0; i < count; ++i) {
        if (name == "CPU") {
            logger.addMetric(name, dist(rng));
        } else if (name == "HTTP RPS") {
            logger.addMetric(name, static_cast<int>(dist(rng) * 100));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
}

int main() {
    MetricsLogger logger("metrics.txt");

    std::vector<std::thread> threads;
    threads.emplace_back(simulateMetric, std::ref(logger), "CPU", 10, 150);
    threads.emplace_back(simulateMetric, std::ref(logger), "HTTP RPS", 10, 200);

    for (auto& t : threads) t.join();

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Метрики записаны в файл metrics.txt" << std::endl;
    return 0;
}
