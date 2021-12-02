// eCos memory layout - Tue Aug 14 12:49:58 2001

// This is a generated file - do not edit

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>
#endif

#define CYGMEM_REGION_ram (0)
/*
#define CYGMEM_REGION_ram_SIZE (0x1600000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#define CYGMEM_REGION_rest (0x1600000)
#define CYGMEM_REGION_rest_SIZE (0x2A00000)
#define CYGMEM_REGION_rest_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
*/
/****** for old usr  *********/
#define CYGMEM_REGION_ram_SIZE (0x2600000)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#define CYGMEM_REGION_rest (0x2600000)
#define CYGMEM_REGION_rest_SIZE (0x1A00000)
#define CYGMEM_REGION_rest_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
/***** for old usr  ********/

#define CYGMEM_REGION_rom (0x34000000)
#define CYGMEM_REGION_rom_SIZE (0x800000)
#define CYGMEM_REGION_rom_ATTR (CYGMEM_REGION_ATTR_R)
#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
//#define CYGMEM_SECTION_heap1_SIZE (0x1600000 - (size_t) CYG_LABEL_NAME (__heap1))

/***** for old usr  *********/
#define CYGMEM_SECTION_heap1_SIZE (0x2600000 - (size_t) CYG_LABEL_NAME (__heap1))
/***** for old usr  ********/
