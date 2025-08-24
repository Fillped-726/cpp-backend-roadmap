#include "../include/split_view.hpp"
#include <vector> 
#include <chrono>
#include <iostream>
#include <string>

constexpr std::size_t kLoop = 1'000'000;
constexpr char kSample[] = "hello,world,string_view,fast,enough";

int main() {
    const std::string input(kSample);

    // ---------- string_view ----------
    auto t0 = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < kLoop; ++i) {
        volatile auto v = split_view(input, ',');
        (void)v;
    }
    auto t1 = std::chrono::steady_clock::now();
    auto us_sv = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    // ---------- std::string::substr ----------
    t0 = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < kLoop; ++i) {
        std::vector<std::string> res;
        std::size_t first = 0;
        while (first < input.size()) {
            auto second = input.find(',', first);
            if (second == std::string::npos) {
                res.emplace_back(input.substr(first));
                break;
            }
            res.emplace_back(input.substr(first, second - first));
            first = second + 1;
        }
        volatile auto v = res;
        (void)v;
    }
    t1 = std::chrono::steady_clock::now();
    auto us_substr = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    std::cout << "string_view : " << us_sv << " µs\n";
    std::cout << "substr      : " << us_substr << " µs\n";
    std::cout << "speedup ≈ " << double(us_substr) / us_sv << "x\n";
}