#include "Disk.h"
#include "DRAM.h"
#include "defs.h"
#include "TournamentTree.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

// Constructor
Disk::Disk(unsigned long long maxCap, long lat, long bw, const char *dType, int nOutputBufferl)
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
        outputBuffers.maxCap = (unsigned long long)nOutputBuffer * PAGE_SIZE;
        capacity = MAX_CAPACITY - (unsigned long long)nOutputBuffer * PAGE_SIZE; // capacity of the input buffers
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
    unsortedRuns.push_back(run->clone());

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
bool Disk::addRunToOutputBuffer(const char *runFile, int bytesToWrite)
{
    return outputBuffers.addRun(runFile, bytesToWrite);
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
    temp.push_back(run->clone());
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

bool Disk::delFirstPageFromRunK(int k)
{
    if (k >= 0 && k < unsortedRuns.size() && !unsortedRuns[k]->isEmpty())
    {
        int pSize = unsortedRuns[k]->getBytes();
        unsortedRuns[k]->removeFisrtPage();
        capacity += pSize;
        return true;
    }
    return false;
}

void Disk::mergeFromSelfToSelf(const char *outputTXT)
{
    // Create SSD-Sized runs first
    int fanIn = SSD_SIZE / DRAM_SIZE;
    fanIn = std::min(numUnsortedRuns, fanIn);
    while (numUnsortedRuns > 0)
    {
        std::vector<Run *> currMemSizedRuns;
        for (int i = 0; i < fanIn; i++)
        {
            currMemSizedRuns.push_back(unsortedRuns[i]->clone());
            // set bit to invalid
            eraseRun(i);
        }
        // clean segmented free space
        cleanInvalidRuns();

        TournamentTree *tree = new TournamentTree(fanIn, currMemSizedRuns, this);
        Run *curr = new Run();
        int bytesToWrite = 0;
        // write sorted output to output buffer
        while (tree->hasNext())
        {
            Record *winner = tree->popWinner();
            curr->addRecord(winner);
            bytesToWrite += winner->getSize();
        }
        // Add SSD-Sized run to a temp vector
        addRunToTempList(curr);
        outputAccessState(ACCESS_WRITE, curr->getBytes(), outputTXT);
    }

    moveAllTempToUnsorted();
    // Merge all SSD-Sized runs at once
    if (numUnsortedRuns > 1)
    {
        TournamentTree *tree = new TournamentTree(fanIn, unsortedRuns, this);
        Run *curr = new Run();
        int bytesToWrite = 0;
        // write sorted output to output buffer
        while (tree->hasNext())
        {
            Record *winner = tree->popWinner();
            curr->addRecord(winner);
            bytesToWrite += winner->getSize();
        }
        addRun(curr);
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
        capacity = MAX_CAPACITY - nOutputBuffer * PAGE_SIZE;
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

    // Print output to both console and file
    outputFile << "ACCESS -> A " << accessType << " to " << diskType << " was made with size " << totalBytes << " bytes and latency " << duration.count() << " us\n";

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
    outputFile << "STATE -> MERGE_RUNS_" << diskType << ": Merge sorted runs on the SSD device\n";

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
        while (!curr->isEmpty())
        {
            const char *bytes = curr->getFirstRecord()->serialize();
            outputFile.write(bytes, strlen(bytes));
            outputFile << '\n';
            curr->removeFisrtRecord();
        }
        outputRun->removeFisrtPage();
    }

    outputFile.close();
    return 0;
}

int Disk::createRunFolder(const char *LOCAL_DIR, int newRunId)
{
    char separator = get_directory_separator();
    std::string newRunDir = "run" + std::to_string(newRunId);
    std::string newRunPath = LOCAL_DIR + separator + newRunDir;

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
    std::string pageFilePath = runFolderPath + separator + pageFileName;
    std::ofstream pageFile(pageFilePath, std::ios::binary);

    if (!pageFile)
    {
        // printf("Fail to create file for page %d\n", pageIdx);
        std::cerr << "Error opening file for writing page" << pageIdx << " ." << std::endl;
        return 0;
    }

    while (!page->isEmpty())
    {
        const char *bytes = page->getFirstRecord()->serialize();
        pageFile.write(bytes, strlen(bytes));
        pageFile << '\n';
        page->removeFisrtRecord();
    }
    pageFile.close();

    return 0;
}

// Read run file from runFile stored in localPath
// Run *Disk::scanRun(const char *runFile)
// {
//     std::ifstream file(runFile);
//     Run *recordsInRun = new Run();
//     int countTotal = 0;
//     if (file.is_open())
//     {
//         TRACE(true);
//         std::string line;
//         while (std::getline(file, line))
//         {
//             countTotal++;
//             recordsInRun->addRecord(new Record(recordSize, line.c_str()));
//         }
//         file.close();
//     }
//     else
//     {
//         printf("FILE cannot be opend\n");
//     }

//     return recordsInRun;
// }

int Disk::clearOuputBuffer()
{
    for (long unsigned int i = 0; i < outputBuffers.runFiles.size(); i++)
    {
        const char *filename = outputBuffers.runFiles[i];
        if (remove(filename) != 0)
        {
            perror("File deletion failed");
            return 1; // Non-zero return means the file was not deleted
        }
        else
        {
            printf("File deleted successfully\n");
        }
    }

    std::vector<const char *> newRunFiles;
    outputBuffers.runFiles.swap(newRunFiles);
    outputBuffers.bytesStored = 0;

    return 0;
}

void Disk::print() const
{
    printf("\n------------------------------------%s Data------------------------------------\n", diskType);
    if (std::strcmp(diskType, SSD) == 0)
    {
        printf("MAX Capacity %llu bytes, remaining cap %llu bytes %d\n", MAX_CAPACITY, capacity);
    }
    else
    {
        printf("MAX Capacity INF bytes\n");
    }
    printf("In total has %ld runs\n", unsortedRuns.size());
    for (long unsigned int i = 0; i < unsortedRuns.size(); i++)
    {
        printf(">>>>>>>>>>>>>>>>>>> %d th Run, valid: %d >>>>>>>>>>>>>>>>>>\n", i, static_cast<int>(runBitmap[i]));
        unsortedRuns[i]->print();
    }
    printf("--------------------------------------------------------------------\n");
}

void Disk::printTemp() const
{
    printf("\n------------------------------------%s Merged/Tempoary Saved Data------------------------------------\n", diskType);
    if (std::strcmp(diskType, SSD) > 0)
    {
        printf("MAX Capacity %llu bytes, remaining cap %llu bytes %i\n", MAX_CAPACITY, capacity);
    }
    else
    {
        printf("MAX Capacity INF bytes\n");
    }
    printf("In total has %ld merged runs\n", temp.size());
    for (long unsigned int i = 0; i < temp.size(); i++)
    {
        printf(">>>>>>>>>>>>>>>>>>> %d th Merged Run >>>>>>>>>>>>>>>>>>\n", i);
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

    return outputBuffers.runFiles.size();
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

// To test this main individually:
// use: g++ Run.cpp Record.cpp HeapSort.cpp DRAM.cpp SSD.cpp -o ssd
// int main() {

//     HeapSort tree1;
//     HeapSort tree2;
//     HeapSort tree3;

//     int recordSize = 20;
//     int numRecords = 15;

//     int totalBytes = recordSize * numRecords;

//     Record * r1 = new Record(recordSize, "alsdfeei");
//     Record * r2 = new Record(recordSize, "ewfeasdf");
//     Record * r3 = new Record(recordSize, "bdfsewfh");
//     Record * r4 = new Record(recordSize, "abcdasdf");
//     Record * r5 = new Record(recordSize, "eeerghwr");

//     Record * r6 = new Record(recordSize, "alsdmeei");
//     Record * r7 = new Record(recordSize, "ewfqesdf");
//     Record * r8 = new Record(recordSize, "alsdiefd");
//     Record * r9 = new Record(recordSize, "msjdfhwd");
//     Record * r10 = new Record(recordSize, "edprghwr");

//     Record * r11 = new Record(recordSize, "zlsdmeei");
//     Record * r12 = new Record(recordSize, "oeqesdfw");
//     Record * r13 = new Record(recordSize, "skfjhuwc");
//     Record * r14 = new Record(recordSize, "kviwudvg");
//     Record * r15 = new Record(recordSize, "aaabkkee");

//     r1->setSlot(0);
//     r2->setSlot(0);
//     r3->setSlot(0);
//     r4->setSlot(0);
//     r5->setSlot(0);

//     r6->setSlot(1);
//     r7->setSlot(1);
//     r8->setSlot(1);
//     r9->setSlot(1);
//     r10->setSlot(1);

//     r11->setSlot(2);
//     r12->setSlot(2);
//     r13->setSlot(2);
//     r14->setSlot(2);
//     r15->setSlot(2);

//     // Insert some records
//     tree1.insert(r1);
//     tree1.insert(r2);
//     tree1.insert(r3);
//     tree1.insert(r4);
//     tree1.insert(r5);

//     tree2.insert(r6);
//     tree2.insert(r7);
//     tree2.insert(r8);
//     tree2.insert(r9);
//     tree2.insert(r10);

//     tree3.insert(r11);
//     tree3.insert(r12);
//     tree3.insert(r13);
//     tree3.insert(r14);
//     tree3.insert(r15);

//     std::vector<HeapSort*> runs;
//     runs.push_back(&tree1);
//     runs.push_back(&tree2);
//     runs.push_back(&tree3);

//     // Create an DRAM object with capacity 10GB, latency 0.1ms, and bandwidth 200MB/s
//     DRAM* dram = new DRAM();

//     std::vector<Run*> dramOutput = dram->merge(runs, recordSize);
//     for(long unsigned int i = 0; i < dramOutput.size(); i++) {
// 		Run* run = dramOutput[i];
// 		printf("-------------DRAM Output %d th Run -----------\n", i);
// 		run->print();
// 		printf("\n");
// 	}

//     SSD* ssd = new SSD(0.0001, 200 * 1024 * 1024);

//     ssd->writeData(totalBytes);

//     std::vector<Run*> ssdOutput = ssd->merge(dramOutput, recordSize);

//     for(long unsigned int i = 0; i < ssdOutput.size(); i++) {
// 		Run* run = ssdOutput[i];
// 		printf("------------- SSD Output %d th Run -----------\n", i);
// 		run->print();
// 		printf("\n");
// 	}

//     return 0;
// }
