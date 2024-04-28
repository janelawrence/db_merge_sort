#ifndef DRAM_H
#define DRAM_H

#include "Run.h"
#include "Page.h"
#include "Disk.h"
#include "Disk.h"
#include "TournamentTree.h"

#include <vector>

struct DramOutputBuffers
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
    unsigned long long capacity;     // Input buffer Capacity in Bytes

    int buffersUsed; // number of input buffers, each buffer holds one page
    int numOutputBuffers;
    std::vector<Page *> inputBuffers; // vecotr of intput buffers
    std::vector<bool> inputBuffersBitmap;
    // Read one page from LOCAL_INPUT_DIR
    Page *readPage(const char *LOCAL_INPUT_DIR, int pageIdx, int recordSize);

public:
    DramOutputBuffers outputBuffers; // wrapping numOutputBuffers output buffers in a run

    // Constructor
    DRAM(unsigned long long maxCap, int nOutputBuffers);
    ~DRAM();

    Page *getPage(int idx);

    // return false if memory is out of space
    //  Add to input buffer
    bool addPage(Page *page);

    Page *getFirstPage();

    void removeFirstPage();

    bool insertPage(Page *page, int idx);

    bool erasePage(int pageIdx);

    void cleanInvalidPagesinInputBuffer();

    bool delFirstRecordFromBufferK(int k);

    unsigned long long readRecords(const char *LOCAL_INPUT_DIR,
                                   int pageStart, int pageEnd,
                                   int recordSize);

    void clear();

    bool isFull() const;

    // merge data on dram and output runs to Dest
    void mergeFromSelfToDest(Disk *dest,
                             const char *outputTXT, std::vector<Run *> &rTable,
                             int outputRunidx);

    // Getters
    unsigned long long getCapacity() const;
    std::vector<Page *> getInputBuffers() const;
    DramOutputBuffers getOuputBuffers() const;
};

#endif // DRAM_H