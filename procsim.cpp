#include "procsim.hpp"
#include <iostream>

using namespace std;

#define DEBUG 0

proc_info_t info;
reg_file_t reg_file[8];

/**
 * Subroutine for initializing the processor. You many add and initialize any global or heap
 * variables as needed.
 * XXX: You're responsible for completing this routine
 *
 * @d Dispatch queue multiplier
 * @k0 Number of k0 FUs
 * @k1 Number of k1 FUs
 * @k2 Number of k2 FUs
 * @f Number of instructions to fetch
 * @m Schedule queue multiplier
 * @c Number of CDBs
 */
void setup_proc(uint64_t d, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t m, uint64_t c) {
	// Populate the information structure
	info.dispatch_q_mult = (int) d;
	info.num_k0 = (int) k0;
	info.num_k1 = (int) k1;
	info.num_k2 = (int) k2;
	info.fetch_rate = (int) f;
	info.schedule_q_mult = (int) m;
	info.num_cdb = (int) c;

	if (DEBUG) {
		std::cout << "Num K0: " << info.num_k0 << '\n';
		std::cout << "Num K1: " << info.num_k1 << '\n';
		std::cout << "Num K2: " << info.num_k2 << '\n';
		std::cout << "Fetch Rate: " << info.fetch_rate << '\n';
		std::cout << "Dispatch Queue Multiplier: " << info.dispatch_q_mult << '\n';
		std::cout << "Schedule Queue Multiplier: " << info.schedule_q_mult << '\n';
		std::cout << "Num CDB: " << info.num_cdb << "\n\n";
	}

	// Set up the register file
	for (int i = 0; i < 8; i++) {
		reg_file[i].ready = true;
		reg_file[i].tag = -1;
		reg_file[i].value = -1;

		if (DEBUG) {
			std::cout << "[" << i << "].ready: " << reg_file[i].ready << '\n';
			std::cout << "[" << i << "].tag: " << reg_file[i].tag << '\n';
			std::cout << "[" << i << "].value: " << reg_file[i].value << "\n\n";
		}
	}

	// Print the headings
	printf("INST\tFETCH\tDISP\tSCHEDU\t EXEC\tSTATE\n");
}

/**
 * Subroutine that simulates the processor.
 *   The processor should fetch instructions as appropriate, until all instructions have executed
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void run_proc(proc_stats_t* p_stats) {

}

/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC or branch prediction percentage
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_proc(proc_stats_t *p_stats) {

}

void fetch() {

}

void dispatch() {

}

void schedule() {

}

void execute() {

}

void state_update() {
	
}