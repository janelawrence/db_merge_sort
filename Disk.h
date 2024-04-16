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

    int numUnsortedRuns;
    const char *diskType;
    std::vector<Run *> unsortedRuns;
    // runBitmap[i] = true if run is valid
    // runBitmap[i] = false if run is been evicted
    std::vector<bool> runBitmap;

    int numTempRuns;
    std::vector<Run *> temp; // stored intermiate merged runs

public:
    // Constructor
    Disk(unsigned long long maxCap, long lat, long bw, const char *dType);

    // return false if Disk is out of space
    // add to unsortedRuns
    bool addRun(Run *run);

    bool addRunToTempList(Run *run);

    void moveRunToTempList(int runIdx);

    void moveAllTempToUnsorted();

    bool eraseRun(int runIdx);

    bool delFirstPageFromRunK(int k);

    void clear();

    int outputSpillState(const char *outputTXT);
    int outputAccessState(const char *accessType,
                          unsigned long long totalBytes,
                          const char *outputTXT);
    int outputMergeMsg(const char *outputTXT);

    // Smilate Reading data from Disk with given size in bytes
    int readData(unsigned long long sizeInBytes);

    // Smilate Writing runs to Disk
    int writeData(unsigned long long sizeInBytes);

    int writeOutputTable(const char *outputTXT);

    // Clean std::<vecotr> runs physically based on bitmap
    void cleanInvalidRuns();

    bool isFull() const;
    // Getters
    unsigned long long getCapacity() const;
    int getNumUnsortedRuns() const;
    int getNumTempRuns() const;

    unsigned long long getMaxCap() const;

    bool runIsValid(int idx) const;

    long getLatency() const;
    long getBandwidth() const;
    const char *getType() const;

    Run *getRun(int k) const;
    Run *getRunCopy(int k) const;

    void print() const;

    void printTemp() const;

    // Setter
    void setMaxCap(unsigned long long newCap);
    void setCapacity(unsigned long long newCap);
};

#endif // DISK_H
