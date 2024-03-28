#ifndef DRAM_H
#define DRAM_H


class DRAM {
    private:
        double MAX_CAPACITY = 100 * 1024 * 1024;  // Capacity is 100 MB
        double capacity = MAX_CAPACITY;  // Capacity in B
        double latency;   // Latency in milliseconds
        double bandwidth; // Bandwidth in MB/s

    public:
        // Constructor
        DRAM(double lat, double bw);

        // Read data from SSD with given size in bytes
        void readData(double sizeInBytes);

        // Write data to SSD with given size in bytes
        void writeData(double sizeInBytes);

        // Getters
        double getCapacity() const;
        double getLatency() const;
        double getBandwidth() const;
};



#endif //DRAM_H