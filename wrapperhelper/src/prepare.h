#pragma once

#ifndef PREPARE_H
#define PREPARE_H

#include <stdio.h>

#include "cstring.h"
#include "lang.h"

typedef struct prepare_s prepare_t;

prepare_t *prepare_new_file(FILE *f, const char *filename); // Takes ownership of f
void prepare_del(prepare_t *src);
preproc_token_t pre_next_token(prepare_t *src, int allow_comments);

void prepare_set_line(prepare_t *src, char *filename, size_t lineno); // Takes ownership of filename if != NULL

void prepare_mark_nocomment(prepare_t *src); // Change the state (usually from COMMENT) to NONE
int pre_next_newline_token(prepare_t *src, string_t *buf); // In a comment append everything until the EOL or EOF to the buffer

void prepare_cleanup(void); // Frees loginfo filenames

#endif // PREPARE_H
