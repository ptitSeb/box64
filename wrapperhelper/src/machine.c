#include "machine.h"

#include <stdio.h>

#include "lang.h"

machine_t machine_x86_64;
machine_t machine_x86;
machine_t machine_aarch64;

#define PASTE2(a, b) a ## b
#define PASTE(a, b) PASTE2(a, b)
#define STRINGIFY2(a) #a
#define STRINGIFY(a) STRINGIFY2(a)
#define MACHINE_STR STRINGIFY(CUR_MACHINE)
#define MACHINE_VAL PASTE(machine_, CUR_MACHINE)

#define PATHS_OFFSET_PRE 2 // There are two paths that are always included before any other
#define ADD_PATH(path) \
	if (!(MACHINE_VAL.include_path[failure_id] = strdup(path))) {               \
		log_memory("failed to add include path to " MACHINE_STR " platform\n"); \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _paths));                        \
	}                                                                           \
	++failure_id;
#define INIT_PATHS \
	MACHINE_VAL.npaths = PATHS_OFFSET_PRE + npaths + paths_offset_post;         \
	if (!(MACHINE_VAL.include_path =                                            \
	      malloc(MACHINE_VAL.npaths * sizeof *MACHINE_VAL.include_path))) {     \
		log_memory("failed to add include path to " MACHINE_STR " platform\n"); \
		goto PASTE(failed_, PASTE(CUR_MACHINE, _nopath));                       \
	}                                                                           \
	failure_id = 0;                                                             \
	ADD_PATH("include-override/" MACHINE_STR)                                   \
	ADD_PATH("include-override/common")                                         \
	while (failure_id < PATHS_OFFSET_PRE + npaths) {                            \
		ADD_PATH(extra_include_path[failure_id - PATHS_OFFSET_PRE])             \
	}

int init_machines(size_t npaths, const char *const *extra_include_path) {
	size_t failure_id;
	
	size_t paths_offset_post = 0;
#define DO_PATH(_path) ++paths_offset_post;
#include "machine.gen"
#undef DO_PATH
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-malloc-leak"
#define CUR_MACHINE x86_64
	machine_x86_64.size_long = 8;
	machine_x86_64.align_longdouble = 16;
	machine_x86_64.size_longdouble = 16;
	machine_x86_64.align_valist = 8;
	machine_x86_64.size_valist = 24;
	machine_x86_64.max_align = 8;
	machine_x86_64.has_int128 = 1;
	machine_x86_64.unsigned_char = 0;
	machine_x86_64.unnamed_bitfield_aligns = 0;
	INIT_PATHS
#define DO_PATH ADD_PATH
#include "machine.gen"
#undef DO_PATH
#undef CUR_MACHINE
	
#define CUR_MACHINE x86
	machine_x86.size_long = 4;
	machine_x86.align_longdouble = 4;
	machine_x86.size_longdouble = 12;
	machine_x86.align_valist = 4;
	machine_x86.size_valist = 4;
	machine_x86.max_align = 4;
	machine_x86.has_int128 = 0;
	machine_x86.unsigned_char = 0;
	machine_x86.unnamed_bitfield_aligns = 0;
	INIT_PATHS
#define DO_PATH ADD_PATH
#include "machine.gen"
#undef DO_PATH
#undef CUR_MACHINE
	
#define CUR_MACHINE aarch64
	machine_aarch64.size_long = 8;
	machine_aarch64.align_longdouble = 16;
	machine_aarch64.size_longdouble = 16;
	machine_aarch64.align_valist = 8;
	machine_aarch64.size_valist = 32;
	machine_aarch64.max_align = 8;
	machine_aarch64.has_int128 = 1;
	machine_aarch64.unsigned_char = 1;
	machine_aarch64.unnamed_bitfield_aligns = 1;
	INIT_PATHS
#define DO_PATH ADD_PATH
#include "machine.gen"
#undef DO_PATH
#undef CUR_MACHINE
#pragma GCC diagnostic pop
	
	return 1;
	
failed_aarch64_paths:
	while (failure_id--) {
		free(machine_aarch64.include_path[failure_id]);
	}
	free(machine_aarch64.include_path);
failed_aarch64_nopath:
	failure_id = machine_x86.npaths;
failed_x86_paths:
	while (failure_id--) {
		free(machine_x86.include_path[failure_id]);
	}
	free(machine_x86.include_path);
failed_x86_nopath:
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
	for (size_t path_no = m->npaths; path_no--;) {
		free(m->include_path[path_no]);
	}
	free(m->include_path);
}
void del_machines(void) {
	machine_del(&machine_x86_64);
	machine_del(&machine_x86);
	machine_del(&machine_aarch64);
}

