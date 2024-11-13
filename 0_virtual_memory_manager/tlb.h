
using frame  = int;
using page   = int;

using namespace std;

class tlb {

	int tlb_page_limit;
	int tlb_hit;
	int tlb_miss;
	unordered_map<page, frame> mapPageToFrame;
	list<page> 				   page_queue;	 		// FIFO Page Replacement Data Structure

	bool idTlbFull() {return page_queue.size() == tlb_page_limit; };

	public:
		tlb();
		void incrTlbHit()  {tlb_hit++;       };
		void incrTlbMiss() {tlb_miss++;      };
		int  getTlbHit()   {return tlb_hit;  };
		int  getTlbMiss()  {return tlb_miss; };
		bool find(page page_index);
		void upadteTlb     (page page_index, frame frame_index);
		frame  getFrame    (page page_index);
		void deleteFromTlb (page page_index);
};
