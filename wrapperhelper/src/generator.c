#include "generator.h"

#include "lang.h"
#include "prepare.h"

static const char *rft2str[8] = {
	[RQT_FUN] = "",
	[RQT_FUN_2] = "",
	[RQT_FUN_MY] = "(my) ",
	[RQT_FUN_D] = "(D) ",
	[RQT_DATA] = "",
	[RQT_DATAV] = "(V) ",
	[RQT_DATAB] = "(B) ",
	[RQT_DATAM] = "(my) ",
};
#define IS_RQT_FUN2(rty) (((rty) == RQT_FUN_2) || ((rty) == RQT_FUN_D))
#define IS_RQT_FUNCTION(rty) ((rty) < RQT_DATA)

void request_print(request_t *req) {
	printf("%s", string_content(req->obj_name));
	if (req->has_default && req->has_val && (IS_RQT_FUNCTION(req->def.rty) != IS_RQT_FUNCTION(req->val.rty))) {
		printf(" => conflict: was/is data, is/was function\n");
	}
	int is_fun;
	if (req->has_default) is_fun = IS_RQT_FUNCTION(req->def.rty);
	else if (req->has_val) is_fun = IS_RQT_FUNCTION(req->val.rty);
	else {
		printf(" => (no value)\n");
		return;
	}
	
	if (is_fun) {
		printf(" => %sfunction", req->weak ? "weak " : "");
		if (req->has_default) {
			printf(" with %sdefault %s%s%s%s",
				req->default_comment ? "commented " : "",
				rft2str[req->def.rty],
				string_content(req->def.fun.typ),
				(req->def.rty == RQT_FUN_2) ? " -> " : "",
				(req->def.rty == RQT_FUN_2) ? string_content(req->def.fun.fun2) : "");
		}
		if (req->has_val) {
			printf(" with solved %s%s%s%s",
				rft2str[req->val.rty],
				string_content(req->val.fun.typ),
				(req->val.rty == RQT_FUN_2) ? " -> " : "",
				(req->val.rty == RQT_FUN_2) ? string_content(req->val.fun.fun2) : "");
		}
	} else {
		printf(" => %sdata", req->weak ? "weak " : "");
		if (req->has_default) {
			if (req->def.dat.has_size) printf(" with default %zu", req->def.dat.sz);
			else printf(" with no default");
		}
		if (req->has_val) {
			if (req->def.dat.has_size) printf(" with solved %zu", req->def.dat.sz);
			else printf(" with no solved");
		}
	}
	printf("\n");
}
void request_print_check(request_t *req) {
	if (req->has_default && req->has_val && (IS_RQT_FUNCTION(req->def.rty) != IS_RQT_FUNCTION(req->val.rty))) {
		printf("%s => conflict: was/is data, is/was function\n", string_content(req->obj_name));
	}
	int is_fun;
	if (req->has_default) is_fun = IS_RQT_FUNCTION(req->def.rty);
	else if (req->has_val) is_fun = IS_RQT_FUNCTION(req->val.rty);
	else {
		printf(" => (no value)\n");
		return;
	}
	if (is_fun) {
		if (req->has_val) {
			if (req->has_default) {
				int similar = 1;
				// if (req->def.rty != req->val.rty) similar = 0;
				if (similar && strcmp(string_content(req->def.fun.typ), string_content(req->val.fun.typ))) {
					// similar = 0;
					// TODO
					if (req->def.rty == RQT_FUN_MY) {
						similar =
							string_len(req->def.fun.typ) >= 3 &&
							string_content(req->def.fun.typ)[2] == 'E' &&
							!strncmp(string_content(req->def.fun.typ), string_content(req->val.fun.typ), 2) &&
							!strcmp(string_content(req->def.fun.typ) + 3, string_content(req->val.fun.typ) + 2);
						if (similar) {
							// We need to add the 'E' back here
						}
					} else {
						similar = 0;
					}
				}
				if (!similar) {
					printf("%s => %sfunction with %sdefault %s%s%s%s and different solved %s%s%s%s\n",
						string_content(req->obj_name),
						req->weak ? "weak " : "",
						req->default_comment ? "commented " : "",
						rft2str[req->def.rty],
						string_content(req->def.fun.typ),
						(req->def.rty == RQT_FUN_2) ? " -> " : "",
						(req->def.rty == RQT_FUN_2) ? string_content(req->def.fun.fun2) : "",
						rft2str[req->val.rty],
						string_content(req->val.fun.typ),
						(req->val.rty == RQT_FUN_2) ? " -> " : "",
						(req->val.rty == RQT_FUN_2) ? string_content(req->val.fun.fun2) : "");
				}
			} else {
				printf("%s => %sfunction with solved %s%s%s%s\n",
					string_content(req->obj_name),
					req->weak ? "weak " : "",
					rft2str[req->val.rty],
					string_content(req->val.fun.typ),
					(req->val.rty == RQT_FUN_2) ? " -> " : "",
					(req->val.rty == RQT_FUN_2) ? string_content(req->val.fun.fun2) : "");
			}
		} else if (req->has_default) {
			/* printf("%s => unsolved %sfunction with %sdefault %s%s%s%s\n",
				string_content(req->obj_name),
				req->weak ? "weak " : "",
				req->default_comment ? "commented " : "",
				rft2str[req->def.rty],
				string_content(req->def.fun.typ),
				(req->def.rty == RQT_FUN_2) ? " -> " : "",
				(req->def.rty == RQT_FUN_2) ? string_content(req->def.fun.fun2) : ""); */
		}
	} else {
		printf("%s => %sdata", string_content(req->obj_name), req->weak ? "weak " : "");
		if (req->has_default) {
			if (req->def.dat.has_size) printf(" with default %zu", req->def.dat.sz);
			else printf(" with no default");
		}
		if (req->has_val) {
			if (req->def.dat.has_size) printf(" with solved %zu", req->def.dat.sz);
			else printf(" with no solved");
		}
	}
}
void request_del(request_t *req) {
	string_del(req->obj_name);
	if (req->has_default) {
		switch (req->def.rty) {
		case RQT_FUN:    string_del(req->def.fun.typ);                                break;
		case RQT_FUN_2:  string_del(req->def.fun.typ); string_del(req->def.fun.fun2); break;
		case RQT_FUN_MY: string_del(req->def.fun.typ);                                break;
		case RQT_FUN_D:  string_del(req->def.fun.typ); string_del(req->def.fun.fun2); break;
		case RQT_DATA:   break;
		case RQT_DATAV:  break;
		case RQT_DATAB:  break;
		case RQT_DATAM:  break;
		}
	}
	if (req->has_val) {
		switch (req->val.rty) {
		case RQT_FUN:    string_del(req->val.fun.typ);                                break;
		case RQT_FUN_2:  string_del(req->val.fun.typ); string_del(req->val.fun.fun2); break;
		case RQT_FUN_MY: string_del(req->val.fun.typ);                                break;
		case RQT_FUN_D:  string_del(req->val.fun.typ); string_del(req->val.fun.fun2); break;
		case RQT_DATA:   break;
		case RQT_DATAV:  break;
		case RQT_DATAB:  break;
		case RQT_DATAM:  break;
		}
	}
}

