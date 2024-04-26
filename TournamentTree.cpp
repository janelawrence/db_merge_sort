#include <iostream>
#include <vector>
#include "Record.h"
#include "Run.h"
#include "TournamentTree.h"
#include <cstring>

bool TournamentTree::isGhostNode(int node)
{
    if (node >= 0 && node < tree.size())
    {
        return tree[node] == GHOST_KEY;
    }
    return true;
}

TournamentTree::TournamentTree(int n, std::vector<Run *> &rTable)
    : runTable(rTable)
{
    disk = nullptr;
    // Calculate the size of the tree based on the number of contestants
    size = 2;
    while (size < n)
    {
        size *= 2;
    }
    // Resize the tree to accommodate all matches
    tree.resize(2 * size);

    assignGhost();
    initialize();
}

TournamentTree::TournamentTree(int n, Disk *d,
                               std::vector<int> rIdxTable,
                               const char *rPathPhysical)
    : disk(d),
      runIdxTable(rIdxTable),
      runPathPhysical(rPathPhysical)
{
    // Calculate the size of the tree based on the number of contestants
    size = 2;
    while (size < n)
    {
        size *= 2;
    }
    // Resize the tree to accommodate all matches
    tree.resize(2 * size);

    assignGhost();
    initializeForRunsStoredDisk();
}

TournamentTree::~TournamentTree()
{
}

void TournamentTree::assignGhost()
{
    for (long unsigned int i = 0; i < tree.size(); i++)
    {
        tree[i] = GHOST_KEY;
    }
}

void TournamentTree::initialize()
{
    int totalBytesRead = 0;
    if (runTable.size() > 0)
    { // Reading pages from DRAM output buffer
        for (long unsigned int i = 0; i < runTable.size(); i++)
        {
            int fetchedPageSize = runTable[i]->getFirstPage()->getBytes();
            Record *r = runTable[i]->popFirstRecord();
            update(i, r);
            totalBytesRead += fetchedPageSize;
        }
    }
    else
    {
        printf("Input run table is empty, nothing to initialize\n");
    }
}

void TournamentTree::initializeForRunsStoredDisk()
{
    int totalBytesRead = 0;
    if (disk != nullptr && runIdxTable.size() > 0)
    { // Reading pages from Disk
        int i = 0;
        for (int runIdxOnDisk : runIdxTable)
        {
            // for run Idx on Disk, read first page using disk->readPageJFromRunK funciton
            //      after reading the page
            int totalPagesInRun = disk->getNumPagesInRunOnDisk(runPathPhysical, runIdxOnDisk);
            if (totalPagesInRun == 0)
            {
                continue;
            }
            // Fetch first page with pageIdx 0
            Page *fetchedPage = disk->readPageJFromRunK(runPathPhysical, runIdxOnDisk, 0);
            if (fetchedPage->isEmpty())
            {
                printf("Page fetched from disk is empty.\n");
                continue;
            }

            runSizeTable.push_back(totalPagesInRun);

            Record *r = new Record(*(fetchedPage->getFirstRecord()));
            fetchedPage->removeFisrtRecord();
            pageTable.push_back(fetchedPage);

            nextPageIdxTable.push_back(1);

            update(i, r); // <record inde in tree vector, record>
            int fetchedPageSize = fetchedPage->getBytes();
            totalBytesRead += fetchedPageSize;

            i++;
        }
        disk->outputReadSortedRunState(outputTXT);
        disk->outputAccessState(ACCESS_READ, totalBytesRead, outputTXT);
    }
    else
    {
        printf("Disk in nullptr, invalid disk input\n");
    }
}

// Function to update the result of a match
void TournamentTree::update(int index, Record *record)
{
    index += size;
    records.push_back(record);
    int recordIdx = records.size() - 1;
    tree[index] = recordIdx;
    recIdx2TreeIdx.push_back(index);

    // Update parent nodes
    while (index > 1)
    {
        index /= 2;
        // perforam competition upward
        compete(index);
    }
}

/*
Insert into index, and trigger competitions
 */
void TournamentTree::insert(int index, Record *record)
{
    // - Before looking up treeIdx of this rec
    // delete the mem allo to this record from records vector
    // and then assign the new one to the released mem
    if (records[index] != nullptr)
    {
        delete records[index];
        records[index] = nullptr;
    }
    records[index] = record;
    tree[index + size] = index;
    index += size;
    // TRACE(true);
    // Competing/Update parent nodes
    while (index > 1)
    {
        index /= 2;
        // perforam competition upward
        compete(index);
    }
}

const char *TournamentTree::getRecordKey(int node)
{
    return records[tree[node]]->getKey();
}

bool TournamentTree::hasNext()
{
    return tree[1] != GHOST_KEY;
}

