#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Record.h"
#include "HDD.h"
#include "TreeOfLosers.h"
#include "CACHE.h"
#include "Run.h"

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
	std::vector<Run*> sortedRunsInCache = cache->readFromHDD(recordSize, hdd);
	
	for(int i = 0; i < sortedRunsInCache.size(); i++) {
		Run* run = sortedRunsInCache[i];
		printf("------------- %d th Run -----------\n", i);
		run->printRun();
		printf("\n");
	}



	delete hdd;
	delete record;

	// Plan * const plan = new ScanPlan (7);
	// // new SortPlan ( new FilterPlan ( new ScanPlan (7) ) );

	// Iterator * const it = plan->init ();
	// it->run ();
	// delete it;

	// delete plan;

	return 0;
} // main
