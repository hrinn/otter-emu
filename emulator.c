// 32-BIT RISCV EMULATOR
#include "emulator.h"

void init_cpu(struct cpu *otter);
void run(struct cpu *otter);
uint32_t fetch(struct cpu *otter);
void execute(struct cpu *otter, uint32_t instruction);
void execute_math(struct cpu *otter, uint32_t instruction, uint8_t opcode);
void execute_store(struct cpu *otter, uint32_t instruction);
void execute_load(struct cpu *otter, uint32_t instruction);
void execute_branch(struct cpu *otter, uint32_t instruction, uint8_t *pc_set);
void execute_jalr(struct cpu *otter, uint32_t instruction, uint8_t *pc_set);
void execute_jal(struct cpu *otter, uint32_t instruction, uint8_t *pc_set);
void execute_auipc(struct cpu *otter, uint32_t instruction);
void execute_lui(struct cpu *otter, uint32_t instruction);
void write_regfile(struct cpu *otter, uint8_t addr, int32_t data);
int32_t read_regfile(struct cpu *otter, uint8_t addr);
uint8_t get_opcode(uint32_t instruction);
uint8_t get_funct3(uint32_t instruction);
uint8_t get_funct7(uint32_t instruction);
uint8_t get_rs1(uint32_t instruction);
uint8_t get_rs2(uint32_t instruction);
uint8_t get_rd(uint32_t instruction);
int32_t get_immed_I(uint32_t instruction);
int32_t get_immed_S(uint32_t instruction);
int32_t get_immed_B(uint32_t instruction);
int32_t get_immed_J(uint32_t instruction);
int32_t get_immed_U(uint32_t instruction);
void dump_regfile(struct cpu *otter);

const char *reg_labels[32] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0/fp", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

int main(int argc, char *argv[]) {
    int fd;

    if (argc != 2) {
        fprintf(stderr, "Incorrect number of arguments. Usage: ./emulator [prog.bin]\n");
        return -1;
    }

    // Read binary file
    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Unable to open memory file: %s\n", argv[1]);
        return -1;
    }

    // Setup CPU, load program, and run
    struct cpu otter;
    init_cpu(&otter);
    init_mem(&otter.ram, fd);
    run(&otter);

    free(otter.ram.mem);
    close(fd);
    return 0;
}

void init_cpu(struct cpu *otter) {
    otter->pc = MEM_BASE;   // Initialize pc to base of memory
    memset(otter->regfile, 0, sizeof(otter->regfile));
    otter->regfile[2] = MEM_BASE + MEM_SIZE; // Initialize stack pointer to top of memory
    otter->ram.mem = alloc_mem();
}

void run(struct cpu *otter) {
    uint32_t instruction = 0;

    while (1) {
        if (otter->pc == 0) break;
        instruction = fetch(otter);
        if (instruction == 0) break;
        execute(otter, instruction);
    }

    if (DEBUG) dump_regfile(otter);
}

void dump_regfile(struct cpu *otter) {
    int i;

    printf("REGFILE:\n");
    for (i = 0; i < 32; i++) {
        printf("x%02d/%s: 0x%x\n", i, reg_labels[i], otter->regfile[i]);
    }
}

uint32_t fetch(struct cpu *otter) {
    uint32_t instruction = 0;
    read_mem(&otter->ram, otter->pc, &instruction, WORD);
    if (DEBUG) printf("PC=0x%x, INSTR=0x%08x\n", otter->pc, instruction);
    return instruction;
}

void execute(struct cpu *otter, uint32_t instruction) {
    uint8_t opcode = get_opcode(instruction);
    uint8_t pc_set = 0;

    switch (opcode) {
        case MATH:
        case MATHI:  
            execute_math(otter, instruction, opcode);
            break;
        case STORE: 
            execute_store(otter, instruction);
            break;
        case LOAD:
            execute_load(otter, instruction);
            break;
        case BRANCH:
            execute_branch(otter, instruction, &pc_set);
            break;
        case JALR:  
            execute_jalr(otter, instruction, &pc_set);
            break;
        case JAL:   
            execute_jal(otter, instruction, &pc_set);
            break;
        case AUIPC: 
            execute_auipc(otter, instruction);
            break;
        case LUI:   
            execute_lui(otter, instruction);
            break;
        default:
            fprintf(stderr, "Unknown opcode: %X\n", opcode);
            break;
    }

    if (!pc_set) {
        otter->pc += 4;
    }
}

