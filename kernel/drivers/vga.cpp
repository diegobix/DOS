#include "vga.h"
#include "types.h"
#include <printf.h>
#include <stdarg.h>

namespace vga
{

namespace
{

constexpr u32 BUFF_ADDR = 0xb8000;
constexpr u32 ROWS = 25;
constexpr u32 COLS = 80;

volatile u16 *buffer = reinterpret_cast<u16 *>(BUFF_ADDR);
u32 cur_row = 0;
u32 cur_col = 0;
u8 cur_color = 0;

u8 make_color(Color fg, Color bg)
{
  return static_cast<u8>(fg) | (static_cast<u8>(bg) << 4);  
}

u16 make_entry(char c, u8 color)
{
  return static_cast<u16>(c) | (static_cast<u16>(color) << 8);
}

void *memset(void *src, int content, u32 num)
{
  auto p = static_cast<u8*>(src);
  auto val = static_cast<u8>(content);

  for (u32 i = 0; i < num; i++)
  {
    p[i] = val;
  }

  return src;
}

void scroll()
{
  for (u32 row = 0; row < ROWS - 1; row++)
    for (u32 col = 0; col < COLS; col++)
      buffer[row * COLS + col] = buffer[(row + 1) * COLS + col];

  for (u32 col = 0; col < COLS; col++)
    buffer[(ROWS - 1) * COLS + col] = make_entry(' ', cur_color);

  cur_row = ROWS - 1;
}

void vprintf(const char *fmt, va_list args)
{
  for (; *fmt; fmt++)
  {
    if (*fmt != '%')
    {
      putchar(*fmt);
      continue;
    }

    fmt++;
    switch (*fmt)
    {
      case 'c':
      {
        char c = static_cast<char>(va_arg(args, int));
        putchar(c);
        break;
      }
      case 's':
      {
        auto str = static_cast<const char *>(va_arg(args, const char *));
        while (*str) putchar(*str++);
        break;
      }
      case 'd':
      case 'u':
        print_int(va_arg(args, int));
        break;

      case 'x':
        print_hex(va_arg(args, u32));
        break;

      case '%':
        putchar('%');
        break;

      case 'p':
      {
        auto ptr = va_arg(args, u32*);
        auto addr = reinterpret_cast<u32>(ptr);
        print_hex(addr);
        break;
      }

      default:
        putchar('?');
        break;
    }
  }
}

} // namespace anonimo

void init()
{
  cur_color = make_color(Color::LightGrey, Color::Black);
  clear();
}

void clear()
{
  u16 blank_entry = make_entry(' ', cur_col);

  for (u32 i = 0; i < ROWS * COLS; i++)
  {
    buffer[i] = blank_entry;
  }

  cur_col = 0;
  cur_row = 0;
}

void set_color(Color fg, Color bg)
{
  cur_color = make_color(fg, bg);
}


void newline()
{
  cur_col = 0;
  cur_row++;

  if (cur_row >= ROWS)
    scroll();
}

void putchar(char c)
{
  switch (c)
  {
    case '\n': newline(); return;
    case '\r': cur_col = 0; return;
    case '\t':
      cur_col += 4;
      if (cur_col >= COLS)
        newline();
      return;
  }

  buffer[cur_row * COLS + cur_col] = make_entry(c, cur_color);
  cur_col++;
  if (cur_col >= COLS)
    newline();
}

void print(const char *str)
{
  while (*str)
    putchar(*str++);
}

void print_hex(u32 n)
{
  print("0x");

  for (int i = 7; i >=0; i--)
  {
     u8 nibble = (n >> (i*4)) & 0xF;
     char c = (nibble < 10) ? '0' + nibble : 'A' + nibble - 10;
     putchar(c);
  }
}

void print_int(i32 n)
{
  if (n == 0)
  {
    putchar('0');
    return;
  }

  if (n < 0)
  {
    putchar('-');
    n = -n;
  }

  char buff[12];
  memset(buff, 0, sizeof(buff));

  int i = 0;
  while (n > 0)
  {
    buff[i++] = '0' + (n % 10);
    n /= 10;
  }

  while (--i >= 0)
    putchar(buff[i]);
}

void printf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  kfmt::printf(putchar, fmt, args);
  va_end(args);
}

void println(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  putchar('\n');
}

} // namespace vga 


