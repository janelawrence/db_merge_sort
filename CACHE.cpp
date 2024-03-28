#include "CACHE.h"
#include "defs.h"
#include "Run.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <dirent.h>
#include <vector>

// Constructor
CACHE::CACHE(){};

std::vector<Run*> CACHE::readFromHDD(int totalSize, int recordSize, HDD* hdd, DRAM* dram){
    bool fitInCache = true;

    if(totalSize > capacity) {
        fitInCache = false;
    }
    std::vector<Run*> runs;
    // TODO: May not need this case
    if(fitInCache) {
        std::vector<Record*> records = hdd->readFilesInHDD(recordSize);
        //sorting
        sort(hdd, dram, records, 0);
        return runs;

    }else {
	    int maxNumRecords = capacity / recordSize;
        std::vector<Record*> records;

        DIR* dir = opendir(hdd->getDir().c_str());

        std::vector<Record*> records;
        if (!dir) {
            std::cerr << "Failed to open directory" << std::endl;
            return;
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
                Run* run = sort(hdd, dram, records, slotIdx);
                runs.push_back(run);

                slotIdx++;
                records.clear();
            }

        }

        closedir(dir);
    }
    return runs;
};

Run* CACHE::sort(HDD* hdd, DRAM* dram, std::vector<Record*> records, int slotIdx){
    for(int i = 0; i < records.size(); i++) {
        Record* record = records[i];
        record->setSlot(slotIdx);
        tree.insert(record);
    }
    //  Clear cache by passing run to DRAM
    printf("\n\nCLEAR CACHE NOW\n");
    std::vector<Record*> sortedRecords;

    while(!tree.isEmpty()) {
        Record* curr = tree.getMin();
        sortedRecords.push_back(curr);
        printf("Key: %s, slot: %d\n", curr->getKey(), curr->getSlot());
        // std::cout << "Minimum key after deletion: " << tree.getMin()->key << std::endl;
        tree.deleteMin();
    }
    return new Run(sortedRecords);
}

void CACHE::writeToDRAM(DRAM* dram){};

double CACHE::getCapacity() const{
    return capacity;
}