static int valid_reqtype(string_t *t) {
	const char *s = string_content(t);
	if (!((s[0] >= 'A') && (s[0] <= 'Z')) && !((s[0] >= 'a') && (s[0] <= 'z'))) return 0;
	if (s[1] != 'F') return 0;
	for (size_t i = 2; i < string_len(t); ++i) {
		if (!((s[i] >= 'A') && (s[i] <= 'Z')) && !((s[i] >= 'a') && (s[i] <= 'z'))) return 0;
	}
	return 1;
}
static const char *rqt_suffix[8] = {
	[RQT_FUN] = "",
	[RQT_FUN_2] = "2",
	[RQT_FUN_MY] = "M",
	[RQT_FUN_D] = "D",
	[RQT_DATA] = "",
	[RQT_DATAV] = "V",
	[RQT_DATAB] = "B",
	[RQT_DATAM] = "M",
};

static void request_output(FILE *f, request_t *req) {
	if (!req->has_val) {
		if (!req->has_default) {
			// printf("Warning: %s has no value and no default, assuming function\n", string_content(req->obj_name));
			fprintf(f, "//GO%s(%s, \n", req->weak ? "W" : "", string_content(req->obj_name));
		} else if (IS_RQT_FUNCTION(req->def.rty)) {
			fprintf(f, "%sGO%s%s(%s, %s%s%s%s%s)%s\n",
				req->default_comment ? "//" : "",
				req->weak ? "W" : "",
				rqt_suffix[req->def.rty],
				string_content(req->obj_name),
				valid_reqtype(req->def.fun.typ) ? "" : "\"",
				string_content(req->def.fun.typ),
				valid_reqtype(req->def.fun.typ) ? "" : "\"",
				IS_RQT_FUN2(req->def.rty) ? ", " : "",
				IS_RQT_FUN2(req->def.rty) ? string_content(req->def.fun.fun2) : "",
				req->default_comment ? "" : " // Warning: failed to confirm");
		} else {
			if (req->def.dat.has_size) {
				fprintf(f, "%sDATA%s%s(%s, %zu) // Warning: failed to confirm\n",
					req->default_comment ? "//" : "",
					req->weak ? "W" : "",
					rqt_suffix[req->def.rty],
					string_content(req->obj_name),
					req->def.dat.sz);
			} else {
				fprintf(f, "//DATA%s%s(%s, \n",
					req->weak ? "W" : "",
					rqt_suffix[req->def.rty],
					string_content(req->obj_name));
			}
		}
	} else {
		if (IS_RQT_FUNCTION(req->val.rty)) {
			int is_comment =
				(req->has_default && !req->default_comment) ? (req->val.rty != req->def.rty) : (req->val.rty != RQT_FUN);
			fprintf(f, "%sGO%s%s(%s, %s%s%s)\n",
				is_comment ? "//" : "",
				req->weak ? "W" : "",
				rqt_suffix[req->val.rty],
				string_content(req->obj_name),
				string_content(req->val.fun.typ),
				IS_RQT_FUN2(req->val.rty) ? ", " : "",
				IS_RQT_FUN2(req->val.rty) ? req->val.fun.fun2 ? string_content(req->val.fun.fun2) : "<error: no val>" : "");
		} else {
			if (req->val.dat.has_size) {
				int is_comment = !req->has_default || req->default_comment || (req->def.rty != req->val.rty);
				fprintf(f, "%sDATA%s(%s, %zu)\n",
					is_comment ? "//" : "",
					rqt_suffix[req->val.rty],
					string_content(req->obj_name),
					req->val.dat.sz);
			} else {
				fprintf(f, "//DATA%s(%s, \n",
					rqt_suffix[req->val.rty],
					string_content(req->obj_name));
			}
		}
	}
}
void output_from_requests(FILE *f, VECTOR(requests) *reqs) {
	fprintf(f, "#if !(defined(GO) && defined(GOM) && defined(GO2) && defined(DATA))\n#error Meh...\n#endif\n\n");
	vector_for(requests, req, reqs) {
		request_output(f, req);
	}
}

