#include "phys.h"

#include "debug.h"
#include "multiboot.h"
#include "types.h"

namespace memory
{
namespace phys
{

namespace
{

constexpr u32 FRAME_SIZE = 4096;
constexpr u32 BITS_PER_ENTRY = 32;
constexpr u32 MAX_FRAMES = (0xFFFFFFFF / FRAME_SIZE) + 1;
constexpr u32 BITMAP_SIZE = MAX_FRAMES / BITS_PER_ENTRY;

u32 bitmap[BITMAP_SIZE];
u32 free_frames = 0;
u32 total_frames = MAX_FRAMES;

void set_used(u32 frame)
{
  bitmap[frame / BITS_PER_ENTRY] |= (1u << (frame % BITS_PER_ENTRY));
}

void set_free(u32 frame)
{
  bitmap[frame / BITS_PER_ENTRY] &= ~(1u << (frame % BITS_PER_ENTRY));
}

bool is_used(u32 frame)
{
  return bitmap[frame / BITS_PER_ENTRY] & (1u << (frame % BITS_PER_ENTRY));
}

void mark_range_used(u32 base, u32 len)
{
  u32 first = static_cast<u32>(base) / FRAME_SIZE;
  u32 last = static_cast<u32>(base + len + FRAME_SIZE - 1) / FRAME_SIZE;

  for (u32 f = first; f < last && f < MAX_FRAMES; f++)
  {
    if (!is_used(f))
    {
      set_used(f);
      free_frames--;
    }
  }
}

void mark_range_free(u32 base, u32 len)
{
  u32 first = static_cast<u32>(base) / FRAME_SIZE;
  u32 last = static_cast<u32>(base + len + FRAME_SIZE - 1) / FRAME_SIZE;

  for (u32 f = first; f < last && f < MAX_FRAMES; f++)
  {
    if (is_used(f))
    {
      set_free(f);
      free_frames++;
    }
  }
}



} // namespace anon


void init(multiboot::MultibootInfo *info, u32 kernel_end)
{
  total_frames = MAX_FRAMES;
  free_frames = 0;

  // 1. Marcar todo el bitmap como usado
  for (u32 i = 0; i < BITMAP_SIZE; i++)
  {
    bitmap[i] = 0xFFFFFFFF;
  }
  
  // 2. Iterar mmap y marcar como usado lo ocupado
  multiboot::MMap mmap(info);
  for (auto &entry : mmap)
  {
    if (entry.type == multiboot::MMap::AVAILABLE && entry.addr < 0x100000000ULL)
    {
      mark_range_free(entry.addr, entry.len);
    }
  }
  
  // 3. Marcar primer MB como usado
  mark_range_used(0x0, 0x100000);
  
  // 4. Marcar kernel como usado (1MB a kernel_end)
  u32 kernel_end_phys = virt_to_phys(kernel_end);
  u32 kernel_size = kernel_end_phys - 0x100000;
  mark_range_used(0x100000, kernel_size);
}

void print_stats()
{
  DEBUG::log("PHYS FRAME ALLOCATOR\n");
  DEBUG::log("\tFrames: %u\n", total_frames);
  DEBUG::log("\tFree  : %u\n", free_frames);
}

u32 alloc_frame()
{
  if (free_frames == 0) return 0;
  
  for (u32 i = 0; i < BITMAP_SIZE; i++)
  {
    if (bitmap[i] == 0xFFFFFFFF) continue;

    for (u32 bit = 0; bit < BITS_PER_ENTRY; bit++)
    {
      if ((~(bitmap[i]) & (1u << bit)))
      {
        u32 frame = i * BITS_PER_ENTRY + bit;
        set_used(frame);
        free_frames--;
        return frame * FRAME_SIZE; // return the addr
      }
    }
  }

  return 0;
}

void free_frame(u32 frame_addr)
{

}

void free_frame(u32 *frame_ptr)
{

}

} //namespace phys
  
}// namespace memory