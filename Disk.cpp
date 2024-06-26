#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <fstream>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>
#include <unordered_map>

#include "Disk.h"
#include "DRAM.h"
#include "defs.h"
#include "TournamentTree.h"

// Constructor
Disk::Disk(unsigned long long maxCap, long lat, long bw, const char *dType, int nOutputBuffer)
    : MAX_CAPACITY(maxCap), latency(lat), bandwidth(bw), nOutputBuffer(nOutputBuffer)
{
    capacity = maxCap;
    diskType = dType;
    // localPath = path;
    numUnsortedRuns = 0;
    numTempRuns = 0;
    if (nOutputBuffer > 0)
    { // HDD doesn't need output buffers
        outputBuffers.nBuffer = nOutputBuffer;
        outputBuffers.maxCap = (unsigned long long)nOutputBuffer * SSD_PAGE_SIZE;
        outputBuffers.bytesStored = 0;
        outputBuffers.numberRuns = 0;
        capacity = MAX_CAPACITY - (unsigned long long)nOutputBuffer * SSD_PAGE_SIZE; // capacity of the input buffers
    }
}

/*
Write cloned run to Disk (virtually)
*/
bool Disk::addRun(Run *run)
{
    if (std::strcmp(diskType, SSD) > 0 && run->getBytes() > capacity)
    {
        printf("Disk does Not enough space\n");
        return false;
    }
    unsortedRuns.push_back(run);

    // Decrease Disk capacity
    capacity -= run->getBytes();
    runBitmap.push_back(true);
    numUnsortedRuns++;
    // printf("WATCH ME: %d\n", static_cast<int>(runBitmap[runBitmap.size() - 1]));
    return true;
}

/*
Write cloned run to Disk (Physically add to localPath)
By the time this function is called, the run
has been written to a local run file,
so it only needs to keep track of which file it's
*/
bool Disk::addRunToOutputBuffer(int bytesToWrite)
{
    return outputBuffers.addRun(bytesToWrite);
}

void Disk::moveRunToTempList(int runIdx)
{
    Run *run = getRunCopy(0);

    if (!eraseRun(runIdx))
    {
        return;
    }
    addRunToTempList(run);
}

bool Disk::addRunToTempList(Run *run)
{
    if (std::strcmp(diskType, SSD) > 0 && run->getBytes() > capacity)
    {
        printf("Disk does Not enough space to store tempoarary run\n");
        return false;
    }
    temp.push_back(run);
    // Decrease Disk capacity
    capacity -= run->getBytes();
    // tempRunBitmap.push_back(true);
    numTempRuns++;
    return true;
}

void Disk::moveAllTempToUnsorted()
{
    if (temp.size() == 0)
    {
        return;
    }
    // Assume all runs in temps are valid
    // Clean temp list
    int usedBytes = 0;
    capacity = MAX_CAPACITY;
    unsortedRuns.swap(temp);
    temp.clear();
    numTempRuns = 0;

    numUnsortedRuns = unsortedRuns.size();
    runBitmap.clear();
    for (long unsigned int i = 0; i < unsortedRuns.size(); i++)
    {
        // Assume all runs in temps are valid
        runBitmap.push_back(true);
        capacity -= unsortedRuns[i]->getBytes();
    }
}

bool Disk::eraseRun(int runIdx)
{
    if (runIdx > unsortedRuns.size())
    {
        printf("run index invalid");
        return false;
    }
    // Increase Disk capacity
    if (std::strcmp(diskType, SSD) > 0)
    {
        capacity += unsortedRuns[runIdx]->getBytes();
    }
    runBitmap[runIdx] = false;
    numUnsortedRuns--;
    return true;
}


