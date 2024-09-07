#include "prepare.h"

#include <string.h>

struct prepare_s {
	FILE *f;
	int buf[4];
	int buf_len; // <= 4 (though 3 *should* be enough)
	char *srcn;
	enum prepare_state {
		PREPST_NONE = 0,
		PREPST_NL,
		PREPST_HASH,
		PREPST_INCL,
		PREPST_DEF,
		PREPST_DEFID,
		PREPST_COMMENT,
	} st;
};

prepare_t *prepare_new_file(FILE *f, const char *filename) {
	prepare_t *ret = malloc(sizeof *ret);
	if (!ret) {
		fclose(f);
		return NULL;
	}
	*ret = (prepare_t){
		.f = f,
		.buf = {0, 0, 0},
		.buf_len = 0,
		.srcn = strdup(filename),
		.st = PREPST_NL,
	};
	return ret;
}

void prepare_del(prepare_t *prep) {
	if (prep->f) fclose(prep->f);
	if (prep->srcn) free(prep->srcn);
	free(prep);
}

static int get_char(prepare_t *src) {
start_get_char:
	int c = src->buf_len ? src->buf[--src->buf_len] : getc(src->f);
	src->buf_len = 0;
	if (c == '\\') {
		c = src->buf_len ? src->buf[--src->buf_len] : getc(src->f);
		if (c == '\n') goto start_get_char;
		src->buf[src->buf_len++] = c;
		return '\\';
	}
	return c;
}
// Do not call this more than twice in a row if the last character retrieved is '\\'
static void unget_char(prepare_t *src, int c) {
	src->buf[src->buf_len++] = c;
}

static void fill_ident(prepare_t *src, string_t *buf) {
	while (1) {
		int c = get_char(src);
		if ((c == '_') || ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))) {
			string_add_char(buf, (char)c);
		} else {
			unget_char(src, c);
			return;
		}
	}
}

static void fill_num(prepare_t *src, string_t *buf) {
	int started_exp = 0;
	while (1) {
		int c = get_char(src);
		if ((c == '_') || (c == '.') || ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'))
		 || (started_exp && ((c == '+') || (c == '-')))) {
			started_exp = (c == 'e') || (c == 'E') || (c == 'p') || (c == 'P');
			string_add_char(buf, (char)c);
		} else {
			unget_char(src, c);
			return;
		}
	}
}

static void fill_str(prepare_t *src, string_t *buf, char end_c, int can_esc) {
	int has_esc = 0;
	while (1) {
		int c = get_char(src);
		if (has_esc && (c >= 0) && (c <= 0x7F) && (c != '\n')) {
			// Not technically standard compliant (should support \ooo, \x..., \u..., \U...)
			// Since we don't really care about parsing the content, only the delimiters, this is good enough
			string_add_char(buf, '\\');
			string_add_char(buf, (char)c);
			has_esc = 0;
		} else if (c == '\\') {
			if (can_esc) {
				has_esc = 1;
			} else {
				string_add_char(buf, '\\');
			}
		} else if ((c >= 0) && (c <= 0x7F) && (c != '\n') && (c != end_c)) {
			has_esc = 0;
			string_add_char(buf, (char)c);
		} else {
			if (has_esc) {
				// c is invalid or a '\n', or can_esc = 0 and c = end_c
				string_add_char(buf, '\\');
			}
			if (c != end_c)
				unget_char(src, c);
			return;
		}
	}
}

#define BASE_NSYMS 25
static const struct symbs_s {
	char c;
	enum token_sym_type_e sym;
	int nnext;
	const struct symbs_s *next;
} *symbs = (struct symbs_s[BASE_NSYMS]){
#define TERM(ch, t) { .c = ch, .sym = t, .nnext = 0, .next = NULL }
#define NONTERM(ch, t, n, ...) { .c = ch, .sym = t, .nnext = n, .next = (struct symbs_s[n]){__VA_ARGS__} }
	// Only '..' must have a sym > LAST_SYM; change next_token if this is not the case
	NONTERM('.', SYM_DOT, 1, NONTERM('.', LAST_SYM + 1, 1, TERM('.', SYM_VARIADIC))),
	TERM('{', SYM_LBRACKET),
	TERM('}', SYM_RBRACKET),
	TERM('[', SYM_LSQBRACKET),
	TERM(']', SYM_RSQBRACKET),
	TERM('(', SYM_LPAREN),
	TERM(')', SYM_RPAREN),
	NONTERM('#', SYM_HASH, 1, TERM('#', SYM_HASHHASH)),
	TERM(';', SYM_SEMICOLON),
	NONTERM(':', SYM_COLON, 1, TERM(':', SYM_COLONCOLON)),
	TERM('?', SYM_QUESTION),
	TERM('~', SYM_TILDE),
	NONTERM('!', SYM_EXCL, 1, TERM('=', SYM_EXCLEQ)),
	NONTERM('+', SYM_PLUS, 2, TERM('=', SYM_PLUSEQ), TERM('+', SYM_PLUSPLUS)),
	NONTERM('-', SYM_DASH, 3, TERM('=', SYM_DASHEQ), TERM('-', SYM_DASHDASH), TERM('>', SYM_DASHGT)),
	NONTERM('*', SYM_STAR, 1, TERM('=', SYM_STAREQ)),
	NONTERM('/', SYM_SLASH, 1, TERM('=', SYM_SLASHEQ)),
	NONTERM('%', SYM_PERCENT, 1, TERM('=', SYM_PERCENTEQ)),
	NONTERM('^', SYM_HAT, 1, TERM('=', SYM_HATEQ)),
	NONTERM('&', SYM_AMP, 2, TERM('=', SYM_AMPEQ), TERM('&', SYM_AMPAMP)),
	NONTERM('|', SYM_PIPE, 2, TERM('=', SYM_PIPEEQ), TERM('|', SYM_PIPEPIPE)),
	NONTERM('=', SYM_EQ, 1, TERM('=', SYM_EQEQ)),
	NONTERM('<', SYM_LT, 2, TERM('=', SYM_LTEQ), NONTERM('<', SYM_LTLT, 1, TERM('=', SYM_LTLTEQ))),
	NONTERM('>', SYM_GT, 2, TERM('=', SYM_GTEQ), NONTERM('>', SYM_GTGT, 1, TERM('=', SYM_GTGTEQ))),
	TERM(',', SYM_COMMA),
#undef NONTERM
#undef TERM
};

