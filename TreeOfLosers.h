#ifndef TREEOFLOSER_H
#define TREEOFLOSER_H

#include "Record.h"
// #include <iostream>
#include <queue>


// Comparator function for priority queue
struct Compare {
    bool operator()(Record* n1, Record* n2) {
        return std::strcmp(n1->getKey(), n2->getKey()) > 0; // Min-heap
    }
};


class TreeOfLosers {
private:
    std::priority_queue<Record*, std::vector<Record*>, Compare> minHeap;

public:
    // Constructor
    TreeOfLosers();
    TreeOfLosers* clone();


    ~TreeOfLosers();

    //Insert(char* key) 
    void insert(Record* record);

    void deleteMin();

    Record* getMin();

    bool isEmpty();

    void print();
    void clear();


};

#endif // TREEOFLOSER_H
