#include "procsim.hpp"
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

#define DEBUG1 0 // Level 1 debugging, previously used debugging options
#define DEBUG2 1 // Currently used debugging options
#define DEBUG3 1 // Level 3 debugging, future debugging options

#define CYCLE_END 30
#define INSTR_END 100

proc_info_t info; // Structure that holds all the infomation
vector <proc_inst_t> instructions; // Instruction vector, this should stay in order
vector <vector<fu_t> > function_units; // Two-Dimensional FU data structure
reg_file_t reg_file[NUM_REGISTERS]; // Register file

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
	info.disp_q_mult = (int) d;
	info.fu[0] = (int) k0;
	info.fu[1] = (int) k1;
	info.fu[2] = (int) k2;
	info.fetch_rate = (int) f;
	info.sched_q_mult = (int) m;
	info.num_cdb = (int) c;

	// Calculate the sizes of each queue
	info.disp_q_size = info.disp_q_mult*info.sched_q_mult*(info.fu[0]+info.fu[1]+info.fu[2]);
	info.sched_size[0] = info.sched_q_mult*info.fu[0];
	info.sched_size[1] = info.sched_q_mult*info.fu[1];
	info.sched_size[2] = info.sched_q_mult*info.fu[2];

	// Set the counts to zero
	info.sched_count[0] = 0;
	info.sched_count[1] = 0;
	info.sched_count[2] = 0;
	info.disp_count = 0;
	info.cdb_count = 0;

	if (DEBUG1) {
		std::cout << "Num K0: " << info.fu[0] << '\n';
		std::cout << "Num K1: " << info.fu[1] << '\n';
		std::cout << "Num K2: " << info.fu[2] << '\n';
		std::cout << "Fetch Rate: " << info.fetch_rate << '\n';
		std::cout << "Dispatch Queue Multiplier: " << info.disp_q_mult << '\n';
		std::cout << "Schedule Queue Multiplier: " << info.sched_q_mult << '\n';
		std::cout << "Num CDB: " << info.num_cdb << "\n\n";
	}

	// Set up the register file
	for (int i = 0; i < 8; i++) {
		reg_file[i].ready = true;
		reg_file[i].tag = -1;

		if (DEBUG1) {
			std::cout << "[" << i << "].ready: " << reg_file[i].ready << '\n';
			std::cout << "[" << i << "].tag: " << reg_file[i].tag << '\n';
		}
	}

	// Set up the FU data structure
	for (int i = 0; i < 3; i++) {
		vector<fu_t> temp_vec(info.fu[i]);
		for (int j = 0; j < info.fu[i]; j++) {
			temp_vec[j].stage[0] = -1;
			temp_vec[j].stage[1] = -1;
			temp_vec[j].stage[2] = -1;
		}

		function_units.push_back(temp_vec);

		if (DEBUG1) {
			for (int j = 0; j < info.fu[i]; j++) {

				std::cout << "Function Unit Type " << i << " Number " << j << '\n';
				std::cout << function_units[i][j].stage[0] << ' '
					 	  << function_units[i][j].stage[1] << ' '
					 	  << function_units[i][j].stage[2] << '\n';
			}
		}
	}
}

