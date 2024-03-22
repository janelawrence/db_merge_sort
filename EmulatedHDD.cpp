#include "EmulatedHDD.h"
#include <iostream>
#include "defs.h"
#include <fstream>
#include <chrono>
#include <thread>
#include <ctime>          // For localtime and strftime
#include <sstream>        // For stringstream
#include <iomanip>        // For setfill and setw
#include <sys/stat.h> // For mkdir function


// Constructor
// size in MB
// bandiwith in MB
EmulatedHDD::EmulatedHDD(const std::string& dir, double lat, double bw) : latency(lat), bandwidth(bw) {
    if(dir.empty()) {
        directory = createHDD();
    }else{
        directory = dir;
    }
}

// Method to simulate read operation
// size in MB
void EmulatedHDD::readData(const std::string& filename, int size) {
    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    // Open file for reading
    std::ifstream file(directory + "/" + filename, std::ios::binary);
    if (!file.is_open()) {
        printf("Error: Failed to open file for reading: %s\n", filename.c_str());
        return;
    }

    // Read data from file
    char* buffer = new char[size];
    file.read(buffer, size);

    // Close file
    file.close();

    // Simulate bandwidth
    double transferTime = static_cast<double>(size) / bandwidth;
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(transferTime * 1000)));

    // std::cout << "Read " << size << " bytes from file: " << filename << ". Transfer time: " << transferTime << " seconds\n";
    printf("Read %d bytes from file: %s. Transfer time: %f seconds\n", size, filename.c_str(), transferTime);

    delete[] buffer;
}

// Method to simulate write operation
// size in MB
// bandiwith in MB
void EmulatedHDD::writeData(const std::string& filename, int size) {
    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    // Open file for writing
    std::ofstream file(directory + "/" + filename, std::ios::binary);
    if (!file.is_open()) {
        // std::cerr << "Error: Failed to open file for writing: " << filename << std::endl;
        printf("Error: Failed to open file for writing: %s\n", filename.c_str());
        
        return;
    }
    int sizeInBytes = size * 1024 * 1024;
    // Generate random data
    char* buffer = new char[sizeInBytes];
    for (int i = 0; i < sizeInBytes; ++i) {
        buffer[i] = rand() % 256; // Random byte value
    }

    // Write data to file
    file.write(buffer, sizeInBytes);

    // Close file
    file.close();

    // Simulate bandwidth
    double transferTime = static_cast<double>(size) / bandwidth;
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(transferTime * 1000)));

    // std::cout << "Wrote " << size << " bytes to file: " << filename << ". Transfer time: " << transferTime << " seconds\n";
    printf("Wrote %d bytes to file: %s. Transfer time: %f seconds\n", size, filename.c_str(), transferTime);

    delete[] buffer;
}

std::string EmulatedHDD::createHDD() {
    const std::string dirname = getHDDNameWithCurrentTime(); // Replace with desired directory path

    // Convert std::string to const char*
    const char* cdirname = dirname.c_str();

    // Create directory
    int status = mkdir(cdirname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (status == 0) {
        // std::cout << "Directory created successfully: " << dirname << std::endl;
        printf("Directory created successfully: %s", dirname.c_str());
    } else {
        // std::cerr << "Error: Failed to create directory" << std::endl;
        printf("Error: Failed to create directory");

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
    ss << std::setfill('0') << std::setw(4) << (tm->tm_year + 1900); // Year with 4 digits
    ss << std::setw(2) << (tm->tm_mon + 1);  // Month with leading zero
    ss << std::setw(2) << tm->tm_mday;       // Day with leading zero
    ss << "_";
    ss << std::setw(2) << tm->tm_hour;       // Hour with leading zero
    ss << std::setw(2) << tm->tm_min;        // Minute with leading zero
    ss << std::setw(2) << tm->tm_sec;        // Second with leading zero

    return ss.str();
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
