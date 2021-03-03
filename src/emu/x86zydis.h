#ifndef _X86_ZYDIS_H_
#define _X86_ZYDIS_H_

#include <stdint.h>

// Re-create Zydis structures
#define ZYDIS_DECODER_MODE_MAX_VALUE 8

typedef uint32_t ZyanStatus;
#define ZYAN_SUCCESS(status) \
    (!((status) & 0x80000000))
#define ZYAN_FAILED(status) \
    ((status) & 0x80000000)
#define ZYAN_CHECK(status) \
    do \
    { \
        const ZyanStatus status_047620348 = (status); \
        if (!ZYAN_SUCCESS(status_047620348)) \
        { \
            return status_047620348; \
        } \
    } while (0)
/*
 * @brief   Checks, if the bit at index `b` is required to present the ordinal value `n`.
 *
 * @param   n   The ordinal value.
 * @param   b   The bit index.
 *
 * @return  `ZYAN_TRUE`, if the bit at index `b` is required to present the ordinal value `n` or
 *          `ZYAN_FALSE`, if not.
 *
 * Note that this macro always returns `ZYAN_FALSE` for `n == 0`.
 */
#define ZYAN_NEEDS_BIT(n, b) (((unsigned long)(n) >> (b)) > 0)
/*
 * @brief   Returns the number of bits required to represent the ordinal value `n`.
 *
 * @param   n   The ordinal value.
 *
 * @return  The number of bits required to represent the ordinal value `n`.
 *
 * Note that this macro returns `0` for `n == 0`.
 */
#define ZYAN_BITS_TO_REPRESENT(n) \
    ( \
        ZYAN_NEEDS_BIT(n,  0) + ZYAN_NEEDS_BIT(n,  1) + \
        ZYAN_NEEDS_BIT(n,  2) + ZYAN_NEEDS_BIT(n,  3) + \
        ZYAN_NEEDS_BIT(n,  4) + ZYAN_NEEDS_BIT(n,  5) + \
        ZYAN_NEEDS_BIT(n,  6) + ZYAN_NEEDS_BIT(n,  7) + \
        ZYAN_NEEDS_BIT(n,  8) + ZYAN_NEEDS_BIT(n,  9) + \
        ZYAN_NEEDS_BIT(n, 10) + ZYAN_NEEDS_BIT(n, 11) + \
        ZYAN_NEEDS_BIT(n, 12) + ZYAN_NEEDS_BIT(n, 13) + \
        ZYAN_NEEDS_BIT(n, 14) + ZYAN_NEEDS_BIT(n, 15) + \
        ZYAN_NEEDS_BIT(n, 16) + ZYAN_NEEDS_BIT(n, 17) + \
        ZYAN_NEEDS_BIT(n, 18) + ZYAN_NEEDS_BIT(n, 19) + \
        ZYAN_NEEDS_BIT(n, 20) + ZYAN_NEEDS_BIT(n, 21) + \
        ZYAN_NEEDS_BIT(n, 22) + ZYAN_NEEDS_BIT(n, 23) + \
        ZYAN_NEEDS_BIT(n, 24) + ZYAN_NEEDS_BIT(n, 25) + \
        ZYAN_NEEDS_BIT(n, 26) + ZYAN_NEEDS_BIT(n, 27) + \
        ZYAN_NEEDS_BIT(n, 28) + ZYAN_NEEDS_BIT(n, 29) + \
        ZYAN_NEEDS_BIT(n, 30) + ZYAN_NEEDS_BIT(n, 31)   \
    )

typedef size_t    ZyanUSize;
typedef uint8_t    ZyanU8;
typedef uint16_t    ZyanU16;
typedef uint32_t    ZyanU32;
typedef uint64_t    ZyanU64;
typedef int32_t    ZyanI32;
typedef int64_t    ZyanI64;
typedef uint8_t    ZyanBool;

#define ZYDIS_MAX_INSTRUCTION_LENGTH 15
#define ZYDIS_MAX_OPERAND_COUNT      10

struct ZyanAllocator_;

/**
 * @brief   Defines the `ZyanAllocatorAllocate` function prototype.
 *
 * @param   allocator       A pointer to the `ZyanAllocator` instance.
 * @param   p               Receives a pointer to the first memory block sufficient to hold an
 *                          array of `n` elements with a size of `element_size`.
 * @param   element_size    The size of a single element.
 * @param   n               The number of elements to allocate storage for.
 *
 * @return  A zyan status code.
 *
 * This prototype is used for the `allocate()` and `reallocate()` functions.
 *
 * The result of the `reallocate()` function is undefined, if `p` does not point to a memory block
 * previously obtained by `(re-)allocate()`.
 */
typedef ZyanStatus (*ZyanAllocatorAllocate)(struct ZyanAllocator_* allocator, void** p,
    ZyanUSize element_size, ZyanUSize n);

/**
 * @brief   Defines the `ZyanAllocatorDeallocate` function prototype.
 *
 * @param   allocator       A pointer to the `ZyanAllocator` instance.
 * @param   p               The pointer obtained from `(re-)allocate()`.
 * @param   element_size    The size of a single element.
 * @param   n               The number of elements earlier passed to `(re-)allocate()`.
 *
  * @return  A zyan status code.
 */
typedef ZyanStatus (*ZyanAllocatorDeallocate)(struct ZyanAllocator_* allocator, void* p,
    ZyanUSize element_size, ZyanUSize n);

/**
 * @brief   Defines the `ZyanAllocator` struct.
 *
 * This is the base class for all custom allocator implementations.
 *
 * All fields in this struct should be considered as "private". Any changes may lead to unexpected
 * behavior.
 */
typedef struct ZyanAllocator_
{
    /**
     * @brief   The allocate function.
     */
    ZyanAllocatorAllocate allocate;
    /**
     * @brief   The reallocate function.
     */
    ZyanAllocatorAllocate reallocate;
    /**
     * @brief   The deallocate function.
     */
    ZyanAllocatorDeallocate deallocate;
} ZyanAllocator;

typedef struct ZyanVector_
{
    /**
     * @brief   The memory allocator.
     */
    ZyanAllocator* allocator;
    /**
     * @brief   The growth factor.
     */
    float growth_factor;
    /**
     * @brief   The shrink threshold.
     */
    float shrink_threshold;
    /**
     * @brief   The current number of elements in the vector.
     */
    ZyanUSize size;
    /**
     * @brief   The maximum capacity (number of elements).
     */
    ZyanUSize capacity;
    /**
     * @brief   The size of a single element in bytes.
     */
    ZyanUSize element_size;
    /**
     * @brief   The data pointer.
     */
    void* data;
} ZyanVector;
typedef ZyanU8 ZyanStringFlags;
/**
 * @brief   Defines the `ZyanString` struct.
 *
 * The `ZyanString` type is implemented as a size-prefixed string - which allows for a lot of
 * performance optimizations.
 * Nevertheless null-termination is guaranteed at all times to provide maximum compatibility with
 * default C-style strings (use `ZyanStringGetData` to access the C-style string).
 *
 * All fields in this struct should be considered as "private". Any changes may lead to unexpected
 * behavior.
 */
typedef struct ZyanString_
{
    /**
     * @brief   String flags.
     */
    ZyanStringFlags flags;
    /**
     * @brief   The vector that contains the actual string.
     */
    ZyanVector vector;
} ZyanString;

/* ---------------------------------------------------------------------------------------------- */
/* View                                                                                           */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Defines the `ZyanStringView` struct.
 *
 * The `ZyanStringView` type provides a view inside a string (`ZyanString` instances, null-
 * terminated C-style strings, or even not-null-terminated custom strings). A view is immutable
 * by design and can't be directly converted to a C-style string.
 *
 * Views might become invalid (e.g. pointing to invalid memory), if the underlying string gets
 * destroyed or resized.
 *
 * The `ZYAN_STRING_TO_VIEW` macro can be used to cast a `ZyanString` to a `ZyanStringView` pointer
 * without any runtime overhead.
 * Casting a view to a normal string is not supported and will lead to unexpected behavior (use
 * `ZyanStringDuplicate` to create a deep-copy instead).
 *
 * All fields in this struct should be considered as "private". Any changes may lead to unexpected
 * behavior.
 */
typedef struct ZyanStringView_
{
    /**
     * @brief   The string data.
     *
     * The view internally re-uses the normal string struct to allow casts without any runtime
     * overhead.
     */
    ZyanString string;
} ZyanStringView;

typedef enum ZydisMachineMode_
{
    /**
     * @brief 64 bit mode.
     */
    ZYDIS_MACHINE_MODE_LONG_64,
    /**
     * @brief 32 bit protected mode.
     */
    ZYDIS_MACHINE_MODE_LONG_COMPAT_32,
    /**
     * @brief 16 bit protected mode.
     */
    ZYDIS_MACHINE_MODE_LONG_COMPAT_16,
    /**
     * @brief 32 bit protected mode.
     */
    ZYDIS_MACHINE_MODE_LEGACY_32,
    /**
     * @brief 16 bit protected mode.
     */
    ZYDIS_MACHINE_MODE_LEGACY_16,
    /**
     * @brief 16 bit real mode.
     */
    ZYDIS_MACHINE_MODE_REAL_16,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_MACHINE_MODE_MAX_VALUE = ZYDIS_MACHINE_MODE_REAL_16,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_MACHINE_MODE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_MACHINE_MODE_MAX_VALUE)
} ZydisMachineMode;

typedef enum ZydisAddressWidth_
{
    ZYDIS_ADDRESS_WIDTH_16,
    ZYDIS_ADDRESS_WIDTH_32,
    ZYDIS_ADDRESS_WIDTH_64,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_ADDRESS_WIDTH_MAX_VALUE = ZYDIS_ADDRESS_WIDTH_64,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_ADDRESS_WIDTH_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_ADDRESS_WIDTH_MAX_VALUE)
} ZydisAddressWidth;

typedef struct ZydisDecoder_
{
    /**
     * @brief   The machine mode.
     */
    ZydisMachineMode machine_mode;
    /**
     * @brief   The address width.
     */
    ZydisAddressWidth address_width;
    /**
     * @brief   The decoder mode array.
     */
    uint8_t decoder_mode[ZYDIS_DECODER_MODE_MAX_VALUE + 1];
} ZydisDecoder;

typedef enum ZydisFormatterStyle_
{
    /**
     * @brief   Generates `AT&T`-style disassembly.
     */
    ZYDIS_FORMATTER_STYLE_ATT,
    /**
     * @brief   Generates `Intel`-style disassembly.
     */
    ZYDIS_FORMATTER_STYLE_INTEL,
    /**
     * @brief   Generates `MASM`-style disassembly that is directly accepted as input for the
     *          `MASM` assembler.
     *
     * The runtime-address is ignored in this mode.
     */
    ZYDIS_FORMATTER_STYLE_INTEL_MASM,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_FORMATTER_STYLE_MAX_VALUE = ZYDIS_FORMATTER_STYLE_INTEL_MASM,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_FORMATTER_STYLE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_FORMATTER_STYLE_MAX_VALUE)
} ZydisFormatterStyle;

/**
 * @brief   Defines the `ZydisRegister` enum.
 */
typedef enum ZydisRegister_
{
    ZYDIS_REGISTER_NONE,

    // General purpose registers  8-bit
    ZYDIS_REGISTER_AL,
    ZYDIS_REGISTER_CL,
    ZYDIS_REGISTER_DL,
    ZYDIS_REGISTER_BL,
    ZYDIS_REGISTER_AH,
    ZYDIS_REGISTER_CH,
    ZYDIS_REGISTER_DH,
    ZYDIS_REGISTER_BH,
    ZYDIS_REGISTER_SPL,
    ZYDIS_REGISTER_BPL,
    ZYDIS_REGISTER_SIL,
    ZYDIS_REGISTER_DIL,
    ZYDIS_REGISTER_R8B,
    ZYDIS_REGISTER_R9B,
    ZYDIS_REGISTER_R10B,
    ZYDIS_REGISTER_R11B,
    ZYDIS_REGISTER_R12B,
    ZYDIS_REGISTER_R13B,
    ZYDIS_REGISTER_R14B,
    ZYDIS_REGISTER_R15B,
    // General purpose registers 16-bit
    ZYDIS_REGISTER_AX,
    ZYDIS_REGISTER_CX,
    ZYDIS_REGISTER_DX,
    ZYDIS_REGISTER_BX,
    ZYDIS_REGISTER_SP,
    ZYDIS_REGISTER_BP,
    ZYDIS_REGISTER_SI,
    ZYDIS_REGISTER_DI,
    ZYDIS_REGISTER_R8W,
    ZYDIS_REGISTER_R9W,
    ZYDIS_REGISTER_R10W,
    ZYDIS_REGISTER_R11W,
    ZYDIS_REGISTER_R12W,
    ZYDIS_REGISTER_R13W,
    ZYDIS_REGISTER_R14W,
    ZYDIS_REGISTER_R15W,
    // General purpose registers 32-bit
    ZYDIS_REGISTER_EAX,
    ZYDIS_REGISTER_ECX,
    ZYDIS_REGISTER_EDX,
    ZYDIS_REGISTER_EBX,
    ZYDIS_REGISTER_ESP,
    ZYDIS_REGISTER_EBP,
    ZYDIS_REGISTER_ESI,
    ZYDIS_REGISTER_EDI,
    ZYDIS_REGISTER_R8D,
    ZYDIS_REGISTER_R9D,
    ZYDIS_REGISTER_R10D,
    ZYDIS_REGISTER_R11D,
    ZYDIS_REGISTER_R12D,
    ZYDIS_REGISTER_R13D,
    ZYDIS_REGISTER_R14D,
    ZYDIS_REGISTER_R15D,
    // General purpose registers 64-bit
    ZYDIS_REGISTER_RAX,
    ZYDIS_REGISTER_RCX,
    ZYDIS_REGISTER_RDX,
    ZYDIS_REGISTER_RBX,
    ZYDIS_REGISTER_RSP,
    ZYDIS_REGISTER_RBP,
    ZYDIS_REGISTER_RSI,
    ZYDIS_REGISTER_RDI,
    ZYDIS_REGISTER_R8,
    ZYDIS_REGISTER_R9,
    ZYDIS_REGISTER_R10,
    ZYDIS_REGISTER_R11,
    ZYDIS_REGISTER_R12,
    ZYDIS_REGISTER_R13,
    ZYDIS_REGISTER_R14,
    ZYDIS_REGISTER_R15,
    // Floating point legacy registers
    ZYDIS_REGISTER_ST0,
    ZYDIS_REGISTER_ST1,
    ZYDIS_REGISTER_ST2,
    ZYDIS_REGISTER_ST3,
    ZYDIS_REGISTER_ST4,
    ZYDIS_REGISTER_ST5,
    ZYDIS_REGISTER_ST6,
    ZYDIS_REGISTER_ST7,
    ZYDIS_REGISTER_X87CONTROL,
    ZYDIS_REGISTER_X87STATUS,
    ZYDIS_REGISTER_X87TAG,
    // Floating point multimedia registers
    ZYDIS_REGISTER_MM0,
    ZYDIS_REGISTER_MM1,
    ZYDIS_REGISTER_MM2,
    ZYDIS_REGISTER_MM3,
    ZYDIS_REGISTER_MM4,
    ZYDIS_REGISTER_MM5,
    ZYDIS_REGISTER_MM6,
    ZYDIS_REGISTER_MM7,
    // Floating point vector registers 128-bit
    ZYDIS_REGISTER_XMM0,
    ZYDIS_REGISTER_XMM1,
    ZYDIS_REGISTER_XMM2,
    ZYDIS_REGISTER_XMM3,
    ZYDIS_REGISTER_XMM4,
    ZYDIS_REGISTER_XMM5,
    ZYDIS_REGISTER_XMM6,
    ZYDIS_REGISTER_XMM7,
    ZYDIS_REGISTER_XMM8,
    ZYDIS_REGISTER_XMM9,
    ZYDIS_REGISTER_XMM10,
    ZYDIS_REGISTER_XMM11,
    ZYDIS_REGISTER_XMM12,
    ZYDIS_REGISTER_XMM13,
    ZYDIS_REGISTER_XMM14,
    ZYDIS_REGISTER_XMM15,
    ZYDIS_REGISTER_XMM16,
    ZYDIS_REGISTER_XMM17,
    ZYDIS_REGISTER_XMM18,
    ZYDIS_REGISTER_XMM19,
    ZYDIS_REGISTER_XMM20,
    ZYDIS_REGISTER_XMM21,
    ZYDIS_REGISTER_XMM22,
    ZYDIS_REGISTER_XMM23,
    ZYDIS_REGISTER_XMM24,
    ZYDIS_REGISTER_XMM25,
    ZYDIS_REGISTER_XMM26,
    ZYDIS_REGISTER_XMM27,
    ZYDIS_REGISTER_XMM28,
    ZYDIS_REGISTER_XMM29,
    ZYDIS_REGISTER_XMM30,
    ZYDIS_REGISTER_XMM31,
    // Floating point vector registers 256-bit
    ZYDIS_REGISTER_YMM0,
    ZYDIS_REGISTER_YMM1,
    ZYDIS_REGISTER_YMM2,
    ZYDIS_REGISTER_YMM3,
    ZYDIS_REGISTER_YMM4,
    ZYDIS_REGISTER_YMM5,
    ZYDIS_REGISTER_YMM6,
    ZYDIS_REGISTER_YMM7,
    ZYDIS_REGISTER_YMM8,
    ZYDIS_REGISTER_YMM9,
    ZYDIS_REGISTER_YMM10,
    ZYDIS_REGISTER_YMM11,
    ZYDIS_REGISTER_YMM12,
    ZYDIS_REGISTER_YMM13,
    ZYDIS_REGISTER_YMM14,
    ZYDIS_REGISTER_YMM15,
    ZYDIS_REGISTER_YMM16,
    ZYDIS_REGISTER_YMM17,
    ZYDIS_REGISTER_YMM18,
    ZYDIS_REGISTER_YMM19,
    ZYDIS_REGISTER_YMM20,
    ZYDIS_REGISTER_YMM21,
    ZYDIS_REGISTER_YMM22,
    ZYDIS_REGISTER_YMM23,
    ZYDIS_REGISTER_YMM24,
    ZYDIS_REGISTER_YMM25,
    ZYDIS_REGISTER_YMM26,
    ZYDIS_REGISTER_YMM27,
    ZYDIS_REGISTER_YMM28,
    ZYDIS_REGISTER_YMM29,
    ZYDIS_REGISTER_YMM30,
    ZYDIS_REGISTER_YMM31,
    // Floating point vector registers 512-bit
    ZYDIS_REGISTER_ZMM0,
    ZYDIS_REGISTER_ZMM1,
    ZYDIS_REGISTER_ZMM2,
    ZYDIS_REGISTER_ZMM3,
    ZYDIS_REGISTER_ZMM4,
    ZYDIS_REGISTER_ZMM5,
    ZYDIS_REGISTER_ZMM6,
    ZYDIS_REGISTER_ZMM7,
    ZYDIS_REGISTER_ZMM8,
    ZYDIS_REGISTER_ZMM9,
    ZYDIS_REGISTER_ZMM10,
    ZYDIS_REGISTER_ZMM11,
    ZYDIS_REGISTER_ZMM12,
    ZYDIS_REGISTER_ZMM13,
    ZYDIS_REGISTER_ZMM14,
    ZYDIS_REGISTER_ZMM15,
    ZYDIS_REGISTER_ZMM16,
    ZYDIS_REGISTER_ZMM17,
    ZYDIS_REGISTER_ZMM18,
    ZYDIS_REGISTER_ZMM19,
    ZYDIS_REGISTER_ZMM20,
    ZYDIS_REGISTER_ZMM21,
    ZYDIS_REGISTER_ZMM22,
    ZYDIS_REGISTER_ZMM23,
    ZYDIS_REGISTER_ZMM24,
    ZYDIS_REGISTER_ZMM25,
    ZYDIS_REGISTER_ZMM26,
    ZYDIS_REGISTER_ZMM27,
    ZYDIS_REGISTER_ZMM28,
    ZYDIS_REGISTER_ZMM29,
    ZYDIS_REGISTER_ZMM30,
    ZYDIS_REGISTER_ZMM31,
    // Flags registers
    ZYDIS_REGISTER_FLAGS,
    ZYDIS_REGISTER_EFLAGS,
    ZYDIS_REGISTER_RFLAGS,
    // Instruction-pointer registers
    ZYDIS_REGISTER_IP,
    ZYDIS_REGISTER_EIP,
    ZYDIS_REGISTER_RIP,
    // Segment registers
    ZYDIS_REGISTER_ES,
    ZYDIS_REGISTER_CS,
    ZYDIS_REGISTER_SS,
    ZYDIS_REGISTER_DS,
    ZYDIS_REGISTER_FS,
    ZYDIS_REGISTER_GS,
    // Table registers
    ZYDIS_REGISTER_GDTR,
    ZYDIS_REGISTER_LDTR,
    ZYDIS_REGISTER_IDTR,
    ZYDIS_REGISTER_TR,
    // Test registers
    ZYDIS_REGISTER_TR0,
    ZYDIS_REGISTER_TR1,
    ZYDIS_REGISTER_TR2,
    ZYDIS_REGISTER_TR3,
    ZYDIS_REGISTER_TR4,
    ZYDIS_REGISTER_TR5,
    ZYDIS_REGISTER_TR6,
    ZYDIS_REGISTER_TR7,
    // Control registers
    ZYDIS_REGISTER_CR0,
    ZYDIS_REGISTER_CR1,
    ZYDIS_REGISTER_CR2,
    ZYDIS_REGISTER_CR3,
    ZYDIS_REGISTER_CR4,
    ZYDIS_REGISTER_CR5,
    ZYDIS_REGISTER_CR6,
    ZYDIS_REGISTER_CR7,
    ZYDIS_REGISTER_CR8,
    ZYDIS_REGISTER_CR9,
    ZYDIS_REGISTER_CR10,
    ZYDIS_REGISTER_CR11,
    ZYDIS_REGISTER_CR12,
    ZYDIS_REGISTER_CR13,
    ZYDIS_REGISTER_CR14,
    ZYDIS_REGISTER_CR15,
    // Debug registers
    ZYDIS_REGISTER_DR0,
    ZYDIS_REGISTER_DR1,
    ZYDIS_REGISTER_DR2,
    ZYDIS_REGISTER_DR3,
    ZYDIS_REGISTER_DR4,
    ZYDIS_REGISTER_DR5,
    ZYDIS_REGISTER_DR6,
    ZYDIS_REGISTER_DR7,
    ZYDIS_REGISTER_DR8,
    ZYDIS_REGISTER_DR9,
    ZYDIS_REGISTER_DR10,
    ZYDIS_REGISTER_DR11,
    ZYDIS_REGISTER_DR12,
    ZYDIS_REGISTER_DR13,
    ZYDIS_REGISTER_DR14,
    ZYDIS_REGISTER_DR15,
    // Mask registers
    ZYDIS_REGISTER_K0,
    ZYDIS_REGISTER_K1,
    ZYDIS_REGISTER_K2,
    ZYDIS_REGISTER_K3,
    ZYDIS_REGISTER_K4,
    ZYDIS_REGISTER_K5,
    ZYDIS_REGISTER_K6,
    ZYDIS_REGISTER_K7,
    // Bound registers
    ZYDIS_REGISTER_BND0,
    ZYDIS_REGISTER_BND1,
    ZYDIS_REGISTER_BND2,
    ZYDIS_REGISTER_BND3,
    ZYDIS_REGISTER_BNDCFG,
    ZYDIS_REGISTER_BNDSTATUS,
    // Uncategorized
    ZYDIS_REGISTER_MXCSR,
    ZYDIS_REGISTER_PKRU,
    ZYDIS_REGISTER_XCR0,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_REGISTER_MAX_VALUE = ZYDIS_REGISTER_XCR0,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_REGISTER_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_REGISTER_MAX_VALUE)
} ZydisRegister;

typedef enum ZydisNumericBase_
{
    /**
     * @brief   Decimal system.
     */
    ZYDIS_NUMERIC_BASE_DEC,
    /**
     * @brief   Hexadecimal system.
     */
    ZYDIS_NUMERIC_BASE_HEX,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_NUMERIC_BASE_MAX_VALUE = ZYDIS_NUMERIC_BASE_HEX,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_NUMERIC_BASE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_NUMERIC_BASE_MAX_VALUE)
} ZydisNumericBase;

