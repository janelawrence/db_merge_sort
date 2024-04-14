#include "SSD.h"
#include "DRAM.h"
#include "defs.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>

// Constructor
SSD::SSD(unsigned long long maxCap, long lat, long bw) : MAX_CAPACITY(maxCap), latency(lat), bandwidth(bw)
{
    capacity = maxCap;
}

/*
Write cloned run to SSD
*/
bool SSD::addRun(Run *run)
{
    if (run->getBytes() > capacity)
    {
        printf("SSD does Not enough space");
        return false;
    }
    runs.push_back(run->clone());

    // Decrease SSD capacity
    capacity -= run->getBytes();
    runBitmap.push_back(true);
    numRuns++;
    // printf("WATCH ME: %d\n", static_cast<int>(runBitmap[runBitmap.size() - 1]));
    return true;
}

bool SSD::eraseRun(int runIdx)
{
    if (runIdx > runs.size())
    {
        printf("run index invalid");
        return false;
    }
    // Increase SSD capacity
    capacity += runs[runIdx]->getBytes();
    runBitmap[runIdx] = false;
    numRuns--;
    return true;
}

void SSD::clear()
{
    capacity = MAX_CAPACITY;
    std::vector<Run *> emptyRuns;
    // std::vector<Run *> emptyDRAMRuns;

    std::vector<bool> emptyRunBitmap;
    // std::vector<bool> emptyDramRunBitmap;

    runs.swap(emptyRuns);
    // dramSizedRuns.swap(emptyDRAMRuns);
    runBitmap.swap(emptyRunBitmap);
    // dramRunBitmap.swap(emptyDramRunBitmap);

    // numCacheRuns = 0;
    // numDramRuns = 0;
}

int SSD::outputSpillState(const char *outputTXT)
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
    outputFile << "STATE -> SPILL_RUNS_SSD: Spill sorted runs to the SSD device\n";

    // close file
    outputFile.close();
}

int SSD::outputAccessState(const char *accessType,
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
    outputFile << "ACCESS -> A " << accessType << " to SSD was made with size " << totalBytes << " bytes and latency " << duration.count() << " us\n";

    // close file
    outputFile.close();
}

int SSD::outputMergeMsg(const char *outputTXT)
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
    outputFile << "STATE -> MERGE_RUNS_SSD: Merge sorted runs on the SSD device\n";

    // close file
    outputFile.close();
}

// Method to simulate read operation
void SSD::readData(unsigned long long sizeInBytes)
{
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(latency)));

    std::cout << "Read " << sizeInBytes << " bytes from SSD. Transfer time: " << transferTime << " seconds\n";
}

// Method to simulate write operation
void SSD::writeData(unsigned long long sizeInBytes)
{
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    double totalLatency = latency + transferTime;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(totalLatency)));
}

void SSD::print() const
{
    printf("\n------------------------------------SSD Data------------------------------------\n");
    printf("MAX Capacity %llu bytes, remaining cap %llu bytes %d\n", MAX_CAPACITY, capacity);
    printf("In total has %d runs\n", runs.size());
    for (int i = 0; i < runs.size(); i++)
    {
        printf(">>>>>>>>>>>>>>>>>>> %d th Run, valid: %d >>>>>>>>>>>>>>>>>>\n", i, static_cast<int>(runBitmap[i]));
        runs[i]->print();
    }
    printf("--------------------------------------------------------------------\n");
}

bool SSD::isFull() const
{
    return capacity == MAX_CAPACITY;
}

unsigned long long SSD::getCapacity() const
{
    return capacity;
}

long SSD::getLatency() const
{
    return latency;
}

Run *SSD::getRunCopy(int k) const
{
    if (k < 0 || k >= runs.size() || runBitmap[k] == false)
    {
        printf("Invalid index k\n");
        return nullptr;
    }
    return runs[k]->clone();
}

Run *SSD::getRun(int k) const
{
    if (k < 0 || k >= runs.size() || runBitmap[k] == false)
    {
        printf("Invalid index k\n");
        // print();
        return nullptr;
    }
    return runs[k];
}

bool SSD::runIsValid(int idx) const
{
    if (idx < 0 || idx >= runBitmap.size())
    {
        return false;
    }

    return runBitmap[idx];
}

int SSD::getNumRuns() const
{
    return numRuns;
}

// Physically Remove invalid runs from SSD
void SSD::cleanInvalidRuns()
{
    std::vector<Run *> cleanedRuns;
    std::vector<bool> cleanedBitmap;
    for (int i = 0; i < runs.size(); i++)
    {
        if (runBitmap[i])
        {
            cleanedRuns.push_back(runs[i]->clone());
            cleanedBitmap.push_back(true);
        }
    }
    numRuns = cleanedRuns.size();
    runs.swap(cleanedRuns);
    runBitmap.swap(cleanedBitmap);
}

long SSD::getBandwidth() const
{
    return bandwidth;
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
