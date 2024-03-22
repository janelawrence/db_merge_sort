#include "Iterator.h"
#include "Scan.h"
#include "Filter.h"
#include "Sort.h"
#include "Record.h"
#include "EmulatedHDD.h"

#include <cstdlib> // For atoi function



int main (int argc, char * argv [])
{
	TRACE (true);

	//  Default value
	int numRecords = 2;
	int recordSize = 50; // 50 MB

	if (argc  >= 3) {
		numRecords = atoi(argv[1]);
		recordSize = atoi(argv[2]);
	}else {
        // If there are not enough command-line arguments, inform the user
        printf("\nUsage: %s <numRecords> <recordSize>\n", argv[0]);
        return 1; // Return with an error status
    }

	// Create a hdd to store unsorted records
	EmulatedHDD hdd("", 5, 100);

	// Generate numRecords number of records in HDD
	for(int i = 0; i < numRecords; i++) {
		Record record(recordSize, "");
		hdd.writeData(record.getKey(), recordSize);
	}

	Plan * const plan = new ScanPlan (7);
	// new SortPlan ( new FilterPlan ( new ScanPlan (7) ) );

	Iterator * const it = plan->init ();
	it->run ();
	delete it;

	delete plan;

	return 0;
} // main
