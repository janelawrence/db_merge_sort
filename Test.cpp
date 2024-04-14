#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Record.h"
#include "HDD.h"
#include "TreeOfLosers.h"
#include "CACHE.h"
#include "Run.h"
#include "DRAM.h"
#include "SSD.h"

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

// unsigned long long CACHE_SIZE = 1ULL * 1024 * 1024; //1 MB
unsigned long long CACHE_SIZE = 10ULL * 1024; // 10 KB

unsigned long long DRAM_SIZE = 100ULL * 1024 * 1024; // 100 MB

unsigned long long SSD_SIZE = 10ULL * 1024 * 1024 * 1024; // 1 GB
char *INPUT_TXT = "medium_200_1024_input.txt";

// char * INPUT_TXT = "input.txt";
// char * INPUT_TXT = "input_table";

long SSD_LAT = 100;											 // 0.1 ms = 100 microseconds(us)
unsigned long long SSD_BAN = 200ULL * 1024 * 1024 / 1000000; // 200 MB/s = 200 MB/us

long HDD_LAT = 500;										  // 5 ms = 0.005 s
unsigned long long HDD_BAN = 100 * 1024 * 1024 / 1000000; // 100 MB/s = 100 MB/us

// int PAGE_SIZE = 2048; // 5 KB
int PAGE_SIZE = 10240; // 10 KB
// int PAGE_SIZE = 20480; // 20 KB
// int PAGE_SIZE = 40; //40B

int recordSize = 0;
int numRecords = 0;

const char *ACCESS_WRITE = "write";
const char *ACCESS_READ = "read";

const char *spillToSSDState = "STATE -> SPILL_RUNS_SSD: Spill sorted runs to the SSD device";

