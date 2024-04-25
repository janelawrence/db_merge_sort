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

TournamentTree::TournamentTree(int n, std::vector<Run *> &rTable, Disk *d)
    : runTable(rTable), disk(d)
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
    initialize();
}

TournamentTree::~TournamentTree()
{
}

void TournamentTree::assignGhost()
{
    for (int i = 0; i < tree.size(); i++)
    {
        // tree[i].winner = std::numeric_limits<int>::max();
        tree[i] = GHOST_KEY;
    }
}

void TournamentTree::initialize()
{
    int totalBytesRead = 0;
    for (int i = 0; i < runTable.size(); i++)
    {
        int fetchedPageSize = runTable[i]->getFirstPage()->getBytes();
        Record *r = runTable[i]->popFirstRecord();
        update(i, r);
        totalBytesRead += fetchedPageSize;
    }
    if (disk != nullptr)
    {
        disk->outputReadSortedRunState(outputTXT);
        disk->outputAccessState(ACCESS_READ, totalBytesRead, outputTXT);
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

// Can be used to empty the tree by inserting ghost record
void TournamentTree::replaceWinner(Record *record)
{
    int winnerRecIndx = tree[1];
    if (winnerRecIndx == GHOST_KEY)
    {
        printf("Tree is empty, inserting to first place\n");
        update(0, record);
    }
    else
    {
        // The rec to be replaced is located at records[winnerRecIndx]
        insert(winnerRecIndx, record);
    }
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
    if (!runTable[winnerRecIdx]->isEmpty())
    {
        int fetchedPageSize = runTable[winnerRecIdx]->getFirstPage()->getBytes();
        Record *r = runTable[winnerRecIdx]->popFirstRecord();
        insert(winnerRecIdx, r);
        if (disk != nullptr)
        {
            disk->outputReadSortedRunState(outputTXT);
            disk->outputAccessState(ACCESS_READ, fetchedPageSize, outputTXT);
        }
    }
    return winner;
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
    for (int i = 0; i < tree.size(); i++)
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