void Disk::mergeMemorySizedRuns(const char *outputTXT, const char *OUTPUT_TABLE)
{
    // Create SSD-Sized runs first
    int fanIn = SSD_SIZE / DRAM_SIZE;
    int runIdxOffset = 0;

    int totalNumberMemorySizedRuns = countRunsInDirectory(std::string(LOCAL_DRAM_SIZED_RUNS_DIR));
    fanIn = std::min(totalNumberMemorySizedRuns, fanIn);
    if (totalNumberMemorySizedRuns == 0)
    {
        printf("No memory-sized runs to be merged on HDD\n");
        return;
    }
    if (totalNumberMemorySizedRuns == 1)
    {
        printf("Only one memory-sized runs created, no need to be merge\n");
        return;
    }
    std::vector<Run *> dummyTable;
    int outputRunidx = 0;
    while (runIdxOffset < totalNumberMemorySizedRuns)
    {
        // Prepare metadata table for runs
        std::vector<int> runTable;
        for (int i = runIdxOffset; i < runIdxOffset + fanIn && i < totalNumberMemorySizedRuns; i++)
        {
            runTable.push_back(i);
        }
        TournamentTree *tree = new TournamentTree(fanIn, this, runTable, LOCAL_DRAM_SIZED_RUNS_DIR);

        // Create new Run folder in LOCAL_SSD_SIZED_RUNS_DIR
        char separator = get_directory_separator();
        std::string newRunDir = "run" + std::to_string(outputRunidx);
        std::string newRunPath = std::string(LOCAL_SSD_SIZED_RUNS_DIR) + separator + newRunDir;

        createRunFolder(LOCAL_SSD_SIZED_RUNS_DIR, outputRunidx);

        // Initialize pageIdx
        int outputPageIdx = 0;
        int outputPageBytesOccupied = 0;
        std::string pageFileName = std::to_string(outputPageIdx);
        std::string pageFilePath = std::string(newRunPath) + separator + pageFileName;
        std::ofstream pageFile(pageFilePath, std::ios::binary);

        int bytesToWrite = 0;
        std::string prevKey = "None";
        // // write sorted output to output buffer
        while (tree->hasNext())
        {
            Record *winner = tree->popWinner();
            if(prevKey== "None") {
                prevKey = winner->key +winner->content;
            }else if(prevKey == winner->key +winner->content) 
            {
                numDuplicate++;
                continue;
            }else{
                prevKey = winner->key +winner->content;
            }
            if (HDD_PAGE_SIZE - outputPageBytesOccupied < recordSize)
            {
                pageFile.close();
                // Create a new page file inside LOCAL_INPUT_DIR
                outputPageIdx++;
                pageFileName = std::to_string(outputPageIdx);
                pageFilePath = std::string(newRunPath) + separator + pageFileName;
                pageFile.open(pageFilePath, std::ios::binary);
                outputPageBytesOccupied = 0;
                if (!pageFile)
                {
                    std::cerr << "Error opening file for writing page " << pageFilePath << " ." << std::endl;
                    return;
                }
            }
            // write record to page file
            pageFile.write(winner->key.data(), winner->key.size());
            pageFile.write(winner->content.data(), winner->content.size());

            // write content
            delete winner;
            pageFile << "\n";
            outputPageBytesOccupied += recordSize;
        }
        pageFile.close();
        outputAccessState(ACCESS_WRITE, outputPageBytesOccupied, outputTXT);
        runIdxOffset += fanIn;
        outputRunidx++;
        delete tree;
    }
}

