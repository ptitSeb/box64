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
#ifndef DT_GNU_HASH
#define DT_GNU_HASH	0x6ffffef5
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
    *SH = box_calloc(1, s->sh_size);
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

void LoadNamedSection(FILE *f, Elf64_Shdr *s, int size, char* SHStrTab, const char* name, const char* clearname, uint32_t type, void** what, size_t* num)
{
    int n = FindSection(s, size, SHStrTab, name);
    printf_dump(LOG_DEBUG, "Loading %s (idx = %d)\n", clearname, n);
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
        if(strstr(name, ".so")) {
            // less naging on libs...
            printf_dump(LOG_DEBUG, "Not a 64bits ELF (%d)\n", header.e_ident[EI_CLASS]);
            return NULL;
        }
        if(header.e_ident[EI_CLASS]==ELFCLASS32) {
            printf_log(LOG_INFO, "This is a 32bits ELF! box64 can only run 64bits ELF (%s)!\n", name);
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

    elfheader_t *h = box_calloc(1, sizeof(elfheader_t));
    h->name = box_strdup(name);
    h->entrypoint = header.e_entry;
    h->numPHEntries = header.e_phnum;
    h->numSHEntries = header.e_shnum;
    h->SHIdx = header.e_shstrndx;
    h->e_type = header.e_type;
    // special cases for nums
    if(header.e_shentsize && !h->numSHEntries) {
        printf_dump(LOG_DEBUG, "Read number of Sections in 1st Section\n");
        // read 1st section header and grab actual number from here
        fseeko64(f, header.e_shoff, SEEK_SET);
        Elf64_Shdr section;
        if(fread(&section, sizeof(Elf64_Shdr), 1, f)!=1) {
            box_free(h);
            printf_log(LOG_INFO, "Cannot read Initial Section Header\n");
            return NULL;
        }
        h->numSHEntries = section.sh_size;
    }
    if(header.e_shentsize && h->numSHEntries) {
        // now read all section headers
        printf_dump(LOG_DEBUG, "Read %zu Section header\n", h->numSHEntries);
        h->SHEntries = (Elf64_Shdr*)box_calloc(h->numSHEntries, sizeof(Elf64_Shdr));
        fseeko64(f, header.e_shoff ,SEEK_SET);
        if(fread(h->SHEntries, sizeof(Elf64_Shdr), h->numSHEntries, f)!=h->numSHEntries) {
                FreeElfHeader(&h);
                printf_log(LOG_INFO, "Cannot read all Section Header\n");
                return NULL;
        }

        if(h->numPHEntries == PN_XNUM) {
            printf_dump(LOG_DEBUG, "Read number of Program Header in 1st Section\n");
            // read 1st section header and grab actual number from here
            h->numPHEntries = h->SHEntries[0].sh_info;
        }
    }

    printf_dump(LOG_DEBUG, "Read %zu Program header\n", h->numPHEntries);
    h->PHEntries = (Elf64_Phdr*)box_calloc(h->numPHEntries, sizeof(Elf64_Phdr));
    fseeko64(f, header.e_phoff ,SEEK_SET);
    if(fread(h->PHEntries, sizeof(Elf64_Phdr), h->numPHEntries, f)!=h->numPHEntries) {
            FreeElfHeader(&h);
            printf_log(LOG_INFO, "Cannot read all Program Header\n");
            return NULL;
    }

    if(header.e_shentsize && header.e_shnum) {
        if(h->SHIdx == SHN_XINDEX) {
            printf_dump(LOG_DEBUG, "Read number of String Table in 1st Section\n");
            h->SHIdx = h->SHEntries[0].sh_link;
        }
        if(h->SHIdx > h->numSHEntries) {
            printf_log(LOG_INFO, "Incoherent Section String Table Index : %zu / %zu\n", h->SHIdx, h->numSHEntries);
            FreeElfHeader(&h);
            return NULL;
        }
        // load Section table
        printf_dump(LOG_DEBUG, "Loading Sections Table String (idx = %zu)\n", h->SHIdx);
        if(LoadSH(f, h->SHEntries+h->SHIdx, (void*)&h->SHStrTab, ".shstrtab", SHT_STRTAB)) {
            FreeElfHeader(&h);
            return NULL;
        }
        if(box64_dump) DumpMainHeader(&header, h);

        LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".strtab", "SymTab Strings", SHT_STRTAB, (void**)&h->StrTab, NULL);
        LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".symtab", "SymTab", SHT_SYMTAB, (void**)&h->SymTab, &h->numSymTab);
        if(box64_dump && h->SymTab) DumpSymTab(h);

        LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".dynamic", "Dynamic", SHT_DYNAMIC, (void**)&h->Dynamic, &h->numDynamic);
        if(box64_dump && h->Dynamic) DumpDynamicSections(h);
        // grab DT_REL & DT_RELA stuffs
        // also grab the DT_STRTAB string table
        {
            for (size_t i=0; i<h->numDynamic; ++i) {
                Elf64_Dyn d = h->Dynamic[i];
                Elf64_Word val = d.d_un.d_val;
                Elf64_Addr ptr = d.d_un.d_ptr;
                switch (d.d_tag) {
                case DT_REL:
                    h->rel = ptr;
                    break;
                case DT_RELSZ:
                    h->relsz = val;
                    break;
                case DT_RELENT:
                    h->relent = val;
                    break;
                case DT_RELA:
                    h->rela = ptr;
                    break;
                case DT_RELASZ:
                    h->relasz = val;
                    break;
                case DT_RELAENT:
                    h->relaent = val;
                    break;
                case DT_PLTGOT:
                    h->pltgot = ptr;
                    break;
                case DT_PLTREL:
                    h->pltrel = val;
                    break;
                case DT_PLTRELSZ:
                    h->pltsz = val;
                    break;
                case DT_JMPREL:
                    h->jmprel = ptr;
                    break;
                case DT_STRTAB:
                    h->DynStrTab = (char*)(ptr);
                    break;
                case DT_STRSZ:
                    h->szDynStrTab = val;
                    break;
                case DT_INIT: // Entry point
                    h->initentry = ptr;
                    printf_dump(LOG_DEBUG, "The DT_INIT is at address %p\n", (void*)h->initentry);
                    break;
                case DT_INIT_ARRAY:
                    h->initarray = ptr;
                    printf_dump(LOG_DEBUG, "The DT_INIT_ARRAY is at address %p\n", (void*)h->initarray);
                    break;
                case DT_INIT_ARRAYSZ:
                    h->initarray_sz = val / sizeof(Elf64_Addr);
                    printf_dump(LOG_DEBUG, "The DT_INIT_ARRAYSZ is %zu\n", h->initarray_sz);
                    break;
                case DT_PREINIT_ARRAYSZ:
                    #ifndef ANDROID
                        if(val)
                            printf_log(LOG_NONE, "Warning, PreInit Array (size=%d) present and ignored!\n", val);
                    #endif
                    break;
                case DT_FINI: // Exit hook
                    h->finientry = ptr;
                    printf_dump(LOG_DEBUG, "The DT_FINI is at address %p\n", (void*)h->finientry);
                    break;
                case DT_FINI_ARRAY:
                    h->finiarray = ptr;
                    printf_dump(LOG_DEBUG, "The DT_FINI_ARRAY is at address %p\n", (void*)h->finiarray);
                    break;
                case DT_FINI_ARRAYSZ:
                    h->finiarray_sz = val / sizeof(Elf64_Addr);
                    printf_dump(LOG_DEBUG, "The DT_FINI_ARRAYSZ is %zu\n", h->finiarray_sz);
                    break;
                case DT_VERNEEDNUM:
                    h->szVerNeed = val;
                    printf_dump(LOG_DEBUG, "The DT_VERNEEDNUM is %d\n", h->szVerNeed);
                    break;
                case DT_VERNEED:
                    h->VerNeed = (Elf64_Verneed*)ptr;
                    printf_dump(LOG_DEBUG, "The DT_VERNEED is at address %p\n", h->VerNeed);
                    break;
                case DT_VERDEFNUM:
                    h->szVerDef = val;
                    printf_dump(LOG_DEBUG, "The DT_VERDEFNUM is %d\n", h->szVerDef);
                    break;
                case DT_VERDEF:
                    h->VerDef = (Elf64_Verdef*)ptr;
                    printf_dump(LOG_DEBUG, "The DT_VERDEF is at address %p\n", h->VerDef);
                    break;
                case DT_FLAGS:
                    h->flags = val;
                    printf_dump(LOG_DEBUG, "The DT_FLAGS is 0x%x\n", h->flags);
                    break;
                case DT_HASH:
                    h->hash = ptr;
                    printf_dump(LOG_DEBUG, "The DT_HASH is at address %p\n", (void*)h->hash);
                    break;
                case DT_GNU_HASH:
                    h->gnu_hash = ptr;
                    printf_dump(LOG_DEBUG, "The DT_GNU_HASH is at address %p\n", (void*)h->gnu_hash);
                    break;
                }
            }
            if(h->rel) {
                if(h->relent != sizeof(Elf64_Rel)) {
                    printf_log(LOG_NONE, "Rel Table Entry size invalid (0x%x should be 0x%zx)\n", h->relent, sizeof(Elf64_Rel));
                    FreeElfHeader(&h);
                    return NULL;
                }
                printf_dump(LOG_DEBUG, "Rel Table @%p (0x%zx/0x%x)\n", (void*)h->rel, h->relsz, h->relent);
            }
            if(h->rela) {
                if(h->relaent != sizeof(Elf64_Rela)) {
                    printf_log(LOG_NONE, "RelA Table Entry size invalid (0x%x should be 0x%zx)\n", h->relaent, sizeof(Elf64_Rela));
                    FreeElfHeader(&h);
                    return NULL;
                }
                printf_dump(LOG_DEBUG, "RelA Table @%p (0x%zx/0x%x)\n", (void*)h->rela, h->relasz, h->relaent);
            }
            if(h->jmprel) {
                if(h->pltrel == DT_REL) {
                    h->pltent = sizeof(Elf64_Rel);
                } else if(h->pltrel == DT_RELA) {
                    h->pltent = sizeof(Elf64_Rela);
                } else {
                    printf_log(LOG_NONE, "PLT Table type is unknown (size = 0x%zx, type=%ld)\n", h->pltsz, h->pltrel);
                    FreeElfHeader(&h);
                    return NULL;
                }
                if((h->pltsz / h->pltent)*h->pltent != h->pltsz) {
                    printf_log(LOG_NONE, "PLT Table Entry size invalid (0x%zx, ent=0x%x, type=%ld)\n", h->pltsz, h->pltent, h->pltrel);
                    FreeElfHeader(&h);
                    return NULL;
                }
                printf_dump(LOG_DEBUG, "PLT Table @%p (type=%ld 0x%zx/0x%0x)\n", (void*)h->jmprel, h->pltrel, h->pltsz, h->pltent);
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
            printf_dump(LOG_DEBUG, "The GOT.PLT Table is at address %p\n", (void*)h->gotplt);
        }
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".got");
        if(ii) {
            h->got = h->SHEntries[ii].sh_addr;
            h->got_end = h->got + h->SHEntries[ii].sh_size;
            printf_dump(LOG_DEBUG, "The GOT Table is at address %p..%p\n", (void*)h->got, (void*)h->got_end);
        }
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".plt");
        if(ii) {
            h->plt = h->SHEntries[ii].sh_addr;
            h->plt_end = h->plt + h->SHEntries[ii].sh_size;
            printf_dump(LOG_DEBUG, "The PLT Table is at address %p..%p\n", (void*)h->plt, (void*)h->plt_end);
        }
        // grab version of symbols
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".gnu.version");
        if(ii) {
            h->VerSym = (Elf64_Half*)(h->SHEntries[ii].sh_addr);
            printf_dump(LOG_DEBUG, "The .gnu.version is at address %p\n", h->VerSym);
        }
        // grab .text for main code
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".text");
        if(ii) {
            h->text = (uintptr_t)(h->SHEntries[ii].sh_addr);
            h->textsz = h->SHEntries[ii].sh_size;
            printf_dump(LOG_DEBUG, "The .text is at address %p, and is %zu big\n", (void*)h->text, h->textsz);
        }
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".eh_frame");
        if(ii) {
            h->ehframe = (uintptr_t)(h->SHEntries[ii].sh_addr);
            h->ehframe_end = h->ehframe + h->SHEntries[ii].sh_size;
            printf_dump(LOG_DEBUG, "The .eh_frame section is at address %p..%p\n", (void*)h->ehframe, (void*)h->ehframe_end);
        }
        ii = FindSection(h->SHEntries, h->numSHEntries, h->SHStrTab, ".eh_frame_hdr");
        if(ii) {
            h->ehframehdr = (uintptr_t)(h->SHEntries[ii].sh_addr);
            printf_dump(LOG_DEBUG, "The .eh_frame_hdr section is at address %p\n", (void*)h->ehframehdr);
        }

        LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".dynstr", "DynSym Strings", SHT_STRTAB, (void**)&h->DynStr, NULL);
        LoadNamedSection(f, h->SHEntries, h->numSHEntries, h->SHStrTab, ".dynsym", "DynSym", SHT_DYNSYM, (void**)&h->DynSym, &h->numDynSym);
    }
    
    return h;
}

const char* BindSym(int bind)
{
    switch(bind) {
        case STB_GLOBAL: return "STB_GLOBAL";
        case STB_WEAK: return "STB_WEAK";
        case STB_LOCAL: return "STB_LOCAL";
        case STB_GNU_UNIQUE: return "STB_GNU_UNIQUE";
    }
    static char tmp[50];
    sprintf(tmp, "??? 0x%x", bind);
    return tmp;
}