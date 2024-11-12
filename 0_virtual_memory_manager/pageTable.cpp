#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>
#include <bitset>
#include <fstream>
#include "pageTable.h"
#include "env.cpp"

using namespace std;

// ----------------------------------------------------------------------
// ------ Page Table Constructor
// ----------------------------------------------------------------------
pageTable::pageTable(int psize, int npages, int rsize, std::vector<int>* ram, std::vector<int>* disk, ofstream* outFile, int page_rep_algm) {
	std::cout << "pageTable constructor called!" << std::endl;
	
	page_size          = psize;
	assert(((psize-1)&psize) == 0);   // checks if the number is power of 2
	num_page_size_bits = get_bits(psize-1);

	num_pages = npages;
	size_of_mm = rsize;
	num_frames = size_of_mm/page_size;
	table.assign(num_pages, vector<int>());   // initialize the table with dummy -1 frame number and all invalid bits, i.e., 0
	initTable();

	RAM = ram; disk_data = disk;

	page_replacement_algm = page_rep_algm;
	assert(page_rep_algm >= 0 && page_rep_algm <=3);
	initPageRepDataStrct();

	debug = get_bool_env("ENABLE_DEBUG", false);
	num_page_hits      = 0;
	num_page_faults    = 0;
	printInitialStats();
	program_outFile = outFile;
	std::cout << "pageTable constructor completed execution!" << std::endl;
}

// ----------------------------------------------------------------------
// ------ finds the address in the page table
// ----------------------------------------------------------------------
void pageTable::find(int add) {
	int page_index   = get_page_index  (add);
	int offset       = get_page_offset (add);
	printBinFormatAdd(add);

	if (table[page_index][1] == 1) {    // page found (valid bit set)
		num_page_hits++;
	} else {
		num_page_faults++;				// page fault happens
		int frame_base_location = get_frame_location_tobe_replaced();   // based on some page-replacement technique
		fetch_data_from_disk(page_index, frame_base_location);
	}

	int frame_base_location = table[page_index][0]*page_size;
	updatePageRepDataStruct(page_index);
	if (debug) std::cout << "content at logical address " << add << " is: " << (*RAM)[frame_base_location + offset] <<std::endl;
	(*program_outFile) << (*RAM)[frame_base_location + offset] << std::endl; // writing the content to file as well (for verifying later)
}

// ----------------------------------------------------------------------
// ------ prints the address in the binary format
// ----------------------------------------------------------------------
void pageTable::printBinFormatAdd(int add) {
	if (!debug) return;
	int page_index   = get_page_index  (add);
	int offset       = get_page_offset (add);
	std::cout << "address: " << bitset<16>(add) << ", index: " << bitset<16>(page_index) << ", offset: " << bitset<16>(offset) << std::endl; 
}

// -----------------------------------------------------------------------------------------------------------------------------------
// ------ FIFO: Page Replacement Algorithm
// -----------------------------------------------------------------------------------------------------------------------------------
int pageTable::callFifo () {
	int frame_index = frame_queue.front();
	frame_queue.pop_front();
	frame_queue.push_back(frame_index);
	return frame_index;
}

// -----------------------------------------------------------------------------------------------------------------------------------
// ------ LRU: Page Replacement Algorithm
// -----------------------------------------------------------------------------------------------------------------------------------
int pageTable::callLru () {
	for (int i = 0; i < frame_access.size(); i++) {
		if (frame_access[i] == num_frames-1) {
			if (debug) std::cout << "LRU selected page index: " << i << ", and access time: " << num_frames-1 << std::endl;
			return i;
		}
	}
	assert(1 == 0);   // program control should not reach here
}

// -----------------------------------------------------------------------------------------------------------------------------------
// ------ LRU's Update Option
// -----------------------------------------------------------------------------------------------------------------------------------
void pageTable::callLruUpdate (int frame_index) {
	for (int i = 0; i < frame_access.size(); i++) {
		if (frame_access[i] < frame_access[frame_index]) {   // only update the access times of the times smaller than access time of frame_index
			frame_access[i] = (frame_access[i] + 1)%num_frames;
		}
	}
	assert(frame_access.size() == num_frames);
	if (debug) std::cout << "resetting frame index: " << frame_index << " to 0" << std::endl;
	frame_access[frame_index] = 0;
}