VECTOR_IMPL(requests, request_del)

VECTOR(requests) *requests_from_file(const char *filename, FILE *f) {
	prepare_t *prep = prepare_new_file(f, filename);
	if (!prep) {
		printf("Failed to create the prepare structure for the requests file\n");
		return NULL;
	}
	
	VECTOR(requests) *ret = vector_new(requests);
	if (!ret) {
		prepare_del(prep);
		return NULL;
	}
	
	int lineno = 1;
	
	// Ignore the first 3 lines
	preproc_token_t tok;
	do {
		tok = pre_next_token(prep, 0);
		if (tok.tokt == PPTOK_NEWLINE) ++lineno;
	} while (!preproc_token_isend(&tok) && (lineno < 4));
	
	// TODO: better conditionals handling
	// Also, for now assume we have no definition
	int if_depth = 0, entered_depth = 0;
	while (1) {
		int is_comment = 0;
		tok = pre_next_token(prep, 1);
		while (tok.tokt == PPTOK_START_LINE_COMMENT) {
			is_comment = 1;
			// Empty destructor
			tok = pre_next_token(prep, 1);
		}
		if ((tok.tokt == PPTOK_SYM) && (tok.tokv.sym == SYM_HASH)) {
			if (is_comment) {
				preproc_token_del(&tok);
				tok = pre_next_newline_token(prep); // Returns a newline
				++lineno;
				continue;
			}
			tok = pre_next_token(prep, 0);
			if (tok.tokt != PPTOK_IDENT) {
				printf("Error: invalid requests file: invalid preprocessor line\n");
				preproc_token_del(&tok);
				goto failed;
			}
			if (!strcmp(string_content(tok.tokv.str), "ifdef")) {
				string_del(tok.tokv.str);
				tok = pre_next_token(prep, 0);
				if (tok.tokt != PPTOK_IDENT) {
					printf("Error: invalid requests file: invalid '#ifdef' line\n");
					preproc_token_del(&tok);
					goto failed;
				}
				++if_depth;
				string_del(tok.tokv.str);
				tok = pre_next_token(prep, 0);
			} else if (!strcmp(string_content(tok.tokv.str), "ifndef")) {
				string_del(tok.tokv.str);
				tok = pre_next_token(prep, 0);
				if (tok.tokt != PPTOK_IDENT) {
					printf("Error: invalid requests file: invalid '#ifndef' line\n");
					preproc_token_del(&tok);
					goto failed;
				}
				if (if_depth == entered_depth) ++entered_depth;
				++if_depth;
				string_del(tok.tokv.str);
				tok = pre_next_token(prep, 0);
			} else if (!strcmp(string_content(tok.tokv.str), "else")) {
				string_del(tok.tokv.str);
				tok = pre_next_token(prep, 0);
				if (if_depth == entered_depth + 1) ++entered_depth;
				else if (if_depth == entered_depth) --entered_depth;
			} else if (!strcmp(string_content(tok.tokv.str), "endif")) {
				string_del(tok.tokv.str);
				tok = pre_next_token(prep, 0);
				if (if_depth == entered_depth) --entered_depth;
				--if_depth;
			} else {
				printf("Error: invalid requests file: invalid preprocessor command '%s'\n", string_content(tok.tokv.str));
				string_del(tok.tokv.str);
				goto failed;
			}
			while (!preproc_token_isend(&tok) && (tok.tokt != PPTOK_NEWLINE)) {
				preproc_token_del(&tok);
				tok = pre_next_token(prep, 0);
			}
			++lineno;
			if (preproc_token_isend(&tok)) {
				if (tok.tokt == PPTOK_EOF) goto success;
				else {
					preproc_token_del(&tok);
					goto failed;
				}
			}
		} else if (tok.tokt == PPTOK_NEWLINE) {
			++lineno;
		} else if (tok.tokt == PPTOK_EOF) {
			goto success;
		} else if ((tok.tokt == PPTOK_IDENT)
		        && (!strcmp(string_content(tok.tokv.str), "GO")
		         || !strcmp(string_content(tok.tokv.str), "GO2")
		         || !strcmp(string_content(tok.tokv.str), "GOD")
		         || !strcmp(string_content(tok.tokv.str), "GOM")
		         || !strcmp(string_content(tok.tokv.str), "GOW")
		         || !strcmp(string_content(tok.tokv.str), "GOW2")
		         || !strcmp(string_content(tok.tokv.str), "GOWD")
		         || !strcmp(string_content(tok.tokv.str), "GOWM"))) {
			int isweak = (string_content(tok.tokv.str)[2] == 'W');
			request_t req = {
				.has_default = 0,
				.default_comment = is_comment,
				.has_val = 0,
				.obj_name = NULL,
				.weak = isweak,
				.def = {
					.rty =
						(string_content(tok.tokv.str)[isweak ? 3 : 2] == '2') ? RQT_FUN_2 :
						(string_content(tok.tokv.str)[isweak ? 3 : 2] == 'D') ? RQT_FUN_D :
						(string_content(tok.tokv.str)[isweak ? 3 : 2] == 'M') ? RQT_FUN_MY : RQT_FUN,
					.fun.typ = NULL,
					.fun.fun2 = NULL,
				},
			};
			string_del(tok.tokv.str);
			tok = pre_next_token(prep, 0);
			if ((tok.tokt != PPTOK_SYM) || (tok.tokv.sym != SYM_LPAREN)) {
				printf("Error: invalid requests file: invalid GO line %d (lparen)\n", lineno);
				preproc_token_del(&tok);
				goto failed;
			}
			// Empty destructor
			tok = pre_next_token(prep, 0);
			if (tok.tokt != PPTOK_IDENT) {
				printf("Error: invalid requests file: invalid GO line %d (obj_name)\n", lineno);
				preproc_token_del(&tok);
				goto failed;
			}
			req.obj_name = tok.tokv.str;
			// Token moved
			tok = pre_next_token(prep, 0);
			if ((tok.tokt != PPTOK_SYM) || (tok.tokv.sym != SYM_COMMA)) {
				printf("Error: invalid requests file: invalid GO line %d (comma)\n", lineno);
				string_del(req.obj_name);
				preproc_token_del(&tok);
				goto failed;
			}
			// Empty destructor
			tok = pre_next_token(prep, 0);
			if ((tok.tokt == PPTOK_IDENT) || (tok.tokt == PPTOK_STRING)) {
				req.def.fun.typ = (tok.tokt == PPTOK_STRING) ? tok.tokv.sstr : tok.tokv.str;
				req.has_default = 1;
				// Token moved
				tok = pre_next_token(prep, 0);
				if ((req.def.rty == RQT_FUN_2) || (req.def.rty == RQT_FUN_D)) {
					if ((tok.tokt != PPTOK_SYM) || (tok.tokv.sym != SYM_COMMA)) {
						printf("Error: invalid requests file: invalid GO line %d (comma 2)\n", lineno);
						string_del(req.obj_name);
						string_del(req.def.fun.typ);
						preproc_token_del(&tok);
						goto failed;
					}
					// Empty destructor
					tok = pre_next_token(prep, 0);
					if (tok.tokt != PPTOK_IDENT) {
						printf("Error: invalid requests file: invalid GO line %d (redirect)\n", lineno);
						string_del(req.obj_name);
						string_del(req.def.fun.typ);
						preproc_token_del(&tok);
						goto failed;
					}
					req.def.fun.fun2 = tok.tokv.str;
					// Token moved
					tok = pre_next_token(prep, 0);
				}
				if ((tok.tokt != PPTOK_SYM) || (tok.tokv.sym != SYM_RPAREN)) {
					printf("Error: invalid requests file: invalid GO line %d (rparen)\n", lineno);
					string_del(req.obj_name);
					string_del(req.def.fun.typ);
					if (req.def.fun.fun2) string_del(req.def.fun.fun2);
					preproc_token_del(&tok);
					goto failed;
				}
				// Empty destructor
				tok = pre_next_token(prep, 0);
			}
			if (tok.tokt != PPTOK_NEWLINE) {
				printf("Error: invalid requests file: invalid GO line %d (newline)\n", lineno);
				string_del(req.obj_name);
				if (req.def.fun.typ) string_del(req.def.fun.typ);
				if (req.def.fun.fun2) string_del(req.def.fun.fun2);
				preproc_token_del(&tok);
				goto failed;
			}
			if (if_depth == entered_depth) {
				if (!vector_push(requests, ret, req)) {
					printf("Error: failed to add request for %s\n", string_content(req.obj_name));
					string_del(req.obj_name);
					if (req.def.fun.typ) string_del(req.def.fun.typ);
					if (req.def.fun.fun2) string_del(req.def.fun.fun2);
					// Empty destructor
					goto failed;
				}
			}
			++lineno;
		} else if ((tok.tokt == PPTOK_IDENT)
		        && (!strcmp(string_content(tok.tokv.str), "DATA")
		         || !strcmp(string_content(tok.tokv.str), "DATAV")
		         || !strcmp(string_content(tok.tokv.str), "DATAB")
		         || !strcmp(string_content(tok.tokv.str), "DATAM"))) {
			request_t req = {
				.has_default = 1,
				.default_comment = is_comment,
				.has_val = 0,
				.obj_name = NULL,
				.weak = (string_content(tok.tokv.str)[4] == 'V'),
				.def = {
					.rty =
						(string_content(tok.tokv.str)[4] == 'V') ? RQT_DATAV :
						(string_content(tok.tokv.str)[4] == 'B') ? RQT_DATAB :
						(string_content(tok.tokv.str)[4] == 'M') ? RQT_DATAM : RQT_DATA,
					.dat.has_size = 0,
					.dat.sz = 0,
				},
			};
			string_del(tok.tokv.str);
			tok = pre_next_token(prep, 0);
			if ((tok.tokt != PPTOK_SYM) || (tok.tokv.sym != SYM_LPAREN)) {
				printf("Error: invalid requests file: invalid DATA line %d (lparen)\n", lineno);
				preproc_token_del(&tok);
				goto failed;
			}
			// Empty destructor
			tok = pre_next_token(prep, 0);
			if (tok.tokt != PPTOK_IDENT) {
				printf("Error: invalid requests file: invalid DATA line %d (obj_name)\n", lineno);
				preproc_token_del(&tok);
				goto failed;
			}
			req.obj_name = tok.tokv.str;
			// Token moved
			tok = pre_next_token(prep, 0);
			if ((tok.tokt != PPTOK_SYM) || (tok.tokv.sym != SYM_COMMA)) {
				printf("Error: invalid requests file: invalid DATA line %d (comma)\n", lineno);
				string_del(req.obj_name);
				preproc_token_del(&tok);
				goto failed;
			}
			// Empty destructor
			tok = pre_next_token(prep, 0);
			if (tok.tokt == PPTOK_NUM) {
				num_constant_t cst;
				if (!num_constant_convert(tok.tokv.str, &cst)) {
					printf("Error: invalid requests file: invalid DATA line %d (num conversion)\n", lineno);
					string_del(req.obj_name);
					preproc_token_del(&tok);
					goto failed;
				}
				switch (cst.typ) {
				case NCT_FLOAT:
				case NCT_DOUBLE:
				case NCT_LDOUBLE:
					printf("Error: invalid requests file: invalid DATA line %d (num conversion)\n", lineno);
					string_del(req.obj_name);
					preproc_token_del(&tok);
					goto failed;
				case NCT_INT32:  req.def.dat.sz = (size_t)cst.val.i32; break;
				case NCT_UINT32: req.def.dat.sz = (size_t)cst.val.u32; break;
				case NCT_INT64:  req.def.dat.sz = (size_t)cst.val.i64; break;
				case NCT_UINT64: req.def.dat.sz = (size_t)cst.val.u64; break;
				}
				req.def.dat.has_size = 1;
				// Token moved
				tok = pre_next_token(prep, 0);
				if ((tok.tokt != PPTOK_SYM) || (tok.tokv.sym != SYM_RPAREN)) {
					printf("Error: invalid requests file: invalid DATA line %d (rparen)\n", lineno);
					string_del(req.obj_name);
					preproc_token_del(&tok);
					goto failed;
				}
				// Empty destructor
				tok = pre_next_token(prep, 0);
			}
			if (tok.tokt != PPTOK_NEWLINE) {
				printf("Error: invalid requests file: invalid DATA line %d (newline)\n", lineno);
				string_del(req.obj_name);
				preproc_token_del(&tok);
				goto failed;
			}
			if (if_depth == entered_depth) {
				if (!vector_push(requests, ret, req)) {
					printf("Error: failed to add request for %s\n", string_content(req.obj_name));
					string_del(req.obj_name);
					// Empty destructor
					goto failed;
				}
			}
			++lineno;
		} else {
			if (is_comment) {
				preproc_token_del(&tok);
				tok = pre_next_newline_token(prep); // Returns a newline
				++lineno;
				continue;
			}
			printf("Error: invalid requests file: invalid token:\n");
			preproc_token_print(&tok);
			preproc_token_del(&tok);
			goto failed;
		}
	}
	
failed:
	prepare_del(prep);
	vector_del(requests, ret);
	return NULL;
	
success:
	prepare_del(prep);
	return ret;
}

