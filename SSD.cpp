#include "SSD.h"
#include "DRAM.h"
#include "defs.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

// Constructor
SSD::SSD(double lat, double bw) : latency(lat), bandwidth(bw) {}

// Method to simulate read operation
void SSD::readData(double sizeInBytes) {
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    std::cout << "Read " << sizeInBytes << " bytes from SSD. Transfer time: " << transferTime << " seconds\n";
}

// Method to simulate write operation
void SSD::writeData(double sizeInBytes) {
    // Calculate transfer time based on bandwidth
    double transferTime = static_cast<double>(sizeInBytes) / bandwidth;

    // Simulate latency
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(latency)));

    std::cout << "Wrote " << sizeInBytes << " bytes to SSD. Transfer time: " << transferTime << " seconds\n";
}

std::vector<Run*> SSD::merge(std::vector<Run*> runs, int recordSize) {

    std::vector<Run*> output;
    if (runs.size() > 0) {
                // maximum number of records stored in a DRAM-size run
        int maxNumRecords = MAX_CAPACITY / recordSize;
        int numRuns = runs.size();
        int i = 0;
        // Keep track of slot index for run stored in buffer
        int currSlot = 0;
        int totalUsedRuns = 0;
        Run* buffer = new Run();
        while(true) {
            if(totalUsedRuns == numRuns) {
                break;
            }
            // Handles 2 cases: 
            // (1) num of input runs <= maxNumRecords, 
            //     tree has a winner when tree size == numRuns
            // (2) num of input runs > maxNumRecords, 
            //     tree has a winner when tree size == maxNumRecords
            if(tree.getSize() == min(numRuns, maxNumRecords)) {
                Record * winner = tree.getMin();
                int prevSlot = winner->getSlot();
                Run * prevRun = runs[prevSlot];
                // If buffer is full, write to output run list
                if(buffer->getSize() == maxNumRecords) {
                    output.push_back(buffer->clone());
                    // clear buffer
                    buffer->clear();
                    ++currSlot;

                }
                winner->setSlot(currSlot);
                buffer->add(new Record(*winner));
                tree.deleteMin();
                capacity += recordSize;
                if(!prevRun->isEmpty()){
                    tree.insert(new Record(*(prevRun->getFirst())));
                    capacity -= recordSize;
                    prevRun->removeFisrt();

                    // if prevRun becomes empty, inc counter
                    if(prevRun->isEmpty()) {
                        ++totalUsedRuns;
                    }
                }
                continue;

            }
            i = i % numRuns;
            Run * run = runs[i];
            if(run->isEmpty()) {
                i++;
                continue;
            }
            if(capacity >= recordSize) {
                // insert one record from runs[i] into tree
                tree.insert(new Record(*(run->getFirst())));
                run->removeFisrt();
                if(run->isEmpty()) {
                    totalUsedRuns++;
                }
                capacity -= recordSize;
            }
            i++;
        }
        // Check if tree still has Records
        while(!tree.isEmpty()) {
            Record * winner = tree.getMin();
            winner->setSlot(currSlot);
            // If buffer is full, write to output run list
            if(buffer->getSize() == maxNumRecords) {
                output.push_back(buffer->clone());
                // clear buffer
                buffer->clear();
                currSlot++;

            }
            buffer->add(new Record(*winner));
            tree.deleteMin();
        }

        if(!buffer->isEmpty()) {
            output.push_back(buffer->clone());
            // clear buffer
            buffer->clear();
        }
        printf("CurrSlot: %d, numRunsEmptied: %d\n", currSlot, totalUsedRuns);
    }
    return output;
}


double SSD::getCapacity() const {
    return capacity;
}

double SSD::getLatency() const {
    return latency;
}

double SSD::getBandwidth() const {
    return bandwidth;
}


// To test this main individually:
// use: g++ Run.cpp Record.cpp TreeOfLosers.cpp DRAM.cpp SSD.cpp -o ssd
// int main() {

//     TreeOfLosers tree1;
//     TreeOfLosers tree2;
//     TreeOfLosers tree3;

//     int recordSize = 20;
//     int numRecords = 15;

//     int totalBytes = recordSize * numRecords;

//     Record * r1 = new Record(recordSize, "alsdfeei");
//     Record * r2 = new Record(recordSize, "ewfeasdf");
//     Record * r3 = new Record(recordSize, "bdfsewfh");
//     Record * r4 = new Record(recordSize, "abcdasdf");
//     Record * r5 = new Record(recordSize, "eeerghwr");

//     Record * r6 = new Record(recordSize, "alsdmeei");
//     Record * r7 = new Record(recordSize, "ewfqesdf");
//     Record * r8 = new Record(recordSize, "alsdiefd");
//     Record * r9 = new Record(recordSize, "msjdfhwd");
//     Record * r10 = new Record(recordSize, "edprghwr");

//     Record * r11 = new Record(recordSize, "zlsdmeei");
//     Record * r12 = new Record(recordSize, "oeqesdfw");
//     Record * r13 = new Record(recordSize, "skfjhuwc");
//     Record * r14 = new Record(recordSize, "kviwudvg");
//     Record * r15 = new Record(recordSize, "aaabkkee");

//     r1->setSlot(0);
//     r2->setSlot(0);
//     r3->setSlot(0);
//     r4->setSlot(0);
//     r5->setSlot(0);

//     r6->setSlot(1);
//     r7->setSlot(1);
//     r8->setSlot(1);
//     r9->setSlot(1);
//     r10->setSlot(1);

//     r11->setSlot(2);
//     r12->setSlot(2);
//     r13->setSlot(2);
//     r14->setSlot(2);
//     r15->setSlot(2);




//     // Insert some records
//     tree1.insert(r1);
//     tree1.insert(r2);
//     tree1.insert(r3);
//     tree1.insert(r4);
//     tree1.insert(r5);

//     tree2.insert(r6);
//     tree2.insert(r7);
//     tree2.insert(r8);
//     tree2.insert(r9);
//     tree2.insert(r10);

//     tree3.insert(r11);
//     tree3.insert(r12);
//     tree3.insert(r13);
//     tree3.insert(r14);
//     tree3.insert(r15);

//     std::vector<TreeOfLosers*> runs;
//     runs.push_back(&tree1);
//     runs.push_back(&tree2);
//     runs.push_back(&tree3);



//     // Create an DRAM object with capacity 10GB, latency 0.1ms, and bandwidth 200MB/s
//     DRAM* dram = new DRAM();

//     std::vector<Run*> dramOutput = dram->merge(runs, recordSize);
//     for(int i = 0; i < dramOutput.size(); i++) {
// 		Run* run = dramOutput[i];
// 		printf("-------------DRAM Output %d th Run -----------\n", i);
// 		run->print();
// 		printf("\n");
// 	}

//     SSD* ssd = new SSD(0.0001, 200 * 1024 * 104);
    
//     ssd->writeData(totalBytes);

//     std::vector<Run*> ssdOutput = ssd->merge(dramOutput, recordSize);

//     for(int i = 0; i < ssdOutput.size(); i++) {
// 		Run* run = ssdOutput[i];
// 		printf("------------- SSD Output %d th Run -----------\n", i);
// 		run->print();
// 		printf("\n");
// 	}
//     // Simulate read and write operations
//     // dram->readData(1024); // Reading 1KB
//     // dram->writeData(1024); // Writing 1KB

//     return 0;
// }

