# Database - External Merge Sort Implementation

## Overview
This project implements an external merge sort algorithm, which is particularly effective for sorting very large datasets that exceed the size of the main memory. The algorithm is capable of handling data sets ranging from 50 MB up to 120 GB, with individual record sizes of 20 to 2000 bytes.

### Hardware Configuration
The implementation is designed to run in a simulated environment with the following specifications:

- **CPU:** Single-core with 1 MB cache
- **Memory:** 100 MB DRAM
- **Storage:**
    - **SSD:** 10 GB capacity, 0.1 ms latency, 200 MB/s bandwidth
    - **HDD:** Unlimited capacity, 5 ms latency, 100 MB/s bandwidth
  
### Performance Metrics

- **Memory Access Latency:** 6375 nanoseconds
- **Memory Bandwidth:** 1.0016 GB/s
- **Page Size Calculation:**
    - Using the memory access latency and bandwidth, we calculate the page size as follows:
    - `Page Size = 6375 nanoseconds * 1.0016 GB/s`
    - This calculation yields a page size of approximately 6.7 KB.
    - For practical implementation, we round this up to 8 KB, equivalent to 8192 bytes.


## Implementation Details
### Definitions
Here are the key variables used in the sorting process:
- `M`: DRAM_SIZE - The total size of DRAM to be used.
- `C`: CACHE_SIZE - The size of the cache available.
- `R`: recordSize - The size of each record in bytes.
- `N`: numRecords - The total number of records to be sorted.
- `I`: totalInputBytes - The total size of the input data, calculated as `I = R * N`.
- `P`: PAGE_SIZE - The size of a page in bytes. For performance calculations, we use 8192 B.
- `n`: recordsPerPage - The number of records that fit within a single page, determined as `n = P / R`.
- `F`: Fan-in - The number of pages that can be read into memory simultaneously, calculated as `F = (M / P) - 2`.

### Sorting Process (Using Alternative 1)
1. **Reading Data:**
    - For every `M` bytes of data, read `M / P` pages into DRAM.

2. **Creating Mini-Runs Using Cache:**
    - Determine the number of pages that can fit in the cache: `C / P`.
    - Calculate the number of records that can fit in the cache: ` (pages can fit in cache) \* (records in a page) = (C / P) * (P / R)`.

3. **Writing Mini-Runs:**
    - Write mini-runs to DRAM and merge them in memory.
    - Then write memory-sized runs to SSD, followed by a transfer to HDD.

4. **Merging Runs on HDD:**
    - Read `F` pages from SSD to DRAM, where `F` is the calculated fan-in value.
    - F = (M/P - 2) 


## Main Code Structure

CACHE.h/CACHE.cpp: Implements the cache system for sorting data. It manages the simulation of memory caching, sorting, and processing of data.

Run.h/Run.cpp: Manages sequences of records (runs) that are processed and sorted by the cache or tournament tree.

Page.h/Page.cpp: Represents a page of records, handling the collection of records as per the page size.

Record.h/Record.cpp: Defines the data structure for individual records within pages.

HeapSort.h/HeapSort.cpp: Implements the sorting algorithm used by the cache to sort records in a run.

disk.h/disk.cpp: Simulates a hard disk drive for storage and retrieval of data.

TournamentTree.h/TournamentTree.cpp: Implements the tournament tree logic including node comparisons, record management, and tree operations.


### Execute the program

To execute the program, use the `make run` command. This command is equivalent to running `./ExternalSort.exe -c 51200 -s 1024 -o trace_test.txt` directly from the terminal. It outputs all system statistics and displays the records stored in input.txt.

Alternatively, you can execute the program using a shell script by running `bash run.sh` from your terminal.

If you need to modify the command line arguments, you can adjust the parameters on `line 40 of the Makefile`, where the run target is defined.


## Techniques Considerations && Contributions

| Techniques                         | contributions                                                  |
| ---------------------------------- |----------------------------------------------------------------|
| 1. Cache-size mini runs            | (Jane)                                                         |
| ---------------------------------- | -------------------------------------------------------------- |
| 2. Minimum count of row            | (Jane)                                                         |
| ---------------------------------- | -------------------------------------------------------------- |
| 3. Device-optimized page sizes     | WRITE ANALYSIS in next section                                 |
| ---------------------------------- | -------------------------------------------------------------- |
| 4. Spilling memory-to-SSD          | (Jane) Done                                                    |
| ---------------------------------- | -------------------------------------------------------------- |
| 5. Spilling from SSD to disk       | (Jane) Done                                                    |
| ---------------------------------- | -------------------------------------------------------------- |
| 6. Graceful degradation            | (Ziqi) Done                                                    |
| a. into merging                    | (Ziqi) Done                                                    |
| b. beyond one merge step           | (Ziqi) Done                                                    |
| ---------------------------------- | -------------------------------------------------------------- |
| 7. Optimized merge patterns        | Using pointers to records instead of index (explain)           |
| ---------------------------------- | -------------------------------------------------------------- |
| 8. Verifying: sort order           | (Jane) Test.cpp -> verityOrder()                               |
| a. sets of rows & values           |                                                                |
| ---------------------------------- | -------------------------------------------------------------- |
| 9. Tournament Trees                | (Jane)TournamenTree.h, TournamenTree.cpp                       |
| ---------------------------------- | -------------------------------------------------------------- |
| 10. Duplicate Removel              | (Jane) Scan.cpp -> Run *ScanPlan::scan(const char *INPUT_TXT)  |



