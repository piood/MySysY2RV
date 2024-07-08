# MySysY2RV
北大编译原理课程，使用C++，独立完成的C语言子集SysY编译器，实现了从C语言编译到Koopa ir，再从Koopa ir到RISC-V汇编的编译。

```C
int a = 10;

int inc() {
  a = a + 1;
  return a;
}

void print_a() {
  putint(a);
  putch(10);
}

int main() {
  int i = 0;
  while (i < 10) {
    inc();
    int a = 1;
    a = a + 2;
    putint(a);
    putch(10);
    print_a();
    i = i + 1;
  }
  return 0;
}
```

通过将上述C语言（SysY语言）翻译成Koopa IR

```Koopa IR
decl @putint(i32)
decl @putch(i32)

global @COMPILER__a_ = alloc i32, 10
fun @inc(): i32{
%entry:
  %0 = load @COMPILER__a_
  %1 = add %0, 1
  store %1, @COMPILER__a_
  %2 = load @COMPILER__a_
  ret %2

}

fun @print_a(){
%entry:
  %3 = load @COMPILER__a_
  call @putint(%3)
  call @putch(10)
  ret
}

fun @main(): i32{
%entry:
  @COMPILER__i__43_0 = alloc i32
  store 0, @COMPILER__i__43_0
  jump %while_1

%while_1:
  %4 = load @COMPILER__i__43_0
  %5 = lt %4, 10
	br %5, %while_then_1, %end_while_1

%while_then_1:
  %6 = call @inc()
  @COMPILER__a__43_0_31_0 = alloc i32
  store 1, @COMPILER__a__43_0_31_0
  %7 = load @COMPILER__a__43_0_31_0
  %8 = add %7, 2
  store %8, @COMPILER__a__43_0_31_0
  %9 = load @COMPILER__a__43_0_31_0
  call @putint(%9)
  call @putch(10)
  call @print_a()
  %10 = load @COMPILER__i__43_0
  %11 = add %10, 1
  store %11, @COMPILER__i__43_0
	jump %while_1

%end_while_1:
  ret 0

}

```

再将Koopa IR转换成RSIC-V汇编程序，可以直接运行

```RSIC-V asm
  .data
  .globl COMPILER__a_
COMPILER__a_:
  .word 10

  .text
  .globl inc
inc:
  addi sp, sp, -32
  la t0, COMPILER__a_
  lw t0, 0(t0)
  li t4, 32
  add t4, t4, sp
  sw t0, (t4)
  lw t0, 32(sp)
  li t1, 1
  add t0, t1, t0
  sw t0, 28(sp)

  lw t0, 28(sp)
  la t1, COMPILER__a_
  sw t0, 0(t1)
  la t0, COMPILER__a_
  lw t0, 0(t0)
  li t4, 24
  add t4, t4, sp
  sw t0, (t4)
  lw a0, 24(sp)
  addi sp, sp, 32
  ret

  .text
  .globl print_a
print_a:
  addi sp, sp, -32
  la t0, COMPILER__a_
  lw t0, 0(t0)
  li t4, 32
  add t4, t4, sp
  sw t0, (t4)
  sw ra, 28(sp)
  lw a0, 32(sp)
  call putint
  sw a0, 24(sp)

  lw ra, 28(sp)
  sw ra, 20(sp)
  li t0, 10
  sw t0, 16(sp)

  lw a0, 16(sp)
  call putch
  sw a0, 12(sp)

  lw ra, 20(sp)
  li a0, 0
  addi sp, sp, 32
  ret

  .text
  .globl main
main:
  addi sp, sp, -112
  li t0, 0
  sw t0, 112(sp)
  j while_1

while_1:
  lw t0, 112(sp)
  sw t0, 108(sp)

  lw t0, 108(sp)
  li t1, 10
  slt t0, t0, t1
  sw t0, 104(sp)

  lw t0, 104(sp)
  bnez t0, while_then_1
  j end_while_1

while_then_1:
  sw ra, 100(sp)
  call inc
  sw a0, 96(sp)

  lw ra, 100(sp)
  li t0, 1
  sw t0, 92(sp)
  lw t0, 92(sp)
  sw t0, 88(sp)

  lw t0, 88(sp)
  li t1, 2
  add t0, t1, t0
  sw t0, 84(sp)

  lw t0, 84(sp)
  sw t0, 92(sp)
  lw t0, 92(sp)
  sw t0, 80(sp)

  sw ra, 76(sp)
  lw a0, 80(sp)
  call putint
  sw a0, 72(sp)

  lw ra, 76(sp)
  sw ra, 68(sp)
  li t0, 10
  sw t0, 64(sp)

  lw a0, 64(sp)
  call putch
  sw a0, 60(sp)

  lw ra, 68(sp)
  sw ra, 56(sp)
  call print_a
  sw a0, 52(sp)

  lw ra, 56(sp)
  lw t0, 112(sp)
  sw t0, 48(sp)

  lw t0, 48(sp)
  li t1, 1
  add t0, t1, t0
  sw t0, 44(sp)

  lw t0, 44(sp)
  sw t0, 112(sp)
  j while_1

end_while_1:
  li a0, 0
  addi sp, sp, 112
  ret
```