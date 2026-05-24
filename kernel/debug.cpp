#include "debug.h"

#include "drivers/serial.h"
#include "drivers/vga.h"
#include "stdarg.h"
#include <printf.h>

namespace DEBUG
{

namespace
{

void debug_putchar(char c)
{
  vga::putchar(c);
  uart::write_char(c);
}

}

void log(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  kfmt::printf(debug_putchar, fmt, args);
  va_end(args);
}

} // namespace DEBUG 
