#ifndef DRAM_H
#define DRAM_H

#include "Run.h"
#include "TreeOfLosers.h"

#include <vector>


class DRAM {
    private: 
        double MAX_CAPACITY = 120;  // DUMMY Value for demo: Capacity is 200 B
        // double MAX_CAPACITY = 100 * 1024 * 1024;  // Capacity is 100 MB
        double capacity = MAX_CAPACITY;  // Capacity in B
        TreeOfLosers tree;

    public:
        // Constructor
        DRAM();

        // merge cache-size runs in DRAM and output DRAM-size runs
        // std::vector<Run*> merge(std::vector<TreeOfLosers*> cacheSizedRuns, int recordSize);

        // Getters
        double getCapacity() const;
};



#endif //DRAM_H