machine_t *convert_machine_name(const char *archname) {
	if (!strcmp(archname, "x86_64"))
		return &machine_x86_64;
	if (!strcmp(archname, "x86"))
		return &machine_x86;
	if (!strcmp(archname, "aarch64"))
		return &machine_aarch64;
	return NULL;
}

void fill_self_recursion(type_t *typ, struct_t *st) {
	if (typ->_internal_use) return; // Recursion, but not self recursion
	typ->_internal_use = 1;
	switch (typ->typ) {
	case TYPE_BUILTIN: break;
	case TYPE_ARRAY:
		fill_self_recursion(typ->val.array.typ, st);
		break;
	case TYPE_PTR:
		fill_self_recursion(typ->val.typ, st);
		break;
	case TYPE_FUNCTION:
		if (typ->val.fun.nargs != (size_t)-1) {
			for (size_t i = 0; (i < typ->val.fun.nargs) && !st->has_self_recursion; ++i) {
				fill_self_recursion(typ->val.fun.args[i], st);
			}
		}
		if (!st->has_self_recursion) fill_self_recursion(typ->val.fun.ret, st);
		break;
	case TYPE_STRUCT_UNION:
		if (typ->val.st == st) {
			st->has_self_recursion = 1;
			break;
		}
		if (!typ->val.st->is_defined) break;
		for (size_t i = 0; (i < typ->val.st->nmembers) && !st->has_self_recursion; ++i) {
			fill_self_recursion(typ->val.st->members[i].typ, st);
		}
		break;
	case TYPE_ENUM:
		fill_self_recursion(typ->val.typ, st);
		break;
	}
	typ->_internal_use = 0;
}

