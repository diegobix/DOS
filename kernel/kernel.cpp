
#include "arch/idt.h"
#include "debug.h"
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

extern "C" void kernel_main(void *multiboot_info)
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

  u32 *ptr = reinterpret_cast<u32 *>(0xDEAD1234);
  u32 val = *ptr;
  DEBUG::log("Val = %d\n", val);

  while (true);
}
