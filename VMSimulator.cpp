#include "page.h"
#include "process.h"
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <deque>
#include <vector>
#include <queue>
#include <list>
#include <iostream>
#include <sstream>
#include <fstream>

#define MEMORYSIZE 512

using namespace std;

// pSize is parsed from pList showing each process id and it's size <<pid, size> ...>
vector<vector<int> > pSize;
// pTrace is parsed from pTrace that keeps track of all the referenced memory locations called by the system <<pid, memoryLocation>...>
vector<vector<int> > pTrace;
// processList represented all the current processes in our simulation, and is a vector of process objects created at the start of the program
vector<Process*> processList;
// mainMemory represents the main memory in this simulation, it is implemented as a deque because we will need to remove from both ends for some algorithms
deque<deque<Page*> > mainMemory;
// virtualMemory represented the virtual memory in this simulation, it is implemented as a vector of a vector of pages (pageTable) which will contain each processes' page table
vector<vector<Page*> > virtualMemory;
// pageSwap will keep count of how many page swaps are performed in the simulation
int pageSwaps = 0;


void FIFO(int sizeOfPages, int numFrames) {
	cout << "In FIFO!" << endl;
	int pid;    // pid is the process id of the process "trace" is currently calling
	int memLoc;    // memLoc is the location of memory that "trace" is currently calling
   	int mruLoc; // freeLoc is the frame's index in main memory of the page that "trace" is currently calling if it is already in memory
   	Page* reqPage; // reqPage is the page that "trace" is currently requesting
    
    for (int i = 0; i < pTrace.size(); i++) {
        pid = pTrace[i][0];        // get process id
        memLoc = pTrace[i][1];    // get memory location  // calculate number of frames per process
        reqPage = virtualMemory[pid][(memLoc-1)/sizeOfPages];	// we do memLoc-1 because memory starts at 1, while our index starts at 0, so we shift it down 1 by subtracting 1

        // DEBUGGING PRINT STATEMENTS
        cout << "DEBUGGING FIFO: Size of Pages: " << sizeOfPages << endl;
        cout << "DEBUGGING FIFO: Iteration Number: " << i << endl;
        cout << "DEBUGGING FIFO: Memory Location: " << memLoc << endl;
        cout << "DEBUGGING FIFO: Size of process is: " << pSize[pid][1] << endl;
        cout << "Getting pid: " << pid << " requesting page number: " << (memLoc-1)/sizeOfPages << endl;

        // First, we need to check if this page is already in memory or not, if not, then we need to put it in memory
        if (reqPage->validBit == 0) {
            cout << "Page is not in memory!" << endl;
            // If there is free space, we can simply push the page to memory, otherwise, we need to replace a frame and perform a pageswap
            if (mainMemory[pid].size() < numFrames) {
                mainMemory[pid].push_back(reqPage);    // push reqPage to main memory
            }
            // If there is no free space, we will pop the front because the initialization of main memory loaded pages from frame 0 up to frame numFrames,
            // so the least recently used page will always be the first frame
            else {
                mainMemory[pid].front()->validBit = 0;    // set page at front of memory to be 0, since it will no longer be in memory
                mainMemory[pid].pop_front();    // pop the first frame since it is being put in first
                mainMemory[pid].push_back(reqPage);    // push the newly requested page into main memory. The last page will be at the back
                cout << "Swapped page!" << endl;
                pageSwaps++;    // increment count of page swap
            }
            reqPage->validBit = 1;    // set validBit of reqPage to 1 because it is now in memory
        }
        // If page is already in memory, we do nothing
        else {
            cout << "Page is already in memory!" << endl;
        }
        cout << endl;
    }
}

