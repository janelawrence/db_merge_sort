#include "Record.h"
#include "TreeOfLosers.h"
#include "CACHE.h"
#include "Run.h"
#include "DRAM.h"
#include "Disk.h"
#include "TournamentTree.h"

#include <limits>
#include <cstdlib> // For atoi function
#include <iostream>
#include <chrono>
#include <filesystem>
#include <dirent.h>
#include <vector>
#include <getopt.h>
#include <fstream>
#include <cmath>

// Set global variable

// Actual params
unsigned long long CACHE_SIZE = 1ULL * 1024 * 1024;		  // 1 MB
unsigned long long DRAM_SIZE = 100ULL * 1024 * 1024;	  // 100MB
unsigned long long SSD_SIZE = 10ULL * 1024 * 1024 * 1024; // 10 GB
int PAGE_SIZE = 10240;									  // 10 KB
char *INPUT_TXT = "input_table";

// >>>>>> Mini test case 1
// unsigned long long CACHE_SIZE = 200;
// unsigned long long DRAM_SIZE = 1000;
// unsigned long long SSD_SIZE = 3000;
// int PAGE_SIZE = 100;
// char *INPUT_TXT = "mini_200_20_input.txt";
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
char *OUTPUT_TABLE = "output_table";

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

	// Calculate stats
	size_t totalBytes = numRecords * recordSize;

	// Number of mem-sized run created in DRAM
	int mergeLevels = static_cast<int>(std::ceil(static_cast<double>(totalBytes) / DRAM_SIZE));

	int maxRecordsInPage = PAGE_SIZE / recordSize;
	int nPagesFitInCache = CACHE_SIZE / PAGE_SIZE;

	int maxTreeSize = CACHE_SIZE / recordSize;

	int nBuffersDRAM = DRAM_SIZE / PAGE_SIZE;
	int nOutputBuffers = 2;
	// Fan-in F during merging
	int nInputBuffersDRAM = nBuffersDRAM - nOutputBuffers; // reserve 2 page according for output

	int nBuffersSSD = SSD_SIZE / PAGE_SIZE;

	printStats(numRecords, recordSize, maxRecordsInPage, nPagesFitInCache, nBuffersDRAM,
			   nOutputBuffers, nInputBuffersDRAM, nBuffersSSD, mergeLevels);

	CACHE cache(CACHE_SIZE, nPagesFitInCache);
	DRAM dram(DRAM_SIZE);
	Disk ssd(SSD_SIZE, SSD_LAT, SSD_BAN, SSD, nOutputBuffers);
	Disk hdd(HDD_SIZE, HDD_LAT, HDD_BAN, HDD, 0);
	// prepare all records stored in pages

	Run *allPages = dram.readRecords(INPUT_TXT, recordSize, numRecords,
									 totalBytes, nBuffersDRAM, maxRecordsInPage);
	printf("%d pages read \n", allPages->getNumPages());

	// Create all cache-sized mini-runs
	std::vector<Run *> allSortedMiniRuns = cache.sort(allPages, maxRecordsInPage, PAGE_SIZE);
	int start = 0;

	int total_bytes_merged = 0;
	for (int i = 0; i < allSortedMiniRuns.size(); i++)
	{
		total_bytes_merged += allSortedMiniRuns[i]->getBytes();
	}

	printf("----- Input %d pages, Total bytes stored in %d allSortedMiniRuns: %d-----------------\n\n",
		   allPages->getNumPages(), allSortedMiniRuns.size(), total_bytes_merged);
	for (int level = 0; level < mergeLevels; level++)
	{
		if (level == mergeLevels - 1)
		{
			// last run in current merge pass
			// TODO: consider Graceful degradation
			printf("TODO: consider Graceful degradation???\n");
		}

		// Get unmerged cache-sized mini-runs
		std::vector<Run *> sortedMiniRuns;
		unsigned long long bytesRead = 0;

		// Simulate reading data in trunks based on DRAM size [2pts]
		int nRuns = static_cast<int>(std::ceil(nBuffersDRAM / nPagesFitInCache));
		unsigned long long bytesWriteToDisk = 0;
		for (int i = 0; start < allSortedMiniRuns.size() && i < nRuns; start++, i++)
		{
			sortedMiniRuns.push_back(allSortedMiniRuns[start]->clone());
			bytesWriteToDisk += sortedMiniRuns[i]->getBytes();
		}
		// trace sorting mini runs state
		cache.outputMiniRunState(outputTXT);

		// create memory-sized sorted runs using Tournament Tree
		printf("At merge level %d, created %d cache-sized runs\n", level, sortedMiniRuns.size());

		// Use Tournament Tree here
		TournamentTree *loserTree = new TournamentTree(sortedMiniRuns.size(), sortedMiniRuns);
		Run *memSizedRun = new Run();
		while (loserTree->hasNext())
		{
			cout << "The new winner is: Contestant ";
			Record *winner = loserTree->popWinner();
			winner->printRecord();
			if (winner == nullptr)
			{
				break;
			}
			memSizedRun->addRecord(winner);
		}
		// memSizedRun->print();
		if (ssd.getCapacity() < bytesWriteToDisk)
		{
			ssd.outputMergeMsg(outputTXT);
			ssd.mergeFromSelfToDest(&hdd, outputTXT);
		}

		if (ssd.getCapacity() >= bytesWriteToDisk)
		{
			// Use Alternative 1
			ssd.outputSpillState(outputTXT);
			ssd.outputAccessState(ACCESS_WRITE, bytesWriteToDisk, outputTXT);

			ssd.addRun(memSizedRun);
		}
	}
	if (ssd.getNumUnsortedRuns() > 0)
	{
		ssd.mergeFromSelfToDest(&hdd, outputTXT);
	}
	ssd.print();
	hdd.print();

	if (hdd.getNumUnsortedRuns() == 1)
	{
		hdd.outputReadSortedRunState(outputTXT);
		hdd.outputAccessState(ACCESS_WRITE, totalBytes, outputTXT);
		hdd.writeOutputTable(OUTPUT_TABLE);
	}
	else
	{
		// Read Page from hdd to ssd, then merge???
		printf("TODO\n");
	}

	return 0;
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
