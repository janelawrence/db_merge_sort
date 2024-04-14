#ifndef DRAM_H
#define DRAM_H

#include "Run.h"
#include "Page.h"
#include "SSD.h"
#include "TreeOfLosers.h"

#include <vector>

struct OutputBuffers
{
    int nBuffer;
    Run *wrapper;

    bool isFull()
    {
        return wrapper->getNumPages() == nBuffer && wrapper->getFirstPage()->isFull() && wrapper->getLastPage()->isFull();
    }

    bool isEmpty()
    {
        return wrapper->isEmpty();
    }
    void clear()
    {
        wrapper->clear();
    }
};

class DRAM
{
private:
    unsigned long long MAX_CAPACITY; // in Bytes
    unsigned long long capacity;     // Capacity in Bytes

    int buffersUsed;
    std::vector<Page *> inputBuffers; // wrapping intput buffers in a run
    std::vector<bool> inputBuffersBitmap;
    // Page *forecastBuffer;        // points to the empty input buffer
    OutputBuffers outputBuffers; // wrapping 2 output buffers in a run

public:
    // Constructor
    DRAM(unsigned long long maxCap);

    // return false if memory is out of space
    bool addPage(Page *page);

    bool insertPage(Page *page, int idx);

    bool erasePage(int pageIdx);

    void forecastFromSSD(int bufferIdx, SSD *ssd);

    void clear();

    bool isFull() const;

    // merge in DRAM and output runs
    void merge(SSD *ssd, int maxTreeSize, const char *outputTXT);

    // Getters
    unsigned long long getCapacity() const;
    std::vector<Page *> getInputBuffers() const;
    OutputBuffers getOuputBuffers() const;
};

#endif // DRAM_H