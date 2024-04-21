#ifndef HEAPSORT_H
#define HEAPSORT_H

#include "Record.h"
#include "Page.h"
// #include <iostream>
#include <queue>
#include <list>

struct Node
{
    Record *winner;
    Record *left;
    Record *right;
};

struct CompareNode
{
    bool operator()(Node n1, Node n2)
    {
        return std::strcmp(n1.winner->getKey(), n2.winner->getKey()) > 0; // Min-heap
    }
};

// Comparator function for priority queue
struct Compare
{
    bool operator()(Record *n1, Record *n2)
    {
        return std::strcmp(n1->getKey(), n2->getKey()) > 0; // Min-heap
    }
};

class HeapSort
{
private:
    int size;
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

    std::list<Record *> toList();
    std::vector<Record *> toVector();
    Page *toNewPages(int pageIdx, int maxRecordsInPage, int pageSize);
};

#endif // HEAPSORT_H
