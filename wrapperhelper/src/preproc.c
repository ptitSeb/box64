// I think this file is too big for GCC to handle properly, there are curious false-positive analyzer warnings
//  that didn't appear before adding preproc_eval
#include "preproc.h"

#include <stdint.h>
#include <string.h>

#include "cstring.h"
#include "khash.h"
#include "machine.h"
#include "prepare.h"

//#define LOG_OPEN
//#define LOG_INCLUDE
//#define LOG_CLOSE

typedef struct mtoken_s {
	enum mtoken_e {
		MTOK_TOKEN,
		MTOK_ARG,
		MTOK_STRINGIFY,
		MTOK_CONCAT,
	} typ;
	union {
		preproc_token_t tok;
		unsigned argid;
		struct { struct mtoken_s *l, *r; } concat;
	} val;
} mtoken_t;
static void mtoken_del(mtoken_t *tok) {
	switch (tok->typ) {
	case MTOK_TOKEN:
		preproc_token_del(&tok->val.tok);
		free(tok);
		return;
		
	case MTOK_CONCAT:
		mtoken_del(tok->val.concat.l);
		mtoken_del(tok->val.concat.r);
		free(tok);
		return;
		
	case MTOK_ARG:
	case MTOK_STRINGIFY:
		free(tok);
		return;
	}
}

KHASH_MAP_INIT_STR(argid_map, unsigned)
static void argid_map_del(khash_t(argid_map) *args) {
	kh_cstr_t str;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	kh_foreach_key(args, str, free((void*)str))
#pragma GCC diagnostic pop
	kh_destroy(argid_map, args);
}
static mtoken_t *mtoken_new_token(preproc_token_t tok) {
	mtoken_t *ret = malloc(sizeof *ret);
	if (!ret) return NULL;
	ret->typ = MTOK_TOKEN;
	ret->val.tok = tok;
	return ret;
}
static mtoken_t *mtoken_new_arg(unsigned argid, int as_string) {
	mtoken_t *ret = malloc(sizeof *ret);
	if (!ret) return NULL;
	ret->typ = as_string ? MTOK_STRINGIFY : MTOK_ARG;
	ret->val.argid = argid;
	return ret;
}
static mtoken_t *mtoken_new_concat(mtoken_t *l, mtoken_t *r) { // Takes ownership of l and r
	mtoken_t *ret = malloc(sizeof *ret);
	if (!ret) {
		mtoken_del(l);
		mtoken_del(r);
		return NULL;
	}
	ret->typ = MTOK_CONCAT;
	ret->val.concat.l = l;
	ret->val.concat.r = r;
	return ret;
}

static inline void macro_tok_print(mtoken_t *m) {
	switch (m->typ) {
	case MTOK_TOKEN:
		printf("token type %u", m->val.tok.tokt);
		if (m->val.tok.tokt == PPTOK_IDENT) printf(" IDENT '%s'", string_content(m->val.tok.tokv.str));
		if (m->val.tok.tokt == PPTOK_NUM  ) printf(" NUM %s", string_content(m->val.tok.tokv.str));
		if (m->val.tok.tokt == PPTOK_SYM) printf(" SYM %s", sym2str[m->val.tok.tokv.sym]);
		return;
		
	case MTOK_ARG:
		printf("argument %u", m->val.argid);
		return;
		
	case MTOK_CONCAT:
		printf("concat {");
		macro_tok_print(m->val.concat.l);
		printf("} {");
		macro_tok_print(m->val.concat.r);
		printf("}");
		return;
		
	case MTOK_STRINGIFY:
		printf("string argument %u", m->val.argid);
		return;
	}
}

VECTOR_DECLARE_STATIC(mtoken, mtoken_t*)
#define mtoken_ptr_del(m) mtoken_del(*(m))
VECTOR_IMPL_STATIC(mtoken, mtoken_ptr_del)
#undef mtoken_ptr_del

typedef struct macro_s {
	int is_funlike;
	int has_varargs;
	unsigned nargs;
	VECTOR(mtoken) *toks;
} macro_t;

KHASH_MAP_INIT_STR(macros_map, macro_t)
KHASH_SET_INIT_STR(string_set)
static void macro_del(macro_t *m) {
	vector_del(mtoken, m->toks);
}
static void macros_map_del(khash_t(macros_map) *args) {
	kh_cstr_t str;
	macro_t *it;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	kh_foreach_key_value_ref(args, str, it, free((void*)str); macro_del(it))
#pragma GCC diagnostic pop
	kh_destroy(macros_map, args);
}
static void macros_set_del(khash_t(string_set) *strset) {
	kh_cstr_t str;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
	kh_foreach_key(strset, str, free((void*)str))
#pragma GCC diagnostic pop
	kh_destroy(string_set, strset);
}

typedef struct ppsource_s {
	enum ppsrc_e {
		PPSRC_PREPARE = 0,
		PPSRC_PPTOKEN,
		PPSRC_PPTOKENS, // Currently, only pushed after macro expansion when the expansion is not empty
		PPSRC_PTOKEN,
	} srct;
	union {
		struct {
			prepare_t *st;
			char *old_dirname;
			char *old_filename;
			_Bool was_sys;
			size_t old_pathno;
			// Note: char is used for the depths, which gives a max depth of 256 #ifs before the program silently bugs
			unsigned char entered_next_ok_cond;
			unsigned char ok_depth, cond_depth;
		} prep;
		preproc_token_t pptok;
		struct {
			VECTOR(preproc) *toks; // Cannot contain a #if, #else, ... statement, they *must* come from a prepare_t
			size_t idx;
		} pptoks;
		proc_token_t ptok; // Added by proc_unget_token
	} srcv;
} ppsource_t;
static void ppsource_del(ppsource_t *src) {
	switch (src->srct) {
	case PPSRC_PREPARE:
		// st may be NULL if we failed to create the prepare_t* in preproc_new_file()
		if (src->srcv.prep.st) prepare_del(src->srcv.prep.st);
		if (src->srcv.prep.old_dirname) free(src->srcv.prep.old_dirname);
		if (src->srcv.prep.old_filename) free(src->srcv.prep.old_filename);
		return;
		
	case PPSRC_PPTOKEN:
		preproc_token_del(&src->srcv.pptok);
		return;
		
	case PPSRC_PPTOKENS:
		// Do not delete the tokens that escaped, only the remaining tokens and the vector
		vector_del_free_from(preproc, src->srcv.pptoks.toks, src->srcv.pptoks.idx);
		return;
		
	case PPSRC_PTOKEN:
		proc_token_del(&src->srcv.ptok);
		return;
	}
}
#define PREPARE_NEW_FILE(f, fn, old_fn, old_dn, was_sys_, old_no) (ppsource_t){ .srct = PPSRC_PREPARE, \
	.srcv.prep = { .st = prepare_new_file((f), (fn)), .old_dirname = (old_dn), .old_filename = (old_fn), .was_sys = was_sys_, \
	               .old_pathno = old_no, .entered_next_ok_cond = 0, .ok_depth = 0, .cond_depth = 0 } }
VECTOR_DECLARE_STATIC(ppsource, ppsource_t)
VECTOR_IMPL_STATIC(ppsource, ppsource_del)

struct preproc_s {
	machine_t *target;
	VECTOR(ppsource) *prep;
	VECTOR(ccharp) *pragma_once;
	enum preproc_state_e {
		PPST_NONE,
		PPST_NL,
		PPST_PRAGMA_EXPLICIT,
	} st;
	khash_t(macros_map) *macros_map;
	khash_t(string_set) *macros_defined, *macros_used;
	char *dirname;
	char *cur_file;
	_Bool is_sys;
	size_t cur_pathno;
};

void preproc_del(preproc_t *src) {
	vector_del(ppsource, src->prep);
	macros_set_del(src->macros_used);
	macros_set_del(src->macros_defined);
	macros_map_del(src->macros_map);
	if (src->dirname) free(src->dirname);
	if (src->cur_file) free(src->cur_file);
	vector_del(ccharp, src->pragma_once);
	free(src);
}

static preproc_token_t ppsrc_next_token(preproc_t *src) {
	switch (vector_last(ppsource, src->prep).srct) {
	case PPSRC_PREPARE:
		return pre_next_token(vector_last(ppsource, src->prep).srcv.prep.st, 0);
		
	case PPSRC_PPTOKEN: {
		preproc_token_t ret = vector_last(ppsource, src->prep).srcv.pptok;
		vector_pop_nodel(ppsource, src->prep);
		return ret; }
		
	case PPSRC_PPTOKENS: {
		ppsource_t *tmp = &vector_last(ppsource, src->prep);
		preproc_token_t ret = vector_content(preproc, tmp->srcv.pptoks.toks)[tmp->srcv.pptoks.idx++];
		if (tmp->srcv.pptoks.idx >= vector_size(preproc, tmp->srcv.pptoks.toks)) {
			vector_del_freed(preproc, tmp->srcv.pptoks.toks);
			vector_pop_nodel(ppsource, src->prep);
		}
		return ret; }
		
	case PPSRC_PTOKEN:
	default:
		return (preproc_token_t){ .tokt = PPTOK_INVALID, .tokv.c = 0 };
	}
}

static int try_open_dir(preproc_t *src, string_t *filename) {
	size_t fnlen = string_len(filename);
	size_t incl_len = src->dirname ? strlen(src->dirname) : 1;
	char *fn = malloc(incl_len + fnlen + 2);
	if (!fn) return 0;
	if (src->dirname) {
		memcpy(fn, src->dirname, incl_len);
		fn[incl_len] = '/';
	} else {
		fn[0] = '.';
		fn[1] = '/';
	}
	strcpy(fn + incl_len + 1, string_content(filename));
	int has_once = 0;
	vector_for(ccharp, fname, src->pragma_once) {
		if (!strcmp(*fname, fn)) {
			has_once = 1;
			break;
		}
	}
	if (has_once) {
#ifdef LOG_INCLUDE
		printf("Skipping opening %s as cur header: '%s' in pragma_once\n", string_content(filename), fn);
#endif
		free(fn);
		return 1;
	} else {
		FILE *f = fopen(fn, "r");
#ifdef LOG_OPEN
		printf("Trying %s: %p\n", fn, f);
#endif
		if (f) {
			char *new_dirname = strchr(fn, '/') ? strndup(fn, (size_t)(strrchr(fn, '/') - fn)) : NULL;
			int ret = vector_push(ppsource, src->prep, PREPARE_NEW_FILE(f, fn, src->cur_file, src->dirname, src->is_sys, src->cur_pathno));
			if (ret) {
				src->is_sys = 0;
				src->cur_file = fn;
				src->dirname = new_dirname;
				src->cur_pathno = 0;
			}
			return ret;
		} else {
			free(fn);
			return 0;
		}
	}
}
static int try_open_sys(preproc_t *src, string_t *filename, size_t array_off) {
	size_t fnlen = string_len(filename);
	for (; array_off < src->target->npaths; ++array_off) {
		size_t incl_len = strlen(src->target->include_path[array_off]);
		char *fn = malloc(incl_len + fnlen + 2);
		if (!fn) return 0;
		memcpy(fn, src->target->include_path[array_off], incl_len);
		fn[incl_len] = '/';
		strcpy(fn + incl_len + 1, string_content(filename));
		int has_once = 0;
		vector_for(ccharp, fname, src->pragma_once) {
			if (!strcmp(*fname, fn)) {
				has_once = 1;
				break;
			}
		}
		if (has_once) {
#ifdef LOG_INCLUDE
			printf("Skipping opening %s as system header: '%s' in pragma_once\n", string_content(filename), fn);
#endif
			free(fn);
			return 1;
		} else {
			FILE *f = fopen(fn, "r");
#ifdef LOG_OPEN
			printf("Trying %s: %p\n", fn, f);
#endif
			if (f) {
				char *new_dirname = strchr(fn, '/') ? strndup(fn, (size_t)(strrchr(fn, '/') - fn)) : NULL;
				int ret = vector_push(ppsource, src->prep, PREPARE_NEW_FILE(f, fn, src->cur_file, src->dirname, src->is_sys, src->cur_pathno));
				if (ret) {
					src->is_sys = 1;
					src->cur_file = fn;
					src->dirname = new_dirname;
					src->cur_pathno = array_off + 1;
				}
				return ret;
			}
			free(fn);
		}
	}
	return 0;
}

preproc_t *preproc_new_file(machine_t *target, FILE *f, char *dirname, const char *filename) {
	preproc_t *ret = malloc(sizeof *ret);
	if (!ret) {
		fclose(f);
		return NULL;
	}
	ret->target = target;
	ret->macros_map = kh_init(macros_map);
	if (!ret->macros_map) {
		fclose(f);
		free(ret);
		return NULL;
	}
	ret->macros_defined = kh_init(string_set);
	if (!ret->macros_defined) {
		kh_destroy(macros_map, ret->macros_map);
		fclose(f);
		free(ret);
		return NULL;
	}
	ret->macros_used = kh_init(string_set);
	if (!ret->macros_used) {
		kh_destroy(macros_map, ret->macros_map);
		kh_destroy(string_set, ret->macros_defined);
		fclose(f);
		free(ret);
		return NULL;
	}
	ret->prep = vector_new_cap(ppsource, 1);
	if (!ret->prep) {
		kh_destroy(macros_map, ret->macros_map);
		kh_destroy(string_set, ret->macros_defined);
		kh_destroy(string_set, ret->macros_used);
		fclose(f);
		free(ret);
		return NULL;
	}
	ret->pragma_once = vector_new(ccharp);
	if (!ret->pragma_once) {
		vector_del(ppsource, ret->prep);
		kh_destroy(macros_map, ret->macros_map);
		kh_destroy(string_set, ret->macros_defined);
		kh_destroy(string_set, ret->macros_used);
		fclose(f);
		free(ret);
		return NULL;
	}
	ret->dirname = NULL;
	ret->cur_file = NULL;
	// ret can now be deleted by preproc_del
	
	// Include the first file
	if (!vector_push(ppsource, ret->prep, PREPARE_NEW_FILE(f, filename, NULL, NULL, 0, 0))) {
		preproc_del(ret);
		return NULL;
	}
	if (!vector_last(ppsource, ret->prep).srcv.prep.st) {
		preproc_del(ret);
		return NULL;
	}
	// Next finish setting up ret
	ret->st = PPST_NL;
	ret->is_sys = 0;
	ret->dirname = dirname;
	ret->cur_file = strdup(filename);
	ret->cur_pathno = 0;
	
	// Also include 'stdc-predef.h' (it will be parsed before the requested file)
	string_t *stdc_predef = string_new_cstr("stdc-predef.h");
	if (!stdc_predef) {
		log_memory("failed to create new string 'stdc-predef.h'\n");
		preproc_del(ret);
		return NULL;
	}
	if (!try_open_sys(ret, stdc_predef, 0)) {
		log_error_nopos("failed to open file 'stdc-predef.h'\n");
		string_del(stdc_predef);
		preproc_del(ret);
		return NULL;
	}
	string_del(stdc_predef);
	
	return ret;
}

static void preprocs_del(VECTOR(preproc) **p) {
	if (!*p) return;
	vector_del(preproc, *p);
}
VECTOR_DECLARE_STATIC(preprocs, VECTOR(preproc)*)
VECTOR_IMPL_STATIC(preprocs, preprocs_del)

static VECTOR(preproc) *preproc_do_expand(loginfo_t *li, const khash_t(macros_map) *macros, const VECTOR(preproc) *toks,
                                          khash_t(string_set) *solved_macros, khash_t(string_set) *opt_used_macros);
	// Does not take any ownership, returns a vector with independent ownerships
	// opt_used_macros is NULL in regular expansion, non-NULL in #if-expansions
static VECTOR(preproc) *preproc_solve_macro(loginfo_t *li,
                                            const khash_t(macros_map) *macros, char *mname, const macro_t *m, VECTOR(preprocs) *margs,
                                            khash_t(string_set) *solved_macros, khash_t(string_set) *opt_used_macros);
	// Moves mname to solved_macros or frees mname, returns a vector with independent ownerships
	// margs may be NULL if m->is_funlike is false
	// May change margs if m->has_varargs, but takes no ownership
	// opt_used_macros is NULL in regular expansion, non-NULL in #if-expansions