static int is_simple_type_ptr_to(type_t *typ, int *needs_D, int *needs_my) {
	switch (typ->typ) {
	case TYPE_BUILTIN:
		return 1; // Assume pointers to builtin are simple
	case TYPE_ARRAY:
		if (typ->val.array.array_sz == (size_t)-1) return 0; // VLA are not simple
		return is_simple_type_ptr_to(typ->val.array.typ, needs_D, needs_my);
	case TYPE_STRUCT_UNION:
		if (typ->val.st->explicit_simple) return 1;
		if (typ->_internal_use) return 1; // Recursive structures are OK as long as every other members are OK
		if (!typ->val.st->is_defined) return 1; // Undefined structures are OK since they are opaque
		typ->_internal_use = 1;
		for (size_t i = 0; i < typ->val.st->nmembers; ++i) {
			st_member_t *mem = &typ->val.st->members[i];
			if (!is_simple_type_ptr_to(mem->typ, needs_D, needs_my)) {
				typ->_internal_use = 0;
				return 0;
			}
		}
		typ->_internal_use = 0;
		return 1;
	case TYPE_ENUM:
		return is_simple_type_ptr_to(typ->val.typ, needs_D, needs_my);
	case TYPE_PTR:
		return is_simple_type_ptr_to(typ->val.typ, needs_D, needs_my);
	case TYPE_FUNCTION:
		*needs_my = 1;
		return 1;
	default:
		printf("Error: is_simple_type_ptr_to on unknown type %u\n", typ->typ);
		return 0;
	}
}
static int is_simple_type(type_t *typ, int *needs_D, int *needs_my) {
	switch (typ->typ) {
	case TYPE_BUILTIN:
		return 1; // Assume pointers to builtin are simple
	case TYPE_ARRAY:
		if (typ->val.array.array_sz == (size_t)-1) return 0; // VLA are not simple
		return is_simple_type_ptr_to(typ->val.array.typ, needs_D, needs_my);
	case TYPE_STRUCT_UNION:
		if (typ->val.st->explicit_simple) return 1;
		if (typ->_internal_use) return 1; // Recursive structures are OK as long as every other members are OK
		// if (!typ->val.st->is_defined) return 1; // Undefined structures are OK since they are opaque
		// To be safe, don't allow opaque structures
		if (!typ->val.st->is_defined) return 0;
		typ->_internal_use = 1;
		for (size_t i = 0; i < typ->val.st->nmembers; ++i) {
			st_member_t *mem = &typ->val.st->members[i];
			if (!is_simple_type(mem->typ, needs_D, needs_my)) {
				typ->_internal_use = 0;
				return 0;
			}
		}
		typ->_internal_use = 0;
		return 1;
	case TYPE_ENUM:
		return is_simple_type(typ->val.typ, needs_D, needs_my);
	case TYPE_PTR:
		return is_simple_type_ptr_to(typ->val.typ, needs_D, needs_my);
	case TYPE_FUNCTION:
		// Functions should be handled differently (GO instead of DATA)
		return 0;
	default:
		printf("Error: is_simple_type on unknown type %u\n", typ->typ);
		return 0;
	}
}

