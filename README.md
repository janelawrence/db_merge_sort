# DB- Merge Sort

### What's currently working

1. 
`make`
`./ExternalSort.exe -c <numRecords> -<recordSize> -o <trace0.txt>`
   
2. Or modify and the use the shell script file by `bash run.sh`

3. To change command line args, modify numbers in `line 43 in MakeFile`

### Code Structure

CACHE.h/CACHE.cpp: Implements the cache system for sorting data. It manages the simulation of memory caching, sorting, and processing of data.

Run.h/Run.cpp: Manages sequences of records (runs) that are processed and sorted by the cache or tournament tree.

Page.h/Page.cpp: Represents a page of records, handling the collection of records as per the page size.

Record.h/Record.cpp: Defines the data structure for individual records within pages.

HeapSort.h/HeapSort.cpp: Implements the sorting algorithm used by the cache to sort records.

disk.h/disk.cpp: Simulates a hard disk drive for storage and retrieval of data.

TournamentTree.h/TournamentTree.cpp: Implements the tournament tree logic including node comparisons, record management, and tree operations.

### Steps:

Memory access latency: 6375 nanoseconds
Memory bandwidth: 1.0016 GB/s
Page Size roughly = 6375 nanoseconds \* 1.0016 GB/m = roughly 6.7 KB = take 8 KB = 8192 B

M = DRAM_SIZE  
C = CACHE_SIZE  
R = recordSize  
N = numRecords  
I = totalInputBytes = R \* N  
P = DRAM_PAGE_SIZE  
SSD_P = SSD_PAGE_SIZE
HDD_P = HDD_PAGE_SIZE
n = recordsPerPage  
D = Number of pages read to memory = (M - P*numOutputBuffersInDRAM) / P

Alternative 1 External Merge Sort Algorithm:
1. For every M display(Markdown('$1.0-q \\approx 0.99,0.5,0.0$')) DRAM_SIZE data, read D pages into DRAM
2. Use Cache to create mini-runs
   - Number of pages can fit in cache = C/P
   - Number of records can fit in cache =
     (pages can fit in cache) \* (records in a page)= (C/P) \* (P/R)
3. Merge mini-runs in DRAM output buffers using tournament tree, when DRAM output buffers are full,
   then spill records to memory-sized runs on SSD output buffers.
   If SSD output buffers are full, spill all mem-sized runs located in SSD output buffers to HDD,
   then continue storing memory-sized runs on SSD output buffers.
5. State: Merge Runs on HDD:
   - Create SSD-Sized runs by merging memory-sized runs
   - Merge all SSD-Sized runs into one final sorted run

### Teammate contributions

| Techniques                         | contributions                                                  |
| ---------------------------------- | -------------------------------------------------------------- |
| 1. Cache-size mini runs            | (Jane)                                                         |
| ---------------------------------- | -------------------------------------------------------------- |
| 2. Minimum count of row            | (Jane)                                                         |
| ---------------------------------- | -------------------------------------------------------------- |
| 3. Device-optimized page sizes     | (Jane)                                                         |
| ---------------------------------- | -------------------------------------------------------------- |
| 4. Spilling memory-to-SSD          | (Jane)                                                         |
| ---------------------------------- | -------------------------------------------------------------- |
| 5. Spilling from SSD to disk       | (Jane)                                                         |
| ---------------------------------- | -------------------------------------------------------------- |
| 6. Graceful degradation            | (Jane)                                                         |
| ---------------------------------- | -------------------------------------------------------------- |
| 7. Optimized merge patterns        | (Jane) Using pointers to records instead of index (explain)           |
| ---------------------------------- | -------------------------------------------------------------- |
| 8. Verifying: sort order           | (Jane) Test.cpp -> verityOrder()                               |
| a. sets of rows & values           |                                                                |
| ---------------------------------- | -------------------------------------------------------------- |
| 9. Tournament Trees                | (Jane)TournamenTree.h, TournamenTree.cpp                       |
| ---------------------------------- | -------------------------------------------------------------- |
| 10. Duplicate Removal              | (Jane) Remove duplicates during merge process                  |
| 11. Report              | (Jane and Ziqi) writing README.md to explain project design and implementation                  |

### Explain

#### Method used: alternative 1

1. Cache-size mini runs

- Cache.cpp -> sort(), creates sorted mini runs and in each run, records are store in pages of DRAM_PAGE_SIZE
- Each mini-run is sorted using the 'HeapSort' algorithm and managed by 'Run' class.
- Records are processed page by page, with sorting occurring at the record level.

