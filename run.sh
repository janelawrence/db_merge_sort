#!/bin/bash
make
# ./ExternalSort.exe -c 200 -s 20 -o trace_20_1024.txt

valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         ./ExternalSort.exe -c 125829120 -s 1024 -o trace_input_table.txt
# ./ExternalSort.exe -c 20 -s 1024 -o output_trace.txt
