#ifndef HEAPSORT_H
#define HEAPSORT_H

#include "Record.h"
#include "Page.h"
// #include <iostream>
#include <queue>
#include <list>
#include <cstring>

// Comparator function for priority queue
struct Compare
{
    bool operator()(Record *n1, Record *n2)
    {
        return std::strcmp(n1->key.data(), n2->key.data()) > 0; // Min-heap
    }
};

class HeapSort
{
private:
    std::priority_queue<Record *, std::vector<Record *>, Compare> minHeap;

public:
    // Constructor
    HeapSort();
    HeapSort *clone();

    ~HeapSort();

    // Insert(char* key)
    void insert(Record *record);

    void deleteMin();

    Record *getMin();
    int getSize();

    bool isEmpty();

    void print();
    void clear();
};

#endif // HEAPSORT_H
