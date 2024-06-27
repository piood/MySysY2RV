  .text
  .globl main
main:
  addi sp, sp, -256
  li t0, 1
  sw t0, 0(sp)
  sw x0, 4(sp)
  li t0, 2
  lw t1, 0(sp)
  add t1, t0, t1
  sw t1, 0(sp)
  li t0, 3
  lw t1, 0(sp)
  add t1, t0, t1
  sw t1, 8(sp)
  li t0, 4
  lw t1, 8(sp)
  add t1, t0, t1
  sw t1, 8(sp)
  lw t0, 8(sp)
  lw t1, 0(sp)
  lw t2, 4(sp)
  add t2, t1, t2
  add t2, t0, t2
  sw t2, 4(sp)
  li t0, 5
  lw t1, 8(sp)
  add t1, t0, t1
  sw t1, 8(sp)
  li t0, 6
  lw t1, 8(sp)
  add t1, t0, t1
  sw t1, 12(sp)
  lw t0, 12(sp)
  lw t1, 0(sp)
  add t1, t0, t1
  sw t1, 0(sp)
  lw t0, 12(sp)
  lw t1, 8(sp)
  lw t2, 0(sp)
  lw t3, 4(sp)
  add t3, t2, t3
  add t3, t1, t3
  add t3, t0, t3
  sw t3, 4(sp)
  lw t0, 0(sp)
  lw t1, 8(sp)
  add t1, t0, t1
  sw t1, 8(sp)
  li t0, 7
  lw t1, 12(sp)
  add t1, t0, t1
  sw t1, 16(sp)
  li t0, 8
  lw t1, 16(sp)
  add t1, t0, t1
  sw t1, 16(sp)
  lw t0, 12(sp)
  lw t1, 8(sp)
  lw t2, 16(sp)
  lw t3, 4(sp)
  add t3, t2, t3
  add t3, t1, t3
  add t3, t0, t3
  sw t3, 4(sp)
  lw t0, 16(sp)
  lw t1, 8(sp)
  add t1, t0, t1
  sw t1, 8(sp)
  li t0, 9
  lw t1, 12(sp)
  add t1, t0, t1
  sw t1, 20(sp)
  li t0, 10
  lw t1, 16(sp)
  add t1, t0, t1
  sw t1, 16(sp)
  li t0, 12
  lw t1, 20(sp)
  add t1, t0, t1
  sw t1, 20(sp)
  lw t0, 12(sp)
  lw t1, 20(sp)
  li t2, 11
  lw t3, 4(sp)
  add t3, t2, t3
  add t3, t1, t3
  add t3, t0, t3
  sw t3, 4(sp)
  lw t0, 20(sp)
  lw t1, 12(sp)
  add t1, t0, t1
  sw t1, 12(sp)
  li t0, 13
  lw t1, 20(sp)
  add t1, t0, t1
  sw t1, 24(sp)
  li t0, 11
  lw t1, 24(sp)
  add t1, t0, t1
  sw t1, 24(sp)
  lw t0, 24(sp)
  lw t1, 20(sp)
  li t2, 11
  lw t3, 4(sp)
  add t3, t2, t3
  add t3, t1, t3
  add t3, t0, t3
  sw t3, 4(sp)
  lw t0, 12(sp)
  lw t1, 4(sp)
  sub t0, t1, t0
  sw t0, 4(sp)
  lw t0, 8(sp)
  lw t1, 4(sp)
  sub t0, t1, t0
  sw t0, 4(sp)
  lw t0, 0(sp)
  lw t1, 4(sp)
  sub t0, t1, t0
  sw t0, 4(sp)
  lw t0, 4(sp)
  li t1, 77
  rem t1, t0, t1
  mv a0, t1
  addi sp, sp, 256
  ret
