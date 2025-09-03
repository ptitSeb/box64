#include "parse.h"

#include <stdint.h>
#include <string.h>

#include "cstring.h"
#include "khash.h"
#include "machine.h"
#include "prepare.h"
#include "preproc.h"

void dump_prepare(const char *filename, FILE *file) {
	prepare_t *prep = prepare_new_file(file, filename);
	if (!prep) {
		log_memory("failed to create the prepare structure\n");
		return;
	}
	while (1) {
		preproc_token_t tok = pre_next_token(prep, 0);
		preproc_token_print(&tok);
		if (preproc_token_isend(&tok)) {
			preproc_token_del(&tok);
			prepare_del(prep);
			return;
		}
		preproc_token_del(&tok);
	}
}
void dump_preproc(machine_t *target, const char *filename, FILE *file) {
	char *dirname = strchr(filename, '/') ? strndup(filename, (size_t)(strrchr(filename, '/') - filename)) : NULL;
	preproc_t *prep = preproc_new_file(target, file, dirname, filename);
	if (!prep) {
		log_memory("failed to create the preproc structure\n");
		if (dirname) free(dirname);
		return;
	}
	while (1) {
		proc_token_t tok = proc_next_token(prep);
		proc_token_print(&tok);
		if (proc_token_isend(&tok)) {
			proc_token_del(&tok);
			preproc_del(prep);
			return;
		}
		proc_token_del(&tok);
	}
}

enum decl_storage {
	TMPSTO_NONE,
	TMPSTO_TYPEDEF,
	TMPSTO_EXTERN,
	TMPSTO_STATIC,
	TMPSTO_TLS,
	TMPSTO_TLS_EXTERN,
	TMPSTO_TLS_STATIC,
	TMPSTO_AUTO,
	TMPSTO_REG,
};
const enum decl_storage_e tmpsto2sto[] = {
	[TMPSTO_NONE] = STORAGE_NONE,
	[TMPSTO_TYPEDEF] = STORAGE_NONE,
	[TMPSTO_EXTERN] = STORAGE_EXTERN,
	[TMPSTO_STATIC] = STORAGE_STATIC,
	[TMPSTO_TLS] = STORAGE_TLS,
	[TMPSTO_TLS_EXTERN] = STORAGE_TLS_EXTERN,
	[TMPSTO_TLS_STATIC] = STORAGE_TLS_STATIC,
	[TMPSTO_AUTO] = STORAGE_AUTO,
	[TMPSTO_REG] = STORAGE_REG,
};
enum decl_spec {
	SPEC_NONE,
	SPEC_BUILTIN,
	SPEC_COMPLEX,
	SPEC_IMAGINARY,
	SPEC_LONGCOMPLEX,
	SPEC_LONGIMAGINARY,
	SPEC_BUILTIN_NOINT,
	SPEC_TYPE,
};
enum fun_spec {
	FSPEC_NONE            = 0b00,
	FSPEC_INLINE          = 0b01,
	FSPEC_NORETURN        = 0b10,
	FSPEC_INLINE_NORETURN = 0b11,
};
VECTOR_DECLARE_STATIC(size_t, size_t)
VECTOR_IMPL_STATIC(size_t, (void))

#define VALIDATION_DECL 1
#define VALIDATION_LAST_DECL 2
#define VALIDATION_FUN 3
static int validate_storage_type(loginfo_t *loginfo, machine_t *target, enum decl_storage storage,
                                 type_t *typ, enum token_sym_type_e sym) {
	// We may still do adjustments here
	if (!validate_type(loginfo, target, typ)) return 0;
	if (typ->typ == TYPE_FUNCTION) {
		if ((storage == TMPSTO_TLS) || (storage == TMPSTO_TLS_EXTERN) || (storage == TMPSTO_TLS_STATIC)) {
			log_error(loginfo, "functions cannot be thread local\n");
			return 0;
		}
		if ((sym == SYM_COMMA) || (sym == SYM_RPAREN) || (sym == SYM_SEMICOLON)) {
			return (sym == SYM_SEMICOLON) ? VALIDATION_LAST_DECL : VALIDATION_DECL;
		} else if (sym == SYM_LBRACKET) {
			return VALIDATION_FUN;
		} else {
			log_error(loginfo, "unexpected symbol %s (%u) after function declaration\n", sym2str[sym], sym);
			return 0;
		}
	} else {
		if ((sym == SYM_COMMA) || (sym == SYM_RPAREN) || (sym == SYM_SEMICOLON) || ((storage != TMPSTO_TYPEDEF) && (sym == SYM_EQ))) {
			return (sym == SYM_SEMICOLON) ? VALIDATION_LAST_DECL : VALIDATION_DECL;
		} else {
			log_error(loginfo, "unexpected symbol %s (%u) after %s declaration\n", sym2str[sym], sym, (storage == TMPSTO_TYPEDEF) ? "type" : "variable");
			return 0;
		}
	}
}

static void promote_csts(num_constant_t *v1, num_constant_t *v2) {
	switch (v1->typ) {
	case NCT_FLOAT:
		switch (v2->typ) {
		case NCT_FLOAT: break;
		case NCT_DOUBLE: v1->typ = NCT_DOUBLE; v1->val.d = v1->val.f; break;
		case NCT_LDOUBLE: v1->typ = NCT_LDOUBLE; v1->val.l = v1->val.f; break;
		case NCT_INT32: v2->typ = NCT_FLOAT; v2->val.f = (float)v2->val.i32; break;
		case NCT_UINT32: v2->typ = NCT_FLOAT; v2->val.f = (float)v2->val.u32; break;
		case NCT_INT64: v2->typ = NCT_FLOAT; v2->val.f = (float)v2->val.i64; break;
		case NCT_UINT64: v2->typ = NCT_FLOAT; v2->val.f = (float)v2->val.u64; break;
		}
		break;
	case NCT_DOUBLE:
		switch (v2->typ) {
		case NCT_FLOAT: v2->typ = NCT_DOUBLE; v2->val.d = v2->val.f; break;
		case NCT_DOUBLE: break;
		case NCT_LDOUBLE: v1->typ = NCT_LDOUBLE; v1->val.l = v1->val.d; break;
		case NCT_INT32: v2->typ = NCT_DOUBLE; v2->val.d = (double)v2->val.i32; break;
		case NCT_UINT32: v2->typ = NCT_DOUBLE; v2->val.d = (double)v2->val.u32; break;
		case NCT_INT64: v2->typ = NCT_DOUBLE; v2->val.d = (double)v2->val.i64; break;
		case NCT_UINT64: v2->typ = NCT_DOUBLE; v2->val.d = (double)v2->val.u64; break;
		}
		break;
	case NCT_LDOUBLE:
		switch (v2->typ) {
		case NCT_FLOAT: v2->typ = NCT_LDOUBLE; v2->val.l = v2->val.f; break;
		case NCT_DOUBLE: v2->typ = NCT_LDOUBLE; v2->val.l = v2->val.d; break;
		case NCT_LDOUBLE: break;
		case NCT_INT32: v2->typ = NCT_LDOUBLE; v2->val.l = (long double)v2->val.i32; break;
		case NCT_UINT32: v2->typ = NCT_LDOUBLE; v2->val.l = (long double)v2->val.u32; break;
		case NCT_INT64: v2->typ = NCT_LDOUBLE; v2->val.l = (long double)v2->val.i64; break;
		case NCT_UINT64: v2->typ = NCT_LDOUBLE; v2->val.l = (long double)v2->val.u64; break;
		}
		break;
	case NCT_INT32:
		switch (v2->typ) {
		case NCT_FLOAT: v1->typ = NCT_FLOAT; v1->val.f = (float)v1->val.i32; break;
		case NCT_DOUBLE: v1->typ = NCT_DOUBLE; v1->val.d = (double)v1->val.i32; break;
		case NCT_LDOUBLE: v1->typ = NCT_LDOUBLE; v1->val.l = (long double)v1->val.i32; break;
		case NCT_INT32: break;
		case NCT_UINT32: v1->typ = NCT_UINT32; v1->val.u32 = (uint32_t)v1->val.i32; break;
		case NCT_INT64: v1->typ = NCT_INT64; v1->val.i64 = v1->val.i32; break;
		case NCT_UINT64: v1->typ = NCT_UINT64; v1->val.u64 = (uint64_t)v1->val.i32; break;
		}
		break;
	case NCT_UINT32:
		switch (v2->typ) {
		case NCT_FLOAT: v1->typ = NCT_FLOAT; v1->val.f = (float)v1->val.u32; break;
		case NCT_DOUBLE: v1->typ = NCT_DOUBLE; v1->val.d = (double)v1->val.u32; break;
		case NCT_LDOUBLE: v1->typ = NCT_LDOUBLE; v1->val.l = (long double)v1->val.u32; break;
		case NCT_INT32: v2->typ = NCT_UINT32; v2->val.u32 = (uint32_t)v2->val.i32; break;
		case NCT_UINT32: break;
		case NCT_INT64: v1->typ = NCT_INT64; v1->val.i64 = v1->val.u32; break;
		case NCT_UINT64: v1->typ = NCT_UINT64; v1->val.u64 = v1->val.u32; break;
		}
		break;
	case NCT_INT64:
		switch (v2->typ) {
		case NCT_FLOAT: v1->typ = NCT_FLOAT; v1->val.f = (float)v1->val.i64; break;
		case NCT_DOUBLE: v1->typ = NCT_DOUBLE; v1->val.d = (double)v1->val.i64; break;
		case NCT_LDOUBLE: v1->typ = NCT_LDOUBLE; v1->val.l = (long double)v1->val.i64; break;
		case NCT_INT32: v2->typ = NCT_INT64; v2->val.i64 = v2->val.i32; break;
		case NCT_UINT32: v2->typ = NCT_INT64; v2->val.i64 = v2->val.u32; break;
		case NCT_INT64: break;
		case NCT_UINT64: v1->typ = NCT_UINT64; v1->val.u64 = (uint64_t)v1->val.i64; break;
		}
		break;
	case NCT_UINT64:
		switch (v2->typ) {
		case NCT_FLOAT: v1->typ = NCT_FLOAT; v1->val.f = (float)v1->val.u64; break;
		case NCT_DOUBLE: v1->typ = NCT_DOUBLE; v1->val.d = (double)v1->val.u64; break;
		case NCT_LDOUBLE: v1->typ = NCT_LDOUBLE; v1->val.l = (long double)v1->val.u64; break;
		case NCT_INT32: v2->typ = NCT_UINT64; v2->val.u64 = (uint64_t)v2->val.i32; break;
		case NCT_UINT32: v2->typ = NCT_UINT64; v2->val.u64 = v2->val.u32; break;
		case NCT_INT64: v2->typ = NCT_UINT64; v2->val.u64 = (uint64_t)v2->val.i64; break;
		case NCT_UINT64: break;
		}
		break;
	}
}

#define TOKEN_MATCH_ATTR(attr) (!strcmp(string_content(tok->tokv.str), #attr) || !strcmp(string_content(tok->tokv.str), "__" #attr "__"))

VECTOR_DECLARE_STATIC(exprs, expr_t*)
#define expr_del_ptr(p) expr_del(*(p))
VECTOR_IMPL_STATIC(exprs, expr_del_ptr)
#undef expr_del_ptr
struct expr_partial_op {
	enum expr_partial_op_e {
		EPO_LPAREN,
		EPO_SIZEOF,
		EPO_CAST,
		EPO_FUNCALL,
		EPO_UNARY,
		EPO_BINARY_ARG,
		EPO_BINARY_ARG_SYM,
		EPO_TERNARY_ARG1,
		EPO_TERNARY_ARG2,
	} typ;
	int once_done_want_level;
	int once_done_is_level;
	union {
		char c; // Empty destructor
		type_t *typ;
		struct {
			expr_t *f;
			VECTOR(exprs) *exprs;
		} funcall;
		enum unary_op_e unop;
		struct {
			enum binary_op_e op;
			expr_t *e1;
			enum token_sym_type_e last_sym;
		} binop;
		struct {
			enum ternary_op_e op;
			expr_t *e1;
			expr_t *e2;
			int once_done2_want_level;
			enum token_sym_type_e arg23_sep;
		} ternop;
	} val;
};
void expr_partial_op_del(struct expr_partial_op *epo) {
	switch (epo->typ) {
	case EPO_LPAREN:
	case EPO_SIZEOF:
		return;
	case EPO_CAST:
		type_del(epo->val.typ);
		return;
	case EPO_FUNCALL:
		expr_del(epo->val.funcall.f);
		vector_del(exprs, epo->val.funcall.exprs);
		return;
	case EPO_UNARY:
		return;
	case EPO_BINARY_ARG:
	case EPO_BINARY_ARG_SYM:
		expr_del(epo->val.binop.e1);
		return;
	case EPO_TERNARY_ARG1:
		expr_del(epo->val.ternop.e1);
		return;
	case EPO_TERNARY_ARG2:
		expr_del(epo->val.ternop.e1);
		expr_del(epo->val.ternop.e2);
		return;
	}
}
VECTOR_DECLARE_STATIC(expr_pops, struct expr_partial_op)
VECTOR_IMPL_STATIC(expr_pops, expr_partial_op_del)

VECTOR_DECLARE_STATIC(types, type_t*)
#define type_ptr_del(t) type_del(*(t))
VECTOR_IMPL_STATIC(types, type_ptr_del)
#undef type_ptr_del
VECTOR_DECLARE_STATIC(st_members, st_member_t)
VECTOR_IMPL_STATIC(st_members, st_member_del)

struct parse_declarator_dest_s {
	file_t *f; // is_init, is_list
	struct {
		khash_t(struct_map) *struct_map;
		khash_t(type_map) *type_map;
		khash_t(type_map) *enum_map;
		khash_t(type_set) *type_set;
		type_t *(*builtins)[LAST_BUILTIN + 1];
		khash_t(const_map) *const_map;
		type_t *dest;
	} argt; // !is_init, !is_list
	struct {
		khash_t(struct_map) *struct_map;
		khash_t(type_map) *type_map;
		khash_t(type_map) *enum_map;
		khash_t(type_set) *type_set;
		type_t *(*builtins)[LAST_BUILTIN + 1];
		khash_t(const_map) *const_map;
		VECTOR(st_members) *dest;
	} structms; // !is_init, is_list
};
#define PDECL_STRUCT_MAP ((is_init && is_list) ? dest->f->struct_map : (!is_init && is_list) ? dest->structms.struct_map : dest->argt.struct_map)
#define PDECL_TYPE_MAP ((is_init && is_list) ? dest->f->type_map : (!is_init && is_list) ? dest->structms.type_map : dest->argt.type_map)
#define PDECL_ENUM_MAP ((is_init && is_list) ? dest->f->enum_map : (!is_init && is_list) ? dest->structms.enum_map : dest->argt.enum_map)
#define PDECL_TYPE_SET ((is_init && is_list) ? dest->f->type_set : (!is_init && is_list) ? dest->structms.type_set : dest->argt.type_set)
#define PDECL_BUILTINS ((is_init && is_list) ? &dest->f->builtins : (!is_init && is_list) ? dest->structms.builtins : dest->argt.builtins)
#define PDECL_CONST_MAP ((is_init && is_list) ? dest->f->const_map : (!is_init && is_list) ? dest->structms.const_map : dest->argt.const_map)
static int parse_declarator(machine_t *target, struct parse_declarator_dest_s *dest, preproc_t *prep, proc_token_t *tok, enum decl_storage storage,
      enum fun_spec fspec, type_t *base_type, int is_init, int is_list, int allow_decl, int allow_abstract);

// declaration-specifier with storage != NULL
// specifier-qualifier-list + static_assert-declaration with storage == NULL
static int parse_declaration_specifier(machine_t *target, khash_t(struct_map) *struct_map, khash_t(type_map) *type_map, khash_t(type_map) *enum_map,
        type_t *(*builtins)[LAST_BUILTIN + 1], khash_t(const_map) *const_map,
        khash_t(type_set) *type_set, preproc_t *prep, proc_token_t *tok, enum decl_storage *storage, enum fun_spec *fspec, enum decl_spec *spec, type_t *typ);

static int is_type_spec_qual_kw(enum token_keyword_type_e kw) {
	return
		(kw == KW_ATOMIC)    ||
		(kw == KW_BOOL)      ||
		(kw == KW_CHAR)      ||
		(kw == KW_COMPLEX)   ||
		(kw == KW_CONST)     ||
		(kw == KW_DOUBLE)    ||
		(kw == KW_ENUM)      ||
		(kw == KW_FLOAT)     ||
		(kw == KW_FLOAT128)  ||
		(kw == KW_IMAGINARY) ||
		(kw == KW_INT)       ||
		(kw == KW_INT128)    ||
		(kw == KW_LONG)      ||
		(kw == KW_RESTRICT)  ||
		(kw == KW_SHORT)     ||
		(kw == KW_SIGNED)    ||
		(kw == KW_STRUCT)    ||
		(kw == KW_UNION)     ||
		(kw == KW_UNSIGNED)  ||
		(kw == KW_VOID)      ||
		(kw == KW_VOLATILE)  ||
		0;
}
#define IS_BEGIN_TYPE_NAME \
	(((tok->tokt == PTOK_KEYWORD) && is_type_spec_qual_kw(tok->tokv.kw)) || \
	 ((tok->tokt == PTOK_IDENT) && ((it = kh_get(type_map, type_map, string_content(tok->tokv.str))) != kh_end(type_map))))
