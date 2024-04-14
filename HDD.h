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
    long latency;   // Latency in microseconds
    long bandwidth; // Bandwidth in MB/s
    int numRuns;
    std::vector<Run *> runs;
    std::vector<bool> runBitmap;

public:
    // Constructor
    HDD(double lat, double bw);

    void addRun(Run *run);

    bool eraseRun(int runIdx);

    int outputSpillState(const char *outputTXT);
    int outputAccessState(const char *accessType,
                          unsigned long long totalBytes,
                          const char *outputTXT);
    int outputMergeMsg(const char *outputTXT);

    // Smilate Reading data from SSD with given size in bytes
    void readData(unsigned long long sizeInBytes);

    // Smilate Writing runs to SSD
    void writeData(unsigned long long sizeInBytes);

    void cleanInvalidRuns();

    // Getters
    int getNumRuns() const;
    bool runIsValid(int idx) const;

    long getLatency() const;
    long getBandwidth() const;

    Run *getRun(int k) const;
    Run *getRunCopy(int k) const;

    void print() const;
};

#endif // HDD_H
