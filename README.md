# Virtual-Memory-Simulator

### Explanation

The goal of the assignment is to emulate virtual memory in an operating system.

1) Page.h represents each page in memory.
2) Process.h represents the page table 
3) We parse pList.txt to setup the page tables of all processes required by the operating system.
4) We also parse pTrace.txt to simulate the operating system requesting different resources from processes.

### How to run this program:
Open up terminal and run the command `./VMSimulator olist.txt ptrace.txt 'sizeOfPages', 'pageReplacementAlgorithm', 'prepaging flag'`, with a custom input of 'sizeOfPages', one of the page replacement algorithms(FIFO, LRU, Clock) and the flag for prepaging (+ or -).