static VECTOR(preproc) *preproc_solve_macro(loginfo_t *li,
                                            const khash_t(macros_map) *macros, char *mname, const macro_t *m, VECTOR(preprocs) *margs,
                                            khash_t(string_set) *solved_macros, khash_t(string_set) *opt_used_macros) {
#define LOG_MEMORY(fmt, ...) log_memory(fmt " while expanding %s\n" __VA_ARGS__, mname)
	if (m->is_funlike && !margs) {
		log_internal(li, "m->is_funlike && !margs in preproc_solve_macro(... %s ...)\n", mname);
		free(mname);
		return NULL;
	}
	if (m->is_funlike
	 && (m->nargs != vector_size(preprocs, margs)) // General case
	 && (!m->has_varargs || (m->nargs > vector_size(preprocs, margs))) // Variadics
	 && (m->nargs || ((vector_size(preprocs, margs) == 1) && (vector_size(preproc, vector_last(preprocs, margs)) == 0)))) { // Zero argument
		log_error(li, "invalid argument count for macro %s\n", mname);
		free(mname);
		return NULL;
	}
	if (m->has_varargs) {
		// Change margs to have the correct __VA_ARGS__ (argument m->nargs)
		if (m->nargs == vector_size(preprocs, margs)) {
			// No varargs, so add an empty one
			VECTOR(preproc) *marg = vector_new(preproc);
			if (!marg) {
				LOG_MEMORY("failed to create __VA_ARGS__");
				free(mname);
				return NULL;
			}
			if (!vector_push(preprocs, margs, marg)) {
				LOG_MEMORY("failed to add __VA_ARGS__");
				vector_del(preproc, marg);
				free(mname);
				return NULL;
			}
		} else if (m->nargs < vector_size(preprocs, margs) - 1) {
			// Too many arguments, merge them with commas
			VECTOR(preproc) *mvarg = vector_content(preprocs, margs)[m->nargs];
			size_t size0 = vector_size(preproc, mvarg);
			vector_for_from(preprocs, it, margs, m->nargs + 1) {
				if (!vector_push(preproc, mvarg, ((preproc_token_t){.tokt = PPTOK_SYM, .tokv.sym = SYM_COMMA}))) {
					LOG_MEMORY("failed to add comma to __VA_ARGS__");
					vector_pop_nodel_slice(preproc, mvarg, vector_size(preproc, mvarg) - size0);
					free(mname);
					return NULL;
				}
				if (!vector_push_vec(preproc, mvarg, *it)) {
					LOG_MEMORY("failed to add extra argument to __VA_ARGS__");
					vector_pop_nodel_slice(preproc, mvarg, vector_size(preproc, mvarg) - size0);
					free(mname);
					return NULL;
				}
			}
			vector_pop_nodel_slice(preprocs, margs, vector_size(preprocs, margs) - m->nargs - 1);
		}
	}
	// Avoid 0-allocations
	VECTOR(preproc) **margs2 = calloc(margs ? (vector_size(preprocs, margs) ? vector_size(preprocs, margs) : 1) : 1, sizeof *margs2);
	if (!margs2) {
		LOG_MEMORY("failed to allocate expanded arguments array");
		free(mname);
		return NULL;
	}
	VECTOR(preproc) *ret = vector_new(preproc);
	if (!ret) {
		LOG_MEMORY("failed to allocate return vector");
		free(margs2);
		free(mname);
		return NULL;
	}
	
	VECTOR(mtoken) *st = vector_new_cap(mtoken, vector_size(mtoken, m->toks));
	if (!st) {
		LOG_MEMORY("failed to allocate auxiliary vector");
		vector_del(preproc, ret);
		free(margs2);
		free(mname);
		return NULL;
	}
	vector_for_rev(mtoken, mtok, m->toks) {
		vector_push(mtoken, st, *mtok);
	}
	int need_concat = 0, concat_cur = 0;
	while (vector_size(mtoken, st)) {
		mtoken_t *mtok = vector_last(mtoken, st);
		switch (mtok->typ) {
		case MTOK_CONCAT: {
			vector_last(mtoken, st) = mtok->val.concat.r;
			if (!vector_push(mtoken, st, mtok->val.concat.l)) {
				LOG_MEMORY("failed to add concatenation left m-token");
				vector_del(preproc, ret);
				ret = NULL;
				goto solve_done;
			}
			++need_concat;
			break; }
			
		case MTOK_TOKEN: {
			int do_add = 1;
			if (concat_cur == 2) {
				preproc_token_t *tok1 = &vector_last(preproc, ret); // Guaranteed to exist
				preproc_token_t *tok2 = &mtok->val.tok;
#define ISIDENT(tt) (((tt) == PPTOK_IDENT) || ((tt) == PPTOK_IDENT_UNEXP))
#define FIND(s, c) strchr(string_content((s)), (c))
#define ONLYDIGS(s) (!FIND((s), '.') && !FIND((s), '+') && !FIND((s), '-'))
				if (ISIDENT(tok1->tokt) && (ISIDENT(tok2->tokt) || ((tok2->tokt == PPTOK_NUM) && ONLYDIGS(tok2->tokv.str)))) {
					do_add = 0;
					tok1->tokt = PPTOK_IDENT;
					if (!string_add_string(tok1->tokv.str, tok2->tokv.str)) {
						LOG_MEMORY("failed to concatenate identifiers");
						vector_del(preproc, ret);
						ret = NULL;
						goto solve_done;
					}
				} else if ((tok1->tokt == PPTOK_NUM) && (ISIDENT(tok2->tokt) || (tok2->tokt == PPTOK_NUM))) {
					do_add = 0;
					if (!string_add_string(tok1->tokv.str, tok2->tokv.str)) {
						LOG_MEMORY("failed to concatenate identifiers");
						vector_del(preproc, ret);
						ret = NULL;
						goto solve_done;
					}
				} else {
					log_warning(li, "unsupported concatenation between token type %u and %u while expanding %s\n", tok1->tokt, tok2->tokt, mname);
				}
#undef ISIDENT
#undef FIND
#undef ONLYDIGS
			}
			if (do_add) {
				preproc_token_t *tok1 = &mtok->val.tok;
				if (tok1->tokt != PPTOK_NEWLINE) {
					preproc_token_t tok2;
					switch (tok1->tokt) {
					case PPTOK_INVALID:
					case PPTOK_NEWLINE:
					case PPTOK_BLANK:
					case PPTOK_START_LINE_COMMENT:
					case PPTOK_EOF: tok2 = (preproc_token_t){.tokt = tok1->tokt, .loginfo = *li, .tokv.c = tok1->tokv.c}; break;
					case PPTOK_SYM: tok2 = (preproc_token_t){.tokt = tok1->tokt, .loginfo = *li, .tokv.sym = tok1->tokv.sym}; break;
					case PPTOK_IDENT:
					case PPTOK_IDENT_UNEXP:
					case PPTOK_NUM: {
						string_t *dup = string_dup(tok1->tokv.str);
						if (!dup) {
							LOG_MEMORY("failed to duplicate string");
							vector_del(preproc, ret);
							ret = NULL;
							goto solve_done;
						}
						tok2 = (preproc_token_t){.tokt = tok1->tokt, .loginfo = *li, .tokv.str = dup};
						break; }
					case PPTOK_INCL:
					case PPTOK_STRING: {
						string_t *dup = string_dup(tok1->tokv.sstr);
						if (!dup) {
							LOG_MEMORY("failed to duplicate string");
							vector_del(preproc, ret);
							ret = NULL;
							goto solve_done;
						}
						tok2 = (preproc_token_t){.tokt = tok1->tokt, .loginfo = *li, .tokv.sstr = dup, .tokv.sisstr = tok1->tokv.sisstr};
						break; }
					}
					if (!vector_push(preproc, ret, tok2)) {
						LOG_MEMORY("failed to add token to output vector");
						preproc_token_del(&tok2);
						vector_del(preproc, ret);
						ret = NULL;
						goto solve_done;
					}
				}
			}
			vector_pop_nodel(mtoken, st);
			if (need_concat) {
				concat_cur = 2;
				--need_concat;
			} else concat_cur = 0;
			break; }
			
		case MTOK_ARG: {
			VECTOR(preproc) *toks_to_add;
			if (!need_concat && !concat_cur) {
				if (!margs2[mtok->val.argid]) {
					margs2[mtok->val.argid] = preproc_do_expand(li, macros, vector_content(preprocs, margs)[mtok->val.argid], solved_macros, opt_used_macros);
					if (!margs2[mtok->val.argid]) {
						vector_del(preproc, ret);
						ret = NULL;
						goto solve_done;
					}
				}
				toks_to_add = margs2[mtok->val.argid];
			} else {
				toks_to_add = vector_content(preprocs, margs)[mtok->val.argid];
			}
			size_t tta_start = 0, len = vector_size(preproc, toks_to_add);
			if (len && (concat_cur == 2)) {
				preproc_token_t *tok1 = &vector_last(preproc, ret); // Guaranteed to exist
				preproc_token_t *tok2 = vector_begin(preproc, toks_to_add);
#define ISIDENT(tt) (((tt) == PPTOK_IDENT) || ((tt) == PPTOK_IDENT_UNEXP))
#define FIND(s, c) strchr(string_content((s)), (c))
#define ONLYDIGS(s) (!FIND((s), '.') && !FIND((s), '+') && !FIND((s), '-'))
				if (ISIDENT(tok1->tokt) && (ISIDENT(tok2->tokt) || ((tok2->tokt == PPTOK_NUM) && ONLYDIGS(tok2->tokv.str)))) {
					tta_start = 1; --len;
					tok1->tokt = PPTOK_IDENT;
					if (!string_add_string(tok1->tokv.str, tok2->tokv.str)) {
						LOG_MEMORY("failed to concatenate identifiers");
						vector_del(preproc, ret);
						ret = NULL;
						goto solve_done;
					}
				} else if ((tok1->tokt == PPTOK_NUM) && (ISIDENT(tok2->tokt) || (tok2->tokt == PPTOK_NUM))) {
					tta_start = 1; --len;
					if (!string_add_string(tok1->tokv.str, tok2->tokv.str)) {
						LOG_MEMORY("failed to concatenate identifiers");
						vector_del(preproc, ret);
						ret = NULL;
						goto solve_done;
					}
				} else {
					log_warning(li, "unsupported concatenation between token type %u and %u while expanding %s\n", tok1->tokt, tok2->tokt, mname);
				}
#undef ISIDENT
#undef FIND
#undef ONLYDIGS
			}
			if (len) {
				preproc_token_t tok2;
				for (preproc_token_t *tok1 = vector_begin(preproc, toks_to_add) + tta_start; tok1 != vector_end(preproc, toks_to_add); ++tok1) {
					if (tok1->tokt == PPTOK_NEWLINE) continue;
					switch (tok1->tokt) {
					case PPTOK_INVALID:
					case PPTOK_NEWLINE:
					case PPTOK_BLANK:
					case PPTOK_START_LINE_COMMENT:
					case PPTOK_EOF: tok2 = (preproc_token_t){.tokt = tok1->tokt, .loginfo = *li, .tokv.c = tok1->tokv.c}; break;
					case PPTOK_SYM: tok2 = (preproc_token_t){.tokt = tok1->tokt, .loginfo = *li, .tokv.sym = tok1->tokv.sym}; break;
					case PPTOK_IDENT:
					case PPTOK_IDENT_UNEXP:
					case PPTOK_NUM: {
						string_t *dup = string_dup(tok1->tokv.str);
						if (!dup) {
							LOG_MEMORY("failed to duplicate string");
							vector_del(preproc, ret);
							ret = NULL;
							goto solve_done;
						}
						tok2 = (preproc_token_t){.tokt = tok1->tokt, .loginfo = *li, .tokv.str = dup};
						break; }
					case PPTOK_INCL:
					case PPTOK_STRING: {
						string_t *dup = string_dup(tok1->tokv.sstr);
						if (!dup) {
							LOG_MEMORY("failed to duplicate string");
							vector_del(preproc, ret);
							ret = NULL;
							goto solve_done;
						}
						tok2 = (preproc_token_t){.tokt = tok1->tokt, .loginfo = *li, .tokv.sstr = dup, .tokv.sisstr = tok1->tokv.sisstr};
						break; }
					}
					if (!vector_push(preproc, ret, tok2)) {
						LOG_MEMORY("failed to add token to output vector");
						preproc_token_del(&tok2);
						vector_del(preproc, ret);
						ret = NULL;
						goto solve_done;
					}
				}
			}
			vector_pop_nodel(mtoken, st);
			if (need_concat) {
				concat_cur = (vector_size(preproc, toks_to_add) || (concat_cur == 2)) ? 2 : 1;
				--need_concat;
			} else concat_cur = 0;
			break; }
			
		case MTOK_STRINGIFY:
			if (concat_cur == 2) {
				log_warning(li, "invalid concatenation with string while expanding %s\n", mname);
			}
			// TODO: better stringifier
			preproc_token_t tok2;
			{
				string_t *dup = string_new_cap(15);
				if (!dup) {
					LOG_MEMORY("failed to allocate stringify string");
					vector_del(preproc, ret);
					ret = NULL;
					goto solve_done;
				}
				string_add_char(dup, '<');
				string_add_char(dup, 'S');
				string_add_char(dup, 't');
				string_add_char(dup, 'r');
				string_add_char(dup, 'i');
				string_add_char(dup, 'n');
				string_add_char(dup, 'g');
				string_add_char(dup, 'i');
				string_add_char(dup, 'f');
				string_add_char(dup, 'y');
				string_add_char(dup, ' ');
				string_add_char(dup, 'a');
				string_add_char(dup, 'r');
				string_add_char(dup, 'g');
				string_add_char(dup, '>');
				tok2 = (preproc_token_t){.tokt = PPTOK_STRING, .loginfo = *li, .tokv.sstr = dup, .tokv.sisstr = 1};
			}
			if (!vector_push(preproc, ret, tok2)) {
				LOG_MEMORY("failed to add token to output vector");
				preproc_token_del(&tok2);
				vector_del(preproc, ret);
				ret = NULL;
				goto solve_done;
			}
			vector_pop_nodel(mtoken, st);
			if (need_concat) {
				concat_cur = 2;
				--need_concat;
			} else concat_cur = 0;
			break;
		}
	}
	
solve_done:
	// Don't call the destructors
	vector_del_freed(mtoken, st);
	if (margs) {
		for (size_t i = 0; i < vector_size(preprocs, margs); ++i) {
			if (margs2[i]) vector_del(preproc, margs2[i]);
		}
	}
	free(margs2);
	
	if (ret) {
		// Success, now expand the result
		// First add mname to the set of expanded macros
		int iret;
		kh_put(string_set, solved_macros, mname, &iret);
		if (iret < 0) {
			LOG_MEMORY("failed to add current macro to the set of expanded macros");
			vector_del(preproc, ret);
			free(mname);
			return NULL;
		}
		
		// Next expand every remaining macros
		vector_trim(preproc, ret);
		VECTOR(preproc) *ret2 = preproc_do_expand(li, macros, ret, solved_macros, opt_used_macros);
		vector_del(preproc, ret);
		ret = ret2;
		if (!ret) return NULL; // There was an error, abort
		
		// Finally pop mname (in case we are expanding an argument)
		khiter_t it = kh_get(string_set, solved_macros, mname);
		if (it == kh_end(solved_macros)) {
			log_internal(li, "failed to find current macro in the set of expanded macros while expanding %s\n" , mname);
			vector_del(preproc, ret);
			return NULL;
		}
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
		free((void*)kh_key(solved_macros, it));
#pragma GCC diagnostic pop
		kh_del(string_set, solved_macros, it);
	}
#undef LOG_MEMORY
	
	return ret;
}
static VECTOR(preproc) *preproc_do_expand(loginfo_t *li, const khash_t(macros_map) *macros, const VECTOR(preproc) *toks,
                                          khash_t(string_set) *solved_macros, khash_t(string_set) *opt_used_macros) {
	VECTOR(preproc) *toks2 = vector_new_cap(preproc, vector_size(preproc, toks));
	if (!toks2) {
		log_memory("failed to allocate full macro expansion auxiliary vector\n");
		return NULL;
	}
	vector_for(preproc, tok, toks) {
		if (tok->tokt == PPTOK_NEWLINE) continue;
		preproc_token_t tok2;
		switch (tok->tokt) {
		case PPTOK_INVALID:
		case PPTOK_NEWLINE:
		case PPTOK_BLANK:
		case PPTOK_START_LINE_COMMENT:
		case PPTOK_EOF: tok2 = (preproc_token_t){.tokt = tok->tokt, .loginfo = tok->loginfo, .tokv.c = tok->tokv.c}; break;
		case PPTOK_SYM: tok2 = (preproc_token_t){.tokt = tok->tokt, .loginfo = tok->loginfo, .tokv.sym = tok->tokv.sym}; break;
		case PPTOK_IDENT:
		case PPTOK_IDENT_UNEXP:
		case PPTOK_NUM: {
			string_t *dup = string_dup(tok->tokv.str);
			if (!dup) {
				log_memory("failed to duplicate string during full macro expansion\n");
				vector_del(preproc, toks2);
				return NULL;
			}
			tok2 = (preproc_token_t){.tokt = tok->tokt, .loginfo = tok->loginfo, .tokv.str = dup};
			break; }
		case PPTOK_INCL:
		case PPTOK_STRING: {
			string_t *dup = string_dup(tok->tokv.sstr);
			if (!dup) {
				log_memory("failed to duplicate string during full macro expansion\n");
				vector_del(preproc, toks2);
				return NULL;
			}
			tok2 = (preproc_token_t){.tokt = tok->tokt, .loginfo = tok->loginfo, .tokv.sstr = dup, .tokv.sisstr = tok->tokv.sisstr};
			break; }
		}
		vector_push(preproc, toks2, tok2); // cap > size, thus this always succeed
	}
	
	VECTOR(preproc) *ret = vector_new_cap(preproc, vector_size(preproc, toks));
	if (!ret) {
		log_memory("failed to allocate full macro expansion result vector\n");
		return NULL;
	}
	vector_for(preproc, tok, toks2) {
		switch (tok->tokt) {
		case PPTOK_IDENT: {
			if (opt_used_macros && !strcmp(string_content(tok->tokv.str), "defined") && (vector_end(preproc, toks2) - tok >= 2)) {
				// We need to support "defined ident" and "defined(ident)"
				string_t *mname = NULL;
				preproc_token_t *tok2;
				// tok2 is the last used token, mname is the ident string
				if ((tok[1].tokt == PPTOK_SYM) && (tok[1].tokv.sym == SYM_LPAREN)) {
					if ((vector_end(preproc, toks2) - tok >= 4) && (tok[3].tokt == PPTOK_SYM) && (tok[3].tokv.sym == SYM_RPAREN)) {
						if ((tok[2].tokt == PPTOK_IDENT) || (tok[2].tokt == PPTOK_IDENT_UNEXP)) {
							tok2 = tok + 3;
							mname = tok[2].tokv.str;
						}
					}
				} else if ((tok[1].tokt == PPTOK_IDENT) || (tok[1].tokt == PPTOK_IDENT_UNEXP)) {
					tok2 = tok + 1;
					mname = tok[1].tokv.str;
				}
				if (mname) {
					// Add the ident to the used_macros
					int iret;
					char *mname2 = strdup(string_content(mname));
					if (!mname2) {
						// Abort
						log_memory("failed to add %s to the list of used macros (strdup returned NULL)\n", string_content(mname));
						goto expand_done;
					}
					kh_put(string_set, opt_used_macros, mname2, &iret);
					if (iret < 0) {
						// Abort
						free(mname2);
						log_memory("failed to add %s to the list of used macros (kh_put ireturned %d)\n", string_content(mname), iret);
						goto expand_done;
					} else if (iret == 0) {
						// Just free mname2, it was already present
						free(mname2);
					}
					
					// Value is either 0 or 1 as a PPTOK_NUM, so we need a string with capacity 1
					string_t *num_str = string_new_cap(1);
					if (!num_str) {
						// Abort
						log_memory("failed to create defined() output\n");
						goto expand_done;
					}
					khiter_t it = kh_get(macros_map, macros, string_content(mname));
					string_add_char(num_str, (it == kh_end(macros)) ? '0' : '1');
					if (!vector_push(preproc, ret, ((preproc_token_t){.tokt = PPTOK_NUM, .loginfo = *li, .tokv.str = num_str}))) {
						log_memory("failed to add defined() result to the output\n");
						string_del(num_str);
						goto expand_done;
					}
					
					// Done
					string_del(tok->tokv.str);
					string_del(mname);
					tok = tok2;
					break;
				}
				// If mname == NULL, we simply ignore it and check if there is a macro called 'defined'
			}
			khiter_t it = kh_get(string_set, solved_macros, string_content(tok->tokv.str));
			if (it != kh_end(solved_macros)) {
				tok->tokt = PPTOK_IDENT_UNEXP;
			} else {
				it = kh_get(macros_map, macros, string_content(tok->tokv.str));
				if (it != kh_end(macros)) {
					macro_t *m = &kh_val(macros, it);
					if (m->is_funlike) {
						preproc_token_t *tok2 = tok + 1;
						if ((tok2 < vector_end(preproc, toks2)) && (tok2->tokt == PPTOK_SYM) && (tok2->tokv.sym == SYM_LPAREN)) {
							unsigned depth = 1;
							VECTOR(preprocs) *margs = vector_new(preprocs);
							if (!margs) {
								log_memory("failed to allocate macro arguments for macro %s during full macro expansion\n", string_content(tok->tokv.str));
								if (margs) vector_del(preprocs, margs);
								goto expand_done;
							}
							VECTOR(preproc) *marg = vector_new(preproc);
							if (!marg) goto gather_args_err_mem;
							
							while (depth && (tok2 < vector_end(preproc, toks2) - 1) && (tok2->tokt != PPTOK_EOF) && (tok2->tokt != PPTOK_INVALID)) {
								++tok2;
								if ((depth == 1) && (tok2->tokt == PPTOK_SYM) && (tok2->tokv.sym == SYM_COMMA)) {
									// Possible optimization: emplace NULL if vector_size(marg) == 0
									// This would avoid allocating a new vector, but needs support in preproc_solve_macro
									vector_trim(preproc, marg);
									if (!vector_push(preprocs, margs, marg)) goto gather_args_err_mem;
									marg = vector_new(preproc);
									if (!marg) goto gather_args_err_mem;
								} else {
									if (!vector_push(preproc, marg, *tok2)) goto gather_args_err_mem;
								}
								if ((tok2->tokt == PPTOK_SYM) && (tok2->tokv.sym == SYM_LPAREN)) ++depth;
								else if ((tok2->tokt == PPTOK_SYM) && (tok2->tokv.sym == SYM_RPAREN)) --depth;
							}
							if (depth) {
								// This may be OK ("Unfinished fun-like macro %s\n", string_content(tok->tokv.str));
								vector_del(preprocs, margs);
								vector_del(preproc, marg);
								goto abort_macro;
							}
							// margs finishes with a SYM_RPAREN token
							vector_pop(preproc, marg);
							vector_trim(preproc, marg);
							if (!vector_push(preprocs, margs, marg)) goto gather_args_err_mem;
							marg = NULL;
							
							if (0) {
							gather_args_err_mem:
								log_memory("failed to gather macro arguments for macro %s during full macro expansion\n", string_content(tok->tokv.str));
								if (marg) vector_del(preproc, marg);
								vector_del(preprocs, margs);
								goto expand_done;
							}
							
							// OK, now expand the macro
							char *mname = string_steal(tok->tokv.str);
							tok = tok2;
							
							VECTOR(preproc) *expanded = preproc_solve_macro(li, macros, mname, m, margs, solved_macros, opt_used_macros);
							vector_del(preprocs, margs);
							if (!expanded) {
								// Error expanding the macro
								goto expand_done;
							}
							if (!vector_push_vec(preproc, ret, expanded)) {
								log_memory("pushing expanded macro to full macro expansion\n");
								vector_del(preproc, expanded);
								goto expand_done;
							}
							vector_del_freed(preproc, expanded);
							// Done
							break;
						}
					} else {
						VECTOR(preproc) *expanded = preproc_solve_macro(li, macros, string_steal(tok->tokv.str), m, NULL, solved_macros, opt_used_macros);
						if (!expanded) {
							++tok; // Current token is already freed (string stolen)
							goto expand_done;
						}
						if (!vector_push_vec(preproc, ret, expanded)) {
							log_memory("failed to add macro expansion in output vector of full macro expansion\n");
							++tok; // Current token is already freed (string stolen)
							goto expand_done;
						}
						vector_del_freed(preproc, expanded);
						break;
					}
				}
			}
		}
			/* FALLTHROUGH */
		abort_macro:
		case PPTOK_IDENT_UNEXP:
		case PPTOK_INVALID:
		case PPTOK_NUM:
		case PPTOK_STRING:
		case PPTOK_INCL:
		case PPTOK_SYM:
		case PPTOK_NEWLINE:
		case PPTOK_BLANK:
		case PPTOK_START_LINE_COMMENT:
		case PPTOK_EOF:
			if (!vector_push(preproc, ret, *tok)) {
				log_memory("failed to add token to output vector during full macro expansion\n");
				goto expand_done;
			}
			break;
		}
		
		if (0) {
		expand_done:
			vector_del_free_from(preproc, toks2, (size_t)(tok - vector_begin(preproc, toks2)));
			vector_del(preproc, ret);
			return NULL;
		}
	}
	vector_del_freed(preproc, toks2);
	
	return ret;
}