// -----------------------------------------------------------------------------------------------------------------------------------
// ------ Clock: Page Replacement Algorithm
// -----------------------------------------------------------------------------------------------------------------------------------
int pageTable::callClock () {
	while (frame_01_array[ptr] != 1) {
		assert (frame_01_array[ptr] == 0);
		frame_01_array[ptr] = 1;
		ptr = (ptr + 1)%num_frames;
	}
	return ptr;
}

// -----------------------------------------------------------------------------------------------------------------------------------
// ------ Optimal: Page Replacement Algorithm
// -----------------------------------------------------------------------------------------------------------------------------------
int pageTable::callOpt () {
	if (mapFrameToPage.size() < num_frames) { 		// pick an empty frame if available
		empty_ptr++;
		return empty_ptr-1;
	}

	int num_cnt = 0;
	frame_01_array.clear();
	frame_01_array.assign(num_frames, 0);
	for (int pos = current_pos+1; pos <= future_pages.size(); pos++) {
		int page_index = future_pages[pos];
		if (table[page_index][1] == 1) { 		// present in ram
			int frame_index = table[page_index][0];
			if (frame_01_array[frame_index] == 0) {		// encountred first time in furture traversal
				num_cnt++;
			}
			if (num_cnt == num_frames) {			    // last accessed
				return frame_index;
			}
			frame_01_array[frame_index] = 1;
		}
	}
	assert(num_cnt < num_frames); 		// if control reaches here this condition must be true

	for (int i = 0; i < num_frames; i++) {
		if (frame_01_array[i] == 0) return i;		// pick any of the frames that are not appearing in the future anymore
	}
	assert(0==1); // control must not reach here
}

// -----------------------------------------------------------------------------------------------------------------------------------
// ------ get the frame to be replaced from RAM & mark the bit invalid for the corresponding page
// -----------------------------------------------------------------------------------------------------------------------------------
int pageTable::get_frame_location_tobe_replaced () {
	int frame_index;

	if (page_replacement_algm == 0) {
		frame_index = callFifo();
	} else if (page_replacement_algm == 1) {
		frame_index = callLru();
	} else if (page_replacement_algm == 2) {
		frame_index = callClock();
	} else if (page_replacement_algm == 3) {
		frame_index = callOpt();
	}

	if (debug) std::cout << "frame_index: " << frame_index << ", frame_base_location: " << frame_index*page_size << std::endl;

	int page_index = mapFrameToPage[frame_index]; // update the page table: mark the page bit invalid 
	table[page_index][1] = 0;
 
	return frame_index*page_size;
}

// -----------------------------------------------------------------------------------------------------------------------------------
// ------ fetches the data from disk (here we assume that logical address and disk address coincidently have the same location)
// -----------------------------------------------------------------------------------------------------------------------------------
void pageTable::fetch_data_from_disk (int page_index, int frame_base_location) {  // move the page from disk to RAM at location: frame_base_location
	int page_base_add = page_index * page_size;
	int disk_location = page_base_add;

	for (int i = 0; i < page_size; i++) {
		int phy_add   = frame_base_location + i;
		(*RAM) [phy_add] =  (*disk_data) [disk_location + i];
	}

	int frame_index = frame_base_location/page_size;    
	table[page_index][0] = frame_index;  table[page_index][1] = 1;   // upadte the page table
	mapFrameToPage[frame_index] = page_index;						 // update the inverse map: frame -> page
}

// ----------------------------------------------------------------------
// ------ prints the info of the page table
// ----------------------------------------------------------------------
void pageTable::printInitialStats () {
	std::cout << "page size in int : " << page_size << std::endl;
	std::cout << "page size in bits: " << num_page_size_bits << std::endl;
	std::cout << "num pages:         " << num_pages << std::endl;
	std::cout << "num frames:        " << num_frames << std::endl;
}

// ----------------------------------------------------------------------
// ------ finds the number of bits used to represent a page
// ----------------------------------------------------------------------
int pageTable::get_bits(int page_size) {
	int cnt = 0;
	while (page_size > 0) {
		page_size = page_size >> 1;
		cnt++;
	}
	return cnt;
}

