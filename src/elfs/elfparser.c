#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#include "box64version.h"
#include "elfloader.h"
#include "debug.h"
#include "elfload_dump.h"
#include "elfloader_private.h"

#ifndef PN_XNUM 
#define PN_XNUM (0xffff)
#endif

int LoadSH(FILE *f, Elf64_Shdr *s, void** SH, const char* name, uint32_t type)
{
    if(type && (s->sh_type != type)) {
        printf_log(LOG_INFO, "Section Header \"%s\" (off=%ld, size=%ld) has incorect type (%d != %d)\n", name, s->sh_offset, s->sh_size, s->sh_type, type);
        return -1;
    }
    if (type==SHT_SYMTAB && s->sh_size%sizeof(Elf64_Sym)) {
        printf_log(LOG_INFO, "Section Header \"%s\" (off=%ld, size=%ld) has size (not multiple of %ld)\n", name, s->sh_offset, s->sh_size, sizeof(Elf64_Sym));
    }
    *SH = calloc(1, s->sh_size);
    fseeko64(f, s->sh_offset ,SEEK_SET);
    if(fread(*SH, s->sh_size, 1, f)!=1) {
            printf_log(LOG_INFO, "Cannot read Section Header \"%s\" (off=%ld, size=%ld)\n", name, s->sh_offset, s->sh_size);
            return -1;
    }

    return 0;
}

int FindSection(Elf64_Shdr *s, int n, char* SHStrTab, const char* name)
{
    for (int i=0; i<n; ++i) {
        if(s[i].sh_type!=SHT_NULL)
            if(!strcmp(SHStrTab+s[i].sh_name, name))
                return i;
    }
    return 0;
}

void LoadNamedSection(FILE *f, Elf64_Shdr *s, int size, char* SHStrTab, const char* name, const char* clearname, uint32_t type, void** what, int* num)
{
    int n = FindSection(s, size, SHStrTab, name);
    printf_log(LOG_DEBUG, "Loading %s (idx = %d)\n", clearname, n);
    if(n)
        LoadSH(f, s+n, what, name, type);
    if(type==SHT_SYMTAB || type==SHT_DYNSYM) {
        if(*what && num)
            *num = s[n].sh_size / sizeof(Elf64_Sym);
    } else if(type==SHT_DYNAMIC) {
        if(*what && num)
            *num = s[n].sh_size / sizeof(Elf64_Dyn);
    }
}

