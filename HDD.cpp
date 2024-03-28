#include "HDD.h"
#include "Record.h"
#include "defs.h"

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
HDD::HDD(const std::string& dir, double lat, double bw) : latency(lat), bandwidth(bw) {
    if(dir.empty()) {
        directory = createHDD();
    }else{
        directory = dir;
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
    TRACE (true);
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

std::string HDD::createHDD() {
    const std::string dirname = getHDDNameWithCurrentTime(); // Replace with desired directory path

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

std::string HDD::getHDDNameWithCurrentTime() {
     // Get current time
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);

    // Format date and time
    std::stringstream ss;
    ss << "hdd_unsorted_";
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


// int main() {
//     // Create an HDD object with empty directory, latency 5ms, and bandwidth 100MB/s
//     HDD hdd("", 5, 100);

//     // Simulate read and write operations
//     hdd.writeData("test.txt", 1024); // Writing 1KB to "test.txt"
//     hdd.writeData("test2.txt", 1024); // Writing 1KB to "test2.txt"

//     hdd.readData("test.txt", 1024);  // Reading 1KB from "test.txt"

//     return 0;
// }
