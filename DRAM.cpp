#include "DRAM.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>

#include "Run.h"
#include "defs.h"

// Constructor
DRAM::DRAM(unsigned long long maxCap, int nOutputBuffers)
    : MAX_CAPACITY(maxCap), numOutputBuffers(nOutputBuffers)
{
    printf("Initialize DRAM\n");
    outputBuffers.nBuffer = nOutputBuffers;
    buffersUsed = 0;
    outputBuffers.wrapper = new Run();
    // Input buffer Capacity in Bytes
    capacity = MAX_CAPACITY - (unsigned long long)nOutputBuffers * PAGE_SIZE;
}

Page *DRAM::getPageCopy(int idx)
{
    if (idx >= 0 && idx < inputBuffers.size())
    {
        return inputBuffers[idx]->clone();
    }
    printf("Invalid page index");
    return nullptr;
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

/*
    return deep copy
*/
Page *DRAM::getFirstPage()
{
    if (inputBuffers.size() > 0)
    {
        Page *firstPage = inputBuffers[0]->clone();
        firstPage->setNext(nullptr);
        return firstPage;
    }
    return nullptr;
}

void DRAM::removeFirstPage()
{
    if (inputBuffers.size() > 0)
    {
        Page *firstPage = inputBuffers[0];
        capacity += firstPage->getBytes();
        buffersUsed -= 1;
        inputBuffers.erase(inputBuffers.begin());
        erasePage(0);
    }
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

void DRAM::cleanInvalidPagesinInputBuffer()
{
    std::vector<Page *> cleanedPages;
    std::vector<bool> cleanedBitmap;
    unsigned long long used = 0;
    for (int i = 0; i < inputBuffers.size(); i++)
    {
        if (inputBuffersBitmap[i])
        {
            cleanedPages.push_back(inputBuffers[i]->clone());
            cleanedBitmap.push_back(true);
            used += inputBuffers[i]->getBytes();
        }
    }
    buffersUsed = cleanedPages.size();
    capacity = MAX_CAPACITY - used;
    inputBuffers.swap(cleanedPages);
    inputBuffersBitmap.swap(cleanedBitmap);
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

/* Function to read records from the input file
    Return: a run * of pages, this run has size same as DRAM size
*/
Run *DRAM::readRecords(const char *fileName, int recordSize, int numRecords, int totalBytes,
                       int nBuffersDRAM, int maxRecordsInPage)
{
    std::ifstream file(fileName);
    Run *allPages = new Run();
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            // printf("%d\n%s\n\n", strlen(line.c_str()), line.c_str());
            Record *r = new Record(recordSize, line.c_str());
            allPages->addRecord(r);
        }
        file.close();
    }
    return allPages;
}

void DRAM::clear()
{
    capacity = MAX_CAPACITY;
    std::vector<Page *> emptyPages;
    std::vector<bool> emptyBitmap;
    Page *emptyPage;
    DramOutputBuffers emptyOutputBuffers;
    emptyOutputBuffers.nBuffer = numOutputBuffers;
    emptyOutputBuffers.wrapper = new Run();

    inputBuffers.swap(emptyPages);
    inputBuffersBitmap.swap(emptyBitmap);
    // forecastBuffer = emptyPage;
    buffersUsed = 0;
    outputBuffers.clear();
    capacity = MAX_CAPACITY - numOutputBuffers * PAGE_SIZE;
}

bool DRAM::isFull() const
{
    return capacity == MAX_CAPACITY;
}

void DRAM::mergeFromSelfToDest(Disk *dest, const char *outputTXT, std::vector<Run *> &rTable)
{
    // Setup of total output buffers size makes sure that
    // output buffers can hold the maximum number of records,
    // where it's equal to the maximum number of cachesized runs;
    int fanin = rTable.size();
    TournamentTree *tree = new TournamentTree(fanin, rTable, nullptr);
    int i = 0;
    Run *curr = new Run();
    printf("buffersUsed: %d\n", buffersUsed);

    while (tree->hasNext())
    {
        Record *winner = tree->popWinner();

        // if output buffer is full
        if (outputBuffers.isFull())
        {
            // Report Spilling happen to trace.txt
            dest->outputSpillState(outputTXT);
            // Simulate write to SSD
            dest->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);

            while (!outputBuffers.isEmpty())
            {
                // Write to curr run in dest Disk
                curr->appendPage(outputBuffers.wrapper->getFirstPage()->clone());
                outputBuffers.wrapper->removeFisrtPage();
            }
            outputBuffers.clear();
        }
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
    // Physically add to destination disk's output buffer
    dest->addRunToOutputBuffer(curr);
}

unsigned long long
DRAM::getCapacity() const
{
    return capacity;
}

std::vector<Page *> DRAM::getInputBuffers() const { return inputBuffers; };

DramOutputBuffers DRAM::getOuputBuffers() const { return outputBuffers; }
