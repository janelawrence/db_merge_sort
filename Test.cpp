#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Record.h"
#include "EmulatedHDD.h"

#include <cstdlib> // For atoi function
#include <iostream>
#include <chrono>
#include <filesystem>
#include <dirent.h>
#include <vector>

// namespace fs = std::filesystem;

std::vector<Record*> listFilesInDirectory(const std::string& directoryPath, EmulatedHDD* hdd, int size) {
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
	EmulatedHDD * const hdd = new EmulatedHDD ("", 5, 100);

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

	listFilesInDirectory(hdd->getDir().c_str(), hdd, recordSize);

	// Cache size = 1MB = 1024 * 1024 B
	int cacheSize = 1024 * 1024;
	int numRecordsInCache = cacheSize / recordSize;
	printf("\n%d records in Cache\n", numRecordsInCache);


	// DRAM size = 100 MB = 100 * 2014 * 1024 B
	int dramSize = 100 * 1024 * 1024;
	int numRecordsInDRAM = dramSize / cacheSize;
	printf("%d runs in DRAM\n", numRecordsInDRAM);



	// SDD size = 10 GB 
	long long sddSize = 10LL * 1024 * 1024 * 1024;
	int numRecordsInSDD = sddSize / dramSize;
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
