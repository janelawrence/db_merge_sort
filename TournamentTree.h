#ifndef TOURNAMENTTREE_H
#define TOURNAMENTTREE_H

#include <iostream>
#include <vector>
#include <unordered_map>
#include "Record.h"
#include "Disk.h"

using namespace std;

const int GHOST_KEY = -1;

// Tournament Tree
class TournamentTree
{
private:
    // General class members
    std::vector<int> tree; // each node is a winner of a match
    Disk *disk;
    std::vector<Record *> records;
    std::vector<int> recIdx2TreeIdx; // keeps track of which leaf node a rec start competing
    void insert(int index, Record *record);
    int size;
    int numRecords;

    // Member used when runs are on DRAM as input
    std::vector<Run *> runTable;

    // Members used when runs are on Disk
    std::vector<int> runSizeTable;
    std::vector<Page *> pageTable;
    std::vector<int> runIdxTable;
    std::vector<int> nextPageIdxTable;
    const char *runPathPhysical;

    // Helper function to determine the winner of a match
    int compete(int idx);
    int compareRecordsInNodes(int node1, int node2);
    bool isGhostNode(int nodeWinner);
    void assignGhost();
    void fetchPageFromRunTable(int winnerRecIdx);
    void fetchPageFromRunOnDisk(int winnerRecIdx);

    void initialize();
    void initializeForRunsStoredDisk();

public:
    // Constructor (1)
    TournamentTree(int n, std::vector<Run *> &rTable);

    // Constructor (2)
    TournamentTree(int n, Disk *d,
                   std::vector<int> runIdxTable,
                   const char *rPathPhysical);

    ~TournamentTree();

    const char *getRecordKey(int node);

    // Function to update the result of a match
    void update(int index, Record *value);

    // check whether tree has next winner
    bool hasNext();

    // Replace winner with a ghost rec in the leaf level
    Record *popWinner();

    // Function to get the winner of the tournament
    // peek at winner
    Record *getWinner() const;

    void printTree() const;
    bool isFull() const;
    bool isEmpty() const;
};

#endif