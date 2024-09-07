#include "machine.h"

#include "preproc_private.h"

machine_t machine_x86_64;
// machine_t machine_x86;
// machine_t machine_arm64;

#define PASTE2(a, b) a ## b
#define PASTE(a, b) PASTE2(a, b)
#define STRINGIFY2(a) #a
#define STRINGIFY(a) STRINGIFY2(a)
#define MACHINE_STR STRINGIFY(CUR_MACHINE)
#define EXTRA_PATHS \
	PASTE(machine_, CUR_MACHINE).npaths = PASTE(CUR_MACHINE, _NPATHS) + npaths;                                  \
	if (!(PASTE(machine_, CUR_MACHINE).include_path =                                                            \
	      malloc((PASTE(CUR_MACHINE, _NPATHS) + npaths) * sizeof *PASTE(machine_, CUR_MACHINE).include_path))) { \
		printf("Failed to add include path to " MACHINE_STR " platform\n");                                      \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _nopath));                                                        \
	}                                                                                                            \
	for (failure_id = 0; failure_id < npaths; ++failure_id) {                                                    \
		if (!(PASTE(machine_, CUR_MACHINE).include_path[failure_id] = strdup(extra_include_path[failure_id]))) { \
			printf("Failed to add include path to " MACHINE_STR " platform\n");                                  \
			goto PASTE(failed_, PASTE(CUR_MACHINE, _paths));                                                     \
		}                                                                                                        \
	}
#define ADD_PATH(path) \
	if (!(PASTE(machine_, CUR_MACHINE).include_path[failure_id] = strdup(path))) { \
		printf("Failed to add include path to " MACHINE_STR " platform\n");        \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _paths));                           \
	}                                                                              \
	++failure_id;
#define EXTRA_MACROS \
	PASTE(machine_, CUR_MACHINE).npredefs = PASTE(CUR_MACHINE, _NPREDEFS);                                       \
	if (!(PASTE(machine_, CUR_MACHINE).predef_macros_name =                                                      \
	      malloc((PASTE(CUR_MACHINE, _NPREDEFS)) * sizeof *PASTE(machine_, CUR_MACHINE).predef_macros_name))) {  \
		printf("Failed to add predefined macro to " MACHINE_STR " platform\n");                                  \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _paths));                                                         \
	}                                                                                                            \
	if (!(PASTE(machine_, CUR_MACHINE).predef_macros =                                                           \
	      malloc((PASTE(CUR_MACHINE, _NPREDEFS)) * sizeof *PASTE(machine_, CUR_MACHINE).predef_macros))) {       \
		printf("Failed to add predefined macro to " MACHINE_STR " platform\n");                                  \
		free(machine_x86_64.predef_macros_name);                                                                 \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _paths));                                                         \
	}                                                                                                            \
	failure_id = 0;
#define ADD_NAME(mname) \
	if (!(PASTE(machine_, CUR_MACHINE).predef_macros_name[failure_id] = strdup(#mname))) { \
		printf("Failed to add predefined macro to " MACHINE_STR " platform\n");            \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _macros));                                  \
	}
#define ADD_MACRO(ntoks) \
	if (!(PASTE(machine_, CUR_MACHINE).predef_macros[failure_id] =                   \
	      malloc(sizeof *PASTE(machine_, CUR_MACHINE).predef_macros[failure_id]))) { \
		printf("Failed to add predefined macro to " MACHINE_STR " platform\n");      \
		free(machine_x86_64.predef_macros_name[failure_id]);                         \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _macros));                            \
	}                                                                                \
	*PASTE(machine_, CUR_MACHINE).predef_macros[failure_id] = (macro_t){             \
		.is_funlike = 0,                                                             \
		.has_varargs = 0,                                                            \
		.nargs = 0,                                                                  \
		.toks = vector_new_cap(mtoken, (ntoks)),                                     \
	};                                                                               \
	++failure_id;                                                                    \
	if (!PASTE(machine_, CUR_MACHINE).predef_macros[failure_id - 1]->toks) {         \
		printf("Failed to add predefined macro to " MACHINE_STR " platform\n");      \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _macros));                            \
	}