#define OPLVL_MUL 2
#define OPLVL_DIV 2
#define OPLVL_MOD 2
#define OPLVL_ADD 3
#define OPLVL_SUB 3
#define OPLVL_LSL 4
#define OPLVL_LSR 4
#define OPLVL_LET 5
#define OPLVL_LEE 5
#define OPLVL_GRT 5
#define OPLVL_GRE 5
#define OPLVL_EQU 6
#define OPLVL_NEQ 6
#define OPLVL_AAN 7  // Bitwise (Arithmetic)
#define OPLVL_XOR 8
#define OPLVL_AOR 9  // Bitwise (Arithmetic)
#define OPLVL_BAN 10 // Boolean
#define OPLVL_BOR 11 // Boolean
#define OPLVL_ALL 12 // Evaluate the entire stack
#define OPLVL_ALS 13 // Evaluate the entire stack, (don't update the states (ignored))
#define OPTYP_MUL 1
#define OPTYP_DIV 2
#define OPTYP_MOD 3
#define OPTYP_ADD 1
#define OPTYP_SUB 2
#define OPTYP_LSL 1
#define OPTYP_LSR 2
#define OPTYP_LET 1
#define OPTYP_LEE 2
#define OPTYP_GRT 3
#define OPTYP_GRE 4
#define OPTYP_EQU 1
#define OPTYP_NEQ 2
#define OPTYP_AAN 1
#define OPTYP_XOR 1
#define OPTYP_AOR 1
#define OPTYP_BAN 1
#define OPTYP_BOR 1
typedef struct preproc_eval_aux_s {
	struct {
		unsigned st0 : 1;
		unsigned st2 : 2;
		unsigned st3 : 2;
		unsigned st4 : 2;
		unsigned st5 : 3;
		unsigned st6 : 2;
		unsigned st7 : 1;
		unsigned st8 : 1;
		unsigned st9 : 1;
		unsigned st_bool : 1;
	};
	int64_t v0;
	unsigned st1;
	const preproc_token_t *v1;
	int64_t v2; loginfo_t li2;
	int64_t v3; loginfo_t li3;
	int64_t v4; loginfo_t li4;
	int64_t v5; loginfo_t li5;
	int64_t v6; loginfo_t li6;
	int64_t v7; loginfo_t li7;
	int64_t v8; loginfo_t li8;
	int64_t v9; loginfo_t li9;
	loginfo_t libool;
	unsigned n_colons; // Number of ':' expected (needs to skip to the end), ie number of ternary where the truth-y branch is taken
} preproc_eval_aux_t;
VECTOR_DECLARE_STATIC(ppeaux, preproc_eval_aux_t)
VECTOR_IMPL_STATIC(ppeaux, (void))
static int64_t eval_stack(preproc_eval_aux_t *st, loginfo_t li, int max_lv, int optype, _Bool is_unsigned, int *success) {
	if (!li.lineno_end) {
		li.lineno_end = li.lineno;
		li.colno_end = li.colno;
	}
	
	int64_t acc = st->v0;
	st->st0 = 0;
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wduplicated-cond" // For the else if (st->stX)
	if (st->st2 == OPTYP_MUL) {
		acc = st->v2 * acc;
		st->st2 = 0;
		li.lineno = st->li2.lineno;
		li.colno = st->li2.colno;
	} else if (st->st2 == OPTYP_DIV) {
		if (!acc) {
			log_error(&li, "division by zero during #if evaluation\n");
			*success = 0;
			return 0;
		}
		acc = st->v2 / acc;
		st->st2 = 0;
		li.lineno = st->li2.lineno;
		li.colno = st->li2.colno;
	} else if (st->st2 == OPTYP_MOD) {
		if (!acc) {
			log_error(&li, "division by zero during #if evaluation\n");
			*success = 0;
			return 0;
		}
		acc = st->v2 % acc;
		st->st2 = 0;
		li.lineno = st->li2.lineno;
		li.colno = st->li2.colno;
	} else if (st->st2) {
		log_internal(&st->li2, "unknown st2 %d during #if evaluation\n", st->st2);
		*success = 0;
		return 0;
	}
	if (max_lv == 2) {
		st->st2 = optype;
		st->v2 = acc;
		st->li2 = li;
		return acc;
	}
	if (st->st3 == OPTYP_ADD) {
		acc = st->v3 + acc;
		st->st3 = 0;
		li.lineno = st->li3.lineno;
		li.colno = st->li3.colno;
	} else if (st->st3 == OPTYP_SUB) {
		acc = st->v3 - acc;
		st->st3 = 0;
		li.lineno = st->li3.lineno;
		li.colno = st->li3.colno;
	} else if (st->st3) {
		log_internal(&st->li3, "unknown st3 %d during #if evaluation\n", st->st3);
		*success = 0;
		return 0;
	}
	if (max_lv == 3) {
		st->st3 = optype;
		st->v3 = acc;
		st->li3 = li;
		return acc;
	}
	if (st->st4 == OPTYP_LSL) {
		acc = st->v4 << acc;
		st->st4 = 0;
		li.lineno = st->li4.lineno;
		li.colno = st->li4.colno;
	} else if (st->st4 == OPTYP_LSR) {
		acc = is_unsigned ? (int64_t)((uint64_t)st->v4 >> (uint64_t)acc) : (st->v4 >> acc);
		st->st4 = 0;
		li.lineno = st->li4.lineno;
		li.colno = st->li4.colno;
	} else if (st->st4) {
		log_internal(&st->li4, "unknown st4 %d during #if evaluation\n", st->st4);
		*success = 0;
		return 0;
	}
	if (max_lv == 4) {
		st->st4 = optype;
		st->v4 = acc;
		st->li4 = li;
		return acc;
	}
	if (st->st5 == OPTYP_LET) {
		acc = is_unsigned ? ((uint64_t)st->v5 < (uint64_t)acc) : (st->v5 < acc);
		is_unsigned = 0;
		st->st5 = 0;
		li.lineno = st->li5.lineno;
		li.colno = st->li5.colno;
	} else if (st->st5 == OPTYP_LEE) {
		acc = is_unsigned ? ((uint64_t)st->v5 <= (uint64_t)acc) : (st->v5 <= acc);
		is_unsigned = 0;
		st->st5 = 0;
		li.lineno = st->li5.lineno;
		li.colno = st->li5.colno;
	} else if (st->st5 == OPTYP_GRT) {
		acc = is_unsigned ? ((uint64_t)st->v5 > (uint64_t)acc) : (st->v5 > acc);
		is_unsigned = 0;
		st->st5 = 0;
		li.lineno = st->li5.lineno;
		li.colno = st->li5.colno;
	} else if (st->st5 == OPTYP_GRE) {
		acc = is_unsigned ? ((uint64_t)st->v5 >= (uint64_t)acc) : (st->v5 >= acc);
		is_unsigned = 0;
		st->st5 = 0;
		li.lineno = st->li5.lineno;
		li.colno = st->li5.colno;
	} else if (st->st5) {
		log_internal(&st->li5, "unknown st5 %d during #if evaluation\n", st->st5);
		*success = 0;
		return 0;
	}
	if (max_lv == 5) {
		st->st5 = optype;
		st->v5 = acc;
		st->li5 = li;
		return acc;
	}
	if (st->st6 == OPTYP_EQU) {
		acc = st->v6 == acc;
		st->st6 = 0;
		li.lineno = st->li6.lineno;
		li.colno = st->li6.colno;
	} else if (st->st6 == OPTYP_NEQ) {
		acc = st->v6 != acc;
		st->st6 = 0;
		li.lineno = st->li6.lineno;
		li.colno = st->li6.colno;
	} else if (st->st6) {
		log_internal(&st->li6, "unknown st6 %d during #if evaluation\n", st->st6);
		*success = 0;
		return 0;
	}
	if (max_lv == 6) {
		st->st6 = optype;
		st->v6 = acc;
		st->li6 = li;
		return acc;
	}
	if (st->st7 == OPTYP_AAN) {
		acc = st->v7 & acc;
		st->st7 = 0;
		li.lineno = st->li7.lineno;
		li.colno = st->li7.colno;
	} else if (st->st7) {
		log_internal(&st->li7, "unknown st7 %d during #if evaluation\n", st->st7);
		*success = 0;
		return 0;
	}
	if (max_lv == 7) {
		st->st7 = optype;
		st->v7 = acc;
		st->li7 = li;
		return acc;
	}
	if (st->st8 == OPTYP_XOR) {
		acc = st->v8 ^ acc;
		st->st8 = 0;
		li.lineno = st->li8.lineno;
		li.colno = st->li8.colno;
	} else if (st->st8) {
		log_internal(&st->li8, "unknown st8 %d during #if evaluation\n", st->st8);
		*success = 0;
		return 0;
	}
	if (max_lv == 8) {
		st->st8 = optype;
		st->v8 = acc;
		st->li8 = li;
		return acc;
	}
	if (st->st9 == OPTYP_AOR) {
		acc = st->v9 | acc;
		st->st9 = 0;
		li.lineno = st->li9.lineno;
		li.colno = st->li9.colno;
	} else if (st->st9) {
		log_internal(&st->li9, "unknown st9 %d during #if evaluation\n", st->st9);
		*success = 0;
		return 0;
	}
	if (max_lv == 9) {
		st->st9 = optype;
		st->v9 = acc;
		st->li9 = li;
		return acc;
	}
	if (max_lv == 10) { // OPLVL == OPLVL_BAN, OPTYP == OPTYP_BAN
		li.lineno = st->libool.lineno;
		li.colno = st->libool.colno;
		st->libool = li;
		return acc;
	}
	if (max_lv == 11) { // OPLVL == OPLVL_BOR, OPTYP == OPTYP_BOR
		li.lineno = st->libool.lineno;
		li.colno = st->libool.colno;
		st->libool = li;
		return acc;
	}
	if (st->st_bool) {
		acc = !!acc;
		st->st_bool = 0;
	}
	if ((max_lv == OPLVL_ALL) || (max_lv == OPLVL_ALS)) {
		return acc;
	}
#pragma GCC diagnostic pop
	log_internal(&li, "invalid max level %d in preprocessor partial expression evaluation\n", max_lv);
	*success = 0;
	return acc;
}
static int64_t preproc_eval(const VECTOR(preproc) *cond, int *aux_ret, int ptr_is_32bits) {
	int success;
	VECTOR(ppeaux) *stack = vector_new_cap(ppeaux, 1);
	if (!stack) {
		log_memory("failed to allocate #if evaluation stack vector\n");
		*aux_ret = 0;
		return 0;
	}
	if (0) {
	eval_fail:
		vector_del(ppeaux, stack);
		*aux_ret = 0;
		return 0;
	}
	vector_push(ppeaux, stack, (preproc_eval_aux_t){0}); // vector_cap >= 1
	int64_t acc; loginfo_t li0; _Bool is_unsigned = 0;
	vector_for(preproc, tok, cond) {
		if (tok->tokt == PPTOK_NUM) {
			// Evaluate token as an integer if st0 == 0, error otherwise
			if (vector_last(ppeaux, stack).st0 == 0) {
				num_constant_t cst;
				if (!num_constant_convert(&tok->loginfo, tok->tokv.str, &cst, ptr_is_32bits)) {
					goto eval_fail;
				}
				switch (cst.typ) {
				case NCT_INT32:                   acc =          cst.val.i32; break;
				case NCT_UINT32: is_unsigned = 1; acc =          cst.val.u32; break;
				case NCT_INT64:                   acc =          cst.val.i64; break;
				case NCT_UINT64: is_unsigned = 1; acc = (int64_t)cst.val.u64; break;
				case NCT_FLOAT:
				case NCT_DOUBLE:
				case NCT_LDOUBLE:
				default:
					log_error(&tok->loginfo, "number '%s' is not a valid integer (during #if evaluation)\n", string_content(tok->tokv.str));
					goto eval_fail;
				}
				li0 = tok->loginfo;
				goto push_acc_to_st0;
			} else {
				log_error(&tok->loginfo, "unexpected number during #if evaluation\n");
				goto eval_fail;
			}
		} else if ((tok->tokt == PPTOK_IDENT) || (tok->tokt == PPTOK_IDENT_UNEXP)) {
			// Evaluate token as 0 if st0 == 0, error otherwise
			if (vector_last(ppeaux, stack).st0 == 0) {
				acc = 0;
				li0 = tok->loginfo;
				goto push_acc_to_st0;
			} else {
				log_error(&tok->loginfo, "unexpected ident '%s' during #if evaluation\n", string_content(tok->tokv.str));
				goto eval_fail;
			}
		} else if (tok->tokt == PPTOK_SYM) {
			int op_lvl;
			int op_typ;
			switch (tok->tokv.sym) {
			case SYM_TILDE:
				// Unary (st0 == 0)
				if (vector_last(ppeaux, stack).st0 != 0) {
					log_error(&tok->loginfo, "unexpected unary '~' in #if expression\n");
					goto eval_fail;
				}
				if (!(vector_last(ppeaux, stack).st1++)) {
					// Also update v1, since this is the first level 1 operator
					vector_last(ppeaux, stack).v1 = tok;
				}
				goto done_partial_eval;
			case SYM_EXCL:
				// Unary (st0 == 0)
				if (vector_last(ppeaux, stack).st0 != 0) {
					log_error(&tok->loginfo, "unexpected unary '!' in #if expression\n");
					goto eval_fail;
				}
				if (!(vector_last(ppeaux, stack).st1++)) {
					// Also update v1, since this is the first level 1 operator
					vector_last(ppeaux, stack).v1 = tok;
				}
				goto done_partial_eval;
			case SYM_PLUS:
				// May be unary (st0 == 0) or binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) {
					// Unary, prepare lv1 then continue to the next token
					if (!(vector_last(ppeaux, stack).st1++)) {
						// Also update v1, since this is the first level 1 operator
						vector_last(ppeaux, stack).v1 = tok;
					}
					goto done_partial_eval;
				} else {
					op_lvl = OPLVL_ADD;
					op_typ = OPTYP_ADD;
					goto add_binop;
				}
			case SYM_DASH:
				// May be unary (st0 == 0) or binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) {
					// Unary, prepare lv1 then continue to the next token
					if (!(vector_last(ppeaux, stack).st1++)) {
						// Also update v1, since this is the first level 1 operator
						vector_last(ppeaux, stack).v1 = tok;
					}
					goto done_partial_eval;
				} else {
					op_lvl = OPLVL_SUB;
					op_typ = OPTYP_SUB;
					goto add_binop;
				}
			case SYM_STAR:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_MUL;
				op_typ = OPTYP_MUL;
				goto add_binop;
			case SYM_SLASH:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_DIV;
				op_typ = OPTYP_DIV;
				goto add_binop;
			case SYM_PERCENT:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_MOD;
				op_typ = OPTYP_MOD;
				goto add_binop;
			case SYM_HAT:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_XOR;
				op_typ = OPTYP_XOR;
				goto add_binop;
			case SYM_AMP:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_AAN;
				op_typ = OPTYP_AAN;
				goto add_binop;
			case SYM_PIPE:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_AOR;
				op_typ = OPTYP_AOR;
				goto add_binop;
			case SYM_EQEQ:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_EQU;
				op_typ = OPTYP_EQU;
				goto add_binop;
			case SYM_EXCLEQ:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_NEQ;
				op_typ = OPTYP_NEQ;
				goto add_binop;
			case SYM_LT:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_LET;
				op_typ = OPTYP_LET;
				goto add_binop;
			case SYM_GT:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_GRT;
				op_typ = OPTYP_GRT;
				goto add_binop;
			case SYM_LTEQ:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_LEE;
				op_typ = OPTYP_LEE;
				goto add_binop;
			case SYM_GTEQ:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_GRE;
				op_typ = OPTYP_GRE;
				goto add_binop;
			case SYM_AMPAMP:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_BAN;
				op_typ = OPTYP_BAN;
				goto add_binop;
			case SYM_PIPEPIPE:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_BOR;
				op_typ = OPTYP_BOR;
				goto add_binop;
			case SYM_LTLT:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_LSL;
				op_typ = OPTYP_LSL;
				goto add_binop;
			case SYM_GTGT:
				// Binary (st0 != 0)
				if (vector_last(ppeaux, stack).st0 == 0) goto invalid_binop;
				op_lvl = OPLVL_LSR;
				op_typ = OPTYP_LSR;
				goto add_binop;
				
			case SYM_LPAREN:
				// May be placed anywhere a constant is expected (st0 == 0) and simply pushes a new stack
				if (vector_last(ppeaux, stack).st0 != 0) {
					log_error(&tok->loginfo, "unexpected opening parenthesis in #if expression\n");
					goto eval_fail;
				}
				if (!vector_push(ppeaux, stack, (preproc_eval_aux_t){0})) {
					log_memory("failed to push a new stack during #if evaluation\n");
					goto eval_fail;
				}
				goto done_partial_eval;
			case SYM_RPAREN:
				// May be placed anywhere after a constant (st0 != 0)
				if (vector_size(ppeaux, stack) == 1) {
					log_error(&tok->loginfo, "unexpected symbol ')' (parenthesis not opened) during #if evaluation\n");
					goto eval_fail;
				}
				if (vector_last(ppeaux, stack).n_colons) {
					log_error(&tok->loginfo, "unexpected symbol ')', expected symbol ':' during #if evaluation\n");
					goto eval_fail;
				}
				if (!vector_last(ppeaux, stack).st0) {
					log_error(&tok->loginfo, "unexpected symbol ')' during #if evaluation\n");
					goto eval_fail;
				}
				
			eval_rparen:
				// Evaluate the top of the stack, then pop
				success = 1;
				acc = eval_stack(&vector_last(ppeaux, stack), li0, OPLVL_ALS, 0, is_unsigned, &success);
				if (!success) {
					goto eval_fail;
				}
				
			eval_rparen_acc:
				vector_pop(ppeaux, stack);
				goto push_acc_to_st0;
				
			case SYM_QUESTION: {
				// May be placed anywhere and may skip some tokens depending on acc
				if (!vector_last(ppeaux, stack).st0) {
					log_error(&tok->loginfo, "unexpected ternary operator during #if evaluation\n");
					goto eval_fail;
				}
				
				// Evaluate the top of the stack, then increase n_colons or skip tokens
				success = 1;
				acc = eval_stack(&vector_last(ppeaux, stack), li0, OPLVL_ALL, 0, is_unsigned, &success);
				if (!success) {
					goto eval_fail;
				}
				
			eval_question_acc:
				is_unsigned = 0;
				if (acc) {
					// Increase n_colons
					++vector_last(ppeaux, stack).n_colons;
					goto done_partial_eval;
				} else {
					// Skip to the corresponding colon
					unsigned nquestions = 0, nparens = 0;
					// Note that we don't really care about the content of the ignored part; it may be syntaxically incorrect
					for (++tok; tok < vector_end(preproc, cond); ++tok) {
						if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_LPAREN)) {
							++nparens;
						} else if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
							if (nparens) --nparens;
							else {
								log_error(&tok->loginfo, "unexpected symbol ')', expected ':' during #if evaluation\n");
								goto eval_fail;
							}
						} else if (!nparens && (tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_QUESTION)) {
							++nquestions;
						} else if (!nparens && (tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_COLON)) {
							if (nquestions) --nquestions;
							else break;
						}
					}
					if (tok == vector_end(preproc, cond)) {
						log_error(&tok->loginfo, "ternary operator never finished during #if evaluation\n");
						goto eval_fail;
					}
					goto done_partial_eval;
				}
			}
			case SYM_COLON: {
				if (!vector_last(ppeaux, stack).n_colons) {
					log_error(&tok->loginfo, "unexpected symbol ':' during #if evaluation\n");
					goto eval_fail;
				}
				if (!vector_last(ppeaux, stack).st0) {
					log_error(&tok->loginfo, "unexpected symbol ':' during #if evaluation\n");
					goto eval_fail;
				}
			eval_colon_acc:
				// Skip to the end of expression or to the corresponding rparen
				if (vector_size(ppeaux, stack) == 1) {
					goto done_complete_stack; // No parenthesis opened, skip to the end of the vector
				}
				// Skip to the next rparen; also, we skip correctly counting of ':' since we don't care about those anymore
				unsigned nparens = 0;
				// Note that we don't really care about the content of the ignored part; it may be syntaxically incorrect
				for (++tok; tok < vector_end(preproc, cond); ++tok) {
					if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_LPAREN)) {
						++nparens;
					} else if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
						if (nparens) --nparens;
						else break;
					}
				}
				// We need to exit at a RPAREN
				if (tok == vector_end(preproc, cond)) {
					log_error(&vector_last(preproc, cond).loginfo, "missing closing parenthesis in #if expression\n");
					goto eval_fail;
				}
				vector_last(ppeaux, stack).n_colons = 0;
				goto eval_rparen; }
				
			case SYM_LBRACKET:
			case SYM_RBRACKET:
			case SYM_LSQBRACKET:
			case SYM_RSQBRACKET:
			case SYM_HASH:
			case SYM_HASHHASH:
			case SYM_SEMICOLON:
			case SYM_COLONCOLON:
			case SYM_VARIADIC:
			case SYM_DOT:
			case SYM_DASHGT:
			case SYM_EQ:
			case SYM_PLUSEQ:
			case SYM_DASHEQ:
			case SYM_STAREQ:
			case SYM_SLASHEQ:
			case SYM_PERCENTEQ:
			case SYM_HATEQ:
			case SYM_AMPEQ:
			case SYM_PIPEEQ:
			case SYM_LTLTEQ:
			case SYM_GTGTEQ:
			case SYM_PLUSPLUS:
			case SYM_DASHDASH:
			case SYM_COMMA:
			default:
				log_error(&tok->loginfo, "invalid symbol ID %u (%s) during #if evaluation\n",
					tok->tokv.sym, (tok->tokv.sym <= LAST_SYM) ? sym2str[tok->tokv.sym] : "invalid");
				goto eval_fail;
			}
			
			// invalid_binop: unexpected binary operation
			// add_binop: add a binary operation
			if (0) {
			invalid_binop:
				log_error(&tok->loginfo, "unexpected binary operator '%s' in #if expression\n", sym2str[tok->tokv.sym]);
				goto eval_fail;
			add_binop:
				if (op_lvl < 2) {
					log_internal(&tok->loginfo, "invalid op_lvl %d < 2 during #if evaluation\n", op_lvl);
					goto eval_fail;
				}
				if (op_lvl >= OPLVL_ALL) {
					log_internal(&tok->loginfo, "invalid op_lvl %d > OPLVL_ALL during #if evaluation\n", op_lvl);
					goto eval_fail;
				}
				
				success = 1;
				acc = eval_stack(&vector_last(ppeaux, stack), li0, op_lvl, op_typ, is_unsigned, &success);
				if (!success) {
					goto eval_fail;
				}
				
				if (op_lvl < OPLVL_BAN) {
					// No post-processing required
					goto done_partial_eval;
				}
				if (op_lvl == OPLVL_BAN) {
					// op_typ == OPLVL_BAN
					vector_last(ppeaux, stack).st_bool = 1;
					if (acc) goto done_partial_eval;
					// We have five possibilities:
					// [0] && x <EOL>     ~> 0
					// [0] && x)          ~> 0)
					// [0] && x ? y : z   ~> z
					// [0] && x : y       ~> 0 : y ~> 0
					// [0] && x || y      ~> y
					// We are at the '&&'; note that we know the top of the stack has no pending operation
					unsigned nparens = 0;
					for (++tok; tok < vector_end(preproc, cond); ++tok) {
						if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_LPAREN)) {
							++nparens;
						} else if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
							if (nparens) --nparens;
							else {
								if (vector_size(ppeaux, stack) == 1) {
									log_error(&tok->loginfo, "unexpected symbol ')' (parenthesis not opened) during #if evaluation\n");
									goto eval_fail;
								}
								if (vector_last(ppeaux, stack).n_colons) {
									log_error(&tok->loginfo, "unexpected symbol ')', expected symbol ':' during #if evaluation\n");
									goto eval_fail;
								}
								
								goto eval_rparen_acc;
							}
						} else if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_QUESTION)) {
							if (!nparens) {
								goto eval_question_acc;
							}
						} else if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_COLON)) {
							if (!nparens) {
								if (!vector_last(ppeaux, stack).n_colons) {
									log_error(&tok->loginfo, "unexpected symbol ':' during #if evaluation\n");
									goto eval_fail;
								}
								goto eval_colon_acc;
							}
						} else if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_PIPEPIPE)) {
							if (!nparens) {
								goto done_partial_eval;
							}
						}
					}
					if (nparens) {
						log_error(&vector_last(preproc, cond).loginfo, "expected symbol ')' during #if evaluation\n");
						goto eval_fail;
					}
					if (vector_last(ppeaux, stack).n_colons) {
						log_error(&vector_last(preproc, cond).loginfo, "expected symbol ':' during #if evaluation\n");
						goto eval_fail;
					}
					if (vector_size(ppeaux, stack) != 1) {
						log_error(&vector_last(preproc, cond).loginfo, "expected symbol ')' during #if evaluation\n");
						goto eval_fail;
					}
					goto done_complete_acc;
				}
				if (op_lvl == OPLVL_BOR) {
					// op_typ == OPLVL_BOR
					vector_last(ppeaux, stack).st_bool = 1;
					if (!acc) goto done_partial_eval;
					// We have four possibilities:
					// [!0] || x <EOL>     ~> 1
					// [!0] || x)          ~> 1)
					// [!0] || x ? y : z   ~> z
					// [!0] || x : y       ~> 1 : y ~> 1
					// We are at the '||'; note that we know the top of the stack has no pending operation
					unsigned nparens = 0;
					for (++tok; tok < vector_end(preproc, cond); ++tok) {
						if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_LPAREN)) {
							++nparens;
						} else if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_RPAREN)) {
							if (nparens) --nparens;
							else {
								if (vector_size(ppeaux, stack) == 1) {
									log_error(&tok->loginfo, "unexpected symbol ')' (parenthesis not opened) during #if evaluation\n");
									goto eval_fail;
								}
								if (vector_last(ppeaux, stack).n_colons) {
									log_error(&tok->loginfo, "unexpected symbol ')', expected symbol ':' during #if evaluation\n");
									goto eval_fail;
								}
								
								acc = 1;
								goto eval_rparen_acc;
							}
						} else if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_QUESTION)) {
							if (!nparens) {
								acc = 1;
								goto eval_question_acc;
							}
						} else if ((tok->tokt == PPTOK_SYM) && (tok->tokv.sym == SYM_COLON)) {
							if (!nparens) {
								if (!vector_last(ppeaux, stack).n_colons) {
									log_error(&tok->loginfo, "unexpected symbol ':' during #if evaluation\n");
									goto eval_fail;
								}
								acc = 1;
								goto eval_colon_acc;
							}
						}
					}
					if (nparens) {
						log_error(&vector_last(preproc, cond).loginfo, "expected symbol ')' during #if evaluation\n");
						goto eval_fail;
					}
					if (vector_last(ppeaux, stack).n_colons) {
						log_error(&vector_last(preproc, cond).loginfo, "expected symbol ':' during #if evaluation\n");
						goto eval_fail;
					}
					if (vector_size(ppeaux, stack) != 1) {
						log_error(&vector_last(preproc, cond).loginfo, "expected symbol ')' during #if evaluation\n");
						goto eval_fail;
					}
					acc = 1;
					goto done_complete_acc;
				}
				log_internal(&li0, "invalid op_lvl %d during #if evaluation\n", op_lvl);
				goto eval_fail;
			}
			
		done_partial_eval:
		} else {
			log_error(&tok->loginfo, "invalid token type %u during #if evaluation\n", tok->tokt);
			goto eval_fail;
		}
		
		// push_acc_to_st0: evaluate all held st1 operations on acc, then set st0 to 1 and v0 to acc
		if (0) {
		push_acc_to_st0:
			while (vector_last(ppeaux, stack).st1) {
				// Only symbols, so this should be safe
				enum token_sym_type_e sym = vector_last(ppeaux, stack).v1[--vector_last(ppeaux, stack).st1].tokv.sym;
				if (sym == SYM_PLUS) {}
				else if (sym == SYM_DASH) acc = -acc;
				else if (sym == SYM_EXCL) acc = !acc;
				else if (sym == SYM_TILDE) acc = ~acc;
				else {
					log_internal(&vector_last(ppeaux, stack).v1[vector_last(ppeaux, stack).st1].loginfo,
					             "unknown level 1 unary operator sym ID %u\n", sym);
				}
			}
			vector_last(ppeaux, stack).v0 = acc;
			vector_last(ppeaux, stack).st0 = 1;
		}
	}
	
	if (vector_size(ppeaux, stack) != 1) {
		log_error(&vector_last(preproc, cond).loginfo, "expected symbol ')' during #if evaluation\n");
		goto eval_fail;
	}
	
