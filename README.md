# otter-emu
A 32 bit RISC-V emulator written in C. Based on the OTTER's architecture--a Cal Poly, SLO CPE Department project. Currently supports the RV32I base instructions.

## Installing RISC-V Toolchain
```
> git clone https://github.com/riscv/riscv-gnu-toolchain
> cd riscv-gnu-toolchain
> ./configure --prefix=<path-to-toolchain> --with-arch=rv32g
> make && make linux
```
Ensure that `path-to-toolchain/bin` is on your path.

## Credits
Inspiration and guidance from:
* [d0iasm's Rust RISC-V Emulator](https://book.rvemu.app/)
* [fmash16's C RISC-V Emulator](https://fmash16.github.io/content/posts/riscv-emulator-in-c.html)