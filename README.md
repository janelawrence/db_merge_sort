# DB- Merge Sort

### What's currently working

1. `make run numberRecords=5 recordSize=50`
   Create a folder as the simulated HDD, and create 5 records in HDD, each with random bytes in size of 50. HDD reads, and writes in 5 ms latency, 100 MB/s bandwidth.

### New files added:

EmulatedHDD.cpp
EmulatedHDD.h

EmulatedSDD.cpp
EmulatedSDD.h

Record.cpp
Record.h
