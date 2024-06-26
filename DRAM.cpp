#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <cstring>
#include <unordered_map>

#include "DRAM.h"
#include "Run.h"
#include "defs.h"

// Constructor
DRAM::DRAM(unsigned long long maxCap, int nOutputBuffers)
    : MAX_CAPACITY(maxCap), numOutputBuffers(nOutputBuffers)
{
    printf("Initialize DRAM\n");
    outputBuffers.nBuffer = nOutputBuffers;
    buffersUsed = 0;
    outputBuffers.wrapper = new Run(DRAM_PAGE_SIZE);
    // Input buffer Capacity in Bytes
    capacity = MAX_CAPACITY - (unsigned long long)nOutputBuffers * DRAM_PAGE_SIZE;
}

DRAM::~DRAM()
{
    capacity = MAX_CAPACITY;
    std::vector<Page *> emptyPages;
    std::vector<bool> emptyBitmap;
    DramOutputBuffers emptyOutputBuffers;
    emptyOutputBuffers.nBuffer = numOutputBuffers;

    for (int i = 0; i < inputBuffers.size(); i++)
    {
        delete inputBuffers[i];
    }
    inputBuffers.clear();

    inputBuffers.swap(emptyPages);
    inputBuffersBitmap.swap(emptyBitmap);
    buffersUsed = 0;
    outputBuffers.clear();
    delete outputBuffers.wrapper;
    capacity = MAX_CAPACITY - numOutputBuffers * DRAM_PAGE_SIZE;
}

