#ifndef DISK_H
#define DISK_H

#include "Run.h"
#include "Run.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

class Disk
{
private:
    unsigned long long MAX_CAPACITY; //
    unsigned long long capacity;     // remaining capacity in B
    long latency;                    // Latency in microseconds
    long bandwidth;                  // Bandwidth in MB/s

    int numRuns;
    const char *diskType;
    std::vector<Run *> runs;

    // runBitmap[i] = true if run is valid
    // runBitmap[i] = false if run is been evicted
    std::vector<bool> runBitmap;
    // std::vector<bool> dramRunBitmap;

public:
    // Constructor
    Disk(unsigned long long maxCap, long lat, long bw, const char *dType);

    // return false if Disk is out of space
    bool addRun(Run *run);

    bool eraseRun(int runIdx);

    void clear();

    int outputSpillState(const char *outputTXT);
    int outputAccessState(const char *accessType,
                          unsigned long long totalBytes,
                          const char *outputTXT);
    int outputMergeMsg(const char *outputTXT);

    // Smilate Reading data from Disk with given size in bytes
    void readData(unsigned long long sizeInBytes);

    // Smilate Writing runs to Disk
    void writeData(unsigned long long sizeInBytes);

    // Clean std::<vecotr> runs physically based on bitmap
    void cleanInvalidRuns();

    // Merge DRAM-Size Runs into Disk-Size Runs
    // std::vector<Run*> merge(std::vector<Run*> runs, int recordSize);
    bool isFull() const;
    // Getters
    unsigned long long getCapacity() const;
    int getNumRuns() const;
    unsigned long long getMaxCap() const;

    // int getNumCacheSizedRuns() const;
    // int getNumDramSizedRuns() const;

    bool runIsValid(int idx) const;

    long getLatency() const;
    long getBandwidth() const;
    const char *getType() const;

    Run *getRun(int k) const;
    Run *getRunCopy(int k) const;

    void print() const;

    // Setter
    void setMaxCap(unsigned long long newCap);
    void setCapacity(unsigned long long newCap);
};

#endif // DISK_H
