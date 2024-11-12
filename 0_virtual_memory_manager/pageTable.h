
class pageTable {

	int num_page_hits;
	int num_page_faults;

	int page_size;           	  // in the power of 2 = 2^num_page_size_bits
	int num_page_size_bits;  	  // num of bits used to represent offset
	int get_bits(int page_size);  // computes num_page_size_bits using page_size
	int num_pages;                // total number of pages in the process

	int get_page_index  (int add);  // based on the page size (in power of 2) 
	int get_page_offset (int add);  // based on the page size (in power of 2) 

	std::vector <std::vector<int>> table;           // stores at index i (page num)    --->  (frame num, valid/invalid bit)
	std::unordered_map <int, int> mapFrameToPage;   // frame index --->  page index  // could have used vector as well
	int num_frames, size_of_mm;   			// mm: main memory
	void initTable();               		// initialize the page table with all entries bit marked 0 (invalid)

	std::vector<int>* disk_data;
	std::vector<int>* RAM;

	int  get_frame_location_tobe_replaced();
	void fetch_data_from_disk (int page_index, int frame_base_location);

	bool debug;
	void printBinFormatAdd(int add);
	void verifyCorrectness();
	std::ofstream* program_outFile;

	int page_replacement_algm;        // default: FIFO (= 0), LRU (=1), Clock (=2), Optimal (=3)
	void initPageRepDataStrct();      // initialize the data structures for each of the page replacement algorithms
	void updatePageRepDataStruct(int page_index);   // update the data structures for each of the page replacement algorithms

	std::list <int> frame_queue;     			    // used for FCFS page replacement technique
	int callFifo();					 		 	    // FIFO page replacement algorithm

	std::vector <int> frame_access;  			    // 0: just used --- -- > 15: least used
	void callLruUpdate(int frame_index);			// updates the access values wheneven a page hit happens
	int callLru();					  				// LRU page replacement algorithm   (Least Recently Used)

	std::vector <int> frame_01_array; // 0: recently accessed, and 1: not recently accessed
	int ptr;						  // pointing on the frame to be replaced
	int callClock();				  // Clock page replacement algorithm   (Second Chance Algorithm)

	std::vector <int> future_pages;   // pages to be accessed in the future (in order)
	int current_pos;
	int empty_ptr;  				  // ptr pointing to unused frame
	int callOpt();				      // Optimal page replacement algorithm (Truely Optimal But Hypothetical Algorithm)

	public:
		pageTable(int psize, int npages, int rsize, std::vector<int>* ram, std::vector<int>* disk, std::ofstream* outFile, int page_rep_algm = 0);
		void find(int add);
		void printInitialStats();   // prints the info about the page table
		void printPostStats();      // prints post-execution stats of the page table
		~pageTable();
};
