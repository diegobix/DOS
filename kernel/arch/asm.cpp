#include "asm.h"

namespace ASM
{

void outb(u16 port, u8 data)
{
  __asm__("out dx, al" : : "a" (data), "d" (port));
}

void outw(u16 port, u16 data)
{
  __asm__("out dx, ax" : : "a" (data), "d" (port));
}

u8 inb(u16 port)
{
  u8 recv;
  __asm__("in al, dx" : "=a" (recv) : "d" (port));
  return recv;
}

u16 inw(u16 port)
{
  u16 recv;
  __asm__("in ax, dx" : "=a" (recv) : "d" (port));
  return recv;
}

}
