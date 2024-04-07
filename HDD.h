#ifndef HDD_H
#define HDD_H

// #include <iostream>
#include <chrono>
#include <thread>
#include "Record.h"
#include "Run.h"
#include "TreeOfLosers.h"
#include "Page.h"



class HDD {
    private:
        double latency;   // Latency in milliseconds
        double bandwidth; // Bandwidth in MB/s
        TreeOfLosers tree;
        std::vector<Page *> pages;
        
    public:
        // Constructor
        HDD(double lat, double bw);


        // Simulate writing time
        void simulateWriteData(double sizeInBytes);


        // Write data to HDD with given size in bytes
        void writeData(const std::string& filename, Record* record);


        // Merge SSD-Size Runs
        // std::vector<Run*> merge(std::vector<Run*> runs, int recordSize);

        int outputAccessState(const char * accessType,
                            unsigned long long totalBytes,
                            const char * outputTXT);

        // Getters
        double getLatency() const;
        double getBandwidth() const;
};

#endif // HDD_H
