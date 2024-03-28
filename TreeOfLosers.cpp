#include <iostream>
#include <queue>
#include <vector>
#include "Record.h"
#include "TreeOfLosers.h"

TreeOfLosers::TreeOfLosers(){}

TreeOfLosers::~TreeOfLosers(){
    while (!minHeap.empty()) {
        delete minHeap.top();
        minHeap.pop();
    }
}

// Method to insert a key into the tree
void TreeOfLosers::insert(Record * record) {
    minHeap.push(record);
}


// Method to delete the minimum key from the tree
void TreeOfLosers::deleteMin() {
    if (!minHeap.empty()) {
        Record* minRecord = minHeap.top();
        minHeap.pop();
        delete minRecord;
    } else {
        std::cout << "Tree is empty. No minimum to delete." << std::endl;
    }
}

// Method to get the minimum key from the tree
Record* TreeOfLosers::getMin() {
    if (!minHeap.empty()) {
        return minHeap.top();
    } else {
        std::cout << "Tree is empty. No minimum available." << std::endl;
        return nullptr; // Return a sentinel value indicating tree is empty
    }
}

bool TreeOfLosers::isEmpty(){
    return minHeap.empty();
}


// Main function for testing
// To test this main individually:
// use: g++ Record.cpp TreeOfLosers.cpp -o tree
// int main() {
//     TreeOfLosers tree;
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


//     // Get and print the minimum key again
//     while(!tree.isEmpty()) {
//         Record* curr = tree.getMin();
//         printf("Key: %s, slot: %d\n", curr->getKey(), curr->getSlot());
//         // std::cout << "Minimum key after deletion: " << tree.getMin()->key << std::endl;
//         tree.deleteMin();
//     }

//     return 0;
// }
