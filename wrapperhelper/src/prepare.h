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
preproc_token_t pre_next_newline_token(prepare_t *src); // In a comment ignore everything until the EOL or EOF

#endif // PREPARE_H
