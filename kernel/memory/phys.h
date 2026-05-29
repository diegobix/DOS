#pragma once

#include "types.h"
#include "multiboot.h"

namespace memory
{

  constexpr u32 virt_to_phys(u32 addr)
  {
    return addr - 0xC0000000;
  }

  constexpr u32 phys_to_virt(u32 paddr)
  {
    return paddr + 0xC0000000;
  }

namespace phys
{

void init(multiboot::MultibootInfo *info, u32 kernel_end);

u32 alloc_frame();

void free_frame(u32 frame_addr);
void free_frame(u32 *frame_ptr);

void print_stats();

} // namespace memory::phys

} // namespace memory