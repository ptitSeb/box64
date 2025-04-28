#include "lang.h"

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if 1
#define DISP_ADDR_FMT ""
#define DISP_ADDR_ARG(v) 
#else
#define DISP_ADDR_FMT "%p "
#define DISP_ADDR_ARG(v) v,
#endif

preproc_token_t preproc_token_dup(preproc_token_t tok) {
	preproc_token_t ret;
	ret.tokt = tok.tokt;
	switch (tok.tokt) {
	case PPTOK_IDENT:
	case PPTOK_IDENT_UNEXP:
	case PPTOK_NUM:
		ret.tokv.str = string_dup(tok.tokv.str);
		if (!ret.tokv.str) {
			ret.tokt = PPTOK_INVALID;
			ret.tokv.c = '\0';
		}
		break;
	case PPTOK_STRING:
	case PPTOK_INCL:
		ret.tokv.sisstr = tok.tokv.sisstr;
		ret.tokv.sstr = string_dup(tok.tokv.sstr);
		if (!ret.tokv.sstr) {
			ret.tokt = PPTOK_INVALID;
			ret.tokv.c = '\0';
		}
		string_del(tok.tokv.sstr);
		break;
	case PPTOK_INVALID:
	case PPTOK_SYM:
	case PPTOK_NEWLINE:
	case PPTOK_BLANK:
	case PPTOK_START_LINE_COMMENT:
	case PPTOK_EOF:
		ret.tokv.c = tok.tokv.c;
		break;
	}
	return ret;
}

void preproc_token_del(preproc_token_t *tok) {
	switch (tok->tokt) {
	case PPTOK_IDENT:
	case PPTOK_IDENT_UNEXP:
	case PPTOK_NUM:
		string_del(tok->tokv.str);
		break;
	case PPTOK_STRING:
	case PPTOK_INCL:
		string_del(tok->tokv.sstr);
		break;
	case PPTOK_INVALID:
	case PPTOK_SYM:
	case PPTOK_NEWLINE:
	case PPTOK_BLANK:
	case PPTOK_START_LINE_COMMENT:
	case PPTOK_EOF:
		break;
	}
}

VECTOR_IMPL(preproc, preproc_token_del)

void proc_token_del(proc_token_t *tok) {
	switch (tok->tokt) {
	case PTOK_IDENT:
	case PTOK_NUM:
		string_del(tok->tokv.str);
		break;
	case PTOK_STRING:
		string_del(tok->tokv.sstr);
		break;
	case PTOK_PRAGMA:
		switch (tok->tokv.pragma.typ) {
		case PRAGMA_SIMPLE_SU:
		case PRAGMA_EXPLICIT_CONV:
		case PRAGMA_EXPLICIT_CONV_STRICT:
			string_del(tok->tokv.pragma.val);
			break;
		case PRAGMA_ALLOW_INTS:
			break;
		}
		break;
	case PTOK_INVALID:
	case PTOK_KEYWORD:
	case PTOK_SYM:
	case PTOK_EOF:
		break;
	}
}

VECTOR_IMPL(proc, proc_token_del)

const char *sym2str[LAST_SYM + 1] = {
	[SYM_LBRACKET] = "{",
	[SYM_RBRACKET] = "}",
	[SYM_LSQBRACKET] = "[",
	[SYM_RSQBRACKET] = "]",
	[SYM_LPAREN] = "(",
	[SYM_RPAREN] = ")",
	[SYM_HASH] = "#",
	[SYM_HASHHASH] = "##",
	[SYM_SEMICOLON] = ";",
	[SYM_COLON] = ":",
	[SYM_COLONCOLON] = "::",
	[SYM_VARIADIC] = "...",
	[SYM_QUESTION] = "?",
	[SYM_DOT] = ".",
	[SYM_DASHGT] = "->",
	[SYM_TILDE] = "~",
	[SYM_EXCL] = "!",
	[SYM_PLUS] = "+",
	[SYM_DASH] = "-",
	[SYM_STAR] = "*",
	[SYM_SLASH] = "/",
	[SYM_PERCENT] = "%",
	[SYM_HAT] = "^",
	[SYM_AMP] = "&",
	[SYM_PIPE] = "|",
	[SYM_EQ] = "=",
	[SYM_PLUSEQ] = "+=",
	[SYM_DASHEQ] = "-=",
	[SYM_STAREQ] = "*=",
	[SYM_SLASHEQ] = "/=",
	[SYM_PERCENTEQ] = "%=",
	[SYM_HATEQ] = "^=",
	[SYM_AMPEQ] = "&=",
	[SYM_PIPEEQ] = "|=",
	[SYM_EQEQ] = "==",
	[SYM_EXCLEQ] = "!=",
	[SYM_LT] = "<",
	[SYM_GT] = ">",
	[SYM_LTEQ] = "<=",
	[SYM_GTEQ] = ">=",
	[SYM_AMPAMP] = "&&",
	[SYM_PIPEPIPE] = "||",
	[SYM_LTLT] = "<<",
	[SYM_GTGT] = ">>",
	[SYM_LTLTEQ] = "<<=",
	[SYM_GTGTEQ] = ">>=",
	[SYM_PLUSPLUS] = "++",
	[SYM_DASHDASH] = "--",
	[SYM_COMMA] = ",",
};

void preproc_token_print(const preproc_token_t *tok) {
	printf("Token: ");
	loginfo_print(&tok->loginfo, 50);
	switch (tok->tokt) {
	case PPTOK_INVALID:
		printf("%7s %hhd (%c)\n", "#INVAL#", tok->tokv.c, (tok->tokv.c >= 0x20) && (tok->tokv.c < 0x7F) ? tok->tokv.c : '?');
		break;
	case PPTOK_IDENT:
		printf("%7s '%s'\n", "IDENT", string_content(tok->tokv.str));
		break;
	case PPTOK_IDENT_UNEXP:
		printf("%7s '%s'\n", "IDENT'", string_content(tok->tokv.str));
		break;
	case PPTOK_NUM:
		printf("%7s '%s'\n", "NUM", string_content(tok->tokv.str));
		break;
	case PPTOK_STRING:
		printf("%7s %c%s%c\n", "STRING",
			tok->tokv.sisstr ? '"' : '\'', string_content(tok->tokv.sstr), tok->tokv.sisstr ? '"' : '\'');
		break;
	case PPTOK_INCL:
		printf("%7s %c%s%c\n", "INCL",
			tok->tokv.sisstr ? '"' : '<', string_content(tok->tokv.sstr), tok->tokv.sisstr ? '"' : '>');
		break;
	case PPTOK_SYM:
		printf("%7s  %-3s (%u)\n", "SYM", sym2str[tok->tokv.sym], tok->tokv.sym);
		break;
	case PPTOK_NEWLINE:
		printf("%7s\n", "NEWLINE");
		break;
	case PPTOK_BLANK:
		printf("%7s\n", "\e[2;31m(blank)\e[m");
		break;
	case PPTOK_START_LINE_COMMENT:
		printf("%7s\n", "\e[2;31m( // ) \e[m");
		break;
	case PPTOK_EOF:
		printf("%7s\n", "EOF");
		break;
	default:
		printf("??? %u\n", tok->tokt);
	}
}

