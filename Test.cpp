#include "Record.h"
#include "TreeOfLosers.h"
#include "CACHE.h"
#include "Run.h"
#include "DRAM.h"
#include "Disk.h"

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
// unsigned long long CACHE_SIZE = 1ULL * 1024 * 1024; // 1 MB
// unsigned long long DRAM_SIZE = 1ULL * 1024 * 1024; // 1MB
// unsigned long long SSD_SIZE = 10ULL * 1024 * 1024 * 1024; // 1 GB
// int PAGE_SIZE = 10240; // 10 KB

// >>>>>> Mini sized params Set up Start
unsigned long long CACHE_SIZE = 200; //
unsigned long long DRAM_SIZE = 1000;
unsigned long long SSD_SIZE = 1000;
int PAGE_SIZE = 100;
// char *INPUT_TXT = "medium_200_1024_input.txt";
char *INPUT_TXT = "mini_200_20_input.txt";
// Mini sized params Set up End <<<<<<<<<<

unsigned long long HDD_SIZE = std::numeric_limits<int>::max();
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
	Return: pages in DRAM stored in a Run *
*/
Run *readRecords(const char *fileName, int recordSize, int numRecords, int totalBytes,
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

	printf("totalBytesInFile: %zu\n", totalBytesInFile);

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
	Run *allPages = new Run();

	while (recordsOffsetHDD < totalBytesInFile)
	{
		// Read at most nBuffersDRAM pages of data from HDD to DRAM
		int bufferUsed = 0;

		while (recordsOffsetHDD < totalBytesInFile && bufferUsed < nBuffersDRAM)
		{
			// Read records data into one page
			Page *newPage = new Page(bufferUsed, maxRecordsInPage, PAGE_SIZE);
			while (recordsOffsetHDD < totalBytesInFile && newPage->getNumRecords() < maxRecordsInPage)
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
				// skipping '\n' and nextline character
				recordsOffsetHDD += 2;
			}
			allPages->appendPage(newPage);
			bufferUsed++;
		}
	}
	delete[] inputData;
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

	int mergeLevels = static_cast<int>(std::ceil(static_cast<double>(totalBytes) / DRAM_SIZE));

	int maxRecordsInPage = PAGE_SIZE / recordSize;
	int nPagesFitInCache = CACHE_SIZE / PAGE_SIZE;

	int maxTreeSize = CACHE_SIZE / recordSize;

	int nBuffersDRAM = DRAM_SIZE / PAGE_SIZE;
	int nBuffersReserved = 2;
	// Fan-in F during merging
	int nInputBuffersDRAM = nBuffersDRAM - nBuffersReserved; // reserve 2 page according for output

	int nBuffersSSD = SSD_SIZE / PAGE_SIZE;

	printStats(numRecords, recordSize, maxRecordsInPage, nPagesFitInCache, nBuffersDRAM,
			   nBuffersReserved, nInputBuffersDRAM, nBuffersSSD, mergeLevels);

	// all pages
	Run *allPages = readRecords(INPUT_TXT, recordSize, numRecords,
								totalBytes, nBuffersDRAM, maxRecordsInPage);

	CACHE cache(CACHE_SIZE, nPagesFitInCache);
	Disk ssd(SSD_SIZE, SSD_LAT, SSD_BAN, SSD);
	DRAM dram(DRAM_SIZE);
	Disk hdd(HDD_SIZE, HDD_LAT, HDD_BAN, HDD);

	// Open the output file in overwrite mode
	std::ofstream outputFile(outputTXT, std::ios::trunc);

	// Check if the file opened successfully
	if (!outputFile.is_open())
	{
		std::cerr << "Error: Could not open file trace0.txt for writing." << std::endl;
		return 1; // Return error code
	}

	// Create all cache-sized mini-runs
	std::vector<Run *> allSortedMiniRuns = cache.sort(allPages, maxRecordsInPage, PAGE_SIZE);
	int start = 0;

	std::vector<Run *> memorySizedRunsOnSSD;
	std::vector<Run *> memorySizedRunsOnHDD;

	// Debug
	int total_bytes_merged = 0;
	for (int i = 0; i < allSortedMiniRuns.size(); i++)
	{
		total_bytes_merged += allSortedMiniRuns[i]->getBytes();
	}

	printf("----- Input %d pages, Total bytes stored in %d allSortedMiniRuns: %d-----------------\n\n",
		   allPages->getNumPages(), allSortedMiniRuns.size(), total_bytes_merged);
	for (int level = 0; level < mergeLevels; level++)
	{
		// Open the output file in overwrite mode
		// std::ofstream outputFile(outputTXT, std::ios::trunc);
		// Print output to file
		std::string outputString = "STATE -> SORT_MINI_RUNS: Sort cache-size mini runs\n";
		outputFile << outputString; // Print to file
		outputFile.close();

		// Get unmerged cache-sized mini-runs
		std::vector<Run *> sortedMiniRuns;
		unsigned long long bytesRead = 0;
		int nRuns = static_cast<int>(std::ceil(nBuffersDRAM / nPagesFitInCache));
		for (int i = 0; start < allSortedMiniRuns.size() && i < nRuns; start++, i++)
		{
			sortedMiniRuns.push_back(allSortedMiniRuns[start]->clone());
		}

		// create memory-sized sorted runs
		printf("At merge level %d, created %d cache-sized runs\n", level, sortedMiniRuns.size());
		unsigned long long bytesWriteToDisk = 0;
		for (int i = 0; i < sortedMiniRuns.size(); i++)
		{
			bytesWriteToDisk += sortedMiniRuns[i]->getBytes();
		}
		if (ssd.getCapacity() >= bytesWriteToDisk)
		{
			for (int i = 0; i < sortedMiniRuns.size(); i++)
			{
				if (!ssd.addRun(sortedMiniRuns[i]))
				{
					break;
				}
			}
			// TODO: Check percentage of memory overflow
			// TODO: Decides whether to do Graceful Degradation

			// Use Alternative 1
			ssd.outputAccessState(ACCESS_WRITE, bytesWriteToDisk, outputTXT);

			// merge cached-sized mini runs on SSD
			ssd.outputMergeMsg(outputTXT);

			while (ssd.getNumUnsortedRuns() != 1)
			// merge until one run left in SSD
			{
				int pagesToRead = std::min(maxTreeSize, nInputBuffersDRAM);
				pagesToRead = std::min(pagesToRead, ssd.getNumUnsortedRuns());
				// Fetch the first page of each of the runs on SSD into DRAM
				ssd.outputAccessState(ACCESS_READ, pagesToRead * PAGE_SIZE, outputTXT);
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
						ssd.setCapacity(ssd.getCapacity() + pageFetched->getBytes());

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
				dram.mergeFromSrcToDest(&ssd, &ssd, maxTreeSize, outputTXT);
				dram.clear();
				// Physically Remove empty run spaces after cleaning, there's no invalid run in SSD
				ssd.cleanInvalidRuns();
			}
			ssd.moveRunToTempList(0);
			ssd.cleanInvalidRuns();
		}
		else // Merge from HDD
		{
			// TODO: Check percentage of memory overflow
			// TODO: Decides whether to do Graceful Degradation

			for (int i = 0; i < sortedMiniRuns.size(); i++)
			{
				hdd.addRun(sortedMiniRuns[i]);
				bytesWriteToDisk += sortedMiniRuns[i]->getBytes();
			}
			// hdd.print();

			// Use Alternative 1
			hdd.outputAccessState(ACCESS_WRITE, bytesWriteToDisk, outputTXT);

			// merge cached-sized mini runs on hdd
			hdd.outputMergeMsg(outputTXT);

			while (hdd.getNumUnsortedRuns() != 1)
			// merge until one run left in hdd
			{
				int pagesToRead = std::min(maxTreeSize, nInputBuffersDRAM);
				pagesToRead = std::min(pagesToRead, hdd.getNumUnsortedRuns());
				// Fetch the first page of each of the runs on hdd into DRAM
				hdd.outputAccessState(ACCESS_READ, pagesToRead * PAGE_SIZE, outputTXT);
				for (int i = 0; i < pagesToRead; i++)
				{
					Run *runOnHDD = hdd.getRun(i);
					if (!runOnHDD->isEmpty())
					{
						Page *pageFetched = runOnHDD->getFirstPage()->clone();
						pageFetched->setSource(FROM_HDD);
						pageFetched->setIdx(i);
						dram.addPage(pageFetched);
						runOnHDD->removeFisrtPage();
						if (runOnHDD->isEmpty())
						{
							hdd.eraseRun(i);
						}
					}
					else
					{
						hdd.eraseRun(i);
					}
				}
				dram.mergeFromSrcToDest(&hdd, &hdd, maxTreeSize, outputTXT);
				dram.clear();
				// // Physically Remove empty run spaces
				// // after cleaning, there's no invalid run in hdd
				hdd.cleanInvalidRuns();
			}
			hdd.moveRunToTempList(0);
			hdd.cleanInvalidRuns();
		}
	}

	hdd.clear();
	hdd.setMaxCap(SSD_SIZE);
	hdd.setCapacity(SSD_SIZE);

	printf("Total number of mem-sized runs on SSD: %d\n", ssd.getNumTempRuns());
	printf("Total number of mem-sized runs on HDD: %d\n", hdd.getNumTempRuns());

	int numMemSizedRunOnSSD = memorySizedRunsOnSSD.size();
	int numMemSizedRunOnHDD = memorySizedRunsOnHDD.size();

	if (numMemSizedRunOnSSD == 1 && numMemSizedRunOnHDD == 0)
	{
		ssd.addRun(memorySizedRunsOnSSD[0]);
		hdd.outputAccessState(ACCESS_WRITE, totalBytes, outputTXT);
		printf("TODO: Write to a output_table");
		ssd.writeOutputTable(OUTPUT_TABLE);
		return 0;
	}

	// for (int i = 0; i < numMemSizedRunOnSSD; i++)
	// {
	// 	ssd.addRun(memorySizedRunsOnSSD[i]->clone());
	// }

	// for (int i = 0; i < numMemSizedRunOnHDD; i++)
	// {
	// 	hdd.addRun(memorySizedRunsOnHDD[i]->clone());
	// }

	// while (ssd.getNumUnsortedRuns() > 0)
	// {
	// 	dram.mergeFromSrcToDest(&ssd, &hdd, maxTreeSize, outputTXT);
	// }

	// while (hdd.getNumUnsortedRuns() > 0)
	// {
	// 	dram.mergeFromSrcToDest(&hdd, &hdd, maxTreeSize, outputTXT);
	// }

	//  delete dynamically allocated memory before exit program
	return 0;
}
