#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mem.h"

#define XLEN 32
#define DEBUG 1

// OPCODES
#define MATH    0b0110011
#define MATHI   0b0010011
#define STORE   0b0100011
#define LOAD    0b0000011
#define BRANCH  0b1100011
#define JALR    0b1100111
#define JAL     0b1101111
#define AUIPC   0b0010111
#define LUI     0b0110111

// FUNCT3 - MATH
#define ADD_SUB 0
#define SLL 1
#define SLT 2
#define SLTU 3
#define XOR 4
#define SRL_SRA 5
#define OR 6
#define AND 7

// FUNCT3 - STORE
#define SB 0
#define SH 1
#define SW 2

// FUNCT3 - LOAD
#define LB 0
#define LH 1
#define LW 2
#define LBU 4
#define LHU 5

// FUNCT3 - BRANCH
#define BEQ 0
#define BNE 1
#define BLT 4
#define BGE 5
#define BLTU 6
#define BGEU 7

struct cpu {
    uint32_t pc;
    int32_t regfile[32];
    struct memory ram;
};