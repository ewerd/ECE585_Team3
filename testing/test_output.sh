#!/bin/bash

# Begin testing
echo "Note: No output past Test Descriptions is good output"
echo "Begin Testing"

echo "Test Read tRCD"
diff -u testing/ground_truth/readSingle_output.txt <(./sim.exe testing/test_traces/readSingle_test.txt)
echo "Test Write tRCD"
diff -u testing/ground_truth/writeSingle_output.txt <(./sim.exe testing/test_traces/writeSingle_test.txt) 
echo "Test iFetch tRCD"				
diff -u testing/ground_truth/ifetchSingle_output.txt <(./sim.exe testing/test_traces/ifetchSingle_test.txt) 			

echo "Test Open Page Policy for Reads"
diff -u testing/ground_truth/readOpenRowLater_output.txt <(./sim.exe testing/test_traces/readOpenRowLater_test.txt)	
echo "Test Open Page Policy for Writes"		
diff -u testing/ground_truth/writeOpenRowLater_output.txt <(./sim.exe testing/test_traces/writeOpenRowLater_test.txt) 		

echo "Test Read tCCD_L"
diff -u testing/ground_truth/readSameBGB_output.txt <(./sim.exe testing/test_traces/readSameBGB_test.txt) 		
echo "Test Write tCCD_L"		
diff -u testing/ground_truth/writeSameBGB_output.txt <(./sim.exe testing/test_traces/writeSameBGB_test.txt) 			

echo "Test Read tCCD_S and tRRD_S"
diff -u testing/ground_truth/readDiffBG_output.txt <(./sim.exe testing/test_traces/readDiffBG_test.txt)	
echo "Test Write tCCD_S and tRRD_S"			
diff -u testing/ground_truth/writeDiffBG_output.txt <(./sim.exe testing/test_traces/writeDiffBG_test.txt)				

echo "Test Read tCCD_L and tRRD_L"
diff -u testing/ground_truth/readSameBGDiffB_output.txt <(./sim.exe testing/test_traces/readSameBGDiffB_test.txt)
echo "Test Write tCCD_L and tRRD_L" 			
diff -u testing/ground_truth/writeSameBGDiffB_output.txt <(./sim.exe testing/test_traces/writeSameBGDiffB_test.txt) 		

echo "Test Read tRAS"
diff -u testing/ground_truth/readThreeDiffRows_output.txt <(./sim.exe testing/test_traces/readThreeDiffRows_test.txt) 		

echo "Test Write tCWD(CWL) + tBurst + tWR"
diff -u testing/ground_truth/writeThreeDiffRows_output.txt <(./sim.exe testing/test_traces/writeThreeDiffRows_test.txt) 		

echo "Test Read tRTP"
diff -u testing/ground_truth/readTwoLongWaitOneRTP_output.txt <(./sim.exe testing/test_traces/readTwoLongWaitOneRTP_test.txt) 	
echo "Test Write tRTP"
diff -u testing/ground_truth/writeTwoLongWaitOneRTP_output.txt <(./sim.exe testing/test_traces/writeTwoLongWaitOneRTP_test.txt) 	

echo "Test Read to Write, all Same Row"
diff -u testing/ground_truth/readWriteSameRow_output.txt <(./sim.exe testing/test_traces/readWriteSameRow_test.txt) 		
echo "Test Read to Write to Read, all Same Row"
diff -u testing/ground_truth/readWriteReadSameRow_output.txt <(./sim.exe testing/test_traces/readWriteReadSameRow_test.txt) 	

echo "Test Write to Read, all Same Row"
diff -u testing/ground_truth/writeReadSameRow_output.txt <(./sim.exe testing/test_traces/writeReadSameRow_test.txt) 		
echo "Test Write to Read to Write, all Same Row"
diff -u testing/ground_truth/writeReadWriteSameRow_output.txt <(./sim.exe testing/test_traces/writeReadWriteSameRow_test.txt) 	

echo "Test multiple commands received on the same CPU clock cycle"
diff -u testing/ground_truth/sameClockCycle_test.txt <(./sim.exe testing/test_traces/sameClockCycle_test.txt) 			

echo "Test consecutive memory accesses to all bank groups and banks"
diff -u testing/ground_truth/allBanksBankGroups_test.txt <(./sim.exe testing/test_traces/allBanksBankGroups_test.txt)
echo "Test consecutive memory accesses to all bank groups and banks with Read Write Read Situations"
diff -u testing/ground_truth/readWriteReadAllBanksBankGroups_output.txt <(./sim.exe testing/test_traces/readWriteReadAllBanksBankGroups_test.txt)
