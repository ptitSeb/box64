#include <stdint.h>
#include <sys/mman.h>

#include "debug.h"
#include "x64emu.h"
#include "emu/x64run_private.h"
#include "custommem.h"

#define OPCODE_READ  (1<<0)
#define OPCODE_WRITE (1<<1)
#define OPCODE_STACK (1<<2)

#define MODREG  ((nextop&0xC0)==0xC0)

int decode_avx(uint8_t* addr, int idx, vex_t vex)
{
    return 0;
}

int decode_0f(uint8_t* addr, int idx, rex_t rex)
{
    uint8_t nextop;
    switch(addr[idx++]) {
        case 0x00:
            return OPCODE_READ;
        case 0x01:
            return OPCODE_WRITE;
        case 0x10:
        case 0x12:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x28:
        case 0x2A:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        case 0x2F:
        case 0x50 ... 0x6B:
        case 0x6E:
        case 0x6F:
        case 0x70:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0xA3:
        case 0xAF:
        case 0xB6:
        case 0xB7:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
        case 0xC2:
        case 0xC4 ... 0xC6:
        case 0xD1 ... 0xD5:
        case 0xD7 ... 0xE5:
        case 0xE8 ... 0xEF:
        case 0xF1 ... 0xFE:
            nextop = addr[idx++];
            return (MODREG)?0:OPCODE_READ;
        case 0x11:
        case 0x13:
        case 0x17:
        case 0x29:
        case 0x2B:
        case 0x7E:
        case 0x7F:
        case 0x90 ... 0x9F:
        case 0xAE:
        case 0xC3:
        case 0xE7:
            nextop = addr[idx++];
            return (MODREG)?0:OPCODE_WRITE;
        case 0x71:
        case 0x72:
        case 0x73:
        case 0xA4:
        case 0xA5:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xB0:
        case 0xB1:
        case 0xB3:
        case 0xBA:
        case 0xBB:
        case 0xC0:
        case 0xC1:
        case 0xC7:
            nextop = addr[idx++];
            return (MODREG)?0:(OPCODE_READ|OPCODE_WRITE);
        case 0xA0:
        case 0xA8:
            return OPCODE_WRITE|OPCODE_STACK;
        case 0xA1:
        case 0xA9:
            return OPCODE_READ|OPCODE_STACK;
        case 0x38:  //todo
            return 0;
        case 0x3A:  //todo
            return 0;
    }
    return 0;
}
int decode_660f(uint8_t* addr, int idx, rex_t rex)
{
    return 0;
}
int decode_f20f(uint8_t* addr, int idx, rex_t rex)
{
    return 0;
}
int decode_f30f(uint8_t* addr, int idx, rex_t rex)
{
    return 0;
}

