#include <err.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#include "generator.h"
#include "lang.h"
#include "machine.h"
#include "parse.h"
#include "prepare.h"
#include "khash.h"

static void help(char *arg0) {
	printf("Usage: %s --help\n"
	       "       %s {-I/path/to/include}* [--gst] [--prepare|--preproc|--proc] [--arch <arch>|-32|-64|--32|--64] <filename_in>\n"
	       "       %s {-I/path/to/include}* [--gst] [[--emu <arch>] [--target <arch>]|-32|-64|--32|--64] <filename_in> <filename_reqs> <filename_out>\n"
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
	       "  -32  --32        Use the x86 architecture as arch/emulated, aarch64 as target\n"
	       "  -64  --64        Use the x86_64 architecture as arch/emulated, aarch64 as target\n"
	       "\n"
	       "  <arch> is one of 'x86', 'x86_64', 'aarch64'\n"
	       "\n"
	       "  --gst            Mark all structures with a tag starting with '_G' and ending with 'Class' as simple\n",
	       arg0, arg0, arg0);
}

enum main_state {
	MAIN_RUN,
	MAIN_PREPARE,
	MAIN_PREPROC,
	MAIN_PROC,
};

enum bits_state {
	BITS_NONE,
	BITS_32,
	BITS_64,
};

int is_gst = 0;