static int parse_type_name(machine_t *target, khash_t(struct_map) *struct_map, khash_t(type_map) *type_map, khash_t(type_map) *enum_map,
        type_t *(*builtins)[LAST_BUILTIN + 1], khash_t(const_map) *const_map,
        khash_t(type_set) *type_set, preproc_t *prep, proc_token_t *tok, enum token_sym_type_e end_sym, type_t **typ) {
	enum decl_spec spec = SPEC_NONE;
	if (!parse_declaration_specifier(target, struct_map, type_map, enum_map, builtins, const_map, type_set, prep, tok, NULL, NULL, &spec, *typ)) {
		type_del(*typ);
		goto failed;
	}
	*typ = type_try_merge(*typ, type_set);
	if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == end_sym)) {
		return validate_type(&tok->loginfo, target, *typ);
	}
	struct parse_declarator_dest_s dest2;
	dest2.argt.dest = NULL;
	dest2.argt.struct_map = struct_map;
	dest2.argt.type_map = type_map;
	dest2.argt.enum_map = enum_map;
	dest2.argt.type_set = type_set;
	dest2.argt.builtins = builtins;
	dest2.argt.const_map = const_map;
	if (!parse_declarator(target, &dest2, prep, tok, TMPSTO_NONE, FSPEC_NONE, *typ, 0, 0, 0, 1)) {
		// Token is deleted
		type_del(*typ);
		goto failed;
	}
	type_del(*typ);
	if (!dest2.argt.dest) {
		log_internal(&tok->loginfo, "parse_type_name: argument type is NULL\n");
		// Empty destructor
		goto failed;
	}
	if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != end_sym)) {
		log_error(&tok->loginfo, "invalid token %s %u after type name\n",
		            (tok->tokt == PTOK_SYM) ? "symbol" : "type", (tok->tokt == PTOK_SYM) ? tok->tokv.sym : tok->tokt);
		type_del(dest2.argt.dest);
		proc_token_del(tok);
		goto failed;
	}
	*typ = dest2.argt.dest;
	*typ = type_try_merge(*typ, type_set);
	return validate_type(&tok->loginfo, target, *typ);
	
failed:
	return 0;
}

static expr_t *parse_expression(machine_t *target, khash_t(struct_map) *struct_map, khash_t(type_map) *type_map, khash_t(type_map) *enum_map,
        type_t *(*builtins)[LAST_BUILTIN + 1], khash_t(const_map) *const_map,
        khash_t(type_set) *type_set, preproc_t *prep, proc_token_t *tok, int expr_level) {
	// Note that expr_level >= 1; expr_level = 0 doesn't appear in the grammar
	if ((expr_level < 1) || (expr_level > 16)) {
		log_internal(&tok->loginfo, "Internal error: invalid expression level %d\n", expr_level);
		return NULL;
	}
	
	VECTOR(expr_pops) *op_stack = vector_new(expr_pops);
	if (!op_stack) {
		log_memory("failed to create operation stack");
		proc_token_del(tok);
		return NULL;
	}
	
	int has_level;
	expr_t *e;
pushed_expr:
	has_level = -1;
	e = NULL;
	
expr_new_token:
	if (tok->tokt == PTOK_IDENT) {
		if (has_level != -1) {
			log_error(&tok->loginfo, "invalid expression: unexpected identifier '%s'\n", string_content(tok->tokv.str));
			string_del(tok->tokv.str);
			goto failed;
		}
		has_level = 0;
		e = malloc(sizeof *e);
		if (!e) {
			log_memory("failed to create new expression atom\n");
			string_del(tok->tokv.str);
			goto failed;
		}
		e->typ = ETY_VAR;
		e->val.var = tok->tokv.str;
		*tok = proc_next_token(prep);
		goto expr_new_token;
	} else if (tok->tokt == PTOK_NUM) {
		if (has_level != -1) {
			log_error(&tok->loginfo, "invalid expression: unexpected number '%s'\n", string_content(tok->tokv.str));
			string_del(tok->tokv.str);
			goto failed;
		}
		has_level = 0;
		e = malloc(sizeof *e);
		if (!e) {
			log_memory("failed to create new expression atom\n");
			string_del(tok->tokv.str);
			goto failed;
		}
		e->typ = ETY_CONST;
		if (!num_constant_convert(&tok->loginfo, tok->tokv.str, &e->val.cst, target->size_long == 4)) {
			string_del(tok->tokv.str);
			goto failed;
		}
		string_del(tok->tokv.str);
		*tok = proc_next_token(prep);
		goto expr_new_token;
	} else if ((tok->tokt == PTOK_STRING) && !tok->tokv.sisstr) {
		if (has_level != -1) {
			log_error(&tok->loginfo, "invalid expression: unexpected character constant '%s'\n", string_content(tok->tokv.sstr));
			string_del(tok->tokv.sstr);
			goto failed;
		}
		if (string_len(tok->tokv.sstr) != 1) {
			log_TODO(&tok->loginfo, "invalid expression: multibyte character constant '%s'\n", string_content(tok->tokv.sstr));
			string_del(tok->tokv.sstr);
			goto failed;
		}
		has_level = 0;
		e = malloc(sizeof *e);
		if (!e) {
			log_memory("failed to create new expression atom\n");
			string_del(tok->tokv.sstr);
			goto failed;
		}
		e->typ = ETY_CONST;
		e->val.cst.typ = NCT_INT32;
		e->val.cst.val.i32 = (int32_t)string_content(tok->tokv.sstr)[0];
		string_del(tok->tokv.sstr);
		*tok = proc_next_token(prep);
		goto expr_new_token;
	}
	
#define UNOP(toksym, opt, main_lv, right_lv) \
	if ((expr_level >= main_lv) && (tok->tokt == PTOK_SYM) && (tok->tokv.sym == toksym)) { \
		if (has_level == -1) {                                                             \
			struct expr_partial_op pop = {                                                 \
				.once_done_want_level = expr_level,                                        \
				.once_done_is_level = main_lv,                                             \
				.typ = EPO_UNARY,                                                          \
				.val.unop = opt                                                            \
			};                                                                             \
			if (!vector_push(expr_pops, op_stack, pop)) {                                  \
				log_memory("failed to add partial operation to operation stack\n");     \
				/* Empty destructor */                                                     \
				goto failed;                                                               \
			}                                                                              \
			expr_level = right_lv;                                                         \
			*tok = proc_next_token(prep);                                                  \
			goto pushed_expr;                                                              \
		}                                                                                  \
	}
#define BINOP(toksym, opt, main_lv, left_lv, right_lv) \
	if ((expr_level >= main_lv) && (tok->tokt == PTOK_SYM) && (tok->tokv.sym == toksym)) { \
		if ((has_level != -1) && (has_level <= left_lv)) {                                 \
			struct expr_partial_op pop = {                                                 \
				.once_done_want_level = expr_level,                                        \
				.once_done_is_level = main_lv,                                             \
				.typ = EPO_BINARY_ARG,                                                     \
				.val.binop = {.op = opt, .e1 = e}                                          \
			};                                                                             \
			if (!vector_push(expr_pops, op_stack, pop)) {                                  \
				log_memory("failed to add partial operation to operation stack\n");     \
				/* Empty destructor */                                                     \
				goto failed;                                                               \
			}                                                                              \
			expr_level = right_lv;                                                         \
			*tok = proc_next_token(prep);                                                  \
			goto pushed_expr;                                                              \
		}                                                                                  \
	}
	
	if ((expr_level >= 1) && (tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_PLUSPLUS)) {
		if ((has_level != -1) && (has_level <= 1)) {
			has_level = 1;
			expr_t *new_e = malloc(sizeof *new_e);
			if (!new_e) {
				log_memory("failed to create new expression atom\n");
				string_del(tok->tokv.str);
				goto failed;
			}
			new_e->typ = ETY_UNARY;
			new_e->val.unary.typ = UOT_POSTINCR;
			new_e->val.unary.e = e;
			e = new_e;
			*tok = proc_next_token(prep);
			goto expr_new_token;
		}
	}
	if ((expr_level >= 1) && (tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_DASHDASH)) {
		if ((has_level != -1) && (has_level <= 1)) {
			has_level = 1;
			expr_t *new_e = malloc(sizeof *new_e);
			if (!new_e) {
				log_memory("failed to create new expression atom\n");
				string_del(tok->tokv.str);
				goto failed;
			}
			new_e->typ = ETY_UNARY;
			new_e->val.unary.typ = UOT_POSTDECR;
			new_e->val.unary.e = e;
			e = new_e;
			*tok = proc_next_token(prep);
			goto expr_new_token;
		}
	}
	if ((expr_level >= 1) && (tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_LSQBRACKET)) {
		if ((has_level != -1) && (has_level <= 1)) {
			struct expr_partial_op pop = {
				.once_done_want_level = expr_level,
				.once_done_is_level = 1,
				.typ = EPO_BINARY_ARG_SYM,
				.val.binop = {.last_sym = SYM_RSQBRACKET, .op = BOT_ARRAY, .e1 = e}
			};
			if (!vector_push(expr_pops, op_stack, pop)) {
				log_memory("failed to add partial operation to operation stack\n");
				/* Empty destructor */
				goto failed;
			}
			expr_level = 16;
			*tok = proc_next_token(prep);
			goto pushed_expr;
		}
	}
	if ((expr_level >= 1) && (tok->tokt == PTOK_SYM) && ((tok->tokv.sym == SYM_DOT) || (tok->tokv.sym == SYM_DASHGT))) {
		if ((has_level != -1) && (has_level <= 1)) {
			has_level = 1;
			int is_ptr = tok->tokv.sym == SYM_DASHGT;
			
			*tok = proc_next_token(prep);
			if (tok->tokt != PTOK_IDENT) {
				log_error(&tok->loginfo, "invalid expression: unexpected token after access symbol\n");
				proc_token_del(tok);
				goto failed;
			}
			expr_t *new_e = malloc(sizeof *new_e);
			if (!new_e) {
				log_memory("failed to create new expression atom\n");
				string_del(tok->tokv.str);
				goto failed;
			}
			new_e->typ = is_ptr ? ETY_PTRACCESS : ETY_ACCESS;
			new_e->val.access.val = e;
			new_e->val.access.member = tok->tokv.str;
			e = new_e;
			*tok = proc_next_token(prep);
			goto expr_new_token;
		}
	}
	UNOP(SYM_PLUSPLUS, UOT_PREINCR, 2, 2)
	UNOP(SYM_DASHDASH, UOT_PREDECR, 2, 2)
	UNOP(SYM_AMP,      UOT_REF,     2, 3)
	UNOP(SYM_PLUS,     UOT_POS,     2, 3)
	UNOP(SYM_DASH,     UOT_NEG,     2, 3)
	UNOP(SYM_STAR,     UOT_DEREF,   2, 3)
	UNOP(SYM_TILDE,    UOT_ANOT,    2, 3)
	UNOP(SYM_EXCL,     UOT_BNOT,    2, 3)
	BINOP(SYM_STAR,      BOT_MUL,         4,  4,  3)
	BINOP(SYM_SLASH,     BOT_DIV,         4,  4,  3)
	BINOP(SYM_PERCENT,   BOT_MOD,         4,  4,  3)
	BINOP(SYM_PLUS,      BOT_ADD,         5,  5,  4)
	BINOP(SYM_DASH,      BOT_SUB,         5,  5,  4)
	BINOP(SYM_LTLT,      BOT_LSH,         6,  6,  5)
	BINOP(SYM_GTGT,      BOT_RSH,         6,  6,  5)
	BINOP(SYM_LT,        BOT_LT,          7,  7,  6)
	BINOP(SYM_GT,        BOT_GT,          7,  7,  6)
	BINOP(SYM_LTEQ,      BOT_LE,          7,  7,  6)
	BINOP(SYM_GTEQ,      BOT_GE,          7,  7,  6)
	BINOP(SYM_EQEQ,      BOT_EQ,          8,  8,  7)
	BINOP(SYM_EXCLEQ,    BOT_NE,          8,  8,  7)
	BINOP(SYM_AMP,       BOT_AAND,        9,  9,  8)
	BINOP(SYM_HAT,       BOT_AXOR,       10, 10,  9)
	BINOP(SYM_PIPE,      BOT_AOR,        11, 11, 10)
	BINOP(SYM_AMPAMP,    BOT_BAND,       12, 12, 11)
	BINOP(SYM_PIPEPIPE,  BOT_BOR,        13, 13, 12)
	if ((expr_level >= 14) && (tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_QUESTION)) {
		if ((has_level != -1) && (has_level <= 13)) {
			struct expr_partial_op pop = {
				.once_done_want_level = expr_level,
				.once_done_is_level = 14,
				.typ = EPO_TERNARY_ARG1,
				.val.ternop = {.arg23_sep = SYM_COLON, .once_done2_want_level = 14, .op = TOT_COND, .e1 = e}
			};
			if (!vector_push(expr_pops, op_stack, pop)) {
				log_memory("failed to add partial operation to operation stack\n");
				// Empty destructor
				goto failed;
			}
			expr_level = 16;
			*tok = proc_next_token(prep);
			goto pushed_expr;
		}
	}
	BINOP(SYM_EQ,        BOT_ASSGN_EQ,   15,  2, 15)
	BINOP(SYM_STAREQ,    BOT_ASSGN_MUL,  15,  2, 15)
	BINOP(SYM_SLASHEQ,   BOT_ASSGN_DIV,  15,  2, 15)
	BINOP(SYM_PERCENTEQ, BOT_ASSGN_MOD,  15,  2, 15)
	BINOP(SYM_PLUSEQ,    BOT_ASSGN_ADD,  15,  2, 15)
	BINOP(SYM_DASHEQ,    BOT_ASSGN_SUB,  15,  2, 15)
	BINOP(SYM_LTLTEQ,    BOT_ASSGN_LSH,  15,  2, 15)
	BINOP(SYM_GTGTEQ,    BOT_ASSGN_RSH,  15,  2, 15)
	BINOP(SYM_AMPEQ,     BOT_ASSGN_AAND, 15,  2, 15)
	BINOP(SYM_HATEQ,     BOT_ASSGN_AXOR, 15,  2, 15)
	BINOP(SYM_PIPEEQ,    BOT_ASSGN_AOR,  15,  2, 15)
	BINOP(SYM_COMMA,     BOT_COMMA,      16, 16, 15)
	
	// expr2 ::= sizeof expr2
	// which includes expr2 ::= sizeof ( expr16 )
	// expr2 ::= sizeof ( type-name )
	if ((has_level == -1) && (expr_level >= 2) && (tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_SIZEOF)) {
		*tok = proc_next_token(prep);
		if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_LPAREN)) {
			struct expr_partial_op pop = {
				.once_done_want_level = expr_level,
				.once_done_is_level = 2,
				.typ = EPO_SIZEOF,
				.val.c = '\0'
			};
			if (!vector_push(expr_pops, op_stack, pop)) {
				log_memory("failed to add partial operation to operation stack\n");
				proc_token_del(tok);
				goto failed;
			}
			expr_level = 2;
			goto pushed_expr;
		}
		// Empty destructor
		*tok = proc_next_token(prep);
		khiter_t it;
		if (IS_BEGIN_TYPE_NAME) {
			type_t *typ = type_new();
			if (!typ) {
				log_memory("failed to create new type info structure\n");
				proc_token_del(tok);
				goto failed;
			}
			if (!parse_type_name(target, struct_map, type_map, enum_map, builtins, const_map, type_set, prep, tok, SYM_RPAREN, &typ)) {
				goto failed;
			}
			if (!typ->is_validated || typ->is_incomplete || (typ->typ == TYPE_FUNCTION)) {
				log_error(&tok->loginfo, "cannot get the size of a function or incomplete type\n");
				type_del(typ);
				proc_token_del(tok);
				goto failed;
			}
			e = malloc(sizeof *e);
			if (!e) {
				log_memory("failed to create new expression atom\n");
				type_del(typ);
				proc_token_del(tok);
				goto failed;
			}
			e->typ = ETY_CONST;
			e->val.cst.typ = NCT_UINT64;
			e->val.cst.val.u64 = typ->szinfo.size;
			has_level = 2;
			type_del(typ);
			if (!e->val.cst.val.u64) {
				log_internal(&tok->loginfo, "size of type is 0\n");
				proc_token_del(tok);
				goto failed;
			}
			// Empty destructor
			*tok = proc_next_token(prep);
			goto expr_new_token;
		} else {
			struct expr_partial_op pop = {
				.once_done_want_level = expr_level,
				.once_done_is_level = 2,
				.typ = EPO_SIZEOF,
				.val.c = 0
			};
			if (!vector_push(expr_pops, op_stack, pop)) {
				log_memory("failed to add partial operation to operation stack\n");
				proc_token_del(tok);
				goto failed;
			}
			pop = (struct expr_partial_op){
				.once_done_want_level = 2,
				.once_done_is_level = 0,
				.typ = EPO_LPAREN,
				.val.c = 0
			};
			if (!vector_push(expr_pops, op_stack, pop)) {
				log_memory("failed to add partial operation to operation stack\n");
				proc_token_del(tok);
				goto failed;
			}
			expr_level = 16;
			goto pushed_expr;
		}
	}
	// expr2 ::= _Alignof ( type-name )
	if ((has_level == -1) && (expr_level >= 2) && (tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_ALIGNOF)) {
		*tok = proc_next_token(prep);
		if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_LPAREN)) {
			log_error(&tok->loginfo, "unexpected token, expected symbol '('\n");
			proc_token_del(tok);
			goto failed;
		}
		// Empty destructor
		*tok = proc_next_token(prep);
		type_t *typ = type_new();
		if (!typ) {
			log_memory("failed to create new type info structure\n");
			proc_token_del(tok);
			goto failed;
		}
		if (!parse_type_name(target, struct_map, type_map, enum_map, builtins, const_map, type_set, prep, tok, SYM_RPAREN, &typ)) {
			goto failed;
		}
		if (!typ->is_validated || typ->is_incomplete || (typ->typ == TYPE_FUNCTION)) {
			log_error(&tok->loginfo, "cannot get the alignment of a function or incomplete type\n");
			type_del(typ);
			proc_token_del(tok);
			goto failed;
		}
		e = malloc(sizeof *e);
		if (!e) {
			log_memory("failed to create new expression atom\n");
			type_del(typ);
			proc_token_del(tok);
			goto failed;
		}
		e->typ = ETY_CONST;
		e->val.cst.typ = NCT_UINT64;
		e->val.cst.val.u64 = typ->szinfo.align;
		has_level = 2;
		type_del(typ);
		if (!e->val.cst.val.u64) {
			log_internal(&tok->loginfo, "alignment of type is 0\n");
			proc_token_del(tok);
			goto failed;
		}
		// Empty destructor
		*tok = proc_next_token(prep);
		goto expr_new_token;
	}
	
	// expr0 ::= ( expr16 )
	// expr1 ::= expr1 ( )
	// expr1 ::= expr1 ( expr15 {, expr15}* )
	// expr1 ::= ( type-name ) { initializer-list ,? }
	// expr3 ::= ( type-name ) expr3
	if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_LPAREN)) {
		if ((has_level != -1) && (expr_level >= 1) && (has_level <= 1)) {
			// Empty destructor
			*tok = proc_next_token(prep);
			if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
				expr_t *new_e = malloc(sizeof *new_e);
				new_e = malloc(sizeof *new_e);
				if (!new_e) {
					log_memory("failed to create new expression atom\n");
					// Empty destructor
					goto failed;
				}
				new_e->typ = ETY_CALL;
				new_e->val.call.fun = e;
				new_e->val.call.nargs = 0;
				new_e->val.call.args = NULL;
				e = new_e;
				*tok = proc_next_token(prep);
				goto expr_new_token;
			}
			// We want a function call with at least one argument
			VECTOR(exprs) *exprs = vector_new_cap(exprs, 1);
			if (!exprs) {
				log_memory("failed to add partial operation to operation stack\n");
				proc_token_del(tok);
				goto failed;
			}
			struct expr_partial_op pop = {
				.once_done_want_level = expr_level,
				.once_done_is_level = 1,
				.typ = EPO_FUNCALL,
				.val.funcall = {.f = e, .exprs = exprs}
			};
			if (!vector_push(expr_pops, op_stack, pop)) {
				log_memory("failed to add partial operation to operation stack\n");
				vector_del(exprs, exprs);
				proc_token_del(tok);
				goto failed;
			}
			expr_level = 15;
			goto pushed_expr;
		}
		if (has_level == -1) {
			*tok = proc_next_token(prep);
			khiter_t it;
			if ((expr_level >= 1) && IS_BEGIN_TYPE_NAME) {
				type_t *typ = type_new();
				if (!typ) {
					log_memory("failed to create new type info structure\n");
					proc_token_del(tok);
					goto failed;
				}
				if (!parse_type_name(target, struct_map, type_map, enum_map, builtins, const_map, type_set, prep, tok, SYM_RPAREN, &typ)) {
					type_del(typ);
					proc_token_del(tok);
					goto failed;
				}
				if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_LBRACKET)) {
					log_TODO(&tok->loginfo, "initializer-list\n");
					type_del(typ);
					proc_token_del(tok);
					goto failed;
				} else if (expr_level < 3) {
					log_error(&tok->loginfo, "cast expression (level 3) but the required expression level is 1 or 2 (expected symbol '{')\n");
					type_del(typ);
					proc_token_del(tok);
					goto failed;
				} else {
					struct expr_partial_op pop = {
						.once_done_want_level = expr_level,
						.once_done_is_level = 3,
						.typ = EPO_CAST,
						.val.typ = typ
					};
					if (!vector_push(expr_pops, op_stack, pop)) {
						log_memory("failed to add partial operation to operation stack\n");
						/* Empty destructor */
						goto failed;
					}
					expr_level = 3;
					*tok = proc_next_token(prep);
					goto pushed_expr;
				}
			} else {
				struct expr_partial_op pop = {
					.once_done_want_level = expr_level,
					.once_done_is_level = 0,
					.typ = EPO_LPAREN,
					.val.c = 0
				};
				if (!vector_push(expr_pops, op_stack, pop)) {
					log_memory("failed to add partial operation to operation stack\n");
					proc_token_del(tok);
					goto failed;
				}
				expr_level = 16;
				goto pushed_expr;
			}
		}
	}
	
	if (vector_size(expr_pops, op_stack)) {
		if (has_level != -1) {
			struct expr_partial_op *pop = &vector_last(expr_pops, op_stack);
			expr_t *new_e;
			switch (pop->typ) {
			case EPO_LPAREN:
				if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
					has_level = pop->once_done_is_level;
					expr_level = pop->once_done_want_level;
					vector_pop_nodel(expr_pops, op_stack);
					// Empty destructor
					*tok = proc_next_token(prep);
					goto expr_new_token;
				}
				break;
			case EPO_SIZEOF:
				log_TODO(&tok->loginfo, "find type of expression\n");
				proc_token_del(tok);
				goto failed;
			case EPO_CAST:
				new_e = malloc(sizeof *new_e);
				if (!new_e) {
					log_memory("failed to create new expression atom\n");
					proc_token_del(tok);
					goto failed;
				}
				has_level = pop->once_done_is_level;
				expr_level = pop->once_done_want_level;
				new_e->typ = ETY_CAST;
				new_e->val.cast.typ = pop->val.typ;
				new_e->val.cast.e = e;
				e = new_e;
				vector_pop_nodel(expr_pops, op_stack);
				// Keep the same token
				goto expr_new_token;
			case EPO_FUNCALL:
				if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
					has_level = pop->once_done_is_level;
					expr_level = pop->once_done_want_level;
					if (!vector_push(exprs, pop->val.funcall.exprs, e)) {
						log_memory("failed to add argument to argument stack\n");
						// Empty destructor
						goto failed;
					}
					e = malloc(sizeof *e);
					if (!e) {
						log_memory("failed to create new expression atom\n");
						// Empty destructor
						goto failed;
					}
					e->typ = ETY_CALL;
					e->val.call.fun = pop->val.funcall.f;
					e->val.call.nargs = vector_size(exprs, pop->val.funcall.exprs);
					e->val.call.args = vector_steal(exprs, pop->val.funcall.exprs);
					vector_pop_nodel(expr_pops, op_stack);
					// Empty destructor
					*tok = proc_next_token(prep);
					goto expr_new_token;
				} else if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_COMMA)) {
					if (!vector_push(exprs, pop->val.funcall.exprs, e)) {
						log_memory("failed to add argument to argument stack\n");
						// Empty destructor
						goto failed;
					}
					// expr_level is already 15
					// Empty destructor
					*tok = proc_next_token(prep);
					goto pushed_expr;
				}
				break;
			case EPO_UNARY:
				new_e = malloc(sizeof *new_e);
				if (!new_e) {
					log_memory("failed to create new expression atom\n");
					proc_token_del(tok);
					goto failed;
				}
				has_level = pop->once_done_is_level;
				expr_level = pop->once_done_want_level;
				new_e->typ = ETY_UNARY;
				new_e->val.unary.typ = pop->val.unop;
				new_e->val.unary.e = e;
				e = new_e;
				vector_pop_nodel(expr_pops, op_stack);
				// Keep the same token
				goto expr_new_token;
			case EPO_BINARY_ARG_SYM:
				if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != pop->val.binop.last_sym)) break;
				// Empty destructor
				*tok = proc_next_token(prep);
				/* FALLTHROUGH */
			case EPO_BINARY_ARG:
				new_e = malloc(sizeof *new_e);
				if (!new_e) {
					log_memory("failed to create new expression atom\n");
					proc_token_del(tok);
					goto failed;
				}
				has_level = pop->once_done_is_level;
				expr_level = pop->once_done_want_level;
				new_e->typ = ETY_BINARY;
				new_e->val.binary.typ = pop->val.binop.op;
				new_e->val.binary.e1 = pop->val.binop.e1;
				new_e->val.binary.e2 = e;
				e = new_e;
				vector_pop_nodel(expr_pops, op_stack);
				// Keep the same token
				goto expr_new_token;
			case EPO_TERNARY_ARG1:
				if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == pop->val.ternop.arg23_sep)) {
					pop->typ = EPO_TERNARY_ARG2;
					pop->val.ternop.e2 = e;
					expr_level = pop->val.ternop.once_done2_want_level;
					// Empty destructor
					*tok = proc_next_token(prep);
					goto pushed_expr;
				}
				break;
			case EPO_TERNARY_ARG2:
				new_e = malloc(sizeof *new_e);
				if (!new_e) {
					log_memory("failed to create new expression atom\n");
					proc_token_del(tok);
					goto failed;
				}
				has_level = pop->once_done_is_level;
				expr_level = pop->once_done_want_level;
				new_e->typ = ETY_TERNARY;
				new_e->val.ternary.typ = pop->val.ternop.op;
				new_e->val.ternary.e1 = pop->val.ternop.e1;
				new_e->val.ternary.e2 = pop->val.ternop.e2;
				new_e->val.ternary.e3 = e;
				e = new_e;
				vector_pop_nodel(expr_pops, op_stack);
				// Keep the same token
				goto expr_new_token;
			}
		}
	}
	
	if ((vector_size(expr_pops, op_stack) == 0) && (has_level != -1)) {
		vector_del(expr_pops, op_stack);
		return e;
	}
	log_error(&tok->loginfo, "invalid expression: unexpected token\n");
	proc_token_del(tok);
