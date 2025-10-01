#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "x64trace.h"
#include "box64context.h"
#include "x64emu_private.h"
#ifdef HAVE_TRACE
#ifdef ZYDIS3
#include "x86zydis.h"
#else
#include <Zydis/Zydis.h>
#endif

/* We cannot use

#if ZYDIS_VERSION_MAJOR(ZYDIS_VERSION) == 4

... to check the version, as it's defineed with typecasts, for details, see: https://github.com/zyantific/zydis/pull/577
*/

#ifndef ZYDIS3 // Zydis 4.x
typedef ZyanStatus (*PFNZydisDecoderInit)(ZydisDecoder* decoder, ZydisMachineMode machine_mode, ZydisStackWidth stack_width);
typedef ZyanStatus (*PFNZydisFormatterInit)(ZydisFormatter* formatter, ZydisFormatterStyle style);
typedef ZyanStatus (*PFNZydisDecoderDecodeFull)(const ZydisDecoder* decoder, const void* buffer, ZyanUSize length, ZydisDecodedInstruction* instruction, ZydisDecodedOperand* operands);
typedef ZyanStatus (*PFNZydisFormatterFormatInstruction)(const ZydisFormatter* formatter, const ZydisDecodedInstruction* instruction, const ZydisDecodedOperand* operands, ZyanU8 operand_count, char* buffer, ZyanUSize length, ZyanU64 runtime_address, void* user_data);

typedef struct zydis_s {
    void* lib;
    PFNZydisDecoderInit ZydisDecoderInit;
    PFNZydisFormatterInit ZydisFormatterInit;
    PFNZydisDecoderDecodeFull ZydisDecoderDecodeFull;
    PFNZydisFormatterFormatInstruction ZydisFormatterFormatInstruction;
} zydis_t;

typedef struct zydis_dec_s {
    ZydisDecoder decoder;
    ZydisFormatter formatter;
    ZydisDecodedInstruction instruction;
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
    PFNZydisDecoderDecodeFull ZydisDecoderDecodeFull;
    PFNZydisFormatterFormatInstruction ZydisFormatterFormatInstruction;
} zydis_dec_t;

#else

typedef ZyanStatus (*PFNZydisDecoderInit)(ZydisDecoder* decoder, ZydisMachineMode machine_mode, ZydisAddressWidth address_width);
typedef ZyanStatus (*PFNZydisFormatterInit)(ZydisFormatter* formatter, ZydisFormatterStyle style);
typedef ZyanStatus (*PFNZydisDecoderDecodeBuffer)(const ZydisDecoder* decoder, const void* buffer, ZyanUSize length, ZydisDecodedInstruction* instruction);
typedef ZyanStatus (*PFNZydisFormatterFormatInstruction)(const ZydisFormatter* formatter, const ZydisDecodedInstruction* instruction, char* buffer, ZyanUSize length, ZyanU64 runtime_address);

typedef struct zydis_s {
    void* lib;
    PFNZydisDecoderInit ZydisDecoderInit;
    PFNZydisFormatterInit ZydisFormatterInit;
    PFNZydisDecoderDecodeBuffer ZydisDecoderDecodeBuffer;
    PFNZydisFormatterFormatInstruction ZydisFormatterFormatInstruction;
} zydis_t;

typedef struct zydis_dec_s {
    ZydisDecoder decoder;
    ZydisFormatter formatter;
    ZydisDecodedInstruction instruction;
    PFNZydisDecoderDecodeBuffer ZydisDecoderDecodeBuffer;
    PFNZydisFormatterFormatInstruction ZydisFormatterFormatInstruction;
} zydis_dec_t;

#endif

#else
typedef struct zydis_s {
    int dummy;
} zydis_t;

typedef struct zydis_dec_s {
    int dummy;
} zydis_dec_t;
#endif