int preproc_token_isend(const preproc_token_t *tok) {
	switch (tok->tokt) {
	case PPTOK_IDENT:
	case PPTOK_IDENT_UNEXP:
	case PPTOK_NUM:
	case PPTOK_STRING:
	case PPTOK_INCL:
	case PPTOK_SYM:
	case PPTOK_NEWLINE:
	case PPTOK_BLANK:
	case PPTOK_START_LINE_COMMENT:
		return 0;
	case PPTOK_INVALID:
	case PPTOK_EOF:
	default:
		return 1;
	}
}

const char *kw2str[LAST_KEYWORD + 1] = {
	[KW_ALIGNAS] = "_Alignas",
	[KW_ALIGNOF] = "_Alignof",
	[KW_ATOMIC] = "_Atomic",
	[KW_AUTO] = "auto",
	[KW_BOOL] = "_Bool",
	[KW_BREAK] = "break",
	[KW_CASE] = "case",
	[KW_CHAR] = "char",
	[KW_COMPLEX] = "_Complex",
	[KW_CONST] = "const",
	[KW_CONTINUE] = "continue",
	[KW_DEFAULT] = "default",
	[KW_DO] = "do",
	[KW_DOUBLE] = "double",
	[KW_ELSE] = "else",
	[KW_ENUM] = "enum",
	[KW_EXTERN] = "extern",
	[KW_FLOAT] = "float",
	[KW_FLOAT128] = "__float128",
	[KW_FOR] = "for",
	[KW_GENERIC] = "_Generic",
	[KW_GOTO] = "goto",
	[KW_IF] = "if",
	[KW_IMAGINARY] = "_Imaginary",
	[KW_INLINE] = "inline",
	[KW_INT] = "int",
	[KW_INT128] = "__int128",
	[KW_LONG] = "long",
	[KW_NORETURN] = "_Noreturn",
	[KW_REGISTER] = "register",
	[KW_RESTRICT] = "restrict",
	[KW_RETURN] = "return",
	[KW_SHORT] = "short",
	[KW_SIGNED] = "signed",
	[KW_SIZEOF] = "sizeof",
	[KW_STATIC] = "static",
	[KW_STATIC_ASSERT] = "_Static_assert",
	[KW_STRUCT] = "struct",
	[KW_SWITCH] = "switch",
	[KW_THREAD_LOCAL] = "_Thread_local",
	[KW_TYPEDEF] = "typedef",
	[KW_UNION] = "union",
	[KW_UNSIGNED] = "unsigned",
	[KW_VOID] = "void",
	[KW_VOLATILE] = "volatile",
	[KW_WHILE] = "while",
};

void proc_token_print(const proc_token_t *tok) {
	printf("Token: ");
	loginfo_print(&tok->loginfo, 50);
	switch (tok->tokt) {
	case PTOK_INVALID:
		printf("%7s %hhd (%c)\n", "#INVAL#", tok->tokv.c, (tok->tokv.c >= 0x20) && (tok->tokv.c < 0x7F) ? tok->tokv.c : '?');
		break;
	case PTOK_IDENT:
		printf("%7s '%s'\n", "IDENT", string_content(tok->tokv.str));
		break;
	case PTOK_KEYWORD:
		printf("%7s '%s' (%u)\n", "KEYWORD", kw2str[tok->tokv.kw], tok->tokv.kw);
		break;
	case PTOK_NUM:
		printf("%7s '%s'\n", "NUM", string_content(tok->tokv.str));
		break;
	case PTOK_STRING:
		printf("%7s %c%s%c\n", "STRING",
			tok->tokv.sisstr ? '"' : '\'', string_content(tok->tokv.sstr), tok->tokv.sisstr ? '"' : '\'');
		break;
	case PTOK_SYM:
		printf("%7s  %-3s (%u)\n", "SYM", sym2str[tok->tokv.sym], tok->tokv.sym);
		break;
	case PTOK_PRAGMA:
		switch (tok->tokv.pragma.typ) {
		case PRAGMA_ALLOW_INTS:
			printf("%7s Allow ints\n", "PRAGMA");
			break;
		case PRAGMA_SIMPLE_SU:
			printf("%7s Mark simple: struct or union %s is simple\n", "PRAGMA", string_content(tok->tokv.pragma.val));
			break;
		case PRAGMA_EXPLICIT_CONV:
			printf("%7s Relaxed explicit conversion: destination is %s\n", "PRAGMA", string_content(tok->tokv.pragma.val));
			break;
		case PRAGMA_EXPLICIT_CONV_STRICT:
			printf("%7s Strict explicit conversion: destination is %s\n", "PRAGMA", string_content(tok->tokv.pragma.val));
			break;
		default:
			printf("%7s ??? %u\n", "PRAGMA", tok->tokv.pragma.typ);
		}
		break;
	case PTOK_EOF:
		printf("%7s\n", "EOF");
		break;
	default:
		printf("??? %u\n", tok->tokt);
	}
}

int proc_token_iserror(const proc_token_t *tok) {
	switch (tok->tokt) {
	case PTOK_IDENT:
	case PTOK_KEYWORD:
	case PTOK_NUM:
	case PTOK_STRING:
	case PTOK_SYM:
	case PTOK_PRAGMA:
	case PTOK_EOF:
		return 0;
	case PTOK_INVALID:
	default:
		return 1;
	}
}

