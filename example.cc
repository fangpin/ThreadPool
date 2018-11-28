#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include "ThreadPool.hpp"

void test(int val) {
    std::cout << val << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main() {
    std::vector<int> vec(10);
    {
        Fang::ThreadPool pool(4);

        // example: 1) std::function. 2) no return value.
        for (int i=0; i<10; ++i) {
            pool.insert_task(test, i);
        }

        // example: 1) lambda. 2) return by reference.
        for (int i=0; i<10; ++i) {
            pool.insert_task([&vec, i](){
                vec[i] = i * 10000;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                });
        }
    }
    for (auto x: vec) {
        std::cout << x << std::endl;
    }
    return 0;
}
