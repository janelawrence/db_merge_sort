#ifndef HDD_H
#define HDD_H

// #include <iostream>
#include <chrono>
#include <thread>
#include "Record.h"
#include "Run.h"
#include "Page.h"

class HDD
{
private:
    double latency;   // Latency in microseconds
    double bandwidth; // Bandwidth in MB/s
    std::vector<Page *> pages;
    std::vector<Run *> runs;

public:
    // Constructor
    HDD(double lat, double bw);

    // Simulate writing time
    void simulateWriteData(double sizeInBytes);

    // Write data to HDD with given size in bytes
    void writeData(unsigned long long sizeInBytes);

    int outputAccessState(const char *accessType,
                          unsigned long long totalBytes,
                          const char *outputTXT);

    // Getters
    double getLatency() const;
    double getBandwidth() const;

    void print() const;
};

#endif // HDD_H
