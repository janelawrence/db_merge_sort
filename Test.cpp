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
#include <getopt.h>
#include <fstream>


// Set global variable

unsigned long long CACHE_SIZE = 1ULL * 1024 * 1024; //1 MB
unsigned long long DRAM_SIZE = 100ULL * 1024 * 1024; //100 MB
unsigned long long SSD_SIZE = 10ULL * 1024 * 1024 * 1024; //1 GB
char * INPUT_TXT = "input.txt";
// char * INPUT_TXT = "input_table";


long SSD_LAT = 100; // 0.1 ms = 100 microseconds(us)
unsigned long long SSD_BAN = 200ULL * 1024 * 1024 / 1000000; // 200 MB/s = 200 MB/us

long HDD_LAT = 500; // 5 ms = 0.005 s
unsigned long long HDD_BAN = 100 * 1024 * 1024/ 1000000; // 100 MB/s = 100 MB/us

int PAGE_SIZE = 20480; //20KB



// Function to read records from the input file
std::vector<std::string> readRecords(const char* fileName, int recordSize, int numRecords) {
    std::ifstream inputFile(fileName);
    std::vector<std::string> records;

    if (!inputFile.is_open()) {
        printf("Error: Could not open file %s\n", fileName);
        return records; // Return an empty vector
    }

    std::string record;
    for (int i = 0; i < numRecords; ++i) {
        std::getline(inputFile, record); // Read a line from the file
        if (record.size() != recordSize) {
            printf("Error: Record size mismatch at line %d\n", i + 1);
            records.clear(); // Clear the vector and return
            return records;
        }
        records.push_back(record); // Store the record in the vector
    }

    inputFile.close();
    return records;
}

int main (int argc, char * argv [])
{
	int c;
	// TRACE (true);
	int numRecords;
	int rSize;
	const char* outputFile = nullptr;

   // Parse command-line options
    while ((c = getopt(argc, argv, "c:s:o:")) != -1) {
        switch (c) {
            case 'c':
                numRecords = std::atoi(optarg);
                break;
            case 's':
                rSize = std::atoi(optarg);
                break;
            case 'o':
                outputFile = optarg;
                break;
            default:
                printf("Usage: %s -c <numRecords> -s <recordSize> -o <outputFileName>\n", argv[0]);
                return 1;
        }
    }
	const int recordSize = rSize;
	 // Check if all required options are provided
    if (recordSize == 0 || numRecords == 0 || outputFile == nullptr) {
        printf("Usage: %s -c <numRecords> -s <recordSize> -o <outputFileName>\n", argv[0]);
        return 1;
    }

	// Calculate stats
	// size_t totalBytes = numRecords * recordSize;

	int nRecordsInPage = PAGE_SIZE/recordSize;
	int nPagesFitInCache = CACHE_SIZE / PAGE_SIZE;

	int nBuffersDRAM = DRAM_SIZE / PAGE_SIZE;
	int nBuffersReserved = 3;
	// Fan-in F
	int nInputBuffersDRAM = nBuffersDRAM - nBuffersReserved; // reserve 3 page according to lecture notes

	int nBuffersSSD = SSD_SIZE/ PAGE_SIZE;
	int nInputBuffersSSD = nBuffersSSD - nBuffersReserved; // reserve 3 page according to lecture notes

	printf("--------------------------------------------System stats--------------------------------------\n");
	printf("Number of records: %d, "
			"Record size: %d\n\n", 
			numRecords, recordSize);
	printf("Page size: %d Bytes\n"
			"- Each page can store %d records\n\n",
			 PAGE_SIZE, nRecordsInPage);
	printf("Cache size: %llu Bytes\n"
			"- Each cache-sized run can store %d pages\n"
			"- Each cache-sized run can store %d records\n\n",
			CACHE_SIZE, nPagesFitInCache, nRecordsInPage * nPagesFitInCache);
	printf("DRAM size: %llu Bytes\n"
			"- Each DRAM-sized run can store %d pages\n"
			"---- %d are input buffers, %d are reserved buffers\n"
			"- Each DRAM-sized run can store %d records\n\n",
			DRAM_SIZE, nBuffersDRAM, 
			nInputBuffersDRAM, nBuffersReserved, 
			nInputBuffersDRAM * nRecordsInPage);
	printf("SSD size: %llu Bytes\n"
			"- Each SSD-sized run can store %d pages\n"
			"---- %d are input buffers,  %d are reserved buffers\n"
			"- Each SSD-sized run can store %d records\n\n",
			SSD_SIZE, nBuffersSSD, 
			nInputBuffersSSD, nBuffersReserved,
			nInputBuffersSSD * nRecordsInPage);

	//  Read input.txt = input data stored in HDD
	std::ifstream inputFile(INPUT_TXT, std::ios::binary);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Could not open file " << INPUT_TXT << std::endl;
        return 1;
    }

	// Get the file size
    inputFile.seekg(0, std::ios::end);
    size_t totalBytes = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

	printf("Size: %llu\n", totalBytes);

    // Allocate buffer to hold the data
    char* inputData = new char[totalBytes];

    // Read data from the file
    inputFile.read(inputData, totalBytes);

    if (inputFile.fail()) {
        std::cerr << "Error while reading file " << INPUT_TXT << std::endl;
        delete[] inputData;
        return 1;
    }

    // Print or process the read data here


    // Don't forget to close the file and deallocate the buffer
    inputFile.close();

	char rawRecord[recordSize];

	while (inputFile.read(rawRecord, recordSize)) {
		for (int i = 0; i < recordSize; ++i) {
			printf("%c", rawRecord[i]);
		}
		printf("\n");

		Record *record = new Record(recordSize, rawRecord);
		record->printRecord();
		// Process or print the record here
	}
	
	for (size_t i = 0; i < totalBytes; i += recordSize) {
        // Print k bytes starting from index i
        for (size_t j = 0; j < recordSize && (i + j) < totalBytes; ++j) {
            // printf("%c", inputData[i + j]);
			rawRecord[j] = inputData[i + j];
        }
		Record* record = new Record(recordSize, rawRecord);
		record->printRecord();
        // printf("\n"); // Add a newline after printing each set of k bytes
    }

	//  delete dynamically allocated memory before exit program
	delete[] inputData;

	return 0;
}