int proc_token_isend(const proc_token_t *tok) {
	switch (tok->tokt) {
	case PTOK_IDENT:
	case PTOK_KEYWORD:
	case PTOK_NUM:
	case PTOK_STRING:
	case PTOK_SYM:
	case PTOK_PRAGMA:
		return 0;
	case PTOK_INVALID:
	case PTOK_EOF:
	default:
		return 1;
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-null-dereference"
KHASH_MAP_IMPL_STR(str2kw, enum token_keyword_type_e)
KHASH_MAP_IMPL_STR(type_map, type_t*)
KHASH_MAP_IMPL_STR(decl_map, declaration_t*)
KHASH_MAP_IMPL_STR(struct_map, struct_t*)
KHASH_MAP_IMPL_STR(const_map, num_constant_t)
#pragma GCC diagnostic pop
khash_t(str2kw) *str2kw;

int init_str2kw(void) {
	if (!(str2kw = kh_init(str2kw))) {
		log_memory("failed to create the string to keyword map (init)\n");
		return 0;
	}
	int iret; khiter_t it;
	for (enum token_keyword_type_e kw = 0; kw <= LAST_KEYWORD; ++kw) {
		it = kh_put(str2kw, str2kw, kw2str[kw], &iret);
		if (iret < 0) {
			log_memory("failed to create the string to keyword map (keyword %u)\n", kw);
			kh_destroy(str2kw, str2kw);
			return 0;
		}
		kh_val(str2kw, it) = kw;
	}
#define ADD_ALIAS(alias, k) \
	it = kh_put(str2kw, str2kw, #alias, &iret);                                                \
	if (iret < 0) {                                                                            \
		log_memory("failed to create the string to keyword map (keyword alias " #alias ")\n"); \
		kh_destroy(str2kw, str2kw);                                                            \
		return 0;                                                                              \
	}                                                                                          \
	kh_val(str2kw, it) = KW_ ## k;
	ADD_ALIAS(__alignof__,   ALIGNOF)
	// ADD_ALIAS(__asm,         asm)
	// ADD_ALIAS(__asm__,       asm)
	ADD_ALIAS(__complex,     COMPLEX)
	ADD_ALIAS(__complex__,   COMPLEX)
	ADD_ALIAS(__const,       CONST)
	ADD_ALIAS(__const__,     CONST)
	// ADD_ALIAS(__decltype,    decltype)
	// ADD_ALIAS(__imag__,      __imag)
	ADD_ALIAS(__inline,      INLINE)
	ADD_ALIAS(__inline__,    INLINE)
	// ADD_ALIAS(__nullptr,     nullptr)
	// ADD_ALIAS(__real__,      __real)
	ADD_ALIAS(__restrict,    RESTRICT)
	ADD_ALIAS(__restrict__,  RESTRICT)
	ADD_ALIAS(__signed,      SIGNED)
	ADD_ALIAS(__signed__,    SIGNED)
	// ADD_ALIAS(__typeof,      typeof)
	// ADD_ALIAS(__typeof__,    typeof)
	ADD_ALIAS(__volatile,    VOLATILE)
	ADD_ALIAS(__volatile__,  VOLATILE)
	return 1;
}
void del_str2kw(void) {
	kh_destroy(str2kw, str2kw);
}

// ptr_is_32bits is actually long_is_32bits
int num_constant_convert(loginfo_t *li, string_t *str, num_constant_t *cst, int ptr_is_32bits) {
	if (string_len(str) == 0) return 0; // Should never happen
#define contains(c) strchr(string_content(str), c)
	if (contains('.')
	     || (!(contains('X') || contains('x')) && (contains('E') || contains('e')))
	     || ((contains('X') || contains('x')) && (contains('P') || contains('p')))) {
#undef contains
		int ok;
		if ((string_end(str)[-1] == 'l') || (string_end(str)[-1] == 'L')) {
			cst->typ = NCT_LDOUBLE;
			char *endc;
			cst->val.l = strtold(string_content(str), &endc);
			ok = (endc == string_end(str) - 1);
		} else if ((string_end(str)[-1] == 'f') || (string_end(str)[-1] == 'F')) {
			cst->typ = NCT_FLOAT;
			char *endc;
			cst->val.f = strtof(string_content(str), &endc);
			ok = (endc == string_end(str) - 1);
		} else {
			cst->typ = NCT_DOUBLE;
			char *endc;
			cst->val.d = strtod(string_content(str), &endc);
			ok = (endc == string_end(str));
		}
		if (!ok) {
			log_error(li, "'%s' is not a valid number\n", string_content(str));
			return 0;
		} else if (errno == ERANGE) {
			log_warning(li, "floating point constant is too large\n");
			return 1;
		}
		return 1;
	} else {
		uint64_t ret = 0;
		unsigned base = (string_content(str)[0] == '0') ? (string_content(str)[1] == 'x') ? 16 : 8 : 10;
		size_t startidx = (base == 16) ? 2 : 0;
		size_t endidx = string_len(str);
		int suffix_type = 0;
#define SUFFIX_U 1
#define SUFFIX_L 2
#define SUFFIX_LL 4
#define SUFFIX_SGN SUFFIX_U
#define SUFFIX_SZ (SUFFIX_L | SUFFIX_LL)
	remove_suffix:
		if ((string_content(str)[endidx - 1] == 'l') || (string_content(str)[endidx - 1] == 'L')) {
			if (suffix_type & SUFFIX_SZ) {
				log_error(li, "'%s' is not a valid number (invalid suffix)\n", string_content(str));
				return 0;
			}
			if (endidx == 1) return 0; // Should never happen
			if ((string_content(str)[endidx - 2] == 'l') || (string_content(str)[endidx - 2] == 'L')) {
				if (endidx == 2) return 0; // Should never happen
				if (string_content(str)[endidx - 2] != string_content(str)[endidx - 1]) {
					log_error(li, "'%s' is not a valid number (invalid suffix)\n", string_content(str));
					return 0;
				}
				endidx -= 2;
				suffix_type |= SUFFIX_LL;
			} else {
				endidx -= 1;
				suffix_type |= SUFFIX_L;
			}
			goto remove_suffix;
		}
		if ((string_content(str)[endidx - 1] == 'u') || (string_content(str)[endidx - 1] == 'U')) {
			if (suffix_type & SUFFIX_SGN) {
				log_error(li, "'%s' is not a valid number (invalid suffix)\n", string_content(str));
				return 0;
			}
			endidx -= 1;
			suffix_type |= SUFFIX_U;
			goto remove_suffix;
		}
		// 0u has startidx=0 endidx=1
		if (endidx <= startidx) {
			log_error(li, "'%s' is not a valid number\n", string_content(str));
		}
		for (size_t i = startidx; i < endidx; ++i) {
			if ((string_content(str)[i] >= '0') && (string_content(str)[i] <= ((base == 8) ? '7' : '9'))) {
				ret = base * ret + (unsigned)(string_content(str)[i] - '0');
			} else if ((base == 16) && (string_content(str)[i] >= 'A') && (string_content(str)[i] <= 'F')) {
				ret = base * ret + 10 + (unsigned)(string_content(str)[i] - 'A');
			} else if ((base == 16) && (string_content(str)[i] >= 'a') && (string_content(str)[i] <= 'f')) {
				ret = base * ret + 10 + (unsigned)(string_content(str)[i] - 'a');
			} else {
				log_error(li, "'%s' is not a valid number\n", string_content(str));
				return 0;
			}
		}
		// If base == 10, keep the signness; in any case, try 32 bits if available else use 64 bits
		cst->typ =
			((suffix_type & SUFFIX_SGN) == SUFFIX_U) ?
				((suffix_type & SUFFIX_SZ) == SUFFIX_L) ? ptr_is_32bits ? NCT_UINT32 : NCT_UINT64 :
				((suffix_type & SUFFIX_SZ) == SUFFIX_LL) ? NCT_UINT64 :
					NCT_UINT32 :
				((suffix_type & SUFFIX_SZ) == SUFFIX_L) ? ptr_is_32bits ? NCT_INT32 : NCT_INT64 :
				((suffix_type & SUFFIX_SZ) == SUFFIX_LL) ? NCT_INT64 :
					NCT_INT32;
		if (cst->typ == NCT_INT32) {
			if (ret < 1ULL << 31) {
				cst->val.i32 = (int32_t)ret;
				return 1;
			}
			// Not in signed 32-bits = int => try long/long long for decimal, unsigned for hexadecimal/octal
			if (base != 10) cst->typ = NCT_UINT32;
			else cst->typ = NCT_INT64;
		}
		if (cst->typ == NCT_UINT32) {
			if (ret < 1ULL << 32) {
				cst->val.u32 = (uint32_t)ret;
				return 1;
			}
			// Not in unsigned 32-bits = unsigned => try unsigned long/long long for decimal and unsigned, long/long long for signed hexadecimal/octal
			if ((base != 10) && ((suffix_type & SUFFIX_SGN) != SUFFIX_U)) cst->typ = NCT_INT64;
			else cst->typ = NCT_UINT64;
		}
		if (cst->typ == NCT_INT64) {
			if (ret < 1ULL << 63) {
				cst->val.i64 = (int64_t)ret;
				return 1;
			}
			// Not in signed 64-bits = long/long long => fail for decimal, try unsigned long/long long for hexadecimal/octal
			if (base != 10) cst->typ = NCT_UINT64;
		}
		if (cst->typ == NCT_UINT64) {
			cst->val.u64 = ret;
			return 1;
		}
		// The constant cannot be typed... (Or internal error)
		return 0;
	}
}

void expr_del(expr_t *e) {
	switch (e->typ) {
	case ETY_VAR:
		string_del(e->val.var);
		break;
		
	case ETY_CONST:
		break;
		
	// case ETY_GENERIC:
	// TODO
		
	case ETY_CALL:
		expr_del(e->val.call.fun);
		for (size_t i = 0; i < e->val.call.nargs; ++i) {
			expr_del(e->val.call.args[i]);
		}
		if (e->val.call.args) free(e->val.call.args);
		break;
		
	case ETY_ACCESS:
	case ETY_PTRACCESS:
		expr_del(e->val.access.val);
		string_del(e->val.access.member);
		break;
		
	case ETY_UNARY:
		expr_del(e->val.unary.e);
		break;
		
	case ETY_BINARY:
		expr_del(e->val.binary.e1);
		expr_del(e->val.binary.e2);
		break;
		
	case ETY_TERNARY:
		expr_del(e->val.ternary.e1);
		expr_del(e->val.ternary.e2);
		expr_del(e->val.ternary.e3);
		break;
		
	// case ETY_INIT_LIST:
	// TODO
		
	case ETY_CAST:
		type_del(e->val.cast.typ);
		expr_del(e->val.cast.e);
		break;
	}
	free(e);
}

static void expr_print_aux(expr_t *e, int parent_level) {
	switch (e->typ) {
	case ETY_VAR:
		// Level 0, no parenthesis
		printf("%s", string_content(e->val.var));
		break;
		
	case ETY_CONST:
		// Level 0, no parenthesis
		switch (e->val.cst.typ) {
		case NCT_FLOAT: printf("%ff", e->val.cst.val.f); break;
		case NCT_DOUBLE: printf("%f", e->val.cst.val.d); break;
		case NCT_LDOUBLE: printf("%Lfl", e->val.cst.val.l); break;
		case NCT_INT32: printf("%d", e->val.cst.val.i32); break;
		case NCT_UINT32: printf("%uu", e->val.cst.val.u32); break;
		case NCT_INT64: printf("%ldll", e->val.cst.val.i64); break;
		case NCT_UINT64: printf("%lullu", e->val.cst.val.u64); break;
		}
		break;
		
	case ETY_CALL:
		// Level 1, no parenthesis
		expr_print_aux(e->val.call.fun, 1);
		printf("(");
		for (size_t i = 0; i < e->val.call.nargs; ++i) {
			expr_print_aux(e->val.call.fun, 15);
		}
		printf(")");
		break;
		
	case ETY_ACCESS:
		// Level 1, no parenthesis
		expr_print_aux(e->val.access.val, 1);
		printf(".%s", string_content(e->val.access.member));
		break;
		
	case ETY_PTRACCESS:
		// Level 1, no parenthesis
		expr_print_aux(e->val.access.val, 1);
		printf(".%s", string_content(e->val.access.member));
		break;
		
	case ETY_UNARY: {
		const char *preop = NULL;
		switch (e->val.unary.typ) {
		case UOT_POSTINCR:
			// Level 1, no parenthesis
			expr_print_aux(e->val.unary.e, 1);
			printf("++");
			break;
		case UOT_POSTDECR:
			// Level 1, no parenthesis
			expr_print_aux(e->val.unary.e, 1);
			printf("--");
			break;
		case UOT_PREINCR: preop = "++"; break;
		case UOT_PREDECR: preop = "--"; break;
		case UOT_REF: preop = "&"; break;
		case UOT_POS: preop = "+"; break;
		case UOT_NEG: preop = "-"; break;
		case UOT_DEREF: preop = "*"; break;
		case UOT_ANOT: preop = "~"; break;
		case UOT_BNOT: preop = "!"; break;
		}
		if (preop) {
			if (parent_level < 2) printf("(");
			printf("%s", preop);
			expr_print_aux(e->val.unary.e, 2);
			if (parent_level < 2) printf(")");
		}
		break; }
		
	case ETY_BINARY: {
		const char *midop, *postop = NULL;
		int mlevel, llevel, rlevel;
		switch (e->val.binary.typ) {
		case BOT_ADD:        mlevel =  5; llevel =  5; rlevel =  4; midop = " + "; break;
		case BOT_SUB:        mlevel =  5; llevel =  5; rlevel =  4; midop = " - "; break;
		case BOT_MUL:        mlevel =  4; llevel =  4; rlevel =  3; midop = " * "; break;
		case BOT_DIV:        mlevel =  4; llevel =  4; rlevel =  3; midop = " / "; break;
		case BOT_MOD:        mlevel =  4; llevel =  4; rlevel =  3; midop = " % "; break;
		case BOT_LSH:        mlevel =  6; llevel =  6; rlevel =  5; midop = " << "; break;
		case BOT_RSH:        mlevel =  6; llevel =  6; rlevel =  5; midop = " >> "; break;
		case BOT_LT:         mlevel =  7; llevel =  7; rlevel =  6; midop = " < "; break;
		case BOT_GT:         mlevel =  7; llevel =  7; rlevel =  6; midop = " > "; break;
		case BOT_LE:         mlevel =  7; llevel =  7; rlevel =  6; midop = " <= "; break;
		case BOT_GE:         mlevel =  7; llevel =  7; rlevel =  6; midop = " >= "; break;
		case BOT_EQ:         mlevel =  8; llevel =  8; rlevel =  7; midop = " == "; break;
		case BOT_NE:         mlevel =  8; llevel =  8; rlevel =  7; midop = " != "; break;
		case BOT_AAND:       mlevel =  9; llevel =  9; rlevel =  8; midop = " & "; break;
		case BOT_AXOR:       mlevel = 10; llevel = 10; rlevel =  9; midop = " ^ "; break;
		case BOT_AOR:        mlevel = 11; llevel = 11; rlevel = 10; midop = " | "; break;
		case BOT_BAND:       mlevel = 12; llevel = 12; rlevel = 11; midop = " && "; break;
		case BOT_BOR:        mlevel = 13; llevel = 13; rlevel = 12; midop = " || "; break;
		case BOT_ASSGN_EQ:   mlevel = 15; llevel =  2; rlevel = 15; midop = " = "; break;
		case BOT_ASSGN_ADD:  mlevel = 15; llevel =  2; rlevel = 15; midop = " += "; break;
		case BOT_ASSGN_SUB:  mlevel = 15; llevel =  2; rlevel = 15; midop = " -= "; break;
		case BOT_ASSGN_MUL:  mlevel = 15; llevel =  2; rlevel = 15; midop = " *= "; break;
		case BOT_ASSGN_DIV:  mlevel = 15; llevel =  2; rlevel = 15; midop = " /= "; break;
		case BOT_ASSGN_MOD:  mlevel = 15; llevel =  2; rlevel = 15; midop = " %= "; break;
		case BOT_ASSGN_LSH:  mlevel = 15; llevel =  2; rlevel = 15; midop = " <<= "; break;
		case BOT_ASSGN_RSH:  mlevel = 15; llevel =  2; rlevel = 15; midop = " >>= "; break;
		case BOT_ASSGN_AAND: mlevel = 15; llevel =  2; rlevel = 15; midop = " &= "; break;
		case BOT_ASSGN_AXOR: mlevel = 15; llevel =  2; rlevel = 15; midop = " ^= "; break;
		case BOT_ASSGN_AOR:  mlevel = 15; llevel =  2; rlevel = 15; midop = " |= "; break;
		case BOT_COMMA:      mlevel = 16; llevel = 16; rlevel = 15; midop = ", "; break;
		case BOT_ARRAY:      mlevel =  1; llevel =  1; rlevel = 16; midop = "["; postop = "]"; break;
		default: mlevel = 17; llevel = rlevel = -1; midop = " ? "; break;
		}
		if (parent_level < mlevel) printf("(");
		expr_print_aux(e->val.binary.e1, llevel);
		printf("%s", midop);
		expr_print_aux(e->val.binary.e2, rlevel);
		if (postop) printf("%s", midop);
		if (parent_level < mlevel) printf(")");
		break; }
		
	case ETY_TERNARY:
		switch (e->val.ternary.typ) {
		case TOT_COND:
			if (parent_level < 15) printf("(");
			expr_print_aux(e->val.ternary.e1, 14);
			printf(" ? ");
			expr_print_aux(e->val.ternary.e2, 16);
			expr_print_aux(e->val.ternary.e2, 15);
			printf(" : ");
			if (parent_level < 15) printf(")");
			break;
		}
		break;
		
	case ETY_CAST:
		if (parent_level < 3) printf("(");
		printf("(");
		type_print(e->val.cast.typ);
		printf(")");
		expr_print(e->val.cast.e);
		if (parent_level < 3) printf(")");
		break;
	}
}
void expr_print(expr_t *e) {
	return expr_print_aux(e, 16);
}

void struct_del_weak(struct_t *st) {
	if (--st->nrefs) return;
	if (!st->tag) struct_del(st);
}
void type_del(type_t *typ) {
	if (--typ->nrefs) return;
	if (typ->converted) string_del(typ->converted);
	switch (typ->typ) {
	case TYPE_BUILTIN:
		break;
	case TYPE_ARRAY:
		type_del(typ->val.array.typ);
		break;
	case TYPE_STRUCT_UNION:
		struct_del_weak(typ->val.st);
		break;
	case TYPE_ENUM:
	case TYPE_PTR:
		type_del(typ->val.typ);
		break;
	case TYPE_FUNCTION:
		type_del(typ->val.fun.ret);
		if (typ->val.fun.nargs != (size_t)-1) {
			for (size_t i = 0; i < typ->val.fun.nargs; ++i) {
				type_del(typ->val.fun.args[i]);
			}
		}
		if (typ->val.fun.args) free(typ->val.fun.args);
		break;
	}
	free(typ);
}
void type_map_del(khash_t(type_map) *map) {
	kh_cstr_t str;
	type_t **it;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	kh_foreach_key_value_ref(map, str, it, free((void*)str); type_del(*it))
#pragma GCC diagnostic pop
	kh_destroy(type_map, map);
}
void conv_map_del(khash_t(conv_map) *map) {
	type_t *typ;
	string_t **conv;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	kh_foreach_key_value_ref(map, typ, conv, type_del(typ); string_del(*conv))
#pragma GCC diagnostic pop
	kh_destroy(conv_map, map);
}
void type_set_del(khash_t(type_set) *set) {
	type_t *it;
	kh_foreach_key(set, it, type_del(it))
	kh_destroy(type_set, set);
}

void const_map_del(khash_t(const_map) *map) {
	kh_cstr_t str;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	kh_foreach_key(map, str, free((void*)str))
#pragma GCC diagnostic pop
	kh_destroy(const_map, map);
}

void decl_del(declaration_t *decl) {
	type_del(decl->typ);
	free(decl);
}
void decl_map_del(khash_t(decl_map) *map) {
	kh_cstr_t str;
	declaration_t **it;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	kh_foreach_key_value_ref(map, str, it, free((void*)str); decl_del(*it))
#pragma GCC diagnostic pop
	kh_destroy(decl_map, map);
}

void st_member_del(st_member_t *member) {
	if (member->name) string_del(member->name);
	type_del(member->typ);
}

void struct_del(struct_t *st) {
	if (st->tag) {
		string_del(st->tag);
		st->tag = NULL;
	}
	if (st->is_defined) {
		for (size_t i = 0; i < st->nmembers; ++i) {
			st_member_del(&st->members[i]);
		}
		free(st->members);
		st->is_defined = 0;
	}
	if (!st->nrefs) free(st);
}
void struct_map_del(khash_t(struct_map) *map) {
	// The keys are the tag in the struct_t
	struct_t **it;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	kh_foreach_value_ref(map, it, struct_del(*it))
#pragma GCC diagnostic pop
	kh_destroy(struct_map, map);
}

type_t *type_new(void) {
	type_t *ret = malloc(sizeof *ret);
	if (!ret) {
		log_memory("failed to create a new type\n");
		return NULL;
	}
	ret->szinfo.align = ret->szinfo.size = 0;
	ret->is_atomic = ret->is_const = ret->is_restrict = ret->is_volatile = ret->is_incomplete = ret->is_validated = ret->_internal_use = 0;
	ret->converted = NULL;
	ret->nrefs = 1;
	ret->typ = TYPE_BUILTIN;
	ret->val.builtin = BTT_INT;
	return ret;
}
type_t *type_new_ptr(type_t *target) {
	type_t *ret = malloc(sizeof *ret);
	if (!ret) {
		log_memory("failed to create a new pointer type\n");
		return NULL;
	}
	ret->szinfo.align = ret->szinfo.size = 0;
	ret->is_atomic = ret->is_const = ret->is_restrict = ret->is_volatile = ret->is_incomplete = ret->is_validated = ret->_internal_use = 0;
	ret->converted = NULL;
	ret->nrefs = 1;
	ret->typ = TYPE_PTR;
	ret->val.typ = target;
	return ret;
}
int type_copy_into(type_t *dest, const type_t *ref) {
	size_t nrefs = dest->nrefs;
	_Bool is_atomic = dest->is_atomic;
	_Bool is_const = dest->is_const;
	_Bool is_restrict = dest->is_restrict;
	_Bool is_volatile = dest->is_volatile;
	memcpy(dest, ref, sizeof *dest);
	switch (ref->typ) {
	case TYPE_BUILTIN:
		break;
	case TYPE_ARRAY:
		++ref->val.array.typ->nrefs;
		break;
	case TYPE_STRUCT_UNION:
		++dest->val.st->nrefs;
		break;
	case TYPE_ENUM:
	case TYPE_PTR:
		++ref->val.typ->nrefs;
		break;
	case TYPE_FUNCTION:
		++ref->val.fun.ret->nrefs;
		if (ref->val.fun.nargs != (size_t)-1) {
			if (!ref->val.fun.args) {
				break;
			}
			type_t **args = malloc(sizeof *args * ref->val.fun.nargs);
			if (!args) {
				log_memory("failed to allocate new argument array\n");
				return 0;
			}
			for (size_t i = 0; i < ref->val.fun.nargs; ++i) {
				++ref->val.fun.args[i]->nrefs;
				args[i] = ref->val.fun.args[i];
			}
			dest->val.fun.args = args;
		}
		break;
	}
	dest->is_atomic |= is_atomic;
	dest->is_const |= is_const;
	dest->is_restrict |= is_restrict;
	dest->is_volatile |= is_volatile;
	dest->nrefs = nrefs;
	return 1;
}

struct_t *struct_new(int is_struct, string_t *tag) {
	struct_t *ret = malloc(sizeof *ret);
	if (!ret) {
		log_memory("Error: failed to create a new struct\n");
		return NULL;
	}
	ret->is_struct = is_struct;
	ret->tag = tag;
	ret->is_defined = 0;
	ret->is_simple = 0;
	ret->nrefs = 1;
	ret->has_self_recursion = 0; // Undefined structures cannot have self recursion
	return ret;
}

khint_t type_t_hash(type_t *typ) {
	switch (typ->typ) {
	case TYPE_BUILTIN: return kh_int_hash_func(typ->val.builtin);
	case TYPE_ARRAY: return kh_int_hash_func((typ->val.array.array_sz << 12) + type_t_hash(typ->val.array.typ));
	case TYPE_STRUCT_UNION:
		if (typ->val.st->tag) {
			return kh_str_hash_func(string_content(typ->val.st->tag));
		} else {
			khint64_t acc = (typ->val.st->nmembers << 5) + 1;
			for (size_t i = 0; i < typ->val.st->nmembers; ++i) {
				acc += (khint64_t)type_t_hash(typ->val.st->members[i].typ) << ((1ull << (i % 64)) % 61);
			}
			return kh_int64_hash_func(acc);
		}
	case TYPE_ENUM:
	case TYPE_PTR:
		return kh_int_hash_func(type_t_hash(typ->val.typ));
	case TYPE_FUNCTION: {
		khint64_t acc = type_t_hash(typ->val.fun.ret) + typ->val.fun.nargs * 73 + (size_t)typ->val.fun.has_varargs * 77;
		if (typ->val.fun.nargs != (size_t)-1) {
			for (size_t i = 0; i < typ->val.fun.nargs; ++i) {
				acc += (khint64_t)type_t_hash(typ->val.fun.args[i]) << ((3ull << (i % 64)) % 53);
			}
		}
		return kh_int64_hash_func(acc); }
	default: return 0;
	}
}
int type_t_equal_aux(type_t *typ1, type_t *typ2, int is_strict) {
	if (is_strict && (
	    (typ1->is_atomic != typ2->is_atomic) || (typ1->is_const != typ2->is_const)
	 || (typ1->is_restrict != typ2->is_restrict) || (typ1->is_volatile != typ2->is_volatile))) {
		return 0;
	}
	if (typ1->typ != typ2->typ) return 0;
	switch (typ1->typ) {
	case TYPE_BUILTIN: return typ1->val.builtin == typ2->val.builtin;
	case TYPE_ARRAY: return (typ1->val.array.array_sz == typ2->val.array.array_sz) && type_t_equal_aux(typ1->val.array.typ, typ2->val.array.typ, is_strict);
	case TYPE_STRUCT_UNION:
		// This will not do an infinite recursion since only unnamed struct/unions will compare their members
		if (!typ1->val.st->tag != !typ2->val.st->tag) return 0;
		if (typ1->val.st->tag) return !strcmp(string_content(typ1->val.st->tag), string_content(typ2->val.st->tag));
		if (!typ1->val.st->is_defined || !typ2->val.st->is_defined) {
			log_warning_nopos("comparing against incomplete anonymous structure/union\n");
			return 0;
		}
		if (typ1->val.st->nmembers != typ2->val.st->nmembers) return 0;
		for (size_t i = 0; i < typ1->val.st->nmembers; ++i) {
			if (!!typ1->val.st->members[i].name != !!typ2->val.st->members[i].name) return 0;
			if (typ1->val.st->members[i].name
			     && strcmp(string_content(typ1->val.st->members[i].name), string_content(typ2->val.st->members[i].name)))
				return 0;
			if (!type_t_equal_aux(typ1->val.st->members[i].typ, typ2->val.st->members[i].typ, is_strict)) return 0;
		}
		return 1;
	case TYPE_ENUM:
	case TYPE_PTR:
		return type_t_equal_aux(typ1->val.typ, typ2->val.typ, is_strict);
	case TYPE_FUNCTION:
		if (typ1->val.fun.nargs != typ2->val.fun.nargs) return 0;
		if (typ1->val.fun.has_varargs != typ2->val.fun.has_varargs) return 0;
		if (!type_t_equal_aux(typ1->val.fun.ret, typ2->val.fun.ret, is_strict)) return 0;
		if (typ1->val.fun.nargs != (size_t)-1) {
			for (size_t i = 0; i < typ1->val.fun.nargs; ++i) {
				if (!type_t_equal_aux(typ1->val.fun.args[i], typ2->val.fun.args[i], is_strict)) return 0;
			}
		}
		return 1;
	default: return 0;
	}
}
int type_t_equal_lax(type_t *typ1, type_t *typ2) {
	return type_t_equal_aux(typ1, typ2, 0);
}
int type_t_equal(type_t *typ1, type_t *typ2) {
	return type_t_equal_aux(typ1, typ2, 1);
}
__KHASH_IMPL(type_set, , type_t*, char, 0, type_t_hash, type_t_equal)
__KHASH_IMPL(conv_map, , type_t*, string_t*, 1, type_t_hash, type_t_equal_lax)

type_t *type_try_merge(type_t *typ, khash_t(type_set) *set) {
	int iret;
	khiter_t it = kh_put(type_set, set, typ, &iret);
	if (iret < 0) {
		log_memory("Error: failed to add type to type_set\n");
		return NULL;
	} else if (iret == 0) {
		if (typ == kh_key(set, it)) return typ;
		kh_key(set, it)->is_incomplete &= typ->is_incomplete; // In case we have a recursive structure
		type_del(typ);
		++kh_key(set, it)->nrefs;
		return kh_key(set, it);
	} else {
		++typ->nrefs;
	}
	type_t *typ2;
	switch (typ->typ) {
	case TYPE_BUILTIN:
		return typ;
	case TYPE_ARRAY:
		typ2 = type_try_merge(typ->val.array.typ, set);
		if (typ2) typ->val.array.typ = typ2;
		else return NULL;
		return typ;
	case TYPE_STRUCT_UNION:
		if (typ->val.st->is_defined) {
			for (size_t i = 0; i < typ->val.st->nmembers; ++i) {
				typ2 = type_try_merge(typ->val.st->members[i].typ, set);
				if (typ2) typ->val.st->members[i].typ = typ2;
				else return NULL;
			}
		}
		return typ;
	case TYPE_ENUM:
	case TYPE_PTR:
		typ2 = type_try_merge(typ->val.typ, set);
		if (typ2) typ->val.typ = typ2;
		else return NULL;
		return typ;
	case TYPE_FUNCTION:
		typ2 = type_try_merge(typ->val.fun.ret, set);
		if (typ2) typ->val.fun.ret = typ2;
		else return NULL;
		
		if (typ->val.fun.nargs != (size_t)-1) {
			for (size_t i = 0; i < typ->val.fun.nargs; ++i) {
				typ2 = type_try_merge(typ->val.fun.args[i], set);
				if (typ2) typ->val.fun.args[i] = typ2;
				else return NULL;
			}
		}
		return typ;
		
	default: return NULL;
	}
}

const char *builtin2str[LAST_BUILTIN + 1] = {
	[BTT_VOID] = "void",
	[BTT_BOOL] = "_Bool",
	[BTT_CHAR] = "char",
	[BTT_SCHAR] = "signed char",
	[BTT_UCHAR] = "unsigned char",
	[BTT_SHORT] = "short",
	[BTT_SSHORT] = "signed short",
	[BTT_USHORT] = "unsigned short",
	[BTT_INT] = "int",
	[BTT_SINT] = "signed",
	[BTT_UINT] = "unsigned",
	[BTT_LONG] = "long",
	[BTT_SLONG] = "signed long",
	[BTT_ULONG] = "unsigned long",
	[BTT_LONGLONG] = "long long",
	[BTT_SLONGLONG] = "signed long long",
	[BTT_ULONGLONG] = "unsigned long long",
	[BTT_INT128] = "__int128",
	[BTT_SINT128] = "signed __int128",
	[BTT_UINT128] = "unsigned __int128",
	[BTT_S8] = "int8_t",
	[BTT_U8] = "uint8_t",
	[BTT_S16] = "int16_t",
	[BTT_U16] = "uint16_t",
	[BTT_S32] = "int32_t",
	[BTT_U32] = "uint32_t",
	[BTT_S64] = "int64_t",
	[BTT_U64] = "uint64_t",
	[BTT_FLOAT] = "float",
	[BTT_CFLOAT] = "float _Complex",
	[BTT_IFLOAT] = "float _Imaginary",
	[BTT_DOUBLE] = "double",
	[BTT_CDOUBLE] = "double _Complex",
	[BTT_IDOUBLE] = "double _Imaginary",
	[BTT_LONGDOUBLE] = "long double",
	[BTT_CLONGDOUBLE] = "long double _Complex",
	[BTT_ILONGDOUBLE] = "long double _Imaginary",
	[BTT_FLOAT128] = "__float128",
	[BTT_CFLOAT128] = "__float128 _Complex",
	[BTT_IFLOAT128] = "__float128 _Imaginary",
	[BTT_VA_LIST] = "__builtin_va_list",
};
void type_print(type_t *typ) {
	if (typ->_internal_use) {
		printf("...%p", typ);
		return;
	}
	typ->_internal_use = 1;
	printf("<" DISP_ADDR_FMT "n_uses=%zu> ", DISP_ADDR_ARG(typ) typ->nrefs);
	if (!typ->is_validated) printf("!<not validated> ");
	if (typ->is_incomplete) printf("<incomplete> ");
	if (typ->is_validated && !typ->is_incomplete) printf("<size=%zu align=%zu> ", typ->szinfo.size, typ->szinfo.align);
	if (typ->converted) printf("<converted: %s> ", string_content(typ->converted));
	if (typ->is_const) printf("const ");
	if (typ->is_restrict) printf("restrict ");
	if (typ->is_volatile) printf("volatile ");
	if (typ->is_atomic) printf("_Atomic ");
	switch (typ->typ) {
	case TYPE_BUILTIN:
		printf("<builtin %s (%u)>", builtin2str[typ->val.builtin], typ->val.builtin);
		break;
	case TYPE_ARRAY:
		if (typ->val.array.array_sz == (size_t)-1)
			printf("[] ");
		else printf("[%zu] ", typ->val.array.array_sz);
		type_print(typ->val.array.typ);
		break;
	case TYPE_STRUCT_UNION:
		struct_print(typ->val.st);
		break;
	case TYPE_ENUM:
		printf("<enum> ");
		type_print(typ->val.typ);
		break;
	case TYPE_PTR:
		printf("*");
		type_print(typ->val.typ);
		break;
	case TYPE_FUNCTION:
		printf("<function type, ret=");
		type_print(typ->val.fun.ret);
		if (typ->val.fun.nargs == (size_t)-1) {
			printf(", no argument spec>");
		} else {
			printf(", args=(");
			for (size_t i = 0; i < typ->val.fun.nargs; ++i) {
				if (i) printf("), (");
				type_print(typ->val.fun.args[i]);
			}
			if (typ->val.fun.has_varargs && typ->val.fun.nargs) printf(", ...)>");
			else if (typ->val.fun.has_varargs && !typ->val.fun.nargs) printf("...)>");
			else printf(")>");
		}
		break;
	}
	typ->_internal_use = 0;
}
void struct_print(const struct_t *st) {
	printf("<" DISP_ADDR_FMT "n_uses=%zu> ", DISP_ADDR_ARG(st) st->nrefs);
	if (st->is_simple) {
		printf("<simple> %s %s <don't care>", st->is_struct ? "struct" : "union", st->tag ? string_content(st->tag) : "<no tag>");
		return;
	}
	if (st->is_defined) {
		printf(
			"%s %s <with %zu members%s%s> { ",
			st->is_struct ? "struct" : "union",
			st->tag ? string_content(st->tag) : "<no tag>",
			st->nmembers,
			st->has_incomplete ? ", with incomplete" : "",
			st->has_self_recursion ? ", with self-recursion" : "");
		for (size_t i = 0; i < st->nmembers; ++i) {
			if (i) printf(", ");
			type_print(st->members[i].typ);
			printf(" %s", st->members[i].name ? string_content(st->members[i].name) : "<no name>");
			if (st->members[i].is_bitfield) {
				printf(" : %zu", st->members[i].bitfield_width);
			}
		}
		printf(" }");
	} else {
		printf("%s %s <undefined>", st->is_struct ? "struct" : "union", st->tag ? string_content(st->tag) : "<no tag>");
	}
}

file_t *file_new(machine_t *target) {
	file_t *ret = malloc(sizeof *ret);
	if (!ret) {
		log_memory("failed to create a new translation unit structure (init)\n");
		return NULL;
	}
	if (!(ret->struct_map = kh_init(struct_map))) {
		log_memory("failed to create a new translation unit structure (structure map)\n");
		free(ret);
		return NULL;
	}
	if (!(ret->type_map = kh_init(type_map))) {
		log_memory("failed to create a new translation unit structure (type map)\n");
		kh_destroy(struct_map, ret->struct_map);
		free(ret);
		return NULL;
	}
	if (!(ret->enum_map = kh_init(type_map))) {
		log_memory("failed to create a new translation unit structure (enumeration map)\n");
		kh_destroy(struct_map, ret->struct_map);
		kh_destroy(type_map, ret->type_map);
		free(ret);
		return NULL;
	}
	if (!(ret->decl_map = kh_init(decl_map))) {
		log_memory("failed to create a new translation unit structure (declaration map)\n");
		kh_destroy(struct_map, ret->struct_map);
		kh_destroy(type_map, ret->type_map);
		kh_destroy(type_map, ret->enum_map);
		free(ret);
		return NULL;
	}
	if (!(ret->type_set = kh_init(type_set))) {
		log_memory("failed to create a new translation unit structure (type set)\n");
		kh_destroy(struct_map, ret->struct_map);
		kh_destroy(type_map, ret->type_map);
		kh_destroy(type_map, ret->enum_map);
		kh_destroy(decl_map, ret->decl_map);
		free(ret);
		return NULL;
	}
	if (!(ret->const_map = kh_init(const_map))) {
		log_memory("failed to create a new translation unit structure (const map)\n");
		kh_destroy(struct_map, ret->struct_map);
		kh_destroy(type_map, ret->type_map);
		kh_destroy(type_map, ret->enum_map);
		kh_destroy(decl_map, ret->decl_map);
		kh_destroy(type_set, ret->type_set);
		free(ret);
		return NULL;
	}
	if (!(ret->relaxed_type_conversion = kh_init(conv_map))) {
		log_memory("failed to create a new translation unit structure (relaxed type conversion map)\n");
		kh_destroy(struct_map, ret->struct_map);
		kh_destroy(type_map, ret->type_map);
		kh_destroy(type_map, ret->enum_map);
		kh_destroy(decl_map, ret->decl_map);
		kh_destroy(type_set, ret->type_set);
		kh_destroy(const_map, ret->const_map);
		free(ret);
		return NULL;
	}
	
	// Now fill in the builtin types
	int iret; khiter_t it;
	for (enum type_builtin_e i = 0; i < LAST_BUILTIN + 1; ++i) {
		type_t *t = type_new();
		if (!t) {
			log_memory("failed to create a new translation unit structure (builtin type)\n");
			for (; i--;) {
				free(ret->builtins[i]);
			}
			kh_destroy(struct_map, ret->struct_map);
			kh_destroy(type_map, ret->type_map);
			kh_destroy(conv_map, ret->relaxed_type_conversion);
			kh_destroy(type_map, ret->enum_map);
			kh_destroy(decl_map, ret->decl_map);
			kh_destroy(type_set, ret->type_set);
			kh_destroy(const_map, ret->const_map);
			free(ret);
			return NULL;
		}
		t->is_incomplete = (i == BTT_VOID);
		t->nrefs = 2;
		t->typ = TYPE_BUILTIN;
		t->val.builtin = i;
		if (!target->has_int128 && ((i == BTT_INT128) || (i == BTT_SINT128) || (i == BTT_UINT128))) {
			t->converted = string_new_cstr("<invalid int128>");
		}
		validate_type(&(loginfo_t){0}, target, t);
		ret->builtins[i] = t;
		kh_put(type_set, ret->type_set, t, &iret);
		if (iret < 0) {
			log_memory("failed to create a new translation unit structure (failed to add intrinsic type to type_set)\n");
			kh_destroy(struct_map, ret->struct_map);
			kh_destroy(type_map, ret->type_map);
			kh_destroy(conv_map, ret->relaxed_type_conversion);
			kh_destroy(type_map, ret->enum_map);
			kh_destroy(decl_map, ret->decl_map);
			kh_destroy(type_set, ret->type_set);
			kh_destroy(const_map, ret->const_map);
			free(ret);
			return NULL;
		} else if (iret == 0) {
			log_memory("failed to create a new translation unit structure (duplicate intrinsic type in type_set)\n");
			for (++i; i--;) {
				free(ret->builtins[i]);
			}
			kh_destroy(struct_map, ret->struct_map);
			kh_destroy(type_map, ret->type_map);
			kh_destroy(conv_map, ret->relaxed_type_conversion);
			kh_destroy(type_map, ret->enum_map);
			kh_destroy(decl_map, ret->decl_map);
			kh_destroy(type_set, ret->type_set);
			kh_destroy(const_map, ret->const_map);
			free(ret);
			return NULL;
		}
	}
	// ret is valid and can now be deleted by file_del
	
	// Add __builtin_va_list, __int128_t, __uint128_t as builtin typedef
	char *sdup;
#define ADD_TYPEDEF(name, btt) \
		sdup = strdup(#name);                                                                                   \
		if (!sdup) {                                                                                            \
			log_memory("failed to create a new translation unit structure (" #name " name)\n");                 \
			file_del(ret);                                                                                      \
			return NULL;                                                                                        \
		}                                                                                                       \
		it = kh_put(type_map, ret->type_map, sdup, &iret);                                                      \
		if (iret < 0) {                                                                                         \
			log_memory("failed to create a new translation unit structure (add " #name " typedef)\n");          \
			free(sdup);                                                                                         \
			file_del(ret);                                                                                      \
			return NULL;                                                                                        \
		} else if (iret == 0) {                                                                                 \
			log_memory("failed to create a new translation unit structure (" #name " is already a typedef)\n"); \
			free(sdup);                                                                                         \
			file_del(ret);                                                                                      \
			return NULL;                                                                                        \
		}                                                                                                       \
		kh_val(ret->type_map, it) = ret->builtins[BTT_ ## btt];                                                 \
		++ret->builtins[BTT_ ## btt]->nrefs;
	ADD_TYPEDEF(__builtin_va_list, VA_LIST)
	if (target->has_int128) {
		ADD_TYPEDEF(__int128_t, INT128)
		ADD_TYPEDEF(__uint128_t, UINT128)
	}
	
	return ret;
}
void file_del(file_t *f) {
	struct_map_del(f->struct_map);
	type_map_del(f->type_map);
	conv_map_del(f->relaxed_type_conversion);
	type_map_del(f->enum_map);
	decl_map_del(f->decl_map);
	type_set_del(f->type_set);
	const_map_del(f->const_map);
	for (enum type_builtin_e i = 0; i < LAST_BUILTIN + 1; ++i) {
		type_del(f->builtins[i]);
	}
	free(f);
}
