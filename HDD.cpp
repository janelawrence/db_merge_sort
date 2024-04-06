#include "HDD.h"
#include "Record.h"
#include "defs.h"
#include "SSD.h"
#include "DRAM.h"


#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <ctime>          // For localtime and strftime
#include <sstream>        // For stringstream
#include <iomanip>        // For setfill and setw
#include <sys/stat.h> // For mkdir function
#include <cstdlib> // For atoi function
#include <filesystem>
#include <dirent.h>
#include <vector>


// Constructor
// size in B
// bandiwith in MB/s
HDD::HDD(const std::string& dirPrefix, double lat, double bw) : latency(lat), bandwidth(bw) {
    if(dirPrefix.empty()) {
        directory = createHDD("hdd_unsorted");
    }else{
        directory = createHDD(dirPrefix);
    }
}

// Method to simulate read operation
// size in B
Record * HDD::readData(const std::string& filename, int recordSize) {
    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    // Open file for reading
    std::ifstream file(directory + "/" + filename, std::ios::binary);
    if (!file.is_open()) {
        printf("Error: Failed to open file for reading: %s\n", filename.c_str());
        return 0;
    }

    // Read data from file
    char* buffer = new char[recordSize];
    file.read(buffer, recordSize);

    // Close file
    file.close();

    Record * record = Record::deserialize(buffer, recordSize);

    // Simulate bandwidth
    double transferTime = 1000 * static_cast<double>(recordSize) / (bandwidth * 1024 * 1024);
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(transferTime * 1000)));

    // std::cout << "Read " << size << " bytes from file: " << filename << ". Transfer time: " << transferTime << " seconds\n";
    printf("Read %d bytes from file: %s. Transfer time: %.4f ms, total time: %.4f ms\n",\
             recordSize, \
             filename.c_str(), \
             transferTime, \
             transferTime + latency);

    delete[] buffer;

    return record;
}

std::vector<Record*> HDD::readFilesInHDD(int recordSize) {
    DIR* dir = opendir(directory.c_str());

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
        Record * record = readData(entry->d_name, recordSize);

        // Append the records to the vector
		if(record != nullptr) {
			records.push_back(record);
		}
    }

    closedir(dir);
	return records;
}

// Method to simulate write operation
void HDD::simulateWriteData(double sizeInBytes) {
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    std::cout << "Wrote " << sizeInBytes << " bytes to HDD. Transfer time: " << transferTime << " seconds\n";
}

// Method to write operation
// size in B
// bandiwith in MB/s
void HDD::writeData(const std::string& filename, Record* record) {
    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    // Open file for writing
    std::ofstream file(directory + "/" + filename, std::ios::binary);
    if (!file.is_open()) {
        // std::cerr << "Error: Failed to open file for writing: " << filename << std::endl;
        printf("Error: Failed to open file for writing: %s\n", filename.c_str());
        
        return;
    }

    int size = record->getSize();
    // Write data to file
    file.write(record->serialize(), size);

    // Close file
    file.close();

    // Simulate bandwidth in ms
    double transferTime = 1000 * static_cast<double>(size) / (bandwidth * 1024 * 1024);
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(transferTime * 1000)));

    // std::cout << "Wrote " << size << " bytes to file: " << filename << ". Transfer time: " << transferTime << " seconds\n";
    printf("Wrote %d bytes to file: %s. Transfer time: %.4f ms, total time: %.4f ms\n",\
            size,\
            filename.c_str(), \
            transferTime, \
            transferTime + latency);

}

std::string HDD::createHDD(const std::string& prefix) {
    const std::string dirname = HDD::getHDDNameWithCurrentTime(prefix); // Replace with desired directory path

    // Convert std::string to const char*
    const char* cdirname = dirname.c_str();

    // Create directory
    int status = mkdir(cdirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status == 0) {
        // std::cout << "Directory created successfully: " << dirname << std::endl;
        printf("Directory created successfully: %s\n", dirname.c_str());
    } else {
        // std::cerr << "Error: Failed to create directory" << std::endl;
        printf("Error: Failed to create directory\n");

    }
    return dirname;
}

// TODO: It can be void??? as we don't have lower level of memory after HDD
// std::vector<Run*> HDD::merge(std::vector<Run*> runs, int recordSize){

//     std::vector<Run*> output;
//     if (runs.size() > 0) {
//         // int maxNumRecords = inf
//         int numRuns = runs.size();
//         int i = 0;
//         // Keep track of slot index for run stored in buffer
//         int currSlot = 0;
//         int totalUsedRuns = 0;
//         Run* buffer = new Run();
//         while(true) {
//             if(totalUsedRuns == numRuns) {
//                 break;
//             }
//             if(tree.getSize() == numRuns) {
//                 Record * winner = tree.getMin();
//                 int prevSlot = winner->getSlot();
//                 Run * prevRun = runs[prevSlot];
//                 winner->setSlot(currSlot);
//                 buffer->add(new Record(*winner));
//                 tree.deleteMin();
//                 if(!prevRun->isEmpty()){
//                     tree.insert(new Record(*(prevRun->getFirst())));
//                     prevRun->removeFisrt();

//                     // if prevRun becomes empty, inc counter
//                     if(prevRun->isEmpty()) {
//                         ++totalUsedRuns;
//                     }
//                 }
//                 continue;

//             }
//             i = i % numRuns;
//             Run * run = runs[i];
//             if(run->isEmpty()) {
//                 i++;
//                 continue;
//             }
//             // insert one record from runs[i] into tree
//             tree.insert(new Record(*(run->getFirst())));
//             run->removeFisrt();
//             if(run->isEmpty()) {
//                 totalUsedRuns++;
//             }
//             i++;
//         }
//         // Check if tree still has Records
//         while(!tree.isEmpty()) {
//             Record * winner = tree.getMin();
//             winner->setSlot(currSlot);
//             buffer->add(new Record(*winner));
//             tree.deleteMin();
//         }

