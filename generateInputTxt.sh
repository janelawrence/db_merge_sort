#!/bin/bash
g++ -g -o Generator Generator.cpp -std=c++11
./Generator -c 128000 -s 1024 -f input_125mb_128000_1024.txt -p 0
# ./Generator -c 125829120 -s 1024 -f input_120gb_125829120_1024.txt -p 0
