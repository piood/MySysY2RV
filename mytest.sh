cmake -DCMAKE_BUILD_TYPE=Debug -B build
cmake --build build
build/compiler -koopa hello.c -o hello.koopa
build/compiler -riscv hello.c -o hello.S
clang hello.S -c -o hello.o -target riscv32-unknown-linux-elf -march=rv32im -mabi=ilp32
ld.lld hello.o -L$CDE_LIBRARY_PATH/riscv32 -lsysy -o hello
qemu-riscv32-static hello; echo $?