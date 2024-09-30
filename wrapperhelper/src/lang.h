#pragma once

#ifndef LANG_H
#define LANG_H

#include "cstring.h"
#include "khash.h"
#include "log.h"
#include "machine.h"
#include "vector.h"

enum token_sym_type_e {
	SYM_LBRACKET,
	SYM_RBRACKET,
	SYM_LSQBRACKET,
	SYM_RSQBRACKET,
	SYM_LPAREN,
	SYM_RPAREN,
	SYM_HASH,
	SYM_HASHHASH,
	SYM_SEMICOLON,
	SYM_COLON,
	SYM_COLONCOLON,
	SYM_VARIADIC,
	SYM_QUESTION,
	SYM_DOT,
	SYM_DASHGT,
	SYM_TILDE,
	SYM_EXCL,
	SYM_PLUS,
	SYM_DASH,
	SYM_STAR,
	SYM_SLASH,
	SYM_PERCENT,
	SYM_HAT,
	SYM_AMP,
	SYM_PIPE,
	SYM_EQ,
	SYM_PLUSEQ,
	SYM_DASHEQ,
	SYM_STAREQ,
	SYM_SLASHEQ,
	SYM_PERCENTEQ,
	SYM_HATEQ,
	SYM_AMPEQ,
	SYM_PIPEEQ,
	SYM_EQEQ,
	SYM_EXCLEQ,
	SYM_LT,
	SYM_GT,
	SYM_LTEQ,
	SYM_GTEQ,
	SYM_AMPAMP,
	SYM_PIPEPIPE,
	SYM_LTLT,
	SYM_GTGT,
	SYM_LTLTEQ,
	SYM_GTGTEQ,
	SYM_PLUSPLUS,
	SYM_DASHDASH,
	SYM_COMMA,
};
#define LAST_SYM SYM_COMMA

typedef struct preproc_token_s {
	enum preproc_token_e {
		PPTOK_INVALID = 0,
		PPTOK_IDENT,       // Expandable ident
		PPTOK_IDENT_UNEXP, // Unexpandable ident
		PPTOK_NUM,
		PPTOK_STRING,
		PPTOK_INCL,
		PPTOK_SYM,
		PPTOK_NEWLINE,
		PPTOK_BLANK,
		PPTOK_START_LINE_COMMENT,
		PPTOK_EOF
	} tokt;
	loginfo_t loginfo;
	union {
		string_t *str;
		struct {
			string_t *sstr; // The string literal content
			int sisstr; // 0 for '' or <>, 1 for ""
		};
		char c;
		enum token_sym_type_e sym;
	} tokv;
} preproc_token_t;
VECTOR_DECLARE(preproc, preproc_token_t)
preproc_token_t preproc_token_dup(preproc_token_t tok);
void preproc_token_del(preproc_token_t *tok);

enum token_keyword_type_e {
	KW_ALIGNAS = 0,
	KW_ALIGNOF,
	KW_ATOMIC,
	KW_AUTO,
	KW_BOOL,
	KW_BREAK,
	KW_CASE,
	KW_CHAR,
	KW_COMPLEX,
	KW_CONST,
	KW_CONTINUE,
	KW_DEFAULT,
	KW_DO,
	KW_DOUBLE,
	KW_ELSE,
	KW_ENUM,
	KW_EXTERN,
	KW_FLOAT,
	KW_FLOAT128,
	KW_FOR,
	KW_GENERIC,
	KW_GOTO,
	KW_IF,
	KW_IMAGINARY,
	KW_INLINE,
	KW_INT,
	KW_INT128,
	KW_LONG,
	KW_NORETURN,
	KW_REGISTER,
	KW_RESTRICT,
	KW_RETURN,
	KW_SHORT,
	KW_SIGNED,
	KW_SIZEOF,
	KW_STATIC,
	KW_STATIC_ASSERT,
	KW_STRUCT,
	KW_SWITCH,
	KW_THREAD_LOCAL,
	KW_TYPEDEF,
	KW_UNION,
	KW_UNSIGNED,
	KW_VOID,
	KW_VOLATILE,
	KW_WHILE,
};
#define LAST_KEYWORD KW_WHILE

typedef struct proc_token_s {
	enum proc_token_e {
		PTOK_INVALID = 0,
		PTOK_IDENT,
		PTOK_KEYWORD,
		PTOK_NUM,
		PTOK_STRING,
		PTOK_SYM,
		PTOK_PRAGMA,
		PTOK_EOF
	} tokt;
	loginfo_t loginfo;
	union proc_token_val_u {
		string_t *str;
		struct {
			string_t *sstr; // The string literal content
			int sisstr; // 0 for '' or <>, 1 for ""
		};
		char c;
		enum token_sym_type_e sym;
		enum token_keyword_type_e kw;
		struct {
			enum proc_pragma_e {
				PRAGMA_ALLOW_INTS,
				PRAGMA_SIMPLE_SU,
				PRAGMA_EXPLICIT_CONV,
				PRAGMA_EXPLICIT_CONV_STRICT,
			} typ;
			string_t *val;
		} pragma;
	} tokv;
} proc_token_t;
VECTOR_DECLARE(proc, proc_token_t)
void proc_token_del(proc_token_t *tok);

