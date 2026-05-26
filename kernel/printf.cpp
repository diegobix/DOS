#include "printf.h"
#include "types.h"

namespace kfmt
{

namespace
{

void print_int(PutcharFn putchar_fn, int n)
{
  if (n == 0)
  {
    putchar_fn('0');
    return;
  }

  if (n < 0)
  {
    putchar_fn('-');
    n = -n;
  }

  char buff[12];

  int i = 0;
  while (n > 0)
  {
    buff[i++] = '0' + (n % 10);
    n /= 10;
  }

  while (--i >= 0)
    putchar_fn(buff[i]);
}

void print_hex(PutcharFn putchar_fn, u32 n)
{
  putchar_fn('0');
  putchar_fn('x');

  for (int i = 7; i >=0; i--)
  {
     u8 nibble = (n >> (i*4)) & 0xF;
     char c = (nibble < 10) ? '0' + nibble : 'A' + nibble - 10;
     putchar_fn(c);
  }
}

void print_hex(PutcharFn putchar_fn, u64 n)
{
  putchar_fn('0');
  putchar_fn('x');

  for (int i = 15; i >=0; i--)
  {
     u8 nibble = (n >> (i*4)) & 0xF;
     char c = (nibble < 10) ? '0' + nibble : 'A' + nibble - 10;
     putchar_fn(c);
  }
}

  
} // namespace anon

void printf(PutcharFn putchar_fn, const char *fmt, va_list args)
{
  for (; *fmt; fmt++)
  {
    if (*fmt != '%')
    {
      putchar_fn(*fmt);
      continue;
    }

    fmt++;
    switch (*fmt)
    {
      case 'c':
      {
        char c = static_cast<char>(va_arg(args, int));
        putchar_fn(c);
        break;
      }
      case 's':
      {
        auto str = static_cast<const char *>(va_arg(args, const char *));
        while (*str) putchar_fn(*str++);
        break;
      }
      case 'd':
      case 'u':
        print_int(putchar_fn, va_arg(args, int));
        break;

      case 'x':
        print_hex(putchar_fn, va_arg(args, u32));
        break;

      case 'X':
        print_hex(putchar_fn, va_arg(args, u64));
        break;

      case '%':
        putchar_fn('%');
        break;

      case 'p':
      {
        auto ptr = va_arg(args, u32*);
        auto addr = reinterpret_cast<u32>(ptr);
        print_hex(putchar_fn, addr);
        break;
      }

      default:
        putchar_fn('?');
        break;
    }
  }
}

}
