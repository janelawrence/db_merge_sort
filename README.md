# Database - External Merge Sort Implementation

## Teammates
Jane Zhang, Ziqi Liao
Github [https://github.com/janelawrence/db_merge_sort/tree/jane/optimize]

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

### Sorting Process (Using Alternative 1), Test.cpp >> mergeSort()
1. **Reading Data:**
    - For every `M` bytes of data, read `M / P` pages into DRAM.

2. **Creating Mini-Runs Using Cache:**
    - Determine the number of pages that can fit in the cache: `C / P`.
    - Calculate the number of records that can fit in the cache: ` (pages can fit in cache) * (records in a page) = (C / P) * (P / R)`.

3. **Writing Mini-Runs:**
    - Write mini-runs to DRAM and merge them in memory.
    - Then write memory-sized runs to SSD, followed by a transfer to HDD.

4. **Merging Runs on HDD:**
    - First create all SSD-Sized runs by merging memory-sized runs in sequence
        - Merge `F` memory-sized sorted runs. Read `F` pages from F memory-sized sorted runs from HDD, where `F` is the calculated fan-in value.
        - F here  = SSD_SIZE/DRAM_SIZE
    - Then merge all SSD-Sized runs into the final sorted table
      - Here fan-in is equal to total number of SSD-Sized runs created


## Main Code Structure

**Test.cpp >> mergeSort()**: Implements the external merge sort process.

**CACHE.h/CACHE.cpp**: Implements the cache system for sorting data. It manages the simulation of memory caching, sorting, and processing of data.

**Run.h/Run.cpp**: Manages sequences of records (runs) that are processed and sorted by the cache or tournament tree.

**Page.h/Page.cpp**: Represents a page of records, handling the collection of records as per the page size.

**Record.h/Record.cpp**: Defines the data structure for individual records within pages.

**HeapSort.h/HeapSort.cpp**: Implements the sorting algorithm used by the cache to sort records.

**Disk.h/Disk.cpp**: Simulates a hard disk drive for storage and retrieval of data.
  Disk::mergeMemorySizedRuns
  Disk::mergeSSDSizedRuns
  Disk::writeOutputTable

**TournamentTree.h/TournamentTree.cpp**: Implements the tournament tree logic including node comparisons, record management, and tree operations.

**Scan.cpp**:
  ScanPlan::pagingInput >> scan the input_table, and truncate them into DRAM_SIZE pages
  during scanning, remove all non-alphanumeric characters for each records

**DRAM.h/DRAM.cpp**: contains methods for reading records into DRAM input buffers, and in-memory merging
  DRAM::readRecords
  DRAM::mergeFromSelfToDest 


### Execute the program

To execute the program, use the `make` command to create the `ExternalSort.exe` executable.
Then, input `./ExternalSort.exe -c <numRecords> -s <recordSize> -o <trace.txt>` from the terminal. 
It outputs all system statistics and displays the records stored in input.txt.

Alternatively, you can put the above two commands into `run.sh` we prepare, and run this shell script from your terminal.

If you wish to use `make run`, make sure you modify the command line arguments by adjusting `line 40 of the Makefile`, where the run target is defined.

According to the project specification, the input table will have name `input_table`, output table will be named `output_table`.
If you wish to change the name of the input table, you can change it in line 35 in Test.cpp, and line 55 in Test.cpp for the output file name.

## Techniques Considerations && Contributions

| Techniques                         | contributions                                                  |
| ---------------------------------- |----------------------------------------------------------------|
| 1. Cache-size mini runs            | Jane                                                         |
| 2. Minimum count of row            | Jane                                                         |
| 3. Device-optimized page sizes     | Jane                                                           |
| 4. Spilling memory-to-SSD          | Jane                                                         |
| 5. Spilling from SSD to disk       | Jane                                                         |
| 6. Graceful degradation            | Jane(implementation), Ziqi                                  |
| 7. Optimized merge patterns        | Jane                                                           |
| 8. Verifying: sort order           | Jane: Test.cpp -> verityOrder()                                |
| 9. Tournament Trees                | Jane: TournamenTree.h, TournamenTree.cpp                       |
| 10. Duplicate Removal              | Jane: Remove duplicates during merge process                   |
| 11. Report                         | Jane and Ziqi: writing README.md to explain project design and implementation|
| 12. Meetings                       | Jane and Ziqi                                                  |
 


### Technique Usage Details

**1. Cache-size mini runs**

- Cache.cpp -> sort(), creates sorted mini runs and in each run, records are store in pages
- Each mini-run is sorted using the 'HeapSort' algorithm and managed by 'Run' class.
- Records are processed page by page, with sorting occurring at the record level.

API Reference
CACHE::sort(std::vector<Page*>, int, int): Sorts pages into mini-runs.
CACHE::outputMiniRunState(const char*): Wrtie trace outputs reporting the state of sorted mini-runs.
CACHE::getCapacity() const: Returns the current capacity of the cache.

**2. Minimum count of row**

- Based on optmized page size, and size of cache, determine the size of output buffer in DRAM
    - Since size of Cache is 1 MB, size of DRAM is 100 MB, DRAM can store at most 99 cache-sized mini runs
      if using the rest of 1 MB as the output buffers, it can store 128 output buffers, each output buffer has one page of 8 KB. Based on the input record size ranging from 20 Bytes to 2000 Bytes, these 128 Pages can store 524 to 52428 records.  
      Since the output buffer of DRAM will be used to stored the tournament tree used for merging, it needs to have
      the capacity to store one records from each of the cache-sized runs (at most 99 of them).  
      Based on our calculation above, reserving 1 MB as the output buffer is more than enough to store maximum of 99 records, of each from a cache-sized runs. Therefore, at least need 128/4 = 32 output buffers (each output buffer has one page) in the output buffers such that it can store 131 to 13107 records.

- Based on the number output buffers = 32, we can calculate the total size of space in DRAM can be used to store
  input data at once = 100 MB - 32 \* 8K = 100 MB - 0.25 MB = 99.75 MB = 99.75 MB / (8 KB/ Page) = 12768 pages.
  Then each time, DRAM can create 100 cache-size mini runs, where 99 of them are 1 MB large, and the last one is 0.75 MB large.

- Therefore, each time, DRAM's input buffer can store 12,768 pages \* (8 KB/page) / recordSize records. 
  According to the range of record size, which is 20 ~ 2,000 bytes,  
  each time, DRAM's input buffer can store 52,297 to 5,229,772 number of records  
- Therefore, each time, our program will read at most 12,768 pages into DRAM input buffers. During the in-memory merging process, each time DRAM output buffer
 is full, there are 32 output buffers \* 8 KB/page = 256 KB data spilled to SSD.

**3. Device-optimized page sizes**

- DRAM Page Size roughly = 6375 nanoseconds \* 1.0016 GB/m = roughly 6.7 KB = take 8 KB = 8192 B

- SSD Page Size roughly = 200 MB/s \* 0.1 ms = 20 KB

- HDD Page Size roughly = 100 MB/s \* 5 ms = 512 KB

**4. Spilling memory-to-SSD**

- DRAM.h/DRAM.cpp - mergeFromSelfToDest() handle the spilling from DRAM to SSD.
  **- **In the scenario when input data exceeds memory size and must be written to a second storage medium


The process involves:
- Checking if the DRAM output buffer is full.
- Simulating writing to the SSD (represented by an object `dest`, which is a Disk class object) when the buffer is full.
- Managing internal buffers and ensuring they are flushed to SSD properly.

As discussed in 3., each time DRAM output buffer is full, there are 32 output buffers * 8 KB/page = 256 KB data spilled to SSD.


- API Reference:
  DRAM.cpp >> mergeFromSelfToDest() >> line 283 to line 303, and line 306 to 322
  ```
  // if output buffer is full
        if (outputBuffers.isFull())
        {
            // Report Spilling happen to trace.txt
            dest->outputSpillState(outputTXT);
            // Simulate write to SSD
            dest->outputAccessState(ACCESS_WRITE, outputBuffers.wrapper->getBytes(), outputTXT);
            int bytesInDRAMOutputBuffers = outputBuffers.wrapper->getBytes();
            bytesInRun += bytesInDRAMOutputBuffers;
            
            while (!outputBuffers.isEmpty())
            {
                // Write to curr run in dest Disk
                Page *page = outputBuffers.wrapper->getFirstPage();
                int firstPageOriginalBytes = page->getBytes();
                dest->writePageToRunFolder(newRunPath.c_str(), page, pageIdx);
                pageIdx++;
                outputBuffers.wrapper->removeFirstPage(firstPageOriginalBytes, true);
            }
            outputBuffers.clear();
        }
  ```

**5. Spilling from SSD to disk**
- Since we adopted alternative 1, SSD will be used soley for tempoary storage. Therefore we'll use the majority of its space
as outputbuffer, storing memory-sized merged runs spilled from mempry. We use 80% of space of SSD as the output buffers.
The rest of the 20% are saved for when graceful degradation needs it.

- Locate in Test.cpp
-  Buffer management: With data loaded into DRAM from an input source, sorted data accumulates in the SSD's output buffer.
- Data spilled: When the output buffer capacity exceeds(insufficient to store the next memory-size run), the data move to HDD
  -Data transfer: Sequentially moving each run from the SSD to the HDD, SSD's output buffer is cleared post-transfer to free up space for further sorting operations.
- log management: System logs the state of data in HDD, including access and spill states, through designated output methods `(outputSpillState, outputAccessState)`.
- Final merging and sorting occur on the HDD if multiple unsorted runs remain.

- When SSD output buffers are full, spill all runs from SSD to HDD

- When all memory-sized runs are created, but they are still in SSD, spill them to HDD before merge

- API Reference:
  a. Test.cpp >> MergeSort() >> line 319 to line 331,  
  ```
  if (ssd.getOutputBufferCapacity() >= bytesRead)
		{
			// use readPass as the new mem-sized run index
			dram.mergeFromSelfToDest(&ssd, outputTXT, sortedMiniRuns, readPass);
		}
		else
		{
			// In alternative 1, all runs on SSD are of memory-sized
			// write runs on SSD to HDD, then clear SSD
			// write all runs to HDD, and then clear space
			hdd.outputSpillState(outputTXT);
			hdd.outputAccessState(ACCESS_WRITE, ssd.outputBuffers.getBytes(), outputTXT);
			ssd.clearOuputBuffer();
			dram.mergeFromSelfToDest(&ssd, outputTXT, sortedMiniRuns, readPass);
		}
  ```

  b. Test.cpp >> MergeSort() >> line 361 to line 371, 
  ```
    	// // If there are mem-sized runs left in SSD
      // //  Spill them to HDD before merging
      // // write runs on SSD to HDD, then clear SSD
      printf("%d number of memsize runs left in SSD", ssd.getNumUnsortedRuns());
      if (!ssd.outputBuffers.isEmpty())
      {
        hdd.outputSpillState(outputTXT);
        // Trace spilling all runs to HDD, and then clear space
        hdd.outputAccessState(ACCESS_WRITE, ssd.outputBuffers.getBytes(), outputTXT);
        ssd.clearOuputBuffer();
      }
  ```

- Physically:
    a. all memory-sized runs are stored in a folder inside directory `LOCAL_DRAM_SIZED_RUNS_DIR`, which is `mem_sized_runs`.

    b. In each memory-sized run run located in, there are page files of SSD_PAGE_SIZE.
      When the program is fetching 1 page on HDD for merging memory-sized runs, it will fetch HDD_PAGE/SSD_PAGE_SIZE number of
      memory sizes from this local directory.

    c. all SSD-Sized runs are stored in a folder inside `LOCAL_SSD_SIZED_RUNS_DIR` = `ssd_sized_runs`;
        Each it stores floor(SSD_SIZE/DRAM_SIZE) number of runs in this folder

**6. Graceful degradation**
According to the paper, it uses a threshold 0.01.
In line 297 to 299 of Test.cpp, when it's the second to last pass of reading the input table, the program
calculates the ratio of `number of pages left to read in the input table` to `current number of pages loaded in DRAM`. If the ratio is smaller or equal to 0.01, the program will then perform graceful degradation in the
following steps:

    - From the DRAM, it will spill pages to SSD's input buffers until DRAM has enough space to store
      `number of pages left to read in the input table`.

    - Then DRAM will read the last few data: `number of pages left to read in the input table`.

    - Then the data spilled to SSD earlier will be read and loaded into Cache.
      Since we use the ratio 0.01, there are 12768 input buffers. When all 12768 input buffers are used in
      the second to last run, if the ratio is 0.01, which means there are 0.01 * 12768 = 127 pages of records
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
      i . `sortedMiniRuns = cache.sortForGracefulDegradation(dram.getInputBuffers(), pagesInCache, maxRecordsInPage);`
      ii. CACHE.cpp >> `sortForGracefulDegradation`` function is responsible for creating sorted mini-runs
  
**7. Merging Memory-Size runs into SSD-Sized Runs**

1. If there are more than 1 memory-sized runs in HDD, then start merging them into SSD-sized runs  
  - Code Reference:   
      Test.cpp >> mergeSort() >> line 373 to line 377: calling `hdd.mergeMemorySizedRuns(outputTXT, OUTPUT_TABLE);`  
      Then in Disk::mergeMemorySizedRuns, it has the logic for using tournament tree to merge, and write to HDD pages



**8. Merging SSD-Size runs into final sorted Table**
1. If there are more than 1 SSD-sized runs in HDD, then start merging them into SSD-sized runs  
  - Code Reference:   
      Test.cpp >> mergeSort() >> line 381: calling `hdd.mergeSSDSizedRuns(outputTXT, OUTPUT_TABLE);;`  
      Then in Disk::mergeSSDSizedRuns, it has the logic for using tournament tree to merge, and write to HDD pages.
2. Note that since the SSD writing would be a lot of lines output to Trace file, so we only report page
reading on HDD during this process. And the latency for writing out is simulated when the entire sorted
run is written to the ouput_table file. (Consulted with TA, and TA agreed this way is appropriate)

**9. Optimized merge patterns**

- Using pointers to keep record reference.
  When reading records, I define a Record class to store separate the key characters and the rest of the characters. Then they are referred into different data strucutre, such as Page, and Run, and DRAM, SSD,
  and HDD.

- Also, the desgin of duplicate removal is implemented in the process of merging when we pop winner from the Tournament Tree. This process can increase the number of records being merge in next merge level, therefore, can
  optimize the time. Originally, this part is implemented during the initial scanning by hashing. However,
  that takes up a lot of memory to store the hash table, and therefore I changed it to happen during
  merging sorted runs.

**10. Verifying: sort order**

- located in "Test.cpp -> verityOrder()"
- input: Read keys from OUTPUT_TABLE and check if each key is less than the subsequent key.
- output: return 'true' if all records are in ascending order, 'false' otherwise. Additionally, prints "output table is empty".

**11. Tournament Trees**  

Design for sorting of large input by simulating a tournament comparison among records.
Use a tournament tree to determine the "winner" among multiple runs of sorted data.

- ***Implementation***: 
    - use an array as the representation of the tree
    - At the leaf level, every two nodes are competing against each other
    - Whoever has the smaller value is the winner and moves up along the tree
    - When a winner is popped, this class keeps track of which sorted run this winner came from, and 
  grab the next winner and inserted in the leaf node where the winner previously started the competition.
  Then the new record will trigger the competition bottom-up until a new winner is produced.
    - Whenever a run is empty, the leaf node where was used to place candidates from this run is inserted a Ghost Record index, `-1`
    - In any competition between a Non-Ghost record and a ghost record, the non-ghost record wins and moves up the competition hierachy.
    - When a Ghost Key is placed at the winner position of the tree, this means that the tree is empty

- ***Handles two cases***:
    - when runs are in memory, fetching pages of sorted mini cache-sized runs from memory
    - when runs are in HDD, fetching pages of sorted runs from disk

_**API Reference**_
- TournamentTree(int, std::vector<Run _> &, Disk \_): Constructs a tournament tree with specified runs and disk.
- void update(int, Record \*): Updates a tree node with a new record.
- Record \*popWinner(): Removes the winning record from the tree and updates the tree structure.
- bool hasNext(): Checks if there are more records to process.
- Record \*getWinner() const: Returns the current winner without removing it from the tree.

**12. Duplicate Removel**


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
