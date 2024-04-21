#include <iostream>
#include <chrono>
#include <vector>

int main()
{
    const size_t arraySize = 1024 * 1024 * 512; // 512 MB
    std::vector<int> testArray(arraySize, 0);   // Large array to minimize cache effects

    // Access a random element to prevent prefetching
    volatile int sink;
    auto start = std::chrono::high_resolution_clock::now();
    sink = testArray[rand() % arraySize];
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::nano> diff = end - start;
    std::cout << "Memory access latency: " << diff.count() << " nanoseconds" << std::endl;

    return 0;
}