#include "EmulatedHDD.h"
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



// Constructor
// size in B
// bandiwith in MB/s
EmulatedHDD::EmulatedHDD(const std::string& dir, double lat, double bw) : latency(lat), bandwidth(bw) {
    if(dir.empty()) {
        directory = createHDD();
    }else{
        directory = dir;
    }
}

// Method to simulate read operation
// size in B
Record * EmulatedHDD::readData(const std::string& filename, int size) {
    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    // Open file for reading
    std::ifstream file(directory + "/" + filename, std::ios::binary);
    if (!file.is_open()) {
        printf("Error: Failed to open file for reading: %s\n", filename.c_str());
        return 0;
    }

    // Read data from file
    char* buffer = new char[size];
    file.read(buffer, size);

    // Close file
    file.close();

    // TODO: return Record*
    // char* key = new char[11];
    // for(int i = 0; i < 10; ++i) {
    //     key[i] = buffer[i];
    // }
    // Record * record = new Record(size, key);
    // record->setContent(buffer);
    Record * record = Record::deserialize(buffer, size);

    // Simulate bandwidth
    double transferTime = 1000 * static_cast<double>(size) / (bandwidth * 1024 * 1024);
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(transferTime * 1000)));

    // std::cout << "Read " << size << " bytes from file: " << filename << ". Transfer time: " << transferTime << " seconds\n";
    printf("Read %d bytes from file: %s. Transfer time: %.4f ms, total time: %.4f ms\n",\
             size, \
             filename.c_str(), \
             transferTime, \
             transferTime + latency);

    delete[] buffer;

    return record;
}

// Method to simulate write operation
// size in B
// bandiwith in MB/s
void EmulatedHDD::writeData(const std::string& filename, Record* record) {
    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    // Open file for writing
    std::ofstream file(directory + "/" + filename, std::ios::binary);
    if (!file.is_open()) {
        // std::cerr << "Error: Failed to open file for writing: " << filename << std::endl;
        printf("Error: Failed to open file for writing: %s\n", filename.c_str());
        
        return;
    }
    // Generate random data
    // char* buffer = new char[size];
    // for (int i = 10; i < size; ++i) {
    //     buffer[i] = rand() % 256; // Random byte value
    // }
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

std::string EmulatedHDD::createHDD() {
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

std::string EmulatedHDD::getHDDNameWithCurrentTime() {
     // Get current time
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);

    // Format date and time
    std::stringstream ss;
    ss << "hdd_unsorted_";
    ss << std::setw(2) << tm->tm_hour;       // Hour with leading zero
    ss << std::setw(2) << tm->tm_min;        // Minute with leading zero
    ss << std::setw(2) << tm->tm_sec;        // Second with leading zero
    ss << "_";
    ss << std::setfill('0') << std::setw(2) << (tm->tm_mon + 1);  // Month with leading zero
    ss << std::setw(2) << tm->tm_mday;       // Day with leading zero
    ss << "_";
    ss << std::setfill('0') << std::setw(4) << (tm->tm_year + 1900); // Year with 4 digits


    return ss.str();
}

std::string EmulatedHDD::getDir() const{
    return directory;
}

double EmulatedHDD::getLatency() const{
    return latency;
}

double EmulatedHDD::getBandwidth() const{
    return bandwidth;
}


// int main() {
//     // Create an EmulatedHDD object with empty directory, latency 5ms, and bandwidth 100MB/s
//     EmulatedHDD hdd("", 5, 100);

//     // Simulate read and write operations
//     hdd.writeData("test.txt", 1024); // Writing 1KB to "test.txt"
//     hdd.writeData("test2.txt", 1024); // Writing 1KB to "test2.txt"

//     hdd.readData("test.txt", 1024);  // Reading 1KB from "test.txt"

//     return 0;
// }
