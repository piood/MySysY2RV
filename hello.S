  .text
  .globl main
main:
  addi sp, sp, -256
  li t0, 1
  li t1, 2
  li t2, 3
  xor t1, x0, t1
  snez t1, t1
  xor t2, x0, t2
  snez t2, t2
  and t2, t1, t2
  or t0, t2, t0
  xor t0, x0, t0
  snez t0, t0
  sw t0, 0(sp)
  li t0, 1
  xor t1, x0, x0
  snez t1, t1
  xor t0, x0, t0
  snez t0, t0
  and t0, t1, t0
  or t0, x0, t0
  xor t0, x0, t0
  snez t0, t0
  sw t0, 4(sp)
  li t0, 1
  xor t0, x0, t0
  snez t0, t0
  xor t1, x0, x0
  snez t1, t1
  and t1, t0, t1
  li t0, 1
  or t1, t0, t1
  xor t1, x0, t1
  snez t1, t1
  li t0, 4
  mul t0, t0, t1
  sw t0, 8(sp)
  li t0, 5
  sw t0, 12(sp)
  lw t0, 0(sp)
  li t1, 1
  xor t0, t1, t0
  seqz t0, t0
  lw t1, 0(sp)
  li t2, 2
  xor t1, t2, t1
  seqz t1, t1
  or t0, t1, t0
  xor t0, x0, t0
  snez t0, t0
  bnez t0, then_1
  j end_1

then_1:
  j end_1

end_1:
  lw t1, 4(sp)
  xor t1, x0, t1
  seqz t1, t1
  lw t2, 4(sp)
  li t3, 1
  xor t2, t3, t2
  seqz t2, t2
  or t1, t2, t1
  xor t1, x0, t1
  snez t1, t1
  bnez t1, then_2
  j else_2

then_2:
  li t2, 1
  lw t3, 12(sp)
  add t3, t2, t3
  sw t3, 12(sp)
  j end_2

else_2:
  j end_2

end_2:
  lw t2, 0(sp)
  lw t3, 4(sp)
  xor t2, x0, t2
  snez t2, t2
  xor t3, x0, t3
  snez t3, t3
  and t3, t2, t3
  lw t2, 8(sp)
  lw t4, 12(sp)
  xor t2, x0, t2
  snez t2, t2
  xor t4, x0, t4
  snez t4, t4
  and t4, t2, t4
  or t3, t4, t3
  xor t3, x0, t3
  snez t3, t3
  bnez t3, then_3
  j end_3

then_3:
  li t2, 1
  lw t4, 12(sp)
  add t4, t2, t4
  sw t4, 12(sp)
  j end_3

end_3:
  lw t2, 12(sp)
  lw t4, 8(sp)
  xor t4, t4, x0
  seqz t4, t4
  or t2, t4, t2
  xor t2, x0, t2
  snez t2, t2
  mv a0, t2
  addi sp, sp, 256
  ret

