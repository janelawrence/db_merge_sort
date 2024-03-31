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

int main (int argc, char * argv [])
{
	// TRACE (true);

	//  Default value
	int numRecords = 50 * 1024 * 1024;
	int recordSize = 20; // 20 B

	if (argc  >= 3) {
		numRecords = atoi(argv[1]);
		recordSize = atoi(argv[2]);
	}else {
        // If there are not enough command-line arguments, inform the user
        printf("\nUsage: %s <numRecords> <recordSize>\n", argv[0]);
        return 1; // Return with an error status
    }

	if(numRecords <= 0) {
		printf("\nrecordNumber is 0, exit program\n");
		return 0;
	}

	// Create a hdd to store unsorted records
	HDD * const hdd = new HDD ("", 5, 100);

	// Start measuring time
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	Record * record;
	// Generate numRecords number of records in HDD
	for(int i = 0; i < numRecords; i++) {
		record = new Record (recordSize, "");
    	TRACE (true);
		hdd->writeData(record->getKey(), record);
	}

	// Stop measuring time
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    // Calculate duration
    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    printf("\nTotal latency: %lld ms\n", static_cast<long long>(duration.count()));

	// std::vector<Record*> records = readFilesInHDD(hdd->getDir().c_str(), hdd, recordSize);
	CACHE* cache = new CACHE();
	std::vector<TreeOfLosers*> sortedRunsInCache = cache->readFromHDD(recordSize, hdd);
	
	for(int i = 0; i < sortedRunsInCache.size(); i++) {
		TreeOfLosers* run = sortedRunsInCache[i];
		printf("-------------Cache-sized %d th Run -----------\n", i);
		run->print();
		printf("\n");
	}
	int numRunsInCache = sortedRunsInCache.size();
	// output if cache can fit in
	if(numRunsInCache == 1) {
		// sorting done, write to HDD
		HDD* outputHDD = new HDD("sorted", 5, 100);
		std::vector<Record*> sortedRecords = sortedRunsInCache[0]->toVector();
		for(int i = 0; i < sortedRecords.size(); i++) {
			outputHDD->writeData(sortedRecords[i]->getKey(), sortedRecords[i]);
		}
		delete outputHDD;
	} else {
		// WRITE TO DRAM, and merge in DRAM
		DRAM* dram = new DRAM();

		std::vector<Run*> sortedRunsInDRAM = dram->merge(sortedRunsInCache, recordSize);

		int numRunsInDRAM = sortedRunsInDRAM.size();

		// output if DRAM can fit in
		if(numRunsInDRAM == 1) {
			// sorting done, write to HDD
			HDD* outputHDD = new HDD("sorted_after_dram", 5, 100);
			Run* sortedRun = sortedRunsInDRAM[0];
			std::list<Record*> sortedRecords = sortedRunsInDRAM[0]->getRecords();
			for (std::list<Record*>::const_iterator it = sortedRecords.begin(); it != sortedRecords.end(); ++it) {
				outputHDD->writeData((*it)->getKey(), (*it));
			}
			delete outputHDD;
		}else {
			//WRTIE TO SSD
			SSD* ssd = new SSD(0.0001, 200 * 1024 * 1024);
    
    		ssd->writeData(numRecords * recordSize);

    		std::vector<Run*> sortedRunsInSSD = ssd->merge(sortedRunsInDRAM, recordSize);


			if(sortedRunsInSSD.size() == 1) {
				// sorting done, write to HDD
				HDD* outputHDD = new HDD("sorted_after_ssd", 0.005, 100 * 1024 * 1024);
				Run* sortedRun = sortedRunsInSSD[0];
				std::list<Record*> sortedRecords = sortedRunsInSSD[0]->getRecords();
				for (std::list<Record*>::const_iterator it = sortedRecords.begin(); it != sortedRecords.end(); ++it) {
					outputHDD->writeData((*it)->getKey(), (*it));
				}
				delete outputHDD;
			}else {// Merge on HDD and write the sorted result to HDD_sorted
				// Simulate runtime only, does not actually write to disk
				HDD* outputHDD = new HDD("sorted_after_hdd", 0.005, 100 * 1024 * 1024);
    			hdd->simulateWriteData(numRecords * recordSize);
    			std::vector<Run*> hddOutput = hdd->merge(sortedRunsInSSD, recordSize);
				delete outputHDD;
			}

			delete dram;

		}
	delete hdd;
	delete record;


	return 0;
	} // main
}
