#include "DRAM.h"
#include <iostream>
#include <chrono>
#include <thread>

#include "Run.h"
#include "defs.h"
#include "TreeOfLosers.h"

#include <vector>

// Constructor
DRAM::DRAM(unsigned long long maxCap) : MAX_CAPACITY(maxCap), capacity(maxCap)
{
    printf("Initialize DRAM\n");
    outputBuffers.nBuffer = 2;
    outputBuffers.wrapper = new Run();
    outputBuffers.wrapper->print();
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

void DRAM::merge(SSD *ssd, int maxTreeSize, const char *outputTXT)
{
    TreeOfLosers tree;
    int i = 0;
    int temp = 0;
    Run *curr = new Run();
    while (buffersUsed > 0)
    {
        if (tree.getSize() < maxTreeSize)
        {
            if (!inputBuffers[i]->isEmpty())
            {
                Record *newRecord = new Record(*(inputBuffers[i]->getFirstRecord()));
                // set page idx in record
                newRecord->setSlot(i);

                tree.insert(newRecord);
                inputBuffers[i]->removeFisrtRecord();
                if (inputBuffers[i]->isEmpty())
                {
                    erasePage(i);
                    forecastFromSSD(i, ssd);
                }
            }
            i++;
            i = i % inputBuffers.size();
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
                ssd->outputSpillState(outputTXT);
                // Simulate write to SSD
                ssd->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);

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
                    inputBuffers[j]->removeFisrtRecord();
                    if (inputBuffers[j]->isEmpty())
                    {
                        erasePage(j);
                        forecastFromSSD(j, ssd);
                    }
                }
            }
        }
        i++;
        i = i % inputBuffers.size();
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
            ssd->outputSpillState(outputTXT);
            // Simulate write to SSD
            ssd->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);

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
        ssd->outputSpillState(outputTXT);
        // Simulate write to SSD
        ssd->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);

        while (!outputBuffers.wrapper->isEmpty())
        {
            curr->appendPage(outputBuffers.wrapper->getFirstPage()->clone());
            outputBuffers.wrapper->removeFisrtPage();
        }
        outputBuffers.clear();
    }
    ssd->addRun(curr);
}

void DRAM::forecastFromSSD(int bufferIdx, SSD *ssd)
{
    // forecast, prefectch next page
    int runIdxOnSSD = inputBuffers[bufferIdx]->getIdx();
    Run *runOnSSD = ssd->getRun(runIdxOnSSD);
    if (runOnSSD != nullptr)
    {
        Page *pageFetched = runOnSSD->getFirstPage()->clone();
        pageFetched->setSource(FROM_SSD);
        pageFetched->setIdx(runIdxOnSSD);
        // add copy of first page in run to DRAM buffer
        insertPage(pageFetched, bufferIdx);

        runOnSSD->removeFisrtPage();
        if (runOnSSD->isEmpty())
        {
            ssd->eraseRun(runIdxOnSSD);
        }
    }
}

unsigned long long
DRAM::getCapacity() const
{
    return capacity;
}

OutputBuffers DRAM::getOuputBuffers() const { return outputBuffers; }
