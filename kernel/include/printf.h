#pragma once

#include <stdarg.h>

namespace kfmt
{

using PutcharFn = void(*)(char);

void printf(PutcharFn putchar_fn, const char *fmt, va_list args);

}
