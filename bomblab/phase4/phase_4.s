0x4025cf:       "%d %d"

phase_4:
   0x000000000040100c <+0>:     sub    $0x18,%rsp
   0x0000000000401010 <+4>:     lea    0xc(%rsp),%rcx #sscanf的第二个输出参数，设其为b
   0x0000000000401015 <+9>:     lea    0x8(%rsp),%rdx #sscanf的第一个输出参数，设其为a
   0x000000000040101a <+14>:    mov    $0x4025cf,%esi
   0x000000000040101f <+19>:    mov    $0x0,%eax
   0x0000000000401024 <+24>:    call   0x400bf0 <__isoc99_sscanf@plt> #sscanf($输入字符串$, "%d %d", &a, &b)
   0x0000000000401029 <+29>:    cmp    $0x2,%eax
   0x000000000040102c <+32>:    jne    0x401035 <phase_4+41> #如果sscanf的返回值不为2，则爆炸
   0x000000000040102e <+34>:    cmpl   $0xe,0x8(%rsp) #如果a不小于等于14，则爆炸
   0x0000000000401033 <+39>:    jbe    0x40103a <phase_4+46> #jump if below or equal
   0x0000000000401035 <+41>:    call   0x40143a <explode_bomb>
   0x000000000040103a <+46>:    mov    $0xe,%edx   # %edx的初始值为14
   0x000000000040103f <+51>:    mov    $0x0,%esi   # %esi的初始值为0
   0x0000000000401044 <+56>:    mov    0x8(%rsp),%edi # a的值存放于%rdi
   0x0000000000401048 <+60>:    call   0x400fce <func4>
   0x000000000040104d <+65>:    test   %eax,%eax   # 如果func4的返回值不为0，则爆炸，也就是说我们希望func4的返回值为0
   0x000000000040104f <+67>:    jne    0x401058 <phase_4+76>
   0x0000000000401051 <+69>:    cmpl   $0x0,0xc(%rsp)
   0x0000000000401056 <+74>:    je     0x40105d <phase_4+81>
   0x0000000000401058 <+76>:    call   0x40143a <explode_bomb>
   0x000000000040105d <+81>:    add    $0x18,%rsp
   0x0000000000401061 <+85>:    ret    

#递归函数，输入为%edi，%esi，%edx，输出为%eax
#初始化时%esi为0，%edx为14，%edi为a
func4:
   0x0000000000400fce <+0>:     sub    $0x8,%rsp #没有压栈的操作。
   0x0000000000400fd2 <+4>:     mov    %edx,%eax
   0x0000000000400fd4 <+6>:     sub    %esi,%eax # %eax = %edx - %esi
   0x0000000000400fd6 <+8>:     mov    %eax,%ecx  # %ecx = %eax
   0x0000000000400fd8 <+10>:    shr    $0x1f,%ecx # %ecx = (%edx - %esi) >> 31， 这种操作一般用于获得符号位
   0x0000000000400fdb <+13>:    add    %ecx,%eax # %eax = %eax + %ecx(因为上一步是逻辑右移，%ecx为0或1)
   0x0000000000400fdd <+15>:    sar    %eax #当sar的操作数为1时，默认为算数右移一位，即除以2
   0x0000000000400fdf <+17>:    lea    (%rax,%rsi,1),%ecx # %ecx = %rax + %rsi = %eax + %esi
   # 如果设%esi为x，%edx为y，则以上操作可总结为
   # %eax = ((y - x) + (y - x < 0)) / 2 
   # %ecx = (y + (y - x < 0)) / 2
   0x0000000000400fe2 <+20>:    cmp    %edi,%ecx
   0x0000000000400fe4 <+22>:    jle    0x400ff2 <func4+36> #如果%ecx小于等于a，则跳转到0x400ff2
   0x0000000000400fe6 <+24>:    lea    -0x1(%rcx),%edx  # %edx = %ecx - 1
   0x0000000000400fe9 <+27>:    call   0x400fce <func4> #递归调用
   0x0000000000400fee <+32>:    add    %eax,%eax # %eax = 2 * %eax
   0x0000000000400ff0 <+34>:    jmp    0x401007 <func4+57> #跳转到0x401007，返回值为2 * %eax

   0x0000000000400ff2 <+36>:    mov    $0x0,%eax
   0x0000000000400ff7 <+41>:    cmp    %edi,%ecx           #可以发现只要%ecx等于a，就会返回0
   0x0000000000400ff9 <+43>:    jge    0x401007 <func4+57> # 如果%ecx大于等于a，则跳转到0x401007，返回值为0
   0x0000000000400ffb <+45>:    lea    0x1(%rcx),%esi      # %esi = %ecx + 1
   0x0000000000400ffe <+48>:    call   0x400fce <func4> #递归调用
   0x0000000000401003 <+53>:    lea    0x1(%rax,%rax,1),%eax # %eax = 2 * %eax + 1
   0x0000000000401007 <+57>:    add    $0x8,%rsp
   0x000000000040100b <+61>:    ret    