/**
 * Subroutine that simulates the processor.
 *   The processor should fetch instructions as appropriate, until all instructions have executed
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void run_proc(proc_stats_t* p_stats) {

	int num_to_fetch;
	int cycle_count = 0;
	bool end;

	// Go through until we hit the end (or an error)
	while (!end) {

		if (DEBUG2)
			std::cout << "---------------- Cycle " << cycle_count << "-------------\n";

		// Figure out how many instructions we should fetch
		if (info.disp_q_size - info.disp_count > info.fetch_rate)
			num_to_fetch = info.fetch_rate;
		else
			num_to_fetch = info.disp_q_size - info.disp_count;

		if (DEBUG1) {
			std::cout << "Fetch " << num_to_fetch << " instructions...\n";
		}

		// Figure out how many instructions we can put in each
		// schedule queue.
		int to_schedule[3];

		for (int i = 0; i < 3; i++) {
			to_schedule[i] = info.sched_size[i] - info.sched_count[i];
		}

		// Do state update
		if (update_proc(cycle_count) == -1)
			end = true;

		// Do Execute
		if (execute_proc(cycle_count) == -1)
			end = true;

		// Do Schedule
		if (schedule_proc(cycle_count) == -1)
			end = true;

		// Do Dispatch
		if (dispatch_proc(cycle_count, to_schedule) == -1)
			end = true;

		// Do fetch
		if (fetch_proc(cycle_count, num_to_fetch) == -1)
			end = true;

		if (DEBUG2) {
			// Print out the structures
			for (int i = 0; i < (int) instructions.size(); i++) {
				std::cout << "Instruction " << instructions[i].instruction_num << '\n';
				std::cout << std::hex << instructions[i].instruction_address << ' ';
				std::cout << std::dec << instructions[i].op_code << ' '
						  << instructions[i].dest_reg << ' '
						  << instructions[i].src_reg[0] << ' '
						  << instructions[i].src_reg[1] << " \n";
				std::cout << instructions[i].cycle[0] << ' '
						  << instructions[i].cycle[1] << ' '
						  << instructions[i].cycle[2] << ' '
						  << instructions[i].cycle[3] << ' '
						  << instructions[i].cycle[4] << ' '
						  << instructions[i].cdb << ' '
						  << instructions[i].cur_stage << " \n";
				std::cout << instructions[i].sched_q_index << ' '
						  << instructions[i].src_tag[0] << ' '
						  << instructions[i].src_ready[0] << ' '
						  << instructions[i].src_tag[1] << ' '
						  << instructions[i].src_ready[1] << ' '
						  << instructions[i].fireable << " \n";
			}

			std::cout << '\n';

			// for (int i = 0; i < 7; i++) {
			// 	std::cout << "Register " << i << ": "
			// 			  << reg_file[i].ready << ' '
			// 			  << reg_file[i].tag << '\n';

			// }

			// std::cout << '\n';

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < info.fu[i]; j++) {
					std::cout << "Function Unit " << i << ':' << j << '\n';
					std::cout << function_units[i][j].stage[0] << ' '
							  << function_units[i][j].stage[1] << ' '
							  << function_units[i][j].stage[2] << '\n';
				}
			}
		}

		cycle_count++;

		if (cycle_count == CYCLE_END)
			end = true;

	}

}

/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC or branch prediction percentage
 * XXX: You're responsible for completing this routine
 *
 * @p_stats Pointer to the statistics structure
 */
void complete_proc(proc_stats_t *p_stats) {
	// Print the headings
	printf("INST\tFETCH\tDISP\tSCHED\tEXEC\tSTATE\n");

	for (int i = 0; i < (int) instructions.size(); i++) {
		std::cout << instructions[i].instruction_num << '\t'
				  << instructions[i].cycle[0] << '\t'
				  << instructions[i].cycle[1] << '\t'
				  << instructions[i].cycle[2] << '\t'
				  << instructions[i].cycle[3] << '\t'
				  << instructions[i].cycle[4] << '\n';
	}
}


int fetch_proc(int cycle, int num_to_fetch) {
	if (DEBUG2) {
		std::cout << "Fetching/Dispatching " << num_to_fetch << " instructions.\n";
	}

	if (DEBUG3) {
		if (instructions.size() >= INSTR_END)
			return 1;
	}

	proc_inst_t temp_instr;

	for (int i = 0; i < num_to_fetch; i++) {
		if (read_instruction(&temp_instr)) {
			// Successfully read an instruction
			// Alter some of the other proc_inst_t fields
			temp_instr.cycle[0] = cycle;
			temp_instr.cycle[1] = cycle+1;
			temp_instr.cycle[2] = -1;
			temp_instr.cycle[3] = -1;
			temp_instr.cycle[4] = -1;

			temp_instr.src_ready[0] = false;
			temp_instr.src_tag[0] = -1;
			temp_instr.src_ready[1] = false;
			temp_instr.src_tag[1] = -1;
			temp_instr.fireable = false;
			temp_instr.cdb = NOT_READY;
			temp_instr.cur_stage = DISPATCH;
			temp_instr.instruction_num = instructions.size()+1;

			if (temp_instr.op_code == 0 || temp_instr.op_code == -1) {
				temp_instr.sched_q_index = 0;
			}
			else if (temp_instr.op_code == 1) {
				temp_instr.sched_q_index = 1;
			}
			else if (temp_instr.op_code == 2) {
				temp_instr.sched_q_index = 2;
			}
			else {
				std::cout << "Invalid Opcode for Instruction " << temp_instr.instruction_num << '\n';
				return -1;
			}

			// Add it to the instructions list
			instructions.push_back(temp_instr);

			if (DEBUG2) {
				std::cout << "Fetched/Dispatched Instruction " << temp_instr.instruction_num << '\n';
			}

			// Increment the dispatch q
			info.disp_count++;
		}
		else {
			// Unsuccessfully read an instruction, just return, not an error
			// Most likely EOF
			std::cout << "Invalid Fetch\n";
			return 1;
		}
	}

	if (DEBUG2)
		std::cout << '\n';

	return 1;

}