done_complete_stack:
	success = 1;
	acc = eval_stack(&vector_last(ppeaux, stack), li0, OPLVL_ALS, 0, is_unsigned, &success);
	if (!success) {
		goto eval_fail;
	}
	
done_complete_acc:
	vector_del(ppeaux, stack);
	*aux_ret = 1;
	return acc;
}

int proc_unget_token(preproc_t *src, proc_token_t *tok) {
	return vector_push(ppsource, src->prep, ((ppsource_t){ .srct = PPSRC_PTOKEN, .srcv.ptok = *tok }));
}
static proc_token_t proc_next_token_aux(preproc_t *src) {
	if (!vector_size(ppsource, src->prep)) {
		return (proc_token_t){ .tokt = PTOK_EOF, .loginfo = { 0 }, .tokv = {.c = (char)EOF} };
	}
	if (vector_last(ppsource, src->prep).srct == PPSRC_PTOKEN) {
		proc_token_t ret = vector_last(ppsource, src->prep).srcv.ptok;
		vector_pop_nodel(ppsource, src->prep);
		return ret;
	}
check_if_depth:
	{
		if (!vector_size(ppsource, src->prep)) {
			return (proc_token_t){ .tokt = PTOK_EOF, .loginfo = { 0 }, .tokv = {.c = (char)EOF} };
		}
		ppsource_t *ppsrc = &vector_last(ppsource, src->prep);
		if ((ppsrc->srct == PPSRC_PREPARE) && (ppsrc->srcv.prep.ok_depth != ppsrc->srcv.prep.cond_depth)) {
			// Ignore all tokens from ppsrc until:
			//  INVALID -------------- abort (return PTOK_INVALID)
			//  EOF ------------------ print an error, then pop and goto check_if_depth
			//  # if/elif/endif/... -- parse, execute, goto check_if_depth
			// Also, track newlines to keep src->st up-to-date
			
			while (1) {
				preproc_token_t tok = ppsrc_next_token(src);
			skip_cur_token:
				if (tok.tokt == PPTOK_INVALID) {
					src->st = PPST_NONE;
					proc_token_t ret;
					ret.tokt = PTOK_INVALID;
					ret.loginfo = tok.loginfo;
					ret.tokv.c = tok.tokv.c;
					return ret;
				} else if (tok.tokt == PPTOK_NEWLINE) {
					src->st = PPST_NL;
				} else if (tok.tokt == PPTOK_EOF) {
					log_warning(&tok.loginfo, "file ended before closing all conditionals (ignoring)\n");
					vector_pop(ppsource, src->prep);
					goto check_if_depth;
				} else if ((tok.tokt == PPTOK_SYM) && (src->st == PPST_NL) && (tok.tokv.sym == SYM_HASH)) {
					src->st = PPST_NONE;
					tok = ppsrc_next_token(src);
					if ((tok.tokt == PPTOK_NEWLINE) || (tok.tokt == PPTOK_EOF)) {
						// Empty preprocessor command
						src->st = PPST_NL;
						continue;
					}
					if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) goto preproc_ignore_remaining;
					if (!strcmp(string_content(tok.tokv.str), "include")) goto preproc_ignore_remaining;
					else if (!strcmp(string_content(tok.tokv.str), "include_next")) goto preproc_ignore_remaining;
					else if (!strcmp(string_content(tok.tokv.str), "define")) goto preproc_ignore_remaining;
					else if (!strcmp(string_content(tok.tokv.str), "undef")) goto preproc_ignore_remaining;
					else if (!strcmp(string_content(tok.tokv.str), "error")) goto preproc_ignore_remaining;
					else if (!strcmp(string_content(tok.tokv.str), "warning")) goto preproc_ignore_remaining; // C23/extension
					else if (!strcmp(string_content(tok.tokv.str), "pragma")) goto preproc_ignore_remaining;
					else if (!strcmp(string_content(tok.tokv.str), "line")) goto preproc_ignore_remaining;
					else if (!strcmp(string_content(tok.tokv.str), "if")) {
						// Increase cond_depth (we already know we are ignoring the content)
						++ppsrc->srcv.prep.cond_depth;
						goto preproc_ignore_remaining;
					} else if (!strcmp(string_content(tok.tokv.str), "ifdef")) {
						// Increase cond_depth (we already know we are ignoring the content)
						++ppsrc->srcv.prep.cond_depth;
						goto preproc_ignore_remaining;
					} else if (!strcmp(string_content(tok.tokv.str), "ifndef")) {
						// Increase cond_depth (we already know we are ignoring the content)
						++ppsrc->srcv.prep.cond_depth;
						goto preproc_ignore_remaining;
					} else if (!strcmp(string_content(tok.tokv.str), "elif")) {
						if ((ppsrc->srcv.prep.ok_depth == ppsrc->srcv.prep.cond_depth - 1) && !ppsrc->srcv.prep.entered_next_ok_cond) {
							string_del(tok.tokv.str);
							VECTOR(preproc) *cond = vector_new(preproc);
							if (!cond) {
								log_memory("failed to allocate #elif condition vector\n");
								src->st = PPST_NONE;
								return (proc_token_t){ .tokt = PTOK_INVALID, .loginfo = tok.loginfo, .tokv = {.c = '\0'} };
							}
							tok = ppsrc_next_token(src);
							loginfo_t li = tok.loginfo;
							while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
								li.lineno_end = tok.loginfo.lineno_end ? tok.loginfo.lineno_end : tok.loginfo.lineno;
								li.colno_end = tok.loginfo.colno_end ? tok.loginfo.colno_end : tok.loginfo.colno;
								if (!vector_push(preproc, cond, tok)) {
									log_memory("failed to add token to #elif condition vector\n");
									vector_del(preproc, cond);
									src->st = PPST_NONE;
									return (proc_token_t){ .tokt = PTOK_INVALID, .loginfo = tok.loginfo, .tokv = {.c = '\0'} };
								}
								tok = ppsrc_next_token(src);
							}
							vector_trim(preproc, cond);
							khash_t(string_set) *solved_macros = kh_init(string_set);
							if (!solved_macros) {
								log_memory("failed to allocate #elif solved_macros set\n");
								vector_del(preproc, cond);
								src->st = PPST_NONE;
								return (proc_token_t){ .tokt = PTOK_INVALID, .loginfo = tok.loginfo, .tokv = {.c = '\0'} };
							}
							
							VECTOR(preproc) *expanded = preproc_do_expand(&li, src->macros_map, cond, solved_macros, src->macros_used);
							vector_del(preproc, cond);
							macros_set_del(solved_macros);
							if (!expanded) {
								// Not required as preproc_do_expand already prints an error, but this can be useful
								log_error(&li, "failed to expand #elif condition\n");
								src->st = PPST_NONE;
								return (proc_token_t){ .tokt = PTOK_INVALID, .loginfo = tok.loginfo, .tokv = {.c = '\0'} };
							}
							
							// Now we need to compute what is pointed by expanded, and increase cond_depth and ok_depth as needed
							int st;
							int64_t res = preproc_eval(expanded, &st, src->target->size_long == 4);
							vector_del(preproc, expanded);
							if (!st) {
								// Not required as preproc_eval already prints an error, but this can be useful
								log_error(&li, "failed to evaluate #elif condition\n");
								src->st = PPST_NONE;
								return (proc_token_t){ .tokt = PTOK_INVALID, .tokv = {.c = '\0'} };
							}
							if (res) {
								++vector_last(ppsource, src->prep).srcv.prep.ok_depth;
								if (tok.tokt == PPTOK_NEWLINE) {
									src->st = PPST_NL;
									goto check_next_token;
								}
							} else {
								if (tok.tokt == PPTOK_NEWLINE) {
									src->st = PPST_NL;
									continue;
								}
							}
							if (tok.tokt == PPTOK_EOF) {
								log_warning(&tok.loginfo, "file ended before closing all conditionals (ignoring)\n");
								vector_pop(ppsource, src->prep);
								src->st = PPST_NL; // Should be redundant since TOK_NEWLINE is added before TOK_EOF if required
								// EOF has an empty destructor
								// Note that since we have opened the file, the previous file had ok_depth == cond_depth
								goto check_next_token;
							} else /* if (tok.tokt == PPTOK_INVALID) */ {
								src->st = PPST_NONE;
								return (proc_token_t){ .tokt = PTOK_INVALID, .tokv.c = tok.tokv.c };
							}
						} else goto preproc_ignore_remaining;
					} else if (!strcmp(string_content(tok.tokv.str), "elifdef")) {
						string_del(tok.tokv.str);
						tok = ppsrc_next_token(src);
						if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
							log_error(&tok.loginfo, "invalid token type %u after '#elifdef' preprocessor command\n", tok.tokt);
							goto preproc_ignore_remaining;
						}
						if ((ppsrc->srcv.prep.ok_depth == ppsrc->srcv.prep.cond_depth - 1) && !ppsrc->srcv.prep.entered_next_ok_cond) {
							// We may enter the following block, check it
							khiter_t it = kh_get(macros_map, src->macros_map, string_content(tok.tokv.str));
							if (it != kh_end(src->macros_map)) {
								++ppsrc->srcv.prep.ok_depth;
								goto preproc_ignore_remaining_goto;
							}
							goto preproc_ignore_remaining;
						}
					} else if (!strcmp(string_content(tok.tokv.str), "elifndef")) {
						string_del(tok.tokv.str);
						tok = ppsrc_next_token(src);
						if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
							log_error(&tok.loginfo, "invalid token type %u after '#elifdef' preprocessor command\n", tok.tokt);
							goto preproc_ignore_remaining;
						}
						if ((ppsrc->srcv.prep.ok_depth == ppsrc->srcv.prep.cond_depth - 1) && !ppsrc->srcv.prep.entered_next_ok_cond) {
							// We may enter the following block, check it
							khiter_t it = kh_get(macros_map, src->macros_map, string_content(tok.tokv.str));
							if (it == kh_end(src->macros_map)) {
								++ppsrc->srcv.prep.ok_depth;
								goto preproc_ignore_remaining_goto;
							}
							goto preproc_ignore_remaining;
						}
					} else if (!strcmp(string_content(tok.tokv.str), "else")) {
						// Maybe increase ok_depth if ok_depth = cond_depth - 1; also goto check_if_depth
						// Also, we only need to goto check_if_depth if we actually update ok_depth
						// Note that this (very naive) approach allows code such as:
						/* #ifdef M
						 * ... // Not preprocessed                       *** Preprocessed
						 * #else
						 * ... // Preprocessed                           *** Not preprocessed
						 * #else
						 * ... // Not preprocessed due to unrelated code *** Not preprocessed
						 * #else
						 * ... // Preprocessed                           *** Not preprocessed
						 * #endif
						 */
						// Forbidding this code would require a 256-bits bitfield, which is big, thus not implemented.
						if ((ppsrc->srcv.prep.ok_depth == ppsrc->srcv.prep.cond_depth - 1) && !ppsrc->srcv.prep.entered_next_ok_cond) {
							++ppsrc->srcv.prep.ok_depth;
							goto preproc_ignore_remaining_goto;
						} else goto preproc_ignore_remaining;
					} else if (!strcmp(string_content(tok.tokv.str), "endif")) {
						// Decrease cond_depth; also goto check_if_depth
						// Note that 0 <= ppsrc->srcv.prep.ok_depth < ppsrc->srcv.prep.cond_depth, so this code is OK
						--ppsrc->srcv.prep.cond_depth;
						goto preproc_ignore_remaining_goto;
					}
					
					log_warning(&tok.loginfo, "Unknown ignored pp command %s, skipping until EOL\n", string_content(tok.tokv.str));
				preproc_ignore_remaining:
					while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
						preproc_token_del(&tok);
						tok = ppsrc_next_token(src);
					}
					if (tok.tokt == PPTOK_NEWLINE) {
						src->st = PPST_NL;
					}
					goto skip_cur_token;
					
				preproc_ignore_remaining_goto:
					while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
						preproc_token_del(&tok);
						tok = ppsrc_next_token(src);
					}
					if (tok.tokt == PPTOK_NEWLINE) {
						src->st = PPST_NL;
					}
					goto check_if_depth;
				} else {
					src->st = PPST_NONE;
					preproc_token_del(&tok);
				}
			}
		}
	}
	proc_token_t ret;
