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
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

// Set global variable

// Actual params
unsigned long long CACHE_SIZE = 1ULL * 1024 * 1024;		  // 1 MB
unsigned long long DRAM_SIZE = 100ULL * 1024 * 1024;	  // 100MB
unsigned long long SSD_SIZE = 10ULL * 1024 * 1024 * 1024; // 10 GB
int PAGE_SIZE = 8192;									  // 8 KB
char *INPUT_TXT = "input_120gb_125829120_1024.txt";
// char *INPUT_TXT = "input_125mb_128000_1024.txt";
// char *INPUT_TXT = "mini_200_20_input.txt";

// >>>>>> Mini test case 1
// unsigned long long CACHE_SIZE = 4 * 8192;
// unsigned long long DRAM_SIZE = 10 * 8192;
// unsigned long long SSD_SIZE = 20 * 8192;
// int PAGE_SIZE = 8192;
// char *INPUT_TXT = "input_50mb_51200_1024.txt";
// Mini test 1 Set up End < < < < < < < < < <

unsigned long long HDD_SIZE = std::numeric_limits<unsigned long long>::max();
// char *OUTPUT_TABLE = "output_table_10GB";
char *OUTPUT_TABLE = "output_table_test";

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

// local directories for storage
const char *LOCAL_INPUT_DIR = "input_pages";
const char *LOCAL_DRAM_SIZED_RUNS_DIR = "mem_sized_runs";
const char *LOCAL_SSD_SIZED_RUNS_DIR = "ssd_sized_runs";

const char *outputTXT = nullptr;

// TODO: change param uniqueRecordsInPages to page index range
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
		Page *pageToSpill = dram->getPage(i);
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
		uniqueRecordsInPages->removeFirstPage(0);
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
		pagesReadInCache.push_back(spilledData->getFirstPage());
		spilledData->removeFirstPage(0);
	}
	// since the spill threshold is 0.1%, the number of pages spilled is guaranteed
	// could fit in Cache
	return pagesReadInCache;
}

bool directoryExists(const char *path)
{
	struct stat info;
	if (stat(path, &info) != 0)
	{
		return false; // Cannot access the path
	}
	else if (info.st_mode & S_IFDIR)
	{
		return true; // It's a directory
	}
	else
	{
		return false; // It's a file, not a directory
	}
}

int removeCallback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	int rv = remove(fpath);
	if (rv)
		perror(fpath);

	return rv;
}

bool removeDirectoryRecursively(const std::string &dirPath)
{
	return nftw(dirPath.c_str(), removeCallback, 64, FTW_DEPTH | FTW_PHYS) == 0;
}

/**
 * Create a directory in current working directory
 */
int createDir(const char *dirName)
{

	// Check if the directory exists
	if (directoryExists(dirName))
	{
		removeDirectoryRecursively(dirName);
	}

	if (mkdir(dirName, 0755) == -1)
	{
		perror("Failed to create directory");
		return 1;
	}
	return 0;
}

/**
 * Remove a directory and its contents if dir exists
 * */
int removeDir(const char *dirName)
{

	// Attempt to remove the directory
	if (rmdir(dirName) == 0)
	{
		std::cout << "Directory successfully removed." << std::endl;
		return 0;
	}
	else
	{
		perror("Failed to remove directory");
		return 1;
	}

	return 0;
}

/**
 *
 * Program significant functions:
 * Permorm external- merge sort
 */