API Reference
CACHE::sort(std::vector<Page*>, int, int): Sorts pages into mini-runs.
CACHE::outputMiniRunState(const char*): Wrtie trace outputs reporting the state of sorted mini-runs.
CACHE::getCapacity() const: Returns the current capacity of the cache.

2. Minimum count of row

- Based on optmized page size, and size of cache, determine the size of output buffer in DRAM
  - Since size of Cache is 1 MB, size of DRAM is 100 MB, DRAM can store at most 99 cache-sized mini runs
    if using the rest of 1 MB as the output buffers, it can store 128 output buffers, each output buffer has one page of 8 KB. Based on the input record size ranging from 20 Bytes to 2000 Bytes, these 128 Pages can store 524 to 52428 records.  
    Since the output buffer of DRAM will be used to stored the tournament tree used for merging, it needs to have
    the capacity to store one records from each of the cache-sized runs (at most 99 of them). Based on our calculation above, reserving 1 MB as the output buffer is more than enough to store maximum of 99 records, of each from a cache-sized runs. Therefore, at least need 128/4 = 32 output buffers (each output buffer has one page) in the output buffers such that it can store 131 to 13107 records.
    
- Based on the number output buffers = 32, we can calculate the total size of space in DRAM can be used to store
  input data at once = 100 MB - 32 \* 8K = 100 MB - 0.25 MB = 99.75 MB = 99.75 MB / (8 KB/ Page) = 12768 pages.
- Therefore, each time, DRAM can store 12768 pages \* 8 KB/page / recordSize =

3. Device-optimized page sizes = latency * bandwidth

- DRAM Page Size roughly = 6375 nanoseconds \* 1.0016 GB/m = roughly 6.7 KB = take 8 KB = 8192 B

- SSD Page Size roughly = 200 MB/s * 0.1 ms = 20 KB

- HDD Page Size roughly = 100 MB/s * 5 ms = 512 KB

4. Spilling memory-to-SSD

- DRAM.h/DRAM.cpp - mergeFromSelfToDest() handle the spilling from DRAM to SSD.
  **- **In the scenario when input data exceeds memory size and must be written to a second storage medium

The process involves:

- Checking if the DRAM output buffer is full.

- Simulating writing to the SSD (represented by an object `dest`, which is a Disk class object) when the buffer is full.

- Managing internal buffers and ensuring they are flushed to SSD properly.

- Physically: 
  a. all memory-sized runs are stored in a folder inside directory `LOCAL_DRAM_SIZED_RUNS_DIR`    =`mem_sized_runs`.

5. Spilling from SSD to HDD

- When SSD output buffers are full, spill all runs from SSD to HDD

- When all memory-sized runs are created, but they are still in SSD,
  spill them to HDD before merge

- Physically: 
  a. all memory-sized runs are stored in a folder inside directory `LOCAL_DRAM_SIZED_RUNS_DIR`    =`mem_sized_runs`.
  In each memory-sized run run located in, there are page files of SSD_PAGE_SIZE.
  When the program is fetching 1 page on HDD for merging memory-sized runs, it will fetch HDD_PAGE/SSD_PAGE_SIZE number of
  memory sizes from this local directory. 
  b. all SSD-Sized runs are stored in a folder inside `LOCAL_SSD_SIZED_RUNS_DIR` = `ssd_sized_runs`;
  Each it stores floor(SSD_SIZE/DRAM_SIZE) number of runs in this folder

- Test.cpp:

