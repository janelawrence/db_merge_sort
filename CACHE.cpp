#include "CACHE.h"
#include "defs.h"
#include "Run.h"
#include "HDD.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <dirent.h>
#include <vector>

// Constructor
CACHE::CACHE(){}


/**
 * This member function reads records from HDD,
 * and output cache-size sorted runs
 * 
*/
std::vector<TreeOfLosers*> CACHE::readFromHDD(int recordSize, HDD* hdd){

    std::vector<TreeOfLosers*> runs;
    int maxNumRecords = capacity / recordSize;
    std::vector<Record*> records;

    DIR* dir = opendir(hdd->getDir().c_str());

    if (!dir) {
        std::cerr << "Failed to open directory" << std::endl;
        return runs;
    }
    dirent* entry;
    int numReadIn = 0;
    int slotIdx = 0;
    while ((entry = readdir(dir)) != nullptr && numReadIn < maxNumRecords) {
        // Skip directories and special entries
        if (entry->d_type != DT_REG) {
            continue;
        }
        // Read data from file using hdd.readData
        Record * record = hdd->readData(entry->d_name, recordSize);
        // Append the records to the vector
        if(record != nullptr) {
            records.push_back(record);
        }
        numReadIn++;
        if(numReadIn == maxNumRecords) {
            //sorting
            TreeOfLosers* run = sort(hdd, records, slotIdx);
            runs.push_back(run);
            tree.clear();
            slotIdx++;
            records.clear();
            numReadIn = 0;
        }

    }

    // Last run that does not fill up CACHE
    if (numReadIn > 0 && numReadIn < maxNumRecords) {
        //sorting
        TreeOfLosers* run = sort(hdd, records, slotIdx);
        runs.push_back(run);
        tree.clear();

        slotIdx++;
        records.clear();
        numReadIn = 0;
    }
    closedir(dir);
    return runs;
}

TreeOfLosers* CACHE::sort(HDD* hdd, std::vector<Record*> records, int slotIdx){
    
    for(int i = 0; i < records.size(); i++) {
        Record* record = records[i];
        record->setSlot(slotIdx);
        tree.insert(record);
    }
    //  Clear cache by passing run to DRAM
    printf("\n\nCLEAR CACHE NOW\n");
    std::vector<Record*> sortedRecords;

    return tree.clone();
}


double CACHE::getCapacity() const{
    return capacity;
}

// g++ defs.cpp Run.cpp Record.cpp TreeOfLosers.cpp HDD.cpp CACHE.cpp -o cache
// int main (int argc, char * argv []){
//     // Create a hdd to store unsorted records
// 	HDD * const hdd = new HDD ("", 5, 100);

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