int InitX64Trace(box64context_t* context)
{
#ifndef HAVE_TRACE
    return 1;
#else
    if (context->zydis)
        return 0;
    context->zydis = (zydis_t*)box_calloc(1, sizeof(zydis_t));
    if (!context->zydis)
        return 1;
    #ifndef ZYDIS3
    context->zydis->lib = dlopen("libZydis.so.4", RTLD_LAZY);
    #else
    context->zydis->lib = dlopen("libZydis.so.3", RTLD_LAZY);
    #endif
    if(!context->zydis->lib)
        context->zydis->lib = dlopen("libZydis.so", RTLD_LAZY);
    if (!context->zydis->lib) {
        printf_log(LOG_INFO, "Failed to open libZydis: %s\n", dlerror());
        return 1;
    }
#define GO(f)                                                         \
    context->zydis->f = (PFN##f)dlsym(context->zydis->lib, #f);       \
    if (!context->zydis->f) {                                         \
        printf_log(LOG_INFO, "Fail to load Zydis function %s\n", #f); \
        dlclose(context->zydis->lib);                                 \
        context->zydis->lib = NULL;                                   \
        return 1;                                                     \
    }

    GO(ZydisDecoderInit);
    GO(ZydisFormatterInit);
#ifndef ZYDIS3
    GO(ZydisDecoderDecodeFull);
#else
    GO(ZydisDecoderDecodeBuffer);
#endif
    GO(ZydisFormatterFormatInstruction);
#undef GO

    context->dec = InitX64TraceDecoder(context);
    context->dec32 = InitX86TraceDecoder(context);

    return 0;
#endif
}

void DeleteX64Trace(box64context_t* context)
{
#ifdef HAVE_TRACE
    if (!context->zydis)
        return;
    if (context->zydis->lib)
        dlclose(context->zydis->lib);
    box_free(context->zydis);
    context->zydis = NULL;
#endif
}

zydis_dec_t* InitX86TraceDecoder(box64context_t* context)
{
#ifndef HAVE_TRACE
    return NULL;
#else
    if (!context->zydis)
        return NULL;
    zydis_dec_t* dec = (zydis_dec_t*)box_calloc(1, sizeof(zydis_dec_t));
#ifndef ZYDIS3
    dec->ZydisDecoderDecodeFull = context->zydis->ZydisDecoderDecodeFull;
    dec->ZydisFormatterFormatInstruction = context->zydis->ZydisFormatterFormatInstruction;
    context->zydis->ZydisDecoderInit(&dec->decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_STACK_WIDTH_32);
    context->zydis->ZydisFormatterInit(&dec->formatter, ZYDIS_FORMATTER_STYLE_INTEL);
#else
    dec->ZydisDecoderDecodeBuffer = context->zydis->ZydisDecoderDecodeBuffer;
    dec->ZydisFormatterFormatInstruction = context->zydis->ZydisFormatterFormatInstruction;
    context->zydis->ZydisDecoderInit(&dec->decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_ADDRESS_WIDTH_32);
    context->zydis->ZydisFormatterInit(&dec->formatter, ZYDIS_FORMATTER_STYLE_INTEL);
#endif
    return dec;
#endif
}
void DeleteX86TraceDecoder(zydis_dec_t** dec)
{
#ifdef HAVE_TRACE
    box_free(*dec);
    *dec = NULL;
#endif
}

zydis_dec_t* InitX64TraceDecoder(box64context_t* context)
{
#ifndef HAVE_TRACE
    return NULL;
#else
    if (!context->zydis)
        return NULL;
    zydis_dec_t* dec = (zydis_dec_t*)box_calloc(1, sizeof(zydis_dec_t));
#ifndef ZYDIS3
    dec->ZydisDecoderDecodeFull = context->zydis->ZydisDecoderDecodeFull;
    dec->ZydisFormatterFormatInstruction = context->zydis->ZydisFormatterFormatInstruction;
    context->zydis->ZydisDecoderInit(&dec->decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_STACK_WIDTH_64);
    context->zydis->ZydisFormatterInit(&dec->formatter, ZYDIS_FORMATTER_STYLE_INTEL);
#else
    dec->ZydisDecoderDecodeBuffer = context->zydis->ZydisDecoderDecodeBuffer;
    dec->ZydisFormatterFormatInstruction = context->zydis->ZydisFormatterFormatInstruction;
    context->zydis->ZydisDecoderInit(&dec->decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
    context->zydis->ZydisFormatterInit(&dec->formatter, ZYDIS_FORMATTER_STYLE_INTEL);
#endif
    return dec;
#endif
}
void DeleteX64TraceDecoder(zydis_dec_t** dec)
{
#ifdef HAVE_TRACE
    box_free(*dec);
    *dec = NULL;
#endif
}

const char* DecodeX64Trace(zydis_dec_t* dec, uintptr_t p, int withhex)
{
#ifndef HAVE_TRACE
    return "???";
#else
    static char buff[512];
#ifndef ZYDIS3
    if (ZYAN_SUCCESS(dec->ZydisDecoderDecodeFull(&dec->decoder, (char*)p, 15,
            &dec->instruction, dec->operands)))
#else
    if (ZYAN_SUCCESS(dec->ZydisDecoderDecodeBuffer(&dec->decoder, (char*)p, 15,
            &dec->instruction)))
#endif
    {
        char tmp[511];
        buff[0] = '\0';
        if (withhex) {
            for (int i = 0; i < dec->instruction.length; ++i) {
                sprintf(tmp, "%02X ", *((unsigned char*)p + i));
                strcat(buff, tmp);
            }
        }
#ifndef ZYDIS3
        dec->ZydisFormatterFormatInstruction(&dec->formatter, &dec->instruction, dec->operands, dec->instruction.operand_count, tmp, sizeof(tmp), p, ZYAN_NULL);
#else
        dec->ZydisFormatterFormatInstruction(&dec->formatter, &dec->instruction, tmp, sizeof(tmp), p);
#endif
        strncat(buff, tmp, sizeof(buff)-1);
    } else {
        snprintf(buff, sizeof(buff), "Decoder failed @%p: ", (void*)p);
        if (withhex) {
            char tmp[10];
            for (int i = 0; i < 15; ++i) {
                sprintf(tmp, "%02X ", *((unsigned char*)p + i));
                strcat(buff, tmp);
            }
        }
    }
    return buff;
#endif
}

int OpcodeOK(zydis_dec_t* dec, uintptr_t p)
{
#ifndef HAVE_TRACE
    return 1;
#else
#ifndef ZYDIS3
    if (ZYAN_SUCCESS(dec->ZydisDecoderDecodeFull(&dec->decoder, (char*)p, 15,
            &dec->instruction, dec->operands)))
#else
    if (ZYAN_SUCCESS(dec->ZydisDecoderDecodeBuffer(&dec->decoder, (char*)p, 15,
            &dec->instruction)))
#endif
        return 1;
    return 0;
#endif
}