#define ADD_SYM(s) \
	mtok = mtoken_new_token((preproc_token_t){.tokt = PPTOK_SYM, .tokv.sym = SYM_ ## s}); \
	if (!mtok) {                                                                          \
		printf("Failed to add predefined macro to " MACHINE_STR " platform\n");           \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _macros));                                 \
	}                                                                                     \
	vector_push(mtoken, PASTE(machine_, CUR_MACHINE).predef_macros[failure_id - 1]->toks, mtok);
#define ADD_STR(typ, n) \
	s = string_new_cstr(#n);                                                          \
	if (!s) {                                                                         \
		printf("Failed to add predefined macro to " MACHINE_STR " platform\n");       \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _macros));                             \
	}                                                                                 \
	mtok = mtoken_new_token((preproc_token_t){.tokt = PPTOK_ ## typ, .tokv.str = s}); \
	if (!mtok) {                                                                      \
		printf("Failed to add predefined macro to " MACHINE_STR " platform\n");       \
		string_del(s);                                                                \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _macros));                             \
	}                                                                                 \
	vector_push(mtoken, PASTE(machine_, CUR_MACHINE).predef_macros[failure_id - 1]->toks, mtok);

int init_machines(size_t npaths, const char *const *extra_include_path) {
	size_t failure_id;
	string_t *s;
	mtoken_t *mtok;
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-malloc-leak"
#define CUR_MACHINE x86_64
	machine_x86_64.size_long = 8;
#define x86_64_NPATHS 5
	EXTRA_PATHS
	ADD_PATH("include-fixed")
	ADD_PATH("/usr/lib/gcc/x86_64-pc-linux-gnu/14.2.1/include")
	ADD_PATH("/usr/local/include")
	ADD_PATH("/usr/lib/gcc/x86_64-pc-linux-gnu/14.2.1/include-fixed")
	ADD_PATH("/usr/include")
#define x86_64_NPREDEFS 9
	EXTRA_MACROS
	ADD_NAME(__x86_64__)
	ADD_MACRO(1)
	ADD_STR(NUM, 1)
	ADD_NAME(__WCHAR_MAX__)
	ADD_MACRO(1)
	ADD_STR(NUM, 2147483647)
	ADD_NAME(__WCHAR_MIN__)
	ADD_MACRO(5)
	ADD_SYM(LPAREN)
	ADD_SYM(DASH)
	ADD_STR(IDENT, __WCHAR_MAX__)
	ADD_SYM(DASH)
	ADD_STR(NUM, 1)
	ADD_NAME(__CHAR_BIT__)
	ADD_MACRO(1)
	ADD_STR(NUM, 8)
	ADD_NAME(__SCHAR_MAX__)
	ADD_MACRO(1)
	ADD_STR(NUM, 127)
	ADD_NAME(__SHRT_MAX__)
	ADD_MACRO(1)
	ADD_STR(NUM, 32767)
	ADD_NAME(__INT_MAX__)
	ADD_MACRO(1)
	ADD_STR(NUM, 2147483647)
	ADD_NAME(__LONG_MAX__)
	ADD_MACRO(1)
	ADD_STR(NUM, 9223372036854775807L)
	ADD_NAME(__LONG_LONG_MAX__)
	ADD_MACRO(1)
	ADD_STR(NUM, 9223372036854775807LL)
#undef CUR_MACHINE
#pragma GCC diagnostic pop
	
	return 1;
	
failed_x86_64_macros:
	while (failure_id--) {
		macro_del(machine_x86_64.predef_macros[failure_id]);
		free(machine_x86_64.predef_macros[failure_id]);
		free(machine_x86_64.predef_macros_name[failure_id]);
	}
	free(machine_x86_64.predef_macros);
	free(machine_x86_64.predef_macros_name);
	failure_id = machine_x86_64.npaths;
failed_x86_64_paths:
	while (failure_id--) {
		free(machine_x86_64.include_path[failure_id]);
	}
	free(machine_x86_64.include_path);
failed_x86_64_nopath:
	return 0;
}

static void machine_del(machine_t *m) {
	for (size_t predef_id = m->npredefs; predef_id--;) {
		macro_del(m->predef_macros[predef_id]);
		free(m->predef_macros[predef_id]);
		free(m->predef_macros_name[predef_id]);
	}
	free(m->predef_macros);
	free(m->predef_macros_name);
	for (size_t path_no = m->npaths; path_no--;) {
		free(m->include_path[path_no]);
	}
	free(machine_x86_64.include_path);
}
void del_machines(void) {
	machine_del(&machine_x86_64);
}