void Disk::mergeSSDSizedRuns(const char *outputTXT, const char *OUTPUT_TABLE)
{
    // Reset runIdxOffset since they are stored in LOCAL_SSD_SIZED_RUNS_DIR now
    int runIdxOffset = 0;
    int totalNumberSSDSizedRuns = countRunsInDirectory(std::string(LOCAL_SSD_SIZED_RUNS_DIR));
    int fanIn = totalNumberSSDSizedRuns;

    if (totalNumberSSDSizedRuns == 1)
    {
        char separator = get_directory_separator();
        std::string runDir = "run" + std::to_string(0);
        std::string runPath = std::string(LOCAL_SSD_SIZED_RUNS_DIR) + separator + runDir;
        writeRunToOutputTable(runPath.c_str(), OUTPUT_TABLE);
        return;
    }

    // Merge all SSD-Sized runs if more than 1
    // Write to output table directly
    if (totalNumberSSDSizedRuns > 1)
    {
        // Prepare for tournament tree
        std::vector<int> runTable;
        for (int i = runIdxOffset; i < runIdxOffset + fanIn && i < totalNumberSSDSizedRuns; i++)
        {
            runTable.push_back(i);
        }
        TournamentTree *tree = new TournamentTree(fanIn, this, runTable, LOCAL_SSD_SIZED_RUNS_DIR);

        // Create the input table file
        std::ofstream outputFile(OUTPUT_TABLE, std::ios::binary);

        // Check if the file opened successfully
        if (!outputFile.is_open())
        {
            std::cerr << "\nError: Could not open file trace0.txt for writing." << std::endl;
            return;
        }

        unsigned long long bytesToWrite = 0;
        // write merge result to output table
        std::string prevKey = "None";
        while (tree->hasNext())
        {
            Record *winner = tree->popWinner();
             if(prevKey == "None") {
                prevKey = winner->key +winner->content;
            }else if(prevKey == winner->key + winner->content) 
            {
                numDuplicate++;
                continue;
            }else{
                prevKey = winner->key +winner->content;
            }
            // char *bytes = winner->serialize();
            outputFile.write(winner->key.data(), winner->key.size());
            outputFile.write(winner->content.data(), winner->content.size());

            // outputFile.write(bytes, strlen(bytes));
            outputFile << "\n";
            bytesToWrite += recordSize;
            delete winner;
        }
        outputFile.close();
        outputAccessState(ACCESS_WRITE, bytesToWrite, outputTXT);
        runIdxOffset += fanIn;
        delete tree;
    }
}

void Disk::clear()
{
    capacity = MAX_CAPACITY;
    std::vector<Run *> emptyRuns;
    std::vector<Run *> emptyTemp;

    std::vector<bool> emptyRunBitmap;

    unsortedRuns.swap(emptyRuns);
    temp.swap(emptyTemp);
    runBitmap.swap(emptyRunBitmap);
    numUnsortedRuns = 0;
    if (nOutputBuffer > 0)
    {
        clearOuputBuffer();
        capacity = MAX_CAPACITY - nOutputBuffer * SSD_PAGE_SIZE;
    }
}

int Disk::outputSpillState(const char *outputTXT)
{
    // Open the output file in overwrite mode
    std::ofstream outputFile(outputTXT, std::ios::app);

    // Check if the file opened successfully
    if (!outputFile.is_open())
    {
        std::cerr << "Error: Could not open file trace0.txt for writing." << std::endl;
        return 1; // Return error code
    }

    // Print output to both console and file
    outputFile << "STATE -> SPILL_RUNS_" << diskType << ": Spill sorted runs to the " << diskType << " device\n";

    // close file
    outputFile.close();
    return 0;
}

int Disk::outputReadSortedRunState(const char *outputTXT)
{
    // Open the output file in overwrite mode
    std::ofstream outputFile(outputTXT, std::ios::app);

    // Check if the file opened successfully
    if (!outputFile.is_open())
    {
        std::cerr << "Error: Could not open file trace0.txt for writing." << std::endl;
        return 1; // Return error code
    }

    // Print output to both console and file
    outputFile << "STATE -> READ_RUN_PAGES_" << diskType << ": Read sorted run pages from the " << diskType << " device\n";

    // close file
    outputFile.close();
    return 0;
}

int Disk::outputAccessState(const char *accessType,
                            unsigned long long totalBytes,
                            const char *outputTXT)
{
    // Open the output file in overwrite mode
    std::ofstream outputFile(outputTXT, std::ios::app);

    // Check if the file opened successfully
    if (!outputFile.is_open())
    {
        std::cerr << "Error: Could not open file trace0.txt for writing." << std::endl;
        return 1; // Return error code
    }
    // Start measuring time
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    writeData(totalBytes);

    // Stop measuring time
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    // Calcualte duration
    std::chrono::microseconds duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    unsigned long long bytesInMB = totalBytes/1024/1024;
    if(bytesInMB < 1) 
    {
        outputFile << "ACCESS -> A " << accessType << " to " << diskType << " was made with size " << totalBytes << " bytes and latency " << duration.count() << " us\n";
    }
    else
    {
    // Print output to both console and file
        outputFile << "ACCESS -> A " << accessType << " to " << diskType << " was made with size " << bytesInMB << " Mega bytes and latency " << duration.count() << " us\n";
    }
    // close file
    outputFile.close();
    return 0;
}