typedef struct num_constant_s {
	enum num_constant_e {
		NCT_FLOAT,
		NCT_DOUBLE,
		NCT_LDOUBLE,
		NCT_INT32,
		NCT_UINT32,
		NCT_INT64,
		NCT_UINT64,
	} typ;
	union {
		float f;
		double d;
		long double l;
		int32_t i32;
		uint32_t u32;
		int64_t i64;
		uint64_t u64;
	} val;
} num_constant_t;
int num_constant_convert(loginfo_t *li, string_t *str, num_constant_t *cst, int ptr_is_32bits);
KHASH_MAP_DECLARE_STR(const_map, num_constant_t)

typedef struct expr_s {
	enum expr_type_e {
		ETY_VAR,
		ETY_CONST,
		// ETY_GENERIC,
		ETY_CALL,
		ETY_ACCESS,
		ETY_PTRACCESS, // Convertible to DEREF + ACCESS
		ETY_UNARY,
		ETY_BINARY,
		ETY_TERNARY,
		// ETY_INIT_LIST,
		ETY_CAST,
	} typ;
	union {
		string_t *var;
		num_constant_t cst;
		// TODO: _Generic
		struct {
			struct expr_s *fun, **args;
			size_t nargs;
		} call;
		struct {
			struct expr_s *val;
			string_t *member;
		} access;
		struct {
			enum unary_op_e {
				UOT_POSTINCR,
				UOT_POSTDECR,
				UOT_PREINCR,
				UOT_PREDECR,
				UOT_REF,
				UOT_POS,
				UOT_NEG,
				UOT_DEREF,
				UOT_ANOT, // Arithmetic not, ie '~'
				UOT_BNOT, // Boolean not, ie '!'
			} typ;
			struct expr_s *e;
		} unary;
		struct {
			enum binary_op_e {
				BOT_ADD,
				BOT_SUB,
				BOT_MUL,
				BOT_DIV,
				BOT_MOD,
				BOT_LSH,
				BOT_RSH,
				BOT_LT,
				BOT_GT,
				BOT_LE,
				BOT_GE,
				BOT_EQ,
				BOT_NE,
				BOT_AAND,
				BOT_AXOR,
				BOT_AOR,
				BOT_BAND,
				BOT_BOR,
				BOT_ASSGN_EQ,
				BOT_ASSGN_ADD,
				BOT_ASSGN_SUB,
				BOT_ASSGN_MUL,
				BOT_ASSGN_DIV,
				BOT_ASSGN_MOD,
				BOT_ASSGN_LSH,
				BOT_ASSGN_RSH,
				BOT_ASSGN_AAND,
				BOT_ASSGN_AXOR,
				BOT_ASSGN_AOR,
				BOT_COMMA,
				
				BOT_ARRAY, // Convertible to DEREF + ADD
			} typ;
			struct expr_s *e1, *e2;
		} binary;
		struct {
			enum ternary_op_e {
				TOT_COND,
			} typ;
			struct expr_s *e1, *e2, *e3;
		} ternary;
		// TODO: (type){init}
		struct {
			struct type_s *typ;
			struct expr_s *e;
		} cast;
	} val;
} expr_t;
void expr_del(expr_t *e);
void expr_print(expr_t *e);

typedef struct size_info_s {
	size_t size, align;
} size_info_t;

