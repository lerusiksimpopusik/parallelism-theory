#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <thread>
#include <chrono>
#include <cmath>

class MathFunctions {
public:
    template<typename T>
    static T FunSin(T arg, int delay_ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        return std::sin(arg);
    }

    template<typename T>
    static T FunSqrt(T arg, int delay_ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        return std::sqrt(arg);
    }

    template<typename T>
    static T FunPow(T x, T y, int delay_ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        return std::pow(x, y);
    }
};

#endif // FUNCTIONS_H