static int convert_type(string_t *dest, type_t *typ, int is_ret, int *needs_D, int *needs_my, string_t *obj_name) {
	if (typ->is_atomic) {
		printf("TODO: convert_type for atomic types\n");
		return 0;
	}
	switch (typ->typ) {
	case TYPE_BUILTIN: {
		int has_char = 0;
		char c;
		switch (typ->val.builtin) {
		case BTT_VOID: has_char = 1; c = 'v'; break;
		case BTT_BOOL: has_char = 1; c = 'i'; break;
		case BTT_CHAR: has_char = 1; c = 'c'; break;
		case BTT_SCHAR: has_char = 1; c = 'c'; break;
		case BTT_UCHAR: has_char = 1; c = 'C'; break;
		case BTT_SHORT: has_char = 1; c = 'w'; break;
		case BTT_SSHORT: has_char = 1; c = 'w'; break;
		case BTT_USHORT: has_char = 1; c = 'W'; break;
		case BTT_INT: has_char = 1; c = 'i'; break;
		case BTT_SINT: has_char = 1; c = 'i'; break;
		case BTT_UINT: has_char = 1; c = 'u'; break;
		case BTT_LONG: has_char = 1; c = 'l'; break;
		case BTT_SLONG: has_char = 1; c = 'l'; break;
		case BTT_ULONG: has_char = 1; c = 'L'; break;
		case BTT_LONGLONG: has_char = 1; c = 'I'; break;
		case BTT_SLONGLONG: has_char = 1; c = 'I'; break;
		case BTT_ULONGLONG: has_char = 1; c = 'U'; break;
		case BTT_INT128: has_char = 1; c = 'H'; break; // TODO: Is 'H' for signed and unsigned?
		case BTT_SINT128: has_char = 1; c = 'H'; break; // Is 'H' for signed and unsigned?
		case BTT_UINT128: has_char = 1; c = 'H'; break; // Is 'H' for signed and unsigned?
		case BTT_S8: has_char = 1; c = 'c'; break;
		case BTT_U8: has_char = 1; c = 'C'; break;
		case BTT_S16: has_char = 1; c = 'w'; break;
		case BTT_U16: has_char = 1; c = 'W'; break;
		case BTT_S32: has_char = 1; c = 'i'; break;
		case BTT_U32: has_char = 1; c = 'u'; break;
		case BTT_S64: has_char = 1; c = 'I'; break;
		case BTT_U64: has_char = 1; c = 'U'; break;
		case BTT_FLOAT: has_char = 1; c = 'f'; break;
		case BTT_CFLOAT: has_char = 1; c = 'x'; break;
		case BTT_IFLOAT: has_char = 1; c = 'f'; break;
		case BTT_DOUBLE: has_char = 1; c = 'd'; break;
		case BTT_CDOUBLE: has_char = 1; c = 'X'; break;
		case BTT_IDOUBLE: has_char = 1; c = 'd'; break;
		case BTT_LONGDOUBLE: *needs_D = 1; has_char = 1; c = 'D'; break;
		case BTT_CLONGDOUBLE: *needs_D = 1; has_char = 1; c = 'Y'; break;
		case BTT_ILONGDOUBLE: *needs_D = 1; has_char = 1; c = 'D'; break;
		case BTT_VA_LIST: *needs_my = 1; has_char = 1; c = 'A'; break;
		default:
			printf("Error: convert_type on unknown builtin %u\n", typ->val.builtin);
			return 0;
		}
		if (has_char) {
			if (!string_add_char(dest, c)) {
				printf("Error: failed to add type char for %s\n", builtin2str[typ->val.builtin]);
				return 0;
			}
			return 1;
		} else {
			printf("Internal error: unknown state builtin=%u\n", typ->val.builtin);
			return 0;
		} }
	case TYPE_ARRAY:
		printf("Error: convert_type on raw array\n");
		return 0;
	case TYPE_STRUCT_UNION:
		if (!typ->is_validated || typ->is_incomplete) {
			printf("Error: incomplete return type for %s\n", string_content(obj_name));
			return 0;
		}
		if (is_ret) {
			if (typ->szinfo.size <= 8) {
				if (!string_add_char(dest, 'U')) {
					printf("Error: failed to add type char for structure return\n");
					return 0;
				}
				return 1;
			} else if (typ->szinfo.size <= 16) {
				if (!string_add_char(dest, 'H')) {
					printf("Error: failed to add type char for large structure return\n");
					return 0;
				}
				return 1;
			} else {
				if (!string_add_char(dest, 'p')) {
					printf("Error: failed to add type char for very large structure return\n");
					return 0;
				}
				return 1;
			}
		} else {
			if (typ->val.st->nmembers == 1) {
				return convert_type(dest, typ->val.st->members[0].typ, is_ret, needs_D, needs_my, obj_name);
			}
			printf("TODO: convert_type on structure as argument (%s)\n", string_content(obj_name));
			return 0;
		}
	case TYPE_ENUM:
		return convert_type(dest, typ->val.typ, is_ret, needs_D, needs_my, obj_name);
	case TYPE_PTR:
		if ((typ->val.typ->typ == TYPE_STRUCT_UNION) && typ->val.typ->val.st->tag && !strcmp(string_content(typ->val.typ->val.st->tag), "_IO_FILE")) {
			if (!string_add_char(dest, 'S')) {
				printf("Error: failed to add type char for %s\n", builtin2str[typ->val.builtin]);
				return 0;
			}
			return 1;
		}
		if (is_simple_type_ptr_to(typ->val.typ, needs_D, needs_my)) {
			if (!string_add_char(dest, 'p')) {
				printf("Error: failed to add type char for simple pointer\n");
				return 0;
			}
			return 1;
		} else {
			*needs_my = 1;
			if (!string_add_char(dest, 'p')) {
				printf("Error: failed to add type char for %s\n", builtin2str[typ->val.builtin]);
				return 0;
			}
			return 1;
		}
	case TYPE_FUNCTION:
		printf("Error: convert_type on raw function\n");
		return 0;
	default:
		printf("Error: convert_type on unknown type %u\n", typ->typ);
		return 0;
	}
}
static int convert_type_post(string_t *dest, type_t *typ, string_t *obj_name) {
	if (typ->is_atomic) {
		printf("TODO: convert_type_post for atomic types\n");
		return 0;
	}
	switch (typ->typ) {
	case TYPE_BUILTIN: return 1;
	case TYPE_ARRAY: return 1;
	case TYPE_STRUCT_UNION:
		if (!typ->is_validated || typ->is_incomplete) {
			printf("Error: incomplete return type for %s\n", string_content(obj_name));
			return 0;
		}
		if (typ->szinfo.size <= 16) {
			return 1;
		} else {
			if (!string_add_char(dest, 'p')) {
				printf("Error: failed to add type char for very large structure return as parameter\n");
				return 0;
			}
			return 2;
		}
	case TYPE_ENUM: return 1;
	case TYPE_PTR: return 1;
	case TYPE_FUNCTION: return 1;
	default:
		printf("Error: convert_type_post on unknown type %u\n", typ->typ);
		return 0;
	}
}

