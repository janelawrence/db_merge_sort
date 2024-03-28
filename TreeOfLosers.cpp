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
// int main() {
//     TreeOfLosers tree;

//     // Insert some keys
//     tree.insert("alsdfe", 1);
//     tree.insert("eaer", 0);
//     tree.insert("ooo", 5);
//     tree.insert("aab", 2);
//     tree.insert("asefe", 7);

//     // Get and print the minimum key again
//     while(!tree.isEmpty()) {
//         Record* curr = tree.getMin();
//         printf("Key: %s, slot: %d\n", curr->key, curr->slot);
//         // std::cout << "Minimum key after deletion: " << tree.getMin()->key << std::endl;
//         tree.deleteMin();
//     }

//     return 0;
// }
