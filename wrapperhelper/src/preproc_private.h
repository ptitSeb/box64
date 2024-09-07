#pragma once

#ifndef PREPROC_PRIVATE_H
#define PREPROC_PRIVATE_H

#include <stdio.h>

#include "lang.h"

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

VECTOR_DECLARE(mtoken, mtoken_t*)

typedef struct macro_s {
	int is_funlike;
	int has_varargs;
	unsigned nargs;
	VECTOR(mtoken) *toks;
} macro_t;

mtoken_t *mtoken_new_token(preproc_token_t tok);
mtoken_t *mtoken_new_arg(unsigned argid, int as_string);
mtoken_t *mtoken_new_concat(mtoken_t *l, mtoken_t *r);
void mtoken_del(mtoken_t *tok);
void macro_del(macro_t *m);

#endif // PREPROC_PRIVATE_H
