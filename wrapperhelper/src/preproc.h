#pragma once

#ifndef PREPROC_H
#define PREPROC_H

#include <stdio.h>

#include "lang.h"
#include "machine.h"

typedef struct preproc_s preproc_t;

preproc_t *preproc_new_file(machine_t *target, FILE *f, char *dirname, const char *filename); // Takes ownership of f and dirname
proc_token_t proc_next_token(preproc_t *src);
int proc_unget_token(preproc_t *src, proc_token_t *tok);
void preproc_del(preproc_t *src);

#endif // PREPROC_H
