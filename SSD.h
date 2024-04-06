#ifndef SSD_H
#define SSD_H

#include "Run.h"
#include "TreeOfLosers.h"

#include <iostream>
#include <chrono>
#include <thread>

class SSD {
    private:
        double MAX_CAPACITY = 200;  // DUMMY Value for demo: Capacity is 200 B
        // double MAX_CAPACITY = 10 * 1024 * 1024 * 1024;  // Capacity is 10 GB
        double capacity = MAX_CAPACITY;  // Capacity in B
        double latency;   // Latency in milliseconds
        double bandwidth; // Bandwidth in MB/s
        TreeOfLosers tree;
public:
    // Constructor
    SSD(double lat, double bw);

    // Smilate Reading data from SSD with given size in bytes
    void readData(double sizeInBytes);

    // Smilate Writing DRAM-Size runs to SSD 
    void writeData(double sizeInBytes);
    
    // Merge DRAM-Size Runs into SSD-Size Runs
    // std::vector<Run*> merge(std::vector<Run*> runs, int recordSize);

    // Getters
    double getCapacity() const;
    double getLatency() const;
    double getBandwidth() const;
};

#endif // SSD_H
