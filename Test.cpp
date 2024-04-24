#include "Record.h"
#include "HeapSort.h"
#include "CACHE.h"
#include "Run.h"
#include "DRAM.h"
#include "Disk.h"
#include "Scan.h"
#include "getopt.h"

#include <limits>
#include <cstdlib> // For atoi function
#include <iostream>
#include <chrono>
#include <filesystem>
// #include <dirent.h>
#include <vector>
// #include <getopt.h>
#include <fstream>
#include <cmath>

// Set global variable

// Actual params
unsigned long long CACHE_SIZE = 1ULL * 1024 * 1024;		  // 1 MB
unsigned long long DRAM_SIZE = 100ULL * 1024 * 1024;	  // 100MB
unsigned long long SSD_SIZE = 10ULL * 1024 * 1024 * 1024; // 10 GB
int PAGE_SIZE = 8192;									  // 8 KB
char *INPUT_TXT = "input_50mb_51200_1024.txt";

// >>>>>> Mini test case 1
// unsigned long long CACHE_SIZE = 200;
// unsigned long long DRAM_SIZE = 1000;
// unsigned long long SSD_SIZE = 3000;
// int PAGE_SIZE = 100;
// char *INPUT_TXT = "mini_200_20_dup_input.txt";
// Mini test 1 Set up End < < < < < < < < < <

// >>>>>> mini test case 2
// unsigned long long CACHE_SIZE = 1ULL * 1024 * 1024;		  // 1 MB
// unsigned long long DRAM_SIZE = 100ULL * 1024 * 1024;	  // 100MB
// unsigned long long SSD_SIZE = 10ULL * 1024 * 1024 * 1024; // 10 GB
// int PAGE_SIZE = 10240;									  // 10KB								  // 10 KB
// char *INPUT_TXT = "medium_200_1024_input.txt";
// Mini test 2 Set up End <<<<<<<<<<

// >>>>>> test case 0
// unsigned long long CACHE_SIZE = 1ULL * 1024 * 1024;		  // 1 MB
// unsigned long long DRAM_SIZE = 100ULL * 1024 * 1024;	  // 100MB
// unsigned long long SSD_SIZE = 10ULL * 1024 * 1024 * 1024; // 10 GB
// int PAGE_SIZE = 10240;									  // 10KB								  // 10 KB
// char *INPUT_TXT = "test0_10240_10MB_1024_input.txt";
// test 0 Set up End <<<<<<<<<<

// >>>>>> test case 1
// unsigned long long CACHE_SIZE = 1ULL * 1024 * 1024;		  // 1 MB
// unsigned long long DRAM_SIZE = 100ULL * 1024 * 1024;	  // 100MB
// unsigned long long SSD_SIZE = 10ULL * 1024 * 1024 * 1024; // 10 GB
// int PAGE_SIZE = 10240;									  // 10KB								  // 10 KB
// char *INPUT_TXT = "test1_51200_50MB_1024_input.txt";
// test 1 Set up End <<<<<<<<<<

unsigned long long HDD_SIZE = std::numeric_limits<unsigned long long>::max();
char *OUTPUT_TABLE = "output_table_50mb_1024";
// char *OUTPUT_TABLE = "output_table_125mb_1024_new";

long SSD_LAT = 100;											 // 0.1 ms = 100 microseconds(us)
unsigned long long SSD_BAN = 200ULL * 1024 * 1024 / 1000000; // 200 MB/s = 200 MB/us

long HDD_LAT = 500;										  // 5 ms = 0.005 s
unsigned long long HDD_BAN = 100 * 1024 * 1024 / 1000000; // 100 MB/s = 100 MB/us

int recordSize = 0; // initialized
int numRecords = 0; // initialized

const char *HDD = "HDD";
const char *SSD = "SSD";

const char *ACCESS_WRITE = "write";
const char *ACCESS_READ = "read";

const char *outputTXT = nullptr;