/**
 * @brief   Defines the `ZydisSignedness` enum.
 */
typedef enum ZydisSignedness_
{
    /**
     * @brief   Automatically choose the most suitable mode based on the operands
     *          `ZydisDecodedOperand.imm.is_signed` attribute.
     */
    ZYDIS_SIGNEDNESS_AUTO,
    /**
     * @brief   Force signed values.
     */
    ZYDIS_SIGNEDNESS_SIGNED,
    /**
     * @brief   Force unsigned values.
     */
    ZYDIS_SIGNEDNESS_UNSIGNED,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_SIGNEDNESS_MAX_VALUE = ZYDIS_SIGNEDNESS_UNSIGNED,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_SIGNEDNESS_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_SIGNEDNESS_MAX_VALUE)
} ZydisSignedness;
/**
 * @brief   Defines the `ZydisDecorator` enum.
 */
typedef enum ZydisDecorator_
{
    ZYDIS_DECORATOR_INVALID,
    /**
     * @brief   The embedded-mask decorator.
     */
    ZYDIS_DECORATOR_MASK,
    /**
     * @brief   The broadcast decorator.
     */
    ZYDIS_DECORATOR_BC,
    /**
     * @brief   The rounding-control decorator.
     */
    ZYDIS_DECORATOR_RC,
    /**
     * @brief   The suppress-all-exceptions decorator.
     */
    ZYDIS_DECORATOR_SAE,
    /**
     * @brief   The register-swizzle decorator.
     */
    ZYDIS_DECORATOR_SWIZZLE,
    /**
     * @brief   The conversion decorator.
     */
    ZYDIS_DECORATOR_CONVERSION,
    /**
     * @brief   The eviction-hint decorator.
     */
    ZYDIS_DECORATOR_EH,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_DECORATOR_MAX_VALUE = ZYDIS_DECORATOR_EH,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_DECORATOR_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_DECORATOR_MAX_VALUE)
} ZydisDecorator;
/**
 * @brief   Defines the `ZydisPadding` enum.
 */
typedef enum ZydisPadding_
{
    /**
     * @brief   Disables padding.
     */
    ZYDIS_PADDING_DISABLED = 0,
    /**
     * @brief   Padds the value to the current stack-width for addresses, or to the operand-width
     *          for immediate values (hexadecimal only).
     */
    ZYDIS_PADDING_AUTO     = (-1),

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_PADDING_MAX_VALUE = ZYDIS_PADDING_AUTO,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_PADDING_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_PADDING_MAX_VALUE)
} ZydisPadding;

typedef struct ZydisFormatter_ ZydisFormatter;
typedef struct ZydisFormatterBuffer_ ZydisFormatterBuffer;
typedef struct ZydisFormatterContext_ ZydisFormatterContext;

typedef ZyanStatus (*ZydisFormatterFunc)(const ZydisFormatter* formatter,
    ZydisFormatterBuffer* buffer, ZydisFormatterContext* context);
typedef ZyanStatus (*ZydisFormatterRegisterFunc)(const ZydisFormatter* formatter,
    ZydisFormatterBuffer* buffer, ZydisFormatterContext* context, ZydisRegister reg);
typedef ZyanStatus (*ZydisFormatterDecoratorFunc)(const ZydisFormatter* formatter,
    ZydisFormatterBuffer* buffer, ZydisFormatterContext* context, ZydisDecorator decorator);

