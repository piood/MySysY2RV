  .text
  .globl main
main:
  li t0, 2
  li t1, 3
  mul t2, t0, t1
  li t3, 1
  add t4, t2, t3
  mv a0, t4
  ret