// Least recently used will loop through the trace file and begin simulation. Since main memory was initiailized with the first pSize[i]/sizeOfPages frames,
// the pages already in main memory at the start of execution were least recently used at time 0. Thus, we can just proceed by always removing the top frame
void LRU(int sizeOfPages, int numFrames) {
    cout << "In LRU!" << endl;
    int pid;    // pid is the process id of the process "trace" is currently calling
    int memLoc;    // memLoc is the location of memory that "trace" is currently calling
    int mruLoc; // freeLoc is the frame's index in main memory of the page that "trace" is currently calling if it is already in memory
    Page* reqPage; // reqPage is the page that "trace" is currently requesting
    
    for (int i = 0; i < pTrace.size(); i++) {
        pid = pTrace[i][0];        // get process id
        memLoc = pTrace[i][1];    // get memory location  // calculate number of frames per process
        reqPage = virtualMemory[pid][(memLoc-1)/sizeOfPages];	// we do memLoc-1 because memory starts at 1, while our index starts at 0, so we shift it down 1 by subtracting 1
        
        // DEBUGGING PRINT STATEMENTS
        cout << "DEBUGGING LRU: Size of Pages: " << sizeOfPages << endl;
        cout << "DEBUGGING LRU: Iteration Number: " << i << endl;
        cout << "DEBUGGING LRU: Memory Location: " << memLoc << endl;
        cout << "DEBUGGING LRU: Size of process is: " << pSize[pid][1] << endl;
        cout << "Getting pid: " << pid << " requesting page number: " << (memLoc-1)/sizeOfPages << endl;
        
        // First, we need to check if this page is already in memory or not, if not, then we need to put it in memory
        if (reqPage->validBit == 0) {
            cout << "Page is not in memory!" << endl;
            // If there is free space, we can simply push the page to memory, otherwise, we need to replace a frame and perform a pageswap
            if (mainMemory[pid].size() < numFrames) {
                mainMemory[pid].push_back(reqPage);    // push reqPage to main memory
            }
            // If there is no free space, we will pop the front because the initialization of main memory loaded pages from frame 0 up to frame numFrames,
            // so the least recently used page will always be the first frame
            else {
                mainMemory[pid].front()->validBit = 0;    // set page at front of memory to be 0, since it will no longer be in memory
                mainMemory[pid].pop_front();    // pop the first frame since it is the least recently used
                mainMemory[pid].push_back(reqPage);    // push the newly requested page into main memory. The most recently used pages will be at the back
                cout << "Swapped page!" << endl;
                pageSwaps++;    // increment count of page swap
            }
            reqPage->validBit = 1;    // set validBit of reqPage to 1 because it is now in memory
        }
        // If page is already in memory, we can place it at the back of the frame table so the most recently used frames will always be at the back of
        // the frame table in main memory
        else {
            cout << "Page is already in memory!" << endl;
            // First, we need to determine where in main memory the page that "trace" is currently calling is
            for (int j = 0; j < mainMemory[pid].size(); j++) {
                // If the pageId of the frame we are currently searching at is equal to the pageId of the page "trace" is currently calling,
                // we have found the position of the frame
                if (mainMemory[pid][j]->pageId == reqPage->pageId) {
                    mruLoc = j;
                    break;
                }
            }
            mainMemory[pid].erase(mainMemory[pid].begin() + mruLoc);    // we will then erase the frame in main memory, shifting the indexes
            mainMemory[pid].push_back(reqPage);    // we will then push back the requested page from trace
        }
        cout << endl;
    }
}