void execute_math(struct cpu *otter, uint32_t instruction, uint8_t opcode) {
    uint32_t a, b, res;
    uint8_t funct3 = get_funct3(instruction), funct7 = get_funct7(instruction);

    // Get A and B
    a = read_regfile(otter, get_rs1(instruction));

    if (opcode == MATH) {
        b = read_regfile(otter, get_rs2(instruction));
    } else {
        b = get_immed_I(instruction);
    }

    // Decode instruction
    switch (funct3) {
        case ADD_SUB:
            if (funct7 == 0 || opcode == MATHI) { // ADD
                res = a + b;
            } else { // SUB
                res = a - b;
            }
            break;
        case SLL:
            res = a << (b & 0x1F);
            break;
        case SLT:
            res = (a < b) ? 1 : 0;
            break;
        case SLTU:
            res = ((uint32_t)a < (uint32_t)b) ? 1 : 0;
            break;
        case XOR:
            res = a ^ b;
            break;
        case SRL_SRA:
            if (funct7 == 0) { // SRL
                res = (uint32_t)a >> (b & 0x1F);
            } else { // SRA
                res = a >> (b & 0x1F); 
            }
        case OR:
            res = a | b;
            break;
        case AND:
            res = a & b;
            break;
        default:
            fprintf(stderr, "Unknown math funct3: %X\n", funct3);
            return;
    }

    write_regfile(otter, get_rd(instruction), res);
}

void execute_store(struct cpu *otter, uint32_t instruction) {
    uint8_t funct3 = get_funct3(instruction), data_byte;
    uint16_t data_half;
    uint32_t dest = read_regfile(otter, get_rs1(instruction)),
        data_word = read_regfile(otter, get_rs2(instruction)),
        shift = get_immed_S(instruction);

    uint32_t addr = dest + shift;


    switch (funct3) {
        case SB:
            data_byte = data_word & 0xFF;
            write_mem(&otter->ram, addr, &data_byte, BYTE);
            break;
        case SH:
            data_half = data_word & 0xFFFF;
            write_mem(&otter->ram, addr, &data_half, HALFWORD);
            break;
        case SW:
            write_mem(&otter->ram, addr, &data_word, WORD);
            break;
        default:
            fprintf(stderr, "Unknown store funct3: %X\n", funct3);
            return;
    }
}

void execute_load(struct cpu *otter, uint32_t instruction) {
    uint8_t funct3 = get_funct3(instruction);;
    uint32_t addr = read_regfile(otter, get_rs1(instruction)) + get_immed_I(instruction);
    uint8_t size;
    uint32_t mask = 0xFFFFFFFF;
    int32_t data;

    switch (funct3) {
        case LB:
            size = BYTE;
            break;
        case LH:
            size = HALFWORD;
            break;
        case LW:
            size = WORD;
            break;
        case LBU:
            size = BYTE;
            mask = 0x000000FF;
            break;
        case LHU:
            size = HALFWORD;
            mask = 0x0000FFFF;
            break;
        default:
            fprintf(stderr, "Unknown load funct3: %X\n", funct3);
            return;
    }

    if (DEBUG) printf("Loading %dB from memory 0x%x\n", size, addr);
    read_mem(&otter->ram, addr, &data, size);
    data &= mask;
    write_regfile(otter, get_rd(instruction), data);
}

