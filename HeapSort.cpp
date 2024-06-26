#include <iostream>
#include <queue>
#include <vector>

#include "Record.h"
#include "HeapSort.h"

HeapSort::HeapSort() {}

HeapSort *HeapSort::clone()
{
    HeapSort *clonedTree = new HeapSort();
    clonedTree->minHeap = minHeap;
    return clonedTree;
}

HeapSort::~HeapSort()
{
    while (!minHeap.empty())
    {
        delete minHeap.top();
        minHeap.pop();
    }
}

// Method to insert a key into the tree
void HeapSort::insert(Record *record)
{
    minHeap.push(record);
}

// Method to delete the minimum key from the tree
void HeapSort::deleteMin()
{
    if (!minHeap.empty())
    {
        Record *minRecord = minHeap.top();
        minHeap.pop();
        // delete minRecord;
    }
    else
    {
        std::cout << "Tree is empty. No minimum to delete." << std::endl;
    }
}

// Method to get the minimum key from the tree
Record *HeapSort::getMin()
{
    if (!minHeap.empty())
    {
        return minHeap.top();
    }
    else
    {
        std::cout << "Tree is empty. No minimum available." << std::endl;
        return nullptr; // Return a sentinel value indicating tree is empty
    }
}

int HeapSort::getSize()
{
    return minHeap.size();
}

bool HeapSort::isEmpty()
{
    return minHeap.empty();
}

void HeapSort::print()
{
    if (isEmpty())
    {
        printf("Tree is empty\n");
    }
    std::priority_queue<Record *, std::vector<Record *>, Compare> copyHeap = minHeap;
    while (!copyHeap.empty())
    {
        Record *minRecord = copyHeap.top();
        minRecord->printRecord();
        copyHeap.pop();
    }
}

void HeapSort::clear()
{
    // Create an empty priority queue and swap its contents with minHeap
    std::priority_queue<Record *, std::vector<Record *>, Compare> emptyQueue;
    minHeap.swap(emptyQueue);
}

// Main function for testing
// To test this main individually:
// use: g++ Record.cpp HeapSort.cpp -o tree
// int main() {
//     HeapSort tree;
//     Record * r1 = new Record(20, "alsdfeei");
//     Record * r2 = new Record(20, "ewfeasdf");
//     Record * r3 = new Record(20, "bdfsewfh");
//     Record * r4 = new Record(20, "abcdasdf");
//     Record * r5 = new Record(20, "eeerghwr");

//     r1->setSlot(1);
//     r2->setSlot(2);
//     r3->setSlot(3);
//     r4->setSlot(4);
//     r5->setSlot(5);

//     // Insert some records
//     tree.insert(r1);
//     tree.insert(r2);
//     tree.insert(r3);
//     tree.insert(r4);
//     tree.insert(r5);

//     HeapSort* cloned = tree.clone();

//     // Get and print the minimum key again
//     // while(!cloned->isEmpty()) {
//     //     Record* curr = cloned->getMin();
//     //     printf("Key: %s, slot: %d\n", curr->getKey(), curr->getSlot());
//     //     cloned->deleteMin();
//     // }
//     tree.clear();
//     tree.print();

//     cloned->print();

//     return 0;
// }
