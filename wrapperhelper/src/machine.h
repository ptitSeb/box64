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
	// TODO: also have info on unnamed bitfields, etc
} machine_t;

extern machine_t machine_x86_64;
// extern machine_t machine_x86;
// extern machine_t machine_arm64;

int init_machines(size_t npaths, const char *const *extra_include_path);
void del_machines(void);

int validate_type(machine_t *target, struct type_s *typ);

#endif // MACHINE_H