int main(int argc, char **argv) {
	setbuf(stdout, NULL);
	if (!setlocale(LC_NUMERIC, "C")) {
		log_error_nopos("failed to set LC_NUMERIC to C\n");
		return 2;
	}
	
	enum main_state ms = MAIN_RUN;
	enum bits_state bs = BITS_NONE;
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
		} else if (!strcmp(argv[i], "--gst")) {
			is_gst = 1;
		} else if (!strcmp(argv[i], "-pthread")) {
			// Ignore
		} else if (!strcmp(argv[i], "-I") && (i + 1 < argc)) {
			if (!vector_push(charp, paths, argv[i + 1])) {
				log_memory("failed to add path to buffer\n");
				return 2;
			}
			++i;
		} else if ((argv[i][0] == '-') && (argv[i][1] == 'I') && (argv[i][2] != '\0')) {
			if (!vector_push(charp, paths, argv[i] + 2)) {
				log_memory("failed to add path to buffer\n");
				return 2;
			}
		} else if (!strcmp(argv[i], "--arch")) {
			++i;
			if (i < argc) {
				archname = argv[i];
			} else {
				log_error_nopos("invalid '--arch' option in last position\n");
				help(argv[0]);
				return 0;
			}
		} else if (!strcmp(argv[i], "--emu")) {
			++i;
			if (i < argc) {
				emuname = argv[i];
			} else {
				log_error_nopos("invalid '--emu' option in last position\n");
				help(argv[0]);
				return 0;
			}
		} else if (!strcmp(argv[i], "--target")) {
			++i;
			if (i < argc) {
				targetname = argv[i];
			} else {
				log_error_nopos("invalid '--target' option in last position\n");
				help(argv[0]);
				return 0;
			}
		} else if (!strcmp(argv[i], "-32") || !strcmp(argv[i], "--32")) {
			bs = BITS_32;
		} else if (!strcmp(argv[i], "-64") || !strcmp(argv[i], "--64")) {
			bs = BITS_64;
		} else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--filename")) {
			++i;
			if (i < argc) {
				isfile = 1;
			} else {
				log_error_nopos("invalid '--filename' option in last position\n");
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
				log_error_nopos("too many unknown options considered as file\n");
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
			log_error_nopos("too many unknown options/not enough arguments\n");
			help(argv[0]);
			return 2;
		}
		if (emuname || targetname) {
			log_error_nopos("invalid option '--emu' or '--target' in prepare/preprocessor/processor mode\n");
			help(argv[0]);
			return 2;
		}
		if (bs != BITS_NONE) {
			if (archname) {
				log_error_nopos("invalid option '--arch' with '--32' or '--64' in prepare/preprocessor/processor mode\n");
				help(argv[0]);
				return 2;
			}
			archname = (bs == BITS_32) ? "x86" : "x86_64";
		}
		if (!archname) archname = "x86_64";
		break;
	case MAIN_RUN:
		if (in_file && !ref_file && !out_file) {
			ms = MAIN_PROC;
			goto check_proc;
		}
		if (!in_file || !ref_file || !out_file) {
			log_error_nopos("too many unknown options/not enough arguments\n");
			help(argv[0]);
			return 2;
		}
		if (archname) {
			log_error_nopos("invalid option '--arch' in run mode\n");
			help(argv[0]);
			return 2;
		}
		if (bs != BITS_NONE) {
			if (emuname || targetname) {
				log_error_nopos("invalid option '--emu' or '--target' with '--32' or '--64' in run mode\n");
				help(argv[0]);
				return 2;
			}
			emuname = (bs == BITS_32) ? "x86" : "x86_64";
			targetname = "aarch64";
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
		del_str2kw();
		return 2;
	}
	vector_del(charp, paths);
	
	FILE *f = fopen(in_file, "r");
	if (!f) {
		err(2, "Error: failed to open %s", in_file);
		del_machines();
		del_str2kw();
		return 2;
	}
	switch (ms) {
	case MAIN_RUN: {
		machine_t *emu = convert_machine_name(emuname);
		if (!emu) {
			log_error_nopos("invalid emulation architecture '%s'\n", emuname);
			del_machines();
			del_str2kw();
			return 0;
		}
		machine_t *target = convert_machine_name(targetname);
		if (!target) {
			log_error_nopos("invalid target architecture '%s'\n", targetname);
			del_machines();
			del_str2kw();
			return 0;
		}
		
		file_t *emu_content = parse_file(emu, in_file, f); // Takes ownership of f
		if (!emu_content) {
			log_error_nopos("failed to parse the file\n");
			del_machines();
			del_str2kw();
			prepare_cleanup();
			return 0;
		}
		
		f = fopen(in_file, "r");
		if (!f) {
			err(2, "Error: failed to re-open %s", in_file);
			file_del(emu_content);
			del_machines();
			del_str2kw();
			prepare_cleanup();
			return 2;
		}
		file_t *target_content = parse_file(target, in_file, f); // Takes ownership of f
		if (!target_content) {
			log_error_nopos("failed to parse the file\n");
			file_del(emu_content);
			del_machines();
			del_str2kw();
			prepare_cleanup();
			return 0;
		}
		
		FILE *ref = fopen(ref_file, "r");
		if (!ref) {
			err(2, "Error: failed to open %s", ref_file);
			file_del(emu_content);
			file_del(target_content);
			del_machines();
			del_str2kw();
			prepare_cleanup();
			return 2;
		}
		VECTOR(references) *refs = references_from_file(ref_file, ref);
		if (!refs) {
			file_del(emu_content);
			file_del(target_content);
			del_machines();
			del_str2kw();
			prepare_cleanup();
			return 2;
		}
		// vector_for(references, req, refs) request_print(req);
		if (target->size_long != emu->size_long) {
			if (!solve_references(refs, emu_content->decl_map, target_content->decl_map, emu_content->relaxed_type_conversion)) {
				log_warning_nopos("failed to solve all default requests\n");
			}
		} else {
			if (!solve_references_simple(refs, emu_content->decl_map, target_content->decl_map, emu_content->relaxed_type_conversion)) {
				log_warning_nopos("failed to solve all default requests\n");
			}
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
			prepare_cleanup();
			return 2;
		}
		output_from_references(out, refs);
		fclose(out);
		vector_del(references, refs);
		file_del(emu_content);
		file_del(target_content);
		del_machines();
		del_str2kw();
		prepare_cleanup();
		return 0; }
		
	case MAIN_PROC: {
		machine_t *arch = convert_machine_name(archname);
		if (!arch) {
			log_error_nopos("invalid architecture '%s'\n", archname);
		}
		file_t *content = parse_file(arch, in_file, f); // Takes ownership of f
		if (!content) {
			log_error_nopos("failed to parse the file\n");
			del_machines();
			del_str2kw();
			prepare_cleanup();
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
		prepare_cleanup();
		return 0; }
		
	case MAIN_PREPARE:
		// Ignored --arch, since this phase does not depend on the architecture
		dump_prepare(in_file, f); // Takes ownership of f
		del_machines();
		del_str2kw();
		prepare_cleanup();
		return 0;
		
	case MAIN_PREPROC: {
		machine_t *arch = convert_machine_name(archname);
		if (!arch) {
			log_error_nopos("invalid architecture '%s'\n", archname);
		}
		dump_preproc(arch, in_file, f); // Takes ownership of f
		del_machines();
		del_str2kw();
		prepare_cleanup();
		return 0; }
	}
	
	log_internal_nopos("failed to run mode %u\n", ms);
	del_machines();
	del_str2kw();
	prepare_cleanup();
	return 2;
}
