
#include "arch/idt.h"
#include "debug.h"
#include "memory/paging.h"
#include "memory/phys.h"
#include "multiboot.h"
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

extern "C" void kernel_main(multiboot::MultibootInfo *multiboot_info)
{
  vga::init();
  uart::init();

  GDT::init();
  DEBUG::log("GDT OK\n");
  IDT::init();
  DEBUG::log("IDT OK\n");


  int edad = 28;

  vga::printf("Hello World!\nKernel DOS-DOS by Diego Arenas\n");

  DEBUG::log("multiboot ptr: %p", multiboot_info);

  DEBUG::log("MutlibootInfo flags: %x", multiboot_info->flags);

  multiboot::print_mmap(multiboot_info);

  extern u32 __kernel_end;
  u32 kernel_end = reinterpret_cast<u32>(&__kernel_end);
  DEBUG::log("kernel_end: %x\n", kernel_end);
  memory::phys::init(multiboot_info, kernel_end);


  paging::init(kernel_end);

  auto ptr = reinterpret_cast<u32*>(0x1000);
  paging::map_page(0x1000, memory::phys::alloc_frame(), paging::PRESENT | paging::RW);
  *ptr = 3;
  DEBUG::log("%d", *ptr);

  while (true);
}
