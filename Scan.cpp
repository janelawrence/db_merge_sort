#include "Scan.h"
#include <fstream>
#include <iterator>
#include <algorithm>

ScanPlan::ScanPlan(RowCount const count) : _count(count)
{
	TRACE(true);
} // ScanPlan::ScanPlan

ScanPlan::~ScanPlan()
{
	TRACE(true);
} // ScanPlan::~ScanPlan

Run *ScanPlan::scan(const char *INPUT_TXT)
{
	std::ifstream file(INPUT_TXT);
	Run *recordsInPages = new Run();

	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			if (map.find(line) != map.end())
			{
				continue;
			}
			map[line] = new Record(recordSize, line.c_str());
		}
		file.close();
	}
	std::unordered_map<std::string, Record *>::iterator it = map.begin();
	// 3 return the records in the hash table

	std::for_each(map.begin(), map.end(), [&recordsInPages](const std::pair<const std::string, Record *> &p)
				  { recordsInPages->addRecord(new Record(*p.second)); });
	return recordsInPages;
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
