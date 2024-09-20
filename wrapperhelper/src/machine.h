#pragma once

#ifndef MACHINE_H
#define MACHINE_H

#include "cstring.h"
#include "khash.h"
#include "vector.h"

struct type_s;  // lang.h

typedef struct machine_s {
	// Preprocessor
	size_t npaths;
	char **include_path;
	
	// Parsing
	size_t size_long;
	size_t align_valist, size_valist;
	_Bool unsigned_char;
	// Structure parsing
	_Bool unnamed_bitfield_aligns;
} machine_t;

int init_machines(size_t npaths, const char *const *extra_include_path);
void del_machines(void);
machine_t *convert_machine_name(const char *archname);

int validate_type(machine_t *target, struct type_s *typ);

#endif // MACHINE_H
