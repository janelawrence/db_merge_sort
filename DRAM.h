#ifndef DRAM_H
#define DRAM_H

#include "Run.h"
#include "Page.h"
#include "Disk.h"
#include "Disk.h"
#include "TournamentTree.h"

#include <vector>

class DRAM
{
private:
    unsigned long long MAX_CAPACITY; // in Bytes
    unsigned long long capacity;     // Input buffer Capacity in Bytes

    int buffersUsed; // number of input buffers, each buffer holds one page
    int numOutputBuffers;
    std::vector<Page *> inputBuffers; // vecotr of intput buffers
    std::vector<bool> inputBuffersBitmap;

public:
    OutputBuffers outputBuffers; // wrapping numOutputBuffers output buffers in a run

    // Constructor
    DRAM(unsigned long long maxCap, int nOutputBuffers);

    // return false if memory is out of space
    //  Add to input buffer
    bool addPage(Page *page);

    bool insertPage(Page *page, int idx);

    bool erasePage(int pageIdx);

    bool delFirstRecordFromBufferK(int k);

    Run *readRecords(const char *fileName, int recordSize, int numRecords, int totalBytes,
                     int nBuffersDRAM, int maxRecordsInPage);

    void clear();

    bool isFull() const;

    // merge data on dram and output runs to Dest
    void mergeFromSelfToDest(Disk *dest, const char *outputTXT, std::vector<Run *> &rTable);

    // Getters
    unsigned long long getCapacity() const;
    std::vector<Page *> getInputBuffers() const;
    OutputBuffers getOuputBuffers() const;
};

#endif // DRAM_H