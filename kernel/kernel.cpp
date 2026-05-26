
#include "arch/idt.h"
#include "debug.h"
#include "drivers/multiboot.h"
#include "drivers/serial.h"
#include "drivers/vga.h"
#include "arch/gdt.h"

constexpr int BOOTINFO_MAGIC = 0xB007B007;

class diego {
  public:
    int edad = 0;
    diego(int edad) : edad(edad) {};
};

diego yo(28);

extern "C" void kernel_main(multiboot::MutlibootInfo *multiboot_info)
{
  vga::init();
  uart::init();

  GDT::init();
  DEBUG::log("GDT OK\n");
  IDT::init();
  DEBUG::log("IDT OK\n");


  int edad = 28;

  uart::printf("Hola! Soy %s y tengo %d años, variable guardada en %p!\n", "Diego", edad, &edad);
  uart::printf("Hola! Hablando desde la uart!\n");

  vga::printf("Hello World!\nKernel DOS-DOS by Diego Arenas\n");

  DEBUG::log("multiboot ptr: %p", multiboot_info);

  DEBUG::log("MutlibootInfo flags: %x", multiboot_info->flags);

  while (true);
}
