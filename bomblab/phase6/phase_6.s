#地址             值(四字节，转为10进制) 序号(4字节)      next(8字节)
0x6032d0 <node1>:       332            0x00000001      0x00000000006032e0 
0x6032e0 <node2>:       168            0x00000002      0x00000000006032f0 
0x6032f0 <node3>:       924            0x00000003      0x0000000000603300 
0x603300 <node4>:       691            0x00000004      0x0000000000603310 
0x603310 <node5>:       477            0x00000005      0x0000000000603320 
0x603320 <node6>:       443            0x00000006      0x0000000000000000 

要求的顺序 3 4 5 6 1 2
实际输入字符串 4 3 2 1 6 5

phase_6:
   0x00000000004010f4 <+0>:     push   %r14
   0x00000000004010f6 <+2>:     push   %r13
   0x00000000004010f8 <+4>:     push   %r12
   0x00000000004010fa <+6>:     push   %rbp
   0x00000000004010fb <+7>:     push   %rbx
   0x00000000004010fc <+8>:     sub    $0x50,%rsp #开辟80字节的栈空间，看来不怎么简单
   0x0000000000401100 <+12>:    mov    %rsp,%r13 #可以认为%rsp开头存在一个int a[6]数组，这里暂存这个数组
   0x0000000000401103 <+15>:    mov    %rsp,%rsi #将数组a的首地址放入%rsi，作为read_six_numbers的输出
   0x0000000000401106 <+18>:    call   0x40145c <read_six_numbers> 
   0x000000000040110b <+23>:    mov    %rsp,%r14 #暂存%rsp，现在%rsp为首地址的数组存放着我们输入的六个数字
   0x000000000040110e <+26>:    mov    $0x0,%r12d #%r12d置0，设其保存变量i

   0x0000000000401114 <+32>:    mov    %r13,%rbp #根据<+89>的指令，可以认为%r13指向的元素在变化，设其指向a[x]，初始x为0
   0x0000000000401117 <+35>:    mov    0x0(%r13),%eax  
   0x000000000040111b <+39>:    sub    $0x1,%eax
   0x000000000040111e <+42>:    cmp    $0x5,%eax 
   0x0000000000401121 <+45>:    jbe    0x401128 <phase_6+52> #jump if below or equal
   0x0000000000401123 <+47>:    call   0x40143a <explode_bomb> # a[x]大于6则爆炸，同时小于等于0也会 
   0x0000000000401128 <+52>:    add    $0x1,%r12d # i += 1
   0x000000000040112c <+56>:    cmp    $0x6,%r12d
   0x0000000000401130 <+60>:    je     0x401153 <phase_6+95> #如果i等于6则退出循环
   0x0000000000401132 <+62>:    mov    %r12d,%ebx #设%ebx保存变量 j
      0x0000000000401135 <+65>:    movslq %ebx,%rax 
      0x0000000000401138 <+68>:    mov    (%rsp,%rax,4),%eax #4 * j + %rsp,即%eax = a[j]
      0x000000000040113b <+71>:    cmp    %eax,0x0(%rbp) #比较a[j]与a[x] 
      0x000000000040113e <+74>:    jne    0x401145 <phase_6+81>
      0x0000000000401140 <+76>:    call   0x40143a <explode_bomb> #如果a[j] == a[x]则爆炸
      0x0000000000401145 <+81>:    add    $0x1,%ebx
      0x0000000000401148 <+84>:    cmp    $0x5,%ebx
      0x000000000040114b <+87>:    jle    0x401135 <phase_6+65>
   0x000000000040114d <+89>:    add    $0x4,%r13 #%r13，指向数组a的指针移动一个单位，从指向a[x]变为指向a[x + 1]
   0x0000000000401151 <+93>:    jmp    0x401114 <phase_6+32>
   #只要满足输入序列每个数都小于等于6，并且两两不同就触发爆炸
   
   #初始化
   0x0000000000401153 <+95>:    lea    0x18(%rsp),%rsi #将%rsp + 24作为某个数组的首地址，不妨设为b
   0x0000000000401158 <+100>:   mov    %r14,%rax #可以认为%rax指向为a[i]，初始化i为0
   0x000000000040115b <+103>:   mov    $0x7,%ecx #%ecx 置为7

   0x0000000000401160 <+108>:   mov    %ecx,%edx
   0x0000000000401162 <+110>:   sub    (%rax),%edx # %edx = 7 - a[i]（因为a[i]小于等于0，则一定为正）
   0x0000000000401164 <+112>:   mov    %edx,(%rax) # a[i] = 7 - a[i]
   0x0000000000401166 <+114>:   add    $0x4,%rax # i += 1
   0x000000000040116a <+118>:   cmp    %rsi,%rax #将%rsi(此时为%rsp + 24)作为遍历数组a的结束
   0x000000000040116d <+121>:   jne    0x401160 <phase_6+108>
   #以上循环将导致a[i]变为7 - a[i]

   0x000000000040116f <+123>:   mov    $0x0,%esi #初始化%esi为0，不妨假设%rsi处存放变量为n
   0x0000000000401174 <+128>:   jmp    0x401197 <phase_6+163>

   0x0000000000401176 <+130>:   mov    0x8(%rdx),%rdx #8正好是一个链表节点的next指针的偏移量，读取到next指针后赋给%rdx，类似于操作curr = curr->next
   0x000000000040117a <+134>:   add    $0x1,%eax
   0x000000000040117d <+137>:   cmp    %ecx,%eax
   0x000000000040117f <+139>:   jne    0x401176 <phase_6+130> #算上开始的第一次，最后会在node(a[n])停下
   0x0000000000401181 <+141>:   jmp    0x401188 <phase_6+148> #当执行到这里时，%rdx将会是node(a[n])的值

   0x0000000000401183 <+143>:   mov    $0x6032d0,%edx #将0x6032d0的值放入%edx，这是一堆节点的首地址
   0x0000000000401188 <+148>:   mov    %rdx,0x20(%rsp,%rsi,2) #设%rsp + 32是集合b的开始地址，将node(a[n])放置于b[2n]
   0x000000000040118d <+153>:   add    $0x4,%rsi #%rsi只在此处增加，也就是说想要退出循环，要让这个指令运行6次。
   0x0000000000401191 <+157>:   cmp    $0x18,%rsi
   0x0000000000401195 <+161>:   je     0x4011ab <phase_6+183>
   0x0000000000401197 <+163>:   mov    (%rsp,%rsi,1),%ecx #%ecx = a[n]
   0x000000000040119a <+166>:   cmp    $0x1,%ecx 
   0x000000000040119d <+169>:   jle    0x401183 <phase_6+143> #如果a[n] 小于等于1，则跳回<+143>

   0x000000000040119f <+171>:   mov    $0x1,%eax
   0x00000000004011a4 <+176>:   mov    $0x6032d0,%edx
   0x00000000004011a9 <+181>:   jmp    0x401176 <phase_6+130>
   #至此，数组b中应该有6个有效的链表节点的地址

   0x00000000004011ab <+183>:   mov    0x20(%rsp),%rbx #将b的第一个元素放置于%rbx，这就是我们第一个获取到的node的地址
   0x00000000004011b0 <+188>:   lea    0x28(%rsp),%rax #一个快指针，它始终在%rcx的后一个位置
   0x00000000004011b5 <+193>:   lea    0x50(%rsp),%rsi #遍历数组b的终点
   0x00000000004011ba <+198>:   mov    %rbx,%rcx #将第一个获取到的node的地址放置于%rcx，用%rcx进行操作，称其为curr

   0x00000000004011bd <+201>:   mov    (%rax),%rdx #将快指针所指向内存中的节点地址暂存于%rdx
   0x00000000004011c0 <+204>:   mov    %rdx,0x8(%rcx) #curr->next = %rdx(即将链表按b节点中存储的顺序重新连接)
   0x00000000004011c4 <+208>:   add    $0x8,%rax
   0x00000000004011c8 <+212>:   cmp    %rsi,%rax
   0x00000000004011cb <+215>:   je     0x4011d2 <phase_6+222>
   0x00000000004011cd <+217>:   mov    %rdx,%rcx #curr = curr->next
   0x00000000004011d0 <+220>:   jmp    0x4011bd <phase_6+201>
   0x00000000004011d2 <+222>:   movq   $0x0,0x8(%rdx) #curr->next->next = null

   0x00000000004011da <+230>:   mov    $0x5,%ebp #循环5次的意思
   0x00000000004011df <+235>:   mov    0x8(%rbx),%rax
   0x00000000004011e3 <+239>:   mov    (%rax),%eax
   0x00000000004011e5 <+241>:   cmp    %eax,(%rbx)
   0x00000000004011e7 <+243>:   jge    0x4011ee <phase_6+250> #这里我们总算明白最后的目标是什么了，我们要构建一个降序链表
   0x00000000004011e9 <+245>:   call   0x40143a <explode_bomb>
   0x00000000004011ee <+250>:   mov    0x8(%rbx),%rbx
   0x00000000004011f2 <+254>:   sub    $0x1,%ebp
   0x00000000004011f5 <+257>:   jne    0x4011df <phase_6+235>

   0x00000000004011f7 <+259>:   add    $0x50,%rsp
   0x00000000004011fb <+263>:   pop    %rbx
   0x00000000004011fc <+264>:   pop    %rbp
   0x00000000004011fd <+265>:   pop    %r12
   0x00000000004011ff <+267>:   pop    %r13
   0x0000000000401201 <+269>:   pop    %r14
   0x0000000000401203 <+271>:   ret    