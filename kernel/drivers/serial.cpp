#include "serial.h"

#include "asm.h"
#include <stdarg.h>
#include <printf.h>

namespace uart
{

namespace
{
  constexpr u16 PORT = 0x3f8;

  int is_transmit_empty()
  {
    return ASM::inb(PORT + 5) & 0x20;
  }
}

int init()
{
   ASM::outb(PORT + 1, 0x00);    // Disable all interrupts
   ASM::outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   ASM::outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   ASM::outb(PORT + 1, 0x00);    //                  (hi byte)
   ASM::outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   ASM::outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   ASM::outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
   ASM::outb(PORT + 4, 0x1E);    // Set in loopback mode, test the serial chip
   ASM::outb(PORT + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

   // Check if serial is faulty (i.e: not same byte as sent)
   if (0xAE != ASM::inb(PORT + 0)) {
     return 1;
   }

   // If serial is not faulty set it in normal operation mode
   // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
   ASM::outb(PORT + 4, 0x0F);
   return 0;
}

void write_char(char c)
{
  while (is_transmit_empty() == 0);

  ASM::outb(PORT, c);
}

void printf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  kfmt::printf(write_char, fmt, args);
  va_end(args);
}

}
