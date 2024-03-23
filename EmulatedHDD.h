#ifndef EMULATEDHDD_H
#define EMULATEDHDD_H

// #include <iostream>
#include <chrono>
#include <thread>
#include "Record.h"

class EmulatedHDD {
private:
    std::string directory; // Directory where files are stored
    double latency;   // Latency in milliseconds
    double bandwidth; // Bandwidth in MB/s

public:
    // Constructor
    EmulatedHDD(const std::string& dir, double lat, double bw);

    // Read data from EmulatedHDD with given size in bytes
    Record * readData(const std::string& filename, int size);

    // Write data to EmulatedHDD with given size in bytes
    void writeData(const std::string& filename, Record* record);

    // Create directory as a HDD
    std::string createHDD();

    // Get name hdd with date and time as suffix
    std::string getHDDNameWithCurrentTime();

    // Getters
    std::string getDir() const;
    double getLatency() const;
    double getBandwidth() const;
};

#endif // EMULATEDHDD_H