void Clock(int sizeOfPages, int numFrames) {
    cout << "In Clock!" << endl;
    int pid;    // pid is the process id of the process "trace" is currently calling
    int memLoc;    // memLoc is the location of memory that "trace" is currently calling
    int clockHand = 0;	// clockHand represents the index of the frame in mainMemory at which the hand oe the clock is pointing at
    int startHand; 		// startHand keeps track of which frame the clockHand was first pointing at so we know when we have made a full loop
    bool foundPage = false;	// foundPage is a boolean that represents if a page with an rbit with 0 has been found 
    Page* reqPage;	// reqPage is the page that "trace" is currently requesting
    Page* clockPage;	//clockPage is the page that the clock hand is currently pointing to

    for (int i = 0; i < pTrace.size(); i++) {
    	pid = pTrace[i][0];        // get process id
        memLoc = pTrace[i][1];    // get memory location  // calculate number of frames per process
        reqPage = virtualMemory[pid][(memLoc-1)/sizeOfPages];	// we do memLoc-1 because memory starts at 1, while our index starts at 0, so we shift it down 1 by subtracting 1
        clockPage = mainMemory[pid][clockHand];
        foundPage = false;
        
        // DEBUGGING PRINT STATEMENTS
        cout << "DEBUGGING CLOCK: Size of Pages: " << sizeOfPages << endl;
        cout << "DEBUGGING CLOCK: Iteration Number: " << i << endl;
        cout << "DEBUGGING CLOCK: Memory Location: " << memLoc << endl;
        cout << "DEBUGGING CLOCK: Size of process is: " << pSize[pid][1] << endl;
        cout << "Getting pid: " << pid << " requesting page number: " << (memLoc-1)/sizeOfPages << endl;

        // if page requested by "trace" is not in memory, we have to put it in memory
        if (reqPage->validBit == 0) {
        	cout << "Page is not in memory!" << endl;
        	// if there is free space in memory, push the requested page to memory
        	if (mainMemory[pid].size() < numFrames) {
        		cout << "There is free memory, pushing now" << endl;
        		mainMemory[pid].push_back(reqPage);
        		reqPage->validBit = 1;
        	}
        	// if main memory is full, then we will need to search for a page with rBit = 0

        	else {
        		cout << "There is no free memory, finding page with rBit = 0" << endl;
        		// we keep searching until we have found a page with rbit 0 or we go one round around the clock
        		while (foundPage == false) {
        			startHand = clockHand;	// set startHand equal to our starting point so we know when we have made one full lap
        			cout << "Start hand: " << startHand << endl;
        			cout << "Clock hand: " << clockHand << endl;

        			cout << "Clock page pageId: " << clockPage->pageId << endl;
        			cout << "Clock page rBit: " << clockPage ->rBit << endl;
        			// if the clock hand is pointing at a page with rbit 0, then we will replace that page
        			if (clockPage->rBit == 0) {
        				cout << "Found page to replace!" << endl;
        				clockPage->validBit = 0;	// update replaced page's valid bit to 0
        				mainMemory[pid][clockHand] = reqPage;	// replace page with requested page
        				reqPage->validBit = 1;		// update requested page's valid bit to 1
        				cout << "Swapped page!" << endl;
        				pageSwaps++;					// increment page swaps count
        				foundPage = true;			// update boolean
        			}
        			// else, if rbit is 1, toggle it back to 0
        			if (clockPage->rBit == 1) {
        				clockPage->rBit = 0;
        			}
        			clockHand++;	// increment clockHand and search again
        			// if we reach the end of the frame table of pid, then we set clock hand back to beginning of frame table
        			if (clockHand == mainMemory[pid].size()) {
        				clockHand = 0;
        			}
        			clockPage = mainMemory[pid][clockHand];	// update clockPage to get next page
        			// if we have looped back to where we started with no page with rbit 0, then we replace original page
        			if (clockHand == startHand) {
        				cout << "Clockhand is: " << clockHand << " and startHand is : " << startHand << endl;
        				cout << "No page with rbit 0 found, replacing original page" << endl;
        				clockPage->validBit = 0;	// set valid bit and rbit to 0 since it is being replaced
        				clockPage->rBit = 0;
        				mainMemory[pid][startHand] = reqPage;	// replace with requested page
        				reqPage->validBit = 1;	// update valid bit now that page is in memory
        				cout << "Swapped page!" << endl;
        				pageSwaps++;	// increment page swaps count
        				foundPage = true;
        			}
        		}
        	}
        }
        else {
        	// if the page is already in memory, we will simply update the rBit to 1 if it isn't
        	virtualMemory[pid][(memLoc-1)/sizeOfPages]->rBit = 1;
        }
    }
}

