#ifndef ELF_H
#define ELF_H
#define ELF_MAGIC 0x464c457f

#include <stdint.h>


typedef struct {
  uint32_t magic;
  char type;
  char endian;
  char version;
  char abi;
  char padding[8];
  uint16_t type2;
  uint16_t iset;
  uint32_t version2;
  uint32_t entry;
  uint32_t prog_hdr;
  uint32_t section_hdr;
  uint32_t flags;
  uint16_t header_sz;
  uint16_t pheader_ent_sz;
  uint16_t pheader_ent_nm;
  uint16_t sheader_ent_sz;
  uint16_t sheader_ent_nm;
  uint16_t sheader_nm_idx;
} __attribute__((packed)) elf_header;

typedef struct  {
  uint32_t type;
  uint32_t offset;
  uint32_t vaddr;
  uint32_t unused;
  uint32_t filesz;
  uint32_t memsz;
  uint32_t flags;
  uint32_t alignment;
} __attribute__((packed)) elf_pheader;


#endif
