#ifndef PROCSIM_HPP
#define PROCSIM_HPP

#include <cstdint>

#define DEBUG1 0 // Level 1 debugging, previously used debugging options
#define DEBUG2 0 // Currently used debugging options
#define DEBUG3 1 // Level 3 debugging, future debugging options

#define CYCLE_END 2000
#define INSTR_END 1000
#define CYCLE_PRINT 500

#define DEFAULT_K0 1
#define DEFAULT_K1 2
#define DEFAULT_K2 3
#define DEFAULT_D 2
#define DEFAULT_M 2
#define DEFAULT_F 4
#define DEFAULT_C 4

#define NUM_REGISTERS 8

typedef enum _stage_t {
    FETCH = 0,
    DISPATCH = 1,
    SCHEDULE = 2,
    EXEC1 = 3,
    EXEC2 = 4,
    EXEC3 = 5,
    UPDATE = 6,
    REMOVE = 7,
    FINISHED = 8
} stage_t;

typedef enum _cdb_state_t {
    NOT_READY = 0, // Not ready for the CDB
    READY = 1, // Ready for the CDB
    IN_CDB = 2 // In the CDB
} cdb_state_t;

typedef struct _proc_inst_t
{
    uint32_t instruction_address;
    int32_t op_code;
    int32_t src_reg[2];
    int32_t dest_reg;

    // Used to keep track of which cycle each stage was in.
    int cycle[5];

    // Only used for the schedule queue
    int sched_q_index;
    bool src_ready[2];
    int src_tag[2];
    bool fireable; // The instruction is ready to fire

    // Used to figure out when to delete from the dispatch queue
    bool del_dispatch;

    // Used for bus arbitration
    cdb_state_t cdb;

    // The current stage of the instruction, an enum
    stage_t cur_stage;

    // Used when we need to tag a register, synonymous to tag
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

int fetch_proc(int cycle);
int dispatch_proc(int cycle);
int schedule_proc(int cycle);
int execute_proc(int cycle);
int update_proc(int cycle);

// This holds the information that was sent in through the command-line
typedef struct _proc_info_t {
    int disp_q_mult;
    int disp_q_size;

    int fetch_rate;

    int sched_q_mult;
    int sched_size[3];

    int fu[3];
    int num_cdb;

    // queue counts
    int sched_count[3];
    int disp_count;
    int cdb_count;

    // last instructions
    int last_dispatched;
    int last_finished;
} proc_info_t;

// Entries for the register file
typedef struct _reg_file_t {
    bool ready;
    int tag;
} reg_file_t;

// Function unit entries
typedef struct _fu_t {
    int stage[3];
} fu_t;

// CDB entries
typedef struct _cdb_t {
    proc_inst_t *instr;
    bool valid;
} cdb_t;

#endif /* PROCSIM_HPP */