failed:
	vector_del(expr_pops, op_stack);
	if (e) expr_del(e);
	return NULL;
}
static int eval_expression(loginfo_t *li, machine_t *target, expr_t *e, khash_t(const_map) *const_map, num_constant_t *dest, int fatal) {
	// Evaluate the expression (we suppose it is constant)
	switch (e->typ) {
	case ETY_VAR: {
		khiter_t it = kh_get(const_map, const_map, string_content(e->val.var));
		if (it != kh_end(const_map)) {
			*dest = kh_val(const_map, it);
			return 1;
		}
		if (fatal) log_error(li, "failed to evaluate expression: expression is not constant (variable)\n");
		return 0; }
		
	case ETY_CONST:
		*dest = e->val.cst;
		return 1;
		
	// case ETY_GENERIC:
		
	case ETY_CALL:
		if (fatal) log_error(li, "failed to evaluate expression: expression is not constant (function call)\n");
		return 0;
		
	case ETY_ACCESS:
	case ETY_PTRACCESS:
		if (fatal) log_error(li, "failed to evaluate expression: expression is not constant (member access)\n");
		return 0;
		
	case ETY_UNARY:
		if (!eval_expression(li, target, e->val.unary.e, const_map, dest, fatal)) return 0;
		
		switch (e->val.unary.typ) {
		case UOT_POSTINCR:
		case UOT_POSTDECR:
		case UOT_PREINCR:
		case UOT_PREDECR:
		case UOT_REF:
		case UOT_DEREF:
			if (fatal) log_error(li, "failed to evaluate expression: expression is not constant (assignment or memory accesses)\n");
			return 0;
		case UOT_POS:
			return 1; // Nothing to do
		case UOT_NEG:
			switch (dest->typ) {
			case NCT_FLOAT:   dest->val.f   = -dest->val.f;   return 1;
			case NCT_DOUBLE:  dest->val.d   = -dest->val.d;   return 1;
			case NCT_LDOUBLE: dest->val.l   = -dest->val.l;   return 1;
			case NCT_INT32:   dest->val.i32 = -dest->val.i32; return 1;
			case NCT_UINT32:  dest->val.u32 = -dest->val.u32; return 1;
			case NCT_INT64:   dest->val.i64 = -dest->val.i64; return 1;
			case NCT_UINT64:  dest->val.u64 = -dest->val.u64; return 1;
			default: return 0;
			}
		case UOT_ANOT:
			switch (dest->typ) {
			case NCT_FLOAT:
			case NCT_DOUBLE:
			case NCT_LDOUBLE:
				if (fatal) log_error(li, "failed to evaluate expression: cannot bitwise-negate a floating point number\n");
				return 0;
			case NCT_INT32:  dest->val.i32 = ~dest->val.i32; return 1;
			case NCT_UINT32: dest->val.u32 = ~dest->val.u32; return 1;
			case NCT_INT64:  dest->val.i64 = ~dest->val.i64; return 1;
			case NCT_UINT64: dest->val.u64 = ~dest->val.u64; return 1;
			default: return 0;
			}
		case UOT_BNOT:
			switch (dest->typ) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
			case NCT_FLOAT:   dest->typ = NCT_INT32; dest->val.i32 = !dest->val.f;   return 1;
			case NCT_DOUBLE:  dest->typ = NCT_INT32; dest->val.i32 = !dest->val.d;   return 1;
			case NCT_LDOUBLE: dest->typ = NCT_INT32; dest->val.i32 = !dest->val.l;   return 1;
#pragma GCC diagnostic pop
			case NCT_INT32:   dest->typ = NCT_INT32; dest->val.i32 = !dest->val.i32; return 1;
			case NCT_UINT32:  dest->typ = NCT_INT32; dest->val.i32 = !dest->val.u32; return 1;
			case NCT_INT64:   dest->typ = NCT_INT32; dest->val.i32 = !dest->val.i64; return 1;
			case NCT_UINT64:  dest->typ = NCT_INT32; dest->val.i32 = !dest->val.u64; return 1;
			default: return 0;
			}
		default: return 0;
		}
		
	case ETY_BINARY: {
		num_constant_t dest1, dest2;
		if (!eval_expression(li, target, e->val.binary.e1, const_map, &dest1, fatal)) return 0;
		if (!eval_expression(li, target, e->val.binary.e2, const_map, &dest2, fatal)) return 0;
		
		switch (e->val.binary.typ) {
		case BOT_ASSGN_EQ:
		case BOT_ASSGN_ADD:
		case BOT_ASSGN_SUB:
		case BOT_ASSGN_MUL:
		case BOT_ASSGN_DIV:
		case BOT_ASSGN_MOD:
		case BOT_ASSGN_LSH:
		case BOT_ASSGN_RSH:
		case BOT_ASSGN_AAND:
		case BOT_ASSGN_AXOR:
		case BOT_ASSGN_AOR:
		case BOT_ARRAY: // Is this possible?
			if (fatal) log_error(li, "failed to evaluate expression: expression is not constant (assignments)\n");
			return 0;
#define DOIT(op) \
			promote_csts(&dest1, &dest2); \
			switch ((dest->typ = dest1.typ)) { \
			case NCT_FLOAT:   dest->val.f   = dest1.val.f   op dest2.val.f  ; return 1; \
			case NCT_DOUBLE:  dest->val.d   = dest1.val.d   op dest2.val.d  ; return 1; \
			case NCT_LDOUBLE: dest->val.l   = dest1.val.l   op dest2.val.l  ; return 1; \
			case NCT_INT32:   dest->val.i32 = dest1.val.i32 op dest2.val.i32; return 1; \
			case NCT_UINT32:  dest->val.u32 = dest1.val.u32 op dest2.val.u32; return 1; \
			case NCT_INT64:   dest->val.i64 = dest1.val.i64 op dest2.val.i64; return 1; \
			case NCT_UINT64:  dest->val.u64 = dest1.val.u64 op dest2.val.u64; return 1; \
			default: return 0;                                                          \
			}
#define DOIT_INT(op) \
			promote_csts(&dest1, &dest2); \
			switch ((dest->typ = dest1.typ)) { \
			case NCT_FLOAT: \
			case NCT_DOUBLE: \
			case NCT_LDOUBLE: \
				if (fatal) log_error(li, "failed to evaluate expression: binary operation %u incompatible with floating point numbers\n", e->val.binary.typ); \
				return 0; \
			case NCT_INT32:   dest->val.i32 = dest1.val.i32 op dest2.val.i32; return 1; \
			case NCT_UINT32:  dest->val.u32 = dest1.val.u32 op dest2.val.u32; return 1; \
			case NCT_INT64:   dest->val.i64 = dest1.val.i64 op dest2.val.i64; return 1; \
			case NCT_UINT64:  dest->val.u64 = dest1.val.u64 op dest2.val.u64; return 1; \
			default: return 0;                                                          \
			}
#define DOIT_BOOL(op) \
			promote_csts(&dest1, &dest2); \
			switch (dest1.typ) { \
			case NCT_FLOAT:   dest->typ = NCT_UINT32; dest->val.u32 = dest1.val.f   op dest2.val.f  ; return 1; \
			case NCT_DOUBLE:  dest->typ = NCT_UINT32; dest->val.u32 = dest1.val.d   op dest2.val.d  ; return 1; \
			case NCT_LDOUBLE: dest->typ = NCT_UINT32; dest->val.u32 = dest1.val.l   op dest2.val.l  ; return 1; \
			case NCT_INT32:   dest->typ = NCT_UINT32; dest->val.u32 = dest1.val.i32 op dest2.val.i32; return 1; \
			case NCT_UINT32:  dest->typ = NCT_UINT32; dest->val.u32 = dest1.val.u32 op dest2.val.u32; return 1; \
			case NCT_INT64:   dest->typ = NCT_UINT32; dest->val.u32 = dest1.val.i64 op dest2.val.i64; return 1; \
			case NCT_UINT64:  dest->typ = NCT_UINT32; dest->val.u32 = dest1.val.u64 op dest2.val.u64; return 1; \
			default: return 0;                                                                                  \
			}
		case BOT_ADD: DOIT(+)
		case BOT_SUB: DOIT(-)
		case BOT_MUL: DOIT(*)
		case BOT_DIV: DOIT(/)
		case BOT_MOD: DOIT_INT(%)
		case BOT_LSH: DOIT_INT(<<)
		case BOT_RSH: DOIT_INT(>>)
		case BOT_LT: DOIT_BOOL(<)
		case BOT_GT: DOIT_BOOL(>)
		case BOT_LE: DOIT_BOOL(<=)
		case BOT_GE: DOIT_BOOL(>=)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
		case BOT_EQ: DOIT_BOOL(==)
		case BOT_NE: DOIT_BOOL(!=)
#pragma GCC diagnostic pop
		case BOT_AAND: DOIT_INT(&)
		case BOT_AXOR: DOIT_INT(^)
		case BOT_AOR: DOIT_INT(|)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
		case BOT_BAND: DOIT_BOOL(&&)
		case BOT_BOR: DOIT_BOOL(||)
#pragma GCC diagnostic pop
		case BOT_COMMA: *dest = dest2; return 1;
#undef DOIT_BOOL
#undef DOIT_INT
#undef DOIT
		default: return 0;
		} }
		
	case ETY_TERNARY: {
		num_constant_t dest1, dest2, dest3;
		if (!eval_expression(li, target, e->val.ternary.e1, const_map, &dest1, fatal)) return 0;
		if (!eval_expression(li, target, e->val.ternary.e2, const_map, &dest2, fatal)) return 0;
		if (!eval_expression(li, target, e->val.ternary.e3, const_map, &dest3, fatal)) return 0;
		
		switch (e->val.ternary.typ) {
		case TOT_COND:
			promote_csts(&dest2, &dest3);
			switch (dest1.typ) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
			case NCT_FLOAT:   *dest = dest1.val.f   ? dest2 : dest3; return 1;
			case NCT_DOUBLE:  *dest = dest1.val.d   ? dest2 : dest3; return 1;
			case NCT_LDOUBLE: *dest = dest1.val.l   ? dest2 : dest3; return 1;
#pragma GCC diagnostic pop
			case NCT_INT32:   *dest = dest1.val.i32 ? dest2 : dest3; return 1;
			case NCT_UINT32:  *dest = dest1.val.u32 ? dest2 : dest3; return 1;
			case NCT_INT64:   *dest = dest1.val.i64 ? dest2 : dest3; return 1;
			case NCT_UINT64:  *dest = dest1.val.u64 ? dest2 : dest3; return 1;
			default: return 0;
			}
		default: return 0;
		} }
		
	// case ETY_INIT_LIST:
		
	case ETY_CAST:
		if (!eval_expression(li, target, e->val.cast.e, const_map, dest, fatal)) return 0;
		
		if ((e->val.cast.typ->typ == TYPE_BUILTIN) || (e->val.cast.typ->typ == TYPE_ENUM)) {
			type_t *cast = e->val.cast.typ;
			if (cast->typ == TYPE_ENUM) {
				cast = cast->val.typ;
				if (cast->typ != TYPE_BUILTIN) {
					log_internal(li, "invalid cast to enum with non-builtin base type\n");
					return 0;
				}
			}
			switch (cast->val.builtin) {
			case BTT_VOID:
				if (fatal) log_error(li, "invalid cast to void\n");
				return 0;
			case BTT_BOOL:
				switch (dest->typ) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
				case NCT_FLOAT:   dest->val.u32 = (_Bool)dest->val.f; break;
				case NCT_DOUBLE:  dest->val.u32 = (_Bool)dest->val.d; break;
				case NCT_LDOUBLE: dest->val.u32 = (_Bool)dest->val.l; break;
#pragma GCC diagnostic pop
				case NCT_INT32:   dest->val.u32 = (_Bool)dest->val.i32; break;
				case NCT_UINT32:  dest->val.u32 = (_Bool)dest->val.u32; break;
				case NCT_INT64:   dest->val.u32 = (_Bool)dest->val.i64; break;
				case NCT_UINT64:  dest->val.u32 = (_Bool)dest->val.u64; break;
				}
				dest->typ = NCT_UINT32;
				return 1;
			case BTT_SCHAR:
			case BTT_S8:
			cast_s8:
				switch (dest->typ) {
				case NCT_FLOAT:   dest->val.i32 = (int8_t)dest->val.f; break;
				case NCT_DOUBLE:  dest->val.i32 = (int8_t)dest->val.d; break;
				case NCT_LDOUBLE: dest->val.i32 = (int8_t)dest->val.l; break;
				case NCT_INT32:   dest->val.i32 = (int8_t)dest->val.i32; break;
				case NCT_UINT32:  dest->val.i32 = (int8_t)dest->val.u32; break;
				case NCT_INT64:   dest->val.i32 = (int8_t)dest->val.i64; break;
				case NCT_UINT64:  dest->val.i32 = (int8_t)dest->val.u64; break;
				}
				dest->typ = NCT_INT32;
				return 1;
			case BTT_UCHAR:
			case BTT_U8:
			cast_u8:
				switch (dest->typ) {
				case NCT_FLOAT:   dest->val.u32 = (uint8_t)dest->val.f; break;
				case NCT_DOUBLE:  dest->val.u32 = (uint8_t)dest->val.d; break;
				case NCT_LDOUBLE: dest->val.u32 = (uint8_t)dest->val.l; break;
				case NCT_INT32:   dest->val.u32 = (uint8_t)dest->val.i32; break;
				case NCT_UINT32:  dest->val.u32 = (uint8_t)dest->val.u32; break;
				case NCT_INT64:   dest->val.u32 = (uint8_t)dest->val.i64; break;
				case NCT_UINT64:  dest->val.u32 = (uint8_t)dest->val.u64; break;
				}
				dest->typ = NCT_UINT32;
				return 1;
			case BTT_SHORT:
			case BTT_SSHORT:
			case BTT_S16:
				switch (dest->typ) {
				case NCT_FLOAT:   dest->val.i32 = (int16_t)dest->val.f; break;
				case NCT_DOUBLE:  dest->val.i32 = (int16_t)dest->val.d; break;
				case NCT_LDOUBLE: dest->val.i32 = (int16_t)dest->val.l; break;
				case NCT_INT32:   dest->val.i32 = (int16_t)dest->val.i32; break;
				case NCT_UINT32:  dest->val.i32 = (int16_t)dest->val.u32; break;
				case NCT_INT64:   dest->val.i32 = (int16_t)dest->val.i64; break;
				case NCT_UINT64:  dest->val.i32 = (int16_t)dest->val.u64; break;
				}
				dest->typ = NCT_INT32;
				return 1;
			case BTT_USHORT:
			case BTT_U16:
				switch (dest->typ) {
				case NCT_FLOAT:   dest->val.u32 = (uint16_t)dest->val.f; break;
				case NCT_DOUBLE:  dest->val.u32 = (uint16_t)dest->val.d; break;
				case NCT_LDOUBLE: dest->val.u32 = (uint16_t)dest->val.l; break;
				case NCT_INT32:   dest->val.u32 = (uint16_t)dest->val.i32; break;
				case NCT_UINT32:  dest->val.u32 = (uint16_t)dest->val.u32; break;
				case NCT_INT64:   dest->val.u32 = (uint16_t)dest->val.i64; break;
				case NCT_UINT64:  dest->val.u32 = (uint16_t)dest->val.u64; break;
				}
				dest->typ = NCT_UINT32;
				return 1;
			case BTT_INT:
			case BTT_SINT:
			case BTT_S32:
			cast_s32:
				switch (dest->typ) {
				case NCT_FLOAT:   dest->val.i32 = (int32_t)dest->val.f; break;
				case NCT_DOUBLE:  dest->val.i32 = (int32_t)dest->val.d; break;
				case NCT_LDOUBLE: dest->val.i32 = (int32_t)dest->val.l; break;
				case NCT_INT32: break;
				case NCT_UINT32:  dest->val.i32 = (int32_t)dest->val.u32; break;
				case NCT_INT64:   dest->val.i32 = (int32_t)dest->val.i64; break;
				case NCT_UINT64:  dest->val.i32 = (int32_t)dest->val.u64; break;
				}
				dest->typ = NCT_INT32;
				return 1;
			case BTT_UINT:
			case BTT_U32:
			cast_u32:
				switch (dest->typ) {
				case NCT_FLOAT:   dest->val.u32 = (uint32_t)dest->val.f; break;
				case NCT_DOUBLE:  dest->val.u32 = (uint32_t)dest->val.d; break;
				case NCT_LDOUBLE: dest->val.u32 = (uint32_t)dest->val.l; break;
				case NCT_INT32:   dest->val.u32 = (uint32_t)dest->val.i32; break;
				case NCT_UINT32: break;
				case NCT_INT64:   dest->val.u32 = (uint32_t)dest->val.i64; break;
				case NCT_UINT64:  dest->val.u32 = (uint32_t)dest->val.u64; break;
				}
				dest->typ = NCT_UINT32;
				return 1;
			case BTT_LONGLONG:
			case BTT_SLONGLONG:
			case BTT_S64:
			cast_s64:
				switch (dest->typ) {
				case NCT_FLOAT:   dest->val.i64 = (int64_t)dest->val.f; break;
				case NCT_DOUBLE:  dest->val.i64 = (int64_t)dest->val.d; break;
				case NCT_LDOUBLE: dest->val.i64 = (int64_t)dest->val.l; break;
				case NCT_INT32:   dest->val.i64 = (int64_t)dest->val.i32; break;
				case NCT_UINT32:  dest->val.i64 = (int64_t)dest->val.u32; break;
				case NCT_INT64: break;
				case NCT_UINT64:  dest->val.i64 = (int64_t)dest->val.u64; break;
				}
				dest->typ = NCT_INT64;
				return 1;
			case BTT_ULONGLONG:
			case BTT_U64:
			cast_u64:
				switch (dest->typ) {
				case NCT_FLOAT:   dest->val.u64 = (uint64_t)dest->val.f; break;
				case NCT_DOUBLE:  dest->val.u64 = (uint64_t)dest->val.d; break;
				case NCT_LDOUBLE: dest->val.u64 = (uint64_t)dest->val.l; break;
				case NCT_INT32:   dest->val.u64 = (uint64_t)dest->val.i32; break;
				case NCT_UINT32:  dest->val.u64 = (uint64_t)dest->val.u32; break;
				case NCT_INT64:   dest->val.u64 = (uint64_t)dest->val.i64; break;
				case NCT_UINT64: break;
				}
				dest->typ = NCT_UINT64;
				return 1;
			case BTT_CHAR:
				if (target->unsigned_char) goto cast_u8;
				else                       goto cast_s8;
			case BTT_LONG:
			case BTT_SLONG:
				if (target->size_long == 4) {
					goto cast_s32;
				} else if (target->size_long == 8) {
					goto cast_s64;
				} else {
					log_internal(li, "unsupported cast to long with target sizeof(long) == %zu\n", target->size_long);
					return 0;
				}
			case BTT_ULONG:
				if (target->size_long == 4) {
					goto cast_u32;
				} else if (target->size_long == 8) {
					goto cast_u64;
				} else {
					log_internal(li, "unsupported cast to unsigned long with target sizeof(long) == %zu\n", target->size_long);
					return 0;
				}
			case BTT_INT128:
			case BTT_SINT128:
			case BTT_UINT128:
			case BTT_FLOAT:
			case BTT_CFLOAT:
			case BTT_IFLOAT:
			case BTT_DOUBLE:
			case BTT_CDOUBLE:
			case BTT_IDOUBLE:
			case BTT_LONGDOUBLE:
			case BTT_CLONGDOUBLE:
			case BTT_ILONGDOUBLE:
			case BTT_FLOAT128:
			case BTT_CFLOAT128:
			case BTT_IFLOAT128:
			case BTT_VA_LIST:
			default:
				if (fatal) log_TODO(li, "unsupported cast to builtin %s in constant expression\n", builtin2str[cast->val.builtin]);
				return 0;
			}
		} else {
			if (fatal) log_TODO(li, "unsupported cast in constant expression\n");
			return 0;
		}
		
	default: return 0;
	}
}