typedef struct type_s {
	struct {
		unsigned is_atomic : 1;
		unsigned is_const : 1;
		unsigned is_restrict : 1;
		unsigned is_volatile : 1;
		unsigned is_incomplete : 1; // \ The type needs to be complete and
		unsigned is_validated : 1;  // / validated for the size_info to be populated
		unsigned _internal_use : 1;
	};
	size_t nrefs;
	enum type_type_e {
		TYPE_BUILTIN,      // builtin
		TYPE_ARRAY,        // array
		TYPE_STRUCT_UNION, // st
		TYPE_ENUM,         // typ which points to TYPE_BUILTIN
		TYPE_PTR,          // typ
		TYPE_FUNCTION,     // fun
	} typ;
	union {
		enum type_builtin_e {
			BTT_VOID,
			BTT_BOOL,
			BTT_CHAR,
			BTT_SCHAR,
			BTT_UCHAR,
			BTT_SHORT,
			BTT_SSHORT,
			BTT_USHORT,
			BTT_INT,
			BTT_SINT,
			BTT_UINT,
			BTT_LONG,
			BTT_SLONG,
			BTT_ULONG,
			BTT_LONGLONG,
			BTT_SLONGLONG,
			BTT_ULONGLONG,
			BTT_INT128,
			BTT_SINT128,
			BTT_UINT128,
			BTT_S8,
			BTT_U8,
			BTT_S16,
			BTT_U16,
			BTT_S32,
			BTT_U32,
			BTT_S64,
			BTT_U64,
#define BTT_START_INT_EXT BTT_S8
#define BTT_INT_EXTS "__int8_t", "__uint8_t", "__int16_t", "__uint16_t", "__int32_t", "__uint32_t", "__int64_t", "__uint64_t"
			BTT_FLOAT,
			BTT_CFLOAT,
			BTT_IFLOAT,
			BTT_DOUBLE,
			BTT_CDOUBLE,
			BTT_IDOUBLE,
			BTT_LONGDOUBLE,
			BTT_CLONGDOUBLE,
			BTT_ILONGDOUBLE,
			BTT_FLOAT128,
			BTT_CFLOAT128,
			BTT_IFLOAT128,
			BTT_VA_LIST,
		} builtin;
#define LAST_BUILTIN BTT_VA_LIST
		struct type_s *typ;
		struct {
			struct type_s *typ;
			size_t array_sz; // -1 for VLA
		} array;
		struct struct_s *st;
		struct {
			struct type_s *ret;
			size_t nargs; // -1 for no specification
			struct type_s **args;
			int has_varargs;
		} fun;
	} val;
	size_info_t szinfo;
	string_t *converted; // NULL for default behavior
} type_t;
void type_del(type_t *typ);
KHASH_MAP_DECLARE_STR(type_map, type_t*)
KHASH_DECLARE(conv_map, type_t*, string_t*)
void type_map_del(khash_t(type_map) *map);

int type_t_equal(type_t*, type_t*);

typedef struct st_member_s {
	string_t *name; // May be NULL
	type_t *typ;
	_Bool is_bitfield;
	size_t bitfield_width;
	// Filled by validate_type
	size_t byte_offset; unsigned char bit_offset;
} st_member_t;
typedef struct struct_s {
	string_t *tag;
	int is_defined;
	size_t nrefs;
	int is_struct; // 0 = union, 1 = struct
	int is_simple; // Pointers to the structure (in 64 bits) are simple pointers
	int has_incomplete; // 1 if the last element of the structure is a VLA or if an element of the union recursively contains a VLA
	int has_self_recursion; // 1 if the structure contains a reference to itself
	size_t nmembers;
	st_member_t *members;
} struct_t;
void st_member_del(st_member_t *member);
void struct_del(struct_t *st);
KHASH_MAP_DECLARE_STR(struct_map, struct_t*)

type_t *type_new(void); // Create a new (complete) builtin type
type_t *type_new_ptr(type_t *target); // Create a new pointer type; doesn't increment the use counter of the target
int type_copy_into(type_t *dest, const type_t *ref); // Copy ref into dest, keeping additional qualifiers and without changing any use counter

struct_t *struct_new(int is_struct, string_t *tag); // Create a new struct

// Try to merge some types with other types; this may delete ptr and increase a use counter in a type referenced by the table
KHASH_DECLARE(type_set, type_t*, char)
type_t *type_try_merge(type_t *ptr, khash_t(type_set) *set);

extern const char *builtin2str[LAST_BUILTIN + 1];
void type_print(type_t *typ);
void struct_print(const struct_t *st);

typedef struct declaration_s {
	enum decl_storage_e {
		STORAGE_NONE,
		STORAGE_EXTERN,
		STORAGE_STATIC,
		STORAGE_TLS,
		STORAGE_TLS_EXTERN,
		STORAGE_TLS_STATIC,
		STORAGE_AUTO,
		STORAGE_REG,
	} storage;
	int defined;
	type_t *typ;
} declaration_t;
KHASH_MAP_DECLARE_STR(decl_map, declaration_t*)

typedef struct file_s {
	khash_t(struct_map) *struct_map;
	khash_t(type_map) *type_map;
	khash_t(conv_map) *relaxed_type_conversion;
	khash_t(type_map) *enum_map;
	khash_t(decl_map) *decl_map;
	type_t *builtins[LAST_BUILTIN + 1];
	khash_t(const_map) *const_map;
	khash_t(type_set) *type_set;
} file_t;
file_t *file_new(machine_t *target);
void file_del(file_t *f);

extern const char *sym2str[LAST_SYM + 1];
extern const char *kw2str[LAST_KEYWORD + 1];
void preproc_token_print(const preproc_token_t *tok);
int preproc_token_isend(const preproc_token_t *tok);
void proc_token_print(const proc_token_t *tok);
int proc_token_iserror(const proc_token_t *tok);
int proc_token_isend(const proc_token_t *tok);

KHASH_MAP_DECLARE_STR(str2kw, enum token_keyword_type_e)
extern khash_t(str2kw) *str2kw;
int init_str2kw(void);
void del_str2kw(void);

#endif // LANG_H
