#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Record.h"
#include "HDD.h"
#include "TreeOfLosers.h"

#include <cstdlib> // For atoi function
#include <iostream>
#include <chrono>
#include <filesystem>
#include <dirent.h>
#include <vector>

// namespace fs = std::filesystem;

std::vector<Record*> readFilesInHDD(const std::string& directoryPath, HDD* hdd, int size) {
    DIR* dir = opendir(directoryPath.c_str());

	std::vector<Record*> records;
	if (!dir) {
        std::cerr << "Failed to open directory" << std::endl;
        return records;
    }
    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
		// Skip directories and special entries
        if (entry->d_type != DT_REG) {
            continue;
        }
        // Read data from file using hdd.readData
        Record * record = hdd->readData(entry->d_name, size);

        // Append the records to the vector
		if(record != nullptr) {
			records.push_back(record);
		}
    }

    closedir(dir);
	return records;
}

// int ceilDiv(int dividend, int divisor) {
//     if (dividend % divisor == 0) {
//         return dividend / divisor;
//     } else {
//         return dividend / divisor + 1;
//     }
// }

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

	std::vector<Record*> records = readFilesInHDD(hdd->getDir().c_str(), hdd, recordSize);

	// Cache size = 1MB = 1024 * 1024 B
	int CACHE_SIZE = 1024 * 1024;
	int numRecordsInCache = CACHE_SIZE / recordSize;
	// Number of runs:
	int numRunsCache = ceilDiv(records.size(), numRecordsInCache);
	printf("\n%d records in Cache, number runs created %d\n", numRecordsInCache, numRunsCache);

	// PQ Tree
	TreeOfLosers treeInCache;

	for(int idx = 0; idx < numRunsCache; idx ++) {
		// from idx to (idx + 1) * numRecordsInCache
		int i;
		for(i = idx * numRecordsInCache; i < records.size() &&  i < (idx + 1) * numRecordsInCache; i++) {
			Record* record = records[i];
			treeInCache.insert(record->getKey(), idx);
		}
		//  Clear cache by passing run to DRAM
		printf("\n\nCLEAR CACHE NOW\n", i);
		while(!treeInCache.isEmpty()) {
			Node* curr = treeInCache.getMin();
			printf("Key: %s, slot: %d\n", curr->key, curr->slot);
			// std::cout << "Minimum key after deletion: " << tree.getMin()->key << std::endl;
			treeInCache.deleteMin();
		}
	}

	// // Get and print the minimum key again
    // while(!treeInCache.isEmpty()) {
    //     Node* curr = treeInCache.getMin();
    //     printf("Key: %s, slot: %d\n", curr->key, curr->slot);
    //     // std::cout << "Minimum key after deletion: " << tree.getMin()->key << std::endl;
    //     treeInCache.deleteMin();
    // }

	// TODO: When passing runs into DRAM, need to assign them index
	// DRAM size = 100 MB = 100 * 2014 * 1024 B
	int DRAM_SIZE = 100 * 1024 * 1024;
	int numRecordsInDRAM = DRAM_SIZE / CACHE_SIZE;
	printf("%d runs in DRAM\n", numRecordsInDRAM);


	// SDD size = 10 GB 
	long long SDD_SIZE = 10LL * 1024 * 1024 * 1024;
	int numRecordsInSDD = SDD_SIZE / DRAM_SIZE;
	printf("%d runs in SDD\n", numRecordsInSDD);





	// // Test reading record in to Record
	// std::string directory = hdd->getDir(); // Specify the directory path
    // for (const std::filesystem::directory_entry& entry : fs::directory_iterator(directory)) {
    //     if (entry.is_regular_file()) {
    //         std::cout << entry.path().filename() << std::endl; // Print the file name
    //     }
    // }
    // return 0;

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