extern int is_gst; // If 1, mark structures _G*Class as simple

// declaration-specifier with storage != NULL
// specifier-qualifier-list + static_assert-declaration with storage == NULL
static int parse_declaration_specifier(machine_t *target, khash_t(struct_map) *struct_map, khash_t(type_map) *type_map, khash_t(type_map) *enum_map,
        type_t *(*builtins)[LAST_BUILTIN + 1], khash_t(const_map) *const_map,
        khash_t(type_set) *type_set, preproc_t *prep, proc_token_t *tok, enum decl_storage *storage,
        enum fun_spec *fspec, enum decl_spec *spec, type_t *typ) {
	if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_STATIC_ASSERT)) {
		// _Static_assert ( constant-expression , string-literal ) ;
		// Empty destructor
		*tok = proc_next_token(prep);
		if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_LPAREN)) {
			log_error(&tok->loginfo, "unexpected token in static assertion declaration\n");
			proc_token_del(tok);
			goto failed;
		}
		// Empty destructor
		*tok = proc_next_token(prep);
		expr_t *e = parse_expression(target, struct_map, type_map, enum_map, builtins, const_map, type_set, prep, tok, 14);
		if (!e) {
			goto failed;
		}
		if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_COMMA)) {
			log_error(&tok->loginfo, "unexpected token in static assertion expression\n");
			expr_del(e);
			proc_token_del(tok);
			goto failed;
		}
		num_constant_t eval;
		if (!eval_expression(&tok->loginfo, target, e, const_map, &eval, 1)) {
			expr_del(e);
			// Empty destructor
			goto failed;
		}
		expr_del(e);
		*tok = proc_next_token(prep);
		if ((tok->tokt != PTOK_STRING) || !tok->tokv.sisstr) {
			log_error(&tok->loginfo, "unexpected token in static assertion message\n");
			proc_token_del(tok);
			goto failed;
		}
		string_t *errmsg = tok->tokv.str;
		*tok = proc_next_token(prep);
		if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_RPAREN)) {
			log_error(&tok->loginfo, "unexpected token in static assertion message\n");
			proc_token_del(tok);
			goto failed;
		}
		*tok = proc_next_token(prep);
		if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_SEMICOLON)) {
			log_error(&tok->loginfo, "unexpected token after static assertion\n");
			proc_token_del(tok);
			goto failed;
		}
		
		int iserror;
		switch (eval.typ) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
		case NCT_FLOAT:   iserror = !eval.val.f;   break;
		case NCT_DOUBLE:  iserror = !eval.val.d;   break;
		case NCT_LDOUBLE: iserror = !eval.val.l;   break;
#pragma GCC diagnostic pop
		case NCT_INT32:   iserror = !eval.val.i32; break;
		case NCT_UINT32:  iserror = !eval.val.u32; break;
		case NCT_INT64:   iserror = !eval.val.i64; break;
		case NCT_UINT64:  iserror = !eval.val.u64; break;
		default: iserror = 1;
		}
		if (iserror) {
			log_error(&tok->loginfo, "static assertion failed: %s\n", string_content(errmsg));
			string_del(errmsg);
			// Empty destructor
			goto failed;
		}
		string_del(errmsg);
		// Empty destructor
		return 1;
	}
	
