#ifndef ELFLOADER_DUMP_H
#define ELFLOADER_DUMP_H

typedef struct elfheader_s elfheader_t;

const char* DumpSection(Elf64_Shdr *s, char* SST);
const char* DumpDynamic(Elf64_Dyn *s);
const char* DumpPHEntry(Elf64_Phdr *e);
const char* DumpSym(elfheader_t *h, Elf64_Sym* sym, int version);
const char* DumpRelType(int t);
const char* SymName(elfheader_t *h, Elf64_Sym* sym);
const char* IdxSymName(elfheader_t *h, int sym);
void DumpMainHeader(Elf64_Ehdr *header, elfheader_t *h);
void DumpSymTab(elfheader_t *h);
void DumpDynamicSections(elfheader_t *h);
void DumpDynamicNeeded(elfheader_t *h);
void DumpDynamicRPath(elfheader_t *h);
void DumpDynSym(elfheader_t *h);
void DumpRelTable(elfheader_t *h, int cnt, Elf64_Rel *rel, const char* name);
void DumpRelATable(elfheader_t *h, int cnt, Elf64_Rela *rela, const char* name);

void DumpBinary(char* p, int sz);

#endif //ELFLOADER_DUMP_H