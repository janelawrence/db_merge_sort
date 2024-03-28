#ifndef SSD_H
#define SSD_H

#include <iostream>
#include <chrono>
#include <thread>

class SSD {
    private:
        double MAX_CAPACITY = 10 * 1024 * 1024 * 1024;  // Capacity is 10 GB
        double capacity = MAX_CAPACITY;  // Capacity in B
        double latency;   // Latency in milliseconds
        double bandwidth; // Bandwidth in MB/s

public:
    // Constructor
    SSD(double lat, double bw);

    // Read data from SSD with given size in bytes
    void readData(double sizeInBytes);

    // Write data to SSD with given size in bytes
    void writeData(double sizeInBytes);

    // Getters
    double getCapacity() const;
    double getLatency() const;
    double getBandwidth() const;
};

#endif // SSD_H