typedef struct ZydisFormatter_
{
    /**
     * @brief   The formatter style.
     */
    ZydisFormatterStyle style;
    /**
     * @brief   The `ZYDIS_FORMATTER_PROP_FORCE_SIZE` property.
     */
    uint8_t force_memory_size;
    /**
     * @brief   The `ZYDIS_FORMATTER_PROP_FORCE_SEGMENT` property.
     */
    uint8_t force_memory_segment;
    /**
     * @brief   The `ZYDIS_FORMATTER_PROP_FORCE_RELATIVE_BRANCHES` property.
     */
    uint8_t force_relative_branches;
    /**
     * @brief   The `ZYDIS_FORMATTER_PROP_FORCE_RELATIVE_RIPREL` property.
     */
    uint8_t force_relative_riprel;
    /**
     * @brief   The `ZYDIS_FORMATTER_PROP_PRINT_BRANCH_SIZE` property.
     */
    uint8_t print_branch_size;
    /**
     * @brief   The `ZYDIS_FORMATTER_DETAILED_PREFIXES` property.
     */
    uint8_t detailed_prefixes;
    /**
     * @brief   The `ZYDIS_FORMATTER_ADDR_BASE` property.
     */
    ZydisNumericBase addr_base;
    /**
     * @brief   The `ZYDIS_FORMATTER_ADDR_SIGNEDNESS` property.
     */
    ZydisSignedness addr_signedness;
    /**
     * @brief   The `ZYDIS_FORMATTER_ADDR_PADDING_ABSOLUTE` property.
     */
    ZydisPadding addr_padding_absolute;
    /**
     * @brief   The `ZYDIS_FORMATTER_ADDR_PADDING_RELATIVE` property.
     */
    ZydisPadding addr_padding_relative;
    /**
     * @brief   The `ZYDIS_FORMATTER_DISP_BASE` property.
     */
    ZydisNumericBase disp_base;
    /**
     * @brief   The `ZYDIS_FORMATTER_DISP_SIGNEDNESS` property.
     */
    ZydisSignedness disp_signedness;
    /**
     * @brief   The `ZYDIS_FORMATTER_DISP_PADDING` property.
     */
    ZydisPadding disp_padding;
    /**
     * @brief   The `ZYDIS_FORMATTER_IMM_BASE` property.
     */
    ZydisNumericBase imm_base;
    /**
     * @brief   The `ZYDIS_FORMATTER_IMM_SIGNEDNESS` property.
     */
    ZydisSignedness imm_signedness;
    /**
     * @brief   The `ZYDIS_FORMATTER_IMM_PADDING` property.
     */
    ZydisPadding imm_padding;
    /**
     * @brief   The `ZYDIS_FORMATTER_UPPERCASE_PREFIXES` property.
     */
    int32_t case_prefixes;
    /**
     * @brief   The `ZYDIS_FORMATTER_UPPERCASE_MNEMONIC` property.
     */
    int32_t case_mnemonic;
    /**
     * @brief   The `ZYDIS_FORMATTER_UPPERCASE_REGISTERS` property.
     */
    int32_t case_registers;
    /**
     * @brief   The `ZYDIS_FORMATTER_UPPERCASE_TYPECASTS` property.
     */
    int32_t case_typecasts;
    /**
     * @brief   The `ZYDIS_FORMATTER_UPPERCASE_DECORATORS` property.
     */
    int32_t case_decorators;
    /**
     * @brief   The `ZYDIS_FORMATTER_HEX_UPPERCASE` property.
     */
    uint8_t hex_uppercase;
    /**
     * @brief   The number formats for all numeric bases.
     *
     * Index 0 = prefix
     * Index 1 = suffix
     */
    struct
    {
        /**
         * @brief   A pointer to the `ZyanStringView` to use as prefix/suffix.
         */
        const ZyanStringView* string;
        /**
         * @brief   The `ZyanStringView` to use as prefix/suffix
         */
        ZyanStringView string_data;
        /**
         * @brief   The actual string data.
         */
        char buffer[11];
    } number_format[ZYDIS_NUMERIC_BASE_MAX_VALUE + 1][2];
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRE_INSTRUCTION` function.
     */
    ZydisFormatterFunc func_pre_instruction;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_POST_INSTRUCTION` function.
     */
    ZydisFormatterFunc func_post_instruction;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_FORMAT_INSTRUCTION` function.
     */
    ZydisFormatterFunc func_format_instruction;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRE_OPERAND` function.
     */
    ZydisFormatterFunc func_pre_operand;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_POST_OPERAND` function.
     */
    ZydisFormatterFunc func_post_operand;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_FORMAT_OPERAND_REG` function.
     */
    ZydisFormatterFunc func_format_operand_reg;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_FORMAT_OPERAND_MEM` function.
     */
    ZydisFormatterFunc func_format_operand_mem;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_FORMAT_OPERAND_PTR` function.
     */
    ZydisFormatterFunc func_format_operand_ptr;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_FORMAT_OPERAND_IMM` function.
     */
    ZydisFormatterFunc func_format_operand_imm;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRINT_MNEMONIC function.
     */
    ZydisFormatterFunc func_print_mnemonic;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRINT_REGISTER` function.
     */
    ZydisFormatterRegisterFunc func_print_register;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRINT_ADDRESS_ABS` function.
     */
    ZydisFormatterFunc func_print_address_abs;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRINT_ADDRESS_REL` function.
     */
    ZydisFormatterFunc func_print_address_rel;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRINT_DISP` function.
     */
    ZydisFormatterFunc func_print_disp;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRINT_IMM` function.
     */
    ZydisFormatterFunc func_print_imm;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRINT_TYPECAST` function.
     */
    ZydisFormatterFunc func_print_typecast;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRINT_SEGMENT` function.
     */
    ZydisFormatterFunc func_print_segment;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRINT_PREFIXES` function.
     */
    ZydisFormatterFunc func_print_prefixes;
    /**
     * @brief   The `ZYDIS_FORMATTER_FUNC_PRINT_DECORATOR` function.
     */
    ZydisFormatterDecoratorFunc func_print_decorator;

    // ummy padding, in case the structure growth...
    uint32_t    padding[20];
} ZydisFormatter;

/**
 * @brief   Defines the `ZydisMnemonic` enum.
 */
typedef enum ZydisMnemonic_
{
    ZYDIS_MNEMONIC_INVALID,
    ZYDIS_MNEMONIC_AAA,
    ZYDIS_MNEMONIC_AAD,
    ZYDIS_MNEMONIC_AAM,
    ZYDIS_MNEMONIC_AAS,
    ZYDIS_MNEMONIC_ADC,
    ZYDIS_MNEMONIC_ADCX,
    ZYDIS_MNEMONIC_ADD,
    ZYDIS_MNEMONIC_ADDPD,
    ZYDIS_MNEMONIC_ADDPS,
    ZYDIS_MNEMONIC_ADDSD,
    ZYDIS_MNEMONIC_ADDSS,
    ZYDIS_MNEMONIC_ADDSUBPD,
    ZYDIS_MNEMONIC_ADDSUBPS,
    ZYDIS_MNEMONIC_ADOX,
    ZYDIS_MNEMONIC_AESDEC,
    ZYDIS_MNEMONIC_AESDECLAST,
    ZYDIS_MNEMONIC_AESENC,
    ZYDIS_MNEMONIC_AESENCLAST,
    ZYDIS_MNEMONIC_AESIMC,
    ZYDIS_MNEMONIC_AESKEYGENASSIST,
    ZYDIS_MNEMONIC_AND,
    ZYDIS_MNEMONIC_ANDN,
    ZYDIS_MNEMONIC_ANDNPD,
    ZYDIS_MNEMONIC_ANDNPS,
    ZYDIS_MNEMONIC_ANDPD,
    ZYDIS_MNEMONIC_ANDPS,
    ZYDIS_MNEMONIC_ARPL,
    ZYDIS_MNEMONIC_BEXTR,
    ZYDIS_MNEMONIC_BLCFILL,
    ZYDIS_MNEMONIC_BLCI,
    ZYDIS_MNEMONIC_BLCIC,
    ZYDIS_MNEMONIC_BLCMSK,
    ZYDIS_MNEMONIC_BLCS,
    ZYDIS_MNEMONIC_BLENDPD,
    ZYDIS_MNEMONIC_BLENDPS,
    ZYDIS_MNEMONIC_BLENDVPD,
    ZYDIS_MNEMONIC_BLENDVPS,
    ZYDIS_MNEMONIC_BLSFILL,
    ZYDIS_MNEMONIC_BLSI,
    ZYDIS_MNEMONIC_BLSIC,
    ZYDIS_MNEMONIC_BLSMSK,
    ZYDIS_MNEMONIC_BLSR,
    ZYDIS_MNEMONIC_BNDCL,
    ZYDIS_MNEMONIC_BNDCN,
    ZYDIS_MNEMONIC_BNDCU,
    ZYDIS_MNEMONIC_BNDLDX,
    ZYDIS_MNEMONIC_BNDMK,
    ZYDIS_MNEMONIC_BNDMOV,
    ZYDIS_MNEMONIC_BNDSTX,
    ZYDIS_MNEMONIC_BOUND,
    ZYDIS_MNEMONIC_BSF,
    ZYDIS_MNEMONIC_BSR,
    ZYDIS_MNEMONIC_BSWAP,
    ZYDIS_MNEMONIC_BT,
    ZYDIS_MNEMONIC_BTC,
    ZYDIS_MNEMONIC_BTR,
    ZYDIS_MNEMONIC_BTS,
    ZYDIS_MNEMONIC_BZHI,
    ZYDIS_MNEMONIC_CALL,
    ZYDIS_MNEMONIC_CBW,
    ZYDIS_MNEMONIC_CDQ,
    ZYDIS_MNEMONIC_CDQE,
    ZYDIS_MNEMONIC_CLAC,
    ZYDIS_MNEMONIC_CLC,
    ZYDIS_MNEMONIC_CLD,
    ZYDIS_MNEMONIC_CLDEMOTE,
    ZYDIS_MNEMONIC_CLEVICT0,
    ZYDIS_MNEMONIC_CLEVICT1,
    ZYDIS_MNEMONIC_CLFLUSH,
    ZYDIS_MNEMONIC_CLFLUSHOPT,
    ZYDIS_MNEMONIC_CLGI,
    ZYDIS_MNEMONIC_CLI,
    ZYDIS_MNEMONIC_CLRSSBSY,
    ZYDIS_MNEMONIC_CLTS,
    ZYDIS_MNEMONIC_CLWB,
    ZYDIS_MNEMONIC_CLZERO,
    ZYDIS_MNEMONIC_CMC,
    ZYDIS_MNEMONIC_CMOVB,
    ZYDIS_MNEMONIC_CMOVBE,
    ZYDIS_MNEMONIC_CMOVL,
    ZYDIS_MNEMONIC_CMOVLE,
    ZYDIS_MNEMONIC_CMOVNB,
    ZYDIS_MNEMONIC_CMOVNBE,
    ZYDIS_MNEMONIC_CMOVNL,
    ZYDIS_MNEMONIC_CMOVNLE,
    ZYDIS_MNEMONIC_CMOVNO,
    ZYDIS_MNEMONIC_CMOVNP,
    ZYDIS_MNEMONIC_CMOVNS,
    ZYDIS_MNEMONIC_CMOVNZ,
    ZYDIS_MNEMONIC_CMOVO,
    ZYDIS_MNEMONIC_CMOVP,
    ZYDIS_MNEMONIC_CMOVS,
    ZYDIS_MNEMONIC_CMOVZ,
    ZYDIS_MNEMONIC_CMP,
    ZYDIS_MNEMONIC_CMPPD,
    ZYDIS_MNEMONIC_CMPPS,
    ZYDIS_MNEMONIC_CMPSB,
    ZYDIS_MNEMONIC_CMPSD,
    ZYDIS_MNEMONIC_CMPSQ,
    ZYDIS_MNEMONIC_CMPSS,
    ZYDIS_MNEMONIC_CMPSW,
    ZYDIS_MNEMONIC_CMPXCHG,
    ZYDIS_MNEMONIC_CMPXCHG16B,
    ZYDIS_MNEMONIC_CMPXCHG8B,
    ZYDIS_MNEMONIC_COMISD,
    ZYDIS_MNEMONIC_COMISS,
    ZYDIS_MNEMONIC_CPUID,
    ZYDIS_MNEMONIC_CQO,
    ZYDIS_MNEMONIC_CRC32,
    ZYDIS_MNEMONIC_CVTDQ2PD,
    ZYDIS_MNEMONIC_CVTDQ2PS,
    ZYDIS_MNEMONIC_CVTPD2DQ,
    ZYDIS_MNEMONIC_CVTPD2PI,
    ZYDIS_MNEMONIC_CVTPD2PS,
    ZYDIS_MNEMONIC_CVTPI2PD,
    ZYDIS_MNEMONIC_CVTPI2PS,
    ZYDIS_MNEMONIC_CVTPS2DQ,
    ZYDIS_MNEMONIC_CVTPS2PD,
    ZYDIS_MNEMONIC_CVTPS2PI,
    ZYDIS_MNEMONIC_CVTSD2SI,
    ZYDIS_MNEMONIC_CVTSD2SS,
    ZYDIS_MNEMONIC_CVTSI2SD,
    ZYDIS_MNEMONIC_CVTSI2SS,
    ZYDIS_MNEMONIC_CVTSS2SD,
    ZYDIS_MNEMONIC_CVTSS2SI,
    ZYDIS_MNEMONIC_CVTTPD2DQ,
    ZYDIS_MNEMONIC_CVTTPD2PI,
    ZYDIS_MNEMONIC_CVTTPS2DQ,
    ZYDIS_MNEMONIC_CVTTPS2PI,
    ZYDIS_MNEMONIC_CVTTSD2SI,
    ZYDIS_MNEMONIC_CVTTSS2SI,
    ZYDIS_MNEMONIC_CWD,
    ZYDIS_MNEMONIC_CWDE,
    ZYDIS_MNEMONIC_DAA,
    ZYDIS_MNEMONIC_DAS,
    ZYDIS_MNEMONIC_DEC,
    ZYDIS_MNEMONIC_DELAY,
    ZYDIS_MNEMONIC_DIV,
    ZYDIS_MNEMONIC_DIVPD,
    ZYDIS_MNEMONIC_DIVPS,
    ZYDIS_MNEMONIC_DIVSD,
    ZYDIS_MNEMONIC_DIVSS,
    ZYDIS_MNEMONIC_DPPD,
    ZYDIS_MNEMONIC_DPPS,
    ZYDIS_MNEMONIC_EMMS,
    ZYDIS_MNEMONIC_ENCLS,
    ZYDIS_MNEMONIC_ENCLU,
    ZYDIS_MNEMONIC_ENCLV,
    ZYDIS_MNEMONIC_ENDBR32,
    ZYDIS_MNEMONIC_ENDBR64,
    ZYDIS_MNEMONIC_ENTER,
    ZYDIS_MNEMONIC_EXTRACTPS,
    ZYDIS_MNEMONIC_EXTRQ,
    ZYDIS_MNEMONIC_F2XM1,
    ZYDIS_MNEMONIC_FABS,
    ZYDIS_MNEMONIC_FADD,
    ZYDIS_MNEMONIC_FADDP,
    ZYDIS_MNEMONIC_FBLD,
    ZYDIS_MNEMONIC_FBSTP,
    ZYDIS_MNEMONIC_FCHS,
    ZYDIS_MNEMONIC_FCMOVB,
    ZYDIS_MNEMONIC_FCMOVBE,
    ZYDIS_MNEMONIC_FCMOVE,
    ZYDIS_MNEMONIC_FCMOVNB,
    ZYDIS_MNEMONIC_FCMOVNBE,
    ZYDIS_MNEMONIC_FCMOVNE,
    ZYDIS_MNEMONIC_FCMOVNU,
    ZYDIS_MNEMONIC_FCMOVU,
    ZYDIS_MNEMONIC_FCOM,
    ZYDIS_MNEMONIC_FCOMI,
    ZYDIS_MNEMONIC_FCOMIP,
    ZYDIS_MNEMONIC_FCOMP,
    ZYDIS_MNEMONIC_FCOMPP,
    ZYDIS_MNEMONIC_FCOS,
    ZYDIS_MNEMONIC_FDECSTP,
    ZYDIS_MNEMONIC_FDISI8087_NOP,
    ZYDIS_MNEMONIC_FDIV,
    ZYDIS_MNEMONIC_FDIVP,
    ZYDIS_MNEMONIC_FDIVR,
    ZYDIS_MNEMONIC_FDIVRP,
    ZYDIS_MNEMONIC_FEMMS,
    ZYDIS_MNEMONIC_FENI8087_NOP,
    ZYDIS_MNEMONIC_FFREE,
    ZYDIS_MNEMONIC_FFREEP,
    ZYDIS_MNEMONIC_FIADD,
    ZYDIS_MNEMONIC_FICOM,
    ZYDIS_MNEMONIC_FICOMP,
    ZYDIS_MNEMONIC_FIDIV,
    ZYDIS_MNEMONIC_FIDIVR,
    ZYDIS_MNEMONIC_FILD,
    ZYDIS_MNEMONIC_FIMUL,
    ZYDIS_MNEMONIC_FINCSTP,
    ZYDIS_MNEMONIC_FIST,
    ZYDIS_MNEMONIC_FISTP,
    ZYDIS_MNEMONIC_FISTTP,
    ZYDIS_MNEMONIC_FISUB,
    ZYDIS_MNEMONIC_FISUBR,
    ZYDIS_MNEMONIC_FLD,
    ZYDIS_MNEMONIC_FLD1,
    ZYDIS_MNEMONIC_FLDCW,
    ZYDIS_MNEMONIC_FLDENV,
    ZYDIS_MNEMONIC_FLDL2E,
    ZYDIS_MNEMONIC_FLDL2T,
    ZYDIS_MNEMONIC_FLDLG2,
    ZYDIS_MNEMONIC_FLDLN2,
    ZYDIS_MNEMONIC_FLDPI,
    ZYDIS_MNEMONIC_FLDZ,
    ZYDIS_MNEMONIC_FMUL,
    ZYDIS_MNEMONIC_FMULP,
    ZYDIS_MNEMONIC_FNCLEX,
    ZYDIS_MNEMONIC_FNINIT,
    ZYDIS_MNEMONIC_FNOP,
    ZYDIS_MNEMONIC_FNSAVE,
    ZYDIS_MNEMONIC_FNSTCW,
    ZYDIS_MNEMONIC_FNSTENV,
    ZYDIS_MNEMONIC_FNSTSW,
    ZYDIS_MNEMONIC_FPATAN,
    ZYDIS_MNEMONIC_FPREM,
    ZYDIS_MNEMONIC_FPREM1,
    ZYDIS_MNEMONIC_FPTAN,
    ZYDIS_MNEMONIC_FRNDINT,
    ZYDIS_MNEMONIC_FRSTOR,
    ZYDIS_MNEMONIC_FSCALE,
    ZYDIS_MNEMONIC_FSETPM287_NOP,
    ZYDIS_MNEMONIC_FSIN,
    ZYDIS_MNEMONIC_FSINCOS,
    ZYDIS_MNEMONIC_FSQRT,
    ZYDIS_MNEMONIC_FST,
    ZYDIS_MNEMONIC_FSTP,
    ZYDIS_MNEMONIC_FSTPNCE,
    ZYDIS_MNEMONIC_FSUB,
    ZYDIS_MNEMONIC_FSUBP,
    ZYDIS_MNEMONIC_FSUBR,
    ZYDIS_MNEMONIC_FSUBRP,
    ZYDIS_MNEMONIC_FTST,
    ZYDIS_MNEMONIC_FUCOM,
    ZYDIS_MNEMONIC_FUCOMI,
    ZYDIS_MNEMONIC_FUCOMIP,
    ZYDIS_MNEMONIC_FUCOMP,
    ZYDIS_MNEMONIC_FUCOMPP,
    ZYDIS_MNEMONIC_FWAIT,
    ZYDIS_MNEMONIC_FXAM,
    ZYDIS_MNEMONIC_FXCH,
    ZYDIS_MNEMONIC_FXRSTOR,
    ZYDIS_MNEMONIC_FXRSTOR64,
    ZYDIS_MNEMONIC_FXSAVE,
    ZYDIS_MNEMONIC_FXSAVE64,
    ZYDIS_MNEMONIC_FXTRACT,
    ZYDIS_MNEMONIC_FYL2X,
    ZYDIS_MNEMONIC_FYL2XP1,
    ZYDIS_MNEMONIC_GETSEC,
    ZYDIS_MNEMONIC_GF2P8AFFINEINVQB,
    ZYDIS_MNEMONIC_GF2P8AFFINEQB,
    ZYDIS_MNEMONIC_GF2P8MULB,
    ZYDIS_MNEMONIC_HADDPD,
    ZYDIS_MNEMONIC_HADDPS,
    ZYDIS_MNEMONIC_HLT,
    ZYDIS_MNEMONIC_HSUBPD,
    ZYDIS_MNEMONIC_HSUBPS,
    ZYDIS_MNEMONIC_IDIV,
    ZYDIS_MNEMONIC_IMUL,
    ZYDIS_MNEMONIC_IN,
    ZYDIS_MNEMONIC_INC,
    ZYDIS_MNEMONIC_INCSSPD,
    ZYDIS_MNEMONIC_INCSSPQ,
    ZYDIS_MNEMONIC_INSB,
    ZYDIS_MNEMONIC_INSD,
    ZYDIS_MNEMONIC_INSERTPS,
    ZYDIS_MNEMONIC_INSERTQ,
    ZYDIS_MNEMONIC_INSW,
    ZYDIS_MNEMONIC_INT,
    ZYDIS_MNEMONIC_INT1,
    ZYDIS_MNEMONIC_INT3,
    ZYDIS_MNEMONIC_INTO,
    ZYDIS_MNEMONIC_INVD,
    ZYDIS_MNEMONIC_INVEPT,
    ZYDIS_MNEMONIC_INVLPG,
    ZYDIS_MNEMONIC_INVLPGA,
    ZYDIS_MNEMONIC_INVPCID,
    ZYDIS_MNEMONIC_INVVPID,
    ZYDIS_MNEMONIC_IRET,
    ZYDIS_MNEMONIC_IRETD,
    ZYDIS_MNEMONIC_IRETQ,
    ZYDIS_MNEMONIC_JB,
    ZYDIS_MNEMONIC_JBE,
    ZYDIS_MNEMONIC_JCXZ,
    ZYDIS_MNEMONIC_JECXZ,
    ZYDIS_MNEMONIC_JKNZD,
    ZYDIS_MNEMONIC_JKZD,
    ZYDIS_MNEMONIC_JL,
    ZYDIS_MNEMONIC_JLE,
    ZYDIS_MNEMONIC_JMP,
    ZYDIS_MNEMONIC_JNB,
    ZYDIS_MNEMONIC_JNBE,
    ZYDIS_MNEMONIC_JNL,
    ZYDIS_MNEMONIC_JNLE,
    ZYDIS_MNEMONIC_JNO,
    ZYDIS_MNEMONIC_JNP,
    ZYDIS_MNEMONIC_JNS,
    ZYDIS_MNEMONIC_JNZ,
    ZYDIS_MNEMONIC_JO,
    ZYDIS_MNEMONIC_JP,
    ZYDIS_MNEMONIC_JRCXZ,
    ZYDIS_MNEMONIC_JS,
    ZYDIS_MNEMONIC_JZ,
    ZYDIS_MNEMONIC_KADDB,
    ZYDIS_MNEMONIC_KADDD,
    ZYDIS_MNEMONIC_KADDQ,
    ZYDIS_MNEMONIC_KADDW,
    ZYDIS_MNEMONIC_KAND,
    ZYDIS_MNEMONIC_KANDB,
    ZYDIS_MNEMONIC_KANDD,
    ZYDIS_MNEMONIC_KANDN,
    ZYDIS_MNEMONIC_KANDNB,
    ZYDIS_MNEMONIC_KANDND,
    ZYDIS_MNEMONIC_KANDNQ,
    ZYDIS_MNEMONIC_KANDNR,
    ZYDIS_MNEMONIC_KANDNW,
    ZYDIS_MNEMONIC_KANDQ,
    ZYDIS_MNEMONIC_KANDW,
    ZYDIS_MNEMONIC_KCONCATH,
    ZYDIS_MNEMONIC_KCONCATL,
    ZYDIS_MNEMONIC_KEXTRACT,
    ZYDIS_MNEMONIC_KMERGE2L1H,
    ZYDIS_MNEMONIC_KMERGE2L1L,
    ZYDIS_MNEMONIC_KMOV,
    ZYDIS_MNEMONIC_KMOVB,
    ZYDIS_MNEMONIC_KMOVD,
    ZYDIS_MNEMONIC_KMOVQ,
    ZYDIS_MNEMONIC_KMOVW,
    ZYDIS_MNEMONIC_KNOT,
    ZYDIS_MNEMONIC_KNOTB,
    ZYDIS_MNEMONIC_KNOTD,
    ZYDIS_MNEMONIC_KNOTQ,
    ZYDIS_MNEMONIC_KNOTW,
    ZYDIS_MNEMONIC_KOR,
    ZYDIS_MNEMONIC_KORB,
    ZYDIS_MNEMONIC_KORD,
    ZYDIS_MNEMONIC_KORQ,
    ZYDIS_MNEMONIC_KORTEST,
    ZYDIS_MNEMONIC_KORTESTB,
    ZYDIS_MNEMONIC_KORTESTD,
    ZYDIS_MNEMONIC_KORTESTQ,
    ZYDIS_MNEMONIC_KORTESTW,
    ZYDIS_MNEMONIC_KORW,
    ZYDIS_MNEMONIC_KSHIFTLB,
    ZYDIS_MNEMONIC_KSHIFTLD,
    ZYDIS_MNEMONIC_KSHIFTLQ,
    ZYDIS_MNEMONIC_KSHIFTLW,
    ZYDIS_MNEMONIC_KSHIFTRB,
    ZYDIS_MNEMONIC_KSHIFTRD,
    ZYDIS_MNEMONIC_KSHIFTRQ,
    ZYDIS_MNEMONIC_KSHIFTRW,
    ZYDIS_MNEMONIC_KTESTB,
    ZYDIS_MNEMONIC_KTESTD,
    ZYDIS_MNEMONIC_KTESTQ,
    ZYDIS_MNEMONIC_KTESTW,
    ZYDIS_MNEMONIC_KUNPCKBW,
    ZYDIS_MNEMONIC_KUNPCKDQ,
    ZYDIS_MNEMONIC_KUNPCKWD,
    ZYDIS_MNEMONIC_KXNOR,
    ZYDIS_MNEMONIC_KXNORB,
    ZYDIS_MNEMONIC_KXNORD,
    ZYDIS_MNEMONIC_KXNORQ,
    ZYDIS_MNEMONIC_KXNORW,
    ZYDIS_MNEMONIC_KXOR,
    ZYDIS_MNEMONIC_KXORB,
    ZYDIS_MNEMONIC_KXORD,
    ZYDIS_MNEMONIC_KXORQ,
    ZYDIS_MNEMONIC_KXORW,
    ZYDIS_MNEMONIC_LAHF,
    ZYDIS_MNEMONIC_LAR,
    ZYDIS_MNEMONIC_LDDQU,
    ZYDIS_MNEMONIC_LDMXCSR,
    ZYDIS_MNEMONIC_LDS,
    ZYDIS_MNEMONIC_LEA,
    ZYDIS_MNEMONIC_LEAVE,
    ZYDIS_MNEMONIC_LES,
    ZYDIS_MNEMONIC_LFENCE,
    ZYDIS_MNEMONIC_LFS,
    ZYDIS_MNEMONIC_LGDT,
    ZYDIS_MNEMONIC_LGS,
    ZYDIS_MNEMONIC_LIDT,
    ZYDIS_MNEMONIC_LLDT,
    ZYDIS_MNEMONIC_LLWPCB,
    ZYDIS_MNEMONIC_LMSW,
    ZYDIS_MNEMONIC_LODSB,
    ZYDIS_MNEMONIC_LODSD,
    ZYDIS_MNEMONIC_LODSQ,
    ZYDIS_MNEMONIC_LODSW,
    ZYDIS_MNEMONIC_LOOP,
    ZYDIS_MNEMONIC_LOOPE,
    ZYDIS_MNEMONIC_LOOPNE,
    ZYDIS_MNEMONIC_LSL,
    ZYDIS_MNEMONIC_LSS,
    ZYDIS_MNEMONIC_LTR,
    ZYDIS_MNEMONIC_LWPINS,
    ZYDIS_MNEMONIC_LWPVAL,
    ZYDIS_MNEMONIC_LZCNT,
    ZYDIS_MNEMONIC_MASKMOVDQU,
    ZYDIS_MNEMONIC_MASKMOVQ,
    ZYDIS_MNEMONIC_MAXPD,
    ZYDIS_MNEMONIC_MAXPS,
    ZYDIS_MNEMONIC_MAXSD,
    ZYDIS_MNEMONIC_MAXSS,
    ZYDIS_MNEMONIC_MFENCE,
    ZYDIS_MNEMONIC_MINPD,
    ZYDIS_MNEMONIC_MINPS,
    ZYDIS_MNEMONIC_MINSD,
    ZYDIS_MNEMONIC_MINSS,
    ZYDIS_MNEMONIC_MONITOR,
    ZYDIS_MNEMONIC_MONITORX,
    ZYDIS_MNEMONIC_MONTMUL,
    ZYDIS_MNEMONIC_MOV,
    ZYDIS_MNEMONIC_MOVAPD,
    ZYDIS_MNEMONIC_MOVAPS,
    ZYDIS_MNEMONIC_MOVBE,
    ZYDIS_MNEMONIC_MOVD,
    ZYDIS_MNEMONIC_MOVDDUP,
    ZYDIS_MNEMONIC_MOVDIR64B,
    ZYDIS_MNEMONIC_MOVDIRI,
    ZYDIS_MNEMONIC_MOVDQ2Q,
    ZYDIS_MNEMONIC_MOVDQA,
    ZYDIS_MNEMONIC_MOVDQU,
    ZYDIS_MNEMONIC_MOVHLPS,
    ZYDIS_MNEMONIC_MOVHPD,
    ZYDIS_MNEMONIC_MOVHPS,
    ZYDIS_MNEMONIC_MOVLHPS,
    ZYDIS_MNEMONIC_MOVLPD,
    ZYDIS_MNEMONIC_MOVLPS,
    ZYDIS_MNEMONIC_MOVMSKPD,
    ZYDIS_MNEMONIC_MOVMSKPS,
    ZYDIS_MNEMONIC_MOVNTDQ,
    ZYDIS_MNEMONIC_MOVNTDQA,
    ZYDIS_MNEMONIC_MOVNTI,
    ZYDIS_MNEMONIC_MOVNTPD,
    ZYDIS_MNEMONIC_MOVNTPS,
    ZYDIS_MNEMONIC_MOVNTQ,
    ZYDIS_MNEMONIC_MOVNTSD,
    ZYDIS_MNEMONIC_MOVNTSS,
    ZYDIS_MNEMONIC_MOVQ,
    ZYDIS_MNEMONIC_MOVQ2DQ,
    ZYDIS_MNEMONIC_MOVSB,
    ZYDIS_MNEMONIC_MOVSD,
    ZYDIS_MNEMONIC_MOVSHDUP,
    ZYDIS_MNEMONIC_MOVSLDUP,
    ZYDIS_MNEMONIC_MOVSQ,
    ZYDIS_MNEMONIC_MOVSS,
    ZYDIS_MNEMONIC_MOVSW,
    ZYDIS_MNEMONIC_MOVSX,
    ZYDIS_MNEMONIC_MOVSXD,
    ZYDIS_MNEMONIC_MOVUPD,
    ZYDIS_MNEMONIC_MOVUPS,
    ZYDIS_MNEMONIC_MOVZX,
    ZYDIS_MNEMONIC_MPSADBW,
    ZYDIS_MNEMONIC_MUL,
    ZYDIS_MNEMONIC_MULPD,
    ZYDIS_MNEMONIC_MULPS,
    ZYDIS_MNEMONIC_MULSD,
    ZYDIS_MNEMONIC_MULSS,
    ZYDIS_MNEMONIC_MULX,
    ZYDIS_MNEMONIC_MWAIT,
    ZYDIS_MNEMONIC_MWAITX,
    ZYDIS_MNEMONIC_NEG,
    ZYDIS_MNEMONIC_NOP,
    ZYDIS_MNEMONIC_NOT,
    ZYDIS_MNEMONIC_OR,
    ZYDIS_MNEMONIC_ORPD,
    ZYDIS_MNEMONIC_ORPS,
    ZYDIS_MNEMONIC_OUT,
    ZYDIS_MNEMONIC_OUTSB,
    ZYDIS_MNEMONIC_OUTSD,
    ZYDIS_MNEMONIC_OUTSW,
    ZYDIS_MNEMONIC_PABSB,
    ZYDIS_MNEMONIC_PABSD,
    ZYDIS_MNEMONIC_PABSW,
    ZYDIS_MNEMONIC_PACKSSDW,
    ZYDIS_MNEMONIC_PACKSSWB,
    ZYDIS_MNEMONIC_PACKUSDW,
    ZYDIS_MNEMONIC_PACKUSWB,
    ZYDIS_MNEMONIC_PADDB,
    ZYDIS_MNEMONIC_PADDD,
    ZYDIS_MNEMONIC_PADDQ,
    ZYDIS_MNEMONIC_PADDSB,
    ZYDIS_MNEMONIC_PADDSW,
    ZYDIS_MNEMONIC_PADDUSB,
    ZYDIS_MNEMONIC_PADDUSW,
    ZYDIS_MNEMONIC_PADDW,
    ZYDIS_MNEMONIC_PALIGNR,
    ZYDIS_MNEMONIC_PAND,
    ZYDIS_MNEMONIC_PANDN,
    ZYDIS_MNEMONIC_PAUSE,
    ZYDIS_MNEMONIC_PAVGB,
    ZYDIS_MNEMONIC_PAVGUSB,
    ZYDIS_MNEMONIC_PAVGW,
    ZYDIS_MNEMONIC_PBLENDVB,
    ZYDIS_MNEMONIC_PBLENDW,
    ZYDIS_MNEMONIC_PCLMULQDQ,
    ZYDIS_MNEMONIC_PCMPEQB,
    ZYDIS_MNEMONIC_PCMPEQD,
    ZYDIS_MNEMONIC_PCMPEQQ,
    ZYDIS_MNEMONIC_PCMPEQW,
    ZYDIS_MNEMONIC_PCMPESTRI,
    ZYDIS_MNEMONIC_PCMPESTRM,
    ZYDIS_MNEMONIC_PCMPGTB,
    ZYDIS_MNEMONIC_PCMPGTD,
    ZYDIS_MNEMONIC_PCMPGTQ,
    ZYDIS_MNEMONIC_PCMPGTW,
    ZYDIS_MNEMONIC_PCMPISTRI,
    ZYDIS_MNEMONIC_PCMPISTRM,
    ZYDIS_MNEMONIC_PCONFIG,
    ZYDIS_MNEMONIC_PDEP,
    ZYDIS_MNEMONIC_PEXT,
    ZYDIS_MNEMONIC_PEXTRB,
    ZYDIS_MNEMONIC_PEXTRD,
    ZYDIS_MNEMONIC_PEXTRQ,
    ZYDIS_MNEMONIC_PEXTRW,
    ZYDIS_MNEMONIC_PF2ID,
    ZYDIS_MNEMONIC_PF2IW,
    ZYDIS_MNEMONIC_PFACC,
    ZYDIS_MNEMONIC_PFADD,
    ZYDIS_MNEMONIC_PFCMPEQ,
    ZYDIS_MNEMONIC_PFCMPGE,
    ZYDIS_MNEMONIC_PFCMPGT,
    ZYDIS_MNEMONIC_PFCPIT1,
    ZYDIS_MNEMONIC_PFMAX,
    ZYDIS_MNEMONIC_PFMIN,
    ZYDIS_MNEMONIC_PFMUL,
    ZYDIS_MNEMONIC_PFNACC,
    ZYDIS_MNEMONIC_PFPNACC,
    ZYDIS_MNEMONIC_PFRCP,
    ZYDIS_MNEMONIC_PFRCPIT2,
    ZYDIS_MNEMONIC_PFRSQIT1,
    ZYDIS_MNEMONIC_PFSQRT,
    ZYDIS_MNEMONIC_PFSUB,
    ZYDIS_MNEMONIC_PFSUBR,
    ZYDIS_MNEMONIC_PHADDD,
    ZYDIS_MNEMONIC_PHADDSW,
    ZYDIS_MNEMONIC_PHADDW,
    ZYDIS_MNEMONIC_PHMINPOSUW,
    ZYDIS_MNEMONIC_PHSUBD,
    ZYDIS_MNEMONIC_PHSUBSW,
    ZYDIS_MNEMONIC_PHSUBW,
    ZYDIS_MNEMONIC_PI2FD,
    ZYDIS_MNEMONIC_PI2FW,
    ZYDIS_MNEMONIC_PINSRB,
    ZYDIS_MNEMONIC_PINSRD,
    ZYDIS_MNEMONIC_PINSRQ,
    ZYDIS_MNEMONIC_PINSRW,
    ZYDIS_MNEMONIC_PMADDUBSW,
    ZYDIS_MNEMONIC_PMADDWD,
    ZYDIS_MNEMONIC_PMAXSB,
    ZYDIS_MNEMONIC_PMAXSD,
    ZYDIS_MNEMONIC_PMAXSW,
    ZYDIS_MNEMONIC_PMAXUB,
    ZYDIS_MNEMONIC_PMAXUD,
    ZYDIS_MNEMONIC_PMAXUW,
    ZYDIS_MNEMONIC_PMINSB,
    ZYDIS_MNEMONIC_PMINSD,
    ZYDIS_MNEMONIC_PMINSW,
    ZYDIS_MNEMONIC_PMINUB,
    ZYDIS_MNEMONIC_PMINUD,
    ZYDIS_MNEMONIC_PMINUW,
    ZYDIS_MNEMONIC_PMOVMSKB,
    ZYDIS_MNEMONIC_PMOVSXBD,
    ZYDIS_MNEMONIC_PMOVSXBQ,
    ZYDIS_MNEMONIC_PMOVSXBW,
    ZYDIS_MNEMONIC_PMOVSXDQ,
    ZYDIS_MNEMONIC_PMOVSXWD,
    ZYDIS_MNEMONIC_PMOVSXWQ,
    ZYDIS_MNEMONIC_PMOVZXBD,
    ZYDIS_MNEMONIC_PMOVZXBQ,
    ZYDIS_MNEMONIC_PMOVZXBW,
    ZYDIS_MNEMONIC_PMOVZXDQ,
    ZYDIS_MNEMONIC_PMOVZXWD,
    ZYDIS_MNEMONIC_PMOVZXWQ,
    ZYDIS_MNEMONIC_PMULDQ,
    ZYDIS_MNEMONIC_PMULHRSW,
    ZYDIS_MNEMONIC_PMULHRW,
    ZYDIS_MNEMONIC_PMULHUW,
    ZYDIS_MNEMONIC_PMULHW,
    ZYDIS_MNEMONIC_PMULLD,
    ZYDIS_MNEMONIC_PMULLW,
    ZYDIS_MNEMONIC_PMULUDQ,
    ZYDIS_MNEMONIC_POP,
    ZYDIS_MNEMONIC_POPA,
    ZYDIS_MNEMONIC_POPAD,
    ZYDIS_MNEMONIC_POPCNT,
    ZYDIS_MNEMONIC_POPF,
    ZYDIS_MNEMONIC_POPFD,
    ZYDIS_MNEMONIC_POPFQ,
    ZYDIS_MNEMONIC_POR,
    ZYDIS_MNEMONIC_PREFETCH,
    ZYDIS_MNEMONIC_PREFETCHNTA,
    ZYDIS_MNEMONIC_PREFETCHT0,
    ZYDIS_MNEMONIC_PREFETCHT1,
    ZYDIS_MNEMONIC_PREFETCHT2,
    ZYDIS_MNEMONIC_PREFETCHW,
    ZYDIS_MNEMONIC_PREFETCHWT1,
    ZYDIS_MNEMONIC_PSADBW,
    ZYDIS_MNEMONIC_PSHUFB,
    ZYDIS_MNEMONIC_PSHUFD,
    ZYDIS_MNEMONIC_PSHUFHW,
    ZYDIS_MNEMONIC_PSHUFLW,
    ZYDIS_MNEMONIC_PSHUFW,
    ZYDIS_MNEMONIC_PSIGNB,
    ZYDIS_MNEMONIC_PSIGND,
    ZYDIS_MNEMONIC_PSIGNW,
    ZYDIS_MNEMONIC_PSLLD,
    ZYDIS_MNEMONIC_PSLLDQ,
    ZYDIS_MNEMONIC_PSLLQ,
    ZYDIS_MNEMONIC_PSLLW,
    ZYDIS_MNEMONIC_PSRAD,
    ZYDIS_MNEMONIC_PSRAW,
    ZYDIS_MNEMONIC_PSRLD,
    ZYDIS_MNEMONIC_PSRLDQ,
    ZYDIS_MNEMONIC_PSRLQ,
    ZYDIS_MNEMONIC_PSRLW,
    ZYDIS_MNEMONIC_PSUBB,
    ZYDIS_MNEMONIC_PSUBD,
    ZYDIS_MNEMONIC_PSUBQ,
    ZYDIS_MNEMONIC_PSUBSB,
    ZYDIS_MNEMONIC_PSUBSW,
    ZYDIS_MNEMONIC_PSUBUSB,
    ZYDIS_MNEMONIC_PSUBUSW,
    ZYDIS_MNEMONIC_PSUBW,
    ZYDIS_MNEMONIC_PSWAPD,
    ZYDIS_MNEMONIC_PTEST,
    ZYDIS_MNEMONIC_PTWRITE,
    ZYDIS_MNEMONIC_PUNPCKHBW,
    ZYDIS_MNEMONIC_PUNPCKHDQ,
    ZYDIS_MNEMONIC_PUNPCKHQDQ,
    ZYDIS_MNEMONIC_PUNPCKHWD,
    ZYDIS_MNEMONIC_PUNPCKLBW,
    ZYDIS_MNEMONIC_PUNPCKLDQ,
    ZYDIS_MNEMONIC_PUNPCKLQDQ,
    ZYDIS_MNEMONIC_PUNPCKLWD,
    ZYDIS_MNEMONIC_PUSH,
    ZYDIS_MNEMONIC_PUSHA,
    ZYDIS_MNEMONIC_PUSHAD,
    ZYDIS_MNEMONIC_PUSHF,
    ZYDIS_MNEMONIC_PUSHFD,
    ZYDIS_MNEMONIC_PUSHFQ,
    ZYDIS_MNEMONIC_PXOR,
    ZYDIS_MNEMONIC_RCL,
    ZYDIS_MNEMONIC_RCPPS,
    ZYDIS_MNEMONIC_RCPSS,
    ZYDIS_MNEMONIC_RCR,
    ZYDIS_MNEMONIC_RDFSBASE,
    ZYDIS_MNEMONIC_RDGSBASE,
    ZYDIS_MNEMONIC_RDMSR,
    ZYDIS_MNEMONIC_RDPID,
    ZYDIS_MNEMONIC_RDPKRU,
    ZYDIS_MNEMONIC_RDPMC,
    ZYDIS_MNEMONIC_RDRAND,
    ZYDIS_MNEMONIC_RDSEED,
    ZYDIS_MNEMONIC_RDSSPD,
    ZYDIS_MNEMONIC_RDSSPQ,
    ZYDIS_MNEMONIC_RDTSC,
    ZYDIS_MNEMONIC_RDTSCP,
    ZYDIS_MNEMONIC_RET,
    ZYDIS_MNEMONIC_ROL,
    ZYDIS_MNEMONIC_ROR,
    ZYDIS_MNEMONIC_RORX,
    ZYDIS_MNEMONIC_ROUNDPD,
    ZYDIS_MNEMONIC_ROUNDPS,
    ZYDIS_MNEMONIC_ROUNDSD,
    ZYDIS_MNEMONIC_ROUNDSS,
    ZYDIS_MNEMONIC_RSM,
    ZYDIS_MNEMONIC_RSQRTPS,
    ZYDIS_MNEMONIC_RSQRTSS,
    ZYDIS_MNEMONIC_RSTORSSP,
    ZYDIS_MNEMONIC_SAHF,
    ZYDIS_MNEMONIC_SALC,
    ZYDIS_MNEMONIC_SAR,
    ZYDIS_MNEMONIC_SARX,
    ZYDIS_MNEMONIC_SAVEPREVSSP,
    ZYDIS_MNEMONIC_SBB,
    ZYDIS_MNEMONIC_SCASB,
    ZYDIS_MNEMONIC_SCASD,
    ZYDIS_MNEMONIC_SCASQ,
    ZYDIS_MNEMONIC_SCASW,
    ZYDIS_MNEMONIC_SETB,
    ZYDIS_MNEMONIC_SETBE,
    ZYDIS_MNEMONIC_SETL,
    ZYDIS_MNEMONIC_SETLE,
    ZYDIS_MNEMONIC_SETNB,
    ZYDIS_MNEMONIC_SETNBE,
    ZYDIS_MNEMONIC_SETNL,
    ZYDIS_MNEMONIC_SETNLE,
    ZYDIS_MNEMONIC_SETNO,
    ZYDIS_MNEMONIC_SETNP,
    ZYDIS_MNEMONIC_SETNS,
    ZYDIS_MNEMONIC_SETNZ,
    ZYDIS_MNEMONIC_SETO,
    ZYDIS_MNEMONIC_SETP,
    ZYDIS_MNEMONIC_SETS,
    ZYDIS_MNEMONIC_SETSSBSY,
    ZYDIS_MNEMONIC_SETZ,
    ZYDIS_MNEMONIC_SFENCE,
    ZYDIS_MNEMONIC_SGDT,
    ZYDIS_MNEMONIC_SHA1MSG1,
    ZYDIS_MNEMONIC_SHA1MSG2,
    ZYDIS_MNEMONIC_SHA1NEXTE,
    ZYDIS_MNEMONIC_SHA1RNDS4,
    ZYDIS_MNEMONIC_SHA256MSG1,
    ZYDIS_MNEMONIC_SHA256MSG2,
    ZYDIS_MNEMONIC_SHA256RNDS2,
    ZYDIS_MNEMONIC_SHL,
    ZYDIS_MNEMONIC_SHLD,
    ZYDIS_MNEMONIC_SHLX,
    ZYDIS_MNEMONIC_SHR,
    ZYDIS_MNEMONIC_SHRD,
    ZYDIS_MNEMONIC_SHRX,
    ZYDIS_MNEMONIC_SHUFPD,
    ZYDIS_MNEMONIC_SHUFPS,
    ZYDIS_MNEMONIC_SIDT,
    ZYDIS_MNEMONIC_SKINIT,
    ZYDIS_MNEMONIC_SLDT,
    ZYDIS_MNEMONIC_SLWPCB,
    ZYDIS_MNEMONIC_SMSW,
    ZYDIS_MNEMONIC_SPFLT,
    ZYDIS_MNEMONIC_SQRTPD,
    ZYDIS_MNEMONIC_SQRTPS,
    ZYDIS_MNEMONIC_SQRTSD,
    ZYDIS_MNEMONIC_SQRTSS,
    ZYDIS_MNEMONIC_STAC,
    ZYDIS_MNEMONIC_STC,
    ZYDIS_MNEMONIC_STD,
    ZYDIS_MNEMONIC_STGI,
    ZYDIS_MNEMONIC_STI,
    ZYDIS_MNEMONIC_STMXCSR,
    ZYDIS_MNEMONIC_STOSB,
    ZYDIS_MNEMONIC_STOSD,
    ZYDIS_MNEMONIC_STOSQ,
    ZYDIS_MNEMONIC_STOSW,
    ZYDIS_MNEMONIC_STR,
    ZYDIS_MNEMONIC_SUB,
    ZYDIS_MNEMONIC_SUBPD,
    ZYDIS_MNEMONIC_SUBPS,
    ZYDIS_MNEMONIC_SUBSD,
    ZYDIS_MNEMONIC_SUBSS,
    ZYDIS_MNEMONIC_SWAPGS,
    ZYDIS_MNEMONIC_SYSCALL,
    ZYDIS_MNEMONIC_SYSENTER,
    ZYDIS_MNEMONIC_SYSEXIT,
    ZYDIS_MNEMONIC_SYSRET,
    ZYDIS_MNEMONIC_T1MSKC,
    ZYDIS_MNEMONIC_TEST,
    ZYDIS_MNEMONIC_TPAUSE,
    ZYDIS_MNEMONIC_TZCNT,
    ZYDIS_MNEMONIC_TZCNTI,
    ZYDIS_MNEMONIC_TZMSK,
    ZYDIS_MNEMONIC_UCOMISD,
    ZYDIS_MNEMONIC_UCOMISS,
    ZYDIS_MNEMONIC_UD0,
    ZYDIS_MNEMONIC_UD1,
    ZYDIS_MNEMONIC_UD2,
    ZYDIS_MNEMONIC_UMONITOR,
    ZYDIS_MNEMONIC_UMWAIT,
    ZYDIS_MNEMONIC_UNPCKHPD,
    ZYDIS_MNEMONIC_UNPCKHPS,
    ZYDIS_MNEMONIC_UNPCKLPD,
    ZYDIS_MNEMONIC_UNPCKLPS,
    ZYDIS_MNEMONIC_V4FMADDPS,
    ZYDIS_MNEMONIC_V4FMADDSS,
    ZYDIS_MNEMONIC_V4FNMADDPS,
    ZYDIS_MNEMONIC_V4FNMADDSS,
    ZYDIS_MNEMONIC_VADDNPD,
    ZYDIS_MNEMONIC_VADDNPS,
    ZYDIS_MNEMONIC_VADDPD,
    ZYDIS_MNEMONIC_VADDPS,
    ZYDIS_MNEMONIC_VADDSD,
    ZYDIS_MNEMONIC_VADDSETSPS,
    ZYDIS_MNEMONIC_VADDSS,
    ZYDIS_MNEMONIC_VADDSUBPD,
    ZYDIS_MNEMONIC_VADDSUBPS,
    ZYDIS_MNEMONIC_VAESDEC,
    ZYDIS_MNEMONIC_VAESDECLAST,
    ZYDIS_MNEMONIC_VAESENC,
    ZYDIS_MNEMONIC_VAESENCLAST,
    ZYDIS_MNEMONIC_VAESIMC,
    ZYDIS_MNEMONIC_VAESKEYGENASSIST,
    ZYDIS_MNEMONIC_VALIGND,
    ZYDIS_MNEMONIC_VALIGNQ,
    ZYDIS_MNEMONIC_VANDNPD,
    ZYDIS_MNEMONIC_VANDNPS,
    ZYDIS_MNEMONIC_VANDPD,
    ZYDIS_MNEMONIC_VANDPS,
    ZYDIS_MNEMONIC_VBLENDMPD,
    ZYDIS_MNEMONIC_VBLENDMPS,
    ZYDIS_MNEMONIC_VBLENDPD,
    ZYDIS_MNEMONIC_VBLENDPS,
    ZYDIS_MNEMONIC_VBLENDVPD,
    ZYDIS_MNEMONIC_VBLENDVPS,
    ZYDIS_MNEMONIC_VBROADCASTF128,
    ZYDIS_MNEMONIC_VBROADCASTF32X2,
    ZYDIS_MNEMONIC_VBROADCASTF32X4,
    ZYDIS_MNEMONIC_VBROADCASTF32X8,
    ZYDIS_MNEMONIC_VBROADCASTF64X2,
    ZYDIS_MNEMONIC_VBROADCASTF64X4,
    ZYDIS_MNEMONIC_VBROADCASTI128,
    ZYDIS_MNEMONIC_VBROADCASTI32X2,
    ZYDIS_MNEMONIC_VBROADCASTI32X4,
    ZYDIS_MNEMONIC_VBROADCASTI32X8,
    ZYDIS_MNEMONIC_VBROADCASTI64X2,
    ZYDIS_MNEMONIC_VBROADCASTI64X4,
    ZYDIS_MNEMONIC_VBROADCASTSD,
    ZYDIS_MNEMONIC_VBROADCASTSS,
    ZYDIS_MNEMONIC_VCMPPD,
    ZYDIS_MNEMONIC_VCMPPS,
    ZYDIS_MNEMONIC_VCMPSD,
    ZYDIS_MNEMONIC_VCMPSS,
    ZYDIS_MNEMONIC_VCOMISD,
    ZYDIS_MNEMONIC_VCOMISS,
    ZYDIS_MNEMONIC_VCOMPRESSPD,
    ZYDIS_MNEMONIC_VCOMPRESSPS,
    ZYDIS_MNEMONIC_VCVTDQ2PD,
    ZYDIS_MNEMONIC_VCVTDQ2PS,
    ZYDIS_MNEMONIC_VCVTFXPNTDQ2PS,
    ZYDIS_MNEMONIC_VCVTFXPNTPD2DQ,
    ZYDIS_MNEMONIC_VCVTFXPNTPD2UDQ,
    ZYDIS_MNEMONIC_VCVTFXPNTPS2DQ,
    ZYDIS_MNEMONIC_VCVTFXPNTPS2UDQ,
    ZYDIS_MNEMONIC_VCVTFXPNTUDQ2PS,
    ZYDIS_MNEMONIC_VCVTPD2DQ,
    ZYDIS_MNEMONIC_VCVTPD2PS,
    ZYDIS_MNEMONIC_VCVTPD2QQ,
    ZYDIS_MNEMONIC_VCVTPD2UDQ,
    ZYDIS_MNEMONIC_VCVTPD2UQQ,
    ZYDIS_MNEMONIC_VCVTPH2PS,
    ZYDIS_MNEMONIC_VCVTPS2DQ,
    ZYDIS_MNEMONIC_VCVTPS2PD,
    ZYDIS_MNEMONIC_VCVTPS2PH,
    ZYDIS_MNEMONIC_VCVTPS2QQ,
    ZYDIS_MNEMONIC_VCVTPS2UDQ,
    ZYDIS_MNEMONIC_VCVTPS2UQQ,
    ZYDIS_MNEMONIC_VCVTQQ2PD,
    ZYDIS_MNEMONIC_VCVTQQ2PS,
    ZYDIS_MNEMONIC_VCVTSD2SI,
    ZYDIS_MNEMONIC_VCVTSD2SS,
    ZYDIS_MNEMONIC_VCVTSD2USI,
    ZYDIS_MNEMONIC_VCVTSI2SD,
    ZYDIS_MNEMONIC_VCVTSI2SS,
    ZYDIS_MNEMONIC_VCVTSS2SD,
    ZYDIS_MNEMONIC_VCVTSS2SI,
    ZYDIS_MNEMONIC_VCVTSS2USI,
    ZYDIS_MNEMONIC_VCVTTPD2DQ,
    ZYDIS_MNEMONIC_VCVTTPD2QQ,
    ZYDIS_MNEMONIC_VCVTTPD2UDQ,
    ZYDIS_MNEMONIC_VCVTTPD2UQQ,
    ZYDIS_MNEMONIC_VCVTTPS2DQ,
    ZYDIS_MNEMONIC_VCVTTPS2QQ,
    ZYDIS_MNEMONIC_VCVTTPS2UDQ,
    ZYDIS_MNEMONIC_VCVTTPS2UQQ,
    ZYDIS_MNEMONIC_VCVTTSD2SI,
    ZYDIS_MNEMONIC_VCVTTSD2USI,
    ZYDIS_MNEMONIC_VCVTTSS2SI,
    ZYDIS_MNEMONIC_VCVTTSS2USI,
    ZYDIS_MNEMONIC_VCVTUDQ2PD,
    ZYDIS_MNEMONIC_VCVTUDQ2PS,
    ZYDIS_MNEMONIC_VCVTUQQ2PD,
    ZYDIS_MNEMONIC_VCVTUQQ2PS,
    ZYDIS_MNEMONIC_VCVTUSI2SD,
    ZYDIS_MNEMONIC_VCVTUSI2SS,
    ZYDIS_MNEMONIC_VDBPSADBW,
    ZYDIS_MNEMONIC_VDIVPD,
    ZYDIS_MNEMONIC_VDIVPS,
    ZYDIS_MNEMONIC_VDIVSD,
    ZYDIS_MNEMONIC_VDIVSS,
    ZYDIS_MNEMONIC_VDPPD,
    ZYDIS_MNEMONIC_VDPPS,
    ZYDIS_MNEMONIC_VERR,
    ZYDIS_MNEMONIC_VERW,
    ZYDIS_MNEMONIC_VEXP223PS,
    ZYDIS_MNEMONIC_VEXP2PD,
    ZYDIS_MNEMONIC_VEXP2PS,
    ZYDIS_MNEMONIC_VEXPANDPD,
    ZYDIS_MNEMONIC_VEXPANDPS,
    ZYDIS_MNEMONIC_VEXTRACTF128,
    ZYDIS_MNEMONIC_VEXTRACTF32X4,
    ZYDIS_MNEMONIC_VEXTRACTF32X8,
    ZYDIS_MNEMONIC_VEXTRACTF64X2,
    ZYDIS_MNEMONIC_VEXTRACTF64X4,
    ZYDIS_MNEMONIC_VEXTRACTI128,
    ZYDIS_MNEMONIC_VEXTRACTI32X4,
    ZYDIS_MNEMONIC_VEXTRACTI32X8,
    ZYDIS_MNEMONIC_VEXTRACTI64X2,
    ZYDIS_MNEMONIC_VEXTRACTI64X4,
    ZYDIS_MNEMONIC_VEXTRACTPS,
    ZYDIS_MNEMONIC_VFIXUPIMMPD,
    ZYDIS_MNEMONIC_VFIXUPIMMPS,
    ZYDIS_MNEMONIC_VFIXUPIMMSD,
    ZYDIS_MNEMONIC_VFIXUPIMMSS,
    ZYDIS_MNEMONIC_VFIXUPNANPD,
    ZYDIS_MNEMONIC_VFIXUPNANPS,
    ZYDIS_MNEMONIC_VFMADD132PD,
    ZYDIS_MNEMONIC_VFMADD132PS,
    ZYDIS_MNEMONIC_VFMADD132SD,
    ZYDIS_MNEMONIC_VFMADD132SS,
    ZYDIS_MNEMONIC_VFMADD213PD,
    ZYDIS_MNEMONIC_VFMADD213PS,
    ZYDIS_MNEMONIC_VFMADD213SD,
    ZYDIS_MNEMONIC_VFMADD213SS,
    ZYDIS_MNEMONIC_VFMADD231PD,
    ZYDIS_MNEMONIC_VFMADD231PS,
    ZYDIS_MNEMONIC_VFMADD231SD,
    ZYDIS_MNEMONIC_VFMADD231SS,
    ZYDIS_MNEMONIC_VFMADD233PS,
    ZYDIS_MNEMONIC_VFMADDPD,
    ZYDIS_MNEMONIC_VFMADDPS,
    ZYDIS_MNEMONIC_VFMADDSD,
    ZYDIS_MNEMONIC_VFMADDSS,
    ZYDIS_MNEMONIC_VFMADDSUB132PD,
    ZYDIS_MNEMONIC_VFMADDSUB132PS,
    ZYDIS_MNEMONIC_VFMADDSUB213PD,
    ZYDIS_MNEMONIC_VFMADDSUB213PS,
    ZYDIS_MNEMONIC_VFMADDSUB231PD,
    ZYDIS_MNEMONIC_VFMADDSUB231PS,
    ZYDIS_MNEMONIC_VFMADDSUBPD,
    ZYDIS_MNEMONIC_VFMADDSUBPS,
    ZYDIS_MNEMONIC_VFMSUB132PD,
    ZYDIS_MNEMONIC_VFMSUB132PS,
    ZYDIS_MNEMONIC_VFMSUB132SD,
    ZYDIS_MNEMONIC_VFMSUB132SS,
    ZYDIS_MNEMONIC_VFMSUB213PD,
    ZYDIS_MNEMONIC_VFMSUB213PS,
    ZYDIS_MNEMONIC_VFMSUB213SD,
    ZYDIS_MNEMONIC_VFMSUB213SS,
    ZYDIS_MNEMONIC_VFMSUB231PD,
    ZYDIS_MNEMONIC_VFMSUB231PS,
    ZYDIS_MNEMONIC_VFMSUB231SD,
    ZYDIS_MNEMONIC_VFMSUB231SS,
    ZYDIS_MNEMONIC_VFMSUBADD132PD,
    ZYDIS_MNEMONIC_VFMSUBADD132PS,
    ZYDIS_MNEMONIC_VFMSUBADD213PD,
    ZYDIS_MNEMONIC_VFMSUBADD213PS,
    ZYDIS_MNEMONIC_VFMSUBADD231PD,
    ZYDIS_MNEMONIC_VFMSUBADD231PS,
    ZYDIS_MNEMONIC_VFMSUBADDPD,
    ZYDIS_MNEMONIC_VFMSUBADDPS,
    ZYDIS_MNEMONIC_VFMSUBPD,
    ZYDIS_MNEMONIC_VFMSUBPS,
    ZYDIS_MNEMONIC_VFMSUBSD,
    ZYDIS_MNEMONIC_VFMSUBSS,
    ZYDIS_MNEMONIC_VFNMADD132PD,
    ZYDIS_MNEMONIC_VFNMADD132PS,
    ZYDIS_MNEMONIC_VFNMADD132SD,
    ZYDIS_MNEMONIC_VFNMADD132SS,
    ZYDIS_MNEMONIC_VFNMADD213PD,
    ZYDIS_MNEMONIC_VFNMADD213PS,
    ZYDIS_MNEMONIC_VFNMADD213SD,
    ZYDIS_MNEMONIC_VFNMADD213SS,
    ZYDIS_MNEMONIC_VFNMADD231PD,
    ZYDIS_MNEMONIC_VFNMADD231PS,
    ZYDIS_MNEMONIC_VFNMADD231SD,
    ZYDIS_MNEMONIC_VFNMADD231SS,
    ZYDIS_MNEMONIC_VFNMADDPD,
    ZYDIS_MNEMONIC_VFNMADDPS,
    ZYDIS_MNEMONIC_VFNMADDSD,
    ZYDIS_MNEMONIC_VFNMADDSS,
    ZYDIS_MNEMONIC_VFNMSUB132PD,
    ZYDIS_MNEMONIC_VFNMSUB132PS,
    ZYDIS_MNEMONIC_VFNMSUB132SD,
    ZYDIS_MNEMONIC_VFNMSUB132SS,
    ZYDIS_MNEMONIC_VFNMSUB213PD,
    ZYDIS_MNEMONIC_VFNMSUB213PS,
    ZYDIS_MNEMONIC_VFNMSUB213SD,
    ZYDIS_MNEMONIC_VFNMSUB213SS,
    ZYDIS_MNEMONIC_VFNMSUB231PD,
    ZYDIS_MNEMONIC_VFNMSUB231PS,
    ZYDIS_MNEMONIC_VFNMSUB231SD,
    ZYDIS_MNEMONIC_VFNMSUB231SS,
    ZYDIS_MNEMONIC_VFNMSUBPD,
    ZYDIS_MNEMONIC_VFNMSUBPS,
    ZYDIS_MNEMONIC_VFNMSUBSD,
    ZYDIS_MNEMONIC_VFNMSUBSS,
    ZYDIS_MNEMONIC_VFPCLASSPD,
    ZYDIS_MNEMONIC_VFPCLASSPS,
    ZYDIS_MNEMONIC_VFPCLASSSD,
    ZYDIS_MNEMONIC_VFPCLASSSS,
    ZYDIS_MNEMONIC_VFRCZPD,
    ZYDIS_MNEMONIC_VFRCZPS,
    ZYDIS_MNEMONIC_VFRCZSD,
    ZYDIS_MNEMONIC_VFRCZSS,
    ZYDIS_MNEMONIC_VGATHERDPD,
    ZYDIS_MNEMONIC_VGATHERDPS,
    ZYDIS_MNEMONIC_VGATHERPF0DPD,
    ZYDIS_MNEMONIC_VGATHERPF0DPS,
    ZYDIS_MNEMONIC_VGATHERPF0HINTDPD,
    ZYDIS_MNEMONIC_VGATHERPF0HINTDPS,
    ZYDIS_MNEMONIC_VGATHERPF0QPD,
    ZYDIS_MNEMONIC_VGATHERPF0QPS,
    ZYDIS_MNEMONIC_VGATHERPF1DPD,
    ZYDIS_MNEMONIC_VGATHERPF1DPS,
    ZYDIS_MNEMONIC_VGATHERPF1QPD,
    ZYDIS_MNEMONIC_VGATHERPF1QPS,
    ZYDIS_MNEMONIC_VGATHERQPD,
    ZYDIS_MNEMONIC_VGATHERQPS,
    ZYDIS_MNEMONIC_VGETEXPPD,
    ZYDIS_MNEMONIC_VGETEXPPS,
    ZYDIS_MNEMONIC_VGETEXPSD,
    ZYDIS_MNEMONIC_VGETEXPSS,
    ZYDIS_MNEMONIC_VGETMANTPD,
    ZYDIS_MNEMONIC_VGETMANTPS,
    ZYDIS_MNEMONIC_VGETMANTSD,
    ZYDIS_MNEMONIC_VGETMANTSS,
    ZYDIS_MNEMONIC_VGF2P8AFFINEINVQB,
    ZYDIS_MNEMONIC_VGF2P8AFFINEQB,
    ZYDIS_MNEMONIC_VGF2P8MULB,
    ZYDIS_MNEMONIC_VGMAXABSPS,
    ZYDIS_MNEMONIC_VGMAXPD,
    ZYDIS_MNEMONIC_VGMAXPS,
    ZYDIS_MNEMONIC_VGMINPD,
    ZYDIS_MNEMONIC_VGMINPS,
    ZYDIS_MNEMONIC_VHADDPD,
    ZYDIS_MNEMONIC_VHADDPS,
    ZYDIS_MNEMONIC_VHSUBPD,
    ZYDIS_MNEMONIC_VHSUBPS,
    ZYDIS_MNEMONIC_VINSERTF128,
    ZYDIS_MNEMONIC_VINSERTF32X4,
    ZYDIS_MNEMONIC_VINSERTF32X8,
    ZYDIS_MNEMONIC_VINSERTF64X2,
    ZYDIS_MNEMONIC_VINSERTF64X4,
    ZYDIS_MNEMONIC_VINSERTI128,
    ZYDIS_MNEMONIC_VINSERTI32X4,
    ZYDIS_MNEMONIC_VINSERTI32X8,
    ZYDIS_MNEMONIC_VINSERTI64X2,
    ZYDIS_MNEMONIC_VINSERTI64X4,
    ZYDIS_MNEMONIC_VINSERTPS,
    ZYDIS_MNEMONIC_VLDDQU,
    ZYDIS_MNEMONIC_VLDMXCSR,
    ZYDIS_MNEMONIC_VLOADUNPACKHD,
    ZYDIS_MNEMONIC_VLOADUNPACKHPD,
    ZYDIS_MNEMONIC_VLOADUNPACKHPS,
    ZYDIS_MNEMONIC_VLOADUNPACKHQ,
    ZYDIS_MNEMONIC_VLOADUNPACKLD,
    ZYDIS_MNEMONIC_VLOADUNPACKLPD,
    ZYDIS_MNEMONIC_VLOADUNPACKLPS,
    ZYDIS_MNEMONIC_VLOADUNPACKLQ,
    ZYDIS_MNEMONIC_VLOG2PS,
    ZYDIS_MNEMONIC_VMASKMOVDQU,
    ZYDIS_MNEMONIC_VMASKMOVPD,
    ZYDIS_MNEMONIC_VMASKMOVPS,
    ZYDIS_MNEMONIC_VMAXPD,
    ZYDIS_MNEMONIC_VMAXPS,
    ZYDIS_MNEMONIC_VMAXSD,
    ZYDIS_MNEMONIC_VMAXSS,
    ZYDIS_MNEMONIC_VMCALL,
    ZYDIS_MNEMONIC_VMCLEAR,
    ZYDIS_MNEMONIC_VMFUNC,
    ZYDIS_MNEMONIC_VMINPD,
    ZYDIS_MNEMONIC_VMINPS,
    ZYDIS_MNEMONIC_VMINSD,
    ZYDIS_MNEMONIC_VMINSS,
    ZYDIS_MNEMONIC_VMLAUNCH,
    ZYDIS_MNEMONIC_VMLOAD,
    ZYDIS_MNEMONIC_VMMCALL,
    ZYDIS_MNEMONIC_VMOVAPD,
    ZYDIS_MNEMONIC_VMOVAPS,
    ZYDIS_MNEMONIC_VMOVD,
    ZYDIS_MNEMONIC_VMOVDDUP,
    ZYDIS_MNEMONIC_VMOVDQA,
    ZYDIS_MNEMONIC_VMOVDQA32,
    ZYDIS_MNEMONIC_VMOVDQA64,
    ZYDIS_MNEMONIC_VMOVDQU,
    ZYDIS_MNEMONIC_VMOVDQU16,
    ZYDIS_MNEMONIC_VMOVDQU32,
    ZYDIS_MNEMONIC_VMOVDQU64,
    ZYDIS_MNEMONIC_VMOVDQU8,
    ZYDIS_MNEMONIC_VMOVHLPS,
    ZYDIS_MNEMONIC_VMOVHPD,
    ZYDIS_MNEMONIC_VMOVHPS,
    ZYDIS_MNEMONIC_VMOVLHPS,
    ZYDIS_MNEMONIC_VMOVLPD,
    ZYDIS_MNEMONIC_VMOVLPS,
    ZYDIS_MNEMONIC_VMOVMSKPD,
    ZYDIS_MNEMONIC_VMOVMSKPS,
    ZYDIS_MNEMONIC_VMOVNRAPD,
    ZYDIS_MNEMONIC_VMOVNRAPS,
    ZYDIS_MNEMONIC_VMOVNRNGOAPD,
    ZYDIS_MNEMONIC_VMOVNRNGOAPS,
    ZYDIS_MNEMONIC_VMOVNTDQ,
    ZYDIS_MNEMONIC_VMOVNTDQA,
    ZYDIS_MNEMONIC_VMOVNTPD,
    ZYDIS_MNEMONIC_VMOVNTPS,
    ZYDIS_MNEMONIC_VMOVQ,
    ZYDIS_MNEMONIC_VMOVSD,
    ZYDIS_MNEMONIC_VMOVSHDUP,
    ZYDIS_MNEMONIC_VMOVSLDUP,
    ZYDIS_MNEMONIC_VMOVSS,
    ZYDIS_MNEMONIC_VMOVUPD,
    ZYDIS_MNEMONIC_VMOVUPS,
    ZYDIS_MNEMONIC_VMPSADBW,
    ZYDIS_MNEMONIC_VMPTRLD,
    ZYDIS_MNEMONIC_VMPTRST,
    ZYDIS_MNEMONIC_VMREAD,
    ZYDIS_MNEMONIC_VMRESUME,
    ZYDIS_MNEMONIC_VMRUN,
    ZYDIS_MNEMONIC_VMSAVE,
    ZYDIS_MNEMONIC_VMULPD,
    ZYDIS_MNEMONIC_VMULPS,
    ZYDIS_MNEMONIC_VMULSD,
    ZYDIS_MNEMONIC_VMULSS,
    ZYDIS_MNEMONIC_VMWRITE,
    ZYDIS_MNEMONIC_VMXOFF,
    ZYDIS_MNEMONIC_VMXON,
    ZYDIS_MNEMONIC_VORPD,
    ZYDIS_MNEMONIC_VORPS,
    ZYDIS_MNEMONIC_VP4DPWSSD,
    ZYDIS_MNEMONIC_VP4DPWSSDS,
    ZYDIS_MNEMONIC_VPABSB,
    ZYDIS_MNEMONIC_VPABSD,
    ZYDIS_MNEMONIC_VPABSQ,
    ZYDIS_MNEMONIC_VPABSW,
    ZYDIS_MNEMONIC_VPACKSSDW,
    ZYDIS_MNEMONIC_VPACKSSWB,
    ZYDIS_MNEMONIC_VPACKSTOREHD,
    ZYDIS_MNEMONIC_VPACKSTOREHPD,
    ZYDIS_MNEMONIC_VPACKSTOREHPS,
    ZYDIS_MNEMONIC_VPACKSTOREHQ,
    ZYDIS_MNEMONIC_VPACKSTORELD,
    ZYDIS_MNEMONIC_VPACKSTORELPD,
    ZYDIS_MNEMONIC_VPACKSTORELPS,
    ZYDIS_MNEMONIC_VPACKSTORELQ,
    ZYDIS_MNEMONIC_VPACKUSDW,
    ZYDIS_MNEMONIC_VPACKUSWB,
    ZYDIS_MNEMONIC_VPADCD,
    ZYDIS_MNEMONIC_VPADDB,
    ZYDIS_MNEMONIC_VPADDD,
    ZYDIS_MNEMONIC_VPADDQ,
    ZYDIS_MNEMONIC_VPADDSB,
    ZYDIS_MNEMONIC_VPADDSETCD,
    ZYDIS_MNEMONIC_VPADDSETSD,
    ZYDIS_MNEMONIC_VPADDSW,
    ZYDIS_MNEMONIC_VPADDUSB,
    ZYDIS_MNEMONIC_VPADDUSW,
    ZYDIS_MNEMONIC_VPADDW,
    ZYDIS_MNEMONIC_VPALIGNR,
    ZYDIS_MNEMONIC_VPAND,
    ZYDIS_MNEMONIC_VPANDD,
    ZYDIS_MNEMONIC_VPANDN,
    ZYDIS_MNEMONIC_VPANDND,
    ZYDIS_MNEMONIC_VPANDNQ,
    ZYDIS_MNEMONIC_VPANDQ,
    ZYDIS_MNEMONIC_VPAVGB,
    ZYDIS_MNEMONIC_VPAVGW,
    ZYDIS_MNEMONIC_VPBLENDD,
    ZYDIS_MNEMONIC_VPBLENDMB,
    ZYDIS_MNEMONIC_VPBLENDMD,
    ZYDIS_MNEMONIC_VPBLENDMQ,
    ZYDIS_MNEMONIC_VPBLENDMW,
    ZYDIS_MNEMONIC_VPBLENDVB,
    ZYDIS_MNEMONIC_VPBLENDW,
    ZYDIS_MNEMONIC_VPBROADCASTB,
    ZYDIS_MNEMONIC_VPBROADCASTD,
    ZYDIS_MNEMONIC_VPBROADCASTMB2Q,
    ZYDIS_MNEMONIC_VPBROADCASTMW2D,
    ZYDIS_MNEMONIC_VPBROADCASTQ,
    ZYDIS_MNEMONIC_VPBROADCASTW,
    ZYDIS_MNEMONIC_VPCLMULQDQ,
    ZYDIS_MNEMONIC_VPCMOV,
    ZYDIS_MNEMONIC_VPCMPB,
    ZYDIS_MNEMONIC_VPCMPD,
    ZYDIS_MNEMONIC_VPCMPEQB,
    ZYDIS_MNEMONIC_VPCMPEQD,
    ZYDIS_MNEMONIC_VPCMPEQQ,
    ZYDIS_MNEMONIC_VPCMPEQW,
    ZYDIS_MNEMONIC_VPCMPESTRI,
    ZYDIS_MNEMONIC_VPCMPESTRM,
    ZYDIS_MNEMONIC_VPCMPGTB,
    ZYDIS_MNEMONIC_VPCMPGTD,
    ZYDIS_MNEMONIC_VPCMPGTQ,
    ZYDIS_MNEMONIC_VPCMPGTW,
    ZYDIS_MNEMONIC_VPCMPISTRI,
    ZYDIS_MNEMONIC_VPCMPISTRM,
    ZYDIS_MNEMONIC_VPCMPLTD,
    ZYDIS_MNEMONIC_VPCMPQ,
    ZYDIS_MNEMONIC_VPCMPUB,
    ZYDIS_MNEMONIC_VPCMPUD,
    ZYDIS_MNEMONIC_VPCMPUQ,
    ZYDIS_MNEMONIC_VPCMPUW,
    ZYDIS_MNEMONIC_VPCMPW,
    ZYDIS_MNEMONIC_VPCOMB,
    ZYDIS_MNEMONIC_VPCOMD,
    ZYDIS_MNEMONIC_VPCOMPRESSB,
    ZYDIS_MNEMONIC_VPCOMPRESSD,
    ZYDIS_MNEMONIC_VPCOMPRESSQ,
    ZYDIS_MNEMONIC_VPCOMPRESSW,
    ZYDIS_MNEMONIC_VPCOMQ,
    ZYDIS_MNEMONIC_VPCOMUB,
    ZYDIS_MNEMONIC_VPCOMUD,
    ZYDIS_MNEMONIC_VPCOMUQ,
    ZYDIS_MNEMONIC_VPCOMUW,
    ZYDIS_MNEMONIC_VPCOMW,
    ZYDIS_MNEMONIC_VPCONFLICTD,
    ZYDIS_MNEMONIC_VPCONFLICTQ,
    ZYDIS_MNEMONIC_VPDPBUSD,
    ZYDIS_MNEMONIC_VPDPBUSDS,
    ZYDIS_MNEMONIC_VPDPWSSD,
    ZYDIS_MNEMONIC_VPDPWSSDS,
    ZYDIS_MNEMONIC_VPERM2F128,
    ZYDIS_MNEMONIC_VPERM2I128,
    ZYDIS_MNEMONIC_VPERMB,
    ZYDIS_MNEMONIC_VPERMD,
    ZYDIS_MNEMONIC_VPERMF32X4,
    ZYDIS_MNEMONIC_VPERMI2B,
    ZYDIS_MNEMONIC_VPERMI2D,
    ZYDIS_MNEMONIC_VPERMI2PD,
    ZYDIS_MNEMONIC_VPERMI2PS,
    ZYDIS_MNEMONIC_VPERMI2Q,
    ZYDIS_MNEMONIC_VPERMI2W,
    ZYDIS_MNEMONIC_VPERMIL2PD,
    ZYDIS_MNEMONIC_VPERMIL2PS,
    ZYDIS_MNEMONIC_VPERMILPD,
    ZYDIS_MNEMONIC_VPERMILPS,
    ZYDIS_MNEMONIC_VPERMPD,
    ZYDIS_MNEMONIC_VPERMPS,
    ZYDIS_MNEMONIC_VPERMQ,
    ZYDIS_MNEMONIC_VPERMT2B,
    ZYDIS_MNEMONIC_VPERMT2D,
    ZYDIS_MNEMONIC_VPERMT2PD,
    ZYDIS_MNEMONIC_VPERMT2PS,
    ZYDIS_MNEMONIC_VPERMT2Q,
    ZYDIS_MNEMONIC_VPERMT2W,
    ZYDIS_MNEMONIC_VPERMW,
    ZYDIS_MNEMONIC_VPEXPANDB,
    ZYDIS_MNEMONIC_VPEXPANDD,
    ZYDIS_MNEMONIC_VPEXPANDQ,
    ZYDIS_MNEMONIC_VPEXPANDW,
    ZYDIS_MNEMONIC_VPEXTRB,
    ZYDIS_MNEMONIC_VPEXTRD,
    ZYDIS_MNEMONIC_VPEXTRQ,
    ZYDIS_MNEMONIC_VPEXTRW,
    ZYDIS_MNEMONIC_VPGATHERDD,
    ZYDIS_MNEMONIC_VPGATHERDQ,
    ZYDIS_MNEMONIC_VPGATHERQD,
    ZYDIS_MNEMONIC_VPGATHERQQ,
    ZYDIS_MNEMONIC_VPHADDBD,
    ZYDIS_MNEMONIC_VPHADDBQ,
    ZYDIS_MNEMONIC_VPHADDBW,
    ZYDIS_MNEMONIC_VPHADDD,
    ZYDIS_MNEMONIC_VPHADDDQ,
    ZYDIS_MNEMONIC_VPHADDSW,
    ZYDIS_MNEMONIC_VPHADDUBD,
    ZYDIS_MNEMONIC_VPHADDUBQ,
    ZYDIS_MNEMONIC_VPHADDUBW,
    ZYDIS_MNEMONIC_VPHADDUDQ,
    ZYDIS_MNEMONIC_VPHADDUWD,
    ZYDIS_MNEMONIC_VPHADDUWQ,
    ZYDIS_MNEMONIC_VPHADDW,
    ZYDIS_MNEMONIC_VPHADDWD,
    ZYDIS_MNEMONIC_VPHADDWQ,
    ZYDIS_MNEMONIC_VPHMINPOSUW,
    ZYDIS_MNEMONIC_VPHSUBBW,
    ZYDIS_MNEMONIC_VPHSUBD,
    ZYDIS_MNEMONIC_VPHSUBDQ,
    ZYDIS_MNEMONIC_VPHSUBSW,
    ZYDIS_MNEMONIC_VPHSUBW,
    ZYDIS_MNEMONIC_VPHSUBWD,
    ZYDIS_MNEMONIC_VPINSRB,
    ZYDIS_MNEMONIC_VPINSRD,
    ZYDIS_MNEMONIC_VPINSRQ,
    ZYDIS_MNEMONIC_VPINSRW,
    ZYDIS_MNEMONIC_VPLZCNTD,
    ZYDIS_MNEMONIC_VPLZCNTQ,
    ZYDIS_MNEMONIC_VPMACSDD,
    ZYDIS_MNEMONIC_VPMACSDQH,
    ZYDIS_MNEMONIC_VPMACSDQL,
    ZYDIS_MNEMONIC_VPMACSSDD,
    ZYDIS_MNEMONIC_VPMACSSDQH,
    ZYDIS_MNEMONIC_VPMACSSDQL,
    ZYDIS_MNEMONIC_VPMACSSWD,
    ZYDIS_MNEMONIC_VPMACSSWW,
    ZYDIS_MNEMONIC_VPMACSWD,
    ZYDIS_MNEMONIC_VPMACSWW,
    ZYDIS_MNEMONIC_VPMADCSSWD,
    ZYDIS_MNEMONIC_VPMADCSWD,
    ZYDIS_MNEMONIC_VPMADD231D,
    ZYDIS_MNEMONIC_VPMADD233D,
    ZYDIS_MNEMONIC_VPMADD52HUQ,
    ZYDIS_MNEMONIC_VPMADD52LUQ,
    ZYDIS_MNEMONIC_VPMADDUBSW,
    ZYDIS_MNEMONIC_VPMADDWD,
    ZYDIS_MNEMONIC_VPMASKMOVD,
    ZYDIS_MNEMONIC_VPMASKMOVQ,
    ZYDIS_MNEMONIC_VPMAXSB,
    ZYDIS_MNEMONIC_VPMAXSD,
    ZYDIS_MNEMONIC_VPMAXSQ,
    ZYDIS_MNEMONIC_VPMAXSW,
    ZYDIS_MNEMONIC_VPMAXUB,
    ZYDIS_MNEMONIC_VPMAXUD,
    ZYDIS_MNEMONIC_VPMAXUQ,
    ZYDIS_MNEMONIC_VPMAXUW,
    ZYDIS_MNEMONIC_VPMINSB,
    ZYDIS_MNEMONIC_VPMINSD,
    ZYDIS_MNEMONIC_VPMINSQ,
    ZYDIS_MNEMONIC_VPMINSW,
    ZYDIS_MNEMONIC_VPMINUB,
    ZYDIS_MNEMONIC_VPMINUD,
    ZYDIS_MNEMONIC_VPMINUQ,
    ZYDIS_MNEMONIC_VPMINUW,
    ZYDIS_MNEMONIC_VPMOVB2M,
    ZYDIS_MNEMONIC_VPMOVD2M,
    ZYDIS_MNEMONIC_VPMOVDB,
    ZYDIS_MNEMONIC_VPMOVDW,
    ZYDIS_MNEMONIC_VPMOVM2B,
    ZYDIS_MNEMONIC_VPMOVM2D,
    ZYDIS_MNEMONIC_VPMOVM2Q,
    ZYDIS_MNEMONIC_VPMOVM2W,
    ZYDIS_MNEMONIC_VPMOVMSKB,
    ZYDIS_MNEMONIC_VPMOVQ2M,
    ZYDIS_MNEMONIC_VPMOVQB,
    ZYDIS_MNEMONIC_VPMOVQD,
    ZYDIS_MNEMONIC_VPMOVQW,
    ZYDIS_MNEMONIC_VPMOVSDB,
    ZYDIS_MNEMONIC_VPMOVSDW,
    ZYDIS_MNEMONIC_VPMOVSQB,
    ZYDIS_MNEMONIC_VPMOVSQD,
    ZYDIS_MNEMONIC_VPMOVSQW,
    ZYDIS_MNEMONIC_VPMOVSWB,
    ZYDIS_MNEMONIC_VPMOVSXBD,
    ZYDIS_MNEMONIC_VPMOVSXBQ,
    ZYDIS_MNEMONIC_VPMOVSXBW,
    ZYDIS_MNEMONIC_VPMOVSXDQ,
    ZYDIS_MNEMONIC_VPMOVSXWD,
    ZYDIS_MNEMONIC_VPMOVSXWQ,
    ZYDIS_MNEMONIC_VPMOVUSDB,
    ZYDIS_MNEMONIC_VPMOVUSDW,
    ZYDIS_MNEMONIC_VPMOVUSQB,
    ZYDIS_MNEMONIC_VPMOVUSQD,
    ZYDIS_MNEMONIC_VPMOVUSQW,
    ZYDIS_MNEMONIC_VPMOVUSWB,
    ZYDIS_MNEMONIC_VPMOVW2M,
    ZYDIS_MNEMONIC_VPMOVWB,
    ZYDIS_MNEMONIC_VPMOVZXBD,
    ZYDIS_MNEMONIC_VPMOVZXBQ,
    ZYDIS_MNEMONIC_VPMOVZXBW,
    ZYDIS_MNEMONIC_VPMOVZXDQ,
    ZYDIS_MNEMONIC_VPMOVZXWD,
    ZYDIS_MNEMONIC_VPMOVZXWQ,
    ZYDIS_MNEMONIC_VPMULDQ,
    ZYDIS_MNEMONIC_VPMULHD,
    ZYDIS_MNEMONIC_VPMULHRSW,
    ZYDIS_MNEMONIC_VPMULHUD,
    ZYDIS_MNEMONIC_VPMULHUW,
    ZYDIS_MNEMONIC_VPMULHW,
    ZYDIS_MNEMONIC_VPMULLD,
    ZYDIS_MNEMONIC_VPMULLQ,
    ZYDIS_MNEMONIC_VPMULLW,
    ZYDIS_MNEMONIC_VPMULTISHIFTQB,
    ZYDIS_MNEMONIC_VPMULUDQ,
    ZYDIS_MNEMONIC_VPOPCNTB,
    ZYDIS_MNEMONIC_VPOPCNTD,
    ZYDIS_MNEMONIC_VPOPCNTQ,
    ZYDIS_MNEMONIC_VPOPCNTW,
    ZYDIS_MNEMONIC_VPOR,
    ZYDIS_MNEMONIC_VPORD,
    ZYDIS_MNEMONIC_VPORQ,
    ZYDIS_MNEMONIC_VPPERM,
    ZYDIS_MNEMONIC_VPREFETCH0,
    ZYDIS_MNEMONIC_VPREFETCH1,
    ZYDIS_MNEMONIC_VPREFETCH2,
    ZYDIS_MNEMONIC_VPREFETCHE0,
    ZYDIS_MNEMONIC_VPREFETCHE1,
    ZYDIS_MNEMONIC_VPREFETCHE2,
    ZYDIS_MNEMONIC_VPREFETCHENTA,
    ZYDIS_MNEMONIC_VPREFETCHNTA,
    ZYDIS_MNEMONIC_VPROLD,
    ZYDIS_MNEMONIC_VPROLQ,
    ZYDIS_MNEMONIC_VPROLVD,
    ZYDIS_MNEMONIC_VPROLVQ,
    ZYDIS_MNEMONIC_VPRORD,
    ZYDIS_MNEMONIC_VPRORQ,
    ZYDIS_MNEMONIC_VPRORVD,
    ZYDIS_MNEMONIC_VPRORVQ,
    ZYDIS_MNEMONIC_VPROTB,
    ZYDIS_MNEMONIC_VPROTD,
    ZYDIS_MNEMONIC_VPROTQ,
    ZYDIS_MNEMONIC_VPROTW,
    ZYDIS_MNEMONIC_VPSADBW,
    ZYDIS_MNEMONIC_VPSBBD,
    ZYDIS_MNEMONIC_VPSBBRD,
    ZYDIS_MNEMONIC_VPSCATTERDD,
    ZYDIS_MNEMONIC_VPSCATTERDQ,
    ZYDIS_MNEMONIC_VPSCATTERQD,
    ZYDIS_MNEMONIC_VPSCATTERQQ,
    ZYDIS_MNEMONIC_VPSHAB,
    ZYDIS_MNEMONIC_VPSHAD,
    ZYDIS_MNEMONIC_VPSHAQ,
    ZYDIS_MNEMONIC_VPSHAW,
    ZYDIS_MNEMONIC_VPSHLB,
    ZYDIS_MNEMONIC_VPSHLD,
    ZYDIS_MNEMONIC_VPSHLDD,
    ZYDIS_MNEMONIC_VPSHLDQ,
    ZYDIS_MNEMONIC_VPSHLDVD,
    ZYDIS_MNEMONIC_VPSHLDVQ,
    ZYDIS_MNEMONIC_VPSHLDVW,
    ZYDIS_MNEMONIC_VPSHLDW,
    ZYDIS_MNEMONIC_VPSHLQ,
    ZYDIS_MNEMONIC_VPSHLW,
    ZYDIS_MNEMONIC_VPSHRDD,
    ZYDIS_MNEMONIC_VPSHRDQ,
    ZYDIS_MNEMONIC_VPSHRDVD,
    ZYDIS_MNEMONIC_VPSHRDVQ,
    ZYDIS_MNEMONIC_VPSHRDVW,
    ZYDIS_MNEMONIC_VPSHRDW,
    ZYDIS_MNEMONIC_VPSHUFB,
    ZYDIS_MNEMONIC_VPSHUFBITQMB,
    ZYDIS_MNEMONIC_VPSHUFD,
    ZYDIS_MNEMONIC_VPSHUFHW,
    ZYDIS_MNEMONIC_VPSHUFLW,
    ZYDIS_MNEMONIC_VPSIGNB,
    ZYDIS_MNEMONIC_VPSIGND,
    ZYDIS_MNEMONIC_VPSIGNW,
    ZYDIS_MNEMONIC_VPSLLD,
    ZYDIS_MNEMONIC_VPSLLDQ,
    ZYDIS_MNEMONIC_VPSLLQ,
    ZYDIS_MNEMONIC_VPSLLVD,
    ZYDIS_MNEMONIC_VPSLLVQ,
    ZYDIS_MNEMONIC_VPSLLVW,
    ZYDIS_MNEMONIC_VPSLLW,
    ZYDIS_MNEMONIC_VPSRAD,
    ZYDIS_MNEMONIC_VPSRAQ,
    ZYDIS_MNEMONIC_VPSRAVD,
    ZYDIS_MNEMONIC_VPSRAVQ,
    ZYDIS_MNEMONIC_VPSRAVW,
    ZYDIS_MNEMONIC_VPSRAW,
    ZYDIS_MNEMONIC_VPSRLD,
    ZYDIS_MNEMONIC_VPSRLDQ,
    ZYDIS_MNEMONIC_VPSRLQ,
    ZYDIS_MNEMONIC_VPSRLVD,
    ZYDIS_MNEMONIC_VPSRLVQ,
    ZYDIS_MNEMONIC_VPSRLVW,
    ZYDIS_MNEMONIC_VPSRLW,
    ZYDIS_MNEMONIC_VPSUBB,
    ZYDIS_MNEMONIC_VPSUBD,
    ZYDIS_MNEMONIC_VPSUBQ,
    ZYDIS_MNEMONIC_VPSUBRD,
    ZYDIS_MNEMONIC_VPSUBRSETBD,
    ZYDIS_MNEMONIC_VPSUBSB,
    ZYDIS_MNEMONIC_VPSUBSETBD,
    ZYDIS_MNEMONIC_VPSUBSW,
    ZYDIS_MNEMONIC_VPSUBUSB,
    ZYDIS_MNEMONIC_VPSUBUSW,
    ZYDIS_MNEMONIC_VPSUBW,
    ZYDIS_MNEMONIC_VPTERNLOGD,
    ZYDIS_MNEMONIC_VPTERNLOGQ,
    ZYDIS_MNEMONIC_VPTEST,
    ZYDIS_MNEMONIC_VPTESTMB,
    ZYDIS_MNEMONIC_VPTESTMD,
    ZYDIS_MNEMONIC_VPTESTMQ,
    ZYDIS_MNEMONIC_VPTESTMW,
    ZYDIS_MNEMONIC_VPTESTNMB,
    ZYDIS_MNEMONIC_VPTESTNMD,
    ZYDIS_MNEMONIC_VPTESTNMQ,
    ZYDIS_MNEMONIC_VPTESTNMW,
    ZYDIS_MNEMONIC_VPUNPCKHBW,
    ZYDIS_MNEMONIC_VPUNPCKHDQ,
    ZYDIS_MNEMONIC_VPUNPCKHQDQ,
    ZYDIS_MNEMONIC_VPUNPCKHWD,
    ZYDIS_MNEMONIC_VPUNPCKLBW,
    ZYDIS_MNEMONIC_VPUNPCKLDQ,
    ZYDIS_MNEMONIC_VPUNPCKLQDQ,
    ZYDIS_MNEMONIC_VPUNPCKLWD,
    ZYDIS_MNEMONIC_VPXOR,
    ZYDIS_MNEMONIC_VPXORD,
    ZYDIS_MNEMONIC_VPXORQ,
    ZYDIS_MNEMONIC_VRANGEPD,
    ZYDIS_MNEMONIC_VRANGEPS,
    ZYDIS_MNEMONIC_VRANGESD,
    ZYDIS_MNEMONIC_VRANGESS,
    ZYDIS_MNEMONIC_VRCP14PD,
    ZYDIS_MNEMONIC_VRCP14PS,
    ZYDIS_MNEMONIC_VRCP14SD,
    ZYDIS_MNEMONIC_VRCP14SS,
    ZYDIS_MNEMONIC_VRCP23PS,
    ZYDIS_MNEMONIC_VRCP28PD,
    ZYDIS_MNEMONIC_VRCP28PS,
    ZYDIS_MNEMONIC_VRCP28SD,
    ZYDIS_MNEMONIC_VRCP28SS,
    ZYDIS_MNEMONIC_VRCPPS,
    ZYDIS_MNEMONIC_VRCPSS,
    ZYDIS_MNEMONIC_VREDUCEPD,
    ZYDIS_MNEMONIC_VREDUCEPS,
    ZYDIS_MNEMONIC_VREDUCESD,
    ZYDIS_MNEMONIC_VREDUCESS,
    ZYDIS_MNEMONIC_VRNDFXPNTPD,
    ZYDIS_MNEMONIC_VRNDFXPNTPS,
    ZYDIS_MNEMONIC_VRNDSCALEPD,
    ZYDIS_MNEMONIC_VRNDSCALEPS,
    ZYDIS_MNEMONIC_VRNDSCALESD,
    ZYDIS_MNEMONIC_VRNDSCALESS,
    ZYDIS_MNEMONIC_VROUNDPD,
    ZYDIS_MNEMONIC_VROUNDPS,
    ZYDIS_MNEMONIC_VROUNDSD,
    ZYDIS_MNEMONIC_VROUNDSS,
    ZYDIS_MNEMONIC_VRSQRT14PD,
    ZYDIS_MNEMONIC_VRSQRT14PS,
    ZYDIS_MNEMONIC_VRSQRT14SD,
    ZYDIS_MNEMONIC_VRSQRT14SS,
    ZYDIS_MNEMONIC_VRSQRT23PS,
    ZYDIS_MNEMONIC_VRSQRT28PD,
    ZYDIS_MNEMONIC_VRSQRT28PS,
    ZYDIS_MNEMONIC_VRSQRT28SD,
    ZYDIS_MNEMONIC_VRSQRT28SS,
    ZYDIS_MNEMONIC_VRSQRTPS,
    ZYDIS_MNEMONIC_VRSQRTSS,
    ZYDIS_MNEMONIC_VSCALEFPD,
    ZYDIS_MNEMONIC_VSCALEFPS,
    ZYDIS_MNEMONIC_VSCALEFSD,
    ZYDIS_MNEMONIC_VSCALEFSS,
    ZYDIS_MNEMONIC_VSCALEPS,
    ZYDIS_MNEMONIC_VSCATTERDPD,
    ZYDIS_MNEMONIC_VSCATTERDPS,
    ZYDIS_MNEMONIC_VSCATTERPF0DPD,
    ZYDIS_MNEMONIC_VSCATTERPF0DPS,
    ZYDIS_MNEMONIC_VSCATTERPF0HINTDPD,
    ZYDIS_MNEMONIC_VSCATTERPF0HINTDPS,
    ZYDIS_MNEMONIC_VSCATTERPF0QPD,
    ZYDIS_MNEMONIC_VSCATTERPF0QPS,
    ZYDIS_MNEMONIC_VSCATTERPF1DPD,
    ZYDIS_MNEMONIC_VSCATTERPF1DPS,
    ZYDIS_MNEMONIC_VSCATTERPF1QPD,
    ZYDIS_MNEMONIC_VSCATTERPF1QPS,
    ZYDIS_MNEMONIC_VSCATTERQPD,
    ZYDIS_MNEMONIC_VSCATTERQPS,
    ZYDIS_MNEMONIC_VSHUFF32X4,
    ZYDIS_MNEMONIC_VSHUFF64X2,
    ZYDIS_MNEMONIC_VSHUFI32X4,
    ZYDIS_MNEMONIC_VSHUFI64X2,
    ZYDIS_MNEMONIC_VSHUFPD,
    ZYDIS_MNEMONIC_VSHUFPS,
    ZYDIS_MNEMONIC_VSQRTPD,
    ZYDIS_MNEMONIC_VSQRTPS,
    ZYDIS_MNEMONIC_VSQRTSD,
    ZYDIS_MNEMONIC_VSQRTSS,
    ZYDIS_MNEMONIC_VSTMXCSR,
    ZYDIS_MNEMONIC_VSUBPD,
    ZYDIS_MNEMONIC_VSUBPS,
    ZYDIS_MNEMONIC_VSUBRPD,
    ZYDIS_MNEMONIC_VSUBRPS,
    ZYDIS_MNEMONIC_VSUBSD,
    ZYDIS_MNEMONIC_VSUBSS,
    ZYDIS_MNEMONIC_VTESTPD,
    ZYDIS_MNEMONIC_VTESTPS,
    ZYDIS_MNEMONIC_VUCOMISD,
    ZYDIS_MNEMONIC_VUCOMISS,
    ZYDIS_MNEMONIC_VUNPCKHPD,
    ZYDIS_MNEMONIC_VUNPCKHPS,
    ZYDIS_MNEMONIC_VUNPCKLPD,
    ZYDIS_MNEMONIC_VUNPCKLPS,
    ZYDIS_MNEMONIC_VXORPD,
    ZYDIS_MNEMONIC_VXORPS,
    ZYDIS_MNEMONIC_VZEROALL,
    ZYDIS_MNEMONIC_VZEROUPPER,
    ZYDIS_MNEMONIC_WBINVD,
    ZYDIS_MNEMONIC_WRFSBASE,
    ZYDIS_MNEMONIC_WRGSBASE,
    ZYDIS_MNEMONIC_WRMSR,
    ZYDIS_MNEMONIC_WRPKRU,
    ZYDIS_MNEMONIC_WRSSD,
    ZYDIS_MNEMONIC_WRSSQ,
    ZYDIS_MNEMONIC_WRUSSD,
    ZYDIS_MNEMONIC_WRUSSQ,
    ZYDIS_MNEMONIC_XABORT,
    ZYDIS_MNEMONIC_XADD,
    ZYDIS_MNEMONIC_XBEGIN,
    ZYDIS_MNEMONIC_XCHG,
    ZYDIS_MNEMONIC_XCRYPT_CBC,
    ZYDIS_MNEMONIC_XCRYPT_CFB,
    ZYDIS_MNEMONIC_XCRYPT_CTR,
    ZYDIS_MNEMONIC_XCRYPT_ECB,
    ZYDIS_MNEMONIC_XCRYPT_OFB,
    ZYDIS_MNEMONIC_XEND,
    ZYDIS_MNEMONIC_XGETBV,
    ZYDIS_MNEMONIC_XLAT,
    ZYDIS_MNEMONIC_XOR,
    ZYDIS_MNEMONIC_XORPD,
    ZYDIS_MNEMONIC_XORPS,
    ZYDIS_MNEMONIC_XRSTOR,
    ZYDIS_MNEMONIC_XRSTOR64,
    ZYDIS_MNEMONIC_XRSTORS,
    ZYDIS_MNEMONIC_XRSTORS64,
    ZYDIS_MNEMONIC_XSAVE,
    ZYDIS_MNEMONIC_XSAVE64,
    ZYDIS_MNEMONIC_XSAVEC,
    ZYDIS_MNEMONIC_XSAVEC64,
    ZYDIS_MNEMONIC_XSAVEOPT,
    ZYDIS_MNEMONIC_XSAVEOPT64,
    ZYDIS_MNEMONIC_XSAVES,
    ZYDIS_MNEMONIC_XSAVES64,
    ZYDIS_MNEMONIC_XSETBV,
    ZYDIS_MNEMONIC_XSHA1,
    ZYDIS_MNEMONIC_XSHA256,
    ZYDIS_MNEMONIC_XSTORE,
    ZYDIS_MNEMONIC_XTEST,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_MNEMONIC_MAX_VALUE = ZYDIS_MNEMONIC_XTEST,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_MNEMONIC_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_MNEMONIC_MAX_VALUE)
} ZydisMnemonic;
/**
 * @brief   Defines the `ZydisOperandType` enum.
 */
typedef enum ZydisOperandType_
{
    /**
     * @brief   The operand is not used.
     */
    ZYDIS_OPERAND_TYPE_UNUSED,
    /**
     * @brief   The operand is a register operand.
     */
    ZYDIS_OPERAND_TYPE_REGISTER,
    /**
     * @brief   The operand is a memory operand.
     */
    ZYDIS_OPERAND_TYPE_MEMORY,
    /**
     * @brief   The operand is a pointer operand with a segment:offset lvalue.
     */
    ZYDIS_OPERAND_TYPE_POINTER,
    /**
     * @brief   The operand is an immediate operand.
     */
    ZYDIS_OPERAND_TYPE_IMMEDIATE,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_OPERAND_TYPE_MAX_VALUE = ZYDIS_OPERAND_TYPE_IMMEDIATE,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_OPERAND_TYPE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_OPERAND_TYPE_MAX_VALUE)
} ZydisOperandType;
/**
 * @brief   Defines the `ZydisElementSize` datatype.
 */
typedef ZyanU16 ZydisElementSize;
/**
 * @brief   Defines the `ZydisInstructionAttributes` datatype.
 */
typedef ZyanU64 ZydisInstructionAttributes;
/**
 * @brief   Defines the `ZydisOperandEncoding` enum.
 */
typedef enum ZydisOperandEncoding_
{
    ZYDIS_OPERAND_ENCODING_NONE,
    ZYDIS_OPERAND_ENCODING_MODRM_REG,
    ZYDIS_OPERAND_ENCODING_MODRM_RM,
    ZYDIS_OPERAND_ENCODING_OPCODE,
    ZYDIS_OPERAND_ENCODING_NDSNDD,
    ZYDIS_OPERAND_ENCODING_IS4,
    ZYDIS_OPERAND_ENCODING_MASK,
    ZYDIS_OPERAND_ENCODING_DISP8,
    ZYDIS_OPERAND_ENCODING_DISP16,
    ZYDIS_OPERAND_ENCODING_DISP32,
    ZYDIS_OPERAND_ENCODING_DISP64,
    ZYDIS_OPERAND_ENCODING_DISP16_32_64,
    ZYDIS_OPERAND_ENCODING_DISP32_32_64,
    ZYDIS_OPERAND_ENCODING_DISP16_32_32,
    ZYDIS_OPERAND_ENCODING_UIMM8,
    ZYDIS_OPERAND_ENCODING_UIMM16,
    ZYDIS_OPERAND_ENCODING_UIMM32,
    ZYDIS_OPERAND_ENCODING_UIMM64,
    ZYDIS_OPERAND_ENCODING_UIMM16_32_64,
    ZYDIS_OPERAND_ENCODING_UIMM32_32_64,
    ZYDIS_OPERAND_ENCODING_UIMM16_32_32,
    ZYDIS_OPERAND_ENCODING_SIMM8,
    ZYDIS_OPERAND_ENCODING_SIMM16,
    ZYDIS_OPERAND_ENCODING_SIMM32,
    ZYDIS_OPERAND_ENCODING_SIMM64,
    ZYDIS_OPERAND_ENCODING_SIMM16_32_64,
    ZYDIS_OPERAND_ENCODING_SIMM32_32_64,
    ZYDIS_OPERAND_ENCODING_SIMM16_32_32,
    ZYDIS_OPERAND_ENCODING_JIMM8,
    ZYDIS_OPERAND_ENCODING_JIMM16,
    ZYDIS_OPERAND_ENCODING_JIMM32,
    ZYDIS_OPERAND_ENCODING_JIMM64,
    ZYDIS_OPERAND_ENCODING_JIMM16_32_64,
    ZYDIS_OPERAND_ENCODING_JIMM32_32_64,
    ZYDIS_OPERAND_ENCODING_JIMM16_32_32,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_OPERAND_ENCODING_MAX_VALUE = ZYDIS_OPERAND_ENCODING_JIMM16_32_32,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_OPERAND_ENCODING_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_OPERAND_ENCODING_MAX_VALUE)
} ZydisOperandEncoding;
/**
 * @brief   Defines the `ZydisOperandVisibility` enum.
 */
typedef enum ZydisOperandVisibility_
{
    ZYDIS_OPERAND_VISIBILITY_INVALID,
    /**
     * @brief   The operand is explicitly encoded in the instruction.
     */
    ZYDIS_OPERAND_VISIBILITY_EXPLICIT,
    /**
     * @brief   The operand is part of the opcode, but listed as an operand.
     */
    ZYDIS_OPERAND_VISIBILITY_IMPLICIT,
    /**
     * @brief   The operand is part of the opcode, and not typically listed as an operand.
     */
    ZYDIS_OPERAND_VISIBILITY_HIDDEN,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_OPERAND_VISIBILITY_MAX_VALUE = ZYDIS_OPERAND_VISIBILITY_HIDDEN,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_OPERAND_VISIBILITY_REQUIRED_BITS =
        ZYAN_BITS_TO_REPRESENT(ZYDIS_OPERAND_VISIBILITY_MAX_VALUE)
} ZydisOperandVisibility;
/**
 * @brief   Defines the `ZydisOperandAction` enum.
 */
typedef enum ZydisOperandAction_
{
    /* ------------------------------------------------------------------------------------------ */
    /* Elemental actions                                                                          */
    /* ------------------------------------------------------------------------------------------ */

    /**
     * @brief   The operand is read by the instruction.
     */
    ZYDIS_OPERAND_ACTION_READ       = 0x01,
    /**
     * @brief   The operand is written by the instruction (must write).
     */
    ZYDIS_OPERAND_ACTION_WRITE      = 0x02,
    /**
     * @brief   The operand is conditionally read by the instruction.
     */
    ZYDIS_OPERAND_ACTION_CONDREAD   = 0x04,
    /**
     * @brief   The operand is conditionally written by the instruction (may write).
     */
    ZYDIS_OPERAND_ACTION_CONDWRITE  = 0x08,

    /* ------------------------------------------------------------------------------------------ */
    /* Combined actions                                                                           */
    /* ------------------------------------------------------------------------------------------ */

    /**
     * @brief   The operand is read (must read) and written by the instruction (must write).
     */
    ZYDIS_OPERAND_ACTION_READWRITE = ZYDIS_OPERAND_ACTION_READ | ZYDIS_OPERAND_ACTION_WRITE,
    /**
     * @brief   The operand is conditionally read (may read) and conditionally written by the
     *          instruction (may write).
     */
    ZYDIS_OPERAND_ACTION_CONDREAD_CONDWRITE =
        ZYDIS_OPERAND_ACTION_CONDREAD | ZYDIS_OPERAND_ACTION_CONDWRITE,
    /**
     * @brief   The operand is read (must read) and conditionally written by the instruction
     *          (may write).
     */
    ZYDIS_OPERAND_ACTION_READ_CONDWRITE =
        ZYDIS_OPERAND_ACTION_READ | ZYDIS_OPERAND_ACTION_CONDWRITE,
    /**
     * @brief   The operand is written (must write) and conditionally read by the instruction
     *          (may read).
     */
    ZYDIS_OPERAND_ACTION_CONDREAD_WRITE =
        ZYDIS_OPERAND_ACTION_CONDREAD | ZYDIS_OPERAND_ACTION_WRITE,

    /**
     * @brief   Mask combining all reading access flags.
     */
    ZYDIS_OPERAND_ACTION_MASK_READ  = ZYDIS_OPERAND_ACTION_READ | ZYDIS_OPERAND_ACTION_CONDREAD,
    /**
     * @brief   Mask combining all writing access flags.
     */
    ZYDIS_OPERAND_ACTION_MASK_WRITE = ZYDIS_OPERAND_ACTION_WRITE | ZYDIS_OPERAND_ACTION_CONDWRITE,

    /* ------------------------------------------------------------------------------------------ */

    /**
     * @brief   The minimum number of bits required to represent all values of this bitset.
     */
    ZYDIS_OPERAND_ACTION_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_OPERAND_ACTION_CONDWRITE)
} ZydisOperandAction;

/**
 * @brief   Defines the `ZydisOperandActions` datatype.
 */
typedef ZyanU8 ZydisOperandActions;
/**
 * @brief   Defines the `ZydisElementType` enum.
 */
typedef enum ZydisElementType_
{
    ZYDIS_ELEMENT_TYPE_INVALID,
    /**
     * @brief   A struct type.
     */
    ZYDIS_ELEMENT_TYPE_STRUCT,
    /**
     * @brief   Unsigned integer value.
     */
    ZYDIS_ELEMENT_TYPE_UINT,
    /**
     * @brief   Signed integer value.
     */
    ZYDIS_ELEMENT_TYPE_INT,
    /**
     * @brief   16-bit floating point value (`half`).
     */
    ZYDIS_ELEMENT_TYPE_FLOAT16,
    /**
     * @brief   32-bit floating point value (`single`).
     */
    ZYDIS_ELEMENT_TYPE_FLOAT32,
    /**
     * @brief   64-bit floating point value (`double`).
     */
    ZYDIS_ELEMENT_TYPE_FLOAT64,
    /**
     * @brief   80-bit floating point value (`extended`).
     */
    ZYDIS_ELEMENT_TYPE_FLOAT80,
    /**
     * @brief   Binary coded decimal value.
     */
    ZYDIS_ELEMENT_TYPE_LONGBCD,
    /**
     * @brief   A condition code (e.g. used by `CMPPD`, `VCMPPD`, ...).
     */
    ZYDIS_ELEMENT_TYPE_CC,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_ELEMENT_TYPE_MAX_VALUE = ZYDIS_ELEMENT_TYPE_CC,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_ELEMENT_TYPE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_ELEMENT_TYPE_MAX_VALUE)
} ZydisElementType;
/**
 * @brief   Defines the `ZydisInstructionEncoding` enum.
 */
typedef enum ZydisInstructionEncoding_
{
    /**
     * @brief   The instruction uses the legacy encoding.
     */
    ZYDIS_INSTRUCTION_ENCODING_LEGACY,
    /**
     * @brief   The instruction uses the AMD 3DNow-encoding.
     */
    ZYDIS_INSTRUCTION_ENCODING_3DNOW,
    /**
     * @brief   The instruction uses the AMD XOP-encoding.
     */
    ZYDIS_INSTRUCTION_ENCODING_XOP,
    /**
     * @brief   The instruction uses the VEX-encoding.
     */
    ZYDIS_INSTRUCTION_ENCODING_VEX,
    /**
     * @brief   The instruction uses the EVEX-encoding.
     */
    ZYDIS_INSTRUCTION_ENCODING_EVEX,
    /**
     * @brief   The instruction uses the MVEX-encoding.
     */
    ZYDIS_INSTRUCTION_ENCODING_MVEX,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_INSTRUCTION_ENCODING_MAX_VALUE = ZYDIS_INSTRUCTION_ENCODING_MVEX,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_INSTRUCTION_ENCODING_REQUIRED_BITS =
        ZYAN_BITS_TO_REPRESENT(ZYDIS_INSTRUCTION_ENCODING_MAX_VALUE)
} ZydisInstructionEncoding;
/**
 * @brief   Defines the `ZydisOpcodeMap` enum.
 */
typedef enum ZydisOpcodeMap_
{
    ZYDIS_OPCODE_MAP_DEFAULT,
    ZYDIS_OPCODE_MAP_0F,
    ZYDIS_OPCODE_MAP_0F38,
    ZYDIS_OPCODE_MAP_0F3A,
    ZYDIS_OPCODE_MAP_0F0F,
    ZYDIS_OPCODE_MAP_XOP8,
    ZYDIS_OPCODE_MAP_XOP9,
    ZYDIS_OPCODE_MAP_XOPA,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_OPCODE_MAP_MAX_VALUE = ZYDIS_OPCODE_MAP_XOPA,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_OPCODE_MAP_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_OPCODE_MAP_MAX_VALUE)
} ZydisOpcodeMap;
/**
 * @brief   Defines the `ZydisMemoryOperandType` enum.
 */
typedef enum ZydisMemoryOperandType_
{
    ZYDIS_MEMOP_TYPE_INVALID,
    /**
     * @brief   Normal memory operand.
     */
    ZYDIS_MEMOP_TYPE_MEM,
    /**
     * @brief   The memory operand is only used for address-generation. No real memory-access is
     *          caused.
     */
    ZYDIS_MEMOP_TYPE_AGEN,
    /**
     * @brief   A memory operand using `SIB` addressing form, where the index register is not used
     *          in address calculation and scale is ignored. No real memory-access is caused.
     */
    ZYDIS_MEMOP_TYPE_MIB,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_MEMOP_TYPE_MAX_VALUE = ZYDIS_MEMOP_TYPE_MIB,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_MEMOP_TYPE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_MEMOP_TYPE_MAX_VALUE)
} ZydisMemoryOperandType;

/* ---------------------------------------------------------------------------------------------- */
/* Decoded operand                                                                                */
/* ---------------------------------------------------------------------------------------------- */

/**
 * @brief   Defines the `ZydisDecodedOperand` struct.
 */
typedef struct ZydisDecodedOperand_
{
    /**
     * @brief   The operand-id.
     */
    ZyanU8 id;
    /**
     * @brief   The type of the operand.
     */
    ZydisOperandType type;
    /**
     * @brief   The visibility of the operand.
     */
    ZydisOperandVisibility visibility;
    /**
     * @brief   The operand-actions.
     */
    ZydisOperandActions actions;
    /**
     * @brief   The operand-encoding.
     */
    ZydisOperandEncoding encoding;
    /**
     * @brief   The logical size of the operand (in bits).
     */
    ZyanU16 size;
    /**
     * @brief   The element-type.
     */
    ZydisElementType element_type;
    /**
     * @brief   The size of a single element.
     */
    ZydisElementSize element_size;
    /**
     * @brief   The number of elements.
     */
    ZyanU16 element_count;
    /**
     * @brief   Extended info for register-operands.
     */
    struct
    {
        /**
         * @brief   The register value.
         */
        ZydisRegister value;
        // TODO: AVX512_4VNNIW MULTISOURCE registers
    } reg;
    /**
     * @brief   Extended info for memory-operands.
     */
    struct
    {
        /**
         * @brief   The type of the memory operand.
         */
        ZydisMemoryOperandType type;
        /**
         * @brief   The segment register.
         */
        ZydisRegister segment;
        /**
         * @brief   The base register.
         */
        ZydisRegister base;
        /**
         * @brief   The index register.
         */
        ZydisRegister index;
        /**
         * @brief   The scale factor.
         */
        ZyanU8 scale;
        /**
         * @brief   Extended info for memory-operands with displacement.
         */
        struct
        {
            /**
             * @brief   Signals, if the displacement value is used.
             */
            ZyanBool has_displacement;
            /**
             * @brief   The displacement value
             */
            ZyanI64 value;
        } disp;
    } mem;
    /**
     * @brief   Extended info for pointer-operands.
     */
    struct
    {
        ZyanU16 segment;
        ZyanU32 offset;
    } ptr;
    /**
     * @brief   Extended info for immediate-operands.
     */
    struct
    {
        /**
         * @brief   Signals, if the immediate value is signed.
         */
        ZyanBool is_signed;
        /**
         * @brief   Signals, if the immediate value contains a relative offset. You can use
         *          `ZydisCalcAbsoluteAddress` to determine the absolute address value.
         */
        ZyanBool is_relative;
        /**
         * @brief   The immediate value.
         */
        union
        {
            ZyanU64 u;
            ZyanI64 s;
        } value;
    } imm;
} ZydisDecodedOperand;
/**
 * Defines the `ZydisCPUFlags` data-type.
 */
typedef ZyanU32 ZydisCPUFlags;

typedef ZyanU8 ZydisCPUFlag;

/**
 * Carry flag.
 */
#define ZYDIS_CPUFLAG_CF     0
/**
 * Parity flag.
 */
#define ZYDIS_CPUFLAG_PF     2
/**
 * Adjust flag.
 */
#define ZYDIS_CPUFLAG_AF     4
/**
 * Zero flag.
 */
#define ZYDIS_CPUFLAG_ZF     6
/**
 * Sign flag.
 */
#define ZYDIS_CPUFLAG_SF     7
/**
 * Trap flag.
 */
#define ZYDIS_CPUFLAG_TF     8
/**
 * Interrupt enable flag.
 */
#define ZYDIS_CPUFLAG_IF     9
/**
 * Direction flag.
 */
#define ZYDIS_CPUFLAG_DF    10
/**
 * Overflow flag.
 */
#define ZYDIS_CPUFLAG_OF    11
/**
 * I/O privilege level flag.
 */
#define ZYDIS_CPUFLAG_IOPL  12
/**
 * Nested task flag.
 */
#define ZYDIS_CPUFLAG_NT    14
/**
 * Resume flag.
 */
#define ZYDIS_CPUFLAG_RF    16
/**
 * Virtual 8086 mode flag.
 */
#define ZYDIS_CPUFLAG_VM    17
/**
 * Alignment check.
 */
#define ZYDIS_CPUFLAG_AC    18
/**
 * Virtual interrupt flag.
 */
#define ZYDIS_CPUFLAG_VIF   19
/**
 * Virtual interrupt pending.
 */
#define ZYDIS_CPUFLAG_VIP   20
/**
 * Able to use CPUID instruction.
 */
#define ZYDIS_CPUFLAG_ID    21

///////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * FPU condition-code flag 0.
 *
 * DEPRECATED. This flag is not actually part of `FLAGS/EFLAGS/RFLAGS` and will be removed in the
 * next major release. Please refer to the `fpu_flags_read`/`fpu_flags_written` field instead and
 * use one of the `ZYDIS_FPUFLAG_XXX` masks to check for specific a flag.
 */
#define ZYDIS_CPUFLAG_C0    22
/**
 * FPU condition-code flag 1.
 *
 * DEPRECATED. This flag is not actually part of `FLAGS/EFLAGS/RFLAGS` and will be removed in the
 * next major release. Please refer to the `fpu_flags_read`/`fpu_flags_written` field instead and
 * use one of the `ZYDIS_FPUFLAG_XXX` masks to check for specific a flag.
 */
#define ZYDIS_CPUFLAG_C1    23
/**
 * FPU condition-code flag 2.
 *
 * DEPRECATED. This flag is not actually part of `FLAGS/EFLAGS/RFLAGS` and will be removed in the
 * next major release. Please refer to the `fpu_flags_read`/`fpu_flags_written` field instead and
 * use one of the `ZYDIS_FPUFLAG_XXX` masks to check for specific a flag.
 */
#define ZYDIS_CPUFLAG_C2    24
/**
 * FPU condition-code flag 3.
 *
 * DEPRECATED. This flag is not actually part of `FLAGS/EFLAGS/RFLAGS` and will be removed in the
 * next major release. Please refer to the `fpu_flags_read`/`fpu_flags_written` field instead and
 * use one of the `ZYDIS_FPUFLAG_XXX` masks to check for specific a flag.
 */
#define ZYDIS_CPUFLAG_C3    25

/**
 * DEPRECATED. This define will be removed in the next major release.
 */
#define ZYDIS_CPUFLAG_MAX_VALUE     ZYDIS_CPUFLAG_C3

/**
 * @brief   Defines the `ZydisMaskMode` enum.
 */
typedef enum ZydisMaskMode_
{
    ZYDIS_MASK_MODE_INVALID,
    /**
     * @brief   Masking is disabled for the current instruction (`K0` register is used).
     */
    ZYDIS_MASK_MODE_DISABLED,
    /**
     * @brief   The embedded mask register is used as a merge-mask.
     */
    ZYDIS_MASK_MODE_MERGING,
    /**
     * @brief   The embedded mask register is used as a zero-mask.
     */
    ZYDIS_MASK_MODE_ZEROING,
    /**
     * @brief   The embedded mask register is used as a control-mask (element selector).
     */
    ZYDIS_MASK_MODE_CONTROL,
    /**
     * @brief   The embedded mask register is used as a zeroing control-mask (element selector).
     */
    ZYDIS_MASK_MODE_CONTROL_ZEROING,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_MASK_MODE_MAX_VALUE = ZYDIS_MASK_MODE_CONTROL_ZEROING,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_MASK_MODE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_MASK_MODE_MAX_VALUE)
} ZydisMaskMode;
/**
 * @brief   Defines the `ZydisBroadcastMode` enum.
 */
typedef enum ZydisBroadcastMode_
{
    ZYDIS_BROADCAST_MODE_INVALID,
    ZYDIS_BROADCAST_MODE_1_TO_2,
    ZYDIS_BROADCAST_MODE_1_TO_4,
    ZYDIS_BROADCAST_MODE_1_TO_8,
    ZYDIS_BROADCAST_MODE_1_TO_16,
    ZYDIS_BROADCAST_MODE_1_TO_32,
    ZYDIS_BROADCAST_MODE_1_TO_64,
    ZYDIS_BROADCAST_MODE_2_TO_4,
    ZYDIS_BROADCAST_MODE_2_TO_8,
    ZYDIS_BROADCAST_MODE_2_TO_16,
    ZYDIS_BROADCAST_MODE_4_TO_8,
    ZYDIS_BROADCAST_MODE_4_TO_16,
    ZYDIS_BROADCAST_MODE_8_TO_16,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_BROADCAST_MODE_MAX_VALUE = ZYDIS_BROADCAST_MODE_8_TO_16,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_BROADCAST_MODE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_BROADCAST_MODE_MAX_VALUE)
} ZydisBroadcastMode;
/**
 * @brief   Defines the `ZydisRoundingMode` enum.
 */
typedef enum ZydisRoundingMode_
{
    ZYDIS_ROUNDING_MODE_INVALID,
    /**
     * @brief   Round to nearest.
     */
    ZYDIS_ROUNDING_MODE_RN,
    /**
     * @brief   Round down.
     */
    ZYDIS_ROUNDING_MODE_RD,
    /**
     * @brief   Round up.
     */
    ZYDIS_ROUNDING_MODE_RU,
    /**
     * @brief   Round towards zero.
     */
    ZYDIS_ROUNDING_MODE_RZ,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_ROUNDING_MODE_MAX_VALUE = ZYDIS_ROUNDING_MODE_RZ,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_ROUNDING_MODE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_ROUNDING_MODE_MAX_VALUE)
} ZydisRoundingMode;
/**
 * @brief   Defines the `ZydisSwizzleMode` enum.
 */
typedef enum ZydisSwizzleMode_
{
    ZYDIS_SWIZZLE_MODE_INVALID,
    ZYDIS_SWIZZLE_MODE_DCBA,
    ZYDIS_SWIZZLE_MODE_CDAB,
    ZYDIS_SWIZZLE_MODE_BADC,
    ZYDIS_SWIZZLE_MODE_DACB,
    ZYDIS_SWIZZLE_MODE_AAAA,
    ZYDIS_SWIZZLE_MODE_BBBB,
    ZYDIS_SWIZZLE_MODE_CCCC,
    ZYDIS_SWIZZLE_MODE_DDDD,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_SWIZZLE_MODE_MAX_VALUE = ZYDIS_SWIZZLE_MODE_DDDD,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_SWIZZLE_MODE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_SWIZZLE_MODE_MAX_VALUE)
} ZydisSwizzleMode;
/**
 * @brief   Defines the `ZydisConversionMode` enum.
 */
typedef enum ZydisConversionMode_
{
    ZYDIS_CONVERSION_MODE_INVALID,
    ZYDIS_CONVERSION_MODE_FLOAT16,
    ZYDIS_CONVERSION_MODE_SINT8,
    ZYDIS_CONVERSION_MODE_UINT8,
    ZYDIS_CONVERSION_MODE_SINT16,
    ZYDIS_CONVERSION_MODE_UINT16,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_CONVERSION_MODE_MAX_VALUE = ZYDIS_CONVERSION_MODE_UINT16,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_CONVERSION_MODE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_CONVERSION_MODE_MAX_VALUE)
} ZydisConversionMode;
/**
 * @brief   Defines the `ZydisInstructionCategory` enum.
 */
typedef enum ZydisInstructionCategory_
{
    ZYDIS_CATEGORY_INVALID,
    ZYDIS_CATEGORY_ADOX_ADCX,
    ZYDIS_CATEGORY_AES,
    ZYDIS_CATEGORY_AMD3DNOW,
    ZYDIS_CATEGORY_AVX,
    ZYDIS_CATEGORY_AVX2,
    ZYDIS_CATEGORY_AVX2GATHER,
    ZYDIS_CATEGORY_AVX512,
    ZYDIS_CATEGORY_AVX512_4FMAPS,
    ZYDIS_CATEGORY_AVX512_4VNNIW,
    ZYDIS_CATEGORY_AVX512_BITALG,
    ZYDIS_CATEGORY_AVX512_VBMI,
    ZYDIS_CATEGORY_BINARY,
    ZYDIS_CATEGORY_BITBYTE,
    ZYDIS_CATEGORY_BLEND,
    ZYDIS_CATEGORY_BMI1,
    ZYDIS_CATEGORY_BMI2,
    ZYDIS_CATEGORY_BROADCAST,
    ZYDIS_CATEGORY_CALL,
    ZYDIS_CATEGORY_CET,
    ZYDIS_CATEGORY_CLDEMOTE,
    ZYDIS_CATEGORY_CLFLUSHOPT,
    ZYDIS_CATEGORY_CLWB,
    ZYDIS_CATEGORY_CLZERO,
    ZYDIS_CATEGORY_CMOV,
    ZYDIS_CATEGORY_COMPRESS,
    ZYDIS_CATEGORY_COND_BR,
    ZYDIS_CATEGORY_CONFLICT,
    ZYDIS_CATEGORY_CONVERT,
    ZYDIS_CATEGORY_DATAXFER,
    ZYDIS_CATEGORY_DECIMAL,
    ZYDIS_CATEGORY_EXPAND,
    ZYDIS_CATEGORY_FCMOV,
    ZYDIS_CATEGORY_FLAGOP,
    ZYDIS_CATEGORY_FMA4,
    ZYDIS_CATEGORY_GATHER,
    ZYDIS_CATEGORY_GFNI,
    ZYDIS_CATEGORY_IFMA,
    ZYDIS_CATEGORY_INTERRUPT,
    ZYDIS_CATEGORY_IO,
    ZYDIS_CATEGORY_IOSTRINGOP,
    ZYDIS_CATEGORY_KMASK,
    ZYDIS_CATEGORY_KNC,
    ZYDIS_CATEGORY_KNCMASK,
    ZYDIS_CATEGORY_KNCSCALAR,
    ZYDIS_CATEGORY_LOGICAL,
    ZYDIS_CATEGORY_LOGICAL_FP,
    ZYDIS_CATEGORY_LZCNT,
    ZYDIS_CATEGORY_MISC,
    ZYDIS_CATEGORY_MMX,
    ZYDIS_CATEGORY_MOVDIR,
    ZYDIS_CATEGORY_MPX,
    ZYDIS_CATEGORY_NOP,
    ZYDIS_CATEGORY_PADLOCK,
    ZYDIS_CATEGORY_PCLMULQDQ,
    ZYDIS_CATEGORY_PCONFIG,
    ZYDIS_CATEGORY_PKU,
    ZYDIS_CATEGORY_POP,
    ZYDIS_CATEGORY_PREFETCH,
    ZYDIS_CATEGORY_PREFETCHWT1,
    ZYDIS_CATEGORY_PT,
    ZYDIS_CATEGORY_PUSH,
    ZYDIS_CATEGORY_RDPID,
    ZYDIS_CATEGORY_RDRAND,
    ZYDIS_CATEGORY_RDSEED,
    ZYDIS_CATEGORY_RDWRFSGS,
    ZYDIS_CATEGORY_RET,
    ZYDIS_CATEGORY_ROTATE,
    ZYDIS_CATEGORY_SCATTER,
    ZYDIS_CATEGORY_SEGOP,
    ZYDIS_CATEGORY_SEMAPHORE,
    ZYDIS_CATEGORY_SETCC,
    ZYDIS_CATEGORY_SGX,
    ZYDIS_CATEGORY_SHA,
    ZYDIS_CATEGORY_SHIFT,
    ZYDIS_CATEGORY_SMAP,
    ZYDIS_CATEGORY_SSE,
    ZYDIS_CATEGORY_STRINGOP,
    ZYDIS_CATEGORY_STTNI,
    ZYDIS_CATEGORY_SYSCALL,
    ZYDIS_CATEGORY_SYSRET,
    ZYDIS_CATEGORY_SYSTEM,
    ZYDIS_CATEGORY_TBM,
    ZYDIS_CATEGORY_UFMA,
    ZYDIS_CATEGORY_UNCOND_BR,
    ZYDIS_CATEGORY_VAES,
    ZYDIS_CATEGORY_VBMI2,
    ZYDIS_CATEGORY_VFMA,
    ZYDIS_CATEGORY_VPCLMULQDQ,
    ZYDIS_CATEGORY_VTX,
    ZYDIS_CATEGORY_WAITPKG,
    ZYDIS_CATEGORY_WIDENOP,
    ZYDIS_CATEGORY_X87_ALU,
    ZYDIS_CATEGORY_XOP,
    ZYDIS_CATEGORY_XSAVE,
    ZYDIS_CATEGORY_XSAVEOPT,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_CATEGORY_MAX_VALUE = ZYDIS_CATEGORY_XSAVEOPT,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_CATEGORY_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_CATEGORY_MAX_VALUE)
} ZydisInstructionCategory;
/**
 * @brief   Defines the `ZydisISASet` enum.
 */
typedef enum ZydisISASet_
{
    ZYDIS_ISA_SET_INVALID,
    ZYDIS_ISA_SET_ADOX_ADCX,
    ZYDIS_ISA_SET_AES,
    ZYDIS_ISA_SET_AMD,
    ZYDIS_ISA_SET_AMD3DNOW,
    ZYDIS_ISA_SET_AVX,
    ZYDIS_ISA_SET_AVX2,
    ZYDIS_ISA_SET_AVX2GATHER,
    ZYDIS_ISA_SET_AVX512BW_128,
    ZYDIS_ISA_SET_AVX512BW_128N,
    ZYDIS_ISA_SET_AVX512BW_256,
    ZYDIS_ISA_SET_AVX512BW_512,
    ZYDIS_ISA_SET_AVX512BW_KOP,
    ZYDIS_ISA_SET_AVX512CD_128,
    ZYDIS_ISA_SET_AVX512CD_256,
    ZYDIS_ISA_SET_AVX512CD_512,
    ZYDIS_ISA_SET_AVX512DQ_128,
    ZYDIS_ISA_SET_AVX512DQ_128N,
    ZYDIS_ISA_SET_AVX512DQ_256,
    ZYDIS_ISA_SET_AVX512DQ_512,
    ZYDIS_ISA_SET_AVX512DQ_KOP,
    ZYDIS_ISA_SET_AVX512DQ_SCALAR,
    ZYDIS_ISA_SET_AVX512ER_512,
    ZYDIS_ISA_SET_AVX512ER_SCALAR,
    ZYDIS_ISA_SET_AVX512F_128,
    ZYDIS_ISA_SET_AVX512F_128N,
    ZYDIS_ISA_SET_AVX512F_256,
    ZYDIS_ISA_SET_AVX512F_512,
    ZYDIS_ISA_SET_AVX512F_KOP,
    ZYDIS_ISA_SET_AVX512F_SCALAR,
    ZYDIS_ISA_SET_AVX512PF_512,
    ZYDIS_ISA_SET_AVX512_4FMAPS_512,
    ZYDIS_ISA_SET_AVX512_4FMAPS_SCALAR,
    ZYDIS_ISA_SET_AVX512_4VNNIW_512,
    ZYDIS_ISA_SET_AVX512_BITALG_128,
    ZYDIS_ISA_SET_AVX512_BITALG_256,
    ZYDIS_ISA_SET_AVX512_BITALG_512,
    ZYDIS_ISA_SET_AVX512_GFNI_128,
    ZYDIS_ISA_SET_AVX512_GFNI_256,
    ZYDIS_ISA_SET_AVX512_GFNI_512,
    ZYDIS_ISA_SET_AVX512_IFMA_128,
    ZYDIS_ISA_SET_AVX512_IFMA_256,
    ZYDIS_ISA_SET_AVX512_IFMA_512,
    ZYDIS_ISA_SET_AVX512_VAES_128,
    ZYDIS_ISA_SET_AVX512_VAES_256,
    ZYDIS_ISA_SET_AVX512_VAES_512,
    ZYDIS_ISA_SET_AVX512_VBMI2_128,
    ZYDIS_ISA_SET_AVX512_VBMI2_256,
    ZYDIS_ISA_SET_AVX512_VBMI2_512,
    ZYDIS_ISA_SET_AVX512_VBMI_128,
    ZYDIS_ISA_SET_AVX512_VBMI_256,
    ZYDIS_ISA_SET_AVX512_VBMI_512,
    ZYDIS_ISA_SET_AVX512_VNNI_128,
    ZYDIS_ISA_SET_AVX512_VNNI_256,
    ZYDIS_ISA_SET_AVX512_VNNI_512,
    ZYDIS_ISA_SET_AVX512_VPCLMULQDQ_128,
    ZYDIS_ISA_SET_AVX512_VPCLMULQDQ_256,
    ZYDIS_ISA_SET_AVX512_VPCLMULQDQ_512,
    ZYDIS_ISA_SET_AVX512_VPOPCNTDQ_128,
    ZYDIS_ISA_SET_AVX512_VPOPCNTDQ_256,
    ZYDIS_ISA_SET_AVX512_VPOPCNTDQ_512,
    ZYDIS_ISA_SET_AVXAES,
    ZYDIS_ISA_SET_AVX_GFNI,
    ZYDIS_ISA_SET_BMI1,
    ZYDIS_ISA_SET_BMI2,
    ZYDIS_ISA_SET_CET,
    ZYDIS_ISA_SET_CLDEMOTE,
    ZYDIS_ISA_SET_CLFLUSHOPT,
    ZYDIS_ISA_SET_CLFSH,
    ZYDIS_ISA_SET_CLWB,
    ZYDIS_ISA_SET_CLZERO,
    ZYDIS_ISA_SET_CMOV,
    ZYDIS_ISA_SET_CMPXCHG16B,
    ZYDIS_ISA_SET_F16C,
    ZYDIS_ISA_SET_FAT_NOP,
    ZYDIS_ISA_SET_FCMOV,
    ZYDIS_ISA_SET_FMA,
    ZYDIS_ISA_SET_FMA4,
    ZYDIS_ISA_SET_FXSAVE,
    ZYDIS_ISA_SET_FXSAVE64,
    ZYDIS_ISA_SET_GFNI,
    ZYDIS_ISA_SET_I186,
    ZYDIS_ISA_SET_I286PROTECTED,
    ZYDIS_ISA_SET_I286REAL,
    ZYDIS_ISA_SET_I386,
    ZYDIS_ISA_SET_I486,
    ZYDIS_ISA_SET_I486REAL,
    ZYDIS_ISA_SET_I86,
    ZYDIS_ISA_SET_INVPCID,
    ZYDIS_ISA_SET_KNCE,
    ZYDIS_ISA_SET_KNCJKBR,
    ZYDIS_ISA_SET_KNCSTREAM,
    ZYDIS_ISA_SET_KNCV,
    ZYDIS_ISA_SET_KNC_MISC,
    ZYDIS_ISA_SET_KNC_PF_HINT,
    ZYDIS_ISA_SET_LAHF,
    ZYDIS_ISA_SET_LONGMODE,
    ZYDIS_ISA_SET_LZCNT,
    ZYDIS_ISA_SET_MONITOR,
    ZYDIS_ISA_SET_MONITORX,
    ZYDIS_ISA_SET_MOVBE,
    ZYDIS_ISA_SET_MOVDIR,
    ZYDIS_ISA_SET_MPX,
    ZYDIS_ISA_SET_PADLOCK_ACE,
    ZYDIS_ISA_SET_PADLOCK_PHE,
    ZYDIS_ISA_SET_PADLOCK_PMM,
    ZYDIS_ISA_SET_PADLOCK_RNG,
    ZYDIS_ISA_SET_PAUSE,
    ZYDIS_ISA_SET_PCLMULQDQ,
    ZYDIS_ISA_SET_PCONFIG,
    ZYDIS_ISA_SET_PENTIUMMMX,
    ZYDIS_ISA_SET_PENTIUMREAL,
    ZYDIS_ISA_SET_PKU,
    ZYDIS_ISA_SET_POPCNT,
    ZYDIS_ISA_SET_PPRO,
    ZYDIS_ISA_SET_PREFETCHWT1,
    ZYDIS_ISA_SET_PREFETCH_NOP,
    ZYDIS_ISA_SET_PT,
    ZYDIS_ISA_SET_RDPID,
    ZYDIS_ISA_SET_RDPMC,
    ZYDIS_ISA_SET_RDRAND,
    ZYDIS_ISA_SET_RDSEED,
    ZYDIS_ISA_SET_RDTSCP,
    ZYDIS_ISA_SET_RDWRFSGS,
    ZYDIS_ISA_SET_RTM,
    ZYDIS_ISA_SET_SGX,
    ZYDIS_ISA_SET_SGX_ENCLV,
    ZYDIS_ISA_SET_SHA,
    ZYDIS_ISA_SET_SMAP,
    ZYDIS_ISA_SET_SMX,
    ZYDIS_ISA_SET_SSE,
    ZYDIS_ISA_SET_SSE2,
    ZYDIS_ISA_SET_SSE2MMX,
    ZYDIS_ISA_SET_SSE3,
    ZYDIS_ISA_SET_SSE3X87,
    ZYDIS_ISA_SET_SSE4,
    ZYDIS_ISA_SET_SSE42,
    ZYDIS_ISA_SET_SSE4A,
    ZYDIS_ISA_SET_SSEMXCSR,
    ZYDIS_ISA_SET_SSE_PREFETCH,
    ZYDIS_ISA_SET_SSSE3,
    ZYDIS_ISA_SET_SSSE3MMX,
    ZYDIS_ISA_SET_SVM,
    ZYDIS_ISA_SET_TBM,
    ZYDIS_ISA_SET_VAES,
    ZYDIS_ISA_SET_VMFUNC,
    ZYDIS_ISA_SET_VPCLMULQDQ,
    ZYDIS_ISA_SET_VTX,
    ZYDIS_ISA_SET_WAITPKG,
    ZYDIS_ISA_SET_X87,
    ZYDIS_ISA_SET_XOP,
    ZYDIS_ISA_SET_XSAVE,
    ZYDIS_ISA_SET_XSAVEC,
    ZYDIS_ISA_SET_XSAVEOPT,
    ZYDIS_ISA_SET_XSAVES,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_ISA_SET_MAX_VALUE = ZYDIS_ISA_SET_XSAVES,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_ISA_SET_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_ISA_SET_MAX_VALUE)
} ZydisISASet;
/**
 * @brief   Defines the `ZydisISAExt` enum.
 */
typedef enum ZydisISAExt_
{
    ZYDIS_ISA_EXT_INVALID,
    ZYDIS_ISA_EXT_ADOX_ADCX,
    ZYDIS_ISA_EXT_AES,
    ZYDIS_ISA_EXT_AMD3DNOW,
    ZYDIS_ISA_EXT_AVX,
    ZYDIS_ISA_EXT_AVX2,
    ZYDIS_ISA_EXT_AVX2GATHER,
    ZYDIS_ISA_EXT_AVX512EVEX,
    ZYDIS_ISA_EXT_AVX512VEX,
    ZYDIS_ISA_EXT_AVXAES,
    ZYDIS_ISA_EXT_BASE,
    ZYDIS_ISA_EXT_BMI1,
    ZYDIS_ISA_EXT_BMI2,
    ZYDIS_ISA_EXT_CET,
    ZYDIS_ISA_EXT_CLDEMOTE,
    ZYDIS_ISA_EXT_CLFLUSHOPT,
    ZYDIS_ISA_EXT_CLFSH,
    ZYDIS_ISA_EXT_CLWB,
    ZYDIS_ISA_EXT_CLZERO,
    ZYDIS_ISA_EXT_F16C,
    ZYDIS_ISA_EXT_FMA,
    ZYDIS_ISA_EXT_FMA4,
    ZYDIS_ISA_EXT_GFNI,
    ZYDIS_ISA_EXT_INVPCID,
    ZYDIS_ISA_EXT_KNC,
    ZYDIS_ISA_EXT_KNCE,
    ZYDIS_ISA_EXT_KNCV,
    ZYDIS_ISA_EXT_LONGMODE,
    ZYDIS_ISA_EXT_LZCNT,
    ZYDIS_ISA_EXT_MMX,
    ZYDIS_ISA_EXT_MONITOR,
    ZYDIS_ISA_EXT_MONITORX,
    ZYDIS_ISA_EXT_MOVBE,
    ZYDIS_ISA_EXT_MOVDIR,
    ZYDIS_ISA_EXT_MPX,
    ZYDIS_ISA_EXT_PADLOCK,
    ZYDIS_ISA_EXT_PAUSE,
    ZYDIS_ISA_EXT_PCLMULQDQ,
    ZYDIS_ISA_EXT_PCONFIG,
    ZYDIS_ISA_EXT_PKU,
    ZYDIS_ISA_EXT_PREFETCHWT1,
    ZYDIS_ISA_EXT_PT,
    ZYDIS_ISA_EXT_RDPID,
    ZYDIS_ISA_EXT_RDRAND,
    ZYDIS_ISA_EXT_RDSEED,
    ZYDIS_ISA_EXT_RDTSCP,
    ZYDIS_ISA_EXT_RDWRFSGS,
    ZYDIS_ISA_EXT_RTM,
    ZYDIS_ISA_EXT_SGX,
    ZYDIS_ISA_EXT_SGX_ENCLV,
    ZYDIS_ISA_EXT_SHA,
    ZYDIS_ISA_EXT_SMAP,
    ZYDIS_ISA_EXT_SMX,
    ZYDIS_ISA_EXT_SSE,
    ZYDIS_ISA_EXT_SSE2,
    ZYDIS_ISA_EXT_SSE3,
    ZYDIS_ISA_EXT_SSE4,
    ZYDIS_ISA_EXT_SSE4A,
    ZYDIS_ISA_EXT_SSSE3,
    ZYDIS_ISA_EXT_SVM,
    ZYDIS_ISA_EXT_TBM,
    ZYDIS_ISA_EXT_VAES,
    ZYDIS_ISA_EXT_VMFUNC,
    ZYDIS_ISA_EXT_VPCLMULQDQ,
    ZYDIS_ISA_EXT_VTX,
    ZYDIS_ISA_EXT_WAITPKG,
    ZYDIS_ISA_EXT_X87,
    ZYDIS_ISA_EXT_XOP,
    ZYDIS_ISA_EXT_XSAVE,
    ZYDIS_ISA_EXT_XSAVEC,
    ZYDIS_ISA_EXT_XSAVEOPT,
    ZYDIS_ISA_EXT_XSAVES,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_ISA_EXT_MAX_VALUE = ZYDIS_ISA_EXT_XSAVES,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_ISA_EXT_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_ISA_EXT_MAX_VALUE)
} ZydisISAExt;
/**
 * @brief   Defines the `ZydisBranchType` enum.
 */
typedef enum ZydisBranchType_
{
    /**
     * @brief   The instruction is not a branch instruction.
     */
    ZYDIS_BRANCH_TYPE_NONE,
    /**
     * @brief   The instruction is a short (8-bit) branch instruction.
     */
    ZYDIS_BRANCH_TYPE_SHORT,
    /**
     * @brief   The instruction is a near (16-bit or 32-bit) branch instruction.
     */
    ZYDIS_BRANCH_TYPE_NEAR,
    /**
     * @brief   The instruction is a far (intersegment) branch instruction.
     */
    ZYDIS_BRANCH_TYPE_FAR,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_BRANCH_TYPE_MAX_VALUE = ZYDIS_BRANCH_TYPE_FAR,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_BRANCH_TYPE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_BRANCH_TYPE_MAX_VALUE)
} ZydisBranchType;
/**
 * @brief   Defines the `ZydisExceptionClass` enum.
 */
typedef enum ZydisExceptionClass_
{
    ZYDIS_EXCEPTION_CLASS_NONE,
    // TODO: FP Exceptions
    ZYDIS_EXCEPTION_CLASS_SSE1,
    ZYDIS_EXCEPTION_CLASS_SSE2,
    ZYDIS_EXCEPTION_CLASS_SSE3,
    ZYDIS_EXCEPTION_CLASS_SSE4,
    ZYDIS_EXCEPTION_CLASS_SSE5,
    ZYDIS_EXCEPTION_CLASS_SSE7,
    ZYDIS_EXCEPTION_CLASS_AVX1,
    ZYDIS_EXCEPTION_CLASS_AVX2,
    ZYDIS_EXCEPTION_CLASS_AVX3,
    ZYDIS_EXCEPTION_CLASS_AVX4,
    ZYDIS_EXCEPTION_CLASS_AVX5,
    ZYDIS_EXCEPTION_CLASS_AVX6,
    ZYDIS_EXCEPTION_CLASS_AVX7,
    ZYDIS_EXCEPTION_CLASS_AVX8,
    ZYDIS_EXCEPTION_CLASS_AVX11,
    ZYDIS_EXCEPTION_CLASS_AVX12,
    ZYDIS_EXCEPTION_CLASS_E1,
    ZYDIS_EXCEPTION_CLASS_E1NF,
    ZYDIS_EXCEPTION_CLASS_E2,
    ZYDIS_EXCEPTION_CLASS_E2NF,
    ZYDIS_EXCEPTION_CLASS_E3,
    ZYDIS_EXCEPTION_CLASS_E3NF,
    ZYDIS_EXCEPTION_CLASS_E4,
    ZYDIS_EXCEPTION_CLASS_E4NF,
    ZYDIS_EXCEPTION_CLASS_E5,
    ZYDIS_EXCEPTION_CLASS_E5NF,
    ZYDIS_EXCEPTION_CLASS_E6,
    ZYDIS_EXCEPTION_CLASS_E6NF,
    ZYDIS_EXCEPTION_CLASS_E7NM,
    ZYDIS_EXCEPTION_CLASS_E7NM128,
    ZYDIS_EXCEPTION_CLASS_E9NF,
    ZYDIS_EXCEPTION_CLASS_E10,
    ZYDIS_EXCEPTION_CLASS_E10NF,
    ZYDIS_EXCEPTION_CLASS_E11,
    ZYDIS_EXCEPTION_CLASS_E11NF,
    ZYDIS_EXCEPTION_CLASS_E12,
    ZYDIS_EXCEPTION_CLASS_E12NP,
    ZYDIS_EXCEPTION_CLASS_K20,
    ZYDIS_EXCEPTION_CLASS_K21,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_EXCEPTION_CLASS_MAX_VALUE = ZYDIS_EXCEPTION_CLASS_K21,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_EXCEPTION_CLASS_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_EXCEPTION_CLASS_MAX_VALUE)
} ZydisExceptionClass;
/**
 * @brief   Defines the `ZydisPrefixType` enum.
 */
typedef enum ZydisPrefixType_
{
    /**
     * @brief   The prefix is ignored by the instruction.
     *
     * This applies to all prefixes that are not accepted by the instruction in general or the
     * ones that are overwritten by a prefix of the same group closer to the instruction opcode.
     */
    ZYDIS_PREFIX_TYPE_IGNORED,
    /**
     * @brief   The prefix is effectively used by the instruction.
     */
    ZYDIS_PREFIX_TYPE_EFFECTIVE,
    /**
     * @brief   The prefix is used as a mandatory prefix.
     *
     * A mandatory prefix is interpreted as an opcode extension and has no further effect on the
     * instruction.
     */
    ZYDIS_PREFIX_TYPE_MANDATORY,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_PREFIX_TYPE_MAX_VALUE = ZYDIS_PREFIX_TYPE_MANDATORY,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_PREFIX_TYPE_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_PREFIX_TYPE_MAX_VALUE)
} ZydisPrefixType;
/**
 * @brief   Defines the `ZydisCPUFlagAction` enum.
 */
typedef enum ZydisCPUFlagAction_
{
    /**
     * @brief   The CPU flag is not touched by the instruction.
     */
    ZYDIS_CPUFLAG_ACTION_NONE,
    /**
     * @brief   The CPU flag is tested (read).
     */
    ZYDIS_CPUFLAG_ACTION_TESTED,
    /**
     * @brief   The CPU flag is tested and modified aferwards (read-write).
     */
    ZYDIS_CPUFLAG_ACTION_TESTED_MODIFIED,
    /**
     * @brief   The CPU flag is modified (write).
     */
    ZYDIS_CPUFLAG_ACTION_MODIFIED,
    /**
     * @brief   The CPU flag is set to 0 (write).
     */
    ZYDIS_CPUFLAG_ACTION_SET_0,
    /**
     * @brief   The CPU flag is set to 1 (write).
     */
    ZYDIS_CPUFLAG_ACTION_SET_1,
    /**
     * @brief   The CPU flag is undefined (write).
     */
    ZYDIS_CPUFLAG_ACTION_UNDEFINED,

    /**
     * @brief   Maximum value of this enum.
     */
    ZYDIS_CPUFLAG_ACTION_MAX_VALUE = ZYDIS_CPUFLAG_ACTION_UNDEFINED,
    /**
     * @brief   The minimum number of bits required to represent all values of this enum.
     */
    ZYDIS_CPUFLAG_ACTION_REQUIRED_BITS = ZYAN_BITS_TO_REPRESENT(ZYDIS_CPUFLAG_ACTION_MAX_VALUE)
} ZydisCPUFlagAction;
 /**
  * Defines the `ZydisFPUFlags` data-type.
  */
typedef ZyanU8 ZydisFPUFlags;

/**
 * @brief   Defines the `ZydisDecodedInstruction` struct.
 */
typedef struct ZydisDecodedInstruction_
{
    /**
     * The machine mode used to decode this instruction.
     */
    ZydisMachineMode machine_mode;
    /**
     * The instruction-mnemonic.
     */
    ZydisMnemonic mnemonic;
    /**
     * The length of the decoded instruction.
     */
    ZyanU8 length;
    /**
     * The instruction-encoding (`LEGACY`, `3DNOW`, `VEX`, `EVEX`, `XOP`).
     */
    ZydisInstructionEncoding encoding;
    /**
     * The opcode-map.
     */
    ZydisOpcodeMap opcode_map;
    /**
     * The instruction-opcode.
     */
    ZyanU8 opcode;
    /**
     * The stack width.
     */
    ZyanU8 stack_width;
    /**
     * The effective operand width.
     */
    ZyanU8 operand_width;
    /**
     * The effective address width.
     */
    ZyanU8 address_width;
    /**
     * The number of instruction-operands.
     */
    ZyanU8 operand_count;
    /**
     * Detailed info for all instruction operands.
     *
     * Explicit operands are guaranteed to be in the front and ordered as they are printed
     * by the formatter in Intel mode. No assumptions can be made about the order of hidden
     * operands, except that they always located behind the explicit operands.
     */
    ZydisDecodedOperand operands[ZYDIS_MAX_OPERAND_COUNT];
    /**
     * Instruction attributes.
     */
    ZydisInstructionAttributes attributes;
    /**
     * Information about accessed CPU flags.
     *
     * DEPRECATED. This field will be removed in the next major release. Please use the
     * `cpu_flags_read`/`cpu_flags_written` or `fpu_flags_read`/`fpu_flags_written` fields
     * instead.
     */
    struct ZydisDecodedInstructionAccessedFlags_
    {
        /**
         * The CPU-flag action.
         *
         * Use `ZydisGetAccessedFlagsByAction` to get a mask with all flags matching a specific
         * action.
         */
        ZydisCPUFlagAction action;
    } accessed_flags[ZYDIS_CPUFLAG_MAX_VALUE + 1];
    /**
     * A mask containing the CPU flags read by the instruction.
     *
     * The bits in this mask correspond to the actual bits in the `FLAGS/EFLAGS/RFLAGS`
     * register.
     *
     * This mask includes the actions `TESTED` and `TESTED_MODIFIED`.
     */
    ZydisCPUFlags cpu_flags_read;
    /**
     * A mask containing the CPU flags written by the instruction.
     *
     * The bits in this mask correspond to the actual bits in the `FLAGS/EFLAGS/RFLAGS`
     * register.
     *
     * This mask includes the actions `TESTED_MODIFIED`, `SET_0`, `SET_1` and `UNDEFINED`.
     */
    ZydisCPUFlags cpu_flags_written;
    /**
     * A mask containing the FPU flags read by the instruction.
     */
    ZydisFPUFlags fpu_flags_read;
    /**
     * A mask containing the FPU flags written by the instruction.
     */
    ZydisFPUFlags fpu_flags_written;
    /**
     * Extended info for `AVX` instructions.
     */
    struct ZydisDecodedInstructionAvx_
    {
        /**
         * The `AVX` vector-length.
         */
        ZyanU16 vector_length;
        /**
         * Info about the embedded writemask-register (`AVX-512` and `KNC` only).
         */
        struct ZydisDecodedInstructionAvxMask_
        {
            /**
             * The masking mode.
             */
            ZydisMaskMode mode;
            /**
             * The mask register.
             */
            ZydisRegister reg;
        } mask;
        /**
         * Contains info about the `AVX` broadcast.
         */
        struct ZydisDecodedInstructionAvxBroadcast_
        {
            /**
             * Signals, if the broadcast is a static broadcast.
             *
             * This is the case for instructions with inbuilt broadcast functionality, which is
             * always active and not controlled by the `EVEX/MVEX.RC` bits.
             */
            ZyanBool is_static;
            /**
             * The `AVX` broadcast-mode.
             */
            ZydisBroadcastMode mode;
        } broadcast;
        /**
         * Contains info about the `AVX` rounding.
         */
        struct ZydisDecodedInstructionAvxRounding_
        {
            /**
             * The `AVX` rounding-mode.
             */
            ZydisRoundingMode mode;
        } rounding;
        /**
         * Contains info about the `AVX` register-swizzle (`KNC` only).
         */
        struct ZydisDecodedInstructionAvxSwizzle_
        {
            /**
             * The `AVX` register-swizzle mode.
             */
            ZydisSwizzleMode mode;
        } swizzle;
        /**
         * Contains info about the `AVX` data-conversion (`KNC` only).
         */
        struct ZydisDecodedInstructionAvxConversion_
        {
            /**
             * The `AVX` data-conversion mode.
             */
            ZydisConversionMode mode;
        } conversion;
        /**
         * Signals, if the `SAE` (suppress-all-exceptions) functionality is
         * enabled for the instruction.
         */
        ZyanBool has_sae;
        /**
         * Signals, if the instruction has a memory-eviction-hint (`KNC` only).
         */
        ZyanBool has_eviction_hint;
        // TODO: publish EVEX tuple-type and MVEX functionality
    } avx;
    /**
     * Meta info.
     */
    struct ZydisDecodedInstructionMeta_
    {
        /**
         * The instruction category.
         */
        ZydisInstructionCategory category;
        /**
         * The ISA-set.
         */
        ZydisISASet isa_set;
        /**
         * The ISA-set extension.
         */
        ZydisISAExt isa_ext;
        /**
         * The branch type.
         */
        ZydisBranchType branch_type;
        /**
         * The exception class.
         */
        ZydisExceptionClass exception_class;
    } meta;
    /**
     * Detailed info about different instruction-parts like `ModRM`, `SIB` or
     * encoding-prefixes.
     */
    struct ZydisDecodedInstructionRaw_
    {
        /**
         * The number of legacy prefixes.
         */
        ZyanU8 prefix_count;
        /**
         * Detailed info about the legacy prefixes (including `REX`).
         */
        struct ZydisDecodedInstructionRawPrefixes_
        {
            /**
             * The prefix type.
             */
            ZydisPrefixType type;
            /**
             * The prefix byte.
             */
            ZyanU8 value;
        } prefixes[ZYDIS_MAX_INSTRUCTION_LENGTH];
        /**
         * Detailed info about the `REX` prefix.
         */
        struct ZydisDecodedInstructionRawRex_
        {
            /**
             * 64-bit operand-size promotion.
             */
            ZyanU8 W;
            /**
             * Extension of the `ModRM.reg` field.
             */
            ZyanU8 R;
            /**
             * Extension of the `SIB.index` field.
             */
            ZyanU8 X;
            /**
             * Extension of the `ModRM.rm`, `SIB.base`, or `opcode.reg` field.
             */
            ZyanU8 B;
            /**
             * The offset of the effective `REX` byte, relative to the beginning of the
             * instruction, in bytes.
             *
             * This offset always points to the "effective" `REX` prefix (the one closest to the
             * instruction opcode), if multiple `REX` prefixes are present.
             *
             * Note that the `REX` byte can be the first byte of the instruction, which would lead
             * to an offset of `0`. Please refer to the instruction attributes to check for the
             * presence of the `REX` prefix.
             */
            ZyanU8 offset;
        } rex;
        /**
         * Detailed info about the `XOP` prefix.
         */
        struct ZydisDecodedInstructionRawXop_
        {
            /**
             * Extension of the `ModRM.reg` field (inverted).
             */
            ZyanU8 R;
            /**
             * Extension of the `SIB.index` field (inverted).
             */
            ZyanU8 X;
            /**
             * Extension of the `ModRM.rm`, `SIB.base`, or `opcode.reg` field (inverted).
             */
            ZyanU8 B;
            /**
             * Opcode-map specifier.
             */
            ZyanU8 m_mmmm;
            /**
             * 64-bit operand-size promotion or opcode-extension.
             */
            ZyanU8 W;
            /**
             * `NDS`/`NDD` (non-destructive-source/destination) register
             * specifier (inverted).
             */
            ZyanU8 vvvv;
            /**
             * Vector-length specifier.
             */
            ZyanU8 L;
            /**
             * Compressed legacy prefix.
             */
            ZyanU8 pp;
            /**
             * The offset of the first xop byte, relative to the beginning of
             * the instruction, in bytes.
             */
            ZyanU8 offset;
        } xop;
        /**
         * Detailed info about the `VEX` prefix.
         */
        struct ZydisDecodedInstructionRawVex_
        {
            /**
             * Extension of the `ModRM.reg` field (inverted).
             */
            ZyanU8 R;
            /**
             * Extension of the `SIB.index` field (inverted).
             */
            ZyanU8 X;
            /**
             * Extension of the `ModRM.rm`, `SIB.base`, or `opcode.reg` field (inverted).
             */
            ZyanU8 B;
            /**
             * Opcode-map specifier.
             */
            ZyanU8 m_mmmm;
            /**
             * 64-bit operand-size promotion or opcode-extension.
             */
            ZyanU8 W;
            /**
             * `NDS`/`NDD` (non-destructive-source/destination) register specifier
             *  (inverted).
             */
            ZyanU8 vvvv;
            /**
             * Vector-length specifier.
             */
            ZyanU8 L;
            /**
             * Compressed legacy prefix.
             */
            ZyanU8 pp;
            /**
             * The offset of the first `VEX` byte, relative to the beginning of the instruction, in
             * bytes.
             */
            ZyanU8 offset;
            /**
             * The size of the `VEX` prefix, in bytes.
             */
            ZyanU8 size;
        } vex;
        /**
         * Detailed info about the `EVEX` prefix.
         */
        struct ZydisDecodedInstructionRawEvex_
        {
            /**
             * Extension of the `ModRM.reg` field (inverted).
             */
            ZyanU8 R;
            /**
             * Extension of the `SIB.index/vidx` field (inverted).
             */
            ZyanU8 X;
            /**
             * Extension of the `ModRM.rm` or `SIB.base` field (inverted).
             */
            ZyanU8 B;
            /**
             * High-16 register specifier modifier (inverted).
             */
            ZyanU8 R2;
            /**
             * Opcode-map specifier.
             */
            ZyanU8 mm;
            /**
             * 64-bit operand-size promotion or opcode-extension.
             */
            ZyanU8 W;
            /**
             * `NDS`/`NDD` (non-destructive-source/destination) register specifier
             * (inverted).
             */
            ZyanU8 vvvv;
            /**
             * Compressed legacy prefix.
             */
            ZyanU8 pp;
            /**
             * Zeroing/Merging.
             */
            ZyanU8 z;
            /**
             * Vector-length specifier or rounding-control (most significant bit).
             */
            ZyanU8 L2;
            /**
             * Vector-length specifier or rounding-control (least significant bit).
             */
            ZyanU8 L;
            /**
             * Broadcast/RC/SAE context.
             */
            ZyanU8 b;
            /**
             * High-16 `NDS`/`VIDX` register specifier.
             */
            ZyanU8 V2;
            /**
             * Embedded opmask register specifier.
             */
            ZyanU8 aaa;
            /**
             * The offset of the first evex byte, relative to the beginning of the
             * instruction, in bytes.
             */
            ZyanU8 offset;
        } evex;
        /**
        * Detailed info about the `MVEX` prefix.
        */
        struct ZydisDecodedInstructionRawMvex_
        {
            /**
             * Extension of the `ModRM.reg` field (inverted).
             */
            ZyanU8 R;
            /**
             * Extension of the `SIB.index/vidx` field (inverted).
             */
            ZyanU8 X;
            /**
             * Extension of the `ModRM.rm` or `SIB.base` field (inverted).
             */
            ZyanU8 B;
            /**
             * High-16 register specifier modifier (inverted).
             */
            ZyanU8 R2;
            /**
             * Opcode-map specifier.
             */
            ZyanU8 mmmm;
            /**
             * 64-bit operand-size promotion or opcode-extension.
             */
            ZyanU8 W;
            /**
             * `NDS`/`NDD` (non-destructive-source/destination) register specifier
             *  (inverted).
             */
            ZyanU8 vvvv;
            /**
             * Compressed legacy prefix.
             */
            ZyanU8 pp;
            /**
             * Non-temporal/eviction hint.
             */
            ZyanU8 E;
            /**
             * Swizzle/broadcast/up-convert/down-convert/static-rounding controls.
             */
            ZyanU8 SSS;
            /**
             * High-16 `NDS`/`VIDX` register specifier.
             */
            ZyanU8 V2;
            /**
             * Embedded opmask register specifier.
             */
            ZyanU8 kkk;
            /**
             * The offset of the first mvex byte, relative to the beginning of the
             * instruction, in bytes.
             */
            ZyanU8 offset;
        } mvex;
        /**
         * Detailed info about the `ModRM` byte.
         */
        struct ZydisDecodedInstructionModRm_
        {
            /**
             * The addressing mode.
             */
            ZyanU8 mod;
            /**
             * Register specifier or opcode-extension.
             */
            ZyanU8 reg;
            /**
             * Register specifier or opcode-extension.
             */
            ZyanU8 rm;
            /**
             * The offset of the `ModRM` byte, relative to the beginning of the
             * instruction, in bytes.
             */
            ZyanU8 offset;
        } modrm;
        /**
         * Detailed info about the `SIB` byte.
         */
        struct ZydisDecodedInstructionRawSib_
        {
            /**
             * The scale factor.
             */
            ZyanU8 scale;
            /**
             * The index-register specifier.
             */
            ZyanU8 index;
            /**
             * The base-register specifier.
             */
            ZyanU8 base;
            /**
             * The offset of the `SIB` byte, relative to the beginning of the
             * instruction, in bytes.
             */
            ZyanU8 offset;
        } sib;
        /**
         * Detailed info about displacement-bytes.
         */
        struct ZydisDecodedInstructionRawDisp_
        {
            /**
             * The displacement value
             */
            ZyanI64 value;
            /**
             * The physical displacement size, in bits.
             */
            ZyanU8 size;
            // TODO: publish cd8 scale
            /**
             * The offset of the displacement data, relative to the beginning of the
             * instruction, in bytes.
             */
            ZyanU8 offset;
        } disp;
        /**
         * Detailed info about immediate-bytes.
         */
        struct ZydisDecodedInstructionRawImm_
        {
            /**
             * Signals, if the immediate value is signed.
             */
            ZyanBool is_signed;
            /**
             * Signals, if the immediate value contains a relative offset. You can use
             * `ZydisCalcAbsoluteAddress` to determine the absolute address value.
             */
            ZyanBool is_relative;
            /**
             * The immediate value.
             */
            union ZydisDecodedInstructionRawImmValue_
            {
                ZyanU64 u;
                ZyanI64 s;
            } value;
            /**
             * The physical immediate size, in bits.
             */
            ZyanU8 size;
            /**
             * The offset of the immediate data, relative to the beginning of the
             * instruction, in bytes.
             */
            ZyanU8 offset;
        } imm[2];
    } raw;
} ZydisDecodedInstruction;


#endif