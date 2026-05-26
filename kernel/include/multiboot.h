#pragma once

#include "types.h"

namespace multiboot
{

constexpr u32 FLAG_MEM = 1 << 0;
constexpr u32 FLAG_DEVICE = 1 << 1;
constexpr u32 FLAG_CMDLINE = 1 << 2;
constexpr u32 FLAG_MODS = 1 << 3;
constexpr u32 FLAG_SYMS = 0b11 << 4;
constexpr u32 FLAG_MMAP = 1 << 6;
constexpr u32 FLAG_DRIVES = 1 << 7;
constexpr u32 FLAG_CONFIG = 1 << 8;
constexpr u32 FLAG_NAME = 1 << 9;
constexpr u32 FLAG_APM = 1 << 10;
constexpr u32 FLAG_VBE = 1 << 11;
constexpr u32 FLAG_FRAMEBUFFER = 1 << 12;


struct [[gnu::packed]] MultibootInfo
{
  u32 flags;
  u32 mem_lower;
  u32 mem_upper;
  u32 boot_device;
  u32 cmdline;
  u32 mods_count;
  u32 mods_addr;
  u32 syms[4];
  u32 mmap_len;
  u32 mmap_addr;
  u32 drives_len;
  u32 drives_addr;
  u32 config_table;
  u32 boot_loader_name;
  u32 apm_table;
  u32 vbe[6];
  u32 framebuffer[11];
};

struct [[gnu::packed]] MemoryMapEntry
{
  u32 size;
  u64 addr;
  u64 len;
  u32 type;
};

constexpr const char *MMAP_TYPES[] =
{
  "Usable", "Reserved", "ACPI", "NVS", "bad"
};

constexpr const char *mmap_type_to_str(u32 type)
{
  return type <= 5 ? MMAP_TYPES[type-1] : "Unknown";
};

void print_mmap(const MultibootInfo *info);

}
