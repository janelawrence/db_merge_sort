#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>

int main()
{
    const size_t arraySize = 1024 * 1024 * 512; // 512 MB
    std::vector<int> testArray(arraySize, 1);

    auto start = std::chrono::high_resolution_clock::now();
    // Perform a large number of writes
    std::fill(testArray.begin(), testArray.end(), 10);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::nano> duration = end - start;

    double seconds = duration.count() / 1e9; // Convert nanoseconds to seconds
    double bytesTransferred = arraySize * sizeof(int);
    double bandwidth = bytesTransferred / seconds / (1024 * 1024 * 1024); // GB/ms

    std::cout << "Memory bandwidth: " << bandwidth << " GB/ms" << std::endl;

    return 0;
}
