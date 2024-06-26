#include "CACHE.h"
#include "defs.h"
#include "Run.h"
#include "Disk.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <filesystem>
// #include <dirent.h>
#include <vector>

// Constructor
CACHE::CACHE(int cacheSize, int nPages) : MAX_CAPACITY(cacheSize), capacity(cacheSize), nPagesFitInCache(nPages) {}

CACHE::~CACHE()
{
	heap.clear();
}
int CACHE::outputMiniRunState(const char *outputTXT)
{
	// Open the output file in overwrite mode
	std::ofstream outputFile(outputTXT, std::ios::app);

	// Check if the file opened successfully
	if (!outputFile.is_open())
	{
		// std::cerr << "Error: Could not open file" << outputTXT << " for writing." << std::endl;
		printf("File is not open\n");
		return 1; // Return error code
	}

	// Print output to both console and file
	outputFile << "STATE -> SORT_MINI_RUNS: Sort cache-size mini runs\n";

	// close file
	outputFile.close();
	return 0;
}
std::vector<Run *> CACHE::sort(std::vector<Page *> pagesInDRAM, int maxRecordsInPage)
{

	std::vector<Run *> miniRuns;
	int count = 0;
	int pageIdx = 0;
	Record *prevWinner = nullptr;
	while (pageIdx < pagesInDRAM.size())
	{
		Page *curr = pagesInDRAM[pageIdx];
		if (count == nPagesFitInCache)
		{
			Run *miniRun = new Run(DRAM_PAGE_SIZE);
			// one cache-sized run has been filled
			while (!heap.isEmpty())
			{
				Record *winner = heap.getMin();
				heap.deleteMin();
				if(prevWinner == nullptr) 
				{
					prevWinner = winner;
				}
				else if(prevWinner->key + prevWinner->content == winner->key +winner->content) 
				{
					numDuplicate++;
					continue;
				}else{
					prevWinner = winner;
				}
				miniRun->addRecord(winner);
			}
			miniRuns.push_back(miniRun);

			count = 0;
			heap.clear();
		}

		while (curr->getNumRecords() > 0)
		{
			Record *record = curr->getFirstRecord();
			record->setSlot(-1);
			heap.insert(record);
			curr->removeFisrtRecord();
		}
		count++;
		pageIdx++;
		if (pageIdx == pagesInDRAM.size() && !heap.isEmpty())
		{
			Run *miniRun = new Run(DRAM_PAGE_SIZE);
			while (!heap.isEmpty())
			{
				Record *winner = heap.getMin();
				heap.deleteMin();

				if(prevWinner == nullptr) 
				{
					prevWinner = winner;
				}
				else if(prevWinner->key + prevWinner->content == winner->key +winner->content) 
				{
					numDuplicate++;
					continue;
				}else{
					prevWinner = winner;
				}
				miniRun->addRecord(winner);
			}

			miniRuns.push_back(miniRun);
		}
	}
	heap.clear();
	return miniRuns;
}

double CACHE::getCapacity() const
{
	return capacity;
}

// Output miniRuns for Graceful degradation
std::vector<Run *> CACHE::sortForGracefulDegradation(std::vector<Page *> pagesInDRAM,
													std::vector<Page *> pagesInCACHE,
													int maxRecordsInPage)
{
    for (long unsigned int i = 0; i < pagesInCACHE.size(); i++)
    {
        pagesInDRAM.push_back(pagesInCACHE[i]);
    }
	pagesInCACHE.clear();
	
    std::vector<Run *> miniRuns;
    int count = 0;
    int pageIdx = 0;
	Record * prevWinner = nullptr;
    while (pageIdx < pagesInDRAM.size())
    {
            Page *curr = pagesInDRAM[pageIdx];
            if (count == nPagesFitInCache)
            {
				// one cachesized run has been filled
				Run *miniRun = new Run(DRAM_PAGE_SIZE);
				while (!heap.isEmpty())
				{
					Record *winner = heap.getMin();
					heap.deleteMin();
					if(prevWinner == nullptr) 
					{
						prevWinner = winner;
					}
					else if(prevWinner->key + prevWinner->content == winner->key +winner->content) 
					{
						numDuplicate++;
						continue;
					}else{
						prevWinner = winner;
					}
					miniRun->addRecord(winner);
				}
				miniRuns.push_back(miniRun);

				count = 0;
				heap.clear();
            }

            while (curr->getNumRecords() > 0)
            {
				Record *record = curr->getFirstRecord();
				record->setSlot(-1);
				heap.insert(record);
				curr->removeFisrtRecord();
            }
            count++;
            pageIdx++;
            if (pageIdx == pagesInDRAM.size() && !heap.isEmpty())
            {
				Run *miniRun = new Run(DRAM_PAGE_SIZE);
				while (!heap.isEmpty())
				{
					Record *winner = heap.getMin();
					heap.deleteMin();
					if(prevWinner == nullptr) 
					{
						prevWinner = winner;
					}
					else if(prevWinner->key + prevWinner->content == winner->key +winner->content) 
					{
						numDuplicate++;
						continue;
					}
					else
					{
						prevWinner = winner;
					}
					miniRun->addRecord(winner);
				}

				miniRuns.push_back(miniRun);
			}
    }
    return miniRuns;
}