#include <iostream>
#include <fstream>

#include <vector>
#include <unordered_map>
#include <list>
#include <iterator>

#include "pageTable.h"

using namespace std;


void readDiskFile ();               // reads byte addressable data from file and store in disk
std::vector<int>* disk_data;
int ram_size;					    // user specified (always less than 16-bits address space)
std::vector<int>* RAM;              // stores the content at physical index

ofstream* program_outFile;          // stores the output of content accessed by this program
ofstream* expected_outFile;         // expected content

int main(int argc, char *argv[]) {

	std::cout << "\nSpecified Arguments: "; int i = 1;
    while (i < argc) {
        cout << argv[i] << " ";
        i++;
    }
    std::cout<<std::endl; if (argc != 3) {std::cout << "Incorrect number of arguments provided!" << std::endl; 
	std::cout << "Please specify exactly two arguments: <size_of_ram> <page_replacement_algorithm>" << std::endl; exit(0);}

    std::cout << "Virtual Memory Manager Started!" << std::endl;
    ram_size = atoi(argv[1]); 
    RAM = new std::vector<int>(); (*RAM).assign(ram_size, -1);    // initially RAM is filled with dummy values
    std::cout << "RAM size: " << ram_size << std::endl;
	
	readDiskFile();
	program_outFile = new ofstream("program_output.txt");	
	int page_rep_algm = atoi(argv[2]);
	pageTable pt_obj(256, 256, ram_size, RAM, disk_data, program_outFile, page_rep_algm);  // page_size = 256 bytes, num_pages = 256
	
	// --------- read the logical addresses of a process ----------
	expected_outFile = new ofstream("correct_ouput.txt");
	ifstream logicAdd("logical_addresses.txt");
	int add;	
	int ms16bits = 1024*64 - 1;  // most significant 16 bits
	while (logicAdd >> add) {
		int mask_add = add & ms16bits; 
		int expected_content = (*disk_data)[mask_add]; (*expected_outFile) << expected_content << std::endl;
		pt_obj.find(mask_add);           // find the address in the page table
	}

	logicAdd.close();
	(*expected_outFile).close();
	(*program_outFile).close();
	// ------------------------------------------------------------
		
return 0;
}


// ------- loading disk file and writing to disk (binary format)----------------------------------
void readDiskFile () {         // byte addressable
	ifstream diskContent("disk_data.bin");	
	std::vector<char> buffer((std::istreambuf_iterator<char>(diskContent)),(std::istreambuf_iterator<char>()));
	disk_data = new std::vector<int>();
	for (char item: buffer) {
		(*disk_data).push_back((item-'0')+48);
	}
	std::cout << "size of the process on the disk: " << (*disk_data).size() << std::endl;
	diskContent.close();
}
