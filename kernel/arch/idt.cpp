#include "idt.h"

#include "types.h"
#include "debug.h"

namespace IDT
{

namespace
{

struct [[gnu::packed]] Descriptor
{
  u16 offset_low;
  u16 selector;
  u8 reserved;
  u8 type;
  u16 offset_high;
};

struct [[gnu::packed]] Register
{
  u16 limit;
  u32 base;
};

struct [[gnu::packed]] InterruptFrame
{
  // pushad 
  u32 edi, esi, ebp, esp;
  u32 ebx, edx, ecx, eax;

  u32 interrupt_n; // Numero de interrupcion
  u32 err_code;

  // Pushed por cpu 
  u32 eip;
  u32 cs;
  u32 eflags;
};

constexpr u32 IDT_SIZE = 20;
Descriptor table[IDT_SIZE];
Register idtr;

extern "C"
{
    void isr_stub_0();  void isr_stub_1();  void isr_stub_2();
    void isr_stub_3();  void isr_stub_4();  void isr_stub_5();
    void isr_stub_6();  void isr_stub_7();  void isr_stub_8();
    void isr_stub_9();  void isr_stub_10(); void isr_stub_11();
    void isr_stub_12(); void isr_stub_13(); void isr_stub_14();
    void isr_stub_15(); void isr_stub_16(); void isr_stub_17();
    void isr_stub_18(); void isr_stub_19();
}

using IsrStub = void(*)();
IsrStub stubs[IDT_SIZE] = {
    isr_stub_0,  isr_stub_1,  isr_stub_2,  isr_stub_3,  
    isr_stub_4,  isr_stub_5,  isr_stub_6,  isr_stub_7,
    isr_stub_8,  isr_stub_9,  isr_stub_10, isr_stub_11,
    isr_stub_12, isr_stub_13, isr_stub_14, isr_stub_15,
    isr_stub_16, isr_stub_17, isr_stub_18, isr_stub_19,
};

Descriptor make_descriptor(void *handler)
{
  u32 offset = reinterpret_cast<u32>(handler);
  return Descriptor {
    .offset_low = static_cast<u16>(offset & 0xFFFF),
    .selector = 0x08,
    .reserved = 0,
    .type = 0x8E, // 32 bit interrupt gate + present bit
    .offset_high = static_cast<u16>((offset >> 16) & 0xFFFF)
  };
}

constexpr const char *names[] = {
  "Division by zero",      "Debug",
  "NMI",                   "Breakpoint",
  "Overflow",              "Bound range",
  "Invalid opcode",        "Device not available",
  "Double fault",          "Coprocessor overrun",
  "Invalid TSS",           "Segment not present",
  "Stack fault",           "General protection fault",
  "Page fault",            "Reserved",
  "x87 FPU fault",         "Alignment check",
  "Machine check",         "SIMD fault",
};

const char *exception_name(u32 int_number) {
  if (int_number < IDT_SIZE) return names[int_number];
  else return "Unknown";
}

} // namespace anon 

} // namespace IDT 

extern "C" void isr_handler(IDT::InterruptFrame *frame)
{
  DEBUG::log("---EXCEPTION---\n");
  DEBUG::log("Interrupt: %u (%s)\n", frame->interrupt_n, IDT::exception_name(frame->interrupt_n));
  DEBUG::log("Error    : %x\n", frame->err_code);
  DEBUG::log("EIP      : %x\n", frame->eip);
  DEBUG::log("CS       : %x\n", frame->cs);
  DEBUG::log("EFLAGS   : %x\n", frame->eflags);

  if (frame->interrupt_n == 14)
  {
    // Page fault
    u32 cr2; // El registro cr2 tiene la dirección del fallo
    asm volatile("mov %0, cr2" : "=r" (cr2) : );
    DEBUG::log("CR2      : %x\n", cr2);
  }

  DEBUG::log("\n Sistema detenido por excepcion\n");
  asm volatile("cli; hlt");
}

namespace IDT
{

void init()
{
  for (u32 i = 0; i < IDT_SIZE; i++)
  {
    table[i] = make_descriptor(reinterpret_cast<void *>(stubs[i]));
  }

  idtr.limit = sizeof(table) - 1;
  idtr.base = reinterpret_cast<u32>(&table);
  asm volatile("lidt %0" : : "m" (idtr));
}

}
