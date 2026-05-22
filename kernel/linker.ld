ENTRY(kernel_entry)

SECTIONS {
  . = 0x00100000;

  .text : {
    KEEP(*(.multiboot))
    *(.text*)
  }

  .rodata : {
    *(.rodata*)
  }

  .data : {
    *(.data*)
  }

  .bss : {
    __bss_start = .;
    *(.bss*)
    *(COMMON)
    __bss_end = .;
  }

  . = ALIGN(16);
  __stack_bottom = .;
  . += 0x4000;
  __stack_top = .;
}
