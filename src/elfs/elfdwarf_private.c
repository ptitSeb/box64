#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "debug.h"
#include "elfloader_private.h"
#include "elfdwarf_private.h"

typedef struct dwarf_unwind_constr_s {
    uint8_t reg_count;
    uint64_t *table;
    uint8_t *statuses;
#define GET_STATUS(n,i) (((n).statuses[(i) >> 1] >> (((i) & 1) << 2)) & 0xF)
#define SET_STATUS(n,i,v) (n).statuses[(i) >> 1] = ((n).statuses[(i) >> 1] & (0xF0 >> (((i) & 1) << 2))) | (v << (((i) & 1) << 2))
#define REGSTATUS_undefined     0b0000
#define REGSTATUS_same_val      0b0101
#define REGSTATUS_offset        0b0010
#define REGSTATUS_val_offset    0b0011
#define REGSTATUS_register      0b0100
} dwarf_unwind_constr_t;

#define DW_EH_PE_uptr       0x00
#define DW_EH_PE_uleb128    0x01
#define DW_EH_PE_udata2     0x02
#define DW_EH_PE_udata4     0x03
#define DW_EH_PE_udata8     0x04
#define DW_EH_PE_SIZE_MASK  0x07
#define DW_EH_PE_signed     0x08
#define DW_EH_PE_absptr     0x00
#define DW_EH_PE_pcrel      0x10
#define DW_EH_PE_textrel    0x20 // Unsupported
#define DW_EH_PE_datarel    0x30 // Unsupported
#define DW_EH_PE_indirect   0x80 // May crash
#define DW_EH_PE_omit       0xff

