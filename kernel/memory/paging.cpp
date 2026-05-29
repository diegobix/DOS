#include "paging.h"
#include "debug.h"
#include "phys.h"

namespace paging
{
namespace
{

constexpr u32 PAGE_DIR_VADDR = 0xFFFFF000;
constexpr u32 PAGE_TAB_FROM_DIR_BASE = 0xFFC00000;

constexpr u32 PAGE_SIZE = 0x1000;
constexpr u32 TABLE_ENTRIES = 1024;

constexpr u32 pd_index(u32 vaddr) {return vaddr >> 22;}
constexpr u32 pt_index(u32 vaddr) {return (vaddr >> 12) & 0x3FF;}
constexpr u32 pt_offset(u32 vaddr) {return vaddr & 0xFFF;}

inline u32* pd_ptr() {return reinterpret_cast<u32*>(PAGE_DIR_VADDR);}
constexpr u32 pt_vaddr(u32 pdi) {return PAGE_TAB_FROM_DIR_BASE + pdi * 0x1000;}
inline u32* pt_ptr(u32 pdi) {return reinterpret_cast<u32*>(pt_vaddr(pdi));}

inline void flush_tlb(u32 virt) {asm volatile("invlpg [%0]" : : "r"(virt));}

void ensure_page_table(u32 pdi)
{
  if (pd_ptr()[pdi] & PRESENT) return;

  u32 pt_paddr = memory::phys::alloc_frame();
  pd_ptr()[pdi] = pt_paddr | PRESENT | RW;
  
  flush_tlb(pt_vaddr(pdi));
  u32 *new_page_table_ptr = pt_ptr(pdi);
  for (u32 i = 0; i < TABLE_ENTRIES; i++)
  {
    new_page_table_ptr[i] = 0;
  }
}

} // namespace anon

void map_page(u32 virt, u32 phys, u32 flags)
{
  u32 pdi = pd_index(virt);
  u32 pti = pt_index(virt);

  ensure_page_table(pdi);

  pt_ptr(pdi)[pti] = phys | flags;
  flush_tlb(virt);
}

void unmap_page(u32 virt)
{
  u32 pdi = pd_index(virt);
  u32 pti = pt_index(virt);

  if (!(pd_ptr()[pdi] & PRESENT)) return;

  pt_ptr(pdi)[pti] = 0;
  flush_tlb(virt);
}

u32 virt_to_phys(u32 vaddr)
{
  u32 pdi = pd_index(vaddr);
  u32 pti = pt_index(vaddr);

  if (!(pd_ptr()[pdi] & PRESENT)) return 0;
  if (!(pt_ptr(pdi)[pti] & PRESENT)) return 0;

  return (pt_ptr(pdi)[pti] & ~0xFFF) | pt_offset(vaddr);
}

void init(u32 kernel_end)
{
  u32 new_page_dir_paddr = memory::phys::alloc_frame();
  DEBUG::log("Page dir paddr: %x\n", new_page_dir_paddr);
  u32 *new_page_dir_ptr = reinterpret_cast<u32 *>(new_page_dir_paddr);

  for (u32 i = 0; i < TABLE_ENTRIES; i++)
  {
    new_page_dir_ptr[i] = 0;
  }

  new_page_dir_ptr[1023] = new_page_dir_paddr | PRESENT | RW;

  // Mapeo del kernel
  u32 paddr = 0;
  u32 vaddr = 0xC0000000;

  while (vaddr < kernel_end)
  {

    u32 pdi = pd_index(vaddr);
    u32 pti = pt_index(vaddr);

    if (!(new_page_dir_ptr[pdi] & PRESENT))
    {
      u32 pt_addr = memory::phys::alloc_frame();
      new_page_dir_ptr[pdi] = pt_addr | PRESENT | RW;

      u32 *pt_ptr = reinterpret_cast<u32 *>(pt_addr);
      for (u32 i = 0; i < TABLE_ENTRIES; i++)
      {
        pt_ptr[i] = 0;
      }
    }

    u32 pt_addr = new_page_dir_ptr[pdi] & ~0xFFF;
    u32 *pt_ptr = reinterpret_cast<u32 *>(pt_addr);

    pt_ptr[pti] = paddr | PRESENT | RW;

    paddr += 0x1000;
    vaddr += 0x1000;
  }

  {
    u32 vaddr = 0xb8000;
    u32 paddr = vaddr;
    u32 pdi = pd_index(vaddr);
    u32 pti = pt_index(vaddr);

    if (!(new_page_dir_ptr[pdi] & PRESENT))
    {
      u32 pt_addr = memory::phys::alloc_frame();
      new_page_dir_ptr[pdi] = pt_addr | PRESENT | RW;

      u32 *pt_ptr = reinterpret_cast<u32 *>(pt_addr);
      for (u32 i = 0; i < TABLE_ENTRIES; i++)
      {
        pt_ptr[i] = 0;
      }
    }

    u32 pt_addr = new_page_dir_ptr[pdi] & ~0xFFF;
    u32 *pt_ptr = reinterpret_cast<u32 *>(pt_addr);

    pt_ptr[pti] = paddr | PRESENT | RW;
  }

  DEBUG::log("Llega\n");
  asm volatile("mov cr3, %0" : : "r" (new_page_dir_paddr));

  // Desactivar PSE
  u32 cr4;
  asm volatile("mov %0, cr4" : "=r"(cr4) : );
  cr4 &= ~(1u << 4);
  asm volatile("mov cr4, %0" : : "r"(cr4));

  DEBUG::log("[VMM] Paging inicializado correctamente\n");
  DEBUG::log("[VMM] PD paddr: %x\n", new_page_dir_paddr);
  DEBUG::log("[VMM] Kernel mapeado desde 0xC0000000 hasta %x\n", kernel_end);
}

} // namespace paging