// getNextPageNotInMem is a helper function for prepaging for finding next page not in memory
// Given: Id of process requested by ptrace, page requested by ptrace, and size of pages
// Returns: Index of next page not in memory, -1 if next page not in memory is not found
int getNextPageNotInMem(int pid, int page, int sizeOfPages) {
	cout << "Getting page not in memory!" << endl;
	bool foundNextPage = false;	// we keep searching if foundNextPage is not satisfied or if we have searched the entire page table
	int currPageIndex = page + 1;	// we start searching from the page after the current page being inserted to main memory
	int lastPageIndex = ((pSize[pid][1])-1)/sizeOfPages;	// we keep track of the last index so we know when to stop searching

	// we keep searching until we find a page not in memory 
	while (foundNextPage == false) {
		/*
		cout << "Page is: " << pSize[pid][1] << endl;
		cout << "Searching now at page: " << currPageIndex << endl;
		cout << "Last page index: " << lastPageIndex << endl;
		*/
		// if we have searched the entire page table, we return -1
		if (currPageIndex > lastPageIndex) {
			return -1;
		}
		// if valid bit is 0, then we have found the next page not in memory
		if (virtualMemory[pid][currPageIndex]->validBit == 0) {
			return currPageIndex;	// return index of next page not in memory
		}
		//cout << "Current page is already in memory" << endl;
		currPageIndex++;
	}
	cout << "Done searching" << endl;
	return -1;	// if we did not find the next page not in memory, we return -1
}

void pFIFO (int sizeOfPages, int numFrames) {
	int pid;	// process id
	int memLoc;	// memory location requested
	int mruLoc;	// keep track of requested memory location in memory
	Page* reqPage;

	for (int i = 0; i < pTrace.size(); i++) {
		pid = pTrace[i][0];		// get process id
		memLoc = pTrace[i][1];	// get requested memory location
		//numFrames = (MEMORYSIZE/sizeOfPages)/processList.size();	// number of frames allocated per process
		reqPage = virtualMemory[pid][(memLoc-1)/sizeOfPages];			// get requested page

		// DEBUGGING PRINT STATEMENTS
		cout << "DEBUGGING pFIFO: Size of Pages: " << sizeOfPages << endl;
		cout << "DEBUGGING pFIFO: Iteration Number: " << i << endl;
		cout << "DEBUGGING pFIFO: Memory Location: " << memLoc << endl;
		cout << "DEBUGGING pFIFO: Size of process is: " << pSize[pid][1] << endl;
		cout << "Getting pid: " << pid << " requesting page number: " << (memLoc-1)/sizeOfPages << endl;

		// next, we check to see if there is another page in vm not in mm
		int nextPageAvail = getNextPageNotInMem(pid, (memLoc-1)/sizeOfPages, sizeOfPages);
		cout << "Next page available is: " << nextPageAvail << endl;
		cout << "Num frames: " << numFrames << endl;
		cout << "Size of main memory at pid: " << pid << ": " << mainMemory[pid].size() << endl;
		// if requested page is not in memory, then we need to insert it in memory
		if (reqPage->validBit == 0) {
			cout << "Not in memory!" << endl;
			// if there is free memory, we can push it to memory
			if (mainMemory[pid].size() < numFrames) {
				cout << "Free space, can push to memory" << endl;
				mainMemory[pid].push_back(reqPage);
				reqPage->validBit = 1;
			}
			// else, we need to remove the first page since it is the first to be put in and insert reqPage
			if (mainMemory[pid].size() >= numFrames) {
				cout << "No free space, performing page swap" << endl;
				mainMemory[pid].front()->validBit = 0;
				mainMemory[pid].pop_front();
				mainMemory[pid].push_back(reqPage);
				reqPage->validBit = 1;
				cout << "Swapped page" << endl;
				pageSwaps++;
			}
			// if there is a next page not in memory, we need to insert it in memory
			if (nextPageAvail != -1) {
				Page* nextReqPage = virtualMemory[pid][nextPageAvail];
				// if there is free memory, we can push it to memory
				if (numFrames - mainMemory[pid].size() >= 1) {
					mainMemory[pid].push_back(nextReqPage);
					nextReqPage->validBit = 1;
				}
				// if there is no free memory, we need to pop the top of main memory and push the last page to the back of main memory
				else {
					mainMemory[pid].front()->validBit = 0;
					mainMemory[pid].pop_front();
					mainMemory[pid].push_back(nextReqPage);
					nextReqPage->validBit = 1;
					cout << "Swapped page" << endl;
					pageSwaps++;
				}
			}
		}
		else {
			// if it is already in memory, we do nothing
		}
	}
}

