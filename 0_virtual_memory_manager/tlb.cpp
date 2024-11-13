#include <iostream>
#include <unordered_map>
#include <list>
#include "tlb.h"
#include "env.h"

// tlb constructor definition
tlb::tlb() {
	std::cout << "tlb constructor called!" <<std::endl;
	tlb_page_limit = get_int_env("TLB_SIZE", 16); 		// num frames in TLB
	tlb_hit        = 0;
	tlb_miss       = 0;
	std::cout << "size limit of TLB: " << tlb_page_limit << " pages or frames" << std::endl;
}

// ----------------------------------------------------------------------
// ------ checks if the page exists in the tlb or not
// ----------------------------------------------------------------------
bool tlb::find(page page_index) {
	if (mapPageToFrame.find(page_index) != mapPageToFrame.end()) {
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------
// ------ updates TLB if tlb miss happens
// ----------------------------------------------------------------------
void tlb::upadteTlb (page page_index, frame frame_index) {
	if (idTlbFull()) {										// only remove from the TLB when it is full
		page old_page_index = page_queue.front();
		page_queue.pop_front();
		mapPageToFrame.erase(old_page_index);
	}
	page_queue.push_back(page_index);		mapPageToFrame[page_index] = frame_index;
}

// ----------------------------------------------------------------------
// ------ delete from TLB if page replaced in page table is set to invalid
// ----------------------------------------------------------------------
void tlb::deleteFromTlb (page page_index) {
	mapPageToFrame.erase(page_index);
	page_queue.remove(page_index);
}

// ----------------------------------------------------------------------
// ------ fetch the frame index for the page_index
// ----------------------------------------------------------------------
frame tlb::getFrame (page page_index) {
	return mapPageToFrame[page_index];
}