//
// Created by ben on 31/12/17.
//

#ifndef COMMON_HOST_H
#define COMMON_HOST_H

APPCALL_DEF2(write, 0, 1, 0, 0, 0, 0, 0)
APPCALL_DEF2(read, 0, 1, 0, 0, 0, 0, 0)
APPCALL_DEF2(open, 1, 0, 0, 0, 0, 0, 0)
APPCALL_DEF2(openat, 0, 1, 0, 0, 0, 0, 0)
APPCALL_DEF(close)
APPCALL_DEF(dup)
APPCALL_DEF2(fstat, 0, 1, 0, 0, 0, 0, 0)
APPCALL_DEF2(stat, 1, 1, 0, 0, 0, 0, 0)
APPCALL_DEF2(mmap, 1, 0, 0, 0, 0, 0, 1)
APPCALL_DEF(lseek)
APPCALL_DEF2(access, 1, 0, 0, 0, 0, 0, 0)
APPCALL_DEF3(writev)
APPCALL_DEF2(uname, 1, 0, 0, 0, 0, 0, 0)

KERNELCALL_DEF(open)
KERNELCALL_DEF(exit)
KERNELCALL_DEF(write)
KERNELCALL_DEF(read)
KERNELCALL_DEF(stat)
KERNELCALL_DEF(fstat)

KERNELCALL_DEF(set_vma_heap)
KERNELCALL_DEF(map_vma)
KERNELCALL_DEF(unmap_vma)
KERNELCALL_DEF2(allocate_pages)
KERNELCALL_DEF2(map_physical_pages)
KERNELCALL_DEF2(is_vpage_present)
KERNELCALL_DEF2(get_phys_addr)
KERNELCALL_DEF_CUSTOM(unmap_physical_page)

#endif //COMMON_HOST_H
