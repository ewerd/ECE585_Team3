# List of edge cases to consider when testing

## Inputs
- Combination of Spaces/Tabs between the time, command, address
  - Idea: Switch to sscanf after fgets and use whitespace separation between types
- Random new lines in input? 
  - Solution: Check if first character is a new line, if so, read another line.
- Comments in trace file? Look for //?
  - Idea: <- May not be necessary, Check if the first character is a number in the line. <- This may not work as it could have spaces before the numbers.
- Negative time, or not monotonically increasing?
  - Solution: Dr. Faust said that the trace file will be monotonically increasing, so we can make this assumption.
- Out of bounds address or command in parser.

## Outputs

## Runtime Main
- Clock time overflowing its variable bit width (choose a max length and throw an error if the time is too long)
- Queue Overflow. Receiving a 17th instruction before room for it has been made available. <- Keep track of current time and wait until queue is not full to add it and do the same for future instructions after the delay caused by a full queue
- When the queue is empty, there could be a long time before the next instruction arrives.
  - Solution: Check this condition and make sure all banks and bank groups finish their delays before incrementing the time to the next instruction if its far away.
- When to clear memory access requests from the queue? When the read command is issued? When the burst starts? When the burst ends?
