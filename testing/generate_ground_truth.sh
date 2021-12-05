#!/bin/bash

# Make the program
(cd .. && make clean) # Spawn another terminal to cd up a directory and run make clean
(cd .. && make all)   # Spawn another terminal to cd up a directory and run make all

# Begin testing
echo "Begin Testing"

echo "Generate Ground Truth Tests Output for tRCD"
./../sim.exe test_traces/readSingle_test.txt -o ground_truth/readSingle_output.txt 
./../sim.exe test_traces/writeSingle_test.txt -o ground_truth/writeSingle_output.txt	
./../sim.exe test_traces/ifetchSingle_test.txt -o ground_truth/ifetchSingle_output.txt  			

echo "Generate Ground Truth Tests Output for Open Page Policy"
./../sim.exe test_traces/readOpenRowLater_test.txt -o ground_truth/readOpenRowLater_output.txt 			
./../sim.exe test_traces/writeOpenRowLater_test.txt -o ground_truth/writeOpenRowLater_output.txt  		

echo "Generate Ground Truth Tests Output for tCCD_L"
./../sim.exe test_traces/readSameBGB_test.txt -o ground_truth/readSameBGB_output.txt  				
./../sim.exe test_traces/writeSameBGB_test.txt -o ground_truth/writeSameBGB_output.txt  			

echo "Generate Ground Truth Tests Output for tCCD_S and tRRD_S"
./../sim.exe test_traces/readDiffBG_test.txt -o ground_truth/readDiffBG_output.txt 				
./../sim.exe test_traces/writeDiffBG_test.txt -o ground_truth/writeDiffBG_output.txt 				

echo "Generate Ground Truth Tests Output for tCCD_L and tRRD_L"
./../sim.exe test_traces/readSameBGDiffB_test.txt -o ground_truth/readSameBGDiffB_output.txt  			
./../sim.exe test_traces/writeSameBGDiffB_test.txt -o ground_truth/writeSameBGDiffB_output.txt  		

echo "Generate Ground Truth Tests Output for tRAS"
./../sim.exe test_traces/readThreeDiffRows_test.txt -o ground_truth/readThreeDiffRows_output.txt  		

echo "Generate Ground Truth Tests Output for tCWD(CWL + tBurst + tWR"
./../sim.exe test_traces/writeThreeDiffRows_test.txt -o ground_truth/writeThreeDiffRows_output.txt 	

echo "Generate Ground Truth Tests Output for tRTP"
./../sim.exe test_traces/readTwoLongWaitOneRTP_test.txt -o ground_truth/readTwoLongWaitOneRTP_output.txt  	
./../sim.exe test_traces/writeTwoLongWaitOneRTP_test.txt -o ground_truth/writeTwoLongWaitOneRTP_output.txt  	

echo "Generate Ground Truth Tests Output for Read to Write"
./../sim.exe test_traces/readWriteSameRow_test.txt -o ground_truth/readWriteSameRow_output.txt  		
./../sim.exe test_traces/readWriteDiffBGB_test.txt -o ground_truth/readWriteDiffBGB_output.txt  		
./../sim.exe test_traces/readWriteReadSameRow_test.txt -o ground_truth/readWriteReadSameRow_output.txt  	
./../sim.exe test_traces/readWriteReadDiffBGB_test.txt -o ground_truth/readWriteReadDiffBGB_output.txt  	

echo "Generate Ground Truth Tests Output for Write to Read"
./../sim.exe test_traces/writeReadSameRow_test.txt -o ground_truth/writeReadSameRow_output.txt 		
./../sim.exe test_traces/writeReadWriteSameRow_test.txt -o ground_truth/writeReadWriteSameRow_output.txt 	

echo "Generate Ground Truth Tests Output for multiple commands received on the same CPU clock cycle"
./../sim.exe test_traces/sameClockCycle_test.txt -o ground_truth/sameClockCycle_output.txt  			

echo "Generate Ground Truth Tests Output for consecutive memory accesses to all bank groups and banks"
./../sim.exe test_traces/allBanksBankGroups_test.txt -o ground_truth/allBanksBankGroups_test.txt
./../sim.exe test_traces/readWriteReadAllBanksBankGroups_test.txt -o ground_truth/readWriteReadAllBanksBankGroups_output.txt  		