int validate_type(loginfo_t *loginfo, machine_t *target, type_t *typ) {
	if (typ->is_validated) return 1;
	typ->is_validated = 1;
	if (typ->is_restrict) {
		if (typ->typ != TYPE_PTR) {
			log_error(loginfo, "only pointers to object types may be restrict-qualified\n");
			return 0;
		}
		if (typ->val.typ->typ == TYPE_FUNCTION) {
			log_error(loginfo, "only pointers to object types may be restrict-qualified\n");
			return 0;
		}
	}
	if (typ->is_atomic) {
		if ((typ->typ == TYPE_ARRAY) || (typ->typ == TYPE_FUNCTION)) {
			log_error(loginfo, "array types and function types may not be atomic-qualified\n");
			return 0;
		}
	}
	switch (typ->typ) {
	case TYPE_BUILTIN:
		switch (typ->val.builtin) {
		case BTT_VOID:        typ->szinfo.align = typ->szinfo.size = 0; return 1;
		case BTT_BOOL:
		case BTT_CHAR:
		case BTT_SCHAR:
		case BTT_UCHAR:
		case BTT_S8:
		case BTT_U8:          typ->szinfo.align = typ->szinfo.size = 1; return 1;
		case BTT_SHORT:
		case BTT_SSHORT:
		case BTT_USHORT:
		case BTT_S16:
		case BTT_U16:         typ->szinfo.align = typ->szinfo.size = 2; return 1;
		case BTT_INT:
		case BTT_SINT:
		case BTT_UINT:
		case BTT_S32:
		case BTT_U32:         typ->szinfo.align = typ->szinfo.size = 4; return 1;
		case BTT_LONGLONG:
		case BTT_SLONGLONG:
		case BTT_ULONGLONG:
		case BTT_S64:
		case BTT_U64:         typ->szinfo.align = target->max_align; typ->szinfo.size = 8; return 1;
		case BTT_LONG:
		case BTT_SLONG:
		case BTT_ULONG:       typ->szinfo.align = typ->szinfo.size = target->size_long; return 1;
		case BTT_FLOAT:
		case BTT_IFLOAT:      typ->szinfo.align = typ->szinfo.size = 4; return 1;
		case BTT_CFLOAT:
		case BTT_DOUBLE:
		case BTT_IDOUBLE:     typ->szinfo.align = target->max_align; typ->szinfo.size = 8; return 1;
		case BTT_CDOUBLE:     typ->szinfo.align = target->max_align; typ->szinfo.size = 16; return 1;
		case BTT_LONGDOUBLE:
		case BTT_ILONGDOUBLE: typ->szinfo.align = target->align_longdouble; typ->szinfo.size = target->size_longdouble; return 1;
		case BTT_CLONGDOUBLE: typ->szinfo.align = target->align_longdouble; typ->szinfo.size = 2*target->size_longdouble; return 1;
		case BTT_INT128:
		case BTT_SINT128:
		case BTT_UINT128:
			if (!target->has_int128) {
				if (loginfo->filename) log_error(loginfo, "target does not have type __int128\n");
				typ->szinfo.align = typ->szinfo.size = 0; return 0;
			}
			/* FALLTHROUGH */
		case BTT_FLOAT128:
		case BTT_IFLOAT128:   typ->szinfo.align = typ->szinfo.size = 16; return 1;
		case BTT_CFLOAT128:   typ->szinfo.align = 16; typ->szinfo.size = 32; return 1;
		case BTT_VA_LIST:     typ->szinfo.align = target->align_valist; typ->szinfo.size = target->size_valist; return 1;
		default:
			log_error(loginfo, "unknown builtin %u, cannot fill size info\n", typ->val.builtin);
			return 0;
		}
	case TYPE_ARRAY:
		if (typ->val.array.typ->is_incomplete || (typ->val.array.typ->typ == TYPE_FUNCTION)) {
			log_error(loginfo, "array types must point to complete object types\n");
			return 0;
		}
		if ((typ->val.array.typ->typ == TYPE_STRUCT_UNION) && typ->val.array.typ->val.st->has_incomplete) {
			log_error(loginfo, "array types may not (inductively) point to structures which last element is incomplete\n");
			return 0;
		}
		if ((typ->is_atomic) || (typ->is_const) || (typ->is_restrict) || (typ->is_volatile)) {
			// qualifier-type-list in array declaration is only allowed in function argument declaration under certain circumstances
			log_error(loginfo, "array types may not be qualified\n");
			return 0;
		}
		if (!validate_type(loginfo, target, typ->val.array.typ)) return 0;
		if (typ->val.array.array_sz == (size_t)-1) {
			typ->szinfo.size = 0;
			typ->szinfo.align = (typ->val.array.typ->szinfo.align < 16) ? 16 : typ->val.array.typ->szinfo.align;
		} else {
			typ->szinfo.size = typ->val.array.array_sz * typ->val.array.typ->szinfo.size;
			typ->szinfo.align =
				((typ->szinfo.size >= 16) && (typ->val.array.typ->szinfo.align < 16)) ?
				16 :
				typ->val.array.typ->szinfo.align;
		}
		return 1;
	case TYPE_PTR:
		typ->szinfo.size = target->size_long;
		typ->szinfo.align = target->size_long;
		return validate_type(loginfo, target, typ->val.typ);
	case TYPE_FUNCTION:
		if ((typ->val.fun.ret->typ == TYPE_FUNCTION) || (typ->val.fun.ret->typ == TYPE_ARRAY)) {
			log_error(loginfo, "function types may not return function or array types\n");
			return 0;
		}
		if (typ->val.fun.nargs != (size_t)-1) {
			for (size_t i = 0; i < typ->val.fun.nargs; ++i) {
				// Adjust the argument if necessary
				// Assume arrays are already converted
				if (typ->val.fun.args[i]->typ == TYPE_ARRAY) {
					log_error(loginfo, "function argument %zu is an array\n", i + 1);
					return 0;
				}
				if (typ->val.fun.args[i]->typ == TYPE_FUNCTION) {
					// Adjustment to pointer
					type_t *t2 = type_new_ptr(typ->val.fun.args[i]);
					if (!t2) {
						log_error(loginfo, "failed to adjust type of argument from function to pointer\n");
						return 0;
					}
					typ->val.fun.args[i] = t2;
				}
				if (!validate_type(loginfo, target, typ->val.fun.args[i])) return 0;
			}
		}
		typ->szinfo.size = 0;
		typ->szinfo.align = 0;
		return validate_type(loginfo, target, typ->val.fun.ret);
	case TYPE_STRUCT_UNION: {
		if (!typ->val.st->is_defined) return typ->is_incomplete;
		size_t max_align = 1, cur_sz = 0; unsigned char cur_bit = 0;
		for (size_t i = 0; i < typ->val.st->nmembers; ++i) {
			// Adjust the argument if necessary
			st_member_t *mem = &typ->val.st->members[i];
			if (mem->typ->typ == TYPE_FUNCTION) {
				log_error(loginfo, "structures may not contain function members\n");
				return 0;
			}
			if (mem->typ->is_incomplete) {
				if ((i != typ->val.st->nmembers - 1) || !typ->val.st->is_struct || (mem->typ->typ != TYPE_ARRAY)) {
					// The last element of a structure may be a VLA
					log_error(loginfo, "structures may not contain incomplete members\n");
					return 0;
				}
				typ->val.st->has_incomplete = 1;
			}
			if (!validate_type(loginfo, target, mem->typ)) return 0;
			if (!typ->val.st->has_self_recursion) fill_self_recursion(mem->typ, typ->val.st);
			if (!typ->val.st->is_struct && (mem->typ->typ == TYPE_STRUCT_UNION)) {
				typ->val.st->has_incomplete |= mem->typ->val.st->has_incomplete;
			} else if ((mem->typ->typ == TYPE_STRUCT_UNION) && mem->typ->val.st->has_incomplete) {
				log_error(loginfo, "structures may not (inductively) contain structures which last element is incomplete\n");
				return 0;
			} else if (typ->val.st->is_struct) {
				if (mem->typ->is_incomplete) {
					if (i && (i == typ->val.st->nmembers - 1) && (mem->typ->typ == TYPE_ARRAY)) {
						typ->val.st->has_incomplete |= mem->typ->val.st->has_incomplete;
					} else {
						log_error(loginfo, "structures may not have any incomplete element, except that the last, but not first, element may be an incomplete array\n");
						return 0;
					}
				}
			}
			mem->byte_offset = cur_sz;
			mem->bit_offset = cur_bit;
			if (mem->is_bitfield) {
				if (!typ->val.st->is_struct) {
					log_error(loginfo, "TODO: bitfield in union\n");
					return 0;
				}
				if (mem->typ->is_atomic) {
					log_error(loginfo, "atomic bitfields are not supported\n");
					return 0;
				}
				if (mem->typ->typ != TYPE_BUILTIN) {
					log_error(loginfo, "bitfields can only have a specific subset of types\n");
					return 0;
				}
				if ((mem->typ->val.builtin != BTT_BOOL) && (mem->typ->val.builtin != BTT_CHAR)
				 && (mem->typ->val.builtin != BTT_SHORT) && (mem->typ->val.builtin != BTT_INT)
				 && (mem->typ->val.builtin != BTT_LONG) && (mem->typ->val.builtin != BTT_LONGLONG)
				 && (mem->typ->val.builtin != BTT_SCHAR) && (mem->typ->val.builtin != BTT_UCHAR)
				 && (mem->typ->val.builtin != BTT_SSHORT) && (mem->typ->val.builtin != BTT_USHORT)
				 && (mem->typ->val.builtin != BTT_SINT) && (mem->typ->val.builtin != BTT_UINT)
				 && (mem->typ->val.builtin != BTT_SLONG) && (mem->typ->val.builtin != BTT_ULONG)
				 && (mem->typ->val.builtin != BTT_S8) && (mem->typ->val.builtin != BTT_U8)
				 && (mem->typ->val.builtin != BTT_S16) && (mem->typ->val.builtin != BTT_U16)
				 && (mem->typ->val.builtin != BTT_S32) && (mem->typ->val.builtin != BTT_U32)
				 && (mem->typ->val.builtin != BTT_S64) && (mem->typ->val.builtin != BTT_U64)) {
					// C standard: allow _Bool, (s/u)int
					// Implementation: also allow (u/s)char, (u/s)short, (u/s)long, (u/s)long long, [u]intx_t
					log_error(loginfo, "bitfields can only have a specific subset of types\n");
					return 0;
				}
				if (mem->typ->szinfo.size < mem->bitfield_width / 8) {
					log_error(loginfo, "bitfield member %c%s%c has width (%zu) greater than its container size (%zu * 8)\n",
						mem->name ? '\'' : '<',
						mem->name ? string_content(mem->name) : "unnamed",
						mem->name ? '\'' : '>',
						mem->bitfield_width,
						mem->typ->szinfo.size);
					return 0;
				}
				if (mem->bitfield_width) {
					if ((target->unnamed_bitfield_aligns || mem->name) && (max_align < mem->typ->szinfo.align)) max_align = mem->typ->szinfo.align;
					size_t cur_block = cur_sz / mem->typ->szinfo.align;
					size_t end_block = (cur_sz + (cur_bit + mem->bitfield_width - 1) / 8) / mem->typ->szinfo.align;
					if (cur_block == end_block) {
						cur_sz += mem->bitfield_width / 8;
						cur_bit += mem->bitfield_width % 8;
						cur_sz += cur_bit / 8;
						cur_bit %= 8;
					} else {
						cur_sz = ((cur_sz + mem->typ->szinfo.align - 1) & ~(mem->typ->szinfo.align - 1)) + (mem->bitfield_width / 8);
						cur_bit = mem->bitfield_width % 8;
					}
				} else {
					if (max_align < mem->typ->szinfo.align) max_align = mem->typ->szinfo.align;
					cur_sz = ((cur_sz + mem->typ->szinfo.align - 1) & ~(mem->typ->szinfo.align - 1)) + mem->typ->szinfo.size;
					log_error(loginfo, "TODO: unnamed zero-width bitfield member\n");
					return 0;
				}
			} else {
				if (max_align < mem->typ->szinfo.align) max_align = mem->typ->szinfo.align;
				if (typ->val.st->is_struct) {
					if (cur_bit) {
						cur_bit = 0;
						++cur_sz;
					}
					cur_sz = ((cur_sz + mem->typ->szinfo.align - 1) & ~(mem->typ->szinfo.align - 1)) + mem->typ->szinfo.size;
				} else {
					if (cur_sz < mem->typ->szinfo.size) cur_sz = mem->typ->szinfo.size;
				}
			}
		}
		if (cur_bit) {
			cur_bit = 0;
			++cur_sz;
		}
		typ->szinfo.align = max_align;
		typ->szinfo.size = (cur_sz + max_align - 1) & ~(max_align - 1);
		return 1; }
	case TYPE_ENUM:
		if (typ->val.typ->typ != TYPE_BUILTIN) {
			log_error(loginfo, "the underlying type of an enum is not a builtin type\n");
			return 0;
		}
		typ->szinfo = typ->val.typ->szinfo;
		return 1;
	}
	return 0;
}