parse_cur_token_decl:
	if (tok->tokt == PTOK_EOF) {
		log_error(&tok->loginfo, "unexpected end of file in declaration\n");
		goto failed;
	}
	// Storage
	if (storage && (tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_AUTO)) {
		if (*storage == TMPSTO_NONE) *storage = TMPSTO_AUTO;
		else {
			log_error(&tok->loginfo, "unexpected storage class specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if (storage && (tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_EXTERN)) {
		if (*storage == TMPSTO_NONE) *storage = TMPSTO_EXTERN;
		else if (*storage == TMPSTO_TLS) *storage = TMPSTO_TLS_EXTERN;
		else {
			log_error(&tok->loginfo, "unexpected storage class specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if (storage && (tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_REGISTER)) {
		if (*storage == TMPSTO_NONE) *storage = TMPSTO_REG;
		else {
			log_error(&tok->loginfo, "unexpected storage class specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if (storage && (tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_STATIC)) {
		if (*storage == TMPSTO_NONE) *storage = TMPSTO_STATIC;
		else if (*storage == TMPSTO_TLS) *storage = TMPSTO_TLS_STATIC;
		else {
			log_error(&tok->loginfo, "unexpected storage class specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if (storage && (tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_THREAD_LOCAL)) {
		if (*storage == TMPSTO_NONE) *storage = TMPSTO_TLS;
		else if (*storage == TMPSTO_EXTERN) *storage = TMPSTO_TLS_EXTERN;
		else if (*storage == TMPSTO_STATIC) *storage = TMPSTO_TLS_STATIC;
		else {
			log_error(&tok->loginfo, "unexpected storage class specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if (storage && (tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_TYPEDEF)) {
		if (*storage == TMPSTO_NONE) *storage = TMPSTO_TYPEDEF;
		else {
			log_error(&tok->loginfo, "unexpected storage class specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	}
	
	// Function specifier
	if (fspec && (tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_INLINE)) {
		*fspec |= FSPEC_INLINE;
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if (fspec && (tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_NORETURN)) {
		*fspec |= FSPEC_NORETURN;
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	}
	
	// Qualifier
	if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_ATOMIC)) {
		// Empty destructor
		*tok = proc_next_token(prep);
		if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_LPAREN)) {
			if (*spec != SPEC_NONE) {
				log_error(&tok->loginfo, "unexpected symbol '('\n");
				// Empty destructor
				goto failed;
			}
			*spec = SPEC_TYPE;
			if (!parse_type_name(target, struct_map, type_map, enum_map, builtins, const_map, type_set, prep, tok, SYM_RPAREN, &typ)) {
				goto failed;
			}
			if (!typ->is_validated) {
				log_internal(&tok->loginfo, "_Atomic() type is not validated\n");
				type_del(typ);
				// Empty destructor
				goto failed;
			}
			if ((typ->typ == TYPE_ARRAY) || (typ->typ == TYPE_FUNCTION) || typ->is_atomic || typ->is_const || typ->is_restrict || typ->is_volatile) {
				log_error(&tok->loginfo, "the type name in an _Atomic() may not be an array, a function, atomic, or qualified\n");
				type_del(typ);
				// Empty destructor
				goto failed;
			}
			// Empty destructor
			*tok = proc_next_token(prep);
			goto parse_cur_token_decl;
		} else {
			typ->is_atomic = 1;
			goto parse_cur_token_decl;
		}
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_CONST)) {
		typ->is_const = 1;
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_RESTRICT)) {
		typ->is_restrict = 1;
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_VOLATILE)) {
		typ->is_volatile = 1;
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	}
	
	// Specifier
#define SPEC(bt,post) \
		if (*spec == SPEC_NONE) {                                                                              \
			*spec = SPEC_BUILTIN;                                                                              \
			typ->typ = TYPE_BUILTIN;                                                                           \
			typ->val.builtin = BTT_ ## bt;                                                                     \
			post                                                                                               \
		} else {                                                                                               \
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]); \
			goto failed;                                                                                       \
		}                                                                                                      \
		*tok = proc_next_token(prep);                                                                          \
		goto parse_cur_token_decl;
#define SPEC_SIGNED(bt, allow_int) \
		if ((*spec == SPEC_NONE) || (allow_int && (*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_INT))) {                    \
			*spec = (allow_int && (*spec == SPEC_NONE)) ? SPEC_BUILTIN_NOINT : SPEC_BUILTIN;                                      \
			typ->typ = TYPE_BUILTIN;                                                                                              \
			typ->val.builtin = BTT_ ## bt;                                                                                        \
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (allow_int && (*spec == SPEC_BUILTIN))) && (typ->val.builtin == BTT_SINT)) { \
			*spec = allow_int ? *spec : SPEC_BUILTIN;                                                                             \
			typ->val.builtin = BTT_S ## bt;                                                                                       \
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (allow_int && (*spec == SPEC_BUILTIN))) && (typ->val.builtin == BTT_UINT)) { \
			*spec = allow_int ? *spec : SPEC_BUILTIN;                                                                             \
			typ->val.builtin = BTT_U ## bt;                                                                                       \
		} else {                                                                                                                  \
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);                    \
			goto failed;                                                                                                          \
		}                                                                                                                         \
		*tok = proc_next_token(prep);                                                                                             \
		goto parse_cur_token_decl;
	if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_BOOL)) {
		SPEC(BOOL,)
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_COMPLEX)) {
		if (*spec == SPEC_NONE) {
			*spec = SPEC_COMPLEX;
		} else if ((*spec == SPEC_BUILTIN_NOINT) && (typ->val.builtin == BTT_LONG)) {
			*spec = SPEC_LONGCOMPLEX;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_FLOAT)) {
			typ->val.builtin = BTT_CFLOAT;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_FLOAT128)) {
			typ->val.builtin = BTT_CFLOAT128;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_DOUBLE)) {
			typ->val.builtin = BTT_CDOUBLE;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_LONGDOUBLE)) {
			typ->val.builtin = BTT_CLONGDOUBLE;
		} else {
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_IMAGINARY)) {
		if (*spec == SPEC_NONE) {
			*spec = SPEC_IMAGINARY;
		} else if ((*spec == SPEC_BUILTIN_NOINT) && (typ->val.builtin == BTT_LONG)) {
			*spec = SPEC_LONGIMAGINARY;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_FLOAT)) {
			typ->val.builtin = BTT_IFLOAT;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_FLOAT128)) {
			typ->val.builtin = BTT_IFLOAT128;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_DOUBLE)) {
			typ->val.builtin = BTT_IDOUBLE;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_LONGDOUBLE)) {
			typ->val.builtin = BTT_ILONGDOUBLE;
		} else {
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_DOUBLE)) {
		if (*spec == SPEC_NONE) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_DOUBLE;
		} else if (*spec == SPEC_COMPLEX) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_CDOUBLE;
		} else if (*spec == SPEC_IMAGINARY) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_IDOUBLE;
		} else if (*spec == SPEC_LONGCOMPLEX) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_CLONGDOUBLE;
		} else if (*spec == SPEC_LONGIMAGINARY) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_ILONGDOUBLE;
		} else if ((*spec == SPEC_BUILTIN_NOINT) && (typ->val.builtin == BTT_LONG)) {
			*spec = SPEC_BUILTIN;
			typ->val.builtin = BTT_LONGDOUBLE;
		} else {
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_FLOAT)) {
		if (*spec == SPEC_NONE) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_FLOAT;
		} else if (*spec == SPEC_COMPLEX) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_CFLOAT;
		} else if (*spec == SPEC_IMAGINARY) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_IFLOAT;
		} else {
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_FLOAT128)) {
		if (*spec == SPEC_NONE) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_FLOAT128;
		} else if (*spec == SPEC_COMPLEX) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_CFLOAT128;
		} else if (*spec == SPEC_IMAGINARY) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_IFLOAT128;
		} else {
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_CHAR)) {
		SPEC_SIGNED(CHAR, 0)
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_INT)) {
		if (*spec == SPEC_NONE) {
			*spec = SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_INT;
		} else if (*spec == SPEC_BUILTIN_NOINT) {
			*spec = SPEC_BUILTIN;
		} else {
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_INT128)) {
		SPEC_SIGNED(INT128, 0)
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_LONG)) {
		if ((*spec == SPEC_NONE) || ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_INT))) {
			*spec = (*spec == SPEC_NONE) ? SPEC_BUILTIN_NOINT : SPEC_BUILTIN;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_LONG;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_SINT)) {
			typ->val.builtin = BTT_SLONG;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_UINT)) {
			typ->val.builtin = BTT_ULONG;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_LONG)) {
			typ->val.builtin = BTT_LONGLONG;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_ULONG)) {
			typ->val.builtin = BTT_ULONGLONG;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_SLONG)) {
			typ->val.builtin = BTT_SLONGLONG;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_DOUBLE)) {
			typ->val.builtin = BTT_LONGDOUBLE;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_CDOUBLE)) {
			typ->val.builtin = BTT_CLONGDOUBLE;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_IDOUBLE)) {
			typ->val.builtin = BTT_ILONGDOUBLE;
		} else if (*spec == SPEC_COMPLEX) {
			*spec = SPEC_LONGCOMPLEX;
		} else if (*spec == SPEC_IMAGINARY) {
			*spec = SPEC_LONGIMAGINARY;
		} else {
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_SHORT)) {
		SPEC_SIGNED(SHORT, 1)
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_SIGNED)) {
		if (*spec == SPEC_NONE) {
			*spec = SPEC_BUILTIN_NOINT;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_SINT;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_CHAR)) {
			typ->val.builtin = BTT_SCHAR;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_INT)) {
			typ->val.builtin = BTT_SINT;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_LONG)) {
			typ->val.builtin = BTT_SLONG;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_LONGLONG)) {
			typ->val.builtin = BTT_SLONGLONG;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_SHORT)) {
			typ->val.builtin = BTT_SSHORT;
		} else {
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_UNSIGNED)) {
		if (*spec == SPEC_NONE) {
			*spec = SPEC_BUILTIN_NOINT;
			typ->typ = TYPE_BUILTIN;
			typ->val.builtin = BTT_UINT;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_CHAR)) {
			typ->val.builtin = BTT_UCHAR;
		} else if ((*spec == SPEC_BUILTIN) && (typ->val.builtin == BTT_INT)) {
			typ->val.builtin = BTT_UINT;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_LONG)) {
			typ->val.builtin = BTT_ULONG;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_LONGLONG)) {
			typ->val.builtin = BTT_ULONGLONG;
		} else if (((*spec == SPEC_BUILTIN_NOINT) || (*spec == SPEC_BUILTIN)) && (typ->val.builtin == BTT_SHORT)) {
			typ->val.builtin = BTT_USHORT;
		} else {
			log_error(&tok->loginfo, "unexpected type specifier '%s' in declaration\n", kw2str[tok->tokv.kw]);
			goto failed;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_VOID)) {
		SPEC(VOID, typ->is_incomplete = 1;)
	}
#undef SPEC
#undef SPEC_SIGNED
	if ((tok->tokt == PTOK_IDENT) && (*spec == SPEC_NONE)) {
		// The ident is the type-specifier
		khiter_t it = kh_get(type_map, type_map, string_content(tok->tokv.str));
		if (it == kh_end(type_map)) {
			log_error(&tok->loginfo, "invalid type '%s' (ident is not a typedef)\n", string_content(tok->tokv.str));
			string_del(tok->tokv.str);
			goto failed;
		} else {
			*spec = SPEC_TYPE;
			if (!type_copy_into(typ, kh_val(type_map, it))) {
				log_memory("failed to duplicate type infos\n");
				string_del(tok->tokv.str);
				goto failed;
			}
			string_del(tok->tokv.str);
			*tok = proc_next_token(prep);
			goto parse_cur_token_decl;
		}
	}
	if ((tok->tokt == PTOK_KEYWORD) && (*spec == SPEC_NONE) && ((tok->tokv.kw == KW_STRUCT) || (tok->tokv.kw == KW_UNION))) {
		int is_struct = tok->tokv.kw == KW_STRUCT;
		*spec = SPEC_TYPE;
		
		// Empty destructor
		*tok = proc_next_token(prep);
		string_t *tag;
		if (tok->tokt == PTOK_IDENT) {
			tag = tok->tokv.str;
			// Token moved
			*tok = proc_next_token(prep);
			
			khiter_t it; // Iterator into the struct_map
			int iret;
			it = kh_put(struct_map, struct_map, string_content(tag), &iret);
			if (iret < 0) {
				log_memory("failed to add structure to struct map\n");
				proc_token_del(tok);
				goto failed;
			} else if (iret == 0) {
				// Structure already declared or defined
				if (kh_val(struct_map, it)->is_struct != is_struct) {
					log_error(&tok->loginfo, "incoherent struct/union tagging of %s\n", string_content(tag));
					string_del(tag);
					proc_token_del(tok);
					goto failed;
				}
				++kh_val(struct_map, it)->nrefs;
				string_del(tag);
				tag = kh_val(struct_map, it)->tag;
			} else {
				kh_val(struct_map, it) = struct_new(is_struct, tag);
				if (!kh_val(struct_map, it)) {
					log_memory("failed to create new structure metadata structure\n");
					string_del(tag);
					proc_token_del(tok);
					goto failed;
				}
			}
			typ->typ = TYPE_STRUCT_UNION;
			typ->val.st = kh_val(struct_map, it);
			typ->is_incomplete = !typ->val.st->is_defined;
		} else {
			tag = NULL;
			typ->typ = TYPE_STRUCT_UNION;
			typ->val.st = struct_new(is_struct, NULL);
			if (!typ->val.st) {
				log_memory("failed to create new structure metadata structure\n");
				proc_token_del(tok);
				goto failed;
			}
			typ->is_incomplete = 1;
		}
		if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_LBRACKET)) {
			if (typ->val.st->is_defined) {
				log_TODO(&tok->loginfo, "struct redefinition (tag is %s)\n", typ->val.st->tag ? string_content(typ->val.st->tag) : "<no tag>");
				goto failed;
			}
			
			VECTOR(st_members) *members = vector_new(st_members);
			if (!members) {
				log_memory("failed to create a members vector\n");
				goto failed;
			}
			type_t *typ2 = type_new();
			if (!typ2) {
				log_memory("failed to create a type info structure\n");
				vector_del(st_members, members);
				goto failed;
			}
			*tok = proc_next_token(prep);
			while (!proc_token_isend(tok) && ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_RBRACKET))) {
				enum decl_spec spec2 = SPEC_NONE;
				if (!parse_declaration_specifier(target, struct_map, type_map, enum_map, builtins,
				                                 const_map, type_set, prep, tok, NULL, NULL, &spec2, typ2)) {
					vector_del(st_members, members);
					type_del(typ2);
					goto failed;
				}
				if (spec2 == SPEC_NONE) {
					// Empty destructor
					*tok = proc_next_token(prep);
					continue; // Declaration was an assert, typ2 is unchanged
				}
				typ2 = type_try_merge(typ2, type_set);
				if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_SEMICOLON)) {
					// A struct-declaration that does not declare an anonymous structure or anonymous union
					// shall contain a struct-declarator-list.
					if ((typ2->typ != TYPE_STRUCT_UNION) || typ2->val.st->tag) {
						log_error(&tok->loginfo, "missing struct-declarator-list\n");
						vector_del(st_members, members);
						type_del(typ2);
						goto failed;
					}
					if (!vector_push(st_members, members, ((st_member_t){.name = NULL, .typ = typ2, .is_bitfield = 0}))) {
						log_memory("failed to add anonymous structure member\n");
						vector_del(st_members, members);
						type_del(typ2);
						// Empty destructor
						goto failed;
					}
					typ2 = type_new();
					if (!typ2) {
						log_memory("failed to create a type info structure\n");
						vector_del(st_members, members);
						// Empty destructor
						goto failed;
					}
					// Empty destructor
					*tok = proc_next_token(prep);
					continue;
				}
				
				struct parse_declarator_dest_s dest2;
				dest2.structms.struct_map = struct_map;
				dest2.structms.type_map = type_map;
				dest2.structms.enum_map = enum_map;
				dest2.structms.type_set = type_set;
				dest2.structms.builtins = builtins;
				dest2.structms.const_map = const_map;
				dest2.structms.dest = members;
				if (!parse_declarator(target, &dest2, prep, tok, TMPSTO_NONE, FSPEC_NONE, typ2, 0, 1, 1, 1)) {
					log_error(&tok->loginfo, "error parsing struct-declarator-list\n");
					vector_del(st_members, members);
					type_del(typ2);
					// Token is deleted
					goto failed;
				}
				type_del(typ2);
				if ((tok->tokt != PTOK_SYM) && (tok->tokv.sym != SYM_SEMICOLON)) {
					log_error(&tok->loginfo, "error parsing struct-declarator-list (invalid next token)\n");
					vector_del(st_members, members);
					proc_token_del(tok);
					goto failed;
				}
				typ2 = type_new();
				if (!typ2) {
					log_memory("failed to create a type info structure\n");
					vector_del(st_members, members);
					// Empty destructor
					goto failed;
				}
				*tok = proc_next_token(prep);
			}
			type_del(typ2);
			if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_RBRACKET)) {
				log_error(&tok->loginfo, "error parsing struct-declarator-list (invalid next token)\n");
				vector_del(st_members, members);
				proc_token_del(tok);
				goto failed;
			}
			
			typ->is_incomplete = 0;
			typ->is_validated = 0;
			if (tag) {
				khiter_t it = kh_get(type_set, type_set, typ);
				if (it != kh_end(type_set)) {
					kh_key(type_set, it)->is_incomplete = 0;
					kh_key(type_set, it)->is_validated = 0;
				}
			}
			typ->val.st->has_incomplete = 0; // Filled by the validate_type step
			typ->val.st->nmembers = vector_size(st_members, members);
			typ->val.st->members = vector_steal(st_members, members);
			typ->val.st->is_defined = 1;
			if (is_gst
			       && typ->val.st->tag
			       && (string_len(typ->val.st->tag) >= 7)
			       && !strncmp(string_content(typ->val.st->tag), "_G", 2)
			       && !strcmp(string_content(typ->val.st->tag) + string_len(typ->val.st->tag) - 5, "Class")) {
				typ->val.st->is_simple = 1;
			}
			*tok = proc_next_token(prep);
			goto parse_cur_token_decl;
		} else {
			if (!tag) {
				log_error(&tok->loginfo, "invalid structure declaration: missing tag and/or definition\n");
				proc_token_del(tok);
				goto failed;
			}
			goto parse_cur_token_decl;
		}
	}
	if ((tok->tokt == PTOK_KEYWORD) && (*spec == SPEC_NONE) && (tok->tokv.kw == KW_ENUM)) {
		*spec = SPEC_TYPE;
		
		// Empty destructor
		*tok = proc_next_token(prep);
		string_t *tag = NULL;
		if (tok->tokt == PTOK_IDENT) {
			tag = tok->tokv.str;
			*tok = proc_next_token(prep);
		}
		if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_LBRACKET)) {
			if (!tag) {
				log_error(&tok->loginfo, "unexpected token after keyword 'enum'\n");
				proc_token_del(tok);
				goto failed;
			}
			khiter_t it = kh_get(type_map, enum_map, string_content(tag));
			if (it == kh_end(enum_map)) {
				log_error(&tok->loginfo, "enumeration %s has not been defined yet\n", string_content(tag)); // TODO?
				string_del(tag);
				proc_token_del(tok);
				goto failed;
			}
			if (!type_copy_into(typ, kh_val(enum_map, it))) {
				log_memory("failed to duplicate enum type infos\n");
				string_del(tag);
				proc_token_del(tok);
				goto failed;
			}
			string_del(tag);
			goto parse_cur_token_decl;
		}
		// We are defining the enum
		// Try in order:
		//  If all values are 0 <= . <= UINT32_MAX, BTT_U32
		//  If any value is negative and all values are <= INT32_MAX, BTT_S32
		//  If any value is negative and all values are <= INT64_MAX, BTT_S64
		//  If all values are 0 <= . <= UINT64_MAX, BTT_U64
		//  Otherwise, error
		// By default, BTT_U32* and the constant is an NCT_INT32
		// *The AMD ABI says this should rather be BTT_INT (or BTT_S32)
		// Note that BTT_S32 only when has_neg is true
		int has_neg = 0, not_in_i32 = 0;
		enum type_builtin_e btt = BTT_U32;
		num_constant_t cst = { .typ = NCT_INT32, .val.i32 = -1 };
		// Empty destructor
		*tok = proc_next_token(prep);
		if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RBRACKET)) {
			log_error(&tok->loginfo, "expected enum constant name before '}'\n");
			proc_token_del(tok);
			goto failed;
		}
		int iret;
		while ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_RBRACKET)) {
			if (tok->tokt != PTOK_IDENT) {
				log_error(&tok->loginfo, "unexpected token in enumeration definition\n");
				if (tag) string_del(tag);
				proc_token_del(tok);
				goto failed;
			}
			char *ident = string_steal(tok->tokv.str);
			*tok = proc_next_token(prep);
			while ((tok->tokt == PTOK_IDENT) && (!strcmp(string_content(tok->tokv.str), "__attribute__") || !strcmp(string_content(tok->tokv.str), "__attribute"))) {
				// Attribute
				string_del(tok->tokv.str);
				*tok = proc_next_token(prep);
				if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_LPAREN)) {
					log_error(&tok->loginfo, "unexpected token, expected '('\n");
					free(ident);
					if (tag) string_del(tag);
					proc_token_del(tok);
					goto failed;
				}
				// Empty destructor
				*tok = proc_next_token(prep);
				if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_LPAREN)) {
					log_error(&tok->loginfo, "unexpected token, expected '('\n");
					free(ident);
					if (tag) string_del(tag);
					// Empty destructor
					goto failed;
				}
				// Empty destructor
				*tok = proc_next_token(prep);
			enum_attribute_list:
				if ((tok->tokt == PTOK_IDENT) && TOKEN_MATCH_ATTR(deprecated)) {
					log_warning(&tok->loginfo, "ignoring enum constant attribute 'deprecated'\n");
					string_del(tok->tokv.str);
					*tok = proc_next_token(prep);
					if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_COMMA)) {
						// Empty destructor
						*tok = proc_next_token(prep);
						goto enum_attribute_list;
					} else goto enum_attribute_list_end;
				}
				if ((tok->tokt == PTOK_IDENT) && TOKEN_MATCH_ATTR(unavailable)) {
					log_warning(&tok->loginfo, "ignoring enum constant attribute 'unavailable'\n");
					string_del(tok->tokv.str);
					*tok = proc_next_token(prep);
					if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_COMMA)) {
						// Empty destructor
						*tok = proc_next_token(prep);
						goto enum_attribute_list;
					} else goto enum_attribute_list_end;
				}
			enum_attribute_list_end:
				if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_RPAREN)) {
					log_error(&tok->loginfo, "unexpected token, expected ident or ')'\n");
					free(ident);
					if (tag) string_del(tag);
					// Empty destructor
					goto failed;
				}
				// Empty destructor
				*tok = proc_next_token(prep);
				if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_RPAREN)) {
					log_error(&tok->loginfo, "unexpected token, expected ')'\n");
					free(ident);
					if (tag) string_del(tag);
					// Empty destructor
					goto failed;
				}
				// Empty destructor
				*tok = proc_next_token(prep);
			}
			khiter_t it = kh_put(const_map, const_map, ident, &iret);
			if (iret < 0) {
				log_memory("failed to add constant %s to the constants map\n", ident);
				free(ident);
				if (tag) string_del(tag);
				proc_token_del(tok);
				goto failed;
			} else if (iret == 0) {
				log_error(&tok->loginfo, "constant %s is already defined\n", ident);
				free(ident);
				if (tag) string_del(tag);
				proc_token_del(tok);
				goto failed;
			}
			if ((tok->tokt == PTOK_SYM) && ((tok->tokv.sym == SYM_COMMA) || tok->tokv.sym == SYM_RBRACKET)) {
				switch (cst.typ) {
				case NCT_INT32:
					if (cst.val.i32 == INT32_MAX) {
						cst.typ = NCT_UINT32;
						cst.val.u32 = (uint32_t)INT32_MAX + 1;
					} else ++cst.val.i32;
					break;
				case NCT_UINT32:
					if (cst.val.u32 == UINT32_MAX) {
						cst.typ = NCT_INT64;
						cst.val.i64 = (int64_t)UINT32_MAX + 1;
					} else ++cst.val.u32;
					break;
				case NCT_INT64:
					if (cst.val.i64 == INT64_MAX) {
						cst.typ = NCT_UINT64;
						cst.val.u64 = (uint64_t)INT64_MAX + 1;
					} else ++cst.val.i64;
					break;
				case NCT_UINT64:
					if (cst.val.u64 == UINT64_MAX) {
						log_error(&tok->loginfo, "enum constant is too big\n");
						if (tag) string_del(tag);
						proc_token_del(tok);
						goto failed;
					} else ++cst.val.u64;
					break;
				case NCT_FLOAT:
				case NCT_DOUBLE:
				case NCT_LDOUBLE:
				default:
					log_internal(&tok->loginfo, "enum constant is a float/double/ldouble\n");
					if (tag) string_del(tag);
					proc_token_del(tok);
					goto failed;
				}
			} else if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_EQ)) {
				// Empty destructor
				*tok = proc_next_token(prep);
				expr_t *e = parse_expression(target, struct_map, type_map, enum_map, builtins, const_map, type_set, prep, tok, 14);
				if (!e) {
					goto failed;
				}
				if ((tok->tokt != PTOK_SYM) || ((tok->tokv.sym != SYM_COMMA) && (tok->tokv.sym != SYM_RBRACKET))) {
					log_error(&tok->loginfo, "unexpected token during enumeration declaration\n");
					expr_del(e);
					if (tag) string_del(tag);
					proc_token_del(tok);
					goto failed;
				}
				if (!eval_expression(&tok->loginfo, target, e, const_map, &cst, 1)) {
					expr_del(e);
					if (tag) string_del(tag);
					// Empty destructor
					goto failed;
				}
				expr_del(e);
			}
			switch (cst.typ) {
			case NCT_INT32:
				if (cst.val.i32 < 0) {
					has_neg = 1;
					if (btt == BTT_U64) {
						log_error(&tok->loginfo, "enum constant is too big\n");
						if (tag) string_del(tag);
						// Empty destructor
						goto failed;
					}
					btt = not_in_i32 ? BTT_S64 : BTT_S32;
				}
				break;
			case NCT_UINT32:
				if (cst.val.u32 > (uint32_t)INT32_MAX) {
					not_in_i32 = 1;
					btt = has_neg ? BTT_S64 : BTT_U32;
				}
				break;
			case NCT_INT64:
				if (cst.val.i64 < 0) {
					has_neg = 1;
					if (cst.val.i64 < (int64_t)INT32_MIN) {
						not_in_i32 = 1;
					}
					if (btt == BTT_U64) {
						log_error(&tok->loginfo, "enum constant is too big\n");
						if (tag) string_del(tag);
						// Empty destructor
						goto failed;
					}
					btt =
						not_in_i32 ? BTT_S64 :
						BTT_S32;
				}
				if ((cst.val.i64 > (int64_t)INT32_MAX)) {
					not_in_i32 = 1;
					if (has_neg) btt = BTT_S64;
					else if (((btt == BTT_S32) || (btt == BTT_U32)) && (cst.val.i64 <= (int64_t)UINT32_MAX))
						btt = BTT_U32;
					else if (btt != BTT_U64) btt = BTT_S64;
				}
				break;
			case NCT_UINT64:
				if (cst.val.u64 > (uint32_t)INT32_MAX) {
					not_in_i32 = 1;
					if (has_neg && (cst.val.u64 > (uint64_t)INT64_MAX)) {
						log_error(&tok->loginfo, "enum constant is too big\n");
						if (tag) string_del(tag);
						// Empty destructor
						goto failed;
					}
					btt = (cst.val.u64 > (uint64_t)INT64_MAX) ? BTT_U64 : (has_neg || (cst.val.u64 > (uint64_t)UINT32_MAX)) ? BTT_S64 : BTT_U32;
				}
				break;
			case NCT_FLOAT:
			case NCT_DOUBLE:
			case NCT_LDOUBLE:
			default:
				log_error(&tok->loginfo, "invalid floating-point enumeration constant\n");
				if (tag) string_del(tag);
				// Empty destructor
				goto failed;
			}
			kh_val(const_map, it) = cst;
			if (tok->tokv.sym == SYM_COMMA) {
				*tok = proc_next_token(prep);
			}
		}
		if (tag) {
			char *ctag = string_steal(tag);
			khiter_t it = kh_put(type_map, enum_map, ctag, &iret);
			if (iret < 0) {
				log_memory("failed to add enumeration %s to the type map\n", ctag);
				free(ctag);
				// Empty destructor
				goto failed;
			} else if (iret == 0) {
				log_error(&tok->loginfo, "enumeration %s already exists\n", ctag);
				free(ctag);
				// Empty destructor
				goto failed;
			}
			type_t *new_typ = type_new();
			if (!new_typ) {
				log_memory("failed to create type info for enumeration %s\n", ctag);
				free(ctag);
				kh_del(type_map, enum_map, it);
				// Empty destructor
				goto failed;
			}
			typ->typ = new_typ->typ = TYPE_ENUM;
			typ->is_incomplete = new_typ->is_incomplete = 0;
			typ->val.typ = new_typ->val.typ = (*builtins)[btt];
			typ->val.typ->nrefs += 2;
			new_typ = type_try_merge(new_typ, type_set);
			validate_type(&tok->loginfo, target, new_typ); // Assume it returns 1
			kh_val(enum_map, it) = new_typ;
		} else {
			typ->typ = TYPE_ENUM;
			typ->is_incomplete = 0;
			typ->val.typ = (*builtins)[btt];
			++typ->val.typ->nrefs;
		}
		*tok = proc_next_token(prep);
		goto parse_cur_token_decl;
	}
	
	if ((*spec != SPEC_BUILTIN) && (*spec != SPEC_BUILTIN_NOINT) && (*spec != SPEC_TYPE)) goto invalid_token;
	if ((tok->tokt != PTOK_IDENT) && ((tok->tokt != PTOK_SYM) || ((tok->tokv.sym != SYM_COMMA) && (tok->tokv.sym != SYM_SEMICOLON)
	                                                              && (tok->tokv.sym != SYM_STAR) && (tok->tokv.sym != SYM_LSQBRACKET)
	                                                              && (tok->tokv.sym != SYM_LPAREN) && (tok->tokv.sym != SYM_RPAREN)
	                                                              && (tok->tokv.sym != SYM_COLON))))
		goto invalid_token;
	
	return 1;
	
