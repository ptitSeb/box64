#pragma once

#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>

#include "lang.h"

void dump_prepare(const char *filename, FILE *file);
void dump_preproc(const char *filename, FILE *file);
file_t *parse_file(const char *filename, FILE *file);

#endif // PARSE_H
