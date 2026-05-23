#pragma once

#include "types.h"

namespace vga
{

  enum class Color : u8
  {
    Black         = 0,
    Blue          = 1,
    Green         = 2,
    Cyan          = 3,
    Red           = 4,
    Magenta       = 5,
    Brown         = 6,
    LightGrey     = 7,
    DarkGrey      = 8,
    LightBlue     = 9,
    LightGreen    = 10,
    LightCyan     = 11,
    LightRed      = 12,
    LightMagenta  = 13,
    Yellow        = 14,
    White         = 15,
  };

  void init();
  void clear();
  void set_color(Color fg, Color bg);

  void putchar(char c);
  void print(const char *str);
  void printf(const char *fmt, ...);
  void println(const char *fmt, ...);
  void print_hex(u32 n);
  void print_int(i32 n);
  void newline();

} // namespace vga 