int mergeSort()
{
	ScanPlan *sp = new ScanPlan(recordSize);
	// prepare all records stored in pages
	if (createDir(LOCAL_INPUT_DIR) == 1)
	{
		// Return if not able to create LOCAL_INPUT_DIR
		return 1;
	}
	// Filter out duplicate records and store in pages
	// Run *uniqueRecordsInPages = sp.scan(INPUT_TXT, outputTXT);
	int totalPages = sp->pagingInput(INPUT_TXT, LOCAL_INPUT_DIR);

	delete sp;

	if (totalPages == 0)
	{
		printf("Error: only 0 pages are scanned\n");
		return 1;
	}

	printf("Total number of pages in input: %d\n", totalPages);

	if (createDir(LOCAL_DRAM_SIZED_RUNS_DIR) == 1)
	{
		// Return if not able to create LOCAL_INPUT_DIR
		return 1;
	}

	if (createDir(LOCAL_SSD_SIZED_RUNS_DIR) == 1)
	{
		// Return if not able to create LOCAL_INPUT_DIR
		return 1;
	}
	char separator = get_directory_separator();
	// Calculate stats
	size_t totalBytes = numRecords * recordSize;

	int maxRecordsInPage = PAGE_SIZE / recordSize;
	int nPagesFitInCache = CACHE_SIZE / PAGE_SIZE;

	int nBuffersDRAM = DRAM_SIZE / PAGE_SIZE;
	//  Should be enough to hold the tree of fan-in size
	int nOutputBuffers = 32; // 32 * PAGE_SIZE= 256 KB
	// int nOutputBuffers = 2;	 // used for testing when dram size is small

	int nInputBuffersDRAM = nBuffersDRAM - nOutputBuffers; // reserve pages as output buffers

	float outputBufferRatioSSD = 0.8;
	int nBuffersSSD = SSD_SIZE / PAGE_SIZE;

	int nOutputBuffersSSD = nBuffersSSD * outputBufferRatioSSD;

	// // Number of mem-sized run will be created in DRAM
	int readPasses = static_cast<int>(std::ceil(static_cast<double>(totalPages) / nInputBuffersDRAM));

	printStats(numRecords, recordSize, maxRecordsInPage,
			   nPagesFitInCache, nBuffersDRAM, nOutputBuffers,
			   nInputBuffersDRAM, nBuffersSSD, nOutputBuffersSSD, readPasses);

	CACHE cache(CACHE_SIZE, nPagesFitInCache);
	DRAM dram(DRAM_SIZE, nOutputBuffers);
	Disk ssd(SSD_SIZE, SSD_LAT, SSD_BAN, SSD, nOutputBuffersSSD);
	Disk hdd(HDD_SIZE, HDD_LAT, HDD_BAN, HDD, 0);

	printf("Total input pages %d \n", totalPages);

	int pagesLeftInInput = totalPages;
	int pagesOffset = 0;
	for (int readPass = 0; readPass < readPasses && pagesLeftInInput > 0; readPass++) // I/M
	{
		//  Read pages into DRAM
		int pagesToRead = std::min(nInputBuffersDRAM, pagesLeftInInput);
		unsigned long long bytesRead = dram.readRecords(LOCAL_INPUT_DIR, pagesOffset,
														pagesOffset + pagesToRead,
														recordSize);
		pagesLeftInInput -= pagesToRead;
		pagesOffset += pagesToRead;
		std::vector<Run *> sortedMiniRuns;
		// Check whether to do GD before sorting here
		//  1. if only oversize a little be,
		//  2. store few pages from DRAM into SSD,
		//  3. Then empty these pages, and read the new slightly over-sized data into cache
		//  4. Then sorting them altogether
		// When it's second from last pass, look ahead whether we need to do gracefull degradation
		double ratio = (double)pagesLeftInInput / pagesToRead;

		if (readPass == readPasses - 2 && ratio <= 0.01 && pagesLeftInInput != 0)
		{
			printf("Graceful Degradation\n");
			// after degradation, pages are in cache, and in dram
			// 		std::vector<Page *> pagesInCache = graceFulDegradation(uniqueRecordsInPages, &dram, &ssd);
			// Create sorted cache-sized mini runs
			// 		sortedMiniRuns = cache.sortForGracefulDegradation(dram.getInputBuffers(), pagesInCache, maxRecordsInPage, PAGE_SIZE);
		}
		else
		{
			// Get unmerged cache-sized mini-runs
			sortedMiniRuns = cache.sort(dram.getInputBuffers(), maxRecordsInPage, PAGE_SIZE);
		}
		// trace sorting mini runs state
		cache.outputMiniRunState(outputTXT);

		// created memory-sized sorted runs using Tournament Tree
		printf("At scanning pass %d, created %d cache-sized runs\n", readPass, sortedMiniRuns.size());
		if (ssd.getOutputBufferCapacity() >= bytesRead)
		{
			// use readPass as the new mem-sized run index
			dram.mergeFromSelfToDest(&ssd, outputTXT, sortedMiniRuns, readPass);
		}
		else
		{
			// In alternative 1, all runs on SSD are of memory-sized
			// write runs on SSD to HDD, then clear SSD
			// write all runs to HDD, and then clear space
			hdd.outputSpillState(outputTXT);
			hdd.outputAccessState(ACCESS_WRITE, ssd.outputBuffers.getBytes(), outputTXT);
			ssd.clearOuputBuffer();
			dram.mergeFromSelfToDest(&ssd, outputTXT, sortedMiniRuns, readPass);
		}
		dram.clear();
		for (int i = 0; i < sortedMiniRuns.size(); i++)
		{
			delete sortedMiniRuns[i];
		}
		sortedMiniRuns.clear();
	}
	// // // TODO: Consider clear LOCAL_INPUT_DIR physically
	int totalNumberMemorySizedRuns = countRunsInDirectory(std::string(LOCAL_DRAM_SIZED_RUNS_DIR));

	if (readPasses == 1)
	{
		if (totalNumberMemorySizedRuns == 1)
		{
			std::string runFolderPath = std::string(LOCAL_DRAM_SIZED_RUNS_DIR) + separator + "run0";
			hdd.writeRunToOutputTable(runFolderPath.c_str(), OUTPUT_TABLE);
			return 0;
		}
		else
		{
			printf("Have only read one pass, but have %d  memory-sized run\n", totalNumberMemorySizedRuns);
		}
	}

	// // If there are mem-sized runs left in SSD
	// //  Spill them to HDD before merging
	// // write runs on SSD to HDD, then clear SSD
	if (!ssd.outputBuffers.isEmpty())
	{
		hdd.outputSpillState(outputTXT);
		// Trace spilling all runs to HDD, and then clear space
		hdd.outputSpillState(outputTXT);
		hdd.outputAccessState(ACCESS_WRITE, ssd.outputBuffers.getBytes(), outputTXT);
		ssd.clearOuputBuffer();
	}

	// TODO: Consider clear LOCAL_INPUT_DIR physically

	totalNumberMemorySizedRuns = countRunsInDirectory(std::string(LOCAL_DRAM_SIZED_RUNS_DIR));

	// Start merging on HDD
	hdd.outputMergeMsg(outputTXT);
	hdd.mergeMemorySizedRuns(outputTXT, OUTPUT_TABLE);

	int totalNumberSSDSizedRuns = countRunsInDirectory(std::string(LOCAL_SSD_SIZED_RUNS_DIR));

	hdd.mergeSSDSizedRuns(outputTXT, OUTPUT_TABLE);

	return 0;
}

// Verifying sort order [2]
int verityOrder()
{
	std::ifstream file(OUTPUT_TABLE);
	char lastKey[9];
	char currentKey[9];

	int countTotal = 0;
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			line.copy(currentKey, 8, 0);
			if (countTotal > 0)
			{
				if (std::strcmp(lastKey, currentKey) > 0)
				{
					printf("%d and %d \n", countTotal - 1, countTotal);
					printf("%s and %s\n", lastKey, currentKey);
					return -1;
				}
			}
			line.copy(lastKey, 8, 0);
			countTotal++;
		}
		file.close();
	}
	if (countTotal == 0)
	{
		printf("Output table is empty\n");
	}
	return countTotal;
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
	int countOutputRecords = verityOrder();
	if (countOutputRecords > 0)
	{
		printf("\n%d records in output table\n", countOutputRecords);
		printf("\nVerified sorting order is correct\n");
	}
	else
	{
		printf("Wrong sorting");
	}

	return 0;
}
