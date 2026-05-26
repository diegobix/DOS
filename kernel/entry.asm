KERNEL_BASE equ 0xC0000000

%define PHYS_ADDR(x) ((x) - KERNEL_BASE)

PAGE_PRESENT   equ 0x1
PAGE_WRITABLE  equ 0x2
PAGE_SIZE      equ 0x80
PAGE_ENTRY_4MB equ PAGE_PRESENT | PAGE_WRITABLE | PAGE_SIZE ; 0x83

section .multiboot
align 4
  dd 0x1BADB002
  dd 0
  dd -(0x1BADB002 + 0)

section .text.boot
extern __stack_top
extern kernel_main
global kernel_entry
kernel_entry:
  mov esp, PHYS_ADDR(__stack_top)

  push ebx

setup_paging:
  mov edi, PHYS_ADDR(boot_page_directory)
  mov ecx, 1024
  xor eax, eax
  rep stosd

  ; Primera entrada 4MB de identity paging
  mov edi, PHYS_ADDR(boot_page_directory)
  mov dword [edi + 0 * 4], 0x00000000 | PAGE_ENTRY_4MB
  
  ; Entrada 768 high half a físico (xC0000000 / 4MB = 768)
  mov dword [edi + 768 * 4], 0x00000000 | PAGE_ENTRY_4MB

  ; Activar PSE (necesario para paginas de 4MB)
  mov eax, cr4
  or eax, 0x10 
  mov cr4, eax

  ; Cargar en cr3 la page directory 
  mov eax, PHYS_ADDR(boot_page_directory)
  mov cr3, eax

  ; Activar paginacion
  mov eax, cr0 
  or eax, 0x80000000 
  mov cr0, eax 

  jmp kernel_entry_high

section .text
extern __bss_start
extern __bss_end
kernel_entry_high:
  pop ebx
  mov esp, __stack_top

  ; Clean bss
  mov edi, __bss_start
  mov ecx, __bss_end
  sub ecx, edi
  xor eax, eax
  rep stosb

  push ebx
  call kernel_main

  cli
.halt:
  hlt
  jmp .halt

section .bss.bootstrap
align 4096
boot_page_directory:
  resd 1024