invalid_token:
	log_error(&tok->loginfo, "unexpected token (parse_declaration_specifier)\n");
	proc_token_del(tok);
	
failed:
	return 0;
}

static int parse_declarator(machine_t *target, struct parse_declarator_dest_s *dest, preproc_t *prep, proc_token_t *tok, enum decl_storage storage,
      enum fun_spec fspec, type_t *base_type, int is_init, int is_list, int allow_decl, int allow_abstract) {
	int has_list = 0, has_ident = 0;
	// TODO: allow_abstract and 'direct-abstract-declarator(opt) ( parameter-type-list(opt) )'
	
	_Bool array_atomic = 0, array_const = 0, array_restrict = 0, array_static = 0, array_volatile = 0;
	string_t *cur_ident = NULL;
	type_t *typ = base_type; ++typ->nrefs;
	type_t *cur_bottom = NULL;
	VECTOR(size_t) *nptr_stack = vector_new_cap(size_t, 1);
	if (!nptr_stack) {
		log_memory("failed to allocate nptr stack\n");
		proc_token_del(tok);
		goto failed0;
	}
	vector_push(size_t, nptr_stack, 0); // Always succeed (size < cap)
	while (1) {
		switch (tok->tokt) {
		case PTOK_IDENT:
			if (!has_ident && allow_decl) {
				cur_ident = tok->tokv.str;
				has_ident = 1;
				*tok = proc_next_token(prep);
				break;
			} else {
				log_error(&tok->loginfo, "unexpected identifier\n");
				proc_token_del(tok);
				goto failed;
			}
		case PTOK_SYM:
			if (tok->tokv.sym == SYM_LPAREN) {
				if (has_ident) {
					type_t *new_typ;
					
					VECTOR(types) *args = vector_new(types);
					if (!args) {
						log_memory("failed to create new type (function argument)\n");
						// Empty destructor
						goto failed;
					}
					int has_varargs = 0;
					*tok = proc_next_token(prep);
					if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_VOID)) {
						// Empty destructor
						*tok = proc_next_token(prep);
						if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
							vector_del(types, args);
							new_typ = type_new();
							if (!new_typ) {
								log_memory("failed to create new function type\n");
								// Empty destructor
								goto failed;
							}
							new_typ->typ = TYPE_FUNCTION;
							new_typ->val.fun.has_varargs = 0;
							new_typ->val.fun.nargs = 0;
							new_typ->val.fun.args = NULL;
							// ret will be set later
							goto end_fun;
						}
						if (!proc_unget_token(prep, tok)) {
							log_memory("failed to unget processor token\n");
							// Empty destructor
							vector_del(types, args);
							goto failed;
						}
						tok->tokt = PTOK_KEYWORD;
						tok->tokv.kw = KW_VOID;
					} else if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
						goto no_arg;
					}
					while (1) {
						if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_VARIADIC)) {
							has_varargs = 1;
							// Empty destructor
							*tok = proc_next_token(prep);
							if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_RPAREN)) {
								log_error(&tok->loginfo, "invalid token after function variadic argument\n");
								proc_token_del(tok);
								vector_del(types, args);
								goto failed;
							}
							break;
						} else {
							type_t *typ2 = type_new();
							if (!typ2) {
								log_memory("failed to create new type (function argument)\n");
								// Empty destructor
								vector_del(types, args);
								goto failed;
							}
							enum decl_storage storage2 = TMPSTO_NONE;
							enum decl_spec spec2 = SPEC_NONE;
							if (!parse_declaration_specifier(target, PDECL_STRUCT_MAP, PDECL_TYPE_MAP, PDECL_ENUM_MAP, PDECL_BUILTINS,
							                                 PDECL_CONST_MAP, PDECL_TYPE_SET, prep, tok, &storage2, NULL, &spec2, typ2)) {
								// Token is deleted
								vector_del(types, args);
								type_del(typ2);
								goto failed;
							}
							if (spec2 == SPEC_NONE) {
								// _Static_assert declaration; empty destructor
								log_error(&tok->loginfo, "unexpected _Static_assert declaration\n");
								vector_del(types, args);
								type_del(typ2);
								goto failed;
							}
							if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
								// Unnamed argument
								if (typ2->typ == TYPE_ARRAY) {
									// Need to convert type to a pointer
									type_t *typ3 = type_new();
									if (!typ3) {
										log_memory("failed to allocate new type\n");
										type_del(typ2);
										// Empty destructor
										goto failed;
									}
									if (!type_copy_into(typ3, typ2)) {
										log_memory("failed to duplicate array type to temporary type\n");
										type_del(typ3);
										type_del(typ2);
										// Empty destructor
										goto failed;
									}
									type_del(typ2);
									typ3->typ = TYPE_PTR;
									typ3->val.typ = typ3->val.array.typ;
									typ2 = type_try_merge(typ3, PDECL_TYPE_SET);
								}
								if (!vector_push(types, args, typ2)) {
									log_memory("failed to add argument to argument vector\n");
									vector_del(types, args);
									type_del(typ2);
									// Empty destructor
									goto failed;
								}
								break;
							} else if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_COMMA)) {
								// Unnamed argument
								if (typ2->typ == TYPE_ARRAY) {
									// Need to convert type to a pointer
									type_t *typ3 = type_new();
									if (!typ3) {
										log_memory("failed to allocate new type\n");
										type_del(typ2);
										// Empty destructor
										goto failed;
									}
									if (!type_copy_into(typ3, typ2)) {
										log_memory("failed to duplicate array type to temporary type\n");
										type_del(typ3);
										type_del(typ2);
										// Empty destructor
										goto failed;
									}
									type_del(typ2);
									typ3->typ = TYPE_PTR;
									typ3->val.typ = typ3->val.array.typ;
									typ2 = type_try_merge(typ3, PDECL_TYPE_SET);
								}
								if (!vector_push(types, args, typ2)) {
									log_memory("failed to add argument to argument vector\n");
									vector_del(types, args);
									type_del(typ2);
									// Empty destructor
									goto failed;
								}
								// Empty destructor
								*tok = proc_next_token(prep);
								continue;
							}
							// FIXME: Storage specifiers are ignored most of the time?
							struct parse_declarator_dest_s dest2;
							dest2.argt.dest = NULL;
							dest2.argt.struct_map = PDECL_STRUCT_MAP;
							dest2.argt.type_map = PDECL_TYPE_MAP;
							dest2.argt.enum_map = PDECL_ENUM_MAP;
							dest2.argt.type_set = PDECL_TYPE_SET;
							dest2.argt.builtins = PDECL_BUILTINS;
							dest2.argt.const_map = PDECL_CONST_MAP;
							if (!parse_declarator(target, &dest2, prep, tok, TMPSTO_NONE, FSPEC_NONE, typ2, 0, 0, 1, 1)) {
								// Token is deleted
								vector_del(types, args);
								type_del(typ2);
								goto failed;
							}
							type_del(typ2);
							if (!dest2.argt.dest) {
								log_internal(&tok->loginfo, "argument type is NULL\n");
								vector_del(types, args);
								// Empty destructor
								goto failed;
							}
							if (!vector_push(types, args, dest2.argt.dest)) {
								log_memory("failed to add argument to argument vector\n");
								vector_del(types, args);
								type_del(dest2.argt.dest);
								// Empty destructor
								goto failed;
							}
							if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
								break;
							} else if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_COMMA)) {
								// Empty destructor
								*tok = proc_next_token(prep);
								continue;
							}
							log_error(&tok->loginfo, "invalid token after function argument\n");
							vector_del(types, args);
							proc_token_del(tok);
							goto failed;
						}
					}
				no_arg:
					new_typ = type_new();
					if (!new_typ) {
						log_memory("failed to create new function type\n");
						// Empty destructor
						goto failed;
					}
					new_typ->typ = TYPE_FUNCTION;
					new_typ->val.fun.has_varargs = has_varargs;
					new_typ->val.fun.nargs = vector_size(types, args) ? vector_size(types, args) : (size_t)-1;
					new_typ->val.fun.args = vector_steal(types, args);
					
				end_fun: // (void)
					if (cur_bottom) {
						// cur_bottom is a pointer, an array or a function
						// We have cur_bottom(old) --> below, we want cur_bottom(old) --> array=cur_bottom(new) --> below
						new_typ->val.fun.ret =
							(cur_bottom->typ == TYPE_PTR) ? cur_bottom->val.typ :
							(cur_bottom->typ == TYPE_ARRAY) ? cur_bottom->val.array.typ :
							cur_bottom->val.fun.ret;
						*((cur_bottom->typ == TYPE_PTR) ? &cur_bottom->val.typ :
						(cur_bottom->typ == TYPE_ARRAY) ? &cur_bottom->val.array.typ : &cur_bottom->val.fun.ret) = new_typ;
						cur_bottom = new_typ;
					} else {
						// We have top(old), we want array=top(new)=cur_bottom(new) --> top(old)
						new_typ->val.fun.ret = typ;
						cur_bottom = typ = new_typ;
					}
					*tok = proc_next_token(prep);
					break;
				} else {
					if (!vector_push(size_t, nptr_stack, 0)) {
						log_memory("Failed to push 0 to the nptr stack for opening a parenthesis\n");
						// Empty destructor
						goto failed;
					}
					*tok = proc_next_token(prep);
					break;
				}
			} else if (tok->tokv.sym == SYM_STAR) {
				if (has_ident) {
					log_error(&tok->loginfo, "invalid token '*' after identifier in declaration\n");
					// Empty destructor
					goto failed;
				} else {
					type_t *new_typ = type_new_ptr(typ);
					if (!new_typ) {
						log_memory("failed to create a new pointer type info structure\n");
						// Empty destructor
						goto failed;
					}
					typ = new_typ;
					++vector_last(size_t, nptr_stack);
					*tok = proc_next_token(prep);
					break;
				}
			} else if (tok->tokv.sym == SYM_LSQBRACKET) {
				if (!has_ident) {
					if (!allow_abstract) {
						log_error(&tok->loginfo, "invalid token '[' before identifier in declaration\n");
						// Empty destructor
						goto failed;
					}
					has_ident = 1;
				}
				if (array_atomic || array_const || array_restrict || array_static || array_volatile) {
					log_error(&tok->loginfo, "invalid array after array with type qualifier(s) and/or 'static'\n");
					// Empty destructor
					goto failed;
				}
				// Empty destructor
				*tok = proc_next_token(prep);
				// From the standard:
				//   direct-declarator [ type-qualifier-list(opt) assignment-expression(opt) ]
				//   direct-declarator [ static type-qualifier-list(opt) assignment-expression ]
				//   direct-declarator [ type-qualifier-list static assignment-expression ]
				//   direct-declarator [ type-qualifier-list(opt) * ]
				//  The optional type qualifiers and the keyword static shall appear only in a
				//  declaration of a function parameter with an array type, and then only in the outermost
				//  array type derivation.
				size_t nelems; _Bool is_incomplete;
				while (1) {
#define DO_CHECKS \
	if (is_init || is_list || !allow_decl || (cur_bottom && (typ->typ != TYPE_ARRAY))) {                                    \
		log_error(&tok->loginfo, "type qualifiers and 'static' may only appear in function argument array declarations\n"); \
		/* Empty destructor */                                                                                              \
		goto failed;                                                                                                        \
	}
					if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_ATOMIC)) {
						DO_CHECKS
						array_atomic = 1;
						*tok = proc_next_token(prep);
					} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_CONST)) {
						DO_CHECKS
						array_const = 1;
						*tok = proc_next_token(prep);
					} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_RESTRICT)) {
						DO_CHECKS
						array_restrict = 1;
						*tok = proc_next_token(prep);
					} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_STATIC)) {
						DO_CHECKS
						array_static = 1;
						*tok = proc_next_token(prep);
					} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_VOLATILE)) {
						DO_CHECKS
						array_volatile = 1;
						*tok = proc_next_token(prep);
					} else break;
