#ifndef TREEOFLOSER_H
#define TREEOFLOSER_H

#include "Record.h"
// #include <iostream>
#include <cstring>
#include <queue>

// Node structure for Tree of Losers
// struct Node {
//     const char * key;
//     Node* left;
//     Node* right;
//     int slot;

//     Node(const char* k, int slotNum) : key(k), left(nullptr), right(nullptr), slot(slotNum) {}
// };

// Comparator function for priority queue
struct Compare {
    bool operator()(Record* const& n1, Record* const& n2) {
        return std::strcmp(n1->getKey(), n2->getKey()) > 0; // Min-heap
    }
};


class TreeOfLosers {
private:
    std::priority_queue<Record*, std::vector<Record*>, Compare> minHeap;

public:
    // Constructor
    TreeOfLosers();

    ~TreeOfLosers();

    //Insert(char* key) 
    void insert(Record* record);

    void deleteMin();

    Record* getMin();

    bool isEmpty();

};

#endif // TREEOFLOSER_H
