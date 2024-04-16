#include "CACHE.h"
#include "defs.h"
#include "Run.h"
#include "Disk.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
#include <dirent.h>
#include <vector>

// Constructor
CACHE::CACHE(int cacheSize, int nPages) : MAX_CAPACITY(cacheSize), nPagesFitInCache(nPages) {}

std::vector<Run *> CACHE::sort(Run *pagesInDRAM, int maxRecordsInPage, int PAGE_SIZE)
{

	Page *curr = pagesInDRAM->getFirstPage();
	std::vector<Run *> miniRuns;
	Run *miniRun = new Run();
	int count = 0;
	while (curr)
	{
		if (count >= nPagesFitInCache)
		{
			// one cache-sized run has been filled
			miniRun->appendPage(tree.toNewPages(0, maxRecordsInPage, PAGE_SIZE));
			miniRuns.push_back(miniRun);
			miniRun = new Run();

			count = 0;
			tree.clear();
		}

		while (curr->getNumRecords() > 0)
		{
			Record *record = curr->getFirstRecord();
			record->setSlot(-1);
			tree.insert(record);
			curr->removeFisrtRecord();
		}
		count++;
		curr = curr->getNext(); // Get next page in memory
		if (!curr && !tree.isEmpty())
		{
			miniRun->appendPage(tree.toNewPages(0, maxRecordsInPage, PAGE_SIZE));
			miniRuns.push_back(miniRun);
			Run *miniRun = new Run();
			tree.clear();
		}
	}
	return miniRuns;
}

int CACHE::outputMiniRunState(
	const char *outputTXT)
{
	// Open the output file in overwrite mode
	std::ofstream outputFile(outputTXT, std::ios::app);

	// Check if the file opened successfully
	if (!outputFile.is_open())
	{
		std::cerr << "Error: Could not open file" << outputTXT << " for writing." << std::endl;
		return 1; // Return error code
	}

	// Print output to both console and file
	outputFile << "STATE -> SORT_MINI_RUNS: Sort cache-size mini runs\n";

	// close file
	outputFile.close();
}

double CACHE::getCapacity() const
{
	return capacity;
}

// g++ defs.cpp Run.cpp Record.cpp TreeOfLosers.cpp HDD.cpp CACHE.cpp -o cache
// int main (int argc, char * argv []){
//     // Create a hdd to store unsorted records
// 	Disk * const hdd = new HDD ("", 5, 100);

//     int numRecords = 8;
//     int recordSize = 20;

//     Record * record;
// 	// Generate numRecords number of records in HDD
// 	for(int i = 0; i < numRecords; i++) {
// 		record = new Record (recordSize, "");
// 		hdd->writeData(record->getKey(), record);
// 	}

//     CACHE* cache = new CACHE();
// 	std::vector<Run*> sortedRunsInCache = cache->readFromHDD(recordSize, hdd);

//     for(int i = 0; i < sortedRunsInCache.size(); i++) {
// 		Run* run = sortedRunsInCache[i];
// 		printf("------------- %d th Run -----------\n", i);
// 		run->printRun();
// 		printf("\n");
// 	}
// }