int dispatch_proc(int cycle, int *num_to_schedule) {

	// Check each schedule queue and find empty slots
	for (int i = 0; i < 3; i++) {
		// How many should we schedule
		int to_schedule = num_to_schedule[i];

		if (DEBUG2) {
			std::cout << "Trying to Schedule " << to_schedule << " instructions for Queue " << i << '\n';
		}

		// For the number to schedule, look through the dispatch queue and find
		// an instruction to schedule

		for (int j = 0; j < to_schedule; j++) {
			// Go through the instructions. If we find one that is
			// in dispatch and corresponds to the correct schedule q
			// then schedule it.
			for (int k = 0; k < (int) instructions.size(); k++) {
				if (instructions[k].cur_stage == DISPATCH) {
					if (instructions[k].sched_q_index == i) {

						if (DEBUG2)
							std::cout << "Scheduled Instruction " << k+1 << '\n';

						// Get the source tags and whether they are ready
						for (int l = 0; l < 2; l++) {
							if (instructions[k].src_reg[l] != -1) {

								// Set the tags and the ready state
								instructions[k].src_ready[l] =
									reg_file[instructions[k].src_reg[l]].ready;
								instructions[k].src_tag[l] =
									reg_file[instructions[k].src_reg[l]].tag;
							}
							else
								instructions[k].src_ready[l] = true;
						}

						// Change the tag in the register file
						// If the destination reg == -1 then do nothing
						if (instructions[k].dest_reg != -1) {
							// Change the Tag
							reg_file[instructions[k].dest_reg].tag = instructions[k].instruction_num;
							reg_file[instructions[k].dest_reg].ready = false;

							if (DEBUG2) {
								for (int i = 0; i < 7; i++) {
									std::cout << "Register " << i << ": "
							  				  << reg_file[i].ready << ' '
							                  << reg_file[i].tag << '\n';
								}
							}
						}

						if (DEBUG2) {
							// Print the schedule q entry
							std::cout << "Dest: " << instructions[k].dest_reg << ' '
									  << "DTag: " << instructions[k].instruction_num << '\n';
							std::cout << "Src0: " << instructions[k].src_reg[0] << ' '
									  << "Src0_ReadY: " << instructions[k].src_ready[0] << ' '
									  << "Src0_Tag: " << instructions[k].src_tag[0] << '\n';
							std::cout << "Src1: " << instructions[k].src_reg[1] << ' '
									  << "Src1_ReadY: " << instructions[k].src_ready[1] << ' '
									  << "Src1_Tag: " << instructions[k].src_tag[1] << "\n\n";
						}

						if (instructions[k].src_ready[0] && instructions[k].src_ready[1])
							instructions[k].fireable = true;

						// Remove the instruction from the dispatch q
						instructions[k].cur_stage = SCHEDULE;

						// Increment the schedule q count
						info.sched_count[i]++;

						// Decrement the dispatch q count
						info.disp_count--;

						// Set the cycle for the dispatch stage
						instructions[k].cycle[2] = cycle+1;

						break;
					}
				}
			}
		}
	}

	if (DEBUG2)
		std::cout << '\n';

	return 1;
}

