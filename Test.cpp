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
// Mini test 1 Set up End <<<<<<<<<<

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

/* Function to read records from the input file
	Return: a run * of pages, this run has size same as DRAM size
*/
Run *readRecords(const char *fileName, int recordSize, int numRecords, int totalBytes,
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

void printStats(int numRecords, int recordSize, int maxRecordsInPage,
				int nPagesFitInCache, int nBuffersDRAM, int nBuffersReserved,
				int nInputBuffersDRAM, int nBuffersSSD, int mergeLevels)
{
	Record *recordPtr;
	printf("--------------------------------------------System stats--------------------------------------\n");
	printf("Number of records: %d, "
		   "Record size: %d\n\n",
		   numRecords, recordSize);
	printf("Page size: %d Bytes\n"
		   "- Each page can store %d records\n\n",
		   PAGE_SIZE, maxRecordsInPage);
	printf("Cache size: %llu Bytes\n"
		   "- Each cache-sized run can store at most %d pages\n"
		   "- Each cache-sized run can store at most %d records\n"
		   "- Cache can store at most %d Record Pointer\n\n",
		   CACHE_SIZE, nPagesFitInCache,
		   nPagesFitInCache * maxRecordsInPage,
		   CACHE_SIZE / recordSize);
	printf("DRAM size: %llu Bytes\n"
		   "- Each DRAM-sized run can store %d pages\n"
		   "---- %d are input buffers, %d are reserved buffers\n"
		   "- Each DRAM-sized run can store %d records\n\n",
		   DRAM_SIZE, nBuffersDRAM,
		   nInputBuffersDRAM, nBuffersReserved,
		   nInputBuffersDRAM * maxRecordsInPage);
	printf("Merge Levels : %d\n\n",
		   mergeLevels);
	printf("SSD size: %llu Bytes\n"
		   "- Each SSD-sized run can store %d pages\n"
		   "- Each SSD-sized run can store %d records\n\n",
		   SSD_SIZE, nBuffersSSD,
		   nBuffersSSD * maxRecordsInPage);
}
/**
 *
 * Program entrypoint
 */
int main(int argc, char *argv[])
{
	int c;
	const char *outputTXT = nullptr;

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

	// prepare all records stored in pages

	Run *allPages = readRecords(INPUT_TXT, recordSize, numRecords,
								totalBytes, nBuffersDRAM, maxRecordsInPage);
	printf("%d pages read \n", allPages->getNumPages());

	CACHE cache(CACHE_SIZE, nPagesFitInCache);
	DRAM dram(DRAM_SIZE);
	Disk ssd(SSD_SIZE, SSD_LAT, SSD_BAN, SSD, nOutputBuffers);
	Disk hdd(HDD_SIZE, HDD_LAT, HDD_BAN, HDD, 0);

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
			printf("TODO: consider Graceful degradation\n");
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
		hdd.writeOutputTable(OUTPUT_TABLE);
	}
	else
	{
		// Read Page from hdd to ssd, then merge???
		printf("TODO\n");
	}

	return 0;
	// ssd.moveAllTempToUnsorted();
	// hdd.moveAllTempToUnsorted();

	// int numMemSizedRunOnSSD = ssd.getNumUnsortedRuns();
	// int numMemSizedRunOnHDD = hdd.getNumUnsortedRuns();

	// if (numMemSizedRunOnSSD == 1 && numMemSizedRunOnHDD == 0)
	// {
	// 	hdd.outputSpillState(outputTXT);
	// 	hdd.outputAccessState(ACCESS_WRITE, totalBytes, outputTXT);
	// 	printf("line 412, Write to a output_table\n");
	// 	ssd.writeOutputTable(OUTPUT_TABLE);
	// 	return 0;
	// }
	// else
	// {
	// 	// merge all runs on SSD and write to HDD
	// 	while (ssd.getNumUnsortedRuns() > 0)
	// 	{
	// 		ssd.outputMergeMsg(outputTXT);
	// 		// pages to read from SSD
	// 		int pagesToRead = std::min(maxTreeSize, nInputBuffersDRAM);
	// 		pagesToRead = std::min(pagesToRead, ssd.getNumUnsortedRuns());

	// 		ssd.outputAccessState(ACCESS_READ, pagesToRead * PAGE_SIZE, outputTXT);
	// 		printf("Physically reading %d pages\n", pagesToRead);
	// 		// Physically adding pages into DRAM
	// 		for (int i = 0; i < pagesToRead; i++)
	// 		{
	// 			Run *runOnSSD = ssd.getRun(i);
	// 			if (!runOnSSD->isEmpty())
	// 			{
	// 				Page *pageFetched = runOnSSD->getFirstPage()->clone();
	// 				pageFetched->setSource(FROM_SSD);
	// 				pageFetched->setIdx(i);
	// 				dram.addPage(pageFetched);
	// 				ssd.delFirstPageFromRunK(i);
	// 				// runOnSSD->removeFisrtPage();
	// 				// ssd.setCapacity(ssd.getCapacity() + pageFetched->getBytes());

	// 				if (runOnSSD->isEmpty())
	// 				{
	// 					ssd.eraseRun(i);
	// 				}
	// 			}
	// 			else
	// 			{
	// 				ssd.eraseRun(i);
	// 			}
	// 		}
	// 		ssd.cleanInvalidRuns();
	// 		dram.mergeFromSrcToDest(&ssd, &hdd, maxTreeSize, outputTXT);
	// 		ssd.cleanInvalidRuns();
	// 	}

	// 	// merge all runs on HDD and write to HDD
	// 	while (hdd.getNumUnsortedRuns() > 1)
	// 	{
	// 		hdd.outputMergeMsg(outputTXT);
	// 		// pages to read from HDD
	// 		int pagesToRead = std::min(maxTreeSize, nInputBuffersDRAM);
	// 		pagesToRead = std::min(pagesToRead, hdd.getNumUnsortedRuns());

	// 		hdd.outputAccessState(ACCESS_READ, pagesToRead * PAGE_SIZE, outputTXT);
	// 		printf("Physically reading %d pages\n", pagesToRead);
	// 		// Physically adding pages into DRAM
	// 		for (int i = 0; i < pagesToRead; i++)
	// 		{
	// 			Run *runOnHDD = hdd.getRun(i);
	// 			if (!runOnHDD->isEmpty())
	// 			{
	// 				Page *pageFetched = runOnHDD->getFirstPage()->clone();
	// 				pageFetched->setSource(FROM_HDD);
	// 				pageFetched->setIdx(i);
	// 				dram.addPage(pageFetched);
	// 				hdd.delFirstPageFromRunK(i);

	// 				if (runOnHDD->isEmpty())
	// 				{
	// 					hdd.eraseRun(i);
	// 				}
	// 			}
	// 			else
	// 			{
	// 				hdd.eraseRun(i);
	// 			}
	// 		}
	// 		hdd.cleanInvalidRuns();
	// 		dram.mergeFromSrcToDest(&hdd, &hdd, maxTreeSize, outputTXT);
	// 		hdd.cleanInvalidRuns();
	// 	}

	// 	if (hdd.getNumUnsortedRuns() == 1)
	// 	{
	// 		hdd.print();
	// 		hdd.outputSpillState(outputTXT);
	// 		hdd.outputAccessState(ACCESS_WRITE, hdd.getRun(0)->getBytes(), outputTXT);
	// 		hdd.writeOutputTable(OUTPUT_TABLE);
	// 	}
	// }

	//  delete dynamically allocated memory before exit program
	return 0;
}
