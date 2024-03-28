#include "SSD.h"
#include <iostream>
#include <chrono>
#include <thread>

// Constructor
SSD::SSD(double lat, double bw) : latency(lat), bandwidth(bw) {}

// Method to simulate read operation
void SSD::readData(double sizeInBytes) {
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    std::cout << "Read " << sizeInBytes << " bytes from SSD. Transfer time: " << transferTime << " seconds\n";
}

// Method to simulate write operation
void SSD::writeData(double sizeInBytes) {
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    std::cout << "Wrote " << sizeInBytes << " bytes to SSD. Transfer time: " << transferTime << " seconds\n";
}

int main() {
    // Create an SSD object with capacity 10GB, latency 0.1ms, and bandwidth 200MB/s
    SSD ssd(0.1, 200);

    // Simulate read and write operations
    ssd.readData(1024); // Reading 1KB
    ssd.writeData(1024); // Writing 1KB

    return 0;
}
