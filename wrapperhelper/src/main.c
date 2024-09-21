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
	       "       %s {-I/path/to/include}* [--prepare|--preproc|--proc] [--arch <arch>] <filename_in>\n"
	       "       %s {-I/path/to/include}* [--emu <arch>] [--target <arch>] <filename_in> <filename_reqs> <filename_out>\n"
	       "\n"
	       "  --prepare  Dump all preprocessor tokens (prepare phase)\n"
	       "  --preproc  Dump all processor tokens (preprocessor phase)\n"
	       "  --proc     Dump all typedefs, declarations and constants (processor phase, default)\n"
	       "  -I         Add a path to the list of system includes\n"
	       "\n"
	       "Filenames may be preceded by '-f' or '--filename'.\n"
	       "  <filename_in>    Parsing file\n"
	       "  <filename_reqs>  Reference file (example: wrappedlibc_private.h)\n"
	       "  <filename_out>   Output file\n"
	       "\n"
	       "You can also specify the target architecture:"
	       "  --arch <arch>    Use the architecture <arch>\n"
	       "  --emu <arch>     Use the architecture <arch> as emulated architecture\n"
	       "  --target <arch>  Use the architecture <arch> as target/running architecture\n"
	       "\n"
	       "  <arch> is one of 'x86_64', 'aarch64'\n",
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
	const char *archname = NULL, *emuname = NULL, *targetname = NULL;
	
	for (int i = 1; i < argc; ++i) {
		int isfile = 0;
		
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
		} else if (!strcmp(argv[i], "--arch")) {
			++i;
			if (i < argc) {
				archname = argv[i];
			} else {
				printf("Error: invalid '--arch' option in last position\n");
				help(argv[0]);
				return 0;
			}
		} else if (!strcmp(argv[i], "--emu")) {
			++i;
			if (i < argc) {
				emuname = argv[i];
			} else {
				printf("Error: invalid '--emu' option in last position\n");
				help(argv[0]);
				return 0;
			}
		} else if (!strcmp(argv[i], "--target")) {
			++i;
			if (i < argc) {
				targetname = argv[i];
			} else {
				printf("Error: invalid '--target' option in last position\n");
				help(argv[0]);
				return 0;
			}
		} else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--filename")) {
			++i;
			if (i < argc) {
				isfile = 1;
			} else {
				printf("Error: invalid '--filename' option in last position\n");
				help(argv[0]);
				return 0;
			}
		} else {
			isfile = 1;
		}
		if (isfile) {
			if (!in_file) {
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
	}
	switch (ms) {
	case MAIN_PREPARE:
	case MAIN_PREPROC:
	case MAIN_PROC:
	check_proc:
		if (!in_file || ref_file || out_file) {
			printf("Error: too many unknown options/not enough arguments\n");
			help(argv[0]);
			return 2;
		}
		if (emuname || targetname) {
			printf("Error: invalid option '--emu' or '--target' in prepare/preprocessor/processor mode\n");
			help(argv[0]);
			return 2;
		}
		if (!archname) archname = "x86_64";
		break;
	case MAIN_RUN:
		if (in_file && !ref_file && !out_file) {
			ms = MAIN_PROC;
			goto check_proc;
		}
		if (!in_file || !ref_file || !out_file) {
			printf("Error: too many unknown options/not enough arguments\n");
			help(argv[0]);
			return 2;
		}
		if (archname) {
			printf("Error: invalid option '--arch' in run mode\n");
			help(argv[0]);
			return 2;
		}
		if (!emuname) emuname = "x86_64";
		if (!targetname) targetname = "aarch64";
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
		machine_t *emu = convert_machine_name(emuname);
		if (!emu) {
			printf("Error: invalid emulation architecture '%s'\n", emuname);
		}
		machine_t *target = convert_machine_name(targetname);
		if (!target) {
			printf("Error: invalid target architecture '%s'\n", targetname);
		}
		
		file_t *emu_content = parse_file(emu, in_file, f); // Takes ownership of f
		if (!emu_content) {
			printf("Error: failed to parse the file\n");
			del_machines();
			del_str2kw();
			return 0;
		}
		
		f = fopen(in_file, "r");
		if (!f) {
			err(2, "Error: failed to re-open %s", in_file);
			return 2;
		}
		file_t *target_content = parse_file(target, in_file, f); // Takes ownership of f
		if (!target_content) {
			printf("Error: failed to parse the file\n");
			file_del(emu_content);
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
			file_del(emu_content);
			file_del(target_content);
			del_machines();
			del_str2kw();
			return 2;
		}
		// vector_for(references, req, refs) request_print(req);
		if (!solve_references(refs, emu_content->decl_map, target_content->decl_map, emu_content->relaxed_type_conversion)) {
			printf("Warning: failed to solve all default requests\n");
		}
		// vector_for(references, req, refs) request_print(req);
		references_print_check(refs);
		FILE *out = fopen(out_file, "w");
		if (!out) {
			err(2, "Error: failed to open %s", ref_file);
			file_del(emu_content);
			file_del(target_content);
			vector_del(references, refs);
			del_machines();
			del_str2kw();
			return 2;
		}
		output_from_references(out, refs);
		fclose(out);
		vector_del(references, refs);
		file_del(emu_content);
		file_del(target_content);
		del_machines();
		del_str2kw();
		return 0; }
	case MAIN_PROC: {
		machine_t *arch = convert_machine_name(archname);
		if (!arch) {
			printf("Error: invalid architecture '%s'\n", archname);
		}
		file_t *content = parse_file(arch, in_file, f); // Takes ownership of f
		if (!content) {
			printf("Error: failed to parse the file\n");
			del_machines();
			del_str2kw();
			return 0;
		}
		// print content
		const char *name;
		struct_t *st;
		string_t *str;
		type_t *typ;
		declaration_t *decl;
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
		kh_foreach(content->relaxed_type_conversion, typ, str,
			printf("Type conversion: %p -> %s\n", typ, string_content(str));
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
		kh_foreach(content->decl_map, name, decl,
			printf("Declaration: %s -> %p = %u/%p: ", name, decl, decl->storage, decl->typ);
			type_print(decl->typ);
			printf("\n")
		)
		file_del(content);
		del_machines();
		del_str2kw();
		return 0; }
		
	case MAIN_PREPARE:
		// Ignored --arch, since this phase does not depend on the architecture
		dump_prepare(in_file, f); // Takes ownership of f
		del_machines();
		del_str2kw();
		return 0;
		
	case MAIN_PREPROC: {
		machine_t *arch = convert_machine_name(archname);
		if (!arch) {
			printf("Error: invalid architecture '%s'\n", archname);
		}
		dump_preproc(arch, in_file, f); // Takes ownership of f
		del_machines();
		del_str2kw();
		return 0; }
	}
	
	printf("Internal error: failed to run mode %u\n", ms);
	return 2;
}
