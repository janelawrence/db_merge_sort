#include "EmulatedSSD.h"
#include <iostream>
#include <chrono>
#include <thread>

// Constructor
EmulatedSSD::EmulatedSSD(double cap, double lat, double bw) : capacity(cap), latency(lat), bandwidth(bw) {}

// Method to simulate read operation
void EmulatedSSD::readData(double sizeInBytes) {
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    std::cout << "Read " << sizeInBytes << " bytes from EmulatedSSD. Transfer time: " << transferTime << " seconds\n";
}

// Method to simulate write operation
void EmulatedSSD::writeData(double sizeInBytes) {
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    std::cout << "Wrote " << sizeInBytes << " bytes to EmulatedSSD. Transfer time: " << transferTime << " seconds\n";
}

int main() {
    // Create an EmulatedSSD object with capacity 10GB, latency 0.1ms, and bandwidth 200MB/s
    EmulatedSSD ssd(10 * 1024 * 1024 * 1024, 0.1, 200);

    // Simulate read and write operations
    ssd.readData(1024); // Reading 1KB
    ssd.writeData(1024); // Writing 1KB

    return 0;
}