Page *DRAM::getPage(int idx)
{
    if (idx >= 0 && idx < inputBuffers.size())
    {
        return inputBuffers[idx];
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
    inputBuffers.push_back(page);

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
        Page *firstPage = inputBuffers[0];
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
        delete firstPage; // release memory
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
    inputBuffers[idx] = page;

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
    for (long unsigned int i = 0; i < inputBuffers.size(); i++)
    {
        if (inputBuffersBitmap[i])
        {
            cleanedPages.push_back(inputBuffers[i]);
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
    Return: a run * of pages, this run has size same as DRAM input buffer size
*/
unsigned long long DRAM::readRecords(const char *LOCAL_INPUT_DIR, int pageStart, int pageEnd,
                                     int recordSize)
{
    unsigned long long totalBytes = 0;
    for (int i = pageStart; i < pageEnd; i++)
    {
        Page *page = readPage(LOCAL_INPUT_DIR, i, recordSize);
        addPage(page);
        totalBytes += (unsigned long long)page->getBytes();
    }
    return totalBytes;
}

/*
    Read a page from LOCAL_INPUT_DIR.
    Note that pages are stored directly under LOCAL_INPUT_DIR
*/
Page *DRAM::readPage(const char *LOCAL_INPUT_DIR, int pageIdx, int recordSize)
{
    char separator = get_directory_separator();
    Page *page = new Page(pageIdx, DRAM_PAGE_SIZE / recordSize, DRAM_PAGE_SIZE);
    std::string pagePath = std::string(LOCAL_INPUT_DIR) + separator + std::to_string(pageIdx);
    std::ifstream pageFile(pagePath);
    if (pageFile.is_open())
    {
        std::string line;
        while (std::getline(pageFile, line))
        {
            page->addRecord(new Record(recordSize, line));
        }
        pageFile.close();
    }
    else
    {
        // printf("Fail to create file for page %d\n", pageIdx);
        std::cerr << "Error opening file for run page " << pageIdx << " .\n"
                  << std::endl;
        delete page;
        return nullptr;
    }

    return page;
}

void DRAM::clear()
{
    capacity = MAX_CAPACITY;
    std::vector<Page *> emptyPages;
    std::vector<bool> emptyBitmap;
    DramOutputBuffers emptyOutputBuffers;
    emptyOutputBuffers.nBuffer = numOutputBuffers;

    // int count = 0;
    for (int i = 0; i < inputBuffers.size(); i++)
    {
        // count += 1;
        delete inputBuffers[i];
    }
    // printf("Removed %d pages from inputBuffer.\n", count);
    inputBuffers.clear();

    inputBuffers.swap(emptyPages);
    inputBuffersBitmap.swap(emptyBitmap);
    buffersUsed = 0;
    outputBuffers.clear();
    capacity = MAX_CAPACITY - numOutputBuffers * DRAM_PAGE_SIZE;
}

bool DRAM::isFull() const
{
    return capacity == MAX_CAPACITY;
}

void DRAM::mergeFromSelfToDest(Disk *dest, const char *outputTXT, std::vector<Run *> &rTable, int outputRunidx)
{
    // All merged result goes to SSD or HDD
    // But physically all stored in LOCAL_DRAM_SIZED_RUNS_DIR

    // Setup of total output buffers size makes sure that
    // output buffers can hold the maximum number of records,
    // where it's equal to the maximum number of cachesized runs;
    int fanin = rTable.size();

    TournamentTree *tree = new TournamentTree(fanin, rTable);

    unsigned long long bytesInRun = 0;
    printf("buffersUsed: %d\n", buffersUsed);

    char separator = get_directory_separator();
    std::string newRunDir = "run" + std::to_string(outputRunidx);
    std::string newRunPath = std::string(LOCAL_DRAM_SIZED_RUNS_DIR) + separator + newRunDir;

    dest->createRunFolder(LOCAL_DRAM_SIZED_RUNS_DIR, outputRunidx);

    int pageIdx = 0;
    Record * prevWinner = nullptr;
    while (tree->hasNext())
    {
        Record *winner = tree->popWinner();
        if(prevWinner == nullptr) {
            prevWinner = winner;
        }else if(prevWinner->key + prevWinner->content == winner->key +winner->content) 
        {
            numDuplicate++;
            continue;
        }else{
            prevWinner = winner;
        }
        // if output buffer is full
        if (outputBuffers.isFull())
        {
            // Report Spilling happen to trace.txt
            dest->outputSpillState(outputTXT);
            // Simulate write to SSD
            dest->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);
            int bytesInDRAMOutputBuffers = outputBuffers.wrapper->getBytes();
            bytesInRun += bytesInDRAMOutputBuffers;
            
            while (!outputBuffers.isEmpty())
            {
                // Write to curr run in dest Disk
                Page *page = outputBuffers.wrapper->getFirstPage();
                int firstPageOriginalBytes = page->getBytes();
                dest->writePageToRunFolder(newRunPath.c_str(), page, pageIdx);
                pageIdx++;
                outputBuffers.wrapper->removeFirstPage(firstPageOriginalBytes, true);
            }
            outputBuffers.clear();
        }
        outputBuffers.wrapper->addRecord(winner);
    }
    if (!outputBuffers.isEmpty())
    {
        // write all remaining records in output buffers to the run on dest Disk
        // Report Spilling happen to output
        dest->outputSpillState(outputTXT);
        // Simulate write to SSD
        dest->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);
        bytesInRun += outputBuffers.wrapper->getBytes();

        while (!outputBuffers.wrapper->isEmpty())
        {
            Page *page = outputBuffers.wrapper->getFirstPage();
            int firstPageOriginalBytes = page->getBytes();
            // Write to curr run in dest Disk
            dest->writePageToRunFolder(newRunPath.c_str(), page, pageIdx);
            pageIdx++;
            outputBuffers.wrapper->removeFirstPage(firstPageOriginalBytes, true);
        }
        outputBuffers.clear();
    }
    dest->addRunToOutputBuffer(bytesInRun);
    // Keep track of run file in disk's output buffer dest->addRunToOutputBuffer(bytesInRun);
    delete tree;
    for (int i = 0; i < rTable.size(); i++)
    {
        delete rTable[i];
    }
    rTable.clear();
}

unsigned long long
DRAM::getCapacity() const
{
    return capacity;
}

std::vector<Page *> DRAM::getInputBuffers() const { return inputBuffers; };

DramOutputBuffers DRAM::getOuputBuffers() const { return outputBuffers; }
