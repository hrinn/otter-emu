test: test.c
	riscv32-unknown-elf-gcc -S test.c
	riscv32-unknown-elf-gcc -Wl,-Ttext=0x0 -nostdlib -march=rv32i -mabi=ilp32 -o test test.s
	riscv32-unknown-elf-objcopy -O binary test test.bin

clean:
	rm test test.s test.bin