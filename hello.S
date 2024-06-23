  .text
  .globl main
main:
  li t0, 2
  li t1, 3
  xor t1, t1, x0
  seqz t1, t1
  li t2, 4
  or t1, t2, t1
  xor t1, x0, t1
  snez t1, t1
  mul t1, t1, t0
  li t3, 1
  add t3, t1, t3
  li t4, 5
  slt t3, t3, t4
  li t5, 6
  xor t3, t5, t3
  snez t3, t3
  li t6, 7
  sub t6, x0, t6
  xor t3, x0, t3
  snez t3, t3
  xor t6, x0, t6
  snez t6, t6
  and t6, t3, t6
  mv a0, t6
  ret
