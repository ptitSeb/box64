#ifndef ELFLOADER_DUMP_H
#define ELFLOADER_DUMP_H

typedef struct elfheader_s elfheader_t;

const char* SymName32(elfheader_t *h, Elf32_Sym* sym);
const char* SymName64(elfheader_t *h, Elf64_Sym* sym);
const char* DumpRelType32(int t);
const char* DumpRelType64(int t);
void DumpMainHeader32(Elf32_Ehdr *header, elfheader_t *h);
void DumpMainHeader64(Elf64_Ehdr *header, elfheader_t *h);
void DumpSymTab32(elfheader_t *h);
void DumpSymTab64(elfheader_t *h);
void DumpDynamicSections32(elfheader_t *h);
void DumpDynamicSections64(elfheader_t *h);
void DumpDynamicNeeded(elfheader_t *h);
void DumpDynamicRPath(elfheader_t *h);
void DumpDynSym32(elfheader_t *h);
void DumpDynSym64(elfheader_t *h);
void DumpRelTable32(elfheader_t *h, int cnt, Elf32_Rel *rel, const char* name);
void DumpRelTable64(elfheader_t *h, int cnt, Elf64_Rel *rel, const char* name);
void DumpRelATable32(elfheader_t *h, int cnt, Elf32_Rela *rela, const char* name);
void DumpRelATable64(elfheader_t *h, int cnt, Elf64_Rela *rela, const char* name);
void DumpRelRTable32(elfheader_t *h, int cnt, Elf32_Relr *relr, const char *name);
void DumpRelRTable64(elfheader_t *h, int cnt, Elf64_Relr *relr, const char *name);

void DumpBinary(char* p, int sz);

#ifndef SHT_CHECKSUM
#define SHT_CHECKSUM 0x6ffffff8
#endif

#endif //ELFLOADER_DUMP_H