0x4024b0:  "maduiersnfotvbylSo you think you can stop the bomb with ctrl-c, do you?"
0x4024f8:       "Curses, you've found the secret phase!"
0x40251f:       ""
0x402520:       "But finding it and solving it are quite different..."

0x40245e:       "flyers" #要求活字印刷这个字符串

phase_5:
   0x0000000000401062 <+0>:     push   %rbx
   0x0000000000401063 <+1>:     sub    $0x20,%rsp
   0x0000000000401067 <+5>:     mov    %rdi,%rbx #%rbx中暂存输入字符串的首地址
   0x000000000040106a <+8>:     mov    %fs:0x28,%rax
   0x0000000000401073 <+17>:    mov    %rax,0x18(%rsp) #%rsp+24处放置金丝雀值
   0x0000000000401078 <+22>:    xor    %eax,%eax
   0x000000000040107a <+24>:    call   0x40131b <string_length>
   0x000000000040107f <+29>:    cmp    $0x6,%eax #输入要求长度为6
   0x0000000000401082 <+32>:    je     0x4010d2 <phase_5+112>
   0x0000000000401084 <+34>:    call   0x40143a <explode_bomb>
   0x0000000000401089 <+39>:    jmp    0x4010d2 <phase_5+112>
   0x000000000040108b <+41>:    movzbl (%rbx,%rax,1),%ecx #设%rax中参数为i，%rbx中参数为输入字符串str，则%ecx = str[i]
   0x000000000040108f <+45>:    mov    %cl,(%rsp) #%cl是%ecx的低字节，此处解析为char
   0x0000000000401092 <+48>:    mov    (%rsp),%rdx #将str[i]放入栈顶（没有移动栈指针）与%rdx
   0x0000000000401096 <+52>:    and    $0xf,%edx #%edx保留str[i]的低四位，如果按数字解析则为一个0~15的数
   0x0000000000401099 <+55>:    movzbl 0x4024b0(%rdx),%edx #获取0x4024b0+%edx处的字符，放置于%edx中
   0x00000000004010a0 <+62>:    mov    %dl,0x10(%rsp,%rax,1) #将%edx中的字符存放于targetStr[i]中，targetStr即%rsp+16为开头的字符串
   0x00000000004010a4 <+66>:    add    $0x1,%rax
   0x00000000004010a8 <+70>:    cmp    $0x6,%rax #执行6次
   0x00000000004010ac <+74>:    jne    0x40108b <phase_5+41>
   0x00000000004010ae <+76>:    movb   $0x0,0x16(%rsp)
   0x00000000004010b3 <+81>:    mov    $0x40245e,%esi
   0x00000000004010b8 <+86>:    lea    0x10(%rsp),%rdi
   0x00000000004010bd <+91>:    call   0x401338 <strings_not_equal>
   0x00000000004010c2 <+96>:    test   %eax,%eax
   0x00000000004010c4 <+98>:    je     0x4010d9 <phase_5+119>
   0x00000000004010c6 <+100>:   call   0x40143a <explode_bomb>
   0x00000000004010cb <+105>:   nopl   0x0(%rax,%rax,1)
   0x00000000004010d0 <+110>:   jmp    0x4010d9 <phase_5+119>
   0x00000000004010d2 <+112>:   mov    $0x0,%eax
   0x00000000004010d7 <+117>:   jmp    0x40108b <phase_5+41>
   0x00000000004010d9 <+119>:   mov    0x18(%rsp),%rax
   0x00000000004010de <+124>:   xor    %fs:0x28,%rax
   0x00000000004010e7 <+133>:   je     0x4010ee <phase_5+140>
   0x00000000004010e9 <+135>:   call   0x400b30 <__stack_chk_fail@plt>
   0x00000000004010ee <+140>:   add    $0x20,%rsp
   0x00000000004010f2 <+144>:   pop    %rbx
   0x00000000004010f3 <+145>:   ret    