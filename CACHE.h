#ifndef CACHE_H
#define CACHE_H
#include "Record.h"
#include "Disk.h"
#include "DRAM.h"
#include "Run.h"
#include "HeapSort.h"

class CACHE
{
private:
    double MAX_CAPACITY; // DUMMY Value for demo: Capacity is 100 B
    // double MAX_CAPACITY = 1 * 1024 * 1024;  // Capacity is 1 MB
    double capacity = MAX_CAPACITY;
    int nPagesFitInCache;
    HeapSort heap;

public:
    // Constructor
    CACHE(int cacheSize, int nPages);

    int outputMiniRunState(const char *outputTXT);
    // Output miniRuns
    std::vector<Run *> sort(std::vector<Page *>, int maxRecordsInPage, int PAGE_SIZE);

    // Getters
    double getCapacity() const;
};

#endif // CACHE_H
