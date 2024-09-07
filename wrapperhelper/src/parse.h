#pragma once

#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>

#include "lang.h"
#include "machine.h"

void dump_prepare(const char *filename, FILE *file);
void dump_preproc(machine_t *target, const char *filename, FILE *file);
file_t *parse_file(machine_t *target, const char *filename, FILE *file);

#endif // PARSE_H