elfheader_t* ParseElfHeader(FILE* f, const char* name, int exec)
{
    Elf64_Ehdr header;
    int level = (exec)?LOG_INFO:LOG_DEBUG;
    if(fread(&header, sizeof(Elf64_Ehdr), 1, f)!=1) {
        printf_log(level, "Cannot read ELF Header\n");
        return NULL;
    }
    if(memcmp(header.e_ident, ELFMAG, SELFMAG)!=0) {
        printf_log(LOG_INFO, "Not an ELF file (sign=%c%c%c%c)\n", header.e_ident[0], header.e_ident[1], header.e_ident[2], header.e_ident[3]);
        return NULL;
    }
    if(header.e_ident[EI_CLASS]!=ELFCLASS64) {
        if(header.e_ident[EI_CLASS]==ELFCLASS32) {
            printf_log(LOG_INFO, "This is a 32bits ELF! box64 can only run 64bits ELF!\n");
        } else {
            printf_log(LOG_INFO, "Not a 64bits ELF (%d)\n", header.e_ident[EI_CLASS]);
        }
        return NULL;
    }
    if(header.e_ident[EI_DATA]!=ELFDATA2LSB) {
        printf_log(LOG_INFO, "Not a LittleEndian ELF (%d)\n", header.e_ident[EI_DATA]);
        return NULL;
    }
    if(header.e_ident[EI_VERSION]!=EV_CURRENT) {
        printf_log(LOG_INFO, "Incorrect ELF version (%d)\n", header.e_ident[EI_VERSION]);
        return NULL;
    }
    if(header.e_ident[EI_OSABI]!=ELFOSABI_LINUX && header.e_ident[EI_OSABI]!=ELFOSABI_NONE && header.e_ident[EI_OSABI]!=ELFOSABI_SYSV) {
        printf_log(LOG_INFO, "Not a Linux ELF (%d)\n",header.e_ident[EI_OSABI]);
        return NULL;
    }

    if(header.e_type != ET_EXEC && header.e_type != ET_DYN) {
        printf_log(LOG_INFO, "Not an Executable (%d)\n", header.e_type);
        return NULL;
    }

    if(header.e_machine != EM_X86_64) {
        printf_log(level, "Not an x86_64 ELF (%d)\n", header.e_machine);
        return NULL;
    }

    if(header.e_entry == 0 && exec) {
        printf_log(LOG_INFO, "No entry point in ELF\n");
        return NULL;
    }
    if(header.e_phentsize != sizeof(Elf64_Phdr)) {
        printf_log(LOG_INFO, "Program Header Entry size incorrect (%d != %ld)\n", header.e_phentsize, sizeof(Elf64_Phdr));
        return NULL;
    }
    if(header.e_shentsize != sizeof(Elf64_Shdr) && header.e_shentsize != 0) {
        printf_log(LOG_INFO, "Section Header Entry size incorrect (%d != %ld)\n", header.e_shentsize, sizeof(Elf64_Shdr));
        return NULL;
    }

    elfheader_t *h = calloc(1, sizeof(elfheader_t));
    h->name = strdup(name);
    h->entrypoint = header.e_entry;
    h->numPHEntries = header.e_phnum;
    h->numSHEntries = header.e_shnum;
    h->SHIdx = header.e_shstrndx;
    h->e_type = header.e_type;
    if(header.e_shentsize && header.e_shnum) {
        // special cases for nums
        if(h->numSHEntries == 0) {
            printf_log(LOG_DEBUG, "Read number of Sections in 1st Section\n");
            // read 1st section header and grab actual number from here
            fseeko64(f, header.e_shoff, SEEK_SET);
            Elf64_Shdr section;
            if(fread(&section, sizeof(Elf64_Shdr), 1, f)!=1) {
                free(h);
                printf_log(LOG_INFO, "Cannot read Initial Section Header\n");
                return NULL;
            }
            h->numSHEntries = section.sh_size;
        }
        // now read all section headers
        printf_log(LOG_DEBUG, "Read %d Section header\n", h->numSHEntries);
        h->SHEntries = (Elf64_Shdr*)calloc(h->numSHEntries, sizeof(Elf64_Shdr));
        fseeko64(f, header.e_shoff ,SEEK_SET);
        if(fread(h->SHEntries, sizeof(Elf64_Shdr), h->numSHEntries, f)!=h->numSHEntries) {
                FreeElfHeader(&h);
                printf_log(LOG_INFO, "Cannot read all Section Header\n");
                return NULL;
        }

        if(h->numPHEntries == PN_XNUM) {
            printf_log(LOG_DEBUG, "Read number of Program Header in 1st Section\n");
            // read 1st section header and grab actual number from here
            h->numPHEntries = h->SHEntries[0].sh_info;
        }
    }

    printf_log(LOG_DEBUG, "Read %d Program header\n", h->numPHEntries);
    h->PHEntries = (Elf64_Phdr*)calloc(h->numPHEntries, sizeof(Elf64_Phdr));
    fseeko64(f, header.e_phoff ,SEEK_SET);
    if(fread(h->PHEntries, sizeof(Elf64_Phdr), h->numPHEntries, f)!=h->numPHEntries) {
            FreeElfHeader(&h);
            printf_log(LOG_INFO, "Cannot read all Program Header\n");
            return NULL;
    }

    if(header.e_shentsize && header.e_shnum) {
        if(h->SHIdx == SHN_XINDEX) {
            printf_log(LOG_DEBUG, "Read number of String Table in 1st Section\n");
            h->SHIdx = h->SHEntries[0].sh_link;
        }
        if(h->SHIdx > h->numSHEntries) {
            printf_log(LOG_INFO, "Incoherent Section String Table Index : %d / %d\n", h->SHIdx, h->numSHEntries);
            FreeElfHeader(&h);
            return NULL;
        }
        // load Section table
        printf_log(LOG_DEBUG, "Loading Sections Table String (idx = %d)\n", h->SHIdx);
        if(LoadSH(f, h->SHEntries+h->SHIdx, (void*)&h->SHStrTab, ".shstrtab", SHT_STRTAB)) {
            FreeElfHeader(&h);
            return NULL;
        }
        if(box64_log>=LOG_DUMP) DumpMainHeader(&header, h);

        LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".strtab", "SymTab Strings", SHT_STRTAB, (void**)&h->StrTab, NULL);
        LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".symtab", "SymTab", SHT_SYMTAB, (void**)&h->SymTab, &h->numSymTab);
        if(box64_log>=LOG_DUMP && h->SymTab) DumpSymTab(h);

        LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".dynamic", "Dynamic", SHT_DYNAMIC, (void**)&h->Dynamic, &h->numDynamic);
        if(box64_log>=LOG_DUMP && h->Dynamic) DumpDynamicSections(h);
        // grab DT_REL & DT_RELA stuffs
        // also grab the DT_STRTAB string table
        {
            for (int i=0; i<h->numDynamic; ++i) {
                if(h->Dynamic[i].d_tag == DT_REL)
                    h->rel = h->Dynamic[i].d_un.d_ptr;
                else if(h->Dynamic[i].d_tag == DT_RELSZ)
                    h->relsz = h->Dynamic[i].d_un.d_val;
                else if(h->Dynamic[i].d_tag == DT_RELENT)
                    h->relent = h->Dynamic[i].d_un.d_val;
                else if(h->Dynamic[i].d_tag == DT_RELA)
                    h->rela = h->Dynamic[i].d_un.d_ptr;
                else if(h->Dynamic[i].d_tag == DT_RELASZ)
                    h->relasz = h->Dynamic[i].d_un.d_val;
                else if(h->Dynamic[i].d_tag == DT_RELAENT)
                    h->relaent = h->Dynamic[i].d_un.d_val;
                else if(h->Dynamic[i].d_tag == DT_PLTGOT)
                    h->pltgot = h->Dynamic[i].d_un.d_val;
                else if(h->Dynamic[i].d_tag == DT_PLTREL)
                    h->pltrel = h->Dynamic[i].d_un.d_val;
                else if(h->Dynamic[i].d_tag == DT_PLTRELSZ)
                    h->pltsz = h->Dynamic[i].d_un.d_val;
                else if(h->Dynamic[i].d_tag == DT_JMPREL)
                    h->jmprel = h->Dynamic[i].d_un.d_val;
                else if(h->Dynamic[i].d_tag == DT_STRTAB)
                    h->DynStrTab = (char*)(h->Dynamic[i].d_un.d_ptr);
                else if(h->Dynamic[i].d_tag == DT_STRSZ)
                    h->szDynStrTab = h->Dynamic[i].d_un.d_val;
            }
            if(h->rel) {
                if(h->relent != sizeof(Elf64_Rel)) {
                    printf_log(LOG_NONE, "Rel Table Entry size invalid (0x%x should be 0x%lx)\n", h->relent, sizeof(Elf64_Rel));
                    FreeElfHeader(&h);
                    return NULL;
                }
                printf_log(LOG_DEBUG, "Rel Table @%p (0x%x/0x%x)\n", (void*)h->rel, h->relsz, h->relent);
            }
            if(h->rela) {
                if(h->relaent != sizeof(Elf64_Rela)) {
                    printf_log(LOG_NONE, "RelA Table Entry size invalid (0x%x should be 0x%lx)\n", h->relaent, sizeof(Elf64_Rela));
                    FreeElfHeader(&h);
                    return NULL;
                }
                printf_log(LOG_DEBUG, "RelA Table @%p (0x%x/0x%x)\n", (void*)h->rela, h->relasz, h->relaent);
            }
            if(h->jmprel) {
                if(h->pltrel == DT_REL) {
                    h->pltent = sizeof(Elf64_Rel);
                } else if(h->pltrel == DT_RELA) {
                    h->pltent = sizeof(Elf64_Rela);
                } else {
                    printf_log(LOG_NONE, "PLT Table type is unknown (size = 0x%x, type=%ld)\n", h->pltsz, h->pltrel);
                    FreeElfHeader(&h);
                    return NULL;
                }
                if((h->pltsz / h->pltent)*h->pltent != h->pltsz) {
                    printf_log(LOG_NONE, "PLT Table Entry size invalid (0x%x, ent=0x%x, type=%ld)\n", h->pltsz, h->pltent, h->pltrel);
                    FreeElfHeader(&h);
                    return NULL;
                }
                printf_log(LOG_DEBUG, "PLT Table @%p (type=%ld 0x%x/0x%0x)\n", (void*)h->jmprel, h->pltrel, h->pltsz, h->pltent);
            }
            if(h->DynStrTab && h->szDynStrTab) {
                //DumpDynamicNeeded(h); cannot dump now, it's not loaded yet
            }
        }
        // look for PLT Offset
        int ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".got.plt");
        if(ii) {
            h->gotplt = h->SHEntries[ii].sh_addr;
            h->gotplt_end = h->gotplt + h->SHEntries[ii].sh_size;
            printf_log(LOG_DEBUG, "The GOT.PLT Table is at address %p\n", (void*)h->gotplt);
        }
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".got");
        if(ii) {
            h->got = h->SHEntries[ii].sh_addr;
            h->got_end = h->got + h->SHEntries[ii].sh_size;
            printf_log(LOG_DEBUG, "The GOT Table is at address %p..%p\n", (void*)h->got, (void*)h->got_end);
        }
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".plt");
        if(ii) {
            h->plt = h->SHEntries[ii].sh_addr;
            h->plt_end = h->plt + h->SHEntries[ii].sh_size;
            printf_log(LOG_DEBUG, "The PLT Table is at address %p..%p\n", (void*)h->plt, (void*)h->plt_end);
        }
        // look for .init entry point
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".init");
        if(ii) {
            h->initentry = h->SHEntries[ii].sh_addr;
            printf_log(LOG_DEBUG, "The .init is at address %p\n", (void*)h->initentry);
        }
        // and .init_array
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".init_array");
        if(ii) {
            h->initarray_sz = h->SHEntries[ii].sh_size / sizeof(Elf64_Addr);
            h->initarray = (uintptr_t)(h->SHEntries[ii].sh_addr);
            printf_log(LOG_DEBUG, "The .init_array is at address %p, and have %d elements\n", (void*)h->initarray, h->initarray_sz);
        }
        // look for .fini entry point
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".fini");
        if(ii) {
            h->finientry = h->SHEntries[ii].sh_addr;
            printf_log(LOG_DEBUG, "The .fini is at address %p\n", (void*)h->finientry);
        }
        // and .fini_array
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".fini_array");
        if(ii) {
            h->finiarray_sz = h->SHEntries[ii].sh_size / sizeof(Elf64_Addr);
            h->finiarray = (uintptr_t)(h->SHEntries[ii].sh_addr);
            printf_log(LOG_DEBUG, "The .fini_array is at address %p, and have %d elements\n", (void*)h->finiarray, h->finiarray_sz);
        }
        // grab .text for main code
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".text");
        if(ii) {
            h->text = (uintptr_t)(h->SHEntries[ii].sh_addr);
            h->textsz = h->SHEntries[ii].sh_size;
            printf_log(LOG_DEBUG, "The .text is at address %p, and is %d big\n", (void*)h->text, h->textsz);
        }

        LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".dynstr", "DynSym Strings", SHT_STRTAB, (void**)&h->DynStr, NULL);
        LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".dynsym", "DynSym", SHT_DYNSYM, (void**)&h->DynSym, &h->numDynSym);
        if(box64_log>=LOG_DUMP && h->DynSym) DumpDynSym(h);
    }
    
    return h;
}
