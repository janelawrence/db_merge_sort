#include "Generator.h"
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <getopt.h>


Generator::Generator(int rSize, int nRecords) :recordSize(rSize), numRecords(nRecords){}

void Generator::generateRecords(const char* fileName) {
    // Open the file in binary mode
    std::ofstream outputFile(fileName, std::ios::binary);
    
    // Check if the file opened successfully
    if (!outputFile.is_open()) {
        printf("Error: Could not open file %s\n", fileName);
        return;
    }

    // Seed the random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Generate numRecords number of records
    for (int i = 0; i < numRecords * recordSize; ++i) {
        // Generate k random alphanumeric bytes
        char randomByte = 'A' + (std::rand() % 26); // Generate a random uppercase letter
        if (std::rand() % 2 == 0) { // Randomly choose between uppercase and lowercase
            randomByte = std::tolower(randomByte);
        }
        outputFile.write(&randomByte, sizeof(char)); // Write the byte to the file
    }
    
    // Close the file
    outputFile.close();
}

int main(int argc, char* argv[]) {
    int c;
    int recordSize = 0;
    int numRecords = 0;
    const char* fileName = nullptr;

   // Parse command-line options
    while ((c = getopt(argc, argv, "c:s:f:")) != -1) {
        switch (c) {
            case 'c':
                numRecords = std::atoi(optarg);
                break;
            case 's':
                recordSize = std::atoi(optarg);
                break;
            case 'f':
                fileName = optarg;
                break;
            default:
                printf("Usage: %s -c <numRecords> -s <recordSize> -f <fileName>\n", argv[0]);
                return 1;
        }
    }
    
     // Check if all required options are provided
    if (recordSize == 0 || numRecords == 0 || fileName == nullptr) {
        printf("Usage: %s -c <numRecords> -s <recordSize> -f <fileName>\n", argv[0]);
        return 1;
    }

    // Create an instance of Generator
    Generator generator(recordSize, numRecords);

    // Generate records
    generator.generateRecords(fileName);

    printf("Records generated successfully!\n");

    return 0;
}