int Disk::outputMergeMsg(const char *outputTXT)
{
    // Open the output file in overwrite mode
    std::ofstream outputFile(outputTXT, std::ios::app);

    // Check if the file opened successfully
    if (!outputFile.is_open())
    {
        std::cerr << "Error: Could not open file trace0.txt for writing." << std::endl;
        return 1; // Return error code
    }

    // Print output to both console and file
    outputFile << "STATE -> MERGE_RUNS_" << diskType << ": Merge sorted runs on the " << diskType << " device\n";

    // close file
    outputFile.close();
    return 0;
}

// Method to simulate read operation
int Disk::readData(unsigned long long sizeInBytes)
{
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(latency)));

    std::cout << "Read " << sizeInBytes << " bytes from " << diskType << ". Transfer time: " << transferTime << " seconds\n";
    return 0;
}

// Method to simulate write operation
int Disk::writeData(unsigned long long sizeInBytes)
{
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    double totalLatency = latency + transferTime;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(totalLatency)));
    return 0;
}

int Disk::writeOutputTable(const char *outputTXT)
{ // Open the output file in overwrite mode
    std::ofstream outputFile(outputTXT, std::ios::binary);

    // Check if the file opened successfully
    if (!outputFile.is_open())
    {
        std::cerr << "\nError: Could not open file trace0.txt for writing." << std::endl;
        return 1; // Return error code
    }
    printf("numUnsortedRuns: %d\n", numUnsortedRuns);
    if (numUnsortedRuns != 1)
    {
        std::cerr << "\nThere are " << numUnsortedRuns << " runs on disk " << diskType << std::endl;
        std::cerr << "\nError: Should only write One unified sorted Run to a single table\nPlease check Calculations" << std::endl;
        return 1; // Return error code
    }
    Run *outputRun = getRunCopy(0);
    while (!outputRun->isEmpty())
    {
        Page *curr = outputRun->getFirstPage();
        int firstPageOriginalBytes = curr->getBytes();
        while (!curr->isEmpty())
        {
            // const char *bytes = curr->getFirstRecord()->serialize();
            // outputFile.write(bytes, strlen(bytes));
            Record *toBeRemovedRec = curr->getFirstRecord();
            outputFile.write(toBeRemovedRec->key.data(), toBeRemovedRec->key.size());
            outputFile.write(toBeRemovedRec->content.data(), toBeRemovedRec->content.size());

            outputFile << '\n';
            curr->removeFisrtRecord();
            delete toBeRemovedRec;
        }
        outputRun->removeFirstPage(firstPageOriginalBytes, true);
    }

    outputFile.close();
    return 0;
}

int Disk::createRunFolder(const char *LOCAL_DIR, int newRunIdx)
{
    char separator = get_directory_separator();
    std::string newRunDir = "run" + std::to_string(newRunIdx);
    std::string newRunPath = std::string(LOCAL_DIR) + separator + newRunDir;

    // Create the directory specified by fullPathForRun
    if (mkdir(newRunPath.c_str(), 0755) == -1)
    {
        perror("Failed to create directory for run\n");
        return 1;
    }
    return 0;
}

/**
 *
 */
