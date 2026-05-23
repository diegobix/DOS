#pragma once

namespace uart
{

int init();
void write_char(char c);
void printf(const char *fmt, ...);

}
