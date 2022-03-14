// RISCV 32 BIT EMULATOR
#include "emulator.h"

void init_cpu(struct cpu *otter);
void run(struct cpu *otter);
uint32_t fetch(struct cpu *otter);
uint8_t get_opcode(uint32_t instruction);
void execute(struct cpu *otter, uint32_t instruction);

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

    // Instantiate CPU
    struct cpu otter;
    init_cpu(&otter);
    init_mem(&otter.ram, fd);
    run(&otter);

    free(otter.ram.mem);
    close(fd);
    return 0;
}

void init_cpu(struct cpu *otter) {
    otter->pc = 0;
    memset(otter->regfile, 0, sizeof(otter->regfile));
    otter->ram.mem = alloc_mem();
}

void run(struct cpu *otter) {
    uint32_t instruction;

    while (1) {
        instruction = fetch(otter);
        execute(otter, instruction);

        if (instruction == 0) {
            break;
        }
    }
}

uint32_t fetch(struct cpu *otter) {
    uint32_t instruction;
    read_memory(&otter->ram, otter->pc, &instruction, WORD);
    return instruction;
}

void execute(struct cpu *otter, uint32_t instruction) {
    uint8_t opcode = get_opcode(instruction);

    // Decode
    printf("0x%02X - 0x%08X - ", otter->pc, instruction);
    switch (opcode) {
        case MATH:  
            printf("MATH\n");   break;
        case MATHI: 
            printf("MATHI\n");  break;
        case STORE: 
            printf("STORE\n");  break; 
        case LOAD:  
            printf("LOAD\n");   break;
        case BRANCH:
            printf("BRANCH\n"); break;
        case JALR:  
            printf("JALR\n");   break;
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

    
    otter->pc += 4;
}

uint8_t get_opcode(uint32_t instruction) {
    return instruction & 0x7F;
}