int mergeSort()
{
	ScanPlan sp(recordSize);
	// prepare all records stored in pages
	// Filter out duplicate records and store in pages
	Run *uniqueRecordsInPages = sp.scan(INPUT_TXT);

	// Calculate stats
	size_t totalBytes = numRecords * recordSize;

	int maxRecordsInPage = PAGE_SIZE / recordSize;
	int nPagesFitInCache = CACHE_SIZE / PAGE_SIZE;

	int maxTreeSize = CACHE_SIZE / recordSize;

	int nBuffersDRAM = DRAM_SIZE / PAGE_SIZE;
	//  Should be enough to hold the tree of fan - in size
	int nOutputBuffers = 16; // 16 * PAGE_SIZE= 128 KB
	// int nOutputBuffers = 2;

	int nInputBuffersDRAM = nBuffersDRAM - nOutputBuffers; // reserve pages as output buffers

	float outputBufferRatioSSD = 0.8;
	int nBuffersSSD = SSD_SIZE / PAGE_SIZE;

	int nOutputBuffersSSD = nBuffersSSD * outputBufferRatioSSD; //????

	// Number of mem-sized run will be created in DRAM
	int passes = static_cast<int>(std::ceil(static_cast<double>(uniqueRecordsInPages->getNumPages()) / nInputBuffersDRAM));

	printStats(numRecords, recordSize, maxRecordsInPage,
			   nPagesFitInCache, nBuffersDRAM, nOutputBuffers,
			   nInputBuffersDRAM, nBuffersSSD, nOutputBuffersSSD, passes);

	CACHE cache(CACHE_SIZE, nPagesFitInCache);
	DRAM dram(DRAM_SIZE, nOutputBuffers);
	Disk ssd(SSD_SIZE, SSD_LAT, SSD_BAN, SSD, nOutputBuffersSSD);
	Disk hdd(HDD_SIZE, HDD_LAT, HDD_BAN, HDD, 0);

	int totalBytesUnique = uniqueRecordsInPages->getBytes();

	printf("%d pages read \n", uniqueRecordsInPages->getNumPages());

	printf("----- Input %d pages, Total bytes stored %d-----------------\n\n",
		   uniqueRecordsInPages->getNumPages(), totalBytesUnique);
	for (int pass = 0; pass < passes; pass++) // I/M
	{
		if (pass == passes - 1)
		{
			// last run in current merge pass
			// TODO: consider Graceful degradation
			printf("TODO: consider Graceful degradation???\n");
			handleLastMergePass(uniqueRecordsInPages, &dram, &ssd, &cache);
		}

		//  Read pages into DRAM
		int pagesToRead = std::min(nInputBuffersDRAM, uniqueRecordsInPages->getNumPages());
		unsigned long long bytesRead = 0;
		for (int i = 0; i < pagesToRead; i++)
		{
			Page *nextPage = uniqueRecordsInPages->getFirstPage()->clone();
			if (dram.getCapacity() >= nextPage->getSize())
			{
				dram.addPage(nextPage);
				uniqueRecordsInPages->removeFisrtPage();
				bytesRead += nextPage->getBytes();
			}
			else
			{
				break;
			}
		}
		// TODO: Check whether to do GD before sorting here
		//  1. if only oversize a little be,
		//  2. store few pages from DRAM into cache,
		//  3. Then empty these pages, and read in the new slightly over-sized data
		//  4. Then sorting them altogether
		if (dram.isFull() && !uniqueRecordsInPages->isEmpty()){
				performGracefulDegradation(&dram, &cache, &ssd);
	    }

		// Get unmerged cache-sized mini-runs
		std::vector<Run *> sortedMiniRuns = cache.sort(dram.getInputBuffers(), maxRecordsInPage, PAGE_SIZE);

		// trace sorting mini runs state
		cache.outputMiniRunState(outputTXT);

		// created memory-sized sorted runs using Tournament Tree
		printf("At scanning pass %d, created %d cache-sized runs\n", pass, sortedMiniRuns.size());

		// if ssd have enough output buffer space to store the next mem-sized run
		if (ssd.getOutputBufferCapacity() >= bytesRead)
		{
			// memory-sized run are created by merging in memory,
			// and saved to SSD
			dram.mergeFromSelfToDest(&ssd, outputTXT, sortedMiniRuns);
		}
		else
		{
			// In alternative 1, all runs on SSD are of memory-sized
			// write runs on SSD to HDD, then clear SSD
			hdd.outputSpillState(outputTXT);
			// write all runs to HDD, and then clear space
			hdd.outputSpillState(outputTXT);
			hdd.outputAccessState(ACCESS_WRITE, ssd.outputBuffers.getBytes(), outputTXT);
			for (int i = 0; i < ssd.getNumRunsInOutputBuffer(); i++)
			{
				Run *r = ssd.outputBuffers.runs[i]->clone();
				hdd.addRun(r);
			}
			ssd.outputBuffers.clear();
		}

		dram.clear();
	}
	if (ssd.getNumRunsInOutputBuffer() > 0)
	{
		hdd.outputSpillState(outputTXT);
		hdd.outputAccessState(ACCESS_WRITE, ssd.outputBuffers.getBytes(), outputTXT);
		for (int i = 0; i < ssd.getNumRunsInOutputBuffer(); i++)
		{
			Run *r = ssd.outputBuffers.runs[i]->clone();
			hdd.addRun(r);
		}
		ssd.outputBuffers.clear();
	}

	if (hdd.getNumUnsortedRuns() == 1)
	{
		printf("No merging needs to happen in HDD\n");
		hdd.outputReadSortedRunState(outputTXT);
		hdd.outputAccessState(ACCESS_WRITE, totalBytesUnique, outputTXT);
		hdd.writeOutputTable(OUTPUT_TABLE);
	}
	else
	{
		printf("merging on HDD starts\n");
		// Start merging mem-sized runs on HDD
		hdd.mergeFromSelfToSelf(outputTXT);
		hdd.outputAccessState(ACCESS_WRITE, totalBytesUnique, outputTXT);
		hdd.print();
		hdd.writeOutputTable(OUTPUT_TABLE);
	}

	return 0;
}
void handleLastMergePass(Run* uniqueRecordsInPages, DRAM* dram, Disk* ssd, CACHE* cache) {
    // Check if there's enough space in DRAM for the last pass
    if (dram->getCapacity() < uniqueRecordsInPages->getBytes()) {
        // Not enough space in DRAM; we need to spill to SSD.
        while (uniqueRecordsInPages->getBytes() > dram->getCapacity()) {
            Page* pageToSpill = uniqueRecordsInPages->getFirstPage();
            uniqueRecordsInPages->removeFisrtPage(); // Assume this reduces the bytes count of uniqueRecordsInPages
            Run* runToSpill = new Run();
            runToSpill->appendPage(pageToSpill);
            ssd->addRun(runToSpill); // Add the run to SSD
        }
    }
    
    // Load the last of the records into DRAM and perform the merge
    // Assume DRAM has a method to take a Run and perform merge.
    dram->mergeRun(uniqueRecordsInPages);
    // If needed, also consider any data that might be in the cache.
    if (!cache->isEmpty()) {
        // Assume a method to merge data from cache to DRAM exists.
        cache->mergeIntoDRAM(dram);
    }
}


