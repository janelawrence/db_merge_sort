#ifndef HDD_H
#define HDD_H

// #include <iostream>
#include <chrono>
#include <thread>
#include "Record.h"

class HDD {
    private:
        std::string directory; // Directory where files are stored
        double latency;   // Latency in milliseconds
        double bandwidth; // Bandwidth in MB/s

    public:
        // Constructor
        HDD(const std::string& dirPrefix, double lat, double bw);

        // Read one record from HDD with given size in bytes
        Record * readData(const std::string& filename, int recordSize);

        // Read all records in HDD
        std::vector<Record*> readFilesInHDD(int recordSize);

        // Write data to HDD with given size in bytes
        void writeData(const std::string& filename, Record* record);

        // Create directory as a HDD
        std::string createHDD(const std::string& prefix);

        // Get name hdd with date and time as suffix
        static std::string getHDDNameWithCurrentTime(const std::string& prefix);

        // Getters
        std::string getDir() const;
        double getLatency() const;
        double getBandwidth() const;
};

#endif // HDD_H
