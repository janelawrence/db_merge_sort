#ifndef CACHE_H
#define CACHE_H
#include "Record.h"
#include "HDD.h"
#include "DRAM.h"
#include "Run.h"
#include "TreeOfLosers.h"


class CACHE {
    private:
        double MAX_CAPACITY = 100;  // DUMMY Value for demo: Capacity is 100 B
        // double MAX_CAPACITY = 1 * 1024 * 1024;  // Capacity is 1 MB
        double capacity = MAX_CAPACITY;
        TreeOfLosers tree;
    public:
        // Constructor
        CACHE();

        std::vector<TreeOfLosers*> readFromHDD(int recordSize, HDD* hdd);

        TreeOfLosers* sort(HDD* hdd, std::vector<Record*> records, int slotIdx);


        // Getters
        double getCapacity() const;
};



#endif //CACHE_H