### Technique Usage Details

**1. Cache-size mini runs**

- Cache.cpp -> sort(), creates sorted mini runs and in each run, records are store in pages
- Each mini-run is sorted using the 'HeapSort' algorithm and managed by 'Run' class.
- Records are processed page by page, with sorting occurring at the record level.

API Reference
CACHE::sort(std::vector<Page*>, int, int): Sorts pages into mini-runs.
CACHE::outputMiniRunState(const char*): Outputs the state of sorted mini-runs to a specified file.
CACHE::getCapacity() const: Returns the current capacity of the cache.

**2. Minimum count of row**

- Based on optmized page size, and size of cache, determine the size of output buffer in DRAM
    - Since size of Cache is 1 MB, size of DRAM is 100 MB, DRAM can store at most 99 cache-sized mini runs
      if using the rest of 1 MB as the output buffers, it can store 128 output buffers, each output buffer has one page of 8 KB. Based on the input record size ranging from 20 Bytes to 2000 Bytes, these 128 Pages can store 524 to 52428 records.  
      Since the output buffer of DRAM will be used to stored the tournament tree used for merging, it needs to have
      the capacity to store one records from each of the cache-sized runs (at most 99 of them). Based on our calculation above, reserving 1 MB as the output buffer is more than enough to store maximum of 99 records, of each from a cache-sized runs. Therefore, at least need 128/4 = 32 output buffers (each output buffer has one page) in the output buffers such that it can store 131 to 13107 records.

- Based on the number output buffers = 32, we can calculate the total size of space in DRAM can be used to store
  input data at once = 100 MB - 32 \* 8K = 100 MB - 0.25 MB = 99.75 MB = 99.75 MB / (8 KB/ Page) = 12768 pages.
- Therefore, each time, DRAM can store 12768 pages \* 8 KB/page / recordSize =

**3. Device-optimized page sizes**

- Page Size roughly = 6375 nanoseconds \* 1.0016 GB/m = roughly 6.7 KB = take 8 KB = 8192 B

**4. Spilling memory-to-SSD**

- DRAM.h/DRAM.cpp - mergeFromSelfToDest() handle the spilling from DRAM to SSD.
  **- **In the scenario when input data exceeds memory size and must be written to a second storage medium


  The process involves:
- Checking if the output buffer is full.
- Simulating writing to the SSD (represented by dest) when the buffer is full.
- Managing internal buffers and ensuring they are flushed to SSD properly.

**5. Spilling from SSD to disk**
- Locate in Test.cpp
-  Buffer management: With data loaded into DRAM from an input source, sorted data accumulates in the SSD's output buffer.
- Data spilled: When the output buffer capacity exceeds(insufficient to store the next memory-size run), the data move to HDD
  -Data transfer: Sequentially moving each run from the SSD to the HDD, SSD's output buffer is cleared post-transfer to free up space for further sorting operations.
- log management: System logs the state of data in HDD, including access and spill states, through designated output methods `(outputSpillState, outputAccessState)`.
- Final merging and sorting occur on the HDD if multiple unsorted runs remain.


**6. Graceful degradation**
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

**7. Optimized merge patterns**
    - Using pointers to records instead of index (explain)

**8. Verifying: sort order**

- located in "Test.cpp -> verityOrder()"
- input: Read keys from OUTPUT_TABLE and check if each key is less than the subsequent key.
- output: return 'true' if all records are in ascending order, 'false' otherwise. Additionally, prints "output table is empty".

**9. Tournament Trees**
   Design for sorting of large input by simulating a tournament comparison among records.
   Use a tournament tree to determine the "winner" among multiple runs of sorted data.

_**API Reference**_
- TournamentTree(int, std::vector<Run _> &, Disk \_): Constructs a tournament tree with specified runs and disk.
- void update(int, Record \*): Updates a tree node with a new record.
- Record \*popWinner(): Removes the winning record from the tree and updates the tree structure.
- void replaceWinner(Record \*): Replaces the current winner with a new record, useful for continuous sorting.
- bool hasNext(): Checks if there are more records to process.
- Record \*getWinner() const: Returns the current winner without removing it from the tree.

**10. Duplicate Removel**

- In `Scan.cpp -> Run *ScanPlan::scan(const char *INPUT_TXT)`
  It scan through the input table txt, and use a hashmap to store
  hashed record data. When hashed value has existed in the hashmap,
  skip reading in this record, and continue.
  Returns records stored in pages and wrapped in a wrapper class Run.


