#include "Disk.h"
#include "Record.h"
#include "defs.h"
#include "Disk.h"
#include "DRAM.h"

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <ctime>      // For localtime and strftime
#include <sstream>    // For stringstream
#include <iomanip>    // For setfill and setw
#include <sys/stat.h> // For mkdir function
#include <cstdlib>    // For atoi function
#include <filesystem>
#include <dirent.h>
#include <vector>

// Constructor
// size in B
// bandiwith in MB/us
HDD::HDD(double lat, double bw) : latency(lat), bandwidth(bw) {}

/*
Write cloned run to HDD
*/
void HDD::addRun(Run *run)
{

    runs.push_back(run->clone());

    // Decrease HDD capacity
    runBitmap.push_back(true);
    numRuns++;
}

bool HDD::eraseRun(int runIdx)
{
    if (runIdx > runs.size())
    {
        printf("run index invalid");
        return false;
    }
    runBitmap[runIdx] = false;
    numRuns--;
    return true;
}

int HDD::outputSpillState(const char *outputTXT)
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
    outputFile << "STATE -> SPILL_RUNS_HDD: Spill sorted runs to the HDD device\n";

    // close file
    outputFile.close();
}

int HDD::outputAccessState(const char *accessType,
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
    outputFile << "ACCESS -> A " << accessType << " to HDD was made with size " << totalBytes << " bytes and latency " << duration.count() << " us\n";

    // close file
    outputFile.close();
}

// Method to write operation
// size in B
// bandiwith in B/us
// Method to simulate write operation
void HDD::writeData(unsigned long long sizeInBytes)
{
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    double totalLatency = latency + transferTime;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(totalLatency)));
}

// Method to simulate read operation
void HDD::readData(unsigned long long sizeInBytes)
{
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(latency)));

    std::cout << "Read " << sizeInBytes << " bytes from HDD. Transfer time: " << transferTime << " seconds\n";
}

int HDD::outputMergeMsg(const char *outputTXT)
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
    outputFile << "STATE -> MERGE_RUNS_HDD: Merge sorted runs on the HDD device\n";

    // close file
    outputFile.close();
}

void HDD::print() const
{
    printf("\n------------------------------------HDD Data------------------------------------\n");
    printf("In total has %d runs\n", runs.size());
    for (int i = 0; i < runs.size(); i++)
    {
        printf(">>>>>>>>>>>>>>>>>>> %d th Run, valid: %d >>>>>>>>>>>>>>>>>>\n", i, static_cast<int>(runBitmap[i]));
        runs[i]->print();
    }
    printf("--------------------------------------------------------------------\n");
}

long HDD::getLatency() const
{
    return latency;
}

long HDD::getBandwidth() const
{
    return bandwidth;
}

Run *HDD::getRunCopy(int k) const
{
    if (k < 0 || k >= runs.size() || runBitmap[k] == false)
    {
        printf("Invalid index k\n");
        return nullptr;
    }
    return runs[k]->clone();
}

Run *HDD::getRun(int k) const
{
    if (k < 0 || k >= runs.size() || runBitmap[k] == false)
    {
        printf("Invalid index k\n");
        return nullptr;
    }
    return runs[k];
}

bool HDD::runIsValid(int idx) const
{
    if (idx < 0 || idx >= runBitmap.size())
    {
        return false;
    }

    return runBitmap[idx];
}

int HDD::getNumRuns() const
{
    return numRuns;
}

// Physically Remove invalid runs from HDD
void HDD::cleanInvalidRuns()
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