#define READ_U1(var, ptr) do { (var) = *( uint8_t*)(ptr); (ptr) += 1; } while (0)
#define READ_S1(var, ptr) do { (var) = *(  int8_t*)(ptr); (ptr) += 1; } while (0)
#define READ_U2(var, ptr) do { (var) = *(uint16_t*)(ptr); (ptr) += 2; } while (0)
#define READ_S2(var, ptr) do { (var) = *( int16_t*)(ptr); (ptr) += 2; } while (0)
#define READ_U4(var, ptr) do { (var) = *(uint32_t*)(ptr); (ptr) += 4; } while (0)
#define READ_S4(var, ptr) do { (var) = *( int32_t*)(ptr); (ptr) += 4; } while (0)
#define READ_U8(var, ptr) do { (var) = *(uint64_t*)(ptr); (ptr) += 8; } while (0)
#define READ_S8(var, ptr) do { (var) = *( int64_t*)(ptr); (ptr) += 8; } while (0)
#define READ_ULEB128(var, ptr) readULEB(&(var), &(ptr))
#define READ_SLEB128(var, ptr) readSLEB((int64_t*)&(var), &(ptr))
// Seems like DW_EH_PE_signed is actually an invalid encoding
#define READ_ENCODED(var, ptr, enc, isptr) do { uintptr_t optr = (uintptr_t)(ptr); \
         if ((((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_uleb128) &&  ((enc) & DW_EH_PE_signed)) READ_SLEB128(var, ptr); \
    else if ((((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_uleb128) && !((enc) & DW_EH_PE_signed)) READ_ULEB128(var, ptr); \
    else if ((((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_udata2 ) &&  ((enc) & DW_EH_PE_signed)) READ_S2     (var, ptr); \
    else if ((((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_udata2 ) && !((enc) & DW_EH_PE_signed)) READ_U2     (var, ptr); \
    else if ((((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_udata4 ) &&  ((enc) & DW_EH_PE_signed)) READ_S4     (var, ptr); \
    else if ((((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_udata4 ) && !((enc) & DW_EH_PE_signed)) READ_U4     (var, ptr); \
    else if ((((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_udata8 ) &&  ((enc) & DW_EH_PE_signed)) READ_S8     (var, ptr); \
    else if ((((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_udata8 ) && !((enc) & DW_EH_PE_signed)) READ_U8     (var, ptr); \
    else if ((((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_uptr   ) &&  ((enc) & DW_EH_PE_signed)) READ_S8     (var, ptr); \
    else if ((((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_uptr   ) && !((enc) & DW_EH_PE_signed)) READ_U8     (var, ptr); \
    else { printf_log(LOG_DEBUG, "Invalid encoding 0x%02X\n", (enc)); (var) = 0; } \
    if (((enc) & DW_EH_PE_pcrel) && isptr) (var) += optr; \
    if ((enc) & DW_EH_PE_indirect) (var) = *(uint64_t*)(var); \
} while (0)
#define SKIP_1(ptr) (ptr) += 1
#define SKIP_2(ptr) (ptr) += 2
#define SKIP_4(ptr) (ptr) += 4
#define SKIP_8(ptr) (ptr) += 8
#define SKIP_LEB128(ptr) do { ++(ptr); } while (((*(((unsigned char*)(ptr))-1))) & (1 << 7))
#define SKIP_ENCODED(ptr, enc) \
    do { if (((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_uleb128) SKIP_LEB128(ptr); \
    else if (((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_udata2 ) SKIP_2     (ptr); \
    else if (((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_udata4 ) SKIP_4     (ptr); \
    else if (((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_udata8 ) SKIP_8     (ptr); \
    else if (((enc) & DW_EH_PE_SIZE_MASK) == DW_EH_PE_uptr   ) SKIP_8     (ptr); \
    else printf_log(LOG_DEBUG, "Invalid encoding 0x%02X\n", (enc)); \
} while (0)

// LEB128 wil only work on 64 bits numbers (unsigned long, like the Linux kernel)
void readULEB(uint64_t *var, unsigned char **ptr) {
    *var = 0;
    int shift = 0;
    while (1) {
        unsigned char byte = **ptr; ++*ptr;
        *var |= ((byte & 0x7F) << shift);
        if (!(byte & (1 << 7))) break;
        shift += 7;
    }
}
void readSLEB(int64_t *var, unsigned char **ptr) {
    uint64_t tmp = 0;
    int shift = 0;
    unsigned char byte;
    while (1) {
        byte = **ptr; ++*ptr;
        tmp |= ((byte & 0x7F) << shift);
        if (!(byte & (1 << 7))) break;
        shift += 7;
    }
    // Sign extend
    if (byte & (1 << 6)) {
        *(uint64_t*)var = tmp | (0xFFFFFFFFFFFFFFFFull << (shift + 7));
    } else {
        *var = tmp;
    }
}

uintptr_t get_parent_registers(dwarf_unwind_t *unwind, const elfheader_t *ehdr, uintptr_t addr, char *success) {
    if (!ehdr) {
        *success = 0;
        return 0;
    }
    if(!IsAddressInElfSpace(ehdr, ehdr->ehframehdr+ehdr->delta)) {
        *success = 0;
        return 0;
    }
    unsigned char ehfh_version = *(unsigned char*)(ehdr->ehframehdr+ehdr->delta);
    if (ehfh_version != 1) {
        *success = 0;
        return 0;
    }

    // printf_log(LOG_NONE, "Address: 0x%016lX\nEH frame address: 0x%016lX\n", addr, ehdr->ehframe);

    // Not using the binary search table (for now)

    unsigned char *cur_addr = (unsigned char*)(ehdr->ehframe+ehdr->delta);
    unsigned char *end_addr = (unsigned char*)(ehdr->ehframe_end+ehdr->delta);

#define AUG_EHDATA      (1 << 0)
#define AUG_AUGDATA     (1 << 1)
#define AUG_LSDA        (1 << 2)
#define AUG_PARG        (1 << 3)
#define AUG_FDEENC      (1 << 4)
#define AUG_SIGHDLER    (1 << 5)
    unsigned short aug_fields = 0;
    unsigned char lsda_encoding = DW_EH_PE_omit;
    unsigned char fde_encoding  = DW_EH_PE_omit;
    uint64_t eh_data = 0;               // Note: these are never reset, so use aug_fields & AUG_EHDATA
    uint64_t code_alignment_factor = 0;
    int64_t data_alignment_factor = 0;
    uint64_t cie_aug_data_len = 0;      // Note: these are never reset, so use aug_fields & AUG_AUGDATA
    unsigned char *cie_aug_data = NULL; // Note: these are never reset, so use aug_fields & AUG_AUGDATA
    unsigned char *initCIEinstr = NULL; // Never NULL if initialized
    unsigned char *endCIEinstr = NULL;  // Never NULL if initialized
    uint64_t return_addr_reg = 0;       // Description absent in the LSBCS 5.0, but still present
    while (cur_addr < end_addr) {
        // Length
        uint64_t len; int extended = 0;
        READ_U4(len, cur_addr);
        if (!len) {
            // Terminator, end parsing
            *success = 0;
            return 0;
        } else if (len == 0xFFFFFFFF) {
            // Extended Length (optional)
            extended = 1;
            READ_U8(len, cur_addr);
        }
        (void)extended;
        // printf_log(LOG_NONE, "[???] Length          %02X\n", len);
        // printf_log(LOG_NONE, "[???] Extended        %c\n", extended ? 'Y' : 'N');
        unsigned char *next_addr = cur_addr + len;

        // CIE ID (always 0) or CIE Pointer (never 0)
        uint32_t cie_ptr;
        READ_U4(cie_ptr, cur_addr);
        if (cie_ptr == 0) {
            // Current block is a CIE
            // printf_log(LOG_NONE, "[CIE] ID              %02X\n", cie_ptr);

            // Version (always 1 or 3)
            uint8_t cie_version;
            READ_U1(cie_version, cur_addr);
            // printf_log(LOG_NONE, "[CIE] Version         %01X\n", cie_version);
            if ((cie_version != 1) && (cie_version != 3)) { // Only 1 is in the LSBCS 5.0 but I found 3 exists as well
                // Invalid CIE version
                printf_log(LOG_DEBUG, "Invalid CIE version %d (should be 1 or 3), stopping parsing\n", cie_version);
                // Failed to process CIE
                *success = 0;
                return 0;
            }

            // Augmentation String
            char *aug_str = (char*)cur_addr; cur_addr += strlen(aug_str) + 1;
            // printf_log(LOG_NONE, "[CIE] Augmentation st %s\n", aug_str);
            aug_fields = (aug_str[0] == 'z') ? AUG_AUGDATA : 0;
            for (char *aug_str2 = aug_str + ((aug_fields & AUG_AUGDATA) ? 1 : 0); *aug_str2; ++aug_str2) {
                if ((aug_str2[0] == 'e') && (aug_str2[1] == 'h')) {
                    // Use has also been left untold in the Linux Standard Base Core Specification 3.0RC1
                    printf_log(LOG_DEBUG, "Warning: EH data detected but this was removed from the standard\n");
                    aug_fields |= AUG_EHDATA; ++aug_str2;
                } else if (aug_fields & AUG_AUGDATA) {
                    if (aug_str2[0] == 'L') {
                        aug_fields |= AUG_LSDA;
                    } else if (aug_str2[0] == 'P') {
                        aug_fields |= AUG_PARG;
                        printf_log(LOG_DEBUG, "Warning: augmentation string attribute 'P' unsupported\n");
                    } else if (aug_str2[0] == 'S') {
                        aug_fields |= AUG_SIGHDLER;
                        printf_log(LOG_DEBUG, "Warning: augmentation string attribute 'S' ignored\n");
                    } else if (aug_str2[0] == 'R') {
                        aug_fields |= AUG_FDEENC;
                    } else {
                        printf_log(LOG_DEBUG, "Error: invalid augmentation string %s\n", aug_str);
                        // Failed to process augmentation string
                        break;
                    }
                } else {
                    printf_log(LOG_DEBUG, "Error: invalid augmentation string %s\n", aug_str);
                    // Failed to process augmentation string
                    break;
                }
            }
            // printf_log(LOG_NONE, "[CIE] Augmentation fs %02x\n", aug_fields);

            // Code Alignment Factor
            READ_ULEB128(code_alignment_factor, cur_addr);
            // printf_log(LOG_NONE, "[CIE] Code AF         %d\n", code_alignment_factor);
            // Data Alignment Factor
            READ_SLEB128(data_alignment_factor, cur_addr);
            // printf_log(LOG_NONE, "[CIE] Data AF         %d\n", data_alignment_factor);

            if (cie_version == 1) {
                READ_U1(return_addr_reg, cur_addr);
            } else {
                READ_ULEB128(return_addr_reg, cur_addr);
            }
            // printf_log(LOG_NONE, "[CIE] Return addr reg %01X\n", return_addr_reg);

            lsda_encoding = DW_EH_PE_omit;
            fde_encoding  = DW_EH_PE_absptr;
            if (aug_fields & AUG_AUGDATA) {
                // Augmentation Data Length (optional)
                READ_ULEB128(cie_aug_data_len, cur_addr);
                // printf_log(LOG_NONE, "[CIE] Aug data len    %01X\n", cie_aug_data_len);
                // Augmentation Data (optional)
                cie_aug_data = cur_addr; cur_addr += cie_aug_data_len;
                char *cie_aug_data2 = (char*)cie_aug_data;
                for (char *aug_str2 = aug_str + ((aug_fields & AUG_AUGDATA) ? 1 : 0); *aug_str2; ++aug_str2) {
                    if ((aug_str2[0] == 'e') && (aug_str2[1] == 'h')) {
                        break; // Unknown usage
                    } else if (aug_fields & AUG_AUGDATA) {
                        if (aug_str2[0] == 'L') {
                            READ_U1(lsda_encoding, cie_aug_data2);
                            if (lsda_encoding != DW_EH_PE_omit) printf_log(LOG_DEBUG, "Warning: LSDA unsupported\n");
                        } else if (aug_str2[0] == 'P') {
                            unsigned char pencoding;
                            READ_U1(pencoding, cie_aug_data2);
                            SKIP_ENCODED(cie_aug_data2, pencoding);
                        } else if (aug_str2[0] == 'S') {
                        } else if (aug_str2[0] == 'R') {
                            READ_U1(fde_encoding, cie_aug_data2);
                            // printf_log(LOG_NONE, "[CIE] FDE encoding    %02X\n", fde_encoding);
                            if (fde_encoding == DW_EH_PE_omit) printf_log(LOG_DEBUG, "Error: FDE encoding set to 'omit'\n");
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
        
            // Initial Instructions
            initCIEinstr = cur_addr;
            endCIEinstr = next_addr;
        } else if (initCIEinstr) {
            // Current block is a FDE
            // printf_log(LOG_NONE, "[FDE] Ptr             %02X\n", cie_ptr);

            // PC Begin
            uintptr_t pc_begin;
            READ_ENCODED(pc_begin, cur_addr, fde_encoding, 1);
            // printf_log(LOG_NONE, "[FDE] PC begin        %016lX\n", pc_begin);
            // PC Range
            uint64_t pc_range;
            READ_ENCODED(pc_range, cur_addr, fde_encoding, 0);
            // printf_log(LOG_NONE, "[FDE] PC end          %016lX\n", pc_begin + pc_range);
            
            uint64_t aug_data_len;
            unsigned char *aug_data;
            if (aug_fields & AUG_AUGDATA) {
                // Augmentation Data Length (optional)
                READ_ULEB128(aug_data_len, cur_addr);
                // printf_log(LOG_NONE, "[FDE] Aug data len    %01X\n", aug_data_len);
                // Augmentation Data (optional)
                aug_data = cur_addr; cur_addr += aug_data_len;
                (void)aug_data;
            }

            if (lsda_encoding != DW_EH_PE_omit) {
                SKIP_ENCODED(cur_addr, lsda_encoding);
            }

            // Call Frame Instructions
            if ((pc_begin <= addr) && (addr < pc_begin + pc_range)) {
                // Corresponding Frame Descriptor Entry found!

#define setmax(a, b) a = (a < (b) ? (b) : a)
                dwarf_unwind_constr_t unwind_constr;
                unwind_constr.reg_count = return_addr_reg;
                uint64_t maxstacksize = 0;
                uint64_t curstacksize = 0;
                unsigned char *cur_inst = initCIEinstr;
                uint64_t nreg;
#define PARSE_INST \
        switch (inst >> 6) {                                        \
        case 0b01:                                                  \
            break;                                                  \
        case 0b10:                                                  \
            setmax(unwind_constr.reg_count, inst & 0x3F);           \
            SKIP_LEB128(cur_inst);                                  \
            break;                                                  \
        case 0b11:                                                  \
            setmax(unwind_constr.reg_count, inst & 0x3F);           \
            break;                                                  \
        case 0b00:                                                  \
            switch (inst & 0x3F) {                                  \
            case 0b000000:                                          \
                break;                                              \
            case 0b001010:                                          \
                ++curstacksize;                                     \
                setmax(maxstacksize, curstacksize);                 \
                break;                                              \
            case 0b001011:                                          \
                if (!curstacksize) {                                \
                    printf_log(LOG_DEBUG, "Negative stack size during CFI execution\n"); \
                    *success = 0;                                   \
                    return 0;                                       \
                }                                                   \
                --curstacksize;                                     \
                break;                                              \
            case 0b000010:                                          \
                SKIP_1(cur_inst);                                   \
                break;                                              \
            case 0b000011:                                          \
                SKIP_2(cur_inst);                                   \
                break;                                              \
            case 0b000100:                                          \
                SKIP_4(cur_inst);                                   \
                break;                                              \
            case 0b000001:                                          \
                SKIP_8(cur_inst);                                   \
                break;                                              \
            case 0b001110:                                          \
            case 0b010011:                                          \
                SKIP_LEB128(cur_inst);                              \
                break;                                              \
            case 0b000110:                                          \
            case 0b000111:                                          \
            case 0b001000:                                          \
            case 0b001101:                                          \
                READ_ULEB128(nreg, cur_inst);                       \
                setmax(unwind_constr.reg_count, nreg);              \
                break;                                              \
            case 0b000101:                                          \
            case 0b001100:                                          \
            case 0b010001:                                          \
            case 0b010010:                                          \
            case 0b010100:                                          \
            case 0b010101:                                          \
                READ_ULEB128(nreg, cur_inst);                       \
                setmax(unwind_constr.reg_count, nreg);              \
                SKIP_LEB128(cur_inst);                              \
                break;                                              \
            case 0b001001:                                          \
                READ_ULEB128(nreg, cur_inst);                       \
                setmax(unwind_constr.reg_count, nreg);              \
                READ_ULEB128(nreg, cur_inst);                       \
                setmax(unwind_constr.reg_count, nreg);              \
                break;                                              \
            default:                                                \
                /* Contains undefined, user and expression CFIs */  \
                printf_log(LOG_DEBUG, "Unknown CFI 0x%02X\n", inst); \
                *success = 0;                                       \
                return 0;                                        \
            }                                                       \
            break;                                                  \
        }
                while (cur_inst < endCIEinstr) {
                    unsigned char inst; READ_U1(inst, cur_inst);
                    PARSE_INST
                }
                cur_inst = cur_addr;
                while (cur_inst < next_addr) {
                    unsigned char inst; READ_U1(inst, cur_inst);
                    PARSE_INST
                }
#undef PARSE_INST

                uint64_t cfa_reg = -1;
                unsigned char cfa_signed = 0;
                union { uint64_t uoff; int64_t soff; } cfa_offset = { .uoff = 0 };
                ++unwind_constr.reg_count;
                size_t tablelen = unwind_constr.reg_count * sizeof(uint64_t);
                size_t statuseslen = ((unwind_constr.reg_count+1) >> 1) * sizeof(uint8_t);
                unwind_constr.table = (uint64_t*)box_malloc(tablelen);
                unwind_constr.statuses = (uint8_t*)box_calloc((unwind_constr.reg_count+1) >> 1, sizeof(uint8_t));
                // ~~undefined is 0: no initialization needed~~ still initialize the first 17 to same_val
                for (int i = 0; (i < 17) && (i <= unwind_constr.reg_count); ++i) {
                    SET_STATUS(unwind_constr, i, REGSTATUS_same_val);
                }

                curstacksize = 0;
                uint64_t **table_stack = (uint64_t**)box_malloc(maxstacksize * sizeof(uint64_t*));
                for (uint64_t i = 0; i < maxstacksize; ++i) {
                    table_stack[i] = (uint64_t*)box_malloc(tablelen);
                }
                cur_inst = initCIEinstr;
                uintptr_t cur_pointed_addr = pc_begin;
                // Missing:
                /* DW_CFA_def_cfa_expression    0   0x0f      BLOCK                              */
                /* DW_CFA_expression            0   0x10      ULEB128 register  BLOCK            */
                /* DW_CFA_val_expression        0   0x16      ULEB128 BLOCK                      */
                /* DW_CFA_lo_user               0   0x1c                                         */
                /* DW_CFA_GNU_args_size         0   0x2e      ULEB128 argsize                    */
                /* DW_CFA_GNU_negative_offset_extended 0 0x2f ULEB128 register  ULEB128 offset (obsoleted by DW_CFA_offset_extended_sf) */
                /* DW_CFA_hi_user               0   0x3f                                         */
                // Known "bug": DW_CFA_set_loc can go backwards, this is ignored
                uint64_t tmpreg;
                uint64_t tmpuval;
                int64_t tmpsval;
#define PARSE_INST \
        switch (inst >> 6) {                                                                                    \
        case 0b01:         /* DW_CFA_advance_loc           0x1 delta                                        */  \
            cur_pointed_addr += (inst & 0x3F) * code_alignment_factor;                                          \
            break;                                                                                              \
        case 0b10:         /* DW_CFA_offset                0x2 register  ULEB128 offset                     */  \
            READ_ULEB128(tmpuval, cur_inst);                                                                    \
            SET_STATUS(unwind_constr, inst & 0x3F, REGSTATUS_offset);                                           \
            unwind_constr.table[inst & 0x3F] = (int64_t)tmpuval * data_alignment_factor;                        \
            break;                                                                                              \
        case 0b11:         /* DW_CFA_restore               0x3 register                                     */  \
            RESTORE_REG(inst & 0x3F)                                                                            \
            break;                                                                                              \
        case 0b00:                                                                                              \
            switch (inst & 0x3F) {                                                                              \
            case 0b000001: /* DW_CFA_set_loc               0   0x01      address                            */  \
                READ_U8(cur_pointed_addr, cur_inst);                                                            \
                break;                                                                                          \
            case 0b000010: /* DW_CFA_advance_loc1          0   0x02      1-byte delta                       */  \
                READ_U1(tmpuval, cur_inst);                                                                     \
                cur_pointed_addr += tmpuval * code_alignment_factor;                                            \
                break;                                                                                          \
            case 0b000011: /* DW_CFA_advance_loc2          0   0x03      2-byte delta                       */  \
                READ_U2(tmpuval, cur_inst);                                                                     \
                cur_pointed_addr += tmpuval * code_alignment_factor;                                            \
                break;                                                                                          \
            case 0b000100: /* DW_CFA_advance_loc4          0   0x04      4-byte delta                       */  \
                READ_U4(tmpuval, cur_inst);                                                                     \
                cur_pointed_addr += tmpuval * code_alignment_factor;                                            \
                break;                                                                                          \
            \
            case 0b001100: /* DW_CFA_def_cfa               0   0x0c      ULEB128 register  ULEB128 offset   */  \
                READ_ULEB128(cfa_reg, cur_inst);                                                                \
                cfa_signed = 0;                                                                                 \
                READ_ULEB128(cfa_offset.uoff, cur_inst);                                                        \
                break;                                                                                          \
            case 0b010010: /* DW_CFA_def_cfa_sf            0   0x12      ULEB128 register  SLEB128 offset   */  \
                READ_ULEB128(cfa_reg, cur_inst);                                                                \
                cfa_signed = 1;                                                                                 \
                READ_SLEB128(cfa_offset.soff, cur_inst);                                                        \
                cfa_offset.soff *= data_alignment_factor;                                                       \
                break;                                                                                          \
            case 0b001101: /* DW_CFA_def_cfa_register      0   0x0d      ULEB128 register                   */  \
                READ_ULEB128(cfa_reg, cur_inst);                                                                \
                break;                                                                                          \
            case 0b001110: /* DW_CFA_def_cfa_offset        0   0x0e      ULEB128 offset                     */  \
                cfa_signed = 0;                                                                                 \
                READ_ULEB128(cfa_offset.uoff, cur_inst);                                                        \
                break;                                                                                          \
            case 0b010011: /* DW_CFA_def_cfa_offset_sf     0   0x13      SLEB128 offset                     */  \
                READ_SLEB128(cfa_offset.soff, cur_inst);                                                        \
                cfa_offset.soff *= data_alignment_factor;                                                       \
                break;                                                                                          \
            /* DW_CFA_def_cfa_expression */ \
            \
            case 0b000111: /* DW_CFA_undefined             0   0x07      ULEB128 register                   */  \
                READ_ULEB128(tmpreg, cur_inst);                                                                 \
                SET_STATUS(unwind_constr, tmpreg, REGSTATUS_undefined);                                         \
                break;                                                                                          \
            case 0b001000: /* DW_CFA_same_value            0   0x08      ULEB128 register                   */  \
                READ_ULEB128(tmpreg, cur_inst);                                                                 \
                SET_STATUS(unwind_constr, tmpreg, REGSTATUS_same_val);                                          \
                break;                                                                                          \
            case 0b000101: /* DW_CFA_offset_extended       0   0x05      ULEB128 register  ULEB128 offset   */  \
                READ_ULEB128(tmpreg, cur_inst);                                                                 \
                READ_ULEB128(tmpuval, cur_inst);                                                                \
                SET_STATUS(unwind_constr, tmpreg, REGSTATUS_offset);                                            \
                unwind_constr.table[tmpreg] = (int64_t)tmpuval * data_alignment_factor;                         \
                break;                                                                                          \
            case 0b010001: /* DW_CFA_offset_extended_sf    0   0x11      ULEB128 register  SLEB128 offset   */  \
                READ_ULEB128(tmpreg, cur_inst);                                                                 \
                READ_SLEB128(tmpsval, cur_inst);                                                                \
                SET_STATUS(unwind_constr, tmpreg, REGSTATUS_offset);                                            \
                unwind_constr.table[tmpreg] = tmpsval * data_alignment_factor;                                  \
                break;                                                                                          \
            case 0b010100: /* DW_CFA_val_offset            0   0x14      ULEB128 register  ULEB128 offset   */  \
                READ_ULEB128(tmpreg, cur_inst);                                                                 \
                READ_ULEB128(tmpuval, cur_inst);                                                                \
                SET_STATUS(unwind_constr, tmpreg, REGSTATUS_val_offset);                                        \
                unwind_constr.table[tmpreg] = (int64_t)tmpuval * data_alignment_factor;                         \
                break;                                                                                          \
            case 0b010101: /* DW_CFA_val_offset_sf         0   0x15      ULEB128 register  SLEB128 offset   */  \
                READ_ULEB128(tmpreg, cur_inst);                                                                 \
                READ_SLEB128(tmpsval, cur_inst);                                                                \
                SET_STATUS(unwind_constr, tmpreg, REGSTATUS_val_offset);                                        \
                unwind_constr.table[tmpreg] = tmpsval * data_alignment_factor;                                  \
                break;                                                                                          \
            case 0b001001: /* DW_CFA_register              0   0x09      ULEB128 register  ULEB128 register */  \
                READ_ULEB128(tmpreg, cur_inst);                                                                 \
                READ_ULEB128(tmpuval, cur_inst);                                                                \
                SET_STATUS(unwind_constr, tmpreg, REGSTATUS_register);                                          \
                unwind_constr.table[tmpreg] = tmpuval;                                                          \
                break;                                                                                          \
            /* DW_CFA_expression */ \
            /* DW_CFA_val_expression */ \
            case 0b000110: /* DW_CFA_restore_extended      0   0x06      ULEB128 register                   */  \
                READ_ULEB128(tmpreg, cur_inst);                                                                 \
                RESTORE_REG(tmpreg)                                                                             \
                break;                                                                                          \
            \
            case 0b001010: /* DW_CFA_remember_state        0   0x0a                                         */  \
                memcpy(table_stack[curstacksize], unwind_constr.table, tablelen);                               \
                ++curstacksize;                                                                                 \
                break;                                                                                          \
            case 0b001011: /* DW_CFA_restore_state         0   0x0b                                         */  \
                --curstacksize;                                                                                 \
                memcpy(unwind_constr.table, table_stack[curstacksize], tablelen);                               \
                break;                                                                                          \
            \
            case 0b000000: /* DW_CFA_nop                   0   0                                            */  \
                break;                                                                                          \
            default:                                                                                            \
                /* Contains undefined, user and expression CFIs */                                              \
                printf_log(LOG_DEBUG, "Unknown CFI 0x%02X\n", inst);                                             \
                FAILED                                                                                          \
            }                                                                                                   \
            break;                                                                                              \
        }
#define RESTORE_REG(reg) \
    printf_log(LOG_DEBUG, "Trying to restore register 0x%02lX while in the initial CFIs\n", (uint64_t)reg); \
    FAILED
#define FAILED \
    box_free(unwind_constr.statuses);                                       \
    box_free(unwind_constr.table);                                          \
    for (uint64_t i = 0; i < maxstacksize; ++i) box_free(table_stack[i]);   \
    box_free(table_stack);                                                  \
    *success = 0; return 0;
                while (cur_inst < endCIEinstr) {
                    unsigned char inst; READ_U1(inst, cur_inst);
                    // printf_log(LOG_NONE, "Executing pre 0x%02X\n", inst);
                    PARSE_INST
                }
#undef FAILED
#undef RESTORE_REG
                uint64_t *init_table = (uint64_t*)box_malloc(tablelen);
                memcpy(init_table, unwind_constr.table, tablelen);
                uint8_t *init_statuses = (uint8_t*)box_malloc(statuseslen);
                memcpy(init_statuses, unwind_constr.statuses, statuseslen);
                cur_inst = cur_addr;
#define RESTORE_REG(reg) \
    unwind_constr.table[reg] = init_table[reg]; \
    SET_STATUS(unwind_constr, (reg), ((init_statuses[(reg) >> 1] >> (((reg) & 1) << 2)) & 0xF));
#define FAILED \
    box_free(init_statuses);                                                \
    box_free(init_table);                                                   \
    box_free(unwind_constr.statuses);                                       \
    box_free(unwind_constr.table);                                          \
    for (uint64_t i = 0; i < maxstacksize; ++i) box_free(table_stack[i]);   \
    box_free(table_stack);                                                  \
    *success = 0; return 0;
                while ((cur_inst < next_addr) && (cur_pointed_addr <= addr)) {
                    unsigned char inst; READ_U1(inst, cur_inst);
                    // printf_log(LOG_NONE, "Executing post 0x%02X\n", inst);
                    PARSE_INST
                }
#undef FAILED
#undef RESTORE_REG
#undef PARSE_INST
                box_free(init_statuses);
                box_free(init_table);
                for (uint64_t i = 0; i < maxstacksize; ++i) {
                    box_free(table_stack[i]);
                }
                box_free(table_stack);

                dwarf_unwind_t new_unwind;
                new_unwind.reg_count = unwind_constr.reg_count;
                new_unwind.regs = box_calloc(unwind_constr.reg_count, sizeof(uint64_t));
                uintptr_t cfa = unwind->regs[cfa_reg];
                if (cfa_signed) cfa += cfa_offset.soff;
                else            cfa += cfa_offset.uoff;
                
                // printf_log(LOG_NONE, "Done, rewriting registers (CFA at r%d(0x%016lX) + %lld (%c) -> 0x%016lX\n", cfa_reg, unwind->regs[cfa_reg], cfa_offset.soff, cfa_signed ? 's' : 'u', cfa);
                for (uint64_t i = 0; i < unwind_constr.reg_count; ++i) {
                    switch (GET_STATUS(unwind_constr, i)) {
                    case REGSTATUS_undefined:
                        // printf_log(LOG_NONE, "Register %02lX: (undefined)\n", i);
                        break;
                    case REGSTATUS_same_val:
                        if (i >= unwind->reg_count) {
                            printf_log(LOG_DEBUG, "Invalid register status (value copied from register 0x%02lX)\n", i);
                            box_free(unwind_constr.statuses);
                            box_free(unwind_constr.table);
                            box_free(new_unwind.regs);
                            *success = 0;
                            return 0;
                        }
                        new_unwind.regs[i] = unwind->regs[i];
                        // printf_log(LOG_NONE, "Register %02lX: copy   %016lX\n", i, new_unwind.regs[i]);
                        break;
                    case REGSTATUS_offset:
                        new_unwind.regs[i] = *(uint64_t*)(cfa + (int64_t)unwind_constr.table[i]);
                        // printf_log(LOG_NONE, "Register %02lX: offset %016lX [%016lX + %lld]\n", i, new_unwind.regs[i], cfa, (int64_t)unwind_constr.table[i]);
                        break;
                    case REGSTATUS_val_offset:
                        new_unwind.regs[i] = (uint64_t)(cfa + (int64_t)unwind_constr.table[i]);
                        // printf_log(LOG_NONE, "Register %02lX: voff   %016lX\n", i, new_unwind.regs[i]);
                        break;
                    case REGSTATUS_register:
                        if (unwind_constr.table[i] >= unwind->reg_count) {
                            printf_log(LOG_DEBUG, "Invalid register status (value copied from register 0x%02lX)\n", unwind_constr.table[i]);
                            box_free(unwind_constr.statuses);
                            box_free(unwind_constr.table);
                            box_free(new_unwind.regs);
                            *success = 0;
                            return 0;
                        }
                        new_unwind.regs[i] = unwind->regs[unwind_constr.table[i]];
                        // printf_log(LOG_NONE, "Register %02lX: reg    %016lX\n", i, new_unwind.regs[i]);
                        break;
                    }
                }
                *success = (GET_STATUS(unwind_constr, return_addr_reg) == REGSTATUS_undefined) ? 0 : ((aug_fields & AUG_SIGHDLER) ? 2 : 1);
                box_free(unwind_constr.statuses);
                box_free(unwind_constr.table);
                
                box_free(unwind->regs);
                unwind->reg_count = new_unwind.reg_count;
                unwind->regs = new_unwind.regs;

                // Maybe?
                unwind->regs[cfa_reg] = cfa;

                // printf_log(LOG_NONE, "Returning %016lX\n", unwind->regs[return_addr_reg]);
                return unwind->regs[return_addr_reg];
            }
        } else {
            // printf_log(LOG_NONE, "[FDE] Ptr             %02X\n", cie_ptr);
            printf_log(LOG_DEBUG, "Unexpected FDE, corresponding CIE missing\n");
            return 0;
        }

        cur_addr = next_addr;
    }
    *success = 0;
    return 0;
}

dwarf_unwind_t *init_dwarf_unwind_registers(x64emu_t *emu) {
    dwarf_unwind_t *unwind_struct = (dwarf_unwind_t*)box_malloc(sizeof(dwarf_unwind_t));
    unwind_struct->reg_count = 17;
    unwind_struct->regs = (uint64_t*)box_malloc(17*sizeof(uint64_t));
    /* x86_64-abi-0.99.pdf
     * Register Name                    | Number | Abbreviation
     * General Purpose Register RAX     | 0      | %rax
     * General Purpose Register RDX     | 1      | %rdx
     * General Purpose Register RCX     | 2      | %rcx
     * General Purpose Register RBX     | 3      | %rbx
     * General Purpose Register RSI     | 4      | %rsi
     * General Purpose Register RDI     | 5      | %rdi
     * Frame Pointer Register   RBP     | 6      | %rbp
     * Stack Pointer Register   RSP     | 7      | %rsp
     * Extended Integer Registers 8-15  | 8-15   | %r8-%r15
     * Return Address RA                | 16     |
     * Vector Registers 0-7             | 17-24  | %xmm0-%xmm7
     * Extended Vector Registers 8-15   | 25-32  | %xmm8-%xmm15
     * Floating Point Registers 0-7     | 33-40  | %st0-%st7
     * MMX Registers 0-7                | 41-48  | %mm0-%mm7
     * Flag Register                    | 49     | %rFLAGS
     * Segment Register ES              | 50     | %es
     * Segment Register CS              | 51     | %cs
     * Segment Register SS              | 52     | %ss
     * Segment Register DS              | 53     | %ds
     * Segment Register FS              | 54     | %fs
     * Segment Register GS              | 55     | %gs
     * Reserved                         | 56-57  |
     * FS Base address                  | 58     | %fs.base
     * GS Base address                  | 59     | %gs.base
     * Reserved                         | 60-61  |
     * Task Register                    | 62     | %tr
     * LDT Register                     | 63     | %ldtr
     * 128-bit Media Control and Status | 64     | %mxcsr
     * x87 Control Word                 | 65     | %fcw
     * x87 Status Word                  | 66     | %fsw
     */
    unwind_struct->regs[ 0] = emu->regs[_RAX].q[0];
    unwind_struct->regs[ 1] = emu->regs[_RDX].q[0];
    unwind_struct->regs[ 2] = emu->regs[_RCX].q[0];
    unwind_struct->regs[ 3] = emu->regs[_RBX].q[0];
    unwind_struct->regs[ 4] = emu->regs[_RSI].q[0];
    unwind_struct->regs[ 5] = emu->regs[_RDI].q[0];
    unwind_struct->regs[ 6] = emu->regs[_RBP].q[0];
    unwind_struct->regs[ 7] = emu->regs[_RSP].q[0] + 8;
    unwind_struct->regs[ 8] = emu->regs[_R8 ].q[0];
    unwind_struct->regs[ 9] = emu->regs[_R9 ].q[0];
    unwind_struct->regs[10] = emu->regs[_R10].q[0];
    unwind_struct->regs[11] = emu->regs[_R11].q[0];
    unwind_struct->regs[12] = emu->regs[_R12].q[0];
    unwind_struct->regs[13] = emu->regs[_R13].q[0];
    unwind_struct->regs[14] = emu->regs[_R14].q[0];
    unwind_struct->regs[15] = emu->regs[_R15].q[0];
    unwind_struct->regs[16] = emu->ip.q[0];
    return unwind_struct;
}

void free_dwarf_unwind_registers(dwarf_unwind_t **unwind_struct) {
    box_free((*unwind_struct)->regs);
    box_free(*unwind_struct);
    *unwind_struct = NULL;
}