preproc_token_t pre_next_token(prepare_t *src, int allow_comments) {
	if (src->st == PREPST_COMMENT) {
		// In comments, keep everything as 'BLANK' except for idents, newlines and EOF
		int c = get_char(src);
		if (c == EOF) {
			// Force newline at EOF
			unget_char(src, c);
			src->st = PREPST_NL;
			return (preproc_token_t){
				.tokt = PPTOK_NEWLINE,
				.tokv.c = (char)c
			};
		} else if ((c == '_') || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))) {
			preproc_token_t ret;
			ret.tokt = PPTOK_IDENT;
			ret.tokv.str = string_new_cap(1);
			string_add_char(ret.tokv.str, (char)c);
			fill_ident(src, ret.tokv.str);
			return ret;
		} else if ((c >= 0) && (c <= 0x7F)) {
			return (preproc_token_t){
				.tokt = PPTOK_BLANK,
				.tokv.c = (char)c
			};
		} else {
			return (preproc_token_t){
				.tokt = PPTOK_INVALID,
				.tokv.c = (char)c
			};
		}
	}
	
start_next_token:
	int c = get_char(src);
	if (c == EOF) {
		if (src->st == PREPST_NL) {
			return (preproc_token_t){
				.tokt = PPTOK_EOF,
				.tokv.c = (char)c
			};
		} else {
			// Force newline at EOF
			unget_char(src, c);
			src->st = PREPST_NL;
			return (preproc_token_t){
				.tokt = PPTOK_NEWLINE,
				.tokv.c = (char)c
			};
		}
	}
	
	if (src->st == PREPST_INCL && (c == '<')) {
		src->st = PREPST_NONE;
		preproc_token_t ret;
		ret.tokt = PPTOK_INCL;
		ret.tokv.sisstr = 0;
		ret.tokv.sstr = string_new();
		fill_str(src, ret.tokv.sstr, '>', 0);
		return ret;
	}
	if (c == '\'') {
		src->st = PREPST_NONE;
		preproc_token_t ret;
		ret.tokt = PPTOK_STRING;
		ret.tokv.sisstr = 0;
		ret.tokv.sstr = string_new_cap(1); // Usually only one character is inside a char literal
		fill_str(src, ret.tokv.sstr, '\'', 1);
		return ret;
	}
	if (c == '"') {
		preproc_token_t ret;
		ret.tokt = (src->st == PREPST_INCL) ? PPTOK_INCL : PPTOK_STRING;
		src->st = PREPST_NONE;
		ret.tokv.sisstr = 1;
		ret.tokv.sstr = string_new();
		fill_str(src, ret.tokv.sstr, '"', ret.tokt == PPTOK_STRING);
		return ret;
	}
	if ((c == ' ') || (c == '\f') || (c == '\t') || (c == '\v')) {
		if (src->st == PREPST_DEFID) {
			src->st = PREPST_NONE;
			return (preproc_token_t){
				.tokt = PPTOK_BLANK,
				.tokv.c = (char)c
			};
		} else goto start_next_token;
	}
	if (c == '\n') {
		src->st = PREPST_NL;
		return (preproc_token_t){
			.tokt = PPTOK_NEWLINE,
			.tokv.c = (char)c
		};
	}
	if ((c == '_') || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))) {
		preproc_token_t ret;
		ret.tokt = PPTOK_IDENT;
		ret.tokv.str = string_new_cap(1);
		string_add_char(ret.tokv.str, (char)c);
		fill_ident(src, ret.tokv.str);
		src->st =
			((src->st == PREPST_HASH) && (!strcmp(string_content(ret.tokv.str), "include"))) ? PREPST_INCL :
			((src->st == PREPST_HASH) && (!strcmp(string_content(ret.tokv.str), "include_next"))) ? PREPST_INCL :
			((src->st == PREPST_HASH) && (!strcmp(string_content(ret.tokv.str), "define"))) ? PREPST_DEF :
			(src->st == PREPST_DEF) ? PREPST_DEFID :
			PREPST_NONE;
		return ret;
	}
	if ((c >= '0') && (c <= '9')) {
		src->st = PREPST_NONE;
		preproc_token_t ret;
		ret.tokt = PPTOK_NUM;
		ret.tokv.str = string_new_cap(1);
		string_add_char(ret.tokv.str, (char)c);
		fill_num(src, ret.tokv.str);
		return ret;
	}
	if (c == '.') {
		c = get_char(src);
		if ((c >= '0') && (c <= '9')) {
			src->st = PREPST_NONE;
			preproc_token_t ret;
			ret.tokt = PPTOK_NUM;
			ret.tokv.str = string_new_cap(2);
			string_add_char(ret.tokv.str, '.');
			string_add_char(ret.tokv.str, (char)c);
			fill_num(src, ret.tokv.str);
			return ret;
		} else {
			unget_char(src, c);
			c = '.';
		}
	}
	if (c == '/') {
		c = get_char(src);
		if (c == '/') {
			if (allow_comments) {
				src->st = PREPST_COMMENT;
				return (preproc_token_t){
					.tokt = PPTOK_START_LINE_COMMENT,
					.tokv.c = '/'
				};
			}
			
			do {
				c = get_char(src);
			} while ((c != EOF) && (c != '\n'));
			if (c != EOF) {
				if (src->st == PREPST_NL)
					goto start_next_token;
				else {
					src->st = PREPST_NL;
					return (preproc_token_t){
						.tokt = PPTOK_NEWLINE,
						.tokv.c = (char)c
					};
				}
			}
			
			src->st = PREPST_NONE;
			printf("Unfinished comment while preparing %s\n", src->srcn);
			return (preproc_token_t){
				.tokt = PPTOK_INVALID,
				.tokv.c = (char)c
			};
		} else if (c == '*') {
			if (allow_comments) {
				printf("Unsupported multiline comment with allow_comment in %s\n", src->srcn);
				return (preproc_token_t){
					.tokt = PPTOK_INVALID,
					.tokv.c = (char)c
				};
			}
			
			c = get_char(src);
			int last_star = 0;
			while ((c != EOF) && (!last_star || (c != '/'))) {
				last_star = c == '*';
				c = get_char(src);
			}
			if (c != EOF) goto start_next_token;
			
			src->st = PREPST_NONE;
			printf("Unfinished comment while preparing %s\n", src->srcn);
			return (preproc_token_t){
				.tokt = PPTOK_INVALID,
				.tokv.c = (char)c
			};
		} else {
			unget_char(src, c);
			c = '/';
		}
	}
	
	struct symbs_s const *sym = NULL;
	for (int i = 0; i < BASE_NSYMS; ++i) {
		if (c == symbs[i].c) {
			sym = &symbs[i];
			break;
		}
	}
	if (sym) {
		while (sym->nnext) {
			c = get_char(src);
			int found = 0;
			for (int i = 0; i < sym->nnext; ++i) {
				if (c == sym->next[i].c) {
					found = 1;
					sym = &sym->next[i];
					break;
				}
			}
			if (!found) {
				unget_char(src, c);
				break;
			}
		}
		if (sym->sym == LAST_SYM + 1) {
			unget_char(src, sym->c);
			sym = &symbs[0]; // This is where no check is made (see comment in the definition of symbs)
		}
		src->st = ((src->st == PREPST_NL) && (sym->sym == SYM_HASH)) ? PREPST_HASH : PREPST_NONE;
		return (preproc_token_t){
			.tokt = PPTOK_SYM,
			.tokv.sym = sym->sym
		};
	}
	
	src->st = PREPST_NONE;
	printf("Invalid character 0x%X (%c) while preparing %s\n", (unsigned)c, (c >= 0x20) && (c < 127) ? c : '?', src->srcn);
	return (preproc_token_t){
		.tokt = PPTOK_INVALID,
		.tokv.c = (char)c
	};
}

// Warning: unsafe method
void prepare_mark_nocomment(prepare_t *src) {
	src->st = PREPST_NONE;
}
int pre_next_newline_token(prepare_t *src, string_t *buf) {
	while (1) {
		int c = get_char(src);
		if (c == EOF) {
			// Force newline at EOF
			unget_char(src, c);
			src->st = PREPST_NL;
			return 1;
		} else if (c == '\n') {
			src->st = PREPST_NL;
			return 1;
		} else if ((c >= 0) && (c <= 0x7F)) {
			if (!string_add_char(buf, (char)c)) return 0;
		} else {
			return 0;
		}
	}
}
