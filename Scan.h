#include "Iterator.h"
#include "Record.h"
#include "Run.h"
#include <vector>
#include <unordered_map>
#include <string>

class ScanPlan : public Plan
{
	friend class ScanIterator;
	std::unordered_map<std::string, int> map;

public:
	ScanPlan(RowCount const count);
	~ScanPlan();
	Iterator *init() const;
	int pagingInput(const char *INPUT_TXT, const char *LOCAL_INPUT_DIR);
	Run *scan(const char *INPUT_TXT, const char *outputTXT);
	int outputDuplicatesFound(const char *outputTXT, int countTotal, int countDuplicate);

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
