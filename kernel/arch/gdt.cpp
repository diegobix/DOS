#include "gdt.h"

#include "types.h"

namespace GDT
{

namespace
{

struct [[gnu::packed]] Descriptor
{
  u16 limit_low;
  u16 base_low;
  u8 base_mid;
  u8 access_byte;
  u8 flags_and_limit_high;
  u8 base_high;
};

struct [[gnu::packed]] Register
{
  u16 limit;
  u32 base;
};

constexpr u32 DESCRIPTOR_COUNT = 3;
Descriptor table[DESCRIPTOR_COUNT];
Register gdtr;

constexpr Descriptor make_descriptor(u32 base, u32 limit, u8 access, u8 flags)
{
  return Descriptor {
    .limit_low = static_cast<u16>(limit & 0xFFFF),
    .base_low = static_cast<u16>(base & 0xFFFF),
    .base_mid = static_cast<u8>((base >> 16) & 0xFF),
    .access_byte = access,
    .flags_and_limit_high = static_cast<u8>(((flags & 0xF) << 4) | ((limit >> 16) & 0xF) ),
    .base_high = static_cast<u8>((base >> 24) & 0xFF)
  };
}

} // Namespace anon 

void init()
{
  // null Descriptor
  table[0] = make_descriptor(0, 0, 0, 0);

  // Code segment
  table[1] = make_descriptor(0, 0xFFFFFFFF, 0x9A, 0xC);

  // Data segment 
  table[2] = make_descriptor(0, 0xFFFFFFFF, 0x92, 0xC);

  gdtr.limit = sizeof(table) - 1;
  gdtr.base = reinterpret_cast<u32>(&table);

  asm volatile(
      "lgdt %[reg]\n"
      :
      : [reg] "m" (gdtr)
  );
}

} // namespace gdt 