void pLRU (int sizeOfPages, int numFrames) {
    cout << "In pLRU" << endl;
	int pid;	// process id
	int memLoc;	// memory location requested
	int mruLoc;	// keep track of requested memory location in memory
	Page* reqPage;

	for (int i = 0; i < pTrace.size(); i++) {
		pid = pTrace[i][0];		// get process id
		memLoc = pTrace[i][1];	// get requested memory location
		reqPage = virtualMemory[pid][(memLoc-1)/sizeOfPages];			// get requested page
		int nextPageAvail = getNextPageNotInMem(pid, (memLoc-1)/sizeOfPages, sizeOfPages);
		// DEBUGGING PRINT STATEMENTS
		cout << "DEBUGGING pLRU: Size of Pages: " << sizeOfPages << endl;
		cout << "DEBUGGING pLRU: Iteration Number: " << i << endl;
		cout << "DEBUGGING pLRU: Memory Location: " << memLoc << endl;
		cout << "DEBUGGING pLRU: Size of process is: " << pSize[pid][1] << endl;
		cout << "Getting pid: " << pid << " requesting page number: " << (memLoc-1)/sizeOfPages << endl;
		
		// next, we check to see if there is another page in vm not in mm
		cout << "Next page available is: " << nextPageAvail << endl;
		cout << "Num frames: " << numFrames << endl;
		cout << "Size of main memory at pid: " << pid << ": " << mainMemory[pid].size() << endl;
		// if requested page is not in memory, then we need to insert it in memory
		if (reqPage->validBit == 0) {
			cout << "Not in memory!" << endl;
			// if there is free memory, we can push it to memory
			if (mainMemory[pid].size() < numFrames) {
				cout << "Free space, can push to memory" << endl;
				mainMemory[pid].push_back(reqPage);
				reqPage->validBit = 1;
			}
			// else, we need to remove the first page since it is the least recently used and insert reqPage
			if (mainMemory[pid].size() >= numFrames) {
				cout << "No free space, performing page swap" << endl;
				mainMemory[pid].front()->validBit = 0;
				mainMemory[pid].pop_front();
				mainMemory[pid].push_back(reqPage);
				reqPage->validBit = 1;
				cout << "Swapped page" << endl;
				pageSwaps++;
			}
			// if there is a next page not in memory, we need to insert it in memory
			if (nextPageAvail != -1) {
				Page* nextReqPage = virtualMemory[pid][nextPageAvail];
				// if there is free memory, we can push it to memory
				if (numFrames - mainMemory[pid].size() >= 1) {
					mainMemory[pid].push_back(nextReqPage);
					nextReqPage->validBit = 1;
				} 
				// if there is no free memory, we need to pop the top of main memory and push the most recently requested page to the back of main memory
				else {
					mainMemory[pid].front()->validBit = 0;
					mainMemory[pid].pop_front();
					mainMemory[pid].push_back(nextReqPage);
					nextReqPage->validBit = 1;
					cout << "Swapped page" << endl;
					pageSwaps++;
				}
			}
		}
		else {
			// if it is already in memory, we push it back to the end of main memory since it is most recently used
			// first, we find where in memory the requested page is
			for (int j = 0; j < mainMemory[pid].size(); j++) {
				if (mainMemory[pid][j]->pageId == reqPage->pageId) {
					mruLoc = j;
					break;
				}
			}
			mainMemory[pid].erase(mainMemory[pid].begin() + mruLoc);
			mainMemory[pid].push_back(reqPage);
		}
	}
}