int Disk::writePageToRunFolder(const char *runFolderPath, Page *page, int pageIdx)
{
    char separator = get_directory_separator();
    std::string pageFileName = std::to_string(pageIdx);
    std::string pageFilePath = std::string(runFolderPath) + separator + pageFileName;
    std::ofstream pageFile(pageFilePath, std::ios::binary);

    if (!pageFile)
    {
        // printf("Fail to create file for page %d\n", pageIdx);
        std::cerr << "Error opening file for writing page" << pageIdx << " ." << std::endl;
        return 0;
    }

    while (!page->isEmpty())
    {
        Record *toBeRemovedRec = page->getFirstRecord();
        pageFile.write(toBeRemovedRec->key.data(), toBeRemovedRec->key.size());
        pageFile.write(toBeRemovedRec->content.data(), toBeRemovedRec->content.size());

        // delete[] bytes;ls
        pageFile << '\n';
        page->removeFisrtRecord();
        delete toBeRemovedRec;
    }
    pageFile.close();

    return 0;
}
int Disk::writeRunToOutputTable(const char *runFolderPath, const char *OUTPUT_TABLE)
{
    int numPages = countFilesInDirectory(std::string(runFolderPath));
    // For each page in this runFolderPath
    char separator = get_directory_separator();
    // Open the output file in overwrite mode
    std::ofstream outputFile(OUTPUT_TABLE, std::ios::binary);
    int outputNumRecords = 0;

    // Check if the file opened successfully
    if (!outputFile.is_open())
    {
        std::cerr << "\nError: Could not open file trace0.txt for writing." << std::endl;
        return 1; // Return error code
    }

    for (int pageIdx = 0; pageIdx < numPages; pageIdx++)
    {
        // Page *page = new Page(pageIdx, PAGE_SIZE / recordSize, PAGE_SIZE);
        std::string pagePath = std::string(runFolderPath) + separator + std::to_string(pageIdx);
        std::ifstream pageFile(pagePath);
        if (pageFile.is_open())
        {
            std::string line;
            while (std::getline(pageFile, line))
            {
                outputFile.write(line.c_str(), strlen(line.c_str()));
                outputFile << '\n';
                outputNumRecords++;
            }
            pageFile.close();
        }
        else
        {
            // printf("Fail to create file for page %d\n", pageIdx);
            std::cerr << "Error opening file for run page " << pageIdx << " .\n"
                      << std::endl;
            return 1;
        }
    }
    outputFile.close();
    outputAccessState(ACCESS_WRITE, outputNumRecords * recordSize, outputTXT);
    return 0;
}

Page *Disk::readPageJFromRunK(const char *LOCAL_DIR, 
                            int runIdx, 
                            int pageStart,
                            int pageEnd,
                            int pageCurrIdx)
{
    Page *newHDDPage = new Page(pageCurrIdx, HDD_PAGE_SIZE / recordSize, HDD_PAGE_SIZE);
    for (int pageIdx = pageStart; pageIdx < pageEnd; pageIdx++)
    {
        char separator = get_directory_separator();
        std::string runDir = "run" + std::to_string(runIdx);
        std::string runPath = std::string(LOCAL_DIR) + separator + runDir;
        std::string pageFileName = std::to_string(pageIdx);
        std::string pageFilePath = runPath + separator + pageFileName;
        std::ifstream pageFile(pageFilePath);


        if (pageFile.is_open())
        {
            std::string line;
            while (std::getline(pageFile, line))
            {
                Record *r = new Record(recordSize, line);
                r->setSlot(runIdx);
                newHDDPage->addRecord(r);
            }
            pageFile.close();
        }
        else
        {

            std::cerr << "Error opening file for read page from path " << pageFilePath << std::endl;
            return nullptr;
        }
    }
    return newHDDPage;
}

int Disk::getNumPagesInRunOnDisk(const char *LOCAL_DIR, int runIdx)
{
    char separator = get_directory_separator();
    std::string runDir = "run" + std::to_string(runIdx);
    std::string runPath = std::string(LOCAL_DIR) + separator + runDir;
    return countFilesInDirectory(runPath);
}

int Disk::clearOuputBuffer()
{

    outputBuffers.numberRuns = 0;
    outputBuffers.bytesStored = 0;

    return 0;
}

