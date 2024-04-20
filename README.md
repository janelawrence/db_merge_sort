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
F = Fan-in = number of pages read to memory = (M/P - 2)

1. For every M = DRAM_SIZE data, read M/P number of pages into DRAM
2. Use Cache to create mini-runs
   - Number of pages can fit in cache = C/P
   - Number of records can fit in cache =
     (pages can fit in cache) \* (records in a page)= (C/P) \* (P/R)
3. Write mini-runs to DRAM, merge in memory then write mem-sized runs to SSD, and then HDD
4. State: Merge Runs on HDD:
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
| 10. Duplicate Removel              | (Jane) Scan.cpp -> Run *ScanPlan::scan(const char *INPUT_TXT)  |

### Explain

#### Method used: alternative 1

1. Cache-size mini runs

- Cache.cpp -> sort(), creates sorted mini runs and in each run, records are store in pages

2. Minimum count of row
3. Device-optimized page sizes
4. Spilling memory-to-SSD
5. Spilling from SSD to disk
6. Graceful degradation
7. Optimized merge patterns
   - Using pointers to records instead of index (explain)
8. Verifying: sort order

- Test.cpp -> verityOrder()

9. Tournament Trees
10. Duplicate Removel

- In `Scan.cpp -> Run *ScanPlan::scan(const char *INPUT_TXT)`
  It scan through the input table txt, and use a hashmap to store
  hashed record data. When hashed value has existed in the hashmap,
  skip reading in this record, and continue.
  Returns records stored in pages and wrapped in a wrapper class Run.
