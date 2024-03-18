#ifndef EMULATEDSSD_H
#define EMULATEDSSD_H

#include <iostream>
#include <chrono>
#include <thread>

class EmulatedSSD {
private:
    double capacity;  // Capacity in GB
    double latency;   // Latency in milliseconds
    double bandwidth; // Bandwidth in MB/s

public:
    // Constructor
    EmulatedSSD(double cap, double lat, double bw);

    // Read data from EmulatedSSD with given size in bytes
    void readData(double sizeInBytes);

    // Write data to EmulatedSSD with given size in bytes
    void writeData(double sizeInBytes);

    // Getters
    double getCapacity() const;
    double getLatency() const;
    double getBandwidth() const;
};

#endif // EMULATEDSSD_H