// -------------------------------------------------------------------------------
// ------ returns the corresponding page number for the logical address
// -------------------------------------------------------------------------------
int pageTable::get_page_index (int add) {   // based on the page size (in power of 2) 
	return (add - get_page_offset (add)) >> num_page_size_bits;
}

// -------------------------------------------------------------------------------
// ------ returns the corresponding page offset for the logical address
// -------------------------------------------------------------------------------
int pageTable::get_page_offset (int add) {   // based on the page size (in power of 2) 
	return add & (page_size-1);
}

// ------------------------------------------------------------------------------------------
// ------ initialize the page table to contain all invalid bits marked
// ------------------------------------------------------------------------------------------
void pageTable::initTable () {
	std::vector <int> temp = {-1, 0};
	for (int i = 0; i < num_pages; i++) {
		table[i] = temp;
	}
}

// ----------------------------------------------------------------------
// ------ Page Table Destructor
// ----------------------------------------------------------------------
pageTable::~pageTable() {
	printPostStats();
	verifyCorrectness();
}

// ----------------------------------------------------------------------
// ------ prints post execution stats
// ----------------------------------------------------------------------
void pageTable::printPostStats () {
	std::cout << "number of page faults: " << num_page_faults << std::endl;
	std::cout << "number of page hits  : " << num_page_hits << std::endl;
}

// ----------------------------------------------------------------------
// ------ verify correctness of the program
// ----------------------------------------------------------------------
void pageTable::verifyCorrectness () {
	ifstream selfAlgm ("program_output.txt");
	ifstream defAlgm  ("correct_ouput.txt");

	int selfVal, defVal;
	bool incorrect = 0;
	while (defAlgm >> defVal) {
		selfAlgm >> selfVal;
		if (selfVal != defVal) incorrect = true;
	}
	if (incorrect) {
		std::cout << "INCORRECT! The program output does not match the expected output!\n" << std::endl;
	} else {
		std::cout << "CORRECT! The program output matches the expected output!\n" << std::endl;
	}
}

// ----------------------------------------------------------------------
// ------ initialize the data structures for the page replacement
// ----------------------------------------------------------------------
void pageTable::initPageRepDataStrct() {
	if (page_replacement_algm == 0) {				// FIFO Queue
		std::cout << "=============== Page Replacement Algorithm: FIFO ==============" << std::endl;
		for (int i = 0; i < num_frames; i++) {
			frame_queue.push_back(i);
		}
	} 
	else if (page_replacement_algm == 1) {				// LRU Data Structures  // mark timing in all frames in any order
		std::cout << "=============== Page Replacement Algorithm: LRU ==============" << std::endl;
		for (int i = 0; i < num_frames; i++) {
			frame_access.push_back(i);
		}
	} 
	else if (page_replacement_algm == 2) {				// Clock Queue
		std::cout << "=============== Page Replacement Algorithm: Clock ==============" << std::endl;
		for (int i = 0; i < num_frames; i++) {
			frame_01_array.push_back(0);
		}
		ptr = 0;
	} 
	else if (page_replacement_algm == 3) {				// Optimal Page Replacement
		std::cout << "=============== Page Replacement Algorithm: Optimal ==============" << std::endl;

		ifstream logicAdd("logical_addresses.txt"); 		// get the future page accesses
		int add;	
		int ms16bits = 1024*64 - 1;  // most significant 16 bits
		while (logicAdd >> add) {
			int mask_add   = add & ms16bits;
			int page_index = get_page_index (add);
			future_pages.push_back(page_index);
		}
		logicAdd.close();

		current_pos = 0; empty_ptr = 0;
	}
}


// -----------------------------------------------------------------------------------
// ------ update the data structures for each of the page replacement algorithms
// -----------------------------------------------------------------------------------
void pageTable::updatePageRepDataStruct(int page_index) {
	if (page_replacement_algm == 0) {				    // FIFO Queue (do nothing)
		
	} 
	else if (page_replacement_algm == 1) {				// LRU Data Structures  // update access timings for all the frames
		int frame_index = table[page_index][0];
		callLruUpdate(frame_index);
	} 
	else if (page_replacement_algm == 2) {				// Clock Queue
		int frame_index = table[page_index][0];
		frame_01_array[frame_index] = 0;
	} 
	else if (page_replacement_algm == 3) {				// Optimal Page Replacement
		current_pos++;
	}
}
