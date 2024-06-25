  .text
  .globl main
main:
  li t0, 1
  li t1, 4
  sub t0, t1, t0
  mv a0, t0
  ret
