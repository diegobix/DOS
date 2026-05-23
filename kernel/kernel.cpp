
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
  GDT::init();
  vga::init();
  uart::init();


  int edad = 28;

  uart::printf("Hola! Soy %s y tengo %d años, variable guardada en %p!\n", "Diego", edad, &edad);
  uart::printf("Hola! Hablando desde la uart!\n");

  while (true);
}
