# otter-emu
A 32 bit RISC-V emulator. Based on the OTTER's architecture--a Cal Poly CPE Department project.

## Installing RISC-V Toolchain
```
> git clone https://github.com/riscv/riscv-gnu-toolchain
> cd riscv-gnu-toolchain
> ./configure --prefix=<path-to-toolchain> --with-arch=rv32g
> make && make linux
```

## Credits
Inspiration and guidance from:
* [Rust RISC-V Emulator](https://book.rvemu.app/)
* [Simple RISC-V Emulator](https://fmash16.github.io/content/posts/riscv-emulator-in-c.html)