void pClock(int sizeOfPages, int numFrames) {
	cout << "In pClock!" << endl;
    int pid;    // pid is the process id of the process "trace" is currently calling
    int memLoc;    // memLoc is the location of memory that "trace" is currently calling
    int clockHand = 0;	// clockHand represents the index of the frame in mainMemory at which the hand oe the clock is pointing at
    int startHand; 		// startHand keeps track of which frame the clockHand was first pointing at so we know when we have made a full loop
    int numPageReplace;	// keeps track of how many pages to replace
    bool foundPage = false;	// foundPage is a boolean that represents if a page with an rbit with 0 has been found
    Page* reqPage;	// reqPage is the page that "trace" is currently requesting
    Page* clockPage;	// clockPage is the page that the clock hand is currently pointing to
    Page* prePage;	// prepage represents the page if there exists a contigous page after the requested page that is not in memory


    for (int i = 0; i < pTrace.size(); i++) {
    	pid = pTrace[i][0];        // get process id
        memLoc = pTrace[i][1];    // get memory location  // calculate number of frames per process
        reqPage = virtualMemory[pid][(memLoc-1)/sizeOfPages];	// we do memLoc-1 because memory starts at 1, while our index starts at 0, so we shift it down 1 by subtracting 1
        clockPage = mainMemory[pid][clockHand];
        foundPage = false;
        numPageReplace = 1;
        // DEBUGGING PRINT STATEMENTS
        cout << "DEBUGGING pCLOCK: Size of Pages: " << sizeOfPages << endl;
        cout << "DEBUGGING pCLOCK: Iteration Number: " << i << endl;
        cout << "DEBUGGING pCLOCK: Memory Location: " << memLoc << endl;
        cout << "DEBUGGING pCLOCK: Size of process is: " << pSize[pid][1] << endl;
        cout << "Getting pid: " << pid << " requesting page number: " << (memLoc-1)/sizeOfPages << endl;

        // if page requested by "trace" is not in memory, we have to put it in memory
        if (reqPage->validBit == 0) {
        	int nextPageAvail = getNextPageNotInMem(pid, (memLoc-1)/sizeOfPages, sizeOfPages);
        	cout << "Page is not in memory!" << endl;
        	// if there is free space in memory, push the requested page to memory
        	if (mainMemory[pid].size() < numFrames) {
        		cout << "There is free memory, pushing now" << endl;
        		mainMemory[pid].push_back(reqPage);
        		reqPage->validBit = 1;
        	}
        	// if main memory is full, then we will need to search for a page with rBit = 0
        	else {
        		if (nextPageAvail != -1) {
        			prePage = virtualMemory[pid][nextPageAvail];
        			numPageReplace++;

        		}
        		cout << "There is no free memory, finding page with rBit = 0" << endl;
        		// we keep searching until we have found a page with rbit 0 or we go one round around the clock
        		while (numPageReplace > 0) {
        			startHand = clockHand;	// set startHand equal to our starting point so we know when we have made one full lap
        			cout << "Start hand: " << startHand << endl;
        			cout << "Clock hand: " << clockHand << endl;

        			cout << "Clock page pageId: " << clockPage->pageId << endl;
        			cout << "Clock page rBit: " << clockPage ->rBit << endl;
        			// if the clock hand is pointing at a page with rbit 0, then we will replace that page
        			if (clockPage->rBit == 0) {
        				cout << "Found page to replace!" << endl;
        				clockPage->validBit = 0;	// update replaced page's valid bit to 0
        				// if numPageReplace is more than 1, then we are prepaging 2 pages
        				if (numPageReplace > 1) {
        					mainMemory[pid][clockHand] = prePage;
        					prePage -> validBit = 1;
        					cout << "Swapped page!" << endl;
        					pageSwaps++;
        				} 
        				// otherwise, we only have to put the original requested page into memory
        				else {
	        				mainMemory[pid][clockHand] = reqPage;	// replace page with requested page
	        				reqPage->validBit = 1;		// update requested page's valid bit to 1
	        				cout << "Swapped page!" << endl;
	        				pageSwaps++;					// increment page swaps count
	        			}
        				numPageReplace--;			// update numPageReplace
        			}
        			// else, if rbit is 1, toggle it back to 0
        			if (clockPage->rBit == 1) {
        				clockPage->rBit = 0;
        			}
        			clockHand++;	// increment clockHand and search again
        			// if we reach the end of the frame table of pid, then we set clock hand back to beginning of frame table
        			if (clockHand == mainMemory[pid].size()) {
        				clockHand = 0;
        			}
        			clockPage = mainMemory[pid][clockHand];	// update clockPage to get next page
        			// if we have looped back to where we started with no page with rbit 0, then we replace original page
        			if (clockHand == startHand) {
        				cout << "Clockhand is: " << clockHand << " and startHand is : " << startHand << endl;
        				cout << "No page with rbit 0 found, replacing original page" << endl;
        				clockPage->validBit = 0;	// set valid bit and rbit to 0 since it is being replaced
        				clockPage->rBit = 0;
        				// if numPageReplace is more than 1, then we are prepaging 2 pages
        				if (numPageReplace > 1) {
	        				mainMemory[pid][startHand] = prePage;	// replace with requested page
	        				prePage->validBit = 1;	// update valid bit now that page is in memory
	        				cout << "Swapped page!" << endl;
	        				pageSwaps++;	// increment page swaps count
	        			} 
	        			// otherwise, we only have to put the original requested page into memory
	        			else {
	        				mainMemory[pid][startHand] = reqPage;	// replace with requested page
	        				reqPage->validBit = 1;	// update valid bit now that page is in memory
	        				cout << "Swapped page!" << endl;
	        				pageSwaps++;	// increment page swaps count
	        			}
	        			numPageReplace--;	// update numPageReplace
        			}
        		} 		
        	}
        }
        else {
        	// if the page is already in memory, we will simply update the rBit to 1 if it isn't
        	virtualMemory[pid][(memLoc-1)/sizeOfPages]->rBit = 1;
        }
    }
}