check_next_token:
	if (!vector_size(ppsource, src->prep)) {
		ret.tokt = PTOK_EOF;
		ret.loginfo = (loginfo_t){0};
		ret.tokv.c = (char)EOF;
		return ret;
	}
start_next_token:
	preproc_token_t tok = ppsrc_next_token(src);
start_cur_token:
	switch (tok.tokt) {
	case PPTOK_INVALID:
		src->st = PPST_NONE;
		ret.tokt = PTOK_INVALID;
		ret.loginfo = tok.loginfo;
		ret.tokv.c = tok.tokv.c;
		return ret;
	case PPTOK_IDENT:
		src->st = (src->st == PPST_PRAGMA_EXPLICIT) ? PPST_PRAGMA_EXPLICIT : PPST_NONE;
		{
			khiter_t it = kh_get(macros_map, src->macros_map, string_content(tok.tokv.str));
			if (it != kh_end(src->macros_map)) {
				macro_t *m = &kh_val(src->macros_map, it);
				int need_solve = !m->is_funlike;
				VECTOR(preprocs) *margs = NULL;
				loginfo_t li = tok.loginfo;
				if (m->is_funlike) {
					preproc_token_t tok2 = ppsrc_next_token(src);
					size_t nnls = 0;
					while (tok2.tokt == PPTOK_NEWLINE) {
						++nnls;
						tok2 = ppsrc_next_token(src);
					}
					if ((tok2.tokt == PPTOK_SYM) && (tok2.tokv.sym == SYM_LPAREN)) {
						need_solve = 1;
						margs = vector_new(preprocs);
						if (!margs) goto solve_err_mem;
						VECTOR(preproc) *marg = vector_new(preproc);
						if (!marg) goto solve_err_mem;
						while (need_solve && (tok2.tokt != PPTOK_EOF) && (tok2.tokt != PPTOK_INVALID)) {
							tok2 = ppsrc_next_token(src);
							if ((need_solve == 1) && (tok2.tokt == PPTOK_SYM) && (tok2.tokv.sym == SYM_COMMA)) {
								// Possible optimization: emplace NULL if vector_size(marg) == 0
								// This would avoid allocating a new vector
								vector_trim(preproc, marg);
								if (!vector_push(preprocs, margs, marg)) {
									vector_del(preproc, marg);
									goto solve_err_mem;
								}
								marg = vector_new(preproc);
								if (!marg) goto solve_err_mem;
							} else if (tok2.tokt != PPTOK_NEWLINE) {
								if (!vector_push(preproc, marg, tok2)) {
									vector_del(preproc, marg);
									goto solve_err_mem;
								}
							}
							if ((tok2.tokt == PPTOK_SYM) && (tok2.tokv.sym == SYM_LPAREN)) ++need_solve;
							else if ((tok2.tokt == PPTOK_SYM) && (tok2.tokv.sym == SYM_RPAREN)) --need_solve;
						}
						li.lineno_end = tok2.loginfo.lineno_end ? tok2.loginfo.lineno_end : tok2.loginfo.lineno;
						li.colno_end = tok2.loginfo.colno_end ? tok2.loginfo.colno_end : tok2.loginfo.colno;
						if (need_solve) {
							log_error(&li, "unfinished function-like macro %s\n", string_content(tok.tokv.str));
							vector_del(preprocs, margs);
							vector_del(preproc, marg);
							string_del(tok.tokv.str);
							src->st = PPST_NONE;
							ret.tokt = PTOK_INVALID;
							ret.loginfo = tok2.loginfo;
							ret.tokv.c = tok.tokv.c;
							return ret;
						}
						// margs finishes with a SYM_RPAREN token
						vector_pop(preproc, marg);
						vector_trim(preproc, marg);
						if (!vector_push(preprocs, margs, marg)) {
							vector_del(preproc, marg);
							goto solve_err_mem;
						}
						need_solve = 1;
					} else {
						if (!vector_reserve(ppsource, src->prep, vector_size(ppsource, src->prep) + nnls + 1)) {
							log_memory("undoing lookahead for macro use %s\n", string_content(tok.tokv.str));
							string_del(tok.tokv.str);
							preproc_token_del(&tok2);
							src->st = PPST_NONE;
							ret.tokt = PTOK_INVALID;
							ret.loginfo = li;
							ret.tokv.c = '\0';
							return ret;
						}
						vector_push(ppsource, src->prep, ((ppsource_t){.srct = PPSRC_PPTOKEN, .srcv.pptok = tok2}));
						while (nnls--) {
							vector_push(
								ppsource,
								src->prep,
								((ppsource_t){.srct = PPSRC_PPTOKEN, .srcv.pptok = {.tokt = PPTOK_NEWLINE, .tokv.c = '\n'}}));
						}
					}
				}
				if (need_solve) {
					khash_t(string_set) *solved_macros = kh_init(string_set);
					if (!solved_macros) goto solve_err_mem;
					
					char *mname = string_steal(tok.tokv.str);
					
					VECTOR(preproc) *solved = preproc_solve_macro(&li, src->macros_map, mname, m, margs, solved_macros, NULL);
					if (margs) vector_del(preprocs, margs);
					macros_set_del(solved_macros);
					if (!solved) {
						src->st = PPST_NONE;
						ret.tokt = PTOK_INVALID;
						ret.loginfo = li;
						ret.tokv.c = '\0';
						return ret;
					}
					// If the expansion is empty, don't push it
					if (vector_size(preproc, solved)) {
						if (!vector_push(ppsource, src->prep, ((ppsource_t){.srct = PPSRC_PPTOKENS, .srcv.pptoks = {.idx = 0, .toks = solved}}))) {
							log_memory("pushing expanded macro\n");
							vector_del(preproc, solved);
							src->st = PPST_NONE;
							ret.tokt = PTOK_INVALID;
							ret.loginfo = li;
							ret.tokv.c = '\0';
							return ret;
						}
					} else {
						vector_del(preproc, solved);
					}
					// src->prep is not empty (either through the push or by not popping the top)
					goto start_next_token;
				} else {
					if (margs) vector_del(preprocs, margs);
				}
				
				if (0) {
				solve_err_mem:
					log_memory("parsing macro use %s\n", string_content(tok.tokv.str));
					if (margs) vector_del(preprocs, margs);
					string_del(tok.tokv.str);
					src->st = PPST_NONE;
					ret.tokt = PTOK_INVALID;
					ret.loginfo = li;
					ret.tokv.c = '\0';
					return ret;
				}
			}
		}
		/* FALLTHROUGH */
	case PPTOK_IDENT_UNEXP: {
		khiter_t it = kh_get(str2kw, str2kw, string_content(tok.tokv.str));
		if (it == kh_end(str2kw)) {
			ret.tokt = PTOK_IDENT;
			ret.tokv.str = tok.tokv.str;
		} else {
			string_del(tok.tokv.str);
			ret.tokt = PTOK_KEYWORD;
			ret.tokv.kw = kh_val(str2kw, it);
		}
		ret.loginfo = tok.loginfo;
		return ret; }
	case PPTOK_NUM:
		src->st = (src->st == PPST_PRAGMA_EXPLICIT) ? PPST_PRAGMA_EXPLICIT : PPST_NONE;
		ret.tokt = PTOK_NUM;
		ret.loginfo = tok.loginfo;
		ret.tokv.str = tok.tokv.str;
		return ret;
	case PPTOK_STRING:
		src->st = (src->st == PPST_PRAGMA_EXPLICIT) ? PPST_PRAGMA_EXPLICIT : PPST_NONE;
		ret.tokt = PTOK_STRING;
		ret.loginfo = tok.loginfo;
		ret.tokv = (union proc_token_val_u){.sstr = tok.tokv.sstr, .sisstr = tok.tokv.sisstr};
		return ret;
	case PPTOK_INCL:
		src->st = PPST_NONE;
		ret.tokt = PTOK_INVALID;
		ret.loginfo = tok.loginfo;
		ret.tokv.c = tok.tokv.sisstr ? '<' : '"';
		string_del(tok.tokv.sstr);
		return ret;
	case PPTOK_SYM:
		if ((src->st == PPST_NL) && (tok.tokv.sym == SYM_HASH)) {
			tok = ppsrc_next_token(src);
			if ((tok.tokt == PPTOK_NEWLINE) || (tok.tokt == PPTOK_EOF)) {
				// Empty preprocessor command
				goto start_cur_token;
			}
			if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) goto preproc_hash_err;
			if (!strcmp(string_content(tok.tokv.str), "include") || !strcmp(string_content(tok.tokv.str), "include_next")) {
				int is_next = string_content(tok.tokv.str)[7] == '_';
				string_del(tok.tokv.str);
				tok = ppsrc_next_token(src);
				loginfo_t li = tok.loginfo;
				string_t *incl_file;
				int is_sys;
				if (tok.tokt == PPTOK_INCL) {
					incl_file = tok.tokv.sstr;
					// Assume we only have one #include "..." path, so include_next is always a system include
					is_sys = is_next || !tok.tokv.sisstr;
					tok = ppsrc_next_token(src); // Token was moved
					loginfo_t ignored_infos = tok.loginfo; int has_ignored = 0;
					while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
						has_ignored = 1;
						ignored_infos.lineno_end = tok.loginfo.lineno_end ? tok.loginfo.lineno_end : tok.loginfo.lineno;
						ignored_infos.colno_end = tok.loginfo.colno_end ? tok.loginfo.colno_end : tok.loginfo.colno;
						preproc_token_del(&tok);
						tok = ppsrc_next_token(src);
					}
					if (has_ignored) {
						log_warning(&ignored_infos, "ignored tokens after #include%s directive\n", is_next ? "_next" : "");
					}
				} else {
					// Expand macro, then try again
					VECTOR(preproc) *incl = vector_new(preproc);
					if (!incl) {
						log_memory("failed to allocate #include tokens vector\n");
						src->st = PPST_NONE;
						ret.tokt = PTOK_INVALID;
						ret.loginfo = tok.loginfo;
						ret.tokv.c = '\0';
						return ret;
					}
					while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
						if (!vector_push(preproc, incl, tok)) {
							log_memory("failed to add token to #include tokens vector\n");
							vector_del(preproc, incl);
							src->st = PPST_NONE;
							ret.tokt = PTOK_INVALID;
							ret.loginfo = tok.loginfo;
							ret.tokv.c = '\0';
							return ret;
						}
						li.lineno_end = tok.loginfo.lineno_end ? tok.loginfo.lineno_end : tok.loginfo.lineno;
						li.colno_end = tok.loginfo.colno_end ? tok.loginfo.colno_end : tok.loginfo.colno;
						tok = ppsrc_next_token(src);
					}
					vector_trim(preproc, incl);
					khash_t(string_set) *solved_macros = kh_init(string_set);
					if (!solved_macros) {
						log_memory("failed to allocate #include solved_macros set\n");
						vector_del(preproc, incl);
						src->st = PPST_NONE;
						ret.tokt = PTOK_INVALID;
						ret.loginfo = tok.loginfo;
						ret.tokv.c = '\0';
						return ret;
					}
					
					VECTOR(preproc) *expanded = preproc_do_expand(&li, src->macros_map, incl, solved_macros, src->macros_used);
					vector_del(preproc, incl);
					macros_set_del(solved_macros);
					if (!expanded) {
						log_error(&li, "failed to expand #include tokens\n");
						src->st = PPST_NONE;
						ret.tokt = PTOK_INVALID;
						ret.tokv.c = '\0';
						return ret;
					}
					
					// Now we need to check what is pointed by expanded
					if (!vector_size(preproc, expanded)) {
						log_error(&li, "missing #include name\n");
						src->st = PPST_NONE;
						ret.tokt = PTOK_INVALID;
						ret.loginfo = tok.loginfo;
						ret.tokv.c = '\0';
						return ret;
					}
					if (vector_content(preproc, expanded)[0].tokt == PPTOK_STRING) {
						is_sys = is_next || src->is_sys;
						preproc_token_t *exp = vector_content(preproc, expanded);
						log_error(&li, "TODO: #include <expanded, first is string '%s' (%d)>\n", string_content(exp->tokv.sstr), exp->tokv.sisstr);
						vector_del(preproc, expanded);
						src->st = PPST_NONE;
						ret.tokt = PTOK_INVALID;
						ret.loginfo = tok.loginfo;
						ret.tokv.c = '\0';
						return ret;
					} else if ((vector_content(preproc, expanded)[0].tokt == PPTOK_SYM) && (vector_content(preproc, expanded)[0].tokv.sym == SYM_LT)
					        && (vector_last(preproc, expanded).tokt == PPTOK_SYM) && (vector_last(preproc, expanded).tokv.sym == SYM_GT)
					        && (vector_size(preproc, expanded) >= 3)) {
						log_warning(&li, "#include command with macro expansion, assuming no space is present in the file name\n");
						is_sys = 0;
						incl_file = string_new();
						for (vector_preproc_elem *tok2 = expanded->content + 1; tok2 < expanded->content + expanded->vsize - 1; ++tok2) {
							switch (tok2->tokt) {
							case PPTOK_IDENT:
							case PPTOK_IDENT_UNEXP:
								if (!string_add_string(incl_file, tok2->tokv.str)) {
									log_memory("failed to add ident to include string\n");
									vector_del(preproc, expanded);
									string_del(incl_file);
									src->st = PPST_NONE;
									ret.tokt = PTOK_INVALID;
									ret.loginfo = tok.loginfo;
									ret.tokv.c = '\0';
									return ret;
								}
								break;
							case PPTOK_SYM:
								if (!string_add_cstr(incl_file, sym2str[tok2->tokv.sym])) {
									log_memory("failed to add symbol to include string\n");
									vector_del(preproc, expanded);
									string_del(incl_file);
									src->st = PPST_NONE;
									ret.tokt = PTOK_INVALID;
									ret.loginfo = tok.loginfo;
									ret.tokv.c = '\0';
									return ret;
								}
								break;
								
							case PPTOK_INVALID:
							case PPTOK_NUM:
							case PPTOK_STRING:
							case PPTOK_INCL:
							case PPTOK_NEWLINE:
							case PPTOK_BLANK:
							case PPTOK_START_LINE_COMMENT:
							case PPTOK_EOF:
							default:
								log_error(&li, "TODO: add token type %u to include string\n", tok2->tokt);
								vector_del(preproc, expanded);
								string_del(incl_file);
								src->st = PPST_NONE;
								ret.tokt = PTOK_INVALID;
								ret.loginfo = tok.loginfo;
								ret.tokv.c = '\0';
								return ret;
							}
						}
						vector_del(preproc, expanded);
					} else {
						log_error(&li, "invalid #include command (macro expansion does not result in string or <...>)\n");
						vector_del(preproc, expanded);
						src->st = PPST_NONE;
						ret.tokt = PTOK_INVALID;
						ret.loginfo = tok.loginfo;
						ret.tokv.c = '\0';
						return ret;
					}
				}
				// cur_pathno == 0 if cur_file was from an #include "...", otherwise idx + 1
