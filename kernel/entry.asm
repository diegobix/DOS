
section .multiboot
align 4
  dd 0x1BADB002
  dd 0
  dd -(0x1BADB002 + 0)

section .text
extern __stack_top
extern kernel_main
global kernel_entry
kernel_entry:
  mov esp, __stack_top

  push ebx
  call kernel_main

  cli
.halt:
  hlt
  jmp .halt

