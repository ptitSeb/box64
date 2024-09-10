#pragma once

#ifndef GENERATOR_H
#define GENERATOR_H

#include <stdio.h>

#include "cstring.h"
#include "lang.h"

typedef struct request_s {
	string_t *obj_name;
	_Bool default_comment;
	_Bool has_val, ignored;
	_Bool weak;
	struct {
		enum request_type_e {
			RQT_FUN,
			RQT_FUN_2,
			RQT_FUN_MY,
			RQT_FUN_D,
			
			RQT_DATA,
			RQT_DATAB,
			RQT_DATAM,
		} rty;
		union {
			struct {
				string_t *typ;
				string_t *fun2;
			} fun;
			struct {
				int has_size;
				size_t sz;
			} dat;
		};
	} def, val;
} request_t;
typedef struct reference_s {
	enum {
		REF_REQ,
		REF_LINE,
		REF_IFDEF,
		REF_IFNDEF,
		REF_ELSE,
		REF_ENDIF,
	} typ;
	union {
		request_t req;
		string_t *line;
	};
} reference_t;
VECTOR_DECLARE(references, reference_t)
void request_print(const request_t *req);
void request_print_check(const request_t *req);
void references_print_check(const VECTOR(references) *refs);
void output_from_references(FILE *f, const VECTOR(references) *reqs);

VECTOR(references) *references_from_file(const char *filename, FILE *f); // Takes ownership of f
int solve_request(request_t *req, type_t *typ);
int solve_request_map(request_t *req, khash_t(type_map) *decl_map);
int solve_references(VECTOR(references) *reqs, khash_t(type_map) *decl_map);

#endif // GENERATOR_H
