ENTRY(kernel_entry)

KERNEL_PHYS_BASE = 0x00100000;
KERNEL_VIRT_BASE = 0xC0000000;

SECTIONS {

  . = KERNEL_PHYS_BASE;

  .text.boot ALIGN(4K) : {
    KEEP(*(.multiboot))
    *(.text.boot*)
  }

  . += KERNEL_VIRT_BASE;

  __kernel_start = .;

  .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VIRT_BASE) {
    *(.text*)
  }

  .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRT_BASE) {
    *(.rodata*)
  }

  .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRT_BASE) {
    *(.data*)
  }

  .init_array ALIGN(4K) : AT(ADDR(.init_array) - KERNEL_VIRT_BASE) {
    __init_array_start = .;
    KEEP(*(.init_array*))
    KEEP(*(.ctors*))
    __init_array_end = .;
  }

  .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRT_BASE) {
    *(.bss.bootstrap*)
    __bss_start = .;
    *(.bss*)
    *(COMMON)

    . = ALIGN(16);
    __stack_bottom = .;
    . += 0x4000;
    __stack_top = .;
    __bss_end = .;
  }

  __kernel_end = .;
}