int solve_request(request_t *req, type_t *typ) {
	if (typ->typ == TYPE_FUNCTION) {
		int needs_D = 0, needs_my = req->has_default && (req->def.rty == RQT_FUN_MY), needs_2 = 0;
		int convert_post;
		req->val.fun.typ = string_new();
		if (!req->val.fun.typ) {
			printf("Error: failed to create function type string\n");
			return 0;
		}
		if (!convert_type(req->val.fun.typ, typ->val.fun.ret, 1, &needs_D, &needs_my, req->obj_name)) goto fun_fail;
		if (!string_add_char(req->val.fun.typ, 'F')) {
			printf("Error: failed to add convention char\n");
			goto fun_fail;
		}
		if (req->has_default && (req->def.rty == RQT_FUN_MY) && (string_content(req->def.fun.typ)[2] == 'E')) {
			if (!string_add_char(req->val.fun.typ, 'E')) {
				printf("Error: failed to add emu char\n");
				goto fun_fail;
			}
		}
		convert_post = convert_type_post(req->val.fun.typ, typ->val.fun.ret, req->obj_name);
		if (!convert_post) goto fun_fail;
		if (typ->val.fun.nargs == (size_t)-1) {
			printf("Warning: assuming empty specification is void specification\n");
			if (convert_post == 1) {
				if (!string_add_char(req->val.fun.typ, 'v')) {
					printf("Error: failed to add void specification char\n");
					goto fun_fail;
				}
			}
		} else if (!typ->val.fun.nargs && !typ->val.fun.has_varargs) {
			if (convert_post == 1) {
				if (!string_add_char(req->val.fun.typ, 'v')) {
					printf("Error: failed to add void specification char\n");
					goto fun_fail;
				}
			}
		} else {
			for (size_t i = 0; i < typ->val.fun.nargs; ++i) {
				if (!convert_type(req->val.fun.typ, typ->val.fun.args[i], 0, &needs_D, &needs_my, req->obj_name)) goto fun_fail;
			}
			if (typ->val.fun.has_varargs) {
				needs_my = 1;
				if (!string_add_char(req->val.fun.typ, 'V')) {
					printf("Error: failed to add type char for %s\n", builtin2str[typ->val.builtin]);
					goto fun_fail;
				}
			}
		}
		
	// fun_succ:
		if (req->has_default && (req->def.rty == RQT_FUN_2) && !needs_my) {
			needs_2 = 1;
			req->val.fun.fun2 = string_dup(req->def.fun.fun2);
			if (!req->val.fun.fun2) {
				printf("Error: failed to duplicate string (request for function %s with default redirection)\n", string_content(req->obj_name));
				return 0;
			}
		} else if (req->has_default && (req->def.rty == RQT_FUN_D) && !needs_my) {
			needs_2 = 0;
			req->val.fun.fun2 = string_dup(req->def.fun.fun2);
			if (!req->val.fun.fun2) {
				printf("Error: failed to duplicate string (request for function %s with long double types)\n", string_content(req->obj_name));
				return 0;
			}
		} else if (!needs_my && needs_D) {
			req->val.fun.fun2 = string_new();
			if (!req->val.fun.fun2) {
				printf("Error: failed to create empty string (request for function %s with long double types)\n", string_content(req->obj_name));
				return 0;
			}
		}
		req->val.rty =
			needs_my ? RQT_FUN_MY :
			needs_2 ? RQT_FUN_2 :
			needs_D ? RQT_FUN_D : RQT_FUN;
		req->has_val = 1;
		return 1;
		
	fun_fail:
		string_del(req->val.fun.typ);
		return 0;
	} else {
		int needs_D = 0, needs_my = req->has_default && (req->def.rty == RQT_FUN_MY);
		if (is_simple_type(typ, &needs_D, &needs_my)) {
			// TODO: Hmm...
			req->val.rty = needs_my ? RQT_DATAM : req->has_default ? req->def.rty : req->weak ? RQT_DATAV : RQT_DATA;
			req->val.dat.has_size = 1;
			req->val.dat.sz = typ->szinfo.size;
			req->has_val = 1;
			return 1;
		} else {
			printf("Error: TODO: solve_request for data %s with non-simple type ", string_content(req->obj_name));
			type_print(typ);
			printf("\n");
			return 0;
		}
	}
}
int solve_request_map(request_t *req, khash_t(type_map) *decl_map) {
	khiter_t it = kh_get(type_map, decl_map, string_content(req->obj_name));
	if (it == kh_end(decl_map)) {
		if (string_content(req->obj_name)[0] != '_') {
			printf("Error: %s was not declared\n", string_content(req->obj_name));
		}
		return 0;
	}
	return solve_request(req, kh_val(decl_map, it));
}
int solve_requests(VECTOR(requests) *reqs, khash_t(type_map) *decl_map) {
	int ret = 1;
	vector_for(requests, req, reqs) {
		if (!solve_request_map(req, decl_map)) ret = 0;
	}
	return ret;
}
