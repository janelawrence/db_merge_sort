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
   - Read F = (M/P - 3) pages from SSD to DRAM

### Teammate contributions

### Techniques and implementation breakdown

| Techniques                         | Where in Code                                                  |
| ---------------------------------- | -------------------------------------------------------------- |
| 1. Cache-size mini runs            | CACHE.cpp                                                      |
| ---------------------------------- | -------------------------------------------------------------- |
| 2. Device-optimized page sizes     | TO BE DONE                                                     |
| ---------------------------------- | -------------------------------------------------------------- |
| 3. Spilling memory-to-SSD          | TO BE DONE                                                     |
| ---------------------------------- | -------------------------------------------------------------- |
| 4. Spilling from SSD to disk       | TO BE DONE                                                     |
| ---------------------------------- | -------------------------------------------------------------- |
| 5. Graceful degradation            | TO BE DONE                                                     |
| a. into merging                    | TO BE DONE                                                     |
| b. beyond one merge step           | TO BE DONE                                                     |
| ---------------------------------- | -------------------------------------------------------------- |
| 6. Optimized merge patterns        | Using pointers to records instead of index                     |
| ---------------------------------- | -------------------------------------------------------------- |
| 7. Verifying: sort order           | TO BE DONE                                                     |
| a. sets of rows & values           | TO BE DONE                                                     |
| b. offset-value codes              | TO BE DONE                                                     |
| ---------------------------------- | -------------------------------------------------------------- |
| 8. Tournament Trees                | TreeOfLosers.h, TreeOfLosers.cpp                               |
