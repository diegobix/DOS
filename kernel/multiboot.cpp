#include "multiboot.h"

#include "debug.h"

namespace
{
  constexpr u32 phys_to_virt(u32 addr)
  {
    return addr + 0xC0000000;
  }

}

namespace multiboot
{

void print_mmap(const MultibootInfo *info)
{
  if (!(info->flags & FLAG_MMAP))
  {
    DEBUG::log("MMap no disponible!!");
    return;
  }

  MemoryMapEntry *entry = reinterpret_cast<MemoryMapEntry *>(phys_to_virt(info->mmap_addr));

  MemoryMapEntry *end = reinterpret_cast<MemoryMapEntry *>(phys_to_virt(info->mmap_addr + info->mmap_len));

  DEBUG::log("\n\nMAPA DE MEMORIA\n");
  while (entry < end)
  {
    const char *type_str = mmap_type_to_str(entry->type);

    DEBUG::log("  base=%X length=%X type=%s\n", entry->addr, entry->len, type_str);

    entry = reinterpret_cast<MemoryMapEntry *>(
      reinterpret_cast<u32>(entry) + entry->size + sizeof(entry->size)
    );
  }
}

}

