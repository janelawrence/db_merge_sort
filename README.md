# DB- Merge Sort

### What's currently working

1. `make run`, equivalent to `./ExternalSort.exe -c 20 -20 -o trace0.txt`
   prints out all system statistics, and prints the records store in input.txt
2. Or use the shell script file by `bash run.sh`

3. To change command line args, modify numbers in `line 43 in MakeFile`

### Steps:

M = DRAM_SIZE  
C = CACHE_SIZE  
R = recordSize  
N = numRecords  
I = totalInputBytes = R \* N  
P = PAGE_SIZE  
n = recordsPerPage  
F = Fan-in = number of pages read to memory = (M/P - 3)

1. For every M = DRAM_SIZE data, read M/P pages into DRAM
2. Use Cache to create mini-runs
   - Number of pages can fit in cache = C/P
   - Number of records can fit in cache =
     (pages can fit in cache) \* (records in a page)= (C/P) \* (P/R)
3. Write mini-runs to DRAM, then write them to SSD
4. State: Merge Runs on SSD:
   - Read F = (M/P - 2) pages from SSD to DRAM

### Teammate contributions

| Techniques                         | contributions                                                  |
| ---------------------------------- | -------------------------------------------------------------- |
| 1. Cache-size mini runs            | (Jane)                                                         |
| ---------------------------------- | -------------------------------------------------------------- |
| 2. Minimum count of row            | (Jane)                                                         |
| ---------------------------------- | -------------------------------------------------------------- |
| 3. Device-optimized page sizes     | WRITE ANALYSIS in next section                                 |
| ---------------------------------- | -------------------------------------------------------------- |
| 4. Spilling memory-to-SSD          | Done                                                           |
| ---------------------------------- | -------------------------------------------------------------- |
| 5. Spilling from SSD to disk       | Done                                                           |
| ---------------------------------- | -------------------------------------------------------------- |
| 6. Graceful degradation            | TO BE DONE                                                     |
| a. into merging                    | TO BE DONE                                                     |
| b. beyond one merge step           | TO BE DONE                                                     |
| ---------------------------------- | -------------------------------------------------------------- |
| 7. Optimized merge patterns        | Using pointers to records instead of index (explain)           |
| ---------------------------------- | -------------------------------------------------------------- |
| 8. Verifying: sort order           | (Jane) Test.cpp -> verityOrder()                               |
| a. sets of rows & values           |                                                                |
| ---------------------------------- | -------------------------------------------------------------- |
| 9. Tournament Trees                | (Jane)TournamenTree.h, TournamenTree.cpp                       |
| ---------------------------------- | -------------------------------------------------------------- |
| 10. Duplicate Removel              | (Jane) TO BE DONE                                              |

### Explain

1. Cache-size mini runs
2. Minimum count of row
3. Device-optimized page sizes
4. Spilling memory-to-SSD
5. Spilling from SSD to disk
6. Graceful degradation
7. Optimized merge patterns
   a. Using pointers to records instead of index (explain)
8. Verifying: sort order
9. Tournament Trees
10. Duplicate Removel
