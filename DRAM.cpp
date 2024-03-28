#include "DRAM.h"
#include <iostream>
#include <chrono>
#include <thread>

// Constructor
DRAM::DRAM(double lat, double bw) : latency(lat), bandwidth(bw) {}

// Method to simulate read operation
void DRAM::readData(double sizeInBytes) {
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    std::cout << "Read " << sizeInBytes << " bytes from DRAM. Transfer time: " << transferTime << " seconds\n";
}

// Method to simulate write operation
void DRAM::writeData(double sizeInBytes) {
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    std::cout << "Wrote " << sizeInBytes << " bytes to DRAM. Transfer time: " << transferTime << " seconds\n";
}

int main() {
    // Create an DRAM object with capacity 10GB, latency 0.1ms, and bandwidth 200MB/s
    DRAM dram(0.1, 200);

    // Simulate read and write operations
    dram.readData(1024); // Reading 1KB
    dram.writeData(1024); // Writing 1KB

    return 0;
}