6. Graceful degradation
   According to the paper, it uses a threshold 0.01.
   In line x of Test.cpp, when it's the second to last pass of reading the input table, the program
   calculates the ratio of `number of pages left to read in the input table` to `current number of pages loaded in DRAM`. If the ratio is smaller or equal to 0.01, the program will then perform graceful degradation in the
   following steps:

   - From the DRAM, it will spill pages to SSD's input buffers until DRAM has enough space to store
     `number of pages left to read in the input table`.
   - Then DRAM will read the last few data: `number of pages left to read in the input table`.
   - Then the data spilled to SSD earlier will be read and loaded into Cache.
     Since we use the ratio 0.01, there are 12768 input buffers. When all 12768 input buffers are used in
     the second to last run, if the ratio is 0.01, which means there are 0.01 \* 12768 = 127 pages of records
     left in the input table. Since cache size is 1 MB = 1MB / (8KB/ Page) = 128 pages, this means cache will
     also have enough space to store this spilled data when it's empty.
   - After reading the spilled data resulted from performing graceful degradation from SSD into Cache. We now
     using Graceful Degradation to successfully store data more than DRAM can fit.
     Then we continue use the heap sort to create Cache-sized sorted mini runs.
     Then, use the tournament tree in output buffers to merge thse cache-sized sorted mini runs into memory-sized
     runs.

  - Where in Code do we realize it:
    a. In secord to last run, decide whether to do Graceful Degradation:
    b. Clear space in DRAM input buffers to SSD just enough to store `number of pages left to read in the input table`. Calcuate the ratio `double ratio = (double)pagesLeftInInput / pagesToRead`:
       Test.cpp >> mergeSort() >> line 296 - 297
    c. Read `number of pages left to read in the input table` into DRAM:
       Test.cpp >> graceFulDegradation() >> line 114
                >> DRAM.cpp >> readRecords()
    d. Read the spilled data back into Cache
       Test.cpp >> graceFulDegradation() >> line 118 - 131
    e. In-memory merge-sort for data stored in Cache and DRAM:
      Test.cpp >>  mergeSort() >> line 306
        i . `sortedMiniRuns = cache.sortForGracefulDegradation(dram.getInputBuffers(), pagesInCache, maxRecordsInPage);``
        ii. CACHE.cpp >> sortForGracefulDegradation function is responsible for creating sorted mini-runs
`
      

7. Optimized merge patterns
  - Using pointers to keep record reference.
      When reading records, I define a Record class to store separate the key characters and the rest of the characters. Then they are referred into different data strucutre, such as Page, and Run, and DRAM, SSD,
      and HDD.
  - Also, the desgin of duplicate removal is implemented in the process of merging when we pop winner from   Tournament. This process can increase the number of records being merge in next merge level, therefore, can
    optimize the time. Originally, this part is implemented during the initial scanning by hashing. However, 
    that takes up a lot of memory to store the hash table, and therefore I changed it to happen during 
    merging sorted runs.

8. Verifying: sort order

- located in "Test.cpp -> verityOrder()"
- input: Read keys from OUTPUT_TABLE and check if each key is less than the subsequent key.
- output: return 'true' if all records are in ascending order, 'false' otherwise. Additionally, it
  the output table is empty, it prints "output table is empty" to std::out.

9. Tournament Trees
   Design for sorting of large input by simulating a tournament comparison among records.
   Use a tournament tree to determine the "winner" among multiple runs of sorted data.

API Reference

- TournamentTree(int, std::vector<Run _> &, Disk \_): Constructs a tournament tree with specified runs and disk.
  - void update(int, Record \*): Updates a tree node with a new record.
  - Record \*popWinner(): Removes the winning record from the tree and updates the tree structure.
  - bool hasNext(): Checks if there are more records to process.
  - Record \*getWinner() const: Returns the current winner without removing it from the tree.

10. Duplicate Removel

  - During both merging in DRAM, and merging in HDD, I keep track of the last winner.
    Then for every new winner, if it's the same as the last winner, it will not be written to the
    new run.

  - API reference:
    a. Merge cache mini runs in DRAM, and output to a destination disk.
    DRAM::mergeFromSelfToDest
    ```
      Record * prevWinner = nullptr;
      while (tree->hasNext())
      {
          Record *winner = tree->popWinner();
          if(prevWinner == nullptr) {
              prevWinner = winner;
          }else if(prevWinner->key + prevWinner->content == winner->key +winner->content) 
          {
              numDuplicate++;
              continue;
          }else{
              prevWinner = winner;
          }
    ```
    b. Disk::mergeMemorySizedRuns(), 
      ```
      ...
      std::string prevKey = "None";
        // // write sorted output to output buffer
        while (tree->hasNext())
        {
            Record *winner = tree->popWinner();
            if(prevKey== "None") {
                prevKey = winner->key +winner->content;
            }else if(prevKey == winner->key +winner->content) 
            {
                numDuplicate++;
                continue;
            }else{
                prevKey = winner->key +winner->content;
            }
        ...
        }      
      ```
    c. Disk::mergeSSDSizedRuns()
    ```
      ...
      std::string prevKey = "None";
        // // write sorted output to output buffer
        while (tree->hasNext())
        {
            Record *winner = tree->popWinner();
            if(prevKey== "None") {
                prevKey = winner->key +winner->content;
            }else if(prevKey == winner->key +winner->content) 
            {
                numDuplicate++;
                continue;
            }else{
                prevKey = winner->key +winner->content;
            }
        ...
        }      
      ```
