#include "DRAM.h"
#include <iostream>
#include <chrono>
#include <thread>

#include "Run.h"
#include "defs.h"
#include "TreeOfLosers.h"

#include <vector>

// Constructor
DRAM::DRAM(unsigned long long maxCap) : MAX_CAPACITY(maxCap)
{
    printf("Initialize DRAM\n");
    outputBuffers.nBuffer = 2;
    outputBuffers.wrapper = new Run();
    capacity = MAX_CAPACITY - 2 * PAGE_SIZE;
}

bool DRAM::addPage(Page *page)
{
    if (page->getBytes() > capacity)
    {
        printf("DRAM does Not enough space when adding\n");
        return false;
    }
    inputBuffers.push_back(page->clone());

    // Decrease SSD capacity
    capacity -= page->getBytes();
    inputBuffersBitmap.push_back(true);
    buffersUsed++;
    return true;
}

bool DRAM::insertPage(Page *page, int idx)
{
    if (page->getBytes() > capacity)
    {
        printf("DRAM does Not enough space when inserting\n");
        return false;
    }
    if (idx >= inputBuffers.size())
    {
        printf("invalid idx");
        return false;
    }
    inputBuffers[idx] = page->clone();

    // Decrease SSD capacity
    capacity -= page->getBytes();
    inputBuffersBitmap[idx] = true;
    buffersUsed++; // assume buffer has been erased
    return true;
}
/*
release DRAM memory
and flip bits
*/
bool DRAM::erasePage(int bufferIdx)
{
    if (bufferIdx >= inputBuffers.size())
    {
        printf("buffer index invalid\n");
        return false;
    }
    // Increase SSD capacity
    capacity += inputBuffers[bufferIdx]->getBytes();
    inputBuffersBitmap[bufferIdx] = false;
    buffersUsed--;
    return true;
}

bool DRAM::delFirstRecordFromBufferK(int k)
{
    if (k >= 0 && k < inputBuffers.size() && !inputBuffers[k]->isEmpty())
    {
        int rSize = inputBuffers[k]->getFirstRecord()->getSize();
        inputBuffers[k]->removeFisrtRecord();
        capacity += rSize;
        return true;
    }
    return false;
}

void DRAM::clear()
{
    capacity = MAX_CAPACITY;
    std::vector<Page *> emptyPages;
    std::vector<bool> emptyBitmap;
    Page *emptyPage;
    OutputBuffers emptyOutputBuffers;
    emptyOutputBuffers.nBuffer = 2;
    emptyOutputBuffers.wrapper = new Run();

    inputBuffers.swap(emptyPages);
    inputBuffersBitmap.swap(emptyBitmap);
    // forecastBuffer = emptyPage;
    buffersUsed = 0;
    outputBuffers.clear();
}

bool DRAM::isFull() const
{
    return capacity == MAX_CAPACITY;
}

