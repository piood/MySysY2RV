  .text
  .globl main
main:
  addi sp, sp, -256
  li t0, 1
  sw t0, 0(sp)
  lw t0, 0(sp)
  li t1, 1
  sgt t0, t0, t1
  bnez t0, then_1
  j else_1

then_1:
  j while_1

else_1:
  j while_2

while_1:
  li t1, 1
  bnez t1, while_then_1
  j end_while_1

while_2:
  lw t2, 0(sp)
  li t3, 10
  slt t2, t2, t3
  bnez t2, while_then_2
  j end_while_2

while_then_1:
  j while_1

end_while_1:
  j end_1

while_then_2:
  lw t3, 0(sp)
  mv a0, t3
  addi sp, sp, 256
  ret

end_while_2:
  j end_1

end_1:
  lw t4, 0(sp)
  mv a0, t4
  addi sp, sp, 256
  ret

