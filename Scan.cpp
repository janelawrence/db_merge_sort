#include "Scan.h"
#include <fstream>
#include <iterator>
#include <algorithm>
#include <iostream>
// #include <filesystem>
#include <sys/stat.h>
#include <cstring>

// namespace fs = boost::filesystem;

ScanPlan::ScanPlan(RowCount const count) : _count(count)
{
	TRACE(true);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan()
{
	TRACE(true);
	map.clear();
} // ScanPlan::~ScanPlan

// Return total number of pages used to stored input data
int ScanPlan::pagingInput(const char *INPUT_TXT, const char *LOCAL_INPUT_DIR)
{
	std::ifstream file(INPUT_TXT);
	int countDuplicate = 0;
	int countTotal = 0;
	int pageIdx = 0;

	char separator = get_directory_separator();

	if (file.is_open())
	{
		TRACE(true);
		std::string line;

		// Create a page file inside LOCAL_INPUT_DIR
		int bytesOccupied = 0;

		std::string pageFileName = std::to_string(pageIdx);
		std::string pageFilePath = std::string(LOCAL_INPUT_DIR) + separator + pageFileName;
		std::ofstream pageFile(pageFilePath.c_str(), std::ios::binary);

		if (!pageFile)
		{
			// printf("Fail to create file for page %d\n", pageIdx);
			std::cerr << "Error opening file for writing page" << pageIdx << " ." << std::endl;
			return 0;
		}

		while (std::getline(file, line))
		{
			// Remove non-alphanumeric characters from the line
			line.erase(std::remove_if(line.begin(), line.end(),
									  [](unsigned char c)
									  { return !std::isalnum(c); }),
					   line.end());
			countTotal++;
			if (map.find(line) != map.end())
			{
				countDuplicate++;
				continue;
			}
			// if current Page hasn't been filled
			if (PAGE_SIZE - bytesOccupied < recordSize)
			{
				// close current page file
				pageFile.close();

				// Create a new page file inside LOCAL_INPUT_DIR
				pageIdx++;
				pageFileName = std::to_string(pageIdx);
				pageFilePath = std::string(LOCAL_INPUT_DIR) + separator + pageFileName;
				pageFile.open(pageFilePath, std::ios::binary);
				bytesOccupied = 0;
				if (!pageFile)
				{
					std::cerr << "Error opening file for writing page" << pageIdx << " ." << std::endl;
					return 0;
				}
			}
			// write line to page file
			pageFile.write(line.c_str(), strlen(line.c_str()));
			pageFile << "\n";
			bytesOccupied += strlen(line.c_str());
			map[line] = 0;
		}
		pageFile.close();
	}
	else
	{
		printf("FILE cannot be opend\n");
		return 0;
	}
	printf("\n\ntotal records in input table (include dup): %d\n", countTotal);

	if (countTotal == 0)
	{
		return 0;
	}
	// Write numbers to trace file
	outputDuplicatesFound(outputTXT, countTotal, countDuplicate);
	return pageIdx + 1;
}

Run *ScanPlan::scan(const char *INPUT_TXT, const char *outputTXT)
{
	std::ifstream file(INPUT_TXT);
	Run *recordsInPages = new Run();
	int countDuplicate = 0;
	int countTotal = 0;
	if (file.is_open())
	{
		TRACE(true);
		std::string line;
		while (std::getline(file, line))
		{
			countTotal++;
			if (map.find(line) != map.end())
			{
				countDuplicate++;
				continue;
			}
			// Record *r = new Record(recordSize, line.c_str());
			recordsInPages->addRecord(new Record(recordSize, line));
			map[line] = 0;
		}
		file.close();
	}
	else
	{
		printf("FILE cannot be opend\n");
	}
	printf("\n\ntotal read in: %d\n", countTotal);

	// Write numbers to trace file
	outputDuplicatesFound(outputTXT, countTotal, countDuplicate);

	return recordsInPages;
}

// Run *ScanPlan::scanRun(const char *INPUT_TXT, const char *outputTXT)
// {
// 	std::ifstream file(INPUT_TXT);
// 	Run *recordsInRun = new Run();
// 	int countTotal = 0;
// 	if (file.is_open())
// 	{
// 		TRACE(true);
// 		std::string line;
// 		while (std::getline(file, line))
// 		{
// 			countTotal++;

// 			// Record *r = new Record(recordSize, line.c_str());
// 			recordsInRun->addRecord(new Record(recordSize, line.c_str()));
// 		}
// 		file.close();
// 	}
// 	else
// 	{
// 		printf("FILE cannot be opend\n");
// 	}

// 	return recordsInRun;
// }

int ScanPlan::outputDuplicatesFound(const char *outputTXT, int countTotal, int countDuplicate)
{
	// Open the output file in overwrite mode
	std::ofstream outputFile(outputTXT, std::ios::out);

	// Check if the file opened successfully
	if (!outputFile.is_open())
	{
		printf("File cannot be oppend");

		// std::cerr << "Error: Could not open file trace.txt for writing." << std::endl;
		return 1; // Return error code
	}

	// Print output to both console and file
	outputFile << "Number of Duplicates found: " << countDuplicate << " in " << countTotal << " input records\n";

	// close file
	outputFile.close();
	return 0;
}

Iterator *ScanPlan::init() const
{
	TRACE(true);
	return new ScanIterator(this);
} // ScanPlan::init

ScanIterator::ScanIterator(ScanPlan const *const plan) : _plan(plan), _count(0)
{
	TRACE(true);
} // ScanIterator::ScanIterator

ScanIterator::~ScanIterator()
{
	TRACE(true);
	traceprintf("produced %lu of %lu rows\n",
				(unsigned long)(_count),
				(unsigned long)(_plan->_count));
} // ScanIterator::~ScanIterator

bool ScanIterator::next()
{
	TRACE(true);

	if (_count >= _plan->_count)
		return false;

	++_count;
	return true;
} // ScanIterator::next

// int main()
// {
// 	std::unordered_map<std::string, int> map;
// 	map["hello"] = 1;
// 	map["world"] = 2;

// 	for (const auto &pair : map)
// 	{
// 		std::cout << pair.first << ": " << pair.second << std::endl;
// 	}

// 	return 0;
// }
