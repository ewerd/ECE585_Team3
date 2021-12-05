#!/bin/bash

# Testing Statistic Script for DRAM Memory Simulation Program
# Portland State University, ECE 485/585 Final Project
# Presented December 6th, 2021
# Author Drew Seidel

# Run three key tests and output statistics back to back


make stat_comparison/stat_compare.c

echo " "
echo "Beging Statistic Printing"

echo " "
echo " "
echo " "
echo "Miss Test File:  Basic Run Statistics"
./sim.exe stat_comparison/test_file/All_Miss.txt -stat | stat_comparison/stat_compare

echo ""
echo "Miss Test File: Optimal Run Statistics"
./sim.exe stat_comparison/test_file/All_Miss.txt -stat -opt | stat_comparison/stat_compare


echo " "
echo " "
echo " "
echo "Hit Test File: Basic Run Statistics"
./sim.exe stat_comparison/test_file/countThrough_bankgroup.txt -stat | stat_comparison/stat_compare


echo " "
echo "Hit Test File: Optimal Run Statistics"
./sim.exe stat_comparison/test_file/countThrough_bankgroup.txt -stat -opt | stat_comparison/stat_compare


echo " "
echo " "
echo " "
echo "Random Test File: Basic Run Statistics"
./sim.exe stat_comparison/test_file/Long_Random_Test.txt -stat | stat_comparison/stat_compare


echo " "
echo "Random Test File: Optimal Run Statistics"
./sim.exe stat_comparison/test_file/Long_Random_Test.txt -stat -opt | stat_comparison/stat_compare