void Disk::print() const
{
    printf("\n------------------------------------%s Data------------------------------------\n", diskType);
    if (std::strcmp(diskType, SSD) == 0)
    {
        printf("MAX Capacity %llu bytes, remaining cap %llu bytes %llu\n", MAX_CAPACITY, capacity);
    }
    else
    {
        printf("MAX Capacity INF bytes\n");
    }
    printf("In total has %ld runs\n", unsortedRuns.size());
    for (long unsigned int i = 0; i < unsortedRuns.size(); i++)
    {
        printf(">>>>>>>>>>>>>>>>>>> %lu th Run, valid: %d >>>>>>>>>>>>>>>>>>\n", i, static_cast<int>(runBitmap[i]));
        unsortedRuns[i]->print();
    }
    printf("--------------------------------------------------------------------\n");
}

void Disk::printTemp() const
{
    printf("\n------------------------------------%s Merged/Tempoary Saved Data------------------------------------\n", diskType);
    if (std::strcmp(diskType, SSD) > 0)
    {
        printf("MAX Capacity %llu bytes, remaining cap %llu bytes %llu\n", MAX_CAPACITY, capacity);
    }
    else
    {
        printf("MAX Capacity INF bytes\n");
    }
    printf("In total has %ld merged runs\n", temp.size());
    for (long unsigned int i = 0; i < temp.size(); i++)
    {
        printf(">>>>>>>>>>>>>>>>>>> %lu th Merged Run >>>>>>>>>>>>>>>>>>\n", i);
        temp[i]->print();
    }
    printf("--------------------------------------------------------------------\n");
}

bool Disk::isFull() const
{
    return capacity == MAX_CAPACITY;
}

unsigned long long Disk::getCapacity() const
{
    if (std::strcmp(diskType, HDD) == 0)
    {
        printf("%s has unlimited capacity\n", HDD);
    }
    return capacity;
}

unsigned long long Disk::getOutputBufferCapacity() const
{
    if (std::strcmp(diskType, HDD) == 0)
    {
        printf("%s has unlimited capacity\n", HDD);
    }
    return outputBuffers.getCapacity();
}

long Disk::getLatency() const
{
    return latency;
}

Run *Disk::getRunCopy(int k) const
{
    if (k < 0 || k >= unsortedRuns.size() || runBitmap[k] == false)
    {
        printf("Invalid index k\n");
        return nullptr;
    }
    return unsortedRuns[k]->clone();
}

Run *Disk::getRun(int k) const
{
    if (k < 0 || k >= unsortedRuns.size() || runBitmap[k] == false)
    {
        printf("Invalid index k\n");
        return nullptr;
    }
    return unsortedRuns[k];
}

bool Disk::runIsValid(int idx) const
{
    if (idx < 0 || idx >= runBitmap.size())
    {
        return false;
    }

    return runBitmap[idx];
}

int Disk::getNumUnsortedRuns() const
{
    return numUnsortedRuns;
}

int Disk::getNumRunsInOutputBuffer() const
{

    return outputBuffers.numberRuns;
}

int Disk::getNumTempRuns() const
{
    return numTempRuns;
}

unsigned long long Disk::getMaxCap() const
{
    return MAX_CAPACITY;
}

// Physically Remove invalid runs from Disk
void Disk::cleanInvalidRuns()
{
    std::vector<Run *> cleanedRuns;
    std::vector<bool> cleanedBitmap;
    unsigned long long used = 0;
    for (long unsigned int i = 0; i < unsortedRuns.size(); i++)
    {
        if (runBitmap[i])
        {
            cleanedRuns.push_back(unsortedRuns[i]->clone());
            // cleanedRuns.push_back(unsortedRuns[i]);
            cleanedBitmap.push_back(true);
            used += unsortedRuns[i]->getBytes();
        }
    }
    for (int j = 0; j < temp.size(); j++)
    {
        used += temp[j]->getBytes();
    }
    numUnsortedRuns = cleanedRuns.size();
    capacity = MAX_CAPACITY - used;
    unsortedRuns.swap(cleanedRuns);
    runBitmap.swap(cleanedBitmap);
}

long Disk::getBandwidth() const
{
    return bandwidth;
}

const char *Disk::getType() const
{
    return diskType;
}

void Disk::setMaxCap(unsigned long long newCap)
{
    MAX_CAPACITY = newCap;
}

void Disk::setCapacity(unsigned long long newCap)
{
    capacity = newCap;
}