/*
    Pop winner out and replace it with a ghost key/late fence
*/
Record *TournamentTree::popWinner()
{
    Record *winner = new Record(*getWinner());
    int winnerRecIdx = tree[1];
    if (winnerRecIdx == GHOST_KEY)
    {
        printf("Tree is empty, nothing to pop\n");
        return nullptr;
    }

    // Insert ghostKey into leaf idx of this winner
    int index = size + winnerRecIdx;

    tree[index] = GHOST_KEY;

    // And start competing until reach the top
    while (index > 1)
    {
        index /= 2;
        // perforam competition upward
        compete(index);
    }

    delete records[winnerRecIdx];
    records[winnerRecIdx] = nullptr;
    // Insert new record from runTable if needed
    // 2 cases: (1) when runs are stored in memory
    //          (2) when runs are stored from disk
    if (disk == nullptr)
    {
        fetchPageFromRunTable(winnerRecIdx);
    }
    else
    {
        fetchPageFromRunOnDisk(winnerRecIdx);
    }
    // if (!runTable[winnerRecIdx]->isEmpty())
    // {
    //     int fetchedPageSize = runTable[winnerRecIdx]->getFirstPage()->getBytes();
    //     Record *r = runTable[winnerRecIdx]->popFirstRecord();
    //     insert(winnerRecIdx, r);
    //     if (disk != nullptr)
    //     {
    //         disk->outputReadSortedRunState(outputTXT);
    //         disk->outputAccessState(ACCESS_READ, fetchedPageSize, outputTXT);
    //     }
    // }
    return winner;
}

void TournamentTree::fetchPageFromRunTable(int winnerRecIdx)
{
    if (!runTable[winnerRecIdx]->isEmpty())
    {
        int fetchedPageSize = runTable[winnerRecIdx]->getFirstPage()->getBytes();
        Record *r = runTable[winnerRecIdx]->popFirstRecord();
        insert(winnerRecIdx, r);
    }
}
void TournamentTree::fetchPageFromRunOnDisk(int winnerRecIdx)
{
    if (!pageTable[winnerRecIdx]->isEmpty())
    {
        Record *r = new Record(*(pageTable[winnerRecIdx]->getFirstRecord()));
        pageTable[winnerRecIdx]->removeFisrtRecord();
        insert(winnerRecIdx, r);
    }
    else
    {
        //  Fetch new page from the same Run on Disk
        int runIdxOnDisk = runIdxTable[winnerRecIdx];
        int pageIdx = nextPageIdxTable[winnerRecIdx];
        int fetchedPageSize = 0;
        if (pageIdx < runSizeTable[winnerRecIdx])
        {
            // Fetch page from disk
            Page *newPage = disk->readPageJFromRunK(runPathPhysical, runIdxOnDisk, pageIdx);
            if (!newPage->isEmpty())
            {
                delete pageTable[winnerRecIdx];
                fetchedPageSize = newPage->getSize();
                Record *r = new Record(*(newPage->getFirstRecord()));
                newPage->removeFisrtRecord();
                pageTable[winnerRecIdx] = newPage;
                insert(winnerRecIdx, r);
            }
        }
        nextPageIdxTable[winnerRecIdx]++;
        // Trace spilling
        if (fetchedPageSize > 0)
        {
            disk->outputReadSortedRunState(outputTXT);
            disk->outputAccessState(ACCESS_READ, fetchedPageSize, outputTXT);
        }
    }
}

int TournamentTree::compete(int node)
{
    int left = 2 * node;  // tree index
    int right = left + 1; // tree index
    bool leftIsGhost = isGhostNode(left);
    bool rightIsGhost = isGhostNode(right);

    if (right < tree.size() && left < tree.size())
    {
        if (leftIsGhost && rightIsGhost)
        {
            tree[node] = GHOST_KEY;
        }
        else if (leftIsGhost && !rightIsGhost)
        {
            tree[node] = tree[right];
        }
        else if (!leftIsGhost && rightIsGhost)
        {
            tree[node] = tree[left];
        }
        else
        {
            if (std::strcmp(getRecordKey(left), getRecordKey(right)) <= 0)
            {
                tree[node] = tree[left];
            }
            else
            {
                tree[node] = tree[right];
            }
        }
    }
    return 0;
}

int TournamentTree::compareRecordsInNodes(int node1, int node2)
{
    Record *r1 = records[tree[node1]];
    Record *r2 = records[tree[node2]];
    return std::strcmp(r1->getKey(), r2->getKey()) > 0 ? node2 : node1;
}

// Function to get the winner of the tournament
Record *TournamentTree::getWinner() const
{
    return records[tree[1]]; // Root node contains the winner
}

// NOTES: when using it in merge, there are cases
// where the tree is not full but we want to start prunning
bool TournamentTree::isFull() const
{
    return records.size() == size;
}

bool TournamentTree::isEmpty() const
{
    return records.size() == 0;
}

void TournamentTree::printTree() const
{
    for (long unsigned int i = 0; i < tree.size(); i++)
    {
        int recIdx = tree[i];
        if (recIdx != GHOST_KEY && tree[recIdx] < records.size())
        {
            if (records[recIdx] == nullptr)
            {
                printf("%d, ", GHOST_KEY);
                continue;
            }
            printf("%s, ", records[recIdx]->getKey());
        }
        else
        {
            printf("%d, ", GHOST_KEY);
        }
    }
    printf("\n");
}
