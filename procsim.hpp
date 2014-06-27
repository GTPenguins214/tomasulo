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

    // These are used to keep track of the cycle in which the instruction
    // first got into that stage. It's used for the output
    int fetch_cycle;
    int dispatch_cycle;
    int schedule_cycle;
    int execute_cycle;
    int state_cycle;

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

#endif /* PROCSIM_HPP */
