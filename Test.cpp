#include "Record.h"
#include "HeapSort.h"
#include "CACHE.h"
#include "Run.h"
#include "DRAM.h"
#include "Disk.h"
#include "Scan.h"

#include <limits>
#include <cstdlib> // For atoi function
#include <iostream>
#include <chrono>
#include <filesystem>
// #include <dirent.h>
#include <vector>
#include <getopt.h>
#include <fstream>
#include <cmath>

// Set global variable

// Actual params
// unsigned long long CACHE_SIZE = 1ULL * 1024 * 1024;		  // 1 MB
// unsigned long long DRAM_SIZE = 100ULL * 1024 * 1024;	  // 100MB
// unsigned long long SSD_SIZE = 10ULL * 1024 * 1024 * 1024; // 10 GB
// int PAGE_SIZE = 8192;									  // 8 KB
// char *INPUT_TXT = "input_50mb_51200_1024.txt";

// >>>>>> Mini test case 1
unsigned long long CACHE_SIZE = 200;
unsigned long long DRAM_SIZE = 10200;
unsigned long long SSD_SIZE = 40800;
int PAGE_SIZE = 100;
char *INPUT_TXT = "intput_test_gd_1005_20.txt";
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
char *OUTPUT_TABLE = "output_table_20010B_gd";
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

std::vector<Page *> graceFulDegradation(Run *uniqueRecordsInPages, DRAM *dram, Disk *ssd)
{
	// Check if there's enough space in DRAM for the last pass
	Run *runToSpill = new Run();

	// Not enough space in DRAM; we need to spill to SSD.
	int i = 0;
	unsigned long long bytesToSpill = 0;
	while (dram->getCapacity() < uniqueRecordsInPages->getBytes())
	{
		// Page *pageToSpill = dram->getFirstPage();
		Page *pageToSpill = dram->getPageCopy(i);
		bytesToSpill += pageToSpill->getBytes();
		runToSpill->appendPage(pageToSpill);
		dram->erasePage(i); // only flip bit map
		i++;
	}
	// physically remove invalid/evicted pages from memory
	dram->cleanInvalidPagesinInputBuffer();
	// Spill the records from last second run into SSD
	ssd->outputSpillState(outputTXT);
	ssd->outputAccessState(ACCESS_WRITE, bytesToSpill, outputTXT);
	ssd->addRun(runToSpill); // Add the run to SSD

	// Load/read the rest of input data into dram
	// At this point, DRAM should have enough memory to hold the last few data
	while (!uniqueRecordsInPages->isEmpty())
	{
		dram->addPage(uniqueRecordsInPages->getFirstPage()->clone());
		uniqueRecordsInPages->removeFisrtPage();
	}
	// Get ready for creating cach-sized mini runs
	// Read the spilled data into Cache
	ssd->outputAccessState(ACCESS_READ, bytesToSpill, outputTXT);
	std::vector<Page *> pagesReadInCache;
	Run *spilledData = ssd->getRunCopy(0);
	ssd->eraseRun(0);
	ssd->cleanInvalidRuns();

	while (spilledData->getNumPages() > 0)
	{
		pagesReadInCache.push_back(spilledData->getFirstPage()->clone());
		spilledData->removeFisrtPage();
	}
	// since the spill threshold is 0.1%, the number of pages spilled is guaranteed
	// could fit in Cache
	return pagesReadInCache;
}

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
	// int nOutputBuffers = 16; // 16 * PAGE_SIZE= 128 KB
	int nOutputBuffers = 2;

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
	for (int pass = 0; pass < passes && !uniqueRecordsInPages->isEmpty(); pass++) // I/M
	{
		// if (pass == passes - 1)
		// {
		// 	// last run in current merge pass
		// 	// TODO: consider Graceful degradation
		// 	printf("TODO: consider Graceful degradation???\n");
		// 	handleLastMergePass(uniqueRecordsInPages, &dram, &ssd, &cache);
		// }

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

		std::vector<Run *> sortedMiniRuns;
		// Check whether to do GD before sorting here
		//  1. if only oversize a little be,
		//  2. store few pages from DRAM into SSD,
		//  3. Then empty these pages, and read the new slightly over-sized data into cache
		//  4. Then sorting them altogether
		// When it's second from last pass, look ahead whether we need to do gracefull degradation
		double ratio = (double)uniqueRecordsInPages->getNumPages() / pagesToRead;

		if (pass == passes - 2 && ratio <= 0.01 && !uniqueRecordsInPages->isEmpty())
		{
			// after degradation, pages are in cache, and in dram
			std::vector<Page *> pagesInCache = graceFulDegradation(uniqueRecordsInPages, &dram, &ssd);
			// Create sorted cache-sized mini runs
			sortedMiniRuns = cache.sortForGracefulDegradation(dram.getInputBuffers(), pagesInCache, maxRecordsInPage, PAGE_SIZE);
		}
		else
		{
			// Get unmerged cache-sized mini-runs
			sortedMiniRuns = cache.sort(dram.getInputBuffers(), maxRecordsInPage, PAGE_SIZE);
		}
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
		// hdd.print();
		printf("bytes in hdd: %lu\n", hdd.getRun(0)->getBytes());
		hdd.writeOutputTable(OUTPUT_TABLE);
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
