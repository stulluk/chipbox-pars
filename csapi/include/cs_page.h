/*
 * cs_page.h - page size macros for CSAPI user-space mmap helpers.
 *
 * The original CSAPI included <asm/page.h>, which modern kernel UAPI headers no
 * longer export to user space. ARM926EJ-S (CSM120x) always uses 4 KiB pages.
 */
#ifndef CS_PAGE_H
#define CS_PAGE_H

#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12
#endif
#ifndef PAGE_SIZE
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#endif

#endif /* CS_PAGE_H */
