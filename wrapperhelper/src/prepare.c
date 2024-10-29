#include "prepare.h"

#include <string.h>

#include "log.h"

static struct li_filename_s {
	char *name;
	struct li_filename_s *next;
} *li_filenames = NULL;

typedef struct char_s {
	int c;
	loginfo_t li;
} char_t;

struct prepare_s {
	FILE *f;
	char_t buf[5];
	int buf_len; // <= 5 (though 4 *should* be enough)
	char *srcn;
	loginfo_t li;
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
		log_memory("failed to allocate new preparator structure\n");
		fclose(f);
		return NULL;
	}
	
	char *srcn = strdup(filename ? filename : "<unknown filename>");
	if (!srcn) {
		log_memory("failed to duplicate filename\n");
		free(ret);
		return NULL;
	}
	struct li_filename_s *new_lifn = malloc(sizeof *new_lifn);
	if (!new_lifn) {
		log_memory("failed to remember new filename\n");
		free(srcn);
		free(ret);
		return NULL;
	}
	*new_lifn = (struct li_filename_s){ .name = srcn, .next = li_filenames };
	li_filenames = new_lifn;
	
	*ret = (prepare_t){
		.f = f,
		.buf = {{0}, {0}, {0}, {0}},
		.buf_len = 0,
		.srcn = srcn,
		.li = { .filename = srcn, .lineno = 1, .colno = 1, .lineno_end = 0, .colno_end = 0 },
		.st = PREPST_NL,
	};
	return ret;
}

void prepare_del(prepare_t *prep) {
	if (prep->f) fclose(prep->f);
	free(prep);
}

// Do not call this more than twice in a row if the last character retrieved is '\\'
static void unget_char(prepare_t *src, char_t c) {
	src->buf[src->buf_len++] = c;
}
// Transforms \n, \r\n and \r into \n
static char_t get_char(prepare_t *src) {
	if (src->buf_len) {
		return src->buf[--src->buf_len];
	}
start_get_char:
	int c = getc(src->f);
	if (c == '\\') {
		c = getc(src->f);
		if (c == '\n') {
			++src->li.lineno;
			src->li.colno = 1;
			goto start_get_char;
		} else if (c == '\r') {
			++src->li.lineno;
			src->li.colno = 1;
			c = getc(src->f);
			if (c == '\n') {
				goto start_get_char;
			} else {
				unget_char(src, (char_t){.c = c, .li = src->li});
				goto start_get_char;
			}
		}
		src->li.colno += 2;
		src->buf[src->buf_len++] = (char_t){
			.c = c,
			.li = { .filename = src->li.filename, .lineno = src->li.lineno, .colno = src->li.colno - 1, .lineno_end = 0, .colno_end = 0 }
		};
		return (char_t){
			.c = '\\',
			.li = { .filename = src->li.filename, .lineno = src->li.lineno, .colno = src->li.colno - 2, .lineno_end = 0, .colno_end = 0 }
		};
	}
	char_t ret = {
		.c = c,
		.li = { .filename = src->li.filename, .lineno = src->li.lineno, .colno = src->li.colno, .lineno_end = 0, .colno_end = 0 }
	};
	if (c == '\n') {
		++src->li.lineno;
		src->li.colno = 1;
	} else if (c == '\r') {
		++src->li.lineno;
		src->li.colno = 1;
		ret.c = '\n';
		c = getc(src->f);
		if (c != '\n') {
			unget_char(src, (char_t){.c = c, .li = src->li});
		}
	} else {
		++src->li.colno;
	}
	return ret;
}

static void fill_ident(prepare_t *src, string_t *buf, size_t *lineno_end, size_t *colno_end) {
	while (1) {
		char_t c = get_char(src);
		if ((c.c == '_') || ((c.c >= '0') && (c.c <= '9')) || ((c.c >= 'A') && (c.c <= 'Z')) || ((c.c >= 'a') && (c.c <= 'z'))) {
			string_add_char(buf, (char)c.c);
			*lineno_end = c.li.lineno;
			*colno_end = c.li.colno;
		} else {
			unget_char(src, c);
			return;
		}
	}
}

static void fill_num(prepare_t *src, string_t *buf, size_t *lineno_end, size_t *colno_end) {
	int started_exp = 0;
	while (1) {
		char_t c = get_char(src);
		if ((c.c == '_') || (c.c == '.') || ((c.c >= '0') && (c.c <= '9')) || ((c.c >= 'A') && (c.c <= 'Z')) || ((c.c >= 'a') && (c.c <= 'z'))
		 || (started_exp && ((c.c == '+') || (c.c == '-')))) {
			started_exp = (c.c == 'e') || (c.c == 'E') || (c.c == 'p') || (c.c == 'P');
			string_add_char(buf, (char)c.c);
			*lineno_end = c.li.lineno;
			*colno_end = c.li.colno;
		} else {
			unget_char(src, c);
			return;
		}
	}
}