void performGracefulDegradation(DRAM* dram, CACHE* cache, Disk* ssd) {
    // Check if DRAM is full and spill to SSD as needed
    while (dram->isFull()) {
        // Assuming DRAM class has a method to select a Run to spill
        Run* runToSpill = dram->selectRunToSpill(); // Method needs to be implemented in DRAM
        ssd->addRun(runToSpill); // Add the run to SSD
        // Assuming DRAM class has a method to remove a Run
        dram->removeRun(runToSpill); // Method needs to be implemented in DRAM
    }

    // Now DRAM has space, load the oversized data into DRAM for processing
    // Assume a method in DRAM to load Runs from some source and sort them
    dram->loadAndSortOversizedData(); // Method needs to be implemented in DRAM
    
    // Merge the data in DRAM
    // Assume DRAM class has a merge method
    dram->merge();
    
    // If cache was used, it needs to be merged too
    // Assume a method to merge data from cache exists
    cache->mergeIntoDRAM(dram);
}



// Verifying sort order [2]
bool verityOrder()
{
	std::ifstream file(OUTPUT_TABLE);
	char lastKey[9];
	char currentKey[9];

	int i = 0;
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			line.copy(currentKey, 8, 0);
			if (i > 0)
			{
				if (std::strcmp(lastKey, currentKey) >= 0)
				{
					return false;
				}
			}
			line.copy(lastKey, 8, 0);
			i++;
		}
		file.close();
	}
	if (i == 0)
	{
		printf("Output table is empty\n");
	}
	return true;
}

/**
 *
 * Program entrypoint
 */
int main(int argc, char *argv[])
{
	int c;

	// Parse command-line options
	while ((c = getopt(argc, argv, "c:s:o:")) != -1)
	{
		switch (c)
		{
		case 'c':
			numRecords = std::atoi(optarg);
			break;
		case 's':
			recordSize = std::atoi(optarg);
			break;
		case 'o':
			outputTXT = optarg;
			break;
		default:
			printf("Usage: %s -c <numRecords> -s <recordSize> -o <outputFileName>\n", argv[0]);
			return 1;
		}
	}
	// Check if all required options are provided
	if (recordSize == 0 || numRecords == 0 || outputTXT == nullptr)
	{
		printf("Usage: %s -c <numRecords> -s <recordSize> -o <outputFileName>\n", argv[0]);
		return 1;
	}

	mergeSort();
	if (verityOrder())
	{
		printf("Verified sorting order is correct\n");
	}
	else
	{
		printf("Wrong sorting");
	}

	return 0;
}
