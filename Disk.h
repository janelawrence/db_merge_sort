#ifndef DISK_H
#define DISK_H

#include "Run.h"
#include "Run.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

struct OutputBuffers
{
    int nBuffer;
    unsigned long long maxCap;
    unsigned long long bytesStored;
    std::vector<Run *> runs;

    bool isFull()
    {
        return bytesStored == maxCap;
    }

    bool addRun(Run *run)
    {
        if (run->getBytes() > getCapacity())
        {
            printf("Disk output buffer does Not enough space\n");
            return false;
        }
        runs.push_back(run->clone());

        // Decrease Disk capacity
        bytesStored += run->getBytes();
    }

    bool isEmpty()
    {
        bytesStored == 0;
    }
    void clear()
    {
        std::vector<Run *> newRuns;
        runs.swap(newRuns);
        bytesStored = 0;
    }

    unsigned long long getBytes() const
    {
        return bytesStored;
    }
    unsigned long long getCapacity() const
    {
        return maxCap - bytesStored;
    }
};

class Disk
{
private:
    unsigned long long MAX_CAPACITY; //
    unsigned long long capacity;     // remaining capacity in B
    long latency;                    // Latency in microseconds
    long bandwidth;                  // Bandwidth in MB/s

    int numUnsortedRuns;
    int nOutputBuffer;
    const char *diskType;
    std::vector<Run *> unsortedRuns;
    // runBitmap[i] = true if run is valid
    // runBitmap[i] = false if run is been evicted
    std::vector<bool> runBitmap;

    int numTempRuns;
    std::vector<Run *> temp; // stored intermiate merged runs

public:
    OutputBuffers outputBuffers; // wrapping x output buffers in a run

    // Constructor
    Disk(unsigned long long maxCap, long lat, long bw, const char *dType, int nOutputBuffer);

    // return false if Disk is out of space
    // add run to input buffers aka unsortedRuns
    bool addRun(Run *run);

    bool addRunToTempList(Run *run);

    bool addRunToOutputBuffer(Run *run);

    void moveRunToTempList(int runIdx);

    void moveAllTempToUnsorted();

    bool eraseRun(int runIdx);

    bool delFirstPageFromRunK(int k);

    void mergeFromSelfToSelf(const char *outputTXT);

    void clear();

    int outputSpillState(const char *outputTXT);
    int outputReadSortedRunState(const char *outputTXT);
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

    // Getters
    unsigned long long getCapacity() const;
    unsigned long long getOutputBufferCapacity() const;

    int getNumUnsortedRuns() const; // runs stored in input buffer
    int getNumRunsInOutputBuffer() const;
    int getNumTempRuns() const;

    unsigned long long getMaxCap() const;

    bool runIsValid(int idx) const;
    bool isFull() const;
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
