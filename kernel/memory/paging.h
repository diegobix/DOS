#pragma once

#include "types.h"
#include "phys.h"

namespace paging
{

enum PageFlags : u32
{
  PRESENT = 1 << 0,
  RW      = 1 << 1,
  USER    = 1 << 2,
  CACHE   = 1 << 3,
};

void init(u32 kernel_end);

void map_page(u32 virt, u32 phys, u32 flags);
void unmap_page(u32 virt);

u32 virt_to_phys(u32 vaddr);

} // namespace paging