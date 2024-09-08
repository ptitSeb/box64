#include <err.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#include "generator.h"
#include "lang.h"
#include "machine.h"
#include "parse.h"
#include "khash.h"

static void help(char *arg0) {
	printf("Usage: %s --help\n"
	       "       %s {-I/path/to/include}* [--prepare|--preproc|--proc] <filename_in>\n"
	       "       %s {-I/path/to/include}* <filename_in> <filename_reqs> <filename_out>\n"
	       "\n"
	       "  --prepare  Dump all preprocessor tokens (prepare phase)\n"
	       "  --preproc  Dump all processor tokens (preprocessor phase)\n"
	       "  --proc     Dump all typedefs, declarations and constants (processor phase)\n"
	       "  -I         Add a path to the list of system includes\n"
	       "\n"
	       "  <filename_in>    Parsing file\n"
	       "  <filename_reqs>  Reference file (example: wrappedlibc_private.h)\n"
	       "  <filename_out>   Output file\n",
	       arg0, arg0, arg0);
}

enum main_state {
	MAIN_RUN,
	MAIN_PREPARE,
	MAIN_PREPROC,
	MAIN_PROC,
};

int main(int argc, char **argv) {
	setbuf(stdout, NULL);
	if (!setlocale(LC_NUMERIC, "C")) {
		printf("Error: failed to set LC_NUMERIC to C\n");
		return 2;
	}
	
	enum main_state ms = MAIN_RUN;
	const char *in_file = NULL, *ref_file = NULL, *out_file = NULL;
	VECTOR(charp) *paths = vector_new(charp);
	
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "--help")) {
			help(argv[0]);
			return 0;
		} else if (!strcmp(argv[i], "--prepare")) {
			ms = MAIN_PREPARE;
		} else if (!strcmp(argv[i], "--preproc")) {
			ms = MAIN_PREPROC;
		} else if (!strcmp(argv[i], "--proc")) {
			ms = MAIN_PROC;
		} else if (!strcmp(argv[i], "-pthread")) {
			// Ignore
		} else if (!strcmp(argv[i], "-I") && (i + 1 < argc)) {
			if (!vector_push(charp, paths, argv[i + 1])) {
				printf("Error: failed to add path to buffer\n");
				return 2;
			}
			++i;
		} else if ((argv[i][0] == '-') && (argv[i][1] == 'I') && (argv[i][2] != '\0')) {
			if (!vector_push(charp, paths, argv[i] + 2)) {
				printf("Error: failed to add path to buffer\n");
				return 2;
			}
		} else if (!in_file) {
			in_file = argv[i];
		} else if (!ref_file) {
			ref_file = argv[i];
		} else if (!out_file) {
			out_file = argv[i];
		} else {
			printf("Error: too many unknown options considered as file\n");
			help(argv[0]);
			return 2;
		}
	}
	switch (ms) {
	case MAIN_PREPARE:
	case MAIN_PREPROC:
	case MAIN_PROC:
		if (!in_file || ref_file || out_file) {
			printf("Error: too many unknown options/not enough arguments\n");
			help(argv[0]);
			return 2;
		}
		break;
	case MAIN_RUN:
		if (in_file && !ref_file && !out_file) {
			ms = MAIN_PROC;
		} else if (!in_file || !ref_file || !out_file) {
			printf("Error: too many unknown options/not enough arguments\n");
			help(argv[0]);
			return 2;
		}
		break;
	}
	
	if (!init_str2kw()) {
		return 2;
	}
	if (!init_machines(vector_size(charp, paths), (const char*const*)vector_content(charp, paths))) {
		vector_del(charp, paths);
		return 2;
	}
	vector_del(charp, paths);
	
	FILE *f = fopen(in_file, "r");
	if (!f) {
		err(2, "Error: failed to open %s", in_file);
		return 2;
	}
	switch (ms) {
	case MAIN_RUN: {
		file_t *content = parse_file(&machine_x86_64, in_file, f); // Takes ownership of f
		if (!content) {
			printf("Error: failed to parse the file\n");
			del_machines();
			del_str2kw();
			return 0;
		}
		
		FILE *ref = fopen(ref_file, "r");
		if (!ref) {
			err(2, "Error: failed to open %s", ref_file);
			del_machines();
			del_str2kw();
			return 2;
		}
		VECTOR(references) *refs = references_from_file(ref_file, ref);
		if (!refs) {
			file_del(content);
			del_machines();
			del_str2kw();
			return 2;
		}
		// vector_for(references, req, refs) request_print(req);
		if (!solve_references(refs, content->decl_map)) {
			printf("Warning: failed to solve all default requests\n");
		}
		// vector_for(references, req, refs) request_print(req);
		references_print_check(refs);
		FILE *out = fopen(out_file, "w");
		if (!out) {
			err(2, "Error: failed to open %s", ref_file);
			file_del(content);
			vector_del(references, refs);
			del_machines();
			del_str2kw();
			return 2;
		}
		output_from_references(out, refs);
		fclose(out);
		vector_del(references, refs);
		file_del(content);
		del_machines();
		del_str2kw();
		return 0; }
	case MAIN_PROC: {
		file_t *content = parse_file(&machine_x86_64, in_file, f); // Takes ownership of f
		if (!content) {
			printf("Error: failed to parse the file\n");
			del_machines();
			del_str2kw();
			return 0;
		}
		// print content
		const char *name;
		struct_t *st;
		type_t *typ;
		num_constant_t cst;
		/* for (enum type_builtin_e i = 0; i < LAST_BUILTIN; ++i) {
			printf("Builtin %u: %p, ", i, content->builtins[i]);
			type_print(content->builtins[i]);
			printf("\n");
		} */
		kh_foreach(content->struct_map, name, st,
			printf("Struct: %s -> %p = ", name, st);
			struct_print(st);
			printf("\n")
		)
		kh_foreach_key(content->type_set, typ,
			printf("Type: %p = ", typ);
			type_print(typ);
			printf("\n")
		)
		kh_foreach(content->type_map, name, typ,
			printf("Typedef: %s -> %p = ", name, typ);
			type_print(typ);
			printf("\n")
		)
		kh_foreach(content->enum_map, name, typ,
			printf("Enum: %s -> %p = ", name, typ);
			type_print(typ);
			printf("\n")
		)
		kh_foreach(content->const_map, name, cst,
			printf("Constant: %s -> ", name);
			switch (cst.typ) {
			case NCT_FLOAT: printf("%ff", cst.val.f); break;
			case NCT_DOUBLE: printf("%f", cst.val.d); break;
			case NCT_LDOUBLE: printf("%Lfl", cst.val.l); break;
			case NCT_INT32: printf("%d", cst.val.i32); break;
			case NCT_UINT32: printf("%uu", cst.val.u32); break;
			case NCT_INT64: printf("%ldll", cst.val.i64); break;
			case NCT_UINT64: printf("%lullu", cst.val.u64); break;
			}
			printf("\n")
		)
		kh_foreach(content->decl_map, name, typ,
			printf("Declaration: %s -> %p = ", name, typ);
			type_print(typ);
			printf("\n")
		)
		file_del(content);
		del_machines();
		del_str2kw();
		return 0; }
		
	case MAIN_PREPARE:
		dump_prepare(in_file, f); // Takes ownership of f
		del_machines();
		del_str2kw();
		return 0;
		
	case MAIN_PREPROC:
		dump_preproc(&machine_x86_64, in_file, f); // Takes ownership of f
		del_machines();
		del_str2kw();
		return 0;
	}
	
	printf("<internal error> Failed to run mode %u\n", ms);
	return 2;
}