static void fill_str(prepare_t *src, string_t *buf, char end_c, int can_esc, size_t *lineno_end, size_t *colno_end) {
	int has_esc = 0;
	while (1) {
		char_t c = get_char(src);
		if (has_esc && (c.c >= 0) && (c.c <= 0x7F) && (c.c != '\n')) {
			// Not technically standard compliant (should support \ooo, \x..., \u..., \U...)
			// Since we don't really care about parsing the content, only the delimiters, this is good enough
			string_add_char(buf, '\\');
			string_add_char(buf, (char)c.c);
			has_esc = 0;
			*lineno_end = c.li.lineno;
			*colno_end = c.li.colno;
		} else if (c.c == '\\') {
			*lineno_end = c.li.lineno;
			*colno_end = c.li.colno;
			if (can_esc) {
				has_esc = 1;
			} else {
				string_add_char(buf, '\\');
			}
		} else if ((c.c >= 0) && (c.c <= 0x7F) && (c.c != '\n') && (c.c != end_c)) {
			has_esc = 0;
			string_add_char(buf, (char)c.c);
			*lineno_end = c.li.lineno;
			*colno_end = c.li.colno;
		} else {
			// c.c is invalid (> 0x80) or a '\n', or can_esc = 0 and c.c = end_c
			if (has_esc) {
				string_add_char(buf, '\\');
			}
			if (c.c != end_c) {
				unget_char(src, c);
			} else {
				*lineno_end = c.li.lineno;
				*colno_end = c.li.colno;
			}
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
		char_t c = get_char(src);
		if (c.c == EOF) {
			// Force newline at EOF
			unget_char(src, c);
			src->st = PREPST_NL;
			return (preproc_token_t){
				.tokt = PPTOK_NEWLINE,
				.loginfo = c.li,
				.tokv.c = (char)c.c
			};
		} else if ((c.c == '_') || ((c.c >= 'a') && (c.c <= 'z')) || ((c.c >= 'A') && (c.c <= 'Z'))) {
			preproc_token_t ret;
			ret.tokt = PPTOK_IDENT;
			ret.loginfo = c.li;
			ret.tokv.str = string_new_cap(1);
			string_add_char(ret.tokv.str, (char)c.c);
			fill_ident(src, ret.tokv.str, &ret.loginfo.lineno_end, &ret.loginfo.colno_end);
			return ret;
		} else if ((c.c >= 0) && (c.c <= 0x7F)) {
			return (preproc_token_t){
				.tokt = PPTOK_BLANK,
				.loginfo = c.li,
				.tokv.c = (char)c.c
			};
		} else {
			return (preproc_token_t){
				.tokt = PPTOK_INVALID,
				.loginfo = c.li,
				.tokv.c = (char)c.c
			};
		}
	}
	
start_next_token:
	char_t c = get_char(src);
	if (c.c == EOF) {
		if (src->st == PREPST_NL) {
			return (preproc_token_t){
				.tokt = PPTOK_EOF,
				.loginfo = c.li,
				.tokv.c = (char)c.c
			};
		} else {
			// Force newline at EOF
			unget_char(src, c);
			src->st = PREPST_NL;
			return (preproc_token_t){
				.tokt = PPTOK_NEWLINE,
				.loginfo = c.li,
				.tokv.c = (char)c.c
			};
		}
	}
	
	if (src->st == PREPST_INCL && (c.c == '<')) {
		src->st = PREPST_NONE;
		preproc_token_t ret;
		ret.tokt = PPTOK_INCL;
		ret.loginfo = c.li;
		ret.tokv.sisstr = 0;
		ret.tokv.sstr = string_new();
		fill_str(src, ret.tokv.sstr, '>', 0, &ret.loginfo.lineno_end, &ret.loginfo.colno_end);
		return ret;
	}
	if (c.c == '\'') {
		src->st = PREPST_NONE;
		preproc_token_t ret;
		ret.tokt = PPTOK_STRING;
		ret.loginfo = c.li;
		ret.tokv.sisstr = 0;
		ret.tokv.sstr = string_new_cap(1); // Usually only one character is inside a char literal
		fill_str(src, ret.tokv.sstr, '\'', 1, &ret.loginfo.lineno_end, &ret.loginfo.colno_end);
		return ret;
	}
	if (c.c == '"') {
		preproc_token_t ret;
		ret.tokt = (src->st == PREPST_INCL) ? PPTOK_INCL : PPTOK_STRING;
		src->st = PREPST_NONE;
		ret.loginfo = c.li;
		ret.tokv.sisstr = 1;
		ret.tokv.sstr = string_new();
		fill_str(src, ret.tokv.sstr, '"', ret.tokt == PPTOK_STRING, &ret.loginfo.lineno_end, &ret.loginfo.colno_end);
		return ret;
	}
	if ((c.c == ' ') || (c.c == '\f') || (c.c == '\t') || (c.c == '\v')) {
		if (src->st == PREPST_DEFID) {
			src->st = PREPST_NONE;
			return (preproc_token_t){
				.tokt = PPTOK_BLANK,
				.loginfo = c.li,
				.tokv.c = (char)c.c
			};
		} else goto start_next_token;
	}
	if (c.c == '\n') {
		src->st = PREPST_NL;
		return (preproc_token_t){
			.tokt = PPTOK_NEWLINE,
			.loginfo = c.li,
			.tokv.c = (char)c.c
		};
	}
	if ((c.c == '_') || ((c.c >= 'a') && (c.c <= 'z')) || ((c.c >= 'A') && (c.c <= 'Z'))) {
		preproc_token_t ret;
		ret.tokt = PPTOK_IDENT;
		ret.loginfo = c.li;
		ret.tokv.str = string_new_cap(1);
		string_add_char(ret.tokv.str, (char)c.c);
		fill_ident(src, ret.tokv.str, &ret.loginfo.lineno_end, &ret.loginfo.colno_end);
		src->st =
			((src->st == PREPST_HASH) && (!strcmp(string_content(ret.tokv.str), "include"))) ? PREPST_INCL :
			((src->st == PREPST_HASH) && (!strcmp(string_content(ret.tokv.str), "include_next"))) ? PREPST_INCL :
			((src->st == PREPST_HASH) && (!strcmp(string_content(ret.tokv.str), "define"))) ? PREPST_DEF :
			(src->st == PREPST_DEF) ? PREPST_DEFID :
			PREPST_NONE;
		return ret;
	}
	if ((c.c >= '0') && (c.c <= '9')) {
		src->st = PREPST_NONE;
		preproc_token_t ret;
		ret.tokt = PPTOK_NUM;
		ret.loginfo = c.li;
		ret.tokv.str = string_new_cap(1);
		string_add_char(ret.tokv.str, (char)c.c);
		fill_num(src, ret.tokv.str, &ret.loginfo.lineno_end, &ret.loginfo.colno_end);
		return ret;
	}
	if (c.c == '.') {
		loginfo_t oldli = c.li;
		c = get_char(src);
		if ((c.c >= '0') && (c.c <= '9')) {
			src->st = PREPST_NONE;
			preproc_token_t ret;
			ret.tokt = PPTOK_NUM;
			ret.loginfo = oldli;
			ret.loginfo.lineno_end = c.li.lineno;
			ret.loginfo.colno_end = c.li.colno;
			ret.tokv.str = string_new_cap(2);
			string_add_char(ret.tokv.str, '.');
			string_add_char(ret.tokv.str, (char)c.c);
			fill_num(src, ret.tokv.str, &ret.loginfo.lineno_end, &ret.loginfo.colno_end);
			return ret;
		} else {
			unget_char(src, c);
			c.c = '.';
			c.li = oldli;
		}
	}
	if (c.c == '/') {
		char_t c2 = get_char(src);
		if (c2.c == '/') {
			if (allow_comments) {
				src->st = PREPST_COMMENT;
				return (preproc_token_t){
					.tokt = PPTOK_START_LINE_COMMENT,
					.loginfo = {
						.filename = c2.li.filename,
						.lineno = c.li.lineno, .colno = c.li.colno,
						.lineno_end = c2.li.lineno, .colno_end = c2.li.colno,
					},
					.tokv.c = '/',
				};
			}
			
			do {
				c2 = get_char(src);
			} while ((c2.c != EOF) && (c2.c != '\n'));
			if (c2.c != EOF) {
				if (src->st == PREPST_NL)
					goto start_next_token;
				else {
					src->st = PREPST_NL;
					return (preproc_token_t){
						.tokt = PPTOK_NEWLINE,
						.loginfo = c2.li,
						.tokv.c = (char)c2.c
					};
				}
			}
			
			// Force newline at EOF
			unget_char(src, c2);
			src->st = PREPST_NL;
			return (preproc_token_t){
				.tokt = PPTOK_NEWLINE,
				.loginfo = c2.li,
				.tokv.c = (char)c2.c
			};
		} else if (c2.c == '*') {
			if (allow_comments) {
				c.li.lineno_end = c2.li.lineno;
				c.li.colno_end = c2.li.colno;
				log_error(&c.li, "unsupported multiline comment with allow_comment\n");
				return (preproc_token_t){
					.tokt = PPTOK_INVALID,
					.loginfo = c.li,
					.tokv.c = (char)c.c
				};
			}
			
			c2 = get_char(src);
			int last_star = 0;
			while ((c2.c != EOF) && (!last_star || (c2.c != '/'))) {
				last_star = c2.c == '*';
				c2 = get_char(src);
			}
			if (c2.c != EOF) goto start_next_token;
			
			c.li.lineno_end = c2.li.lineno;
			c.li.colno_end = c2.li.colno;
			log_error(&c.li, "unfinished multiline comment\n");
			src->st = PREPST_NONE;
			return (preproc_token_t){
				.tokt = PPTOK_INVALID,
				.loginfo = c2.li,
				.tokv.c = (char)c2.c
			};
		} else {
			unget_char(src, c2);
		}
	}
	
	struct symbs_s const *sym = NULL;
	for (int i = 0; i < BASE_NSYMS; ++i) {
		if (c.c == symbs[i].c) {
			sym = &symbs[i];
			break;
		}
	}
	if (sym) {
		loginfo_t lisym = c.li;
		loginfo_t liend = c.li;
		while (sym->nnext) {
			c = get_char(src);
			int found = 0;
			for (int i = 0; i < sym->nnext; ++i) {
				if (c.c == sym->next[i].c) {
					found = 1;
					sym = &sym->next[i];
					liend = c.li;
					lisym.lineno_end = c.li.lineno;
					lisym.colno_end = c.li.colno;
					break;
				}
			}
			if (!found) {
				unget_char(src, c);
				break;
			}
		}
		if (sym->sym == LAST_SYM + 1) {
			// This is where no check is made (see comment in the definition of symbs)
			unget_char(src, (char_t){.c = sym->c, .li = liend});
			sym = &symbs[0];
			lisym.lineno_end = lisym.colno_end = 0;
		}
		src->st = ((src->st == PREPST_NL) && (sym->sym == SYM_HASH)) ? PREPST_HASH : PREPST_NONE;
		return (preproc_token_t){
			.tokt = PPTOK_SYM,
			.loginfo = lisym,
			.tokv.sym = sym->sym
		};
	}
	
	src->st = PREPST_NONE;
	log_error(&c.li, "invalid character 0x%02X (%c)\n", (unsigned)c.c, (c.c >= 0x20) && (c.c < 0x7F) ? c.c : '?');
	return (preproc_token_t){
		.tokt = PPTOK_INVALID,
		.loginfo = c.li,
		.tokv.c = (char)c.c,
	};
}

void prepare_set_line(prepare_t *src, char *filename, size_t lineno) {
	if (filename) {
		char *srcn = strdup(filename ? filename : "<unknown filename>");
		if (!srcn) {
			log_memory("failed to duplicate filename from #line command\n");
			return;
		}
		struct li_filename_s *new_lifn = malloc(sizeof *new_lifn);
		if (!new_lifn) {
			log_memory("failed to remember new filename from #line command\n");
			free(srcn);
			return;
		}
		*new_lifn = (struct li_filename_s){ .name = srcn, .next = li_filenames };
		li_filenames = new_lifn;
		src->srcn = srcn;
		src->li.filename = srcn;
	}
	size_t colno = 1;
	for (int i = src->buf_len; i--; ) {
		src->buf[i].li.lineno = lineno;
		src->buf[i].li.colno = colno;
		if (src->buf[i].c == '\n') {
			++lineno;
			colno = 1;
		} else {
			++colno;
		}
	}
	src->li.lineno = lineno;
	src->li.colno = colno;
}

// Warning: unsafe method
void prepare_mark_nocomment(prepare_t *src) {
	src->st = PREPST_NONE;
}
int pre_next_newline_token(prepare_t *src, string_t *buf) {
	while (1) {
		char_t c = get_char(src);
		if (c.c == EOF) {
			// Force newline at EOF
			unget_char(src, c);
			src->st = PREPST_NL;
			return 1;
		} else if (c.c == '\n') {
			src->st = PREPST_NL;
			return 1;
		} else if ((c.c >= 0) && (c.c <= 0x7F)) {
			if (!string_add_char(buf, (char)c.c)) return 0;
		} else {
			return 0;
		}
	}
}

void prepare_cleanup(void) {
	while (li_filenames) {
		struct li_filename_s *lifn = li_filenames->next;
		free(li_filenames->name);
		free(li_filenames);
		li_filenames = lifn;
	}
}