int schedule_proc(int cycle) {
	// Look through each function unit and find the ones that are empty
	// Each function unit type
	for (int i = 0; i < 3; i++) {
		// Number of function units per type
		for (int j = 0; j < info.fu[i]; j++) {
			// If nothing is executing, == -1
			if (function_units[i][j].stage[0] == -1) {
				// Go through the instructions
				for (int k = 0; k < (int) instructions.size(); k++) {
					// If it is in the schedule stage
					if (instructions[k].cur_stage == SCHEDULE) {
						// If it is for this schedule queue
						if (instructions[k].sched_q_index == i) {
							if (instructions[k].fireable) {
								if (DEBUG2) {
									std::cout << "Firing Instruction " << k+1 << '\n';
								}

								// Put it into the FU
								function_units[i][j].stage[0] = instructions[k].instruction_num;

								// Set the current stage of the instruction
								instructions[k].cur_stage = EXEC1;

								// Record the cycle it went into exectution
								instructions[k].cycle[3] = cycle+1;

								break;
							}
						}
					}
				}
			}
		}
	}

	// Go through and put instructions into CDBs
	// I am doing this here because of timing issues
	// This doesn't update the register file
	int index = 0;

	// Go through the instruction queue and look for instructions that are ready for the CDB
	while (info.cdb_count < info.num_cdb) {

		if (index >= (int) instructions.size()) {
			break;
		}

		if (instructions[index].cdb == READY) {
			// Change the state of the CDB
			instructions[index].cdb = IN_CDB;

			if (DEBUG2)
				std::cout << "Broadcasting Instruction " << index+1 << '\n';

			info.cdb_count++;
		}

		index++;
	}

	// Run through each instruction and check if this instruction has the CDB
	for (int i = 0; i < (int) instructions.size(); i++) {
		if (instructions[i].cdb == IN_CDB) {

			if (DEBUG2)
				std::cout << "Gettting Broadcast Instruction " << i+1 << '\n';

			// It has the CDB and we need to go through each instruction and
			// update any instructions that use this register.
			for (int j = 0; j < (int) instructions.size(); j++) {
				// Check the first source register
				if (instructions[j].src_tag[0] == instructions[i].instruction_num) {

					if (DEBUG2)
						std::cout << "Setting Instruction " << j+1 << " Source 1 Reg to Ready\n";

					// Set the register to ready
					instructions[j].src_ready[0] = true;

					// Unset the tag
					instructions[j].src_tag[0] = -1;
				}

				// Check the second source register
				if (instructions[j].src_tag[1] == instructions[i].instruction_num) {

					if (DEBUG2)
						std::cout << "Setting Instruction " << j+1 << " Source 1 Reg to Ready\n";

					// Set the register to ready
					instructions[j].src_ready[1] = true;

					// Unset the tag
					instructions[j].src_tag[1] = -1;
				}

				// Set fireable, if possible
				if (instructions[j].src_ready[0] && instructions[j].src_ready[1])
							instructions[j].fireable = true;

			}
		}
	}

	if (DEBUG2)
		std::cout << '\n';

	return 1;
}

int execute_proc(int cycle) {

	// Go through the Function Units
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < info.fu[i]; j++) {
			// Function unit type 0
			// Latency is one cycle so every in struction is finished
			if (i == 0) {
				// If there is an instruction executing, then it's finished
				if (function_units[i][j].stage[0] != -1) {

					int instr_num = function_units[i][j].stage[0]-1;

					if (DEBUG2)
						std::cout << "Finished executing Instruction " << instr_num+1 << '\n';

					// Remove them from the schedule q
					info.sched_count[i]--;

					// Set as cdb_ready
					instructions[instr_num].cdb = READY;

					// Set the current stage as update
					instructions[instr_num].cur_stage = UPDATE;

					// Set the cycle
					instructions[instr_num].cycle[4] = cycle+1;

					// Remove the instruction from the FU
					function_units[i][j].stage[0] = -1;
				}
			}
			// Function unit type 1
			// Latency is two cycles so advance the pipelines
			else if (i == 1) {
				// Advance the last stage of the pipeline
				if (function_units[i][j].stage[1] != -1) {
					// Get the index
					int instr_num = function_units[i][j].stage[1] - 1;

					if (DEBUG2)
						std::cout << "Finished executing Instruction " << instr_num+1 << '\n';

					// Remove them from the schedule q
					info.sched_count[i]--;

					// Set as cdb_ready
					instructions[instr_num].cdb = READY;

					// Set the current stage as update
					instructions[instr_num].cur_stage = UPDATE;

					// Set the cycle
					instructions[instr_num].cycle[4] = cycle+1;

					// Remove the instruction from the FU
					function_units[i][j].stage[1] = -1;
				}

				// Advance the first stage of the pipeline
				if (function_units[i][j].stage[0] != -1) {

					// Get the index
					int instr_num = function_units[i][j].stage[0] -1;

					// Advance the instruction
					function_units[i][j].stage[1] = instr_num+1;

					if (DEBUG2) {
						std::cout << "Advancing Instruction " << instr_num+1 << '\n';
					}

					// Change the state of the function
					instructions[instr_num].cur_stage = EXEC2;

					// Remove the instruction from the previous stage
					function_units[i][j].stage[0] = -1;

				}
			}
			// Function unit type 2
			// Latency is three cycles so advance the pipelines
			else if (i == 2) {
				// Advance the last stage of the pipeline
				if (function_units[i][j].stage[2] != -1) {
					// Get the index
					int instr_num = function_units[i][j].stage[2] - 1;

					if (DEBUG2)
						std::cout << "Finished executing Instruction " << instr_num+1 << '\n';

					// Remove them from the schedule q
					info.sched_count[i]--;

					// Set as cdb_ready
					instructions[instr_num].cdb = READY;

					// Set the current stage as update
					instructions[instr_num].cur_stage = UPDATE;

					// Set the cycle
					instructions[instr_num].cycle[4] = cycle + 1;

					// Remove the instruction from the FU
					function_units[i][j].stage[2] = -1;
				}

				if (function_units[i][j].stage[1] != -1) {
					// Get the index
					int instr_num = function_units[i][j].stage[1] - 1;

					// Advance the instruction
					function_units[i][j].stage[2] = instr_num + 1;

					if (DEBUG2) {
						std::cout << "Advancing Instruction " << instr_num+1 << '\n';
					}

					// Change the state of the function
					instructions[instr_num].cur_stage = EXEC3;

					// Remove the instruction from the FU
					function_units[i][j].stage[1] = -1;
				}

				// Advance the first stage of the pipeline
				if (function_units[i][j].stage[0] != -1) {
					// Get the index
					int instr_num = function_units[i][j].stage[0] - 1;

					// Advance the instruction
					function_units[i][j].stage[1] = instr_num + 1;

					if (DEBUG2) {
						std::cout << "Advancing Instruction " << instr_num+1 << '\n';
					}

					// Change the state of the function
					instructions[instr_num].cur_stage = EXEC2;

					// Remove the instruction from the FU
					function_units[i][j].stage[0] = -1;
				}
			}
		}
	}

	if (DEBUG2)
		std::cout << '\n';

	return 1;
}

int update_proc(int cycle) {

	// Go through the instruction queue and look for instructions that have the CDB
	for (int i = 0; i < (int) instructions.size(); i++) {
		// If it has the CDB
		if (instructions[i].cdb == IN_CDB) {
			//Update the register file
			if (reg_file[instructions[i].dest_reg].tag == instructions[i].instruction_num)
				reg_file[instructions[i].dest_reg].ready = true;

			if (DEBUG2) {
				std::cout << "Updateing Register File from Instruction " << i+1 << '\n';
				std::cout << "Finished Instruction " << i+1 << '\n';
			}

			// Now it doesn't need the CDB anymore
			instructions[i].cdb = NOT_READY;

			// Now the instruction is finished
			instructions[i].cur_stage = FINISHED;

			// Decrement the CDB counter
			info.cdb_count--;
		}

	}

	if (DEBUG2)
		std::cout << '\n';

	return 1;
}