#undef DO_CHECKS
				}
				if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RSQBRACKET)) {
					if (array_static) {
						// Missing expression
						log_error(&tok->loginfo, "unexpected token ']' in static length array declaration\n");
						// Empty destructor
						goto failed;
					}
					// Incomplete VLA
					nelems = (size_t)-1;
					is_incomplete = 1;
				} else if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_STAR)) {
					if (array_static) {
						// Missing expression
						log_error(&tok->loginfo, "unexpected token '*' in static length array declaration\n");
						// Empty destructor
						goto failed;
					}
					// Complete VLA, expecting a ']'
					nelems = (size_t)-1;
					is_incomplete = 0;
					// Empty destructor
					*tok = proc_next_token(prep);
					if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_RSQBRACKET)) {
						// TODO: ...[*expr]
						log_error(&tok->loginfo, "unexpected token during variable length array declaration\n");
						proc_token_del(tok);
						goto failed;
					}
				} else {
					// Constant expression, followed by ']'
					is_incomplete = 0;
					expr_t *e = parse_expression(target, PDECL_STRUCT_MAP, PDECL_TYPE_MAP, PDECL_ENUM_MAP, PDECL_BUILTINS,
					                             PDECL_CONST_MAP, PDECL_TYPE_SET, prep, tok, 15);
					if (!e) {
						goto failed;
					}
					if ((tok->tokt != PTOK_SYM) || (tok->tokv.sym != SYM_RSQBRACKET)) {
						log_error(&tok->loginfo, "unexpected token during array declaration\n");
						expr_del(e);
						proc_token_del(tok);
						goto failed;
					}
					num_constant_t cst;
					if (eval_expression(&tok->loginfo, target, e, PDECL_CONST_MAP, &cst, is_init || is_list || !allow_decl)) {
						expr_del(e);
						int is_neg;
						switch (cst.typ) {
						case NCT_FLOAT:   is_neg = cst.val.f < 0; nelems = (size_t)cst.val.f; break;
						case NCT_DOUBLE:  is_neg = cst.val.d < 0; nelems = (size_t)cst.val.d; break;
						case NCT_LDOUBLE: is_neg = cst.val.l < 0; nelems = (size_t)cst.val.l; break;
						case NCT_INT32:   is_neg = cst.val.i32 < 0; nelems = (size_t)cst.val.i32; break;
						case NCT_UINT32:  is_neg = 0; nelems = (size_t)cst.val.u32; break;
						case NCT_INT64:   is_neg = cst.val.i64 < 0; nelems = (size_t)cst.val.i64; break;
						case NCT_UINT64:  is_neg = 0; nelems = (size_t)cst.val.u64; break;
						default: is_neg = 1;
						}
						if (is_neg) {
							log_error(&tok->loginfo, "the size of an array must be nonnegative");
							// Empty destructor
							goto failed;
						}
					} else {
						expr_del(e);
						// Treated as '*' as function argument (TODO: as anything else)
						if (is_init || is_list || !allow_decl) {
							// Empty destructor
							goto failed;
						}
						nelems = (size_t)-1;
					}
				}
				// Token is ']'
				
				if (cur_bottom) {
					type_t *tmp = type_new();
					if (!tmp) {
						log_memory("failed to create new array type info\n");
						// Empty destructor
						goto failed;
					}
					tmp->typ = TYPE_ARRAY;
					tmp->val.array.array_sz = nelems;
					tmp->is_incomplete = is_incomplete;
					// cur_bottom is a pointer, an array or a function
					// We have cur_bottom(old) --> below, we want cur_bottom(old) --> array=cur_bottom(new) --> below
					tmp->val.array.typ =
						(cur_bottom->typ == TYPE_PTR) ? cur_bottom->val.typ :
						(cur_bottom->typ == TYPE_ARRAY) ? cur_bottom->val.array.typ :
						cur_bottom->val.fun.ret;
					*((cur_bottom->typ == TYPE_PTR) ? &cur_bottom->val.typ :
					  (cur_bottom->typ == TYPE_ARRAY) ? &cur_bottom->val.array.typ : &cur_bottom->val.fun.ret) = tmp;
					cur_bottom = tmp;
				} else {
					type_t *new_typ = type_new();
					if (!new_typ) {
						log_memory("failed to create new array type info\n");
						// Empty destructor
						goto failed;
					}
					new_typ->typ = TYPE_ARRAY;
					new_typ->val.array.array_sz = nelems;
					new_typ->is_incomplete = is_incomplete;
					// We have top(old), we want array=top(new)=cur_bottom(new) --> top(old)
					new_typ->val.array.typ = typ;
					cur_bottom = typ = new_typ;
				}
				*tok = proc_next_token(prep);
				break;
			} else if (tok->tokv.sym == SYM_RPAREN) {
				if (!has_ident) {
					if (!allow_abstract) {
						log_error(&tok->loginfo, "invalid token ')' before identifier in declaration\n");
						// Empty destructor
						goto failed;
					}
					has_ident = 1;
				}
				if (vector_size(size_t, nptr_stack) == 1) {
					if (!is_init && !is_list) goto rparen_ok_ret;
					log_error(&tok->loginfo, "closing unopened parenthesis in declaration\n");
					// Empty destructor
					goto failed;
				}
				size_t ndecr;
				if (cur_bottom) {
					ndecr = vector_last(size_t, nptr_stack);
				} else {
					if (vector_last(size_t, nptr_stack)) {
						cur_bottom = typ;
						ndecr = vector_last(size_t, nptr_stack) - 1;
					} else ndecr = 0;
				}
				for (size_t i = 0; i < ndecr; ++i) {
					cur_bottom =
						(cur_bottom->typ == TYPE_PTR) ? cur_bottom->val.typ :
						(cur_bottom->typ == TYPE_ARRAY) ? cur_bottom->val.array.typ :
						cur_bottom->val.fun.ret;
				}
				vector_pop(size_t, nptr_stack);
				*tok = proc_next_token(prep);
				break;
			} else if ((is_init && (tok->tokv.sym == SYM_EQ))
			        || (tok->tokv.sym == SYM_COMMA)
			        || (tok->tokv.sym == SYM_SEMICOLON)
			        || (is_init && (tok->tokv.sym == SYM_LBRACKET))) {
			rparen_ok_ret: // Last function argument
				if (!allow_abstract && !has_ident) {
					log_error(&tok->loginfo, "invalid symbol '%s' before identifier\n", sym2str[tok->tokv.sym]);
					// Empty destructor
					goto failed;
				}
				if (vector_size(size_t, nptr_stack) != 1) {
					log_error(&tok->loginfo, "invalid symbol '%s' (missing parenthesis?)\n", sym2str[tok->tokv.sym]);
					// Empty destructor
					goto failed;
				}
				
				// Try to free some redundant types
				typ = type_try_merge(typ, PDECL_TYPE_SET);
				
				int validation = validate_storage_type(&tok->loginfo, target, storage, typ, tok->tokv.sym);
				if (!validation) {
					// Empty destructor
					goto failed;
				}
				
				if (validation == VALIDATION_FUN) {
					// Function definition; tok is '{'
					if (!is_init || !is_list || has_list) {
						// We are not at the top-level or we have an initialization list
						log_error(&tok->loginfo, "invalid function definition\n");
						// Empty destructor
						goto failed;
					}
					// Note that here, dest is a file_t
					// No argument in function definition means that the function takes no argument,
					//  whereas no argument in function declaration means the function takes an unspecified number of arguments
					if (typ->val.fun.nargs == (size_t)-1) typ->val.fun.nargs = 0;
					
					declaration_t *decl = malloc(sizeof *decl);
					if (!decl) {
						log_memory("failed to create new declaration\n");
						// Empty destructor
						goto failed;
					}
					decl->storage = tmpsto2sto[storage];
					decl->defined = 1;
					decl->typ = typ;
					
					int iret;
					char *cident = string_steal(cur_ident); cur_ident = NULL;
					khiter_t it = kh_put(decl_map, dest->f->decl_map, cident, &iret);
					if (iret < 0) {
						log_memory("failed to add function '%s' to the declaration map\n", cident);
						free(cident);
						free(decl);
						// Empty destructor
						goto failed;
					} else if (iret == 0) {
						free(decl);
						if (kh_val(dest->f->decl_map, it)->defined) {
							log_error(&tok->loginfo, "function '%s' is already in the declaration map\n", cident);
							free(cident);
							// Empty destructor
							goto failed;
						} else {
							free(cident);
							decl = kh_val(dest->f->decl_map, it);
						}
					}
					
					kh_val(dest->f->decl_map, it) = decl;
					
					// Skip the function body
					int nlbraces = 0;
					do {
						proc_token_del(tok);
						*tok = proc_next_token(prep);
						if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_LBRACKET)) ++nlbraces;
						else if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RBRACKET)) {
							if (nlbraces) --nlbraces;
							else goto success;
						}
					} while (!proc_token_isend(tok));
					log_error(&tok->loginfo, "unexpected token in function body\n");
					goto failed;
				}
				if ((fspec != FSPEC_NONE) && (typ->typ != TYPE_FUNCTION)) {
					log_error(&tok->loginfo, "unexpected function specifier\n");
					// Empty destructor
					goto failed;
				}
				
				declaration_t *decl = NULL;
				if (storage == TMPSTO_TYPEDEF) {
					if (!is_init || !is_list) {
						// We are not at the top-level (note that storage is set to NONE in function arguments)
						log_error(&tok->loginfo, "invalid function definition\n");
						// Empty destructor
						goto failed;
					}
					// Note that here, dest is a file_t
					int iret;
					char *cident = string_steal(cur_ident); cur_ident = NULL;
					khiter_t it = kh_put(type_map, dest->f->type_map, cident, &iret);
					if (iret < 0) {
						log_memory("failed to add '%s' to the type map\n", cident);
						free(cident);
						// Empty destructor
						goto failed;
					} else if (iret == 0) {
						if (!type_t_equal(typ, kh_val(dest->f->type_map, it))) {
							log_error(&tok->loginfo, "'%s' is already in the type map with a different type\n", cident);
							free(cident);
							// Empty destructor
							goto failed;
						}
						// We can safely ignore this since we have typedef-ed the same type
						free(cident);
						type_del(typ);
					} else {
						kh_val(dest->f->type_map, it) = typ;
					}
				} else {
					if (is_init && is_list) {
						decl = malloc(sizeof *decl);
						if (!decl) {
							log_memory("failed to create new declaration\n");
							// Empty destructor
							goto failed;
						}
						if ((typ->typ == TYPE_FUNCTION) && (storage == TMPSTO_NONE))
							storage = TMPSTO_EXTERN;
						decl->storage = tmpsto2sto[storage];
						decl->defined = 0;
						decl->typ = typ;
						
						int iret;
						char *cident = string_steal(cur_ident); cur_ident = NULL;
						khiter_t it = kh_put(decl_map, dest->f->decl_map, cident, &iret);
						if (iret < 0) {
							log_memory("failed to add '%s' to the declaration map\n", cident);
							free(cident);
							free(decl);
							// Empty destructor
							goto failed;
						} else if (iret == 0) {
							if (!type_t_equal(typ, kh_val(dest->f->decl_map, it)->typ)
							 || ((storage == TMPSTO_NONE) && (kh_val(dest->f->decl_map, it)->storage == STORAGE_NONE))) {
								log_error(&tok->loginfo, "'%s' is already in the declaration map\n", cident);
								free(cident);
								free(decl);
								// Empty destructor
								goto failed;
							} else {
								// OK, this is allowed
								free(cident);
								free(decl);
								type_del(typ);
							}
						} else {
							kh_val(dest->f->decl_map, it) = decl;
						}
					} else if (!is_init && !is_list) {
						if (allow_decl) {
							// Function argument
							if (typ->typ == TYPE_ARRAY) {
								// Convert to pointer
								if (typ == base_type) {
									typ = type_new();
									if (!typ) {
										log_memory("failed to allocate new type\n");
										type_del(base_type);
										// Empty destructor
										goto failed;
									}
									if (!type_copy_into(typ, base_type)) {
										log_memory("failed to allocate new type\n");
										// Empty destructor
										goto failed;
									}
								}
								typ->typ = TYPE_PTR;
								typ->val.typ = typ->val.array.typ;
								if (array_atomic)   typ->is_atomic   = 1;
								if (array_const)    typ->is_const    = 1;
								if (array_restrict) typ->is_restrict = 1;
								if (array_volatile) typ->is_volatile = 1;
								typ = type_try_merge(typ, PDECL_TYPE_SET);
							}
						}
						dest->argt.dest = typ;
						if (cur_ident) string_del(cur_ident);
						goto success;
					} else if (!is_init && is_list) {
						if (!vector_push(st_members, dest->structms.dest, ((st_member_t){.name = cur_ident, .typ = typ, .is_bitfield = 0}))) {
							log_memory("failed to add structure member %s\n", string_content(cur_ident));
							string_del(cur_ident);
							// Empty destructor
							goto failed;
						}
					} else {
						log_internal(&tok->loginfo, "unknown is_init/is_list combination %d%d\n", is_init, is_list);
						// Empty destructor
						goto failed;
					}
				}
				typ = base_type; ++typ->nrefs;
				if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_EQ)) {
					// Initialization
					if (!is_init) {
						log_error(&tok->loginfo, "unexpected initializer\n");
						goto failed;
					}
					if (decl) {
						if (decl->defined) {
							log_error(&tok->loginfo, "invalid declaration initializer: variable was already declared\n");
							goto failed;
						}
						decl->defined = 1;
					}
					*tok = proc_next_token(prep);
					if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_LBRACKET)) {
						// { ... }
						int nlbraces = 0;
						do {
							proc_token_del(tok);
							*tok = proc_next_token(prep);
							if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_LBRACKET)) ++nlbraces;
							else if ((tok->tokt == PTOK_SYM) && (tok->tokv.sym == SYM_RBRACKET)) {
								if (nlbraces) --nlbraces;
								else break;
							}
						} while (!proc_token_isend(tok));
						if (proc_token_isend(tok)) {
							log_error(&tok->loginfo, "unexpected token in declaration initializer\n");
							proc_token_del(tok);
							goto failed;
						}
						*tok = proc_next_token(prep);
					} else {
						expr_t *e = parse_expression(target, PDECL_STRUCT_MAP, PDECL_TYPE_MAP, PDECL_ENUM_MAP, PDECL_BUILTINS,
						                             PDECL_CONST_MAP, PDECL_TYPE_SET, prep, tok, 15);
						if (!e) {
							log_error(&tok->loginfo, "invalid declaration initializer\n");
							goto failed;
						}
						expr_del(e);
					}
					if ((tok->tokt != PTOK_SYM) || ((tok->tokv.sym != SYM_COMMA) && (tok->tokv.sym != SYM_SEMICOLON))) {
						log_error(&tok->loginfo, "unexpected token in declaration initializer\n");
						proc_token_del(tok);
						goto failed;
					}
					validation = (tok->tokv.sym == SYM_SEMICOLON) ? VALIDATION_LAST_DECL : VALIDATION_DECL;
				}
				if (validation == VALIDATION_LAST_DECL) {
					--typ->nrefs;
					goto success;
				} else {
					cur_ident = NULL; has_ident = 0;
					cur_bottom = NULL;
					vector_last(size_t, nptr_stack) = 0;
					*tok = proc_next_token(prep);
					break;
				}
			} else if (!is_init && is_list && (tok->tokv.sym == SYM_COLON)) {
				if (vector_size(size_t, nptr_stack) != 1) {
					log_error(&tok->loginfo, "invalid symbol '%s' (missing parenthesis?)\n", sym2str[tok->tokv.sym]);
					// Empty destructor
					goto failed;
				}
				// Try to free some redundant types
				typ = type_try_merge(typ, PDECL_TYPE_SET);
				
				// storage == TMPSTO_NONE
				*tok = proc_next_token(prep);
				expr_t *e = parse_expression(target, dest->structms.struct_map, dest->structms.type_map, dest->structms.enum_map,
				                             dest->structms.builtins, dest->structms.const_map, dest->structms.type_set,
				                             prep, tok, 14);
				if (!e) {
					goto failed;
				}
				if ((tok->tokt != PTOK_SYM) || ((tok->tokv.sym != SYM_COMMA) && (tok->tokv.sym != SYM_SEMICOLON))) {
					log_error(&tok->loginfo, "unexpected token in bitfield width\n");
					expr_del(e);
					proc_token_del(tok);
					goto failed;
				}
				num_constant_t eval;
				if (!eval_expression(&tok->loginfo, target, e, dest->structms.const_map, &eval, 1)) {
					expr_del(e);
					// Empty destructor
					goto failed;
				}
				expr_del(e);
				
				int validation = validate_storage_type(&tok->loginfo, target, storage, typ, tok->tokv.sym);
				if (!validation) {
					// Empty destructor
					goto failed;
				}
				
				if (validation == VALIDATION_FUN) {
					// We are not at the top-level or we have an initialization list
					// Should never happen
					log_error(&tok->loginfo, "invalid function definition in structure definition\n");
					// Empty destructor
					goto failed;
				}
				if ((fspec != FSPEC_NONE) && (typ->typ != TYPE_FUNCTION)) {
					log_error(&tok->loginfo, "unexpected function specifier\n");
					// Empty destructor
					goto failed;
				}
				
				size_t width;
				switch (eval.typ) {
				case NCT_INT32:
					if (eval.val.i32 < 0) {
						log_error(&tok->loginfo, "invalid negative bitfield width\n");
						goto failed;
					}
					width = (size_t)eval.val.i32;
					break;
				case NCT_UINT32:
					width = (size_t)eval.val.u32;
					break;
				case NCT_INT64:
					if (eval.val.i64 < 0) {
						log_error(&tok->loginfo, "invalid negative bitfield width\n");
						goto failed;
					}
					width = (size_t)eval.val.i64;
					break;
				case NCT_UINT64:
					width = (size_t)eval.val.u64;
					break;
				case NCT_FLOAT:
				case NCT_DOUBLE:
				case NCT_LDOUBLE:
				default:
					log_error(&tok->loginfo, "invalid non-integer bitfield width\n");
					goto failed;
				}
				
				if (!vector_push(st_members, dest->structms.dest,
				        ((st_member_t){.name = cur_ident, .typ = typ, .is_bitfield = 1, .bitfield_width = width}))) {
					log_memory("failed to add structure member %s\n", string_content(cur_ident));
					string_del(cur_ident);
					// Empty destructor
					goto failed;
				}
				if (validation == VALIDATION_LAST_DECL) goto success;
				else {
					cur_ident = NULL; has_ident = 0;
					typ = base_type; ++typ->nrefs;
					cur_bottom = NULL;
					vector_last(size_t, nptr_stack) = 0;
					*tok = proc_next_token(prep);
					break;
				}
			}
			/* FALLTHROUGH */
		case PTOK_KEYWORD:
			if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_ATOMIC)) {
				if (has_ident) {
					log_error(&tok->loginfo, "invalid keyword '_Atomic' after identifier\n");
					proc_token_print(tok);
					// Empty destructor
					goto failed;
				} else if (!vector_last(size_t, nptr_stack)) {
					log_error(&tok->loginfo, "invalid keyword '_Atomic' before symbol '*'\n");
					proc_token_print(tok);
					// Empty destructor
					goto failed;
				} else {
					typ->is_atomic = 1;
					*tok = proc_next_token(prep);
					break;
				}
			} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_CONST)) {
				if (has_ident) {
					log_error(&tok->loginfo, "invalid keyword 'const' after identifier\n");
					proc_token_print(tok);
					// Empty destructor
					goto failed;
				} else if (!vector_last(size_t, nptr_stack)) {
					log_error(&tok->loginfo, "invalid keyword 'const' before symbol '*'\n");
					proc_token_print(tok);
					// Empty destructor
					goto failed;
				} else {
					typ->is_const = 1;
					*tok = proc_next_token(prep);
					break;
				}
			} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_RESTRICT)) {
				if (has_ident) {
					log_error(&tok->loginfo, "invalid keyword 'restrict' after identifier\n");
					proc_token_print(tok);
					// Empty destructor
					goto failed;
				} else if (!vector_last(size_t, nptr_stack)) {
					log_error(&tok->loginfo, "invalid keyword 'restrict' before symbol '*'\n");
					proc_token_print(tok);
					// Empty destructor
					goto failed;
				} else {
					typ->is_restrict = 1;
					*tok = proc_next_token(prep);
					break;
				}
			} else if ((tok->tokt == PTOK_KEYWORD) && (tok->tokv.kw == KW_VOLATILE)) {
				if (has_ident) {
					log_error(&tok->loginfo, "invalid keyword 'volatile' after identifier\n");
					proc_token_print(tok);
					// Empty destructor
					goto failed;
				} else if (!vector_last(size_t, nptr_stack)) {
					log_error(&tok->loginfo, "invalid keyword 'volatile' before symbol '*'\n");
					proc_token_print(tok);
					// Empty destructor
					goto failed;
				} else {
					typ->is_volatile = 1;
					*tok = proc_next_token(prep);
					break;
				}
			}
			/* FALLTHROUGH */
		case PTOK_INVALID:
		case PTOK_NUM:
		case PTOK_STRING:
		case PTOK_PRAGMA:
		case PTOK_EOF:
			log_error(&tok->loginfo, "unexpected token (in parse_declarator %d%d%d%d)\n", is_init, is_list, allow_decl, allow_abstract);
			proc_token_del(tok);
			goto failed;
		}
	}
	