#ifdef LOG_INCLUDE
				printf("Opening %s as %s from system path %zu\n", string_content(incl_file),
				       is_sys ? "system header" : "cur header", is_next ? src->cur_pathno : 0);
#endif
				if ((is_sys || !try_open_dir(src, incl_file)) && !try_open_sys(src, incl_file, is_next ? src->cur_pathno : 0)) {
					log_error(&li, "failed to open %s\n", string_content(incl_file));
					string_del(incl_file);
					src->st = PPST_NONE;
					ret.tokt = PTOK_INVALID;
					ret.loginfo = li;
					ret.tokv.c = is_sys ? '<' : '"';
					return ret;
				}
				string_del(incl_file);
				if (tok.tokt == PPTOK_NEWLINE) goto check_next_token;
				else goto start_cur_token;
			} else if (!strcmp(string_content(tok.tokv.str), "define")) {
				string_del(tok.tokv.str);
				tok = ppsrc_next_token(src);
				if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
					log_error(&tok.loginfo, "invalid token type %u after '#define' preprocessor command\n", tok.tokt);
					goto preproc_hash_err;
				}
				string_t *defname = tok.tokv.str;
				macro_t m = (macro_t){ .is_funlike = 0, .has_varargs = 0, .nargs = 0, .toks = vector_new(mtoken) };
				if (!m.toks) {
					log_memory("failed to allocate token vector for macro %s\n", string_content(defname));
					string_del(defname); // Token is now freed
					ret.tokt = PTOK_INVALID;
					ret.loginfo = tok.loginfo;
					ret.tokv.c = '\0';
					return ret;
				}
				khash_t(argid_map) *args = NULL;
				tok = ppsrc_next_token(src);
				if ((tok.tokt == PPTOK_SYM) && (tok.tokv.sym == SYM_LPAREN)) {
					m.is_funlike = 1;
					args = kh_init(argid_map);
					if (!args) {
						log_memory("failed to allocate args map for macro %s\n", string_content(defname));
						string_del(defname); // Token is now freed
						vector_del(mtoken, m.toks);
						ret.tokt = PTOK_INVALID;
						ret.loginfo = tok.loginfo;
						ret.tokv.c = '\0';
						return ret;
					}
					m.nargs = 0;
					tok = ppsrc_next_token(src);
					int ok;
					if ((tok.tokt == PPTOK_SYM) && (tok.tokv.sym == SYM_RPAREN)) {
						ok = 1;
					} else {
						ok = 0;
						while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
							if ((tok.tokt == PPTOK_SYM) && (tok.tokv.sym == SYM_VARIADIC)) {
								m.has_varargs = 1;
								int kh_ret;
								char *tok_str = strdup("__VA_ARGS__");
								khiter_t kh_k = kh_put(argid_map, args, tok_str, &kh_ret); // Moves the string content
								if (kh_ret < 0) { // Failed to move, needs to free here
									log_memory("failed to push arg %s for macro %s\n", tok_str, string_content(defname));
									string_del(defname);
									free(tok_str);
									vector_del(mtoken, m.toks);
									argid_map_del(args);
									ret.tokt = PTOK_INVALID;
									ret.loginfo = tok.loginfo;
									ret.tokv.c = '\0';
									return ret;
								}
								if (kh_ret == 0) {
									log_error(&tok.loginfo, "duplicate argument name %s defining macro %s\n", tok_str, string_content(defname));
									string_del(defname);
									vector_del(mtoken, m.toks);
									argid_map_del(args);
									goto preproc_hash_err;
								}
								kh_val(args, kh_k) = m.nargs;
								// Empty token destructor
								tok = ppsrc_next_token(src);
								if ((tok.tokt != PPTOK_SYM) || (tok.tokv.sym != SYM_RPAREN)) {
									log_error(&tok.loginfo, "invalid token type %u after variadic macro arguments definition\n", tok.tokt);
									string_del(defname);
									vector_del(mtoken, m.toks);
									argid_map_del(args);
									goto preproc_hash_err;
								}
								ok = 1;
								break;
							} else if ((tok.tokt == PPTOK_IDENT) || (tok.tokt == PPTOK_IDENT_UNEXP)) {
								int kh_ret;
								char *tok_str = string_steal(tok.tokv.str);
								khiter_t kh_k = kh_put(argid_map, args, tok_str, &kh_ret); // Moves the string content
								if (kh_ret < 0) { // Failed to move, needs to free here
									log_memory("failed to push arg %s for macro %s\n", tok_str, string_content(defname));
									string_del(defname);
									free(tok_str);
									vector_del(mtoken, m.toks);
									argid_map_del(args);
									ret.tokt = PTOK_INVALID;
									ret.loginfo = tok.loginfo;
									ret.tokv.c = '\0';
									return ret;
								}
								if (kh_ret == 0) {
									log_error(&tok.loginfo, "duplicate argument name %s defining macro %s\n", tok_str, string_content(defname));
									string_del(defname);
									vector_del(mtoken, m.toks);
									argid_map_del(args);
									// Token was already deleted, create a fake token
									tok.tokt = PPTOK_SYM;
									tok.tokv.sym = LAST_SYM;
									goto preproc_hash_err;
								}
								kh_val(args, kh_k) = m.nargs++;
								// Token content is the string, which is moved to the vector
								tok = ppsrc_next_token(src);
								if ((tok.tokt == PPTOK_SYM) && (tok.tokv.sym == SYM_COMMA)) {
									// Empty destructor
									tok = ppsrc_next_token(src);
								} else if ((tok.tokt == PPTOK_SYM) && (tok.tokv.sym == SYM_RPAREN)) {
									// Empty destructor
									ok = 1;
									break;
								} else if ((tok.tokt == PPTOK_SYM) && (tok.tokv.sym == SYM_VARIADIC)) {
									m.has_varargs = 1;
									--m.nargs;
									// Empty token destructor
									tok = ppsrc_next_token(src);
									if ((tok.tokt != PPTOK_SYM) || (tok.tokv.sym != SYM_RPAREN)) {
										log_error(&tok.loginfo, "invalid token type %u after variadic macro arguments definition\n", tok.tokt);
										string_del(defname);
										vector_del(mtoken, m.toks);
										argid_map_del(args);
										goto preproc_hash_err;
									}
									ok = 1;
									break;
								} else {
									log_error(&tok.loginfo, "invalid %s type %u in macro arguments definition\n",
										(tok.tokt == PPTOK_SYM) ? "symbol" : "token",
										(tok.tokt == PPTOK_SYM) ? tok.tokv.sym : tok.tokt);
									string_del(defname);
									vector_del(mtoken, m.toks);
									argid_map_del(args);
									goto preproc_hash_err;
								}
							} else {
								log_error(&tok.loginfo, "invalid %s type %u as macro arguments definition name\n",
									(tok.tokt == PPTOK_SYM) ? "symbol" : "token", (tok.tokt == PPTOK_SYM) ? tok.tokv.sym : tok.tokt);
								string_del(defname);
								vector_del(mtoken, m.toks);
								argid_map_del(args);
								goto preproc_hash_err;
							}
						}
					}
					if (!ok) {
						log_error(&tok.loginfo, "invalid macro definition for %s\n", string_content(defname));
						string_del(defname);
						vector_del(mtoken, m.toks);
						argid_map_del(args);
						goto preproc_hash_err;
					}
					// tok is ')', empty destructor
					tok = ppsrc_next_token(src);
				}
				if (tok.tokt == PPTOK_BLANK) {
					// BLANK has no destructor
					tok = ppsrc_next_token(src);
				}
				// All tokens until INVALID/NL/EOF are macro content
				int state = 0;