void execute_branch(struct cpu *otter, uint32_t instruction, uint8_t *pc_set) {
    uint8_t funct3 = get_funct3(instruction);
    int32_t a = read_regfile(otter, get_rs1(instruction)), 
        b = read_regfile(otter, get_rs2(instruction));

    // Branch condition generator
    uint8_t br_eq = a == b,
        br_lt = a < b,
        br_ltu = (uint32_t)a < (uint32_t)b;

    switch (funct3) {
        case BEQ:
            if (br_eq) *pc_set = 1;
            break;
        case BNE:
            if (!br_eq) *pc_set = 1;
            break;
        case BLT:
            if (br_lt) *pc_set = 1;
            break;
        case BGE:
            if (!br_lt) *pc_set = 1;
            break;
        case BLTU:
            if (br_ltu) *pc_set = 1;
            break;
        case BGEU:
            if (!br_ltu) *pc_set = 1;
        default:
            fprintf(stderr, "Unknown branch funct3: %X\n", funct3);
            return;
    }

    if (*pc_set) {
        otter->pc += get_immed_B(instruction);
        if (DEBUG) printf("Branch taken: PC = 0x%x\n", otter->pc);
    }
}

void execute_jalr(struct cpu *otter, uint32_t instruction, uint8_t *pc_set) {
    uint32_t target = (read_regfile(otter, get_rs1(instruction)) 
        + get_immed_I(instruction));
    // Write PC+4 to RD
    write_regfile(otter, get_rd(instruction), otter->pc + 4);
    // Jump to target
    otter->pc = target;
    *pc_set = 1;
}

void execute_jal(struct cpu *otter, uint32_t instruction, uint8_t *pc_set) {
    write_regfile(otter, get_rd(instruction), otter->pc + 4);
    otter->pc += get_immed_J(instruction);
    *pc_set = 1;
}

void execute_auipc(struct cpu *otter, uint32_t instruction) {
    write_regfile(otter, get_rd(instruction), otter->pc + get_immed_U(instruction));
}

void execute_lui(struct cpu *otter, uint32_t instruction) {
    write_regfile(otter, get_rd(instruction), get_immed_U(instruction));
}

void write_regfile(struct cpu *otter, uint8_t addr, int32_t data) {
    if (addr == 0) {
        return;
    }
    if (DEBUG) printf("Writing 0x%x to x%d\n", data, addr);
    otter->regfile[addr] = data;
}

int32_t read_regfile(struct cpu *otter, uint8_t addr) {
    if (DEBUG) printf("Read 0x%x from x%d\n", otter->regfile[addr], addr);
    return otter->regfile[addr];
}

uint8_t get_opcode(uint32_t instruction) {
    return instruction & 0x7F;
}

uint8_t get_funct3(uint32_t instruction) {
    return (instruction >> 12) & 0x7;
}

uint8_t get_funct7(uint32_t instruction) {
    return instruction >> 25;
}

uint8_t get_rs1(uint32_t instruction) {
    return (instruction >> 15) & 0x1F;
}

uint8_t get_rs2(uint32_t instruction) {
    return (instruction >> 20) & 0x1F;
}

uint8_t get_rd(uint32_t instruction) {
    return (instruction >> 7) & 0x1F;
}

int32_t get_immed_I(uint32_t instruction) {
    return ((int32_t)(instruction & 0x80000000) >> 20) | 
        (instruction & 0xFFF00000) >> 20;
}

int32_t get_immed_S(uint32_t instruction) {
    return ((int32_t)(instruction & 0x80000000) >> 20) |
        ((instruction & 0xFE000000) >> 20) | 
        ((instruction >> 7) & 0x1F);
}

int32_t get_immed_B(uint32_t instruction) {
    return ((int32_t)(instruction & 0x80000000) >> 19) |
        ((instruction & 0x80) << 4) |
        ((instruction >> 20 ) & 0x7E0) |
        ((instruction >> 7) & 0x1E);
}

int32_t get_immed_J(uint32_t instruction) {
    return ((instruction & 0x80000000) >> 11) |
        (instruction & 0xFF000) |
        ((instruction >> 9) & 0x800) |
        ((instruction >> 20) & 0x7FE);
}

int32_t get_immed_U(uint32_t instruction) {
    return instruction & 0xFFFFF000;
}