success:
	vector_del(size_t, nptr_stack);
	// typ has moved, we must not destroy it
	return 1;
failed:
	vector_del(size_t, nptr_stack);
failed0:
	if (cur_ident) string_del(cur_ident);
	type_del(typ);
	return 0;
}

static int finalize_file(machine_t *target, file_t *file) {
#define MARK_SIMPLE(sname) \
	it = kh_get(struct_map, file->struct_map, #sname);                            \
	if (it != kh_end(file->struct_map)) {                                         \
		kh_val(file->struct_map, it)->is_simple = 1;                              \
	} else {                                                                      \
		it = kh_get(type_map, file->type_map, #sname);                            \
		if (it != kh_end(file->type_map)) {                                       \
			type_t *typ2 = kh_val(file->type_map, it);                            \
			if (typ2->typ != TYPE_STRUCT_UNION) {                                 \
				log_error_nopos("invalid typedef " #sname ": not a structure\n"); \
				return 0;                                                         \
			}                                                                     \
			typ2->val.st->is_simple = 1;                                          \
		}                                                                         \
	}
#define SET_WEAK(converted) \
	validate_type(&(loginfo_t){0}, target, typ);                      \
	typ = type_try_merge(typ, file->type_set);                        \
	it = kh_put(conv_map, file->relaxed_type_conversion, typ, &iret); \
	if (iret < 0) {                                                   \
		log_memory("failed to add relaxed conversion to type map\n"); \
		type_del(typ);                                                \
		return 0;                                                     \
	} else if (iret == 0) {                                           \
		log_error_nopos("type already has a relaxed conversion\n");   \
		type_del(typ);                                                \
		return 0;                                                     \
	}                                                                 \
	kh_val(file->relaxed_type_conversion, it) = string_new_cstr(#converted);
#define SET_WEAK_PTR_TO(to_typ, converted) \
	it = kh_get(type_map, file->type_map, #to_typ);             \
	if (it != kh_end(file->type_map)) {                         \
		typ = type_new_ptr(kh_val(file->type_map, it));         \
		if (!typ) {                                             \
			log_memory("failed to create type " #to_typ "*\n"); \
			return 0;                                           \
		}                                                       \
		++kh_val(file->type_map, it)->nrefs;                    \
		SET_WEAK(converted)                                     \
	}
	
	type_t *typ;
	int iret;
	khiter_t it;
	// #pragma type_letters S FILE*
	SET_WEAK_PTR_TO(FILE, S)
	// #pragma type_letters b xcb_connection_t*
	if (target->size_long == 4) {
		SET_WEAK_PTR_TO(xcb_connection_t, n)
	} else {
                SET_WEAK_PTR_TO(xcb_connection_t, b)
	}
	if (target->size_long == 4) {
		// Only on x86, not on x86_64
		it = kh_get(type_map, file->type_map, "locale_t");
		if (it != kh_end(file->type_map)) {
			typ = kh_val(file->type_map, it);
			SET_WEAK(a)
		}
	}
	// #pragma mark_simple ...
	if (target->size_long == 8) {
		// Only on x86_64, not on x86
		MARK_SIMPLE(FTS)
		MARK_SIMPLE(FTS64)
		MARK_SIMPLE(glob_t)
		MARK_SIMPLE(glob64_t)
	}
#undef MARK_SIMPLE
#undef SET_WEAK
#undef SET_WEAK_PTR_TO
	return 1;
}

file_t *parse_file(machine_t *target, const char *filename, FILE *file) {
	char *dirname = strchr(filename, '/') ? strndup(filename, (size_t)(strrchr(filename, '/') - filename)) : NULL;
	preproc_t *prep = preproc_new_file(target, file, dirname, filename);
	if (!prep) {
		log_memory("failed to create the preproc structure\n");
		if (dirname) free(dirname);
		return NULL;
	}
	file_t *ret = file_new(target);
	if (!ret) {
		log_memory("failed to create the file structure\n");
		preproc_del(prep);
		return NULL;
	}
	
	type_t *typ = type_new();
	if (!typ) {
		log_memory("failed to create a type info structure\n");
		goto failed;
	}
	while (1) {
		proc_token_t tok = proc_next_token(prep);
		if (tok.tokt == PTOK_EOF) {
			goto success;
		} else if ((tok.tokt == PTOK_SYM) && (tok.tokv.sym == SYM_SEMICOLON)) {
			// Empty destructor
		} else if (tok.tokt == PTOK_PRAGMA) {
			switch (tok.tokv.pragma.typ) {
			case PRAGMA_ALLOW_INTS: {
				const char *typenames[] = {BTT_INT_EXTS};
				for (size_t i = 0; i < sizeof typenames / sizeof *typenames; ++i) {
					int iret;
					char *dup = strdup(typenames[i]);
					if (!dup) {
						log_memory("failed to create a type info structure\n");
						goto failed;
					}
					type_t *t = ret->builtins[BTT_START_INT_EXT + i];
					khiter_t it = kh_put(type_map, ret->type_map, dup, &iret);
					if (iret < 0) {
						log_memory("failed to add an intrinsic to the type map\n");
						goto failed;
					} else if (iret == 0) {
						if (!type_t_equal(t, kh_val(ret->type_map, it))) {
							log_error(&tok.loginfo, "%s is already defined\n", dup);
							free(dup);
							goto failed;
						}
						free(dup);
					} else {
						++t->nrefs;
						kh_val(ret->type_map, it) = t;
					}
				}
				break; }
			case PRAGMA_SIMPLE_SU: {
				string_t *sutag = tok.tokv.pragma.val;
				struct_t *su;
				khiter_t it = kh_get(struct_map, ret->struct_map, string_content(sutag));
				if (it != kh_end(ret->struct_map)) {
					su = kh_val(ret->struct_map, it);
				} else {
					it = kh_get(type_map, ret->type_map, string_content(sutag));
					if (it != kh_end(ret->struct_map)) {
						type_t *typ2 = kh_val(ret->type_map, it);
						if (typ2->typ != TYPE_STRUCT_UNION) {
							log_error(&tok.loginfo, "failed to find struct/union named %s\n", string_content(sutag));
							string_del(sutag);
							goto failed;
						}
						su = typ2->val.st;
					} else {
						log_error(&tok.loginfo, "failed to find struct/union named %s\n", string_content(sutag));
						string_del(sutag);
						goto failed;
					}
				}
				string_del(sutag);
				su->is_simple = 1;
				// Empty destructor
				break; }
			case PRAGMA_EXPLICIT_CONV: {
				string_t *converted = tok.tokv.pragma.val;
				type_t *typ2 = type_new();
				if (!typ2) {
					log_memory("failed to create new type info structure\n");
					string_del(converted);
					type_del(typ2);
					goto failed;
				}
				tok = proc_next_token(prep);
				if (!parse_type_name(target, ret->struct_map, ret->type_map, ret->enum_map, &ret->builtins, ret->const_map, ret->type_set,
				                     prep, &tok, SYM_SEMICOLON, &typ2)) {
					string_del(converted);
					goto failed;
				}
				int iret;
				khiter_t it = kh_put(conv_map, ret->relaxed_type_conversion, typ2, &iret);
				if (iret < 0) {
					log_memory("failed to add relaxed conversion to type map\n");
					string_del(converted);
					type_del(typ2);
					// Empty destructor
					goto failed;
				} else if (iret == 0) {
					log_error(&tok.loginfo, "type already has a relaxed conversion\n");
					string_del(converted);
					type_del(typ2);
					// Empty destructor
					goto failed;
				}
				string_trim(converted);
				kh_val(ret->relaxed_type_conversion, it) = converted;
				// Empty destructor
				break; }
			case PRAGMA_EXPLICIT_CONV_STRICT: {
				string_t *converted = tok.tokv.pragma.val;
				type_t *typ2 = type_new();
				if (!typ2) {
					log_memory("failed to create new type info structure\n");
					string_del(converted);
					type_del(typ2);
					goto failed;
				}
				tok = proc_next_token(prep);
				if (!parse_type_name(target, ret->struct_map, ret->type_map, ret->enum_map, &ret->builtins, ret->const_map, ret->type_set,
				                     prep, &tok, SYM_SEMICOLON, &typ2)) {
					string_del(converted);
					goto failed;
				}
				type_del(typ2); // typ2 is in the type set, so it is already used
				if (typ2->converted) {
					log_error(&tok.loginfo, "type already has a strict conversion\n");
					string_del(converted);
					// Empty destructor
					goto failed;
				}
				string_trim(converted);
				typ2->converted = converted;
				// Empty destructor
				break; }
			}
		} else if (proc_token_iserror(&tok)) {
			log_error(&tok.loginfo, "unexpected error token\n");
			proc_token_del(&tok);
			goto failed;
		} else {
			enum decl_storage storage = TMPSTO_NONE;
			enum fun_spec fspec = FSPEC_NONE;
			enum decl_spec spec = SPEC_NONE;
			if (!parse_declaration_specifier(target, ret->struct_map, ret->type_map, ret->enum_map, &ret->builtins, ret->const_map,
			                                 ret->type_set, prep, &tok, &storage, &fspec, &spec, typ)) {
				goto failed;
			}
			if (spec == SPEC_NONE) continue; // Declaration was an assert, typ is unchanged
			typ = type_try_merge(typ, ret->type_set);
			if ((tok.tokt != PTOK_SYM) || (tok.tokv.sym != SYM_SEMICOLON)) {
				if (!parse_declarator(target, &(struct parse_declarator_dest_s){.f = ret}, prep, &tok, storage, fspec, typ, 1, 1, 1, 0)) {
					goto failed;
				}
			} else {
				if (validate_storage_type(&tok.loginfo, target, storage, typ, tok.tokv.sym) != VALIDATION_LAST_DECL) {
					goto failed;
				}
				if (fspec != FSPEC_NONE) {
					log_error(&tok.loginfo, "unexpected function specifier\n");
					// Empty destructor
					goto failed;
				}
			}
			// Current token is ';' (or '}' for functions), ie. end of declaration
			type_del(typ);
			typ = type_new();
			if (!typ) {
				log_memory("failed to create a type info structure\n");
				goto failed;
			}
		}
	}
	
success:
	preproc_del(prep);
	type_del(typ);
	if (!finalize_file(target, ret)) {
		log_memory("failed to add builtin aliases\n");
		file_del(ret);
		return NULL;
	}
	return ret;
failed:
	preproc_del(prep);
	if (typ) type_del(typ);
	file_del(ret);
	return NULL;
}