int decode_opcode(uintptr_t rip, int is32bits)
{
    if(!(getProtection(rip)&PROT_READ))
        return 0;
    // check if opcode is one that write to memory... pretty crude for now.
    uint8_t* addr = (uint8_t*)rip;
    int idx = 0;
    rex_t rex = {0};
    int is66 = 0, is67 = 0, rep = 0;
    int lock = 0;
    vex_t vex = {0};
    uint8_t nextop;
    if(is32bits) {
        rex.is32bits = 1;
        while(addr[idx]==0x66 || addr[idx]==0xF2 || addr[idx]==0xF3 || (addr[idx]==0x2E)|| (addr[idx]==0x3E) || (addr[idx]==0x26)|| (addr[idx]==0x36) || (addr[idx]==0xf0) || (addr[idx]==0x64)|| (addr[idx]==0x65)) {
            switch(addr[idx++]) {
                case 0x66: is66=1; break;
                case 0xF0: lock=1; break;
                case 0xF2: rep=1; break;
                case 0xF3: rep=2; break;
            }
        }
    } else {
        while((addr[idx]>=0x40 && addr[idx]<0x4f) || (addr[idx]==0x66 || addr[idx]==0xF2 || addr[idx]==0xF3 || (addr[idx]==0x3E) || (addr[idx]==0x26) || (addr[idx]==0xf0)) || (addr[idx]==0x64)|| (addr[idx]==0x65)) {
            switch(addr[idx++]) {
                case 0x66: is66=1; break;
                case 0xF0: lock=1; break;
                case 0xF2: rep=1; break;
                case 0xF3: rep=2; break;
                case 0x40 ... 0x4f: rex.rex = addr[idx-1]; break;
            }
        }
    }
    if((addr[idx]==0xC4 || addr[idx]==0xC5) && (!is32bits || ((addr[idx+1]&0xc0)!=0xc0))) {
        uint8_t tmp8u;
        switch(addr[idx++]) {
            case 0xC4:
                vex.rex = rex;
                tmp8u = nextop;
                vex.m = tmp8u&0b00011111;
                vex.rex.b = (tmp8u&0b00100000)?0:1;
                vex.rex.x = (tmp8u&0b01000000)?0:1;
                vex.rex.r = (tmp8u&0b10000000)?0:1;
                tmp8u = addr[idx++];
                vex.p = tmp8u&0b00000011;
                vex.l = (tmp8u>>2)&1;
                vex.v = ((~tmp8u)>>3)&0b1111;
                vex.rex.w = (tmp8u>>7)&1;
                break;
            case 0xC5:
                vex.rex = rex;
                tmp8u = nextop;
                vex.p = tmp8u&0b00000011;
                vex.l = (tmp8u>>2)&1;
                vex.v = ((~tmp8u)>>3)&0b1111;
                vex.rex.r = (tmp8u&0b10000000)?0:1;
                vex.rex.b = 0;
                vex.rex.x = 0;
                vex.rex.w = 0;
                vex.m = VEX_M_0F;
                break;
        }
        return decode_avx(addr, idx, vex);
    }
    switch(addr[idx++]) {
        case 0x00:
        case 0x01:
        case 0x08:
        case 0x09:
        case 0x10:
        case 0x11:
        case 0x18:
        case 0x19:
        case 0x20:
        case 0x21:
        case 0x28:
        case 0x29:
        case 0x30:
        case 0x31:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8C:
        case 0xC0:
        case 0xC1:
        case 0xC6:
        case 0xC7:
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
            nextop = addr[idx++];
            return (MODREG)?0:(OPCODE_WRITE|OPCODE_READ);
        case 0x02:
        case 0x03:
        case 0x0a:
        case 0x0b:
        case 0x12:
        case 0x13:
        case 0x1a:
        case 0x1b:
        case 0x22:
        case 0x23:
        case 0x2a:
        case 0x2b:
        case 0x32:
        case 0x33:
        case 0x38:
        case 0x39:
        case 0x3a:
        case 0x3b:
        case 0x69:
        case 0x6B:
        case 0x84:
        case 0x85:
        case 0x8A:
        case 0x8B:
        case 0x8E:
            nextop = addr[idx++];
            return (MODREG)?0:(OPCODE_READ);
        case 0x63:
            // In 32-bit mode this opcode is ARPL Ew, Gw (read + conditional write to r/m16).
            // In 64-bit mode this opcode is MOVSXD Gd, Ed (read-only).
            nextop = addr[idx++];
            if(is32bits)
                return (MODREG)?0:(OPCODE_WRITE|OPCODE_READ);
            return (MODREG)?0:(OPCODE_READ);
        case 0x06: 
        case 0x0E: 
        case 0x16: 
        case 0x1E: 
        case 0x60:
            return is32bits?(OPCODE_WRITE|OPCODE_STACK):0;
        case 0x07: 
        case 0x17: 
        case 0x1F:
        case 0x61: 
            return is32bits?(OPCODE_READ|OPCODE_STACK):0;
        case 0x50 ... 0x57:
        case 0x68:
        case 0x6A:
        case 0x9C:
        case 0xC8:
        case 0xE8:
            return OPCODE_WRITE|OPCODE_STACK;
        case 0x58 ... 0x5F:
        case 0x9D:
        case 0xC2:
        case 0xC3:
        case 0xC9:
        case 0xCA:
        case 0xCB:
        case 0xCF:
            return OPCODE_READ|OPCODE_STACK;
        case 0x80 ... 0x83:
            nextop = addr[idx++];
            return (MODREG)?0:(((((nextop>>3)&7)!=7)?OPCODE_WRITE:0)|OPCODE_READ);
        case 0x8F:
            nextop = addr[idx++];
            return ((MODREG)?0:(OPCODE_WRITE))|OPCODE_READ|OPCODE_STACK;
        case 0xA0:
        case 0xA1:
        case 0xA6:
        case 0xA7:
        case 0xAC:
        case 0xAD:
        case 0xAE:
        case 0xAF:
        case 0xD7:
            return OPCODE_READ;
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xAA:
        case 0xAB:
            return OPCODE_WRITE;
        case 0xF6:
        case 0xF7:
            nextop = addr[idx++];
            if(MODREG) return 0;
            switch((nextop>>3)&7) {
                case 2:
                case 3:
                    return OPCODE_WRITE;
                default:
                    return OPCODE_READ;
            }
        case 0xFE:
        case 0xFF:
            nextop = addr[idx++];
            if(MODREG) return 0;
            switch((nextop>>3)&7) {
                case 0:
                case 1:
                    return OPCODE_WRITE;
                case 2:
                case 3:
                case 6:
                    return OPCODE_READ|OPCODE_WRITE|OPCODE_STACK;
                default:
                    return OPCODE_READ;
            }

        case 0x0F: 
            if(is66) return decode_660f(addr, idx, rex);
            if(rep==1) return decode_f20f(addr, idx, rex);
            if(rep==2) return decode_f30f(addr, idx, rex);
            return decode_0f(addr, idx, rex);
        case 0xD8 ... 0xDF:
            nextop = addr[idx++];
            if(nextop<0xC0) {
                switch(addr[idx-2]) {
                    case 0xD8: 
                    case 0xDA: 
                    case 0xDC:
                    case 0xDE:
                        return OPCODE_READ;
                    case 0xD9: 
                        switch((nextop>>3)&7) {
                            case 0:
                            case 4:
                            case 5:
                                return OPCODE_READ;
                            case 2:
                            case 3:
                            case 6:
                            case 7:
                                return OPCODE_WRITE;
                        }
                        return 0;
                    case 0xDB:
                        switch((nextop>>3)&7) {
                            case 0:
                            case 1:
                            case 2:
                            case 3:
                            case 7:
                                return OPCODE_WRITE;
                            case 5:
                                return OPCODE_READ;
                        }
                        return 0;
                    case 0xDF: 
                        switch((nextop>>3)&7) {
                            case 0:
                            case 4:
                            case 5:
                                return OPCODE_READ;
                            case 1:
                            case 2:
                            case 3:
                            case 6:
                            case 7:
                                return OPCODE_WRITE;
                        }
                }
            }
            return 0;

    }
    return 0;
}

int write_opcode(uintptr_t rip, uintptr_t native_ip, int is32bits)
{
    // TODO, on ARM64, RiSCV and LoongArch, it would be easier to analyse the opcode at the native IP instead, as opcode that write to memory are more limited in quantity
    return (decode_opcode(rip, is32bits)&OPCODE_WRITE)?1:0;
}
