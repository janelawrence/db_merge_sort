#include "Scan.h"
#include <fstream>
#include <iterator>
#include <algorithm>
#include <iostream>

ScanPlan::ScanPlan(RowCount const count) : _count(count)
{
	TRACE(true);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan()
{
	TRACE(true);
} // ScanPlan::~ScanPlan

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
			recordsInPages->addRecord(new Record(recordSize, line.c_str()));
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
