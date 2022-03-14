#include "mem.h"

// Reads the contents of the file pointer into the provided memory
void init_mem(struct memory *m, int fd) {
    struct stat buf;
    fstat(fd, &buf);
    read(fd, m->mem, buf.st_size);
}

// Allocates the memory on the heap
uint8_t *alloc_mem() {
    return calloc(MEM_SIZE, 1);
}

// Reads n bytes from mem[addr], stores it in the provided buffer
void read_mem(struct memory *m, uint32_t addr, void *buffer, uint8_t n) {
    // Trap MMIO accesses

    if (!(n == WORD || n == HALFWORD || n == BYTE)) {
        fprintf(stderr, "Unsupported memory size on read.");
        return;
    }

    memcpy(buffer, m->mem + addr, n);
}

// Writes n bytes from data, stores it at mem[addr]
void write_mem(struct memory *m, uint32_t addr, void *data, uint8_t n) {
    // Trap MMIO

    if (!(n == WORD || n == HALFWORD || n == BYTE)) {
        fprintf(stderr, "Unsupported memory size on read.");
        return;
    }

    memcpy(m->mem + addr, data, n);
}