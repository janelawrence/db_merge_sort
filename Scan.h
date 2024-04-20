#include "Iterator.h"
#include "Record.h"
#include "Run.h"
#include <vector>
#include <unordered_map>
#include <string>

class ScanPlan : public Plan
{
	friend class ScanIterator;
	std::unordered_map<std::string, Record *> map;

public:
	ScanPlan(RowCount const count);
	~ScanPlan();
	Iterator *init() const;
	Run *scan(const char *INPUT_TXT);

private:
	RowCount const _count;
}; // class ScanPlan

class ScanIterator : public Iterator
{
public:
	ScanIterator(ScanPlan const *const plan);
	~ScanIterator();
	bool next();

private:
	ScanPlan const *const _plan;
	RowCount _count;
}; // class ScanIterator
