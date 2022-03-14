// RISCV 32 BIT EMULATOR
#include "emulator.h"

void init_cpu(struct cpu *otter);
void run(struct cpu *otter);
uint32_t fetch(struct cpu *otter);
void execute(struct cpu *otter, uint32_t instruction);
void execute_math(struct cpu *otter, uint32_t instruction, uint8_t opcode);
void execute_store(struct cpu *otter, uint32_t instruction);
void execute_load(struct cpu *otter, uint32_t instruction);
void execute_jalr(struct cpu *otter, uint32_t instruction, uint8_t *pc_set);
void write_regfile(struct cpu *otter, uint8_t addr, uint32_t data);
uint32_t read_regfile(struct cpu *otter, uint8_t addr);
uint8_t get_opcode(uint32_t instruction);
uint8_t get_funct3(uint32_t instruction);
uint8_t get_funct7(uint32_t instruction);
uint8_t get_rs1(uint32_t instruction);
uint8_t get_rs2(uint32_t instruction);
uint8_t get_rd(uint32_t instruction);
uint32_t get_immed_I(uint32_t instruction);
uint32_t get_immed_S(uint32_t instruction);
void dump_regfile(struct cpu *otter);

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
        execute(otter, instruction);
    }

    if (DEBUG) dump_regfile(otter);
}

void dump_regfile(struct cpu *otter) {
    int i;

    printf("REGFILE:\n");
    for (i = 0; i < 32; i++) {
        printf("x%02d: 0x%x\n", i, otter->regfile[i]);
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
            printf("BRANCH\n"); break;
        case JALR:  
            execute_jalr(otter, instruction, &pc_set);
            break;
        case JAL:   
            printf("JAL\n");    break;
        case AUIPC: 
            printf("AUIPC\n");  break;
        case LUI:   
            printf("LUI\n");    break;
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
    uint8_t rd = get_rd(instruction), rs1 = get_rs1(instruction),
        funct3 = get_funct3(instruction), funct7 = get_funct7(instruction);

    // Get A and B
    a = otter->regfile[rs1];

    if (opcode == MATH) {
        b = otter->regfile[get_rs2(instruction)];
    } else {
        b = get_immed_I(instruction);
    }

    // Decode instruction
    switch (funct3) {
        case ADD_SUB:
            if (funct7 == 0) { // ADD
                res = a + b;
            } else { // SUB
                res = a - b;
            }
            break;
        case SLL:
            res = a << (b & 0x1F);
            break;
        case SLT:
            res = ((int32_t)a < (int32_t)b) ? 1 : 0;
            break;
        case SLTU:
            res = (a < b) ? 1 : 0;
            break;
        case XOR:
            res = a ^ b;
            break;
        case SRL_SRA:
            if (funct7 == 0) { // SRL
                res = a >> (b & 0x1F);
            } else { // SRA
                res = (int32_t)a >> (b & 0x1F); 
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

    write_regfile(otter, rd, res);
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
    uint8_t funct3 = get_funct3(instruction), rd = get_rd(instruction);
    uint32_t addr = read_regfile(otter, get_rs1(instruction))  + get_immed_I(instruction);
    uint8_t size;

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
            fprintf(stderr, "LBU not implemented.\n");
            return;
        case LHU:
            fprintf(stderr, "LHU not implemented.\n");
            return;
        default:
            fprintf(stderr, "Unknown load funct3: %X\n", funct3);
            return;
    }

    if (DEBUG) printf("Loading %dB from memory 0x%x\n", size, addr);
    read_mem(&otter->ram, addr, otter->regfile + rd, size);
}

void execute_jalr(struct cpu *otter, uint32_t instruction, uint8_t *pc_set) {
    uint32_t target = read_regfile(otter, get_rs1(instruction)) + get_immed_I(instruction);
    // Set LSB of target to 0
    target &= 0xFFFFFFFE;
    // Write PC+4 to RD
    write_regfile(otter, get_rd(instruction), otter->pc + 4);
    // Jump to target
    otter->pc = target;
    *pc_set = 1;
}

void write_regfile(struct cpu *otter, uint8_t addr, uint32_t data) {
    if (addr == 0) {
        return;
    }
    if (DEBUG) printf("Writing 0x%x to x%d\n", data, addr);
    otter->regfile[addr] = data;
}

uint32_t read_regfile(struct cpu *otter, uint8_t addr) {
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

uint32_t get_immed_I(uint32_t instruction) {
    return (instruction & 0xFFF00000) >> 20;
}

uint32_t get_immed_S(uint32_t instruction) {
    return ((instruction & 0xFE000000) >> 20) | ((instruction >> 7) & 0x1F);
}