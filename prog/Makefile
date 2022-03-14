ctest: ctest.c
	riscv32-unknown-elf-gcc -S ctest.c
	riscv32-unknown-elf-gcc -Wl,-Ttext=0x0 -nostdlib -march=rv32i -mabi=ilp32 -o ctest ctest.s
	riscv32-unknown-elf-objcopy -O binary ctest ctest.bin

stest: stest.s
	riscv32-unknown-elf-gcc -Wl,-Ttext=0x0 -nostdlib -march=rv32i -mabi=ilp32 -o stest stest.s
	riscv32-unknown-elf-objcopy -O binary stest stest.bin
	
clean:
	rm ctest stest *.bin *.o ctest.s