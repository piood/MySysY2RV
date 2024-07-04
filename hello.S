  .text
  .globl main
main:
  addi sp, sp, -256
  li t0, 1
  sw t0, 0(sp)
  sw x0, 4(sp)
  lw t0, 0(sp)
  li t1, 2
  add t0, t1, t0
  sw t0, 0(sp)
  lw t0, 0(sp)
  li t1, 3
  add t0, t1, t0
  sw t0, 8(sp)
  lw t0, 8(sp)
  li t1, 4
  add t0, t1, t0
  sw t0, 8(sp)
  lw t0, 4(sp)
  lw t1, 0(sp)
  add t0, t1, t0
  lw t1, 8(sp)
  add t0, t1, t0
  sw t0, 4(sp)
  lw t0, 8(sp)
  li t1, 5
  add t0, t1, t0
  sw t0, 8(sp)
  lw t0, 8(sp)
  li t1, 6
  add t0, t1, t0
  sw t0, 12(sp)
  lw t0, 0(sp)
  lw t1, 12(sp)
  add t0, t1, t0
  sw t0, 0(sp)
  lw t0, 4(sp)
  lw t1, 0(sp)
  add t0, t1, t0
  lw t1, 8(sp)
  add t0, t1, t0
  lw t1, 12(sp)
  add t0, t1, t0
  sw t0, 4(sp)
  lw t0, 8(sp)
  lw t1, 0(sp)
  add t0, t1, t0
  sw t0, 8(sp)
  lw t0, 12(sp)
  li t1, 7
  add t0, t1, t0
  sw t0, 16(sp)
  lw t0, 16(sp)
  li t1, 8
  add t0, t1, t0
  sw t0, 16(sp)
  lw t0, 4(sp)
  lw t1, 16(sp)
  add t0, t1, t0
  lw t1, 8(sp)
  add t0, t1, t0
  lw t1, 12(sp)
  add t0, t1, t0
  sw t0, 4(sp)
  lw t0, 8(sp)
  lw t1, 16(sp)
  add t0, t1, t0
  sw t0, 8(sp)
  lw t0, 12(sp)
  li t1, 9
  add t0, t1, t0
  sw t0, 20(sp)
  lw t0, 16(sp)
  li t1, 10
  add t0, t1, t0
  sw t0, 16(sp)
  lw t0, 20(sp)
  li t1, 12
  add t0, t1, t0
  sw t0, 20(sp)
  lw t0, 4(sp)
  li t1, 11
  add t0, t1, t0
  lw t1, 20(sp)
  add t0, t1, t0
  lw t1, 12(sp)
  add t0, t1, t0
  sw t0, 4(sp)
  lw t0, 12(sp)
  lw t1, 20(sp)
  add t0, t1, t0
  sw t0, 12(sp)
  lw t0, 20(sp)
  li t1, 13
  add t0, t1, t0
  sw t0, 24(sp)
  lw t0, 24(sp)
  li t1, 11
  add t0, t1, t0
  sw t0, 24(sp)
  lw t0, 4(sp)
  li t1, 11
  add t0, t1, t0
  lw t1, 20(sp)
  add t0, t1, t0
  lw t1, 24(sp)
  add t0, t1, t0
  sw t0, 4(sp)
  lw t0, 4(sp)
  lw t1, 12(sp)
  sub t1, t0, t1
  sw t1, 4(sp)
  lw t0, 4(sp)
  lw t1, 8(sp)
  sub t1, t0, t1
  sw t1, 4(sp)
  lw t0, 4(sp)
  lw t1, 0(sp)
  sub t1, t0, t1
  sw t1, 4(sp)
  lw t0, 4(sp)
  li t1, 77
  rem t1, t0, t1
  mv a0, t1
  addi sp, sp, 256
  ret