#define ST_CONCAT 1
#define ST_STR 2
				while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
					if ((tok.tokt == PPTOK_SYM) && (tok.tokv.sym == SYM_HASH)) {
						if (state & ST_STR) {
							log_warning(&tok.loginfo, "duplicated stringify in macro definition (defining %s)\n", string_content(defname));
						} else {
							state |= ST_STR;
						}
						// Empty destructor
						tok = ppsrc_next_token(src);
						continue;
					}
					if ((tok.tokt == PPTOK_SYM) && (tok.tokv.sym == SYM_HASHHASH)) {
						if (state & ST_CONCAT) {
							log_warning(&tok.loginfo, "duplicated concatenation in macro definition (defining %s)\n", string_content(defname));
						} else if (!vector_size(mtoken, m.toks)) {
							log_warning(&tok.loginfo, "invalid concatenation at start of macro definition (defining %s)\n", string_content(defname));
						} else {
							state |= ST_CONCAT;
						}
						// Empty destructor
						tok = ppsrc_next_token(src);
						continue;
					}
					mtoken_t *mtok;
					unsigned argid = -1u;
					if (m.is_funlike && ((tok.tokt == PPTOK_IDENT) || (tok.tokt == PPTOK_IDENT_UNEXP))) {
						khiter_t kh_k = kh_get(argid_map, args, string_content(tok.tokv.str));
						if (kh_k != kh_end(args)) {
							string_del(tok.tokv.str); // Token freed
							argid = kh_val(args, kh_k);
						}
					}
					if (argid != -1u) {
						mtok = mtoken_new_arg(argid, state & ST_STR);
						if (!mtok) {
							log_memory("failed to allocate new m-token (defining %s)\n", string_content(defname));
							string_del(defname);
							macro_del(&m);
							src->st = PPST_NONE;
							ret.tokt = PTOK_INVALID;
							ret.loginfo = tok.loginfo;
							ret.tokv.c = tok.tokv.c;
							return ret;
						}
						state &= ~ST_STR;
					} else {
						mtok = mtoken_new_token(tok); // Token moved
						if (!mtok) {
							log_memory("failed to allocate new m-token (defining %s)\n", string_content(defname));
							string_del(defname);
							macro_del(&m);
							if (tok.tokt == PPTOK_NEWLINE) goto check_next_token;
							else goto start_cur_token;
						}
						if (state & ST_STR) {
							log_warning(&tok.loginfo, "invalid stringify before token (defining %s)\n", string_content(defname));
							state &= ~ST_STR;
						}
					}
					if (state & ST_CONCAT) {
						mtoken_t *mtok2 = vector_last(mtoken, m.toks); // Guaranteed to exist before setting ST_CONCAT
						mtok = mtoken_new_concat(mtok2, mtok);
						if (!mtok) {
							log_memory("failed to allocate new m-token concatenation (defining %s)\n", string_content(defname));
							string_del(defname);
							macro_del(&m);
							vector_pop_nodel(mtoken, m.toks);
							src->st = PPST_NONE;
							ret.tokt = PTOK_INVALID;
							ret.loginfo = tok.loginfo;
							ret.tokv.c = tok.tokv.c;
							return ret;
						}
						vector_last(mtoken, m.toks) = mtok;
						state &= ~ST_CONCAT;
					} else {
						if (!vector_push(mtoken, m.toks, mtok)) {
							log_memory("failed to add m-token (defining %s)\n", string_content(defname));
							string_del(defname);
							macro_del(&m);
							src->st = PPST_NONE;
							ret.tokt = PTOK_INVALID;
							ret.loginfo = tok.loginfo;
							ret.tokv.c = tok.tokv.c;
							return ret;
						}
					}
					// mtok moved to the vector
					tok = ppsrc_next_token(src);
				}
#undef ST_CONCAT
#undef ST_STR
				if (args) argid_map_del(args);
				if (tok.tokt == PPTOK_INVALID) {
					// Abort
					log_error(&tok.loginfo, "unexpected invalid input token\n");
					string_del(defname);
					macro_del(&m);
					src->st = PPST_NONE;
					ret.tokt = PTOK_INVALID;
					ret.loginfo = tok.loginfo;
					ret.tokv.c = tok.tokv.c;
					return ret;
				} else {
					// NL and EOF have empty destructors
					khiter_t kh_k;
					int iret;
					char *mname_dup = string_steal(defname);
					kh_k = kh_put(string_set, src->macros_defined, mname_dup, &iret);
					if (iret < 0) {
						// Abort
						log_memory("failed to remember macro %s as defined, aborting\n", mname_dup);
						free(mname_dup);
						macro_del(&m);
						src->st = PPST_NONE;
						ret.tokt = PTOK_INVALID;
						ret.loginfo = tok.loginfo;
						ret.tokv.c = tok.tokv.c;
						return ret;
					} else if (iret > 0) {
						mname_dup = strdup(mname_dup);
					}
					kh_k = kh_put(macros_map, src->macros_map, mname_dup, &iret);
					if (iret < 0) {
						// Abort
						log_memory("failed to remember macro %s, aborting\n", mname_dup);
						free(mname_dup);
						macro_del(&m);
						src->st = PPST_NONE;
						ret.tokt = PTOK_INVALID;
						ret.loginfo = tok.loginfo;
						ret.tokv.c = tok.tokv.c;
						return ret;
					} else if (iret == 0) {
						// Ignore
						// Too noisy  log_warning(&tok.loginfo, "Duplicated macro %s\n", mname_dup);
						free(mname_dup);
						macro_del(&m);
					} else {
						kh_val(src->macros_map, kh_k) = m;
					}
					if (tok.tokt == PPTOK_NEWLINE) goto check_next_token;
					else goto start_cur_token;
				}
			} else if (!strcmp(string_content(tok.tokv.str), "undef")) {
				string_del(tok.tokv.str);
				tok = ppsrc_next_token(src);
				if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
					log_error(&tok.loginfo, "invalid token type %u after '#undef' preprocessor command\n", tok.tokt);
					goto preproc_hash_err;
				}
				string_t *mname = tok.tokv.str;
				tok = ppsrc_next_token(src); // Token was moved
				while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
					// TODO: Print warning 'ignored token(s)'
					preproc_token_del(&tok);
					tok = ppsrc_next_token(src);
				}
				khiter_t it = kh_get(macros_map, src->macros_map, string_content(mname));
				if (it != kh_end(src->macros_map)) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
					free((void*)kh_key(src->macros_map, it));
