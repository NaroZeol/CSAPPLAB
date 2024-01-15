0000000000401aab <setval_350>:
<gadget1>->0x401aae
89 e0 90 c3
mov %esp, %eax
nop
ret
<gadget2>->0x401aad
48 89 e0 90 c3
mov %rsp, %rax
nop
ret

00000000004019ca <getval_280>:
<gadget3>->0x4019cc
58 90 c3
pop %rax
nop
ret

00000000004019a0 <addval_273>:
<gadget4>->0x4019a2
48 89 c7 c3
mov %rax, %rdi
ret

00000000004019db <getval_481>:
<gadget5>->0x4019dd
89 c2 90 c3
mov %eax, %edx
nop
ret

0000000000401a11 <addval_436>:
<gadget6>->0x401a13
89 ce 90 90 c3
mov %ecx, %esi
nop
nop
ret

0000000000401a25 <addval_187>:
<gadget7>->0x401a27
89 ce 38 c0 c3
mov %ecx, %esi
cmp %al, %al
ret

0000000000401a33 <getval_159>:
<gadget8>->0x401a34
89 d1 38 c9 c3
mov %edx, %ecx
cmp %cl, %cl
ret

0000000000401a68 <getval_311>:
<gadget9>->0x401a69
89 d1 08 db c3
mov %edx, %ecx
orb %bl, %bl
ret