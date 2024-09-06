#include <err.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#include "generator.h"
#include "lang.h"
#include "parse.h"
#include "khash.h"

static void help(char *arg0) {
	printf("Usage: %s --help\n"
	       "       %s [--prepare|--preproc|--proc] <filename_in>\n"
	       "       %s <filename_in> <filename_reqs> <filename_out>\n"
	       "\n"
	       "  --prepare  Dump all preprocessor tokens (prepare phase)\n"
	       "  --preproc  Dump all processor tokens (preprocessor phase)\n"
	       "  --proc     Dump all typedefs, declarations and constants (processor phase)\n"
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
	
	enum main_state ms;
	int off;
	
	if ((argc == 2) && !strcmp(argv[1], "--help")) {
		help(argv[0]);
		return 0;
	} else if (argc == 2) {
		ms = MAIN_PROC;
		off = 1;
	} else if ((argc == 3) && !strcmp(argv[1], "--prepare")) {
		ms = MAIN_PREPARE;
		off = 2;
	} else if ((argc == 3) && !strcmp(argv[1], "--preproc")) {
		ms = MAIN_PREPROC;
		off = 2;
	} else if ((argc == 3) && !strcmp(argv[1], "--proc")) {
		ms = MAIN_PROC;
		off = 2;
	} else if (argc == 4) {
		ms = MAIN_RUN;
		off = 1;
	} else {
		help(argv[0]);
		return 2;
	}
	
	if (!init_str2kw()) {
		return 2;
	}
	
	FILE *f = fopen(argv[off], "r");
	if (!f) {
		err(2, "Error: failed to open %s", argv[off]);
		return 2;
	}
	switch (ms) {
	case MAIN_RUN: {
		file_t *content = parse_file(argv[off], f); // Takes ownership of f
		if (!content) {
			printf("Error: failed to parse the file\n");
			del_str2kw();
			return 0;
		}
		
		FILE *ref = fopen(argv[off + 1], "r");
		if (!ref) {
			err(2, "Error: failed to open %s", argv[off + 1]);
			del_str2kw();
			return 2;
		}
		VECTOR(requests) *reqs = requests_from_file(argv[off + 1], ref);
		if (!reqs) {
			file_del(content);
			del_str2kw();
			return 2;
		}
		// vector_for(requests, req, reqs) request_print(req);
		if (!solve_requests(reqs, content->decl_map)) {
			printf("Warning: failed to solve all default requests\n");
		}
		// vector_for(requests, req, reqs) request_print(req);
		//vector_for(requests, req, reqs) request_print_check(req);
		FILE *out = fopen(argv[off + 2], "w");
		if (!out) {
			err(2, "Error: failed to open %s", argv[off + 1]);
			file_del(content);
			vector_del(requests, reqs);
			del_str2kw();
			return 2;
		}
		output_from_requests(out, reqs);
		fclose(out);
		vector_del(requests, reqs);
		file_del(content);
		del_str2kw();
		return 0; }
	case MAIN_PROC: {
		file_t *content = parse_file(argv[off], f); // Takes ownership of f
		if (!content) {
			printf("Error: failed to parse the file\n");
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
		del_str2kw();
		return 0; }
		
	case MAIN_PREPARE:
		dump_prepare(argv[off], f); // Takes ownership of f
		del_str2kw();
		return 0;
		
	case MAIN_PREPROC:
		dump_preproc(argv[off], f); // Takes ownership of f
		del_str2kw();
		return 0;
	}
	
	printf("<internal error> Failed to run mode %u\n", ms);
	return 2;
}