#pragma GCC diagnostic pop
					macro_del(&kh_val(src->macros_map, it));
					kh_del(macros_map, src->macros_map, it);
				}
				string_del(mname);
				if (tok.tokt == PPTOK_NEWLINE) goto check_next_token;
				else goto start_cur_token;
			} else if (!strcmp(string_content(tok.tokv.str), "error")) {
				log_error(&tok.loginfo, "#error command:");
				string_del(tok.tokv.str);
				tok = ppsrc_next_token(src);
				while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
					switch (tok.tokt) {
					case PPTOK_IDENT:
					case PPTOK_IDENT_UNEXP:
						printf(" %s", string_content(tok.tokv.str));
						break;
					case PPTOK_SYM:
						printf("%s%s", (tok.tokv.sym == SYM_COMMA) ? "" : " ", sym2str[tok.tokv.sym]);
						break;
					case PPTOK_STRING:
						printf(" %c%s%c", tok.tokv.sisstr ? '"' : '\'', string_content(tok.tokv.sstr), tok.tokv.sisstr ? '"' : '\'');
						break;
						
					case PPTOK_INVALID:
					case PPTOK_NUM:
					case PPTOK_INCL:
					case PPTOK_NEWLINE:
					case PPTOK_BLANK:
					case PPTOK_START_LINE_COMMENT:
					case PPTOK_EOF:
					default:
						printf(" <unknown token type %u>", tok.tokt);
					}
					preproc_token_del(&tok);
					tok = ppsrc_next_token(src);
				}
				printf("\n");
				vector_clear(ppsource, src->prep);
				ret.tokt = PTOK_INVALID;
				ret.loginfo = tok.loginfo;
				ret.tokv.c = (char)EOF;
				return ret;
			} else if (!strcmp(string_content(tok.tokv.str), "warning")) {
				log_warning(&tok.loginfo, "#warning command:");
				tok = ppsrc_next_token(src);
				while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
					switch (tok.tokt) {
					case PPTOK_IDENT:
					case PPTOK_IDENT_UNEXP:
						printf(" %s", string_content(tok.tokv.str));
						break;
					case PPTOK_SYM:
						printf("%s%s", (tok.tokv.sym == SYM_COMMA) ? "" : " ", sym2str[tok.tokv.sym]);
						break;
					case PPTOK_STRING:
						printf(" %c%s%c", tok.tokv.sisstr ? '"' : '\'', string_content(tok.tokv.sstr), tok.tokv.sisstr ? '"' : '\'');
						break;
						
					case PPTOK_INVALID:
					case PPTOK_NUM:
					case PPTOK_INCL:
					case PPTOK_NEWLINE:
					case PPTOK_BLANK:
					case PPTOK_START_LINE_COMMENT:
					case PPTOK_EOF:
					default:
						printf(" <unknown token type %u>", tok.tokt);
					}
					preproc_token_del(&tok);
					tok = ppsrc_next_token(src);
				}
				printf("\n");
				goto preproc_hash_err;
			} else if (!strcmp(string_content(tok.tokv.str), "pragma")) {
				string_del(tok.tokv.str);
				tok = ppsrc_next_token(src);
				if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
					log_error(&tok.loginfo, "unknown pragma directive, skipping until EOL\n");
					goto preproc_hash_err;
				} else if (!strcmp(string_content(tok.tokv.str), "once")) {
					const char *fname = tok.loginfo.filename;
					if (!vector_push(ccharp, src->pragma_once, fname)) {
						log_memory("failed to add filename to #pragma once list\n");
						vector_clear(ppsource, src->prep);
						ret.tokt = PTOK_INVALID;
						ret.loginfo = tok.loginfo;
						ret.tokv.c = (char)EOF;
						return ret;
					}
					string_del(tok.tokv.str);
					tok = ppsrc_next_token(src);
					goto preproc_hash_err;
				} else if (!strcmp(string_content(tok.tokv.str), "wrappers")) {
					string_del(tok.tokv.str);
					tok = ppsrc_next_token(src);
					if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
						log_error(&tok.loginfo, "unknown pragma wrappers directive, skipping until EOL\n");
						goto preproc_hash_err;
					} else if (!strcmp(string_content(tok.tokv.str), "allow_ints_ext")) {
						ret.loginfo = tok.loginfo;
						while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
							preproc_token_del(&tok);
							tok = ppsrc_next_token(src);
						}
						if (tok.tokt == PPTOK_INVALID) goto start_cur_token;
						else {
							ret.tokt = PTOK_PRAGMA;
							ret.tokv.pragma.typ = PRAGMA_ALLOW_INTS;
							return ret;
						}
					} else if (!strcmp(string_content(tok.tokv.str), "mark_simple")) {
						string_del(tok.tokv.str);
						tok = ppsrc_next_token(src);
						if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
							log_error(&tok.loginfo, "invalid pragma wrappers explicit_simple directive, skipping until EOL\n");
							goto preproc_hash_err;
						}
						src->st = PPST_NL;
						ret.tokt = PTOK_PRAGMA;
						ret.loginfo = tok.loginfo;
						ret.tokv.pragma.typ = PRAGMA_SIMPLE_SU;
						ret.tokv.pragma.val = tok.tokv.str;
						tok = ppsrc_next_token(src);
						while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
							preproc_token_del(&tok);
							tok = ppsrc_next_token(src);
						}
						if (tok.tokt == PPTOK_INVALID) {
							string_del(ret.tokv.pragma.val);
							goto start_cur_token;
						} else {
							return ret;
						}
					} else if (!strcmp(string_content(tok.tokv.str), "type_letters")) {
						string_del(tok.tokv.str);
						tok = ppsrc_next_token(src);
						if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
							log_error(&tok.loginfo, "invalid pragma wrappers explicit_simple directive, skipping until EOL\n");
							goto preproc_hash_err;
						}
						src->st = PPST_PRAGMA_EXPLICIT;
						ret.tokt = PTOK_PRAGMA;
						ret.loginfo = tok.loginfo;
						ret.tokv.pragma.typ = PRAGMA_EXPLICIT_CONV;
						ret.tokv.pragma.val = tok.tokv.str;
						return ret;
					} else if (!strcmp(string_content(tok.tokv.str), "type_letters_exact")) {
						string_del(tok.tokv.str);
						tok = ppsrc_next_token(src);
						if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
							log_error(&tok.loginfo, "invalid pragma wrappers explicit_simple directive, skipping until EOL\n");
							goto preproc_hash_err;
						}
						src->st = PPST_PRAGMA_EXPLICIT;
						ret.tokt = PTOK_PRAGMA;
						ret.loginfo = tok.loginfo;
						ret.tokv.pragma.typ = PRAGMA_EXPLICIT_CONV_STRICT;
						ret.tokv.pragma.val = tok.tokv.str;
						return ret;
					} else {
						log_error(&tok.loginfo, "unknown pragma wrappers directive '%s', skipping until EOL\n", string_content(tok.tokv.str));
						goto preproc_hash_err;
					}
				} else {
					log_error(&tok.loginfo, "unknown pragma directive '%s', skipping until EOL\n", string_content(tok.tokv.str));
					goto preproc_hash_err;
				}
			} else if (!strcmp(string_content(tok.tokv.str), "if")) {
				if (vector_last(ppsource, src->prep).srct != PPSRC_PREPARE) {
					log_error(&tok.loginfo, "invalid #if source type %u\n", vector_last(ppsource, src->prep).srct);
					src->st = PPST_NONE;
					ret.tokt = PTOK_INVALID;
					ret.loginfo = tok.loginfo;
					ret.tokv.c = '\0';
					return ret;
				}
				string_del(tok.tokv.str);
				VECTOR(preproc) *cond = vector_new(preproc);
				if (!cond) {
					log_memory("failed to allocate #if condition vector\n");
					src->st = PPST_NONE;
					ret.tokt = PTOK_INVALID;
					ret.loginfo = tok.loginfo;
					ret.tokv.c = '\0';
					return ret;
				}
				tok = ppsrc_next_token(src);
				loginfo_t li = tok.loginfo;
				while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
					if (!vector_push(preproc, cond, tok)) {
						log_memory("failed to add token to #if condition vector\n");
						vector_del(preproc, cond);
						src->st = PPST_NONE;
						ret.tokt = PTOK_INVALID;
						ret.loginfo = tok.loginfo;
						ret.tokv.c = '\0';
						return ret;
					}
					li.lineno_end = tok.loginfo.lineno_end ? tok.loginfo.lineno_end : tok.loginfo.lineno;
					li.colno_end = tok.loginfo.colno_end ? tok.loginfo.colno_end : tok.loginfo.colno;
					tok = ppsrc_next_token(src);
				}
				vector_trim(preproc, cond);
				khash_t(string_set) *solved_macros = kh_init(string_set);
				if (!solved_macros) {
					log_memory("failed to allocate #if solved_macros set\n");
					vector_del(preproc, cond);
					src->st = PPST_NONE;
					ret.tokt = PTOK_INVALID;
					ret.loginfo = tok.loginfo;
					ret.tokv.c = '\0';
					return ret;
				}
				
				VECTOR(preproc) *expanded = preproc_do_expand(&li, src->macros_map, cond, solved_macros, src->macros_used);
				vector_del(preproc, cond);
				macros_set_del(solved_macros);
				if (!expanded) {
					log_error(&li, "Error: failed to expand #if condition\n");
					src->st = PPST_NONE;
					ret.tokt = PTOK_INVALID;
					ret.loginfo = tok.loginfo;
					ret.tokv.c = '\0';
					return ret;
				}
				
				// Now we need to compute what is pointed by expanded, and increase cond_depth and ok_depth as needed
				int st;
				int64_t res = preproc_eval(expanded, &st, src->target->size_long == 4);
				vector_del(preproc, expanded);
				if (!st) {
					log_error(&li, "Error: failed to evaluate #if condition\n");
					src->st = PPST_NONE;
					ret.tokt = PTOK_INVALID;
					ret.loginfo = tok.loginfo;
					ret.tokv.c = '\0';
					return ret;
				}
				++vector_last(ppsource, src->prep).srcv.prep.cond_depth;
				if (res) {
					++vector_last(ppsource, src->prep).srcv.prep.ok_depth;
					if (tok.tokt == PPTOK_NEWLINE) goto check_next_token;
					else goto start_cur_token;
				} else {
					vector_last(ppsource, src->prep).srcv.prep.entered_next_ok_cond = 0;
					if (tok.tokt == PPTOK_NEWLINE) goto check_if_depth;
					else goto start_cur_token; // Returns immediately
				}
			} else if (!strcmp(string_content(tok.tokv.str), "ifdef")) {
				if (vector_last(ppsource, src->prep).srct != PPSRC_PREPARE) {
					log_error(&tok.loginfo, "invalid #ifdef source type %u\n", vector_last(ppsource, src->prep).srct);
					goto preproc_hash_err;
				}
				string_del(tok.tokv.str);
				tok = ppsrc_next_token(src);
				if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
					log_error(&tok.loginfo, "invalid token type %u after '#ifdef' preprocessor command\n", tok.tokt);
					goto preproc_hash_err;
				}
				khiter_t it = kh_get(macros_map, src->macros_map, string_content(tok.tokv.str));
				int iret;
				char *mname = string_steal(tok.tokv.str);
				kh_put(string_set, src->macros_used, mname, &iret);
				if (iret <= 0) {
					free(mname);
				}
				tok.tokt = PPTOK_SYM;
				tok.tokv.sym = LAST_SYM;
				// We don't care about iret(?)
				// TODO: check iret, error if needed
				++vector_last(ppsource, src->prep).srcv.prep.cond_depth;
				if (it != kh_end(src->macros_map)) {
					++vector_last(ppsource, src->prep).srcv.prep.ok_depth;
					goto preproc_hash_err;
				} else {
					vector_last(ppsource, src->prep).srcv.prep.entered_next_ok_cond = 0;
					goto preproc_hash_err_goto;
				}
			} else if (!strcmp(string_content(tok.tokv.str), "ifndef")) {
				if (vector_last(ppsource, src->prep).srct != PPSRC_PREPARE) {
					log_error(&tok.loginfo, "invalid #ifndef source type %u\n", vector_last(ppsource, src->prep).srct);
					goto preproc_hash_err;
				}
				string_del(tok.tokv.str);
				tok = ppsrc_next_token(src);
				if ((tok.tokt != PPTOK_IDENT) && (tok.tokt != PPTOK_IDENT_UNEXP)) {
					log_error(&tok.loginfo, "invalid token type %u after '#ifndef' preprocessor command\n", tok.tokt);
					goto preproc_hash_err;
				}
				khiter_t it = kh_get(macros_map, src->macros_map, string_content(tok.tokv.str));
				int iret;
				char *mname = string_steal(tok.tokv.str);
				kh_put(string_set, src->macros_used, mname, &iret);
				if (iret <= 0) {
					free(mname);
				}
				tok.tokt = PPTOK_SYM;
				tok.tokv.sym = LAST_SYM;
				// We don't care about iret(?)
				// TODO: check iret, error if needed
				++vector_last(ppsource, src->prep).srcv.prep.cond_depth;
				if (it == kh_end(src->macros_map)) {
					++vector_last(ppsource, src->prep).srcv.prep.ok_depth;
					goto preproc_hash_err;
				} else {
					vector_last(ppsource, src->prep).srcv.prep.entered_next_ok_cond = 0;
					goto preproc_hash_err_goto;
				}
			} else if (!strcmp(string_content(tok.tokv.str), "elif")) {
				// We are already in an #if or #elif (or #else) with a match, so we need to leave it and go to the very top
				if (vector_last(ppsource, src->prep).srct != PPSRC_PREPARE) {
					log_error(&tok.loginfo, "invalid #elif source type %u\n", vector_last(ppsource, src->prep).srct);
					goto preproc_hash_err;
				}
				if (vector_last(ppsource, src->prep).srcv.prep.ok_depth) {
					vector_last(ppsource, src->prep).srcv.prep.entered_next_ok_cond = 1;
					--vector_last(ppsource, src->prep).srcv.prep.ok_depth;
					goto preproc_hash_err_goto;
				} else {
					log_warning(&tok.loginfo, "unexpected #elif preprocessor command\n");
					goto preproc_hash_err;
				}
			} else if (!strcmp(string_content(tok.tokv.str), "elifdef")) {
				// We are already in an #if or #elif (or #else) with a match, so we need to leave it and go to the very top
				if (vector_last(ppsource, src->prep).srct != PPSRC_PREPARE) {
					log_error(&tok.loginfo, "invalid #elifdef source type %u\n", vector_last(ppsource, src->prep).srct);
					goto preproc_hash_err;
				}
				string_del(tok.tokv.str);
				tok = ppsrc_next_token(src);
				int iret;
				char *mname = string_steal(tok.tokv.str);
				kh_put(string_set, src->macros_used, mname, &iret);
				if (iret <= 0) {
					free(mname);
				}
				tok.tokt = PPTOK_SYM;
				tok.tokv.sym = LAST_SYM;
				// We don't care about iret(?)
				// TODO: check iret, error if needed
				if (vector_last(ppsource, src->prep).srcv.prep.ok_depth) {
					vector_last(ppsource, src->prep).srcv.prep.entered_next_ok_cond = 1;
					--vector_last(ppsource, src->prep).srcv.prep.ok_depth;
					goto preproc_hash_err_goto;
				} else {
					log_warning(&tok.loginfo, "unexpected #elifdef preprocessor command\n");
					goto preproc_hash_err;
				}
			} else if (!strcmp(string_content(tok.tokv.str), "elifndef")) {
				// We are already in an #if or #elif (or #else) with a match, so we need to leave it and go to the very top
				if (vector_last(ppsource, src->prep).srct != PPSRC_PREPARE) {
					log_error(&tok.loginfo, "invalid #elifndef source type %u\n", vector_last(ppsource, src->prep).srct);
					goto preproc_hash_err;
				}
				string_del(tok.tokv.str);
				tok = ppsrc_next_token(src);
				int iret;
				char *mname = string_steal(tok.tokv.str);
				kh_put(string_set, src->macros_used, mname, &iret);
				if (iret <= 0) {
					free(mname);
				}
				tok.tokt = PPTOK_SYM;
				tok.tokv.sym = LAST_SYM;
				// We don't care about iret(?)
				// TODO: check iret, error if needed
				if (vector_last(ppsource, src->prep).srcv.prep.ok_depth) {
					vector_last(ppsource, src->prep).srcv.prep.entered_next_ok_cond = 1;
					--vector_last(ppsource, src->prep).srcv.prep.ok_depth;
					goto preproc_hash_err_goto;
				} else {
					log_warning(&tok.loginfo, "unexpected #elifndef preprocessor command\n");
					goto preproc_hash_err;
				}
			} else if (!strcmp(string_content(tok.tokv.str), "else")) {
				// We are already in an #if or #elif (or #else) with a match, so we need to leave it and go to the very top
				if (vector_last(ppsource, src->prep).srct != PPSRC_PREPARE) {
					log_error(&tok.loginfo, "invalid #else source type %u\n", vector_last(ppsource, src->prep).srct);
					goto preproc_hash_err;
				}
				if (vector_last(ppsource, src->prep).srcv.prep.ok_depth) {
					// We can safely ignore setting entered_next_ok since two #else is illegal (though this parser doesn't actually detect this)
					--vector_last(ppsource, src->prep).srcv.prep.ok_depth;
					goto preproc_hash_err_goto;
				} else {
					log_warning(&tok.loginfo, "unexpected #else preprocessor command\n");
					goto preproc_hash_err;
				}
			} else if (!strcmp(string_content(tok.tokv.str), "endif")) {
				if (vector_last(ppsource, src->prep).srct != PPSRC_PREPARE) {
					log_error(&tok.loginfo, "invalid #endif source type %u\n", vector_last(ppsource, src->prep).srct);
					goto preproc_hash_err;
				}
				if (vector_last(ppsource, src->prep).srcv.prep.ok_depth) {
					--vector_last(ppsource, src->prep).srcv.prep.ok_depth;
					--vector_last(ppsource, src->prep).srcv.prep.cond_depth;
				} else {
					log_warning(&tok.loginfo, "unexpected #endif preprocessor command\n");
				}
				goto preproc_hash_err;
			}
			
			log_warning(&tok.loginfo, "Unknown preprocessor command %s, skipping until EOL\n", string_content(tok.tokv.str));
		preproc_hash_err:
			while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
				preproc_token_del(&tok);
				tok = ppsrc_next_token(src);
			}
			if (tok.tokt == PPTOK_NEWLINE) goto check_next_token;
			else goto start_cur_token;
			
		preproc_hash_err_goto:
			while ((tok.tokt != PPTOK_NEWLINE) && (tok.tokt != PPTOK_EOF) && (tok.tokt != PPTOK_INVALID)) {
				preproc_token_del(&tok);
				tok = ppsrc_next_token(src);
			}
			if (tok.tokt == PPTOK_NEWLINE) goto check_if_depth;
			else goto start_cur_token; // Returns immediately
		}
		src->st = (src->st == PPST_PRAGMA_EXPLICIT) ? PPST_PRAGMA_EXPLICIT : PPST_NONE;
		ret.tokt = PTOK_SYM;
		ret.loginfo = tok.loginfo;
		ret.tokv.sym = tok.tokv.sym;
		return ret;
	case PPTOK_NEWLINE:
		if (src->st == PPST_PRAGMA_EXPLICIT) {
			src->st = PPST_NL;
			ret.tokt = PTOK_SYM;
			ret.loginfo = tok.loginfo;
			ret.tokv.sym = SYM_SEMICOLON;
			return ret;
		}
		src->st = PPST_NL;
		goto check_next_token;
	case PPTOK_BLANK:
		src->st = PPST_NONE;
		ret.tokt = PTOK_INVALID;
		ret.loginfo = tok.loginfo;
		ret.tokv.c = tok.tokv.c;
		return ret;
	case PPTOK_START_LINE_COMMENT:
		src->st = PPST_NONE;
		ret.tokt = PTOK_INVALID;
		ret.loginfo = tok.loginfo;
		ret.tokv.c = tok.tokv.c;
		return ret;
	case PPTOK_EOF:
		if ((vector_last(ppsource, src->prep).srct == PPSRC_PREPARE) && vector_last(ppsource, src->prep).srcv.prep.cond_depth) {
			log_warning(&tok.loginfo, "file ended before closing all conditionals (ignoring)\n");
		}
#ifdef LOG_CLOSE
		printf("Closing %s\n", src->cur_file);
#endif
		if (vector_last(ppsource, src->prep).srct == PPSRC_PREPARE) {
			if (src->dirname) free(src->dirname);
			if (src->cur_file) free(src->cur_file);
			src->dirname = vector_last(ppsource, src->prep).srcv.prep.old_dirname;
			src->cur_file = vector_last(ppsource, src->prep).srcv.prep.old_filename;
			src->is_sys = vector_last(ppsource, src->prep).srcv.prep.was_sys;
			src->cur_pathno = vector_last(ppsource, src->prep).srcv.prep.old_pathno;
			vector_last(ppsource, src->prep).srcv.prep.old_dirname = NULL;
			vector_last(ppsource, src->prep).srcv.prep.old_filename = NULL;
		}
		vector_pop(ppsource, src->prep);
		if (src->st == PPST_PRAGMA_EXPLICIT) {
			src->st = PPST_NL;
			ret.tokt = PTOK_SYM;
			ret.loginfo = (loginfo_t){0}; // The token's loginfo may have been deleted by the vector_pop above
			ret.tokv.sym = SYM_SEMICOLON;
			// Empty destructor
			return ret;
		}
		src->st = PPST_NL; // Should be redundant since TOK_NEWLINE is added before TOK_EOF if required
		// EOF has an empty destructor
		// Note that since we have opened the file, the previous file also had ok_depth == cond_depth
		goto check_next_token;
	
	default:
		log_error(&tok.loginfo, "unknown preprocessor token type %u, sending INVALID\n", tok.tokt);
		ret.tokt = PTOK_INVALID;
		ret.loginfo = tok.loginfo;
		ret.tokv.c = '\0';
		return ret;
	}
}
proc_token_t proc_next_token(preproc_t *src) {
	proc_token_t ret = proc_next_token_aux(src);
	if ((ret.tokt == PTOK_STRING) && ret.tokv.sisstr) {
		while (1) {
			proc_token_t ret2 = proc_next_token_aux(src);
			if ((ret2.tokt == PTOK_STRING) && ret2.tokv.sisstr) {
				if (!string_add_string(ret.tokv.sstr, ret2.tokv.sstr)) {
					log_memory("failed to concatenate adjacent strings\n");
					string_del(ret.tokv.sstr);
					string_del(ret2.tokv.sstr);
					src->st = PPST_NONE;
					ret.tokt = PTOK_INVALID;
					ret.tokv.c = '\0';
					return ret;
				}
				string_del(ret2.tokv.sstr);
				ret.loginfo.lineno_end = ret2.loginfo.lineno_end ? ret2.loginfo.lineno_end : ret2.loginfo.lineno;
				ret.loginfo.colno_end = ret2.loginfo.colno_end ? ret2.loginfo.colno_end : ret2.loginfo.colno;
			} else {
				if (!proc_unget_token(src, &ret2)) {
					log_memory("failed to unget token next to string token\n");
					string_del(ret.tokv.sstr);
					proc_token_del(&ret2);
					src->st = PPST_NONE;
					ret.tokt = PTOK_INVALID;
					ret.tokv.c = '\0';
					return ret;
				}
				return ret;
			}
		}
	} else return ret;
}
