#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

#include "debug.h"
#include "x64trace.h"
#include "box64context.h"
#include "x86zydis.h"
#include "x64emu_private.h"

typedef ZyanStatus (*PFNZydisDecoderInit)(ZydisDecoder* decoder, ZydisMachineMode machine_mode,
    ZydisAddressWidth address_width);

typedef ZyanStatus (*PFNZydisFormatterInit)(ZydisFormatter* formatter, ZydisFormatterStyle style);

typedef ZyanStatus (*PFNZydisDecoderDecodeBuffer)(const ZydisDecoder* decoder,
    const void* buffer, ZyanUSize length, ZydisDecodedInstruction* instruction);

typedef ZyanStatus (*PFNZydisFormatterFormatInstruction)(const ZydisFormatter* formatter,
    const ZydisDecodedInstruction* instruction, char* buffer, ZyanUSize length,
    ZyanU64 runtime_address);

typedef struct zydis_s {
    void*                       lib;
    PFNZydisDecoderInit         ZydisDecoderInit;
    PFNZydisFormatterInit       ZydisFormatterInit;
    PFNZydisDecoderDecodeBuffer ZydisDecoderDecodeBuffer;
    PFNZydisFormatterFormatInstruction ZydisFormatterFormatInstruction;
} zydis_t;

typedef struct zydis_dec_s {
    ZydisDecoder            decoder;
    ZydisFormatter          formatter;
    ZydisDecodedInstruction instruction;
    PFNZydisDecoderDecodeBuffer ZydisDecoderDecodeBuffer;
    PFNZydisFormatterFormatInstruction ZydisFormatterFormatInstruction;
} zydis_dec_t;

int InitX64Trace(box64context_t *context)
{
    if(context->zydis)
        return 0;
    context->zydis = (zydis_t*)calloc(1, sizeof(zydis_t));
    if(!context->zydis)
        return 1;
    context->zydis->lib = dlopen("libZydis.so", RTLD_LAZY);
    if(!context->zydis->lib) {
        printf_log(LOG_INFO, "Failed to open libZydis: %s\n", dlerror());
        return 1;
    }
    #define GO(f) context->zydis->f = (PFN##f)dlsym(context->zydis->lib, #f);\
         if(!context->zydis->f) {printf_log(LOG_INFO, "Fail to load Zydis function %s\n", #f); dlclose(context->zydis->lib); context->zydis->lib=NULL; return 1;}

    GO(ZydisDecoderInit);
    GO(ZydisFormatterInit);
    GO(ZydisDecoderDecodeBuffer);
    GO(ZydisFormatterFormatInstruction);
    #undef GO

    context->dec = InitX64TraceDecoder(context);

    return 0;
}

void DeleteX64Trace(box64context_t *context)
{
    if(!context->zydis)
        return;
    if(context->zydis->lib)
        dlclose(context->zydis->lib);
    free(context->zydis);
    context->zydis = NULL;
}

zydis_dec_t* InitX64TraceDecoder(box64context_t *context)
{
    if(!context->zydis)
        return NULL;
    zydis_dec_t *dec = (zydis_dec_t*)calloc(1, sizeof(zydis_dec_t));
    dec->ZydisDecoderDecodeBuffer = context->zydis->ZydisDecoderDecodeBuffer;
    dec->ZydisFormatterFormatInstruction = context->zydis->ZydisFormatterFormatInstruction;
    context->zydis->ZydisDecoderInit(&dec->decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
    context->zydis->ZydisFormatterInit(&dec->formatter, ZYDIS_FORMATTER_STYLE_INTEL);

    return dec;
}
void DeleteX64TraceDecoder(zydis_dec_t **dec)
{
    free(*dec);
    *dec = NULL;
}

const char* DecodeX64Trace(zydis_dec_t *dec, uintptr_t p)
{
    static char buff[512];
    if(ZYAN_SUCCESS(dec->ZydisDecoderDecodeBuffer(&dec->decoder, (char*)p, 15,
        &dec->instruction))) {
        char tmp[511];
        buff[0]='\0';
        for (int i=0; i<dec->instruction.length; ++i) {
            sprintf(tmp, "%02X ", *((unsigned char*)p+i));
            strcat(buff, tmp);
        }
        dec->ZydisFormatterFormatInstruction(&dec->formatter, &dec->instruction, tmp, sizeof(tmp),p);
        strcat(buff, tmp);
    } else {
        sprintf(buff, "Decoder failed @%p", (void*)p);
    }
    return buff;
}