int main(int argc, char * const argv[]) {
    
    int sizeOfPages;    // sizeOfPages represents the size of each frame and page
    string pageReplaceAlgo;   // algorithm represents the page replacement algorithm input by the user
    bool prepage = false;       // prepage is a boolean that represents true or false based on the flag, demand prepaging is turned on by default
    string flag;        // flag represents the flag that is input by the user to choose between demand paging or prepaging
    istringstream iss;  // iss is the stringstream object to convert the arguments to strings
    
    // argument variables
    int processId;  // processId represents the process id in plist
    int proMemSize; // proMemSize represents the amount of memory for the process
    int proMemLoc;  // proMemLoc represents the memory of location that the system requests access to
    
    // initialization variables
    int mainMemFrames;    // mainMemFrames represents the number of page frames in main memory
    int numFrames;      // numFrames represents the number of page frames per process in main memory at initialization
    
    // check that there are 6 arguments being entered
    if (argc != 6) {
        cerr << "Error: Please input a valid number of arguments!" << endl;
        return 1;
    }
    // otherwise, if there are the correct number of arguments, start parsing through the arguments
    // grab plist as an argument, if null, then prompt user to input plist file
    ifstream in_1(argv[1]);
    if (!(in_1)) {
        cerr << "Error: Please input a valid plist file!" << endl;
        return 1;
    }
    // parse through plist, storing each process id and its memory size in vector "process", then pushing it to pSize in the form <pid, memsize>
    while(in_1 >> processId >> proMemSize){
        vector<int> process;
        process.push_back(processId);
        process.push_back(proMemSize);
        pSize.push_back(process);
    }
    // grab ptrace as an argument, if null, then prompt user to input ptrace file
    ifstream in_2(argv[2]);
    if (!(in_2)) {
        cerr << "Error: Please input a valid ptrace file!" << endl;
        return 1;
    }
    // parse through ptrace, storing the series of memory access in vector "memLocAccess", then pushing it to pTrace in the form <pid, memLocation>
    while(in_2 >> processId >> proMemLoc){
        vector<int> memLocAccess;
        memLocAccess.push_back(processId);
        memLocAccess.push_back(proMemLoc);
        pTrace.push_back(memLocAccess);
    }
    // check validity of argument 3 (size of pages), convert it to integer and store in variable "sizeOfPages"
    sizeOfPages = atoi(argv[3]);
    if (sizeOfPages <= 0) {
        cerr << "Error: Please input a valid positive integer for size of pages!" << endl;
        return 1;
    }
    // check validity of argument 4 (page replacement algorithm) that it is a string and matches one of the existing algorithms
    iss.str(argv[4]);
    if(!(iss >> pageReplaceAlgo)) {
        cerr << "Error: Please input a valid string argument for page replacement algorithm!" << endl;
        return 1;
    }
    iss.clear();
    if ((pageReplaceAlgo != "FIFO") && (pageReplaceAlgo != "LRU") && (pageReplaceAlgo != "Clock")) {
        cerr << "Error: Please input a valid page replacement algorithm!" << endl;
        return 1;
    }
    // check validity of argument 5 (prepaging flag), store in variable "flag", and check that flag is of string type
    iss.str(argv[5]);
    if (!(iss >> flag)) {
        cerr << "Error: Please input a valid argument for flag!" << endl;
        return 1;
    }
    // check that flag is of one of the two valid symbols (+ or -), if not, return error
    if (((flag.compare("+") != 0) && (flag.compare("-")) != 0)) {
        cerr << "Error: Please input a valid flag!" << endl;
        return 1;
    }
    if (flag.compare("+") == 0) {
        prepage = true;
    } else {
        prepage = false;
    }
    
    // loop through all the process sizes and create a new process object, and push it to processList
    for (int i = 0; i < pSize.size(); i++) {
        proMemSize = pSize[i][1];
        processList.push_back(new Process(i, proMemSize, sizeOfPages));
    }
    
    // initialize virtual memory by pushing each process' page table to virtualMemory
    for(int i = 0; i < processList.size(); i++) {
        for (int j = 0; j < MEMORYSIZE/sizeOfPages/processList.size(); j++) {
            processList[i]->pageTable[j]->validBit = 1;
        }
        virtualMemory.push_back(processList[i]->pageTable);
    }
    
    // set values of mainMemFrames and numFrames
    mainMemFrames = MEMORYSIZE/sizeOfPages;
    numFrames = mainMemFrames/processList.size();
    // initialize equal number of page frames for each process into main memory
    for(int i = 0; i < processList.size(); i++) {
        deque<Page*> frames;
        // for each process, put equal amount of frames from virtual memory into "frames"
        for(int j = 0; j < numFrames; j++) {
            frames.push_back(virtualMemory[i][j]);
        }
        // after equal amount of frames have ben loaded to "frames", load it into main memory
        mainMemory.push_back(frames);
        frames.clear();
    }
    
    
    if (prepage == false) {
        if (pageReplaceAlgo == "FIFO") {
            FIFO(sizeOfPages, numFrames);
            cout << "Page swaps: " << pageSwaps << endl;
        } else if (pageReplaceAlgo == "LRU") {
            LRU(sizeOfPages, numFrames);
            cout << "Page swaps: " << pageSwaps << endl;
        } else {
            Clock(sizeOfPages, numFrames);
            cout << "Page swaps: " << pageSwaps << endl;
        }
    } else {
    	if (pageReplaceAlgo == "FIFO") {
            pFIFO(sizeOfPages, numFrames);
            cout << "Page swaps: " << pageSwaps << endl;
        } else if (pageReplaceAlgo == "LRU") {
            pLRU(sizeOfPages, numFrames);
            cout << "Page swaps: " << pageSwaps << endl;
        } else {
            pClock(sizeOfPages, numFrames);
            cout << "Page swaps: " << pageSwaps << endl;
        }
    }
    
    /*
     // DEBUG
     for (int i = 0; i < processList.size(); i++){
     
     //cout << "Process Size: " << processList[i]->size << endl;
     //cout << "Size of Pages: " << sizeOfPages << endl;
     //cout << "Number of pages: " << (processList[i]->size)/sizeOfPages << endl;
     
     for (int j = 0; j < processList[i]->pageTable.size(); j++){
     cout << "pid: " << virtualMemory[i][j]->pid << " pageid: " << virtualMemory[i][j]->pageId << " Page Name: " << virtualMemory[i][j]->pageName  << " Valid Bit: " << virtualMemory[i][j]->validBit << endl;
     }
     }
     for (int i = 0; i < mainMemory.size(); i++){
     for (int j = 0; j < mainMemory[i].size(); j++){
     cout << "pid: " << mainMemory[i][j]->pid << " pageid: " << mainMemory[i][j]->pageId << " Page Name: " << mainMemory[i][j]->pageName << " Valid Bit: " << mainMemory[i][j]->validBit << endl;
     }
     }
     */
}
