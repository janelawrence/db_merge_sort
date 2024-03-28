#ifndef CACHE_H
#define CACHE_H
#include "Record.h"
#include "HDD.h"
#include "DRAM.h"
#include "TreeOfLosers.h"


class CACHE {
    private:
        double MAX_CAPACITY = 1 * 1024 * 1024;  // Capacity is 1 MB
        double capacity = MAX_CAPACITY;
        TreeOfLosers tree;
    public:
        // Constructor
        CACHE();

        std::vector<Run*> readFromHDD(int totalSize,int recordSize, HDD* hdd, DRAM* dram);

        Run* sort(HDD* hdd, DRAM* dram, std::vector<Record*> records, int slotIdx);

        void writeToDRAM(DRAM* dram);

        // Getters
        double getCapacity() const;
};



#endif //CACHE_H