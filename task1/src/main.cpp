#include <iostream>
#include <cmath>
#include <vector>
#include <typeinfo>


template <typename T>
void fill_and_sum(int size) {
    std::vector<T> data(size);
    T sum = 0.0;
    for (int i = 0; i < size; ++i) {
        data[i] = std::sin(2 * M_PI * i / size);
        sum += data[i];
    }
    std::cout << "Sum " << typeid(sum).name() << " :" << sum << std::endl;
}

int main() {
    const int size = 10000000; 

    #ifdef USE_FLOAT
        fill_and_sum<float>(size);
    #else
        fill_and_sum<double>(size);
    #endif

    return 0;
}
