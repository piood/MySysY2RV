  .text
  .globl half
half:
  mv t0, a0
  li t4, 0
  add t4, t4, sp
  sw t0, (t4)
  lw t0, 0(sp)
  li t1, 2
  div t1, t0, t1
  li t4, t1
  add t4, t4, sp
  lw a0, (t4)
  add t4, t4, 4
  lw ra, (t4)
  add sp, sp, t4
  ret

  .text
  .globl f
f:
  li a0, 0
  add t4, t4, sp
  lw ra, (t4)
  add sp, sp, t4
  ret

  .text
  .globl main
main:
  call f
  li t0, 10
  call half
  sw t0, 0(sp)
  li t4, 
  add t4, t4, sp
  lw a0, (t4)
  add t4, t4, 4
  lw ra, (t4)
  add sp, sp, t4
  ret

