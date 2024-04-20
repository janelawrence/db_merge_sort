#include <iostream>
#include <vector>
#include "Record.h"
#include "Disk.h"

using namespace std;

const int GHOST_KEY = -1;

// Tournament Tree
class TournamentTree
{
private:
    std::vector<int> tree; // each node is a winner of a match

    Disk *disk;

    std::vector<Record *> records;
    std::vector<Run *> runTable;
    std::vector<int> recIdx2TreeIdx; // keeps track of which leaf node a rec start competing
    void insert(int index, Record *record);
    int size;
    int numRecords;

    // Helper function to determine the winner of a match
    int compete(int idx);
    int compareRecordsInNodes(int node1, int node2);
    bool isGhostNode(int nodeWinner);
    void assignGhost();
    void initialize();

public:
    // Constructor
    TournamentTree(int n, std::vector<Run *> &rTable, Disk *d);

    ~TournamentTree();

    const char *getRecordKey(int node);

    // Function to update the result of a match
    void update(int index, Record *value);

    void replaceWinner(Record *record);

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