/* Function to read records from the input file
	Return: pages in DRAM stored in a Run *
*/
Run *readRecordsIntoDRAM(const char *fileName, int recordSize, int numRecords, int totalBytes,
						 int nBuffersDRAM, int maxRecordsInPage)
{
	//  Read input.txt = input data stored in HDD
	std::ifstream inputFile(fileName, std::ios::binary);
	if (!inputFile.is_open())
	{
		std::cerr << "Error: Could not open file " << INPUT_TXT << std::endl;
		return nullptr;
	}
	// Get the file size
	inputFile.seekg(0, std::ios::end);
	size_t totalBytesInFile = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);

	printf("Size: %zu\n", totalBytesInFile);

	if (totalBytesInFile > totalBytes + 2 * numRecords)
	{
		printf("ERROR: bytes in input file is %d bytes larger than recordSize * numRecord\n\n",
			   totalBytesInFile - (numRecords + totalBytes));
	}

	// Allocate buffer to hold the data
	char *inputData = new char[totalBytes];

	// Read data from the file
	inputFile.read(inputData, totalBytes);

	if (inputFile.fail())
	{
		std::cerr << "Error while reading file " << INPUT_TXT << std::endl;
		delete[] inputData;
		return nullptr;
	}

	inputFile.close();

	char rawRecord[recordSize];

	size_t recordsOffsetHDD = 0;
	Run *pagesInDRAM = new Run();

	while (recordsOffsetHDD < totalBytes)
	{
		// Read at most nBuffersDRAM pages of data from HDD to DRAM
		int bufferUsed = 0;
		while (recordsOffsetHDD < totalBytes && bufferUsed < nBuffersDRAM)
		{
			// Read records data into one page
			Page *newPage = new Page(bufferUsed, maxRecordsInPage, PAGE_SIZE);
			while (recordsOffsetHDD < totalBytes && newPage->getNumRecords() < maxRecordsInPage)
			{
				// Organize records into pages
				for (size_t j = 0; j < recordSize && (recordsOffsetHDD + j) < totalBytes; ++j)
				{
					rawRecord[j] = inputData[recordsOffsetHDD + j];
				}
				Record *record = new Record(recordSize, rawRecord);
				record->setSlot(bufferUsed);
				newPage->addRecord(record);
				recordsOffsetHDD += recordSize;
				// skipping '/0' and nextline character
				recordsOffsetHDD += 2;
			}
			pagesInDRAM->appendPage(newPage);
			bufferUsed++;
		}
	}
	delete[] inputData;
	return pagesInDRAM;
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
		   CACHE_SIZE / sizeof(recordPtr));
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

	int mergeLevels = static_cast<int>(std::ceil(static_cast<double>(totalBytes) / DRAM_SIZE));

	int maxRecordsInPage = PAGE_SIZE / recordSize;
	int nPagesFitInCache = CACHE_SIZE / PAGE_SIZE;

	int maxTreeSize = CACHE_SIZE / recordSize;

	int nBuffersDRAM = DRAM_SIZE / PAGE_SIZE;
	int nBuffersReserved = 3;
	// Fan-in F during merging
	int nInputBuffersDRAM = nBuffersDRAM - nBuffersReserved; // reserve 3 page according to lecture notes

	int nBuffersSSD = SSD_SIZE / PAGE_SIZE;

	printStats(numRecords, recordSize, maxRecordsInPage, nPagesFitInCache, nBuffersDRAM,
			   nBuffersReserved, nInputBuffersDRAM, nBuffersSSD, mergeLevels);
	// for i in mergeLevels: // create memory-sized sorted runs
	Run *pagesInDRAM = readRecordsIntoDRAM(INPUT_TXT, recordSize, numRecords,
										   totalBytes, nBuffersDRAM, maxRecordsInPage);
	// pagesInDRAM->print();
	CACHE cache(CACHE_SIZE, nPagesFitInCache);

	// Open the output file in overwrite mode
	std::ofstream outputFile(outputTXT, std::ios::trunc);

	// Check if the file opened successfully
	if (!outputFile.is_open())
	{
		std::cerr << "Error: Could not open file trace0.txt for writing." << std::endl;
		return 1; // Return error code
	}
	// Print output to both console and file
	std::string outputString = "STATE -> SORT_MINI_RUNS: Sort cache-size mini runs\n";
	outputFile << outputString; // Print to file
	outputFile.close();
	// Create cache-sized mini-runs
	std::vector<Run *> sortedMiniRuns = cache.sort(pagesInDRAM, maxRecordsInPage, PAGE_SIZE);
	printf("At merge level %d, created %d cache-sized runs\n", mergeLevels, sortedMiniRuns.size());
	unsigned long long bytesWriteToSSD = 0;
	for (int i = 0; i < sortedMiniRuns.size(); i++)
	{
		// printf("---------------------- Sorted miniRun %d--------------------\n", i);
		// sortedMiniRuns[i]->print();
		bytesWriteToSSD += sortedMiniRuns[i]->getBytes();
	}

	SSD ssd(SSD_SIZE, SSD_LAT, SSD_BAN);
	DRAM dram(DRAM_SIZE);

	for (int i = 0; i < sortedMiniRuns.size(); i++)
	{
		if (!ssd.addRun(sortedMiniRuns[i]))
		{
			break;
		}
		bytesWriteToSSD += sortedMiniRuns[i]->getBytes();
	}

	// Use Alternative 1
	// TODO: Check percentage of memory overflow
	// TODO: Decides whether to do Graceful Degradation

	ssd.outputAccessState(ACCESS_WRITE, bytesWriteToSSD, outputTXT);

	// merge cached-sized mini runs on SSD
	ssd.outputMergeMsg(outputTXT);
	while (ssd.getNumRuns() != 1)
	// merge until one run left in SSD
	{
		int pagesToRead = std::min(maxTreeSize, nInputBuffersDRAM);
		pagesToRead = std::min(pagesToRead, ssd.getNumRuns());
		// Fetch the first page of each of the runs on SSD into DRAM
		ssd.outputAccessState(ACCESS_READ, pagesToRead * PAGE_SIZE, outputTXT);
		// ssd.print();
		for (int i = 0; i < pagesToRead; i++)
		{
			Run *runOnSSD = ssd.getRun(i);
			if (!runOnSSD->isEmpty())
			{
				Page *pageFetched = runOnSSD->getFirstPage()->clone();
				pageFetched->setSource(FROM_SSD);
				pageFetched->setIdx(i);
				dram.addPage(pageFetched);
				runOnSSD->removeFisrtPage();
				if (runOnSSD->isEmpty())
				{
					ssd.eraseRun(i);
				}
			}
			else
			{
				ssd.eraseRun(i);
			}
		}
		dram.merge(&ssd, maxTreeSize, outputTXT);
		// // Physically Remove empty run spaces
		// // after cleaning, there's no invalid run in SSD
		ssd.cleanInvalidRuns();

		// printf("After clean-------\n");
		// ssd.print();
	}
	ssd.print();
	// After creating memory-sized run:
	// 		(1) If SSD is not full: Write memory-sized run to SSD
	//		(2) If SSD is full: Write memory-sized run to HDD

	//  delete dynamically allocated memory before exit program

	return 0;
}