void DRAM::mergeFromSrcToDest(Disk *src, Disk *dest, int maxTreeSize, const char *outputTXT)
{
    TreeOfLosers tree;
    int i = 0;
    Run *curr = new Run();
    maxTreeSize = std::min(maxTreeSize, buffersUsed);
    printf("buffersUsed: %d\n", buffersUsed);
    while (buffersUsed > 0)
    {
        i = i % inputBuffers.size();
        if (tree.getSize() < maxTreeSize)
        {
            if (!inputBuffers[i]->isEmpty())
            {
                Record *newRecord = new Record(*(inputBuffers[i]->getFirstRecord()));
                // set page idx in record
                newRecord->setSlot(i);

                tree.insert(newRecord);
                delFirstRecordFromBufferK(i);
                if (inputBuffers[i]->isEmpty())
                {
                    erasePage(i);
                    if (std::strcmp(SSD, src->getType()) == 0)
                    {
                        forecastFromSSD(i, src);
                    }
                    else
                    {
                        forecastFromHDD(i, src);
                    }
                }
            }
            i++;
            continue;
        }
        else
        {
            // start prunning tree and add to output buffer
            Record *winner = new Record(*tree.getMin());
            tree.deleteMin();
            if (outputBuffers.isFull())
            {
                // Report Spilling happen to output
                dest->outputSpillState(outputTXT);
                // Simulate write to SSD
                dest->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);

                while (!outputBuffers.wrapper->isEmpty())
                {
                    curr->appendPage(outputBuffers.wrapper->getFirstPage()->clone());
                    outputBuffers.wrapper->removeFisrtPage();
                }
                outputBuffers.clear();
            }
            // when prune winner, insert new record from winner source buffer
            int j = winner->getSlot();
            // write to output buffer
            winner->setSlot(-1); // reseting inputbuffer idx for winner
            outputBuffers.wrapper->addRecord(winner);
            // printf("\n %d\n", static_cast<int>(inputBuffersBitmap[j]));
            if (inputBuffersBitmap[j] && !inputBuffers[j]->isEmpty())
            {
                Record *record = inputBuffers[j]->getFirstRecord();
                if (record != nullptr)
                {
                    Record *newRecord = new Record(*record);
                    // set page idx in record
                    newRecord->setSlot(j);
                    tree.insert(newRecord);
                    delFirstRecordFromBufferK(j);
                    if (inputBuffers[j]->isEmpty())
                    {
                        erasePage(j);
                        if (std::strcmp(SSD, src->getType()) == 0)
                        {
                            forecastFromSSD(j, src);
                        }
                        else
                        {
                            forecastFromHDD(j, src);
                        }
                    }
                }
            }
        }
        i++;
    }
    // Empty tree to output buffer
    while (!tree.isEmpty())
    {
        // start prunning tree and add to output buffer
        Record *winner = new Record(*tree.getMin());
        tree.deleteMin();
        if (outputBuffers.isFull())
        {
            // Report Spilling happen to output
            dest->outputSpillState(outputTXT);
            // Simulate write to SSD
            dest->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);

            while (!outputBuffers.wrapper->isEmpty())
            {
                curr->appendPage(outputBuffers.wrapper->getFirstPage()->clone());
                outputBuffers.wrapper->removeFisrtPage();
            }
            outputBuffers.clear();
        }
        // write to output buffer
        winner->setSlot(-1); // reseting inputbuffer idx for winner
        outputBuffers.wrapper->addRecord(winner);
    }

    if (!outputBuffers.isEmpty())
    {
        // write all remaining records in output buffers to the run on SSD
        // Report Spilling happen to output
        dest->outputSpillState(outputTXT);
        // Simulate write to SSD
        dest->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);

        while (!outputBuffers.wrapper->isEmpty())
        {
            curr->appendPage(outputBuffers.wrapper->getFirstPage()->clone());
            outputBuffers.wrapper->removeFisrtPage();
        }
        outputBuffers.clear();
    }
    dest->addRun(curr);
}

void DRAM::forecastFromSSD(int bufferIdx, Disk *ssd)
{
    // forecast, prefectch next page
    int runIdxOnSSD = inputBuffers[bufferIdx]->getIdx();
    Run *runOnSSD = ssd->getRun(runIdxOnSSD);
    // if (runOnSSD != nullptr)
    // {
    //     printf("=====================\n");
    //     runOnSSD->getFirstPage()->print();
    //     printf("=====================\n");
    // }
    if (runOnSSD != nullptr && !runOnSSD->isEmpty())
    {
        Page *pageFetched = runOnSSD->getFirstPage()->clone();
        pageFetched->setSource(FROM_SSD);
        pageFetched->setIdx(runIdxOnSSD);
        // add copy of first page in run to DRAM buffer
        insertPage(pageFetched, bufferIdx);

        runOnSSD->removeFisrtPage();
        ssd->setCapacity(ssd->getCapacity() + pageFetched->getBytes());

        if (runOnSSD->isEmpty())
        {
            ssd->eraseRun(runIdxOnSSD);
        }
    }
}

void DRAM::forecastFromHDD(int bufferIdx, Disk *hdd)
{
    // forecast, prefectch next page
    int runIdxOnHDD = inputBuffers[bufferIdx]->getIdx();
    Run *runOnHDD = hdd->getRun(runIdxOnHDD);
    if (runOnHDD != nullptr)
    {
        Page *pageFetched = runOnHDD->getFirstPage()->clone();
        pageFetched->setSource(FROM_HDD);
        pageFetched->setIdx(runIdxOnHDD);
        // add copy of first page in run to DRAM buffer
        insertPage(pageFetched, bufferIdx);

        runOnHDD->removeFisrtPage();
        if (runOnHDD->isEmpty())
        {
            hdd->eraseRun(runIdxOnHDD);
        }
    }
}

unsigned long long
DRAM::getCapacity() const
{
    return capacity;
}

OutputBuffers DRAM::getOuputBuffers() const { return outputBuffers; }