//         printf("------------- HDD Output %d th Run -----------\n", 0);
//         buffer->print();
//         printf("\n");


//         //Write sorted result to HDD immediately
//         while(!buffer->isEmpty()) {
//             Record * record = buffer->getFirst();
//             writeData(record->getKey(), record);
//             buffer->removeFisrt();
//         }

//         // if(!buffer->isEmpty()) {
//         //     output.push_back(buffer->clone());
//         //     // clear buffer
//         //     buffer->clear();
//         // }

//         printf("CurrSlot: %d, numRunsEmptied: %d\n", currSlot, totalUsedRuns);
//     }
//     return output;
// }


std::string HDD::getHDDNameWithCurrentTime(const std::string& prefix) {
     // Get current time
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);

    // Format date and time
    std::stringstream ss;
    // ss << "hdd_unsorted_";
    ss << prefix;
    ss << "_";
    ss << std::setfill('0') << std::setw(2) << (tm->tm_mon + 1);  // Month with leading zero
    ss << std::setw(2) << tm->tm_mday;       // Day with leading zero
    ss << "_";
    ss << std::setfill('0') << std::setw(2) << tm->tm_hour;       // Hour with leading zero
    ss << std::setw(2) << tm->tm_min;        // Minute with leading zero
    ss << std::setw(2) << tm->tm_sec;        // Second with leading zero
    ss << "_";
    ss << std::setfill('0') << std::setw(4) << (tm->tm_year + 1900); // Year with 4 digits


    return ss.str();
}


std::string HDD::getDir() const{
    return directory;
}

double HDD::getLatency() const{
    return latency;
}

double HDD::getBandwidth() const{
    return bandwidth;
}


// To test this main individually:
// use: g++ Run.cpp Record.cpp TreeOfLosers.cpp DRAM.cpp SSD.cpp HDD.cpp -o hdd
// int main() {

//     TreeOfLosers tree1;
//     TreeOfLosers tree2;
//     TreeOfLosers tree3;

//     int recordSize = 20;
//     int numRecords = 15;

//     int totalBytes = recordSize * numRecords;

//     Record * r1 = new Record(recordSize, "alsdfeei");
//     Record * r2 = new Record(recordSize, "ewfeasdf");
//     Record * r3 = new Record(recordSize, "bdfsewfh");
//     Record * r4 = new Record(recordSize, "abcdasdf");
//     Record * r5 = new Record(recordSize, "eeerghwr");

//     Record * r6 = new Record(recordSize, "alsdmeei");
//     Record * r7 = new Record(recordSize, "ewfqesdf");
//     Record * r8 = new Record(recordSize, "alsdiefd");
//     Record * r9 = new Record(recordSize, "msjdfhwd");
//     Record * r10 = new Record(recordSize, "edprghwr");

//     Record * r11 = new Record(recordSize, "zlsdmeei");
//     Record * r12 = new Record(recordSize, "oeqesdfw");
//     Record * r13 = new Record(recordSize, "skfjhuwc");
//     Record * r14 = new Record(recordSize, "kviwudvg");
//     Record * r15 = new Record(recordSize, "aaabkkee");

//     r1->setSlot(0);
//     r2->setSlot(0);
//     r3->setSlot(0);
//     r4->setSlot(0);
//     r5->setSlot(0);

//     r6->setSlot(1);
//     r7->setSlot(1);
//     r8->setSlot(1);
//     r9->setSlot(1);
//     r10->setSlot(1);

//     r11->setSlot(2);
//     r12->setSlot(2);
//     r13->setSlot(2);
//     r14->setSlot(2);
//     r15->setSlot(2);




//     // Insert some records
//     tree1.insert(r1);
//     tree1.insert(r2);
//     tree1.insert(r3);
//     tree1.insert(r4);
//     tree1.insert(r5);

//     tree2.insert(r6);
//     tree2.insert(r7);
//     tree2.insert(r8);
//     tree2.insert(r9);
//     tree2.insert(r10);

//     tree3.insert(r11);
//     tree3.insert(r12);
//     tree3.insert(r13);
//     tree3.insert(r14);
//     tree3.insert(r15);

//     std::vector<TreeOfLosers*> runs;
//     runs.push_back(&tree1);
//     runs.push_back(&tree2);
//     runs.push_back(&tree3);



//     // Create an DRAM object with capacity 10GB, latency 0.1ms, and bandwidth 200MB/s
//     DRAM* dram = new DRAM();

//     std::vector<Run*> dramOutput = dram->merge(runs, recordSize);
//     for(int i = 0; i < dramOutput.size(); i++) {
// 		Run* run = dramOutput[i];
// 		printf("-------------DRAM Output %d th Run -----------\n", i);
// 		run->print();
// 		printf("\n");
// 	}

//     SSD* ssd = new SSD(0.0001, 200 * 1024 * 104);
    
//     ssd->writeData(totalBytes);

//     std::vector<Run*> ssdOutput = ssd->merge(dramOutput, recordSize);

//     for(int i = 0; i < ssdOutput.size(); i++) {
// 		Run* run = ssdOutput[i];
// 		printf("------------- SSD Output %d th Run -----------\n", i);
// 		run->print();
// 		printf("\n");
// 	}

//     HDD* hdd = new HDD("hdd_sorted", 0.005, 100 * 1024 * 1024);
    
//     // Simulate runtime only, does not actually write to disk
//     hdd->simulateWriteData(totalBytes);

//     std::vector<Run*> hddOutput = hdd->merge(ssdOutput, recordSize);

//     return 0;
// }
