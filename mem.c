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
    if (!(n == WORD || n == HALFWORD || n == BYTE)) {
        fprintf(stderr, "Unsupported memory size on read.");
        return;
    }

    // Trap MMIO accesses
    if (addr >= MEM_BASE && addr < MEM_BASE + MEM_SIZE) {
        memcpy(buffer, (m->mem + addr) - MEM_BASE, n);
    } else {
        fprintf(stderr, "Attempt to read from unsupported memory region: 0x%08X\n", addr);
    }
}

// Writes n bytes from data, stores it at mem[addr]
void write_mem(struct memory *m, uint32_t addr, void *data, uint8_t n) {
    if (!(n == WORD || n == HALFWORD || n == BYTE)) {
        fprintf(stderr, "Unsupported memory size on read.");
        return;
    }

    if (addr >= MEM_BASE && addr < MEM_BASE + MEM_SIZE) {
        memcpy((m->mem + addr) - MEM_BASE, data, n);
        return;
    }

    // Writeable MMIO
    switch (addr) {
        case MMIO_PUTCHAR:
            printf("%c", *(char *)data);
            break;
        case MMIO_PUTNUM:
            printf("%d\n", *(int *)data);
            break;
        default:
            fprintf(stderr, "Attempt to write 0x%x to unsupported memory region: 0x%08X\n", *(int *)data, addr);
    }
}