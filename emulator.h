#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mem.h"

#define XLEN 32

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

struct cpu {
    uint32_t pc;
    uint32_t regfile[32];
    struct memory ram;
};