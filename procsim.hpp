#ifndef PROCSIM_HPP
#define PROCSIM_HPP

#include <cstdint>

#define DEFAULT_K0 1
#define DEFAULT_K1 2
#define DEFAULT_K2 3
#define DEFAULT_D 2
#define DEFAULT_M 2
#define DEFAULT_F 4
#define DEFAULT_C 4

typedef struct _proc_inst_t
{
    uint32_t instruction_address;
    int32_t op_code;
    int32_t src_reg[2];
    int32_t dest_reg;
    
    // You may introduce other fields as needed

    // The current stage of the instruction
    // 1 = fetch, 2 = dispatch, 3 = schedule, 4 = execture, 5 = update
    int cur_stage;

    // Used when we need to tag a register
    int instruction_num;
    
} proc_inst_t;

typedef struct _proc_stats_t
{
    float avg_inst_fire;
    unsigned long retired_instruction;
    unsigned long cycle_count;
    float avg_ipc;
    float perc_branch_pred;
} proc_stats_t;

bool read_instruction(proc_inst_t* p_inst);

void setup_proc(uint64_t d, uint64_t k0, uint64_t k1, uint64_t k2, uint64_t f, uint64_t m, uint64_t c);
void run_proc(proc_stats_t* p_stats);
void complete_proc(proc_stats_t* p_stats);

// This holds the information that was sent in through the command-line
typedef struct _proc_info_t {
    int dispatch_q_mult;
    int fetch_rate;
    int schedule_q_mult;
    int num_k0;
    int num_k1;
    int num_k2;
    int num_cdb;
    // Keep track of the instruction number that is being read
    int inst_num; 
} proc_info_t;

// Entries for the register file
typedef struct _reg_file_t {
    bool ready;
    int tag;
    int value;
} reg_file_t;

// Struct to show schedule q entries
// These entries will be held in a global array/vector
typedef struct _sched_q_entry {
    proc_inst_t inst; // Hold the instruction
    int dest_reg;
    int dest_tag;
    int src1_ready;
    int src1_tag;
    int src2_ready;
    int src2_tag;
    int fireable; // The instruction is ready to fire
} sched_q_entry;

// The dispatch queue will just be a queue (duh!)
// Each entry will just be an instruction proc_inst_t
//
// First problem, should this be global or not
// Second problem, need to use push/pop

// In order to keep track of the cycle that each instruction
// enters each stage, I am going to have a two dimensional array
// There will be a row for every instruction and a column for 
// every stage (Fetch, Dispatch, Schedule, Execute, Update)
//
// First problem, figure out how many instructions there are

// I want to have a structure array/vector that will model the CDBs
// There will be one element for each CDB and when an instruction 
// finishes executing, it will be put into a CDB slot. The schedule
// state will check each CDB for instruction information and then it
// will remove it from the CDB slot. 

#endif /* PROCSIM_HPP */
