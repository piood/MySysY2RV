  .text
  .globl main
main:
  addi sp, sp, -256
  li t0, 21
  sw t0, 4(sp)
  li t0, 1
  lw t1, 4(sp)
  add t1, t0, t1
  sw t1, 0(sp)
  li t1, 147
  lw t2, 0(sp)
  sub t2, t1, t2
  sw t2, 8(sp)
  lw t2, 4(sp)
  sub t2, t1, t2
  sw t2, 12(sp)
  lw t2, 12(sp)
  li t3, 1
  mul t3, t3, t2
  li t4, 1
  mul t4, t4, t3
  li t5, 1
  mul t5, t5, t4
  sw t5, 12(sp)
  lw t5, 12(sp)
  lw t6, 8(sp)
  add t6, t5, t6
  mv a0, t6
  addi sp, sp, 256
  ret
