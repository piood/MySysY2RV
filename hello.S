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

