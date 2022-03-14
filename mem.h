#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define MEM_SIZE 1024 * 1024 * 8 // 8 Mb
#define MEM_BASE 0x80000000

#define WORD 4
#define HALFWORD 2
#define BYTE 1

// MMIO
#define MMIO_PUTCHAR 0x110C0000

struct memory {
    uint8_t *mem;
};

void init_mem(struct memory *m, int fd);
uint8_t *alloc_mem();
void read_mem(struct memory *m, uint32_t addr, void *buffer, uint8_t n);
void write_mem(struct memory *m, uint32_t addr, void *data, uint8_t n);