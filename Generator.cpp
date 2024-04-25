#include "Generator.h"
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <getopt.h>
#include <chrono>
#include <iostream>

Generator::Generator(int rSize, int nRecords) : recordSize(rSize), numRecords(nRecords) {}

void displayProgressBar(int currentStep, int totalSteps)
{
    const int barWidth = 50;

    std::cout << "[";
    int pos = barWidth * currentStep / totalSteps;
    for (int i = 0; i < barWidth; ++i)
    {
        if (i < pos)
            std::cout << "=";
        else if (i == pos)
            std::cout << ">";
        else
            std::cout << " ";
    }
    std::cout << "] " << int((currentStep * 100.0) / totalSteps) << " %\r";
    std::cout.flush();
}

void Generator::generateRecords(const char *fileName)
{
    // Open the file in binary mode
    std::ofstream outputFile(fileName, std::ios::binary);

    // Check if the file opened successfully
    if (!outputFile.is_open())
    {
        printf("Error: Could not open file %s\n", fileName);
        return;
    }

    // Seed the random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    std::cout << "\n";
    // Generate numRecords number of records
    for (int i = 0; i < numRecords; ++i)
    {
        displayProgressBar(i + 1, numRecords);

        for (int j = 0; j < recordSize; j++)
        {
            // Generate k random alphanumeric bytes
            char randomByte = 'A' + (std::rand() % 26); // Generate a random uppercase letter
            if (std::rand() % 2 == 0)
            { // Randomly choose between uppercase and lowercase
                randomByte = std::tolower(randomByte);
            }
            outputFile.write(&randomByte, sizeof(char)); // Write the byte to the file
        }
        outputFile << '\n'; // Add a newline after writing each record
    }

    // Close the file
    outputFile.close();
}

void Generator::generateWitDupRecords(const char *fileName)
{
    // Open the file in binary mode
    std::ofstream outputFile(fileName, std::ios::binary);

    // Check if the file opened successfully
    if (!outputFile.is_open())
    {
        printf("Error: Could not open file %s\n", fileName);
        return;
    }

    // Seed the random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Generate numRecords number of records
    for (int i = 0; i < numRecords;)
    {
        char record[recordSize];
        for (int j = 0; j < recordSize; j++)
        {
            char randomByte = 'A' + (std::rand() % 26); // Uppercase A-Z
            if (std::rand() % 2 == 0)
                randomByte = std::tolower(randomByte); // Convert to lowercase

            record[j] = randomByte; // Store the byte in the record
        }
        outputFile.write(record, recordSize); // Write the full record to the file
        outputFile << '\n';

        bool createDup = (std::rand() % 2) == 1;
        if (createDup)
        {
            int numDuplicate = std::rand() % 5;
            for (int k = 0; k < numDuplicate && (i + k + 1) < numRecords; k++)
            {
                outputFile.write(record, recordSize);
                outputFile << '\n';
            }
            i += numDuplicate + 1; // Update i to reflect duplicates written
        }
        else
        {
            i++;
        }
    }

    // Close the file
    outputFile.close();
}

int main(int argc, char *argv[])
{
    int c;
    int recordSize = 0;
    int numRecords = 0;
    const char *fileName = nullptr;
    int duplicate = 0;

    // Parse command-line options
    while ((c = getopt(argc, argv, "c:s:f:p:")) != -1)
    {
        switch (c)
        {
        case 'c':
            numRecords = std::atoi(optarg);
            break;
        case 's':
            recordSize = std::atoi(optarg);
            break;
        case 'f':
            fileName = optarg;
            break;
        case 'p':
            duplicate = std::atoi(optarg);
            break;
        default:
            printf("Usage: %s -c <numRecords> -s <recordSize> -f <fileName>\n", argv[0]);
            return 1;
        }
    }

    // Check if all required options are provided
    if (recordSize == 0 || numRecords == 0 || fileName == nullptr)
    {
        printf("Usage: %s -c <numRecords> -s <recordSize> -f <fileName>\n", argv[0]);
        return 1;
    }
    printf("dup: %d\n", duplicate);

    // Create an instance of Generator
    Generator generator(recordSize, numRecords);

    if (duplicate == 0)
    {
        // Generate records
        generator.generateRecords(fileName);
    }
    else
    {
        printf("creating records containing dup\n");
        generator.generateWitDupRecords(fileName);
    }

    printf("\nRecords generated successfully!\n\n");

    return 0;
}