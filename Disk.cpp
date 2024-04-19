#include "Disk.h"
#include "DRAM.h"
#include "defs.h"
#include "TournamentTree.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>

// Constructor
Disk::Disk(unsigned long long maxCap, long lat, long bw, const char *dType, int nOutputBuffer)
    : MAX_CAPACITY(maxCap), latency(lat), bandwidth(bw), nOutputBuffer(nOutputBuffer)
{
    capacity = maxCap;
    diskType = dType;
    if (nOutputBuffer > 0)
    { // HDD doesn't need output buffers
        outputBuffers.nBuffer = 2;
        outputBuffers.wrapper = new Run();
        capacity = MAX_CAPACITY - 2 * PAGE_SIZE;
    }
}

/*
Write cloned run to Disk
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
    for (int i = 0; i < unsortedRuns.size(); i++)
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

void Disk::mergeFromSelfToDest(Disk *dest, const char *outputTXT)
{
    int fanIn = numUnsortedRuns;
    TournamentTree *tree = new TournamentTree(fanIn, unsortedRuns);
    Run *curr = new Run();
    // write sorted output to output buffer
    while (tree->hasNext())
    {
        Record *winner = tree->popWinner();

        //      if output buffer is full
        if (outputBuffers.isFull())
        {
            outputReadSortedRunState(outputTXT);
            outputAccessState(ACCESS_READ, outputBuffers.wrapper->getBytes(), outputTXT);
            // Report Spilling happen to output
            dest->outputSpillState(outputTXT);
            // Simulate write to SSD
            dest->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);

            while (!outputBuffers.wrapper->isEmpty())
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
        while (!outputBuffers.wrapper->isEmpty())
        {
            outputReadSortedRunState(outputTXT);
            outputAccessState(ACCESS_READ, outputBuffers.wrapper->getBytes(), outputTXT);
            dest->outputSpillState(outputTXT);
            // Simulate write to SSD
            dest->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);
            curr->appendPage(outputBuffers.wrapper->getFirstPage()->clone());
            outputBuffers.wrapper->removeFisrtPage();
        }
        outputBuffers.clear();
    }
    dest->addRun(curr);
    clear();
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
        outputBuffers.clear();
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
            char *bytes = curr->getFirstRecord()->serialize();
            outputFile.write(bytes, strlen(bytes));
            outputFile << '\n';
            curr->removeFisrtRecord();
        }
        outputRun->removeFisrtPage();
    }

    outputFile.close();
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
    printf("In total has %d runs\n", unsortedRuns.size());
    for (int i = 0; i < unsortedRuns.size(); i++)
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
        printf("MAX Capacity %llu bytes, remaining cap %llu bytes %d\n", MAX_CAPACITY, capacity);
    }
    else
    {
        printf("MAX Capacity INF bytes\n");
    }
    printf("In total has %d merged runs\n", temp.size());
    for (int i = 0; i < temp.size(); i++)
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
    for (int i = 0; i < unsortedRuns.size(); i++)
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
// use: g++ Run.cpp Record.cpp TreeOfLosers.cpp DRAM.cpp SSD.cpp -o ssd
// int main() {

//     TreeOfLosers tree1;
//     TreeOfLosers tree2;
//     TreeOfLosers tree3;

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

//     std::vector<TreeOfLosers*> runs;
//     runs.push_back(&tree1);
//     runs.push_back(&tree2);
//     runs.push_back(&tree3);

//     // Create an DRAM object with capacity 10GB, latency 0.1ms, and bandwidth 200MB/s
//     DRAM* dram = new DRAM();

//     std::vector<Run*> dramOutput = dram->merge(runs, recordSize);
//     for(int i = 0; i < dramOutput.size(); i++) {
// 		Run* run = dramOutput[i];
// 		printf("-------------DRAM Output %d th Run -----------\n", i);
// 		run->print();
// 		printf("\n");
// 	}

//     SSD* ssd = new SSD(0.0001, 200 * 1024 * 1024);

//     ssd->writeData(totalBytes);

//     std::vector<Run*> ssdOutput = ssd->merge(dramOutput, recordSize);

//     for(int i = 0; i < ssdOutput.size(); i++) {
// 		Run* run = ssdOutput[i];
// 		printf("------------- SSD Output %d th Run -----------\n", i);
// 		run->print();
// 		printf("\n");
// 	}

//     return 0;
// }
