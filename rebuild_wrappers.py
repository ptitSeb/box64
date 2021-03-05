#!/usr/bin/env python

import os
import sys

values = ['E', 'e', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'V', 'O', 'S', '2', 'P', 'G', 'N', 'M']
def splitchar(s):
	try:
		ret = [len(s), values.index(s[0])]
		for c in s[2:]:
			ret.append(values.index(c))
		return ret
	except ValueError as e:
		raise ValueError("Value is " + s) from e

def value(define):
	return define[9:-1] if define.startswith("!") else define[8:-1]

def splitdef(dnf, defines):
	cunjs = dnf.split(" || ")
	clauses = [c.split(" && ") for c in cunjs]
	
	ret = [len(cunjs)]
	
	for cunj in clauses:
		for c in cunj:
			ret.append(len(c))
	for cunj in clauses:
		for c in cunj:
			ret.append(defines.index(value(c)) * 2 + (1 if c.startswith("!") else 0))
	ret.append(0)
	return ret

def invert(define):
	return define[1:] if define.startswith("!") else ("!" + define)

def main(root, defines, files, ver):
	global values
	
	# Initialize variables: gbl for all values, redirects for redirections
	gbl = {}
	redirects = {}
	
	# First read the files inside the headers
	for filepath in files:
		filename = filepath.split("/")[-1]
		dependants = []
		with open(filepath, 'r') as file:
			for line in file:
				ln = line.strip()
				# If the line is a `#' line (#ifdef LD80BITS/#ifndef LD80BITS/header)
				if ln.startswith("#"):
					preproc_cmd = ln[1:].strip()
					try:
						if preproc_cmd.startswith("if defined(GO)"):
							continue #if defined(GO) && defined(GOM)...
						elif preproc_cmd.startswith("if !(defined(GO)"):
							continue #if !(defined(GO) && defined(GOM)...)
						elif preproc_cmd.startswith("error"):
							continue #error meh!
						elif preproc_cmd.startswith("endif"):
							if dependants != []:
								dependants.pop()
						elif preproc_cmd.startswith("ifdef"):
							if preproc_cmd[5:].strip() not in defines:
								raise KeyError(preproc_cmd[5:].strip())
							dependants.append("defined(" + preproc_cmd[5:].strip() + ")")
						elif preproc_cmd.startswith("ifndef"):
							if preproc_cmd[6:].strip() not in defines:
								raise KeyError(preproc_cmd[6:].strip())
							dependants.append("!defined(" + preproc_cmd[6:].strip() + ")")
						elif preproc_cmd.startswith("else"):
							dependants[-1] = invert(dependants[-1])
						else:
							raise NotImplementedError("Unknown preprocessor directive: {0} ({1}:{2})".format(
								preproc_cmd.split(" ")[0], filename, line[:-1]
							))
					except KeyError as k:
						raise NotImplementedError("Unknown key: {0} ({1}:{2})".format(
							k.args[0], filename, line[:-1]
						), k)
				# If the line is a `GO...' line (GO/GOM/GO2/...)...
				elif ln.startswith("GO"):
					# ... then look at the second parameter of the line
					ln = ln.split(",")[1].split(")")[0].strip()
					
					if ln[1] not in ["F"]:
						raise NotImplementedError("Bad middle letter {0} ({1}:{2})".format(ln[1], filename, line[:-1]))
					if any(c not in values for c in ln[2:]) or (('v' in ln[2:]) and (len(ln) > 3)):
						old = ln
						# This needs more work
						acceptables = ['v', '0', '1'] + values
						if any(c not in acceptables for c in ln[2:]):
							raise NotImplementedError("{0} ({1}:{2})".format(ln[2:], filename, line[:-1]))
						# Ok, this is acceptable: there is 0, 1 and/or void
						ln = (ln
							.replace("v", "")   # void   -> nothing
							.replace("0", "p")  # 0      -> pointer
							.replace("1", "i")) # 1      -> integer
						redirects.setdefault(" && ".join(dependants), {})
						redirects[" && ".join(dependants)][old] = ln
					# Simply append the function name if it's not yet existing
					gbl.setdefault(" && ".join(dependants), [])
					if ln not in gbl[" && ".join(dependants)]:
						gbl[" && ".join(dependants)].append(ln)
	
	gbl_vals = {}
	for k in gbl:
		ks = k.split(" && ")
		for v in gbl[k]:
			if k == "":
				gbl_vals[v] = []
				continue
			if v in gbl_vals:
				if gbl_vals[v] == []:
					continue
				for other_key in gbl_vals[v]:
					other_key_vals = other_key.split(" && ")
					for other_key_val in other_key_vals:
						if other_key_val not in ks:
							break
					else:
						break
				else:
					gbl_vals[v].append(k)
			else:
				gbl_vals[v] = [k]
	for v in gbl_vals:
		for k in gbl_vals[v]:
			if " && ".join([invert(v2) for v2 in k.split(" && ")]) in gbl_vals[v]:
				gbl_vals[v] = []
				break
	gbl = {}
	gbl_idxs = []
	for k in gbl_vals:
		if len(gbl_vals[k]) == 1:
			key = gbl_vals[k][0]
		else:
			key = "(" + (") || (".join(gbl_vals[k])) + ")"
		gbl[key] = gbl.get(key, []) + [k]
		if (key not in gbl_idxs) and (key != "()"):
			gbl_idxs.append(key)
	gbl_idxs.sort(key=lambda v: splitdef(v, defines))
	
	redirects_vals = {}
	for k in redirects:
		ks = k.split(" && ")
		for v in redirects[k]:
			if k == "":
				redirects_vals[(v, redirects[k][v])] = []
				continue
			if (v, redirects[k][v]) in redirects_vals:
				if redirects_vals[(v, redirects[k][v])] == []:
					continue
				for other_key in redirects_vals[(v, redirects[k][v])]:
					if other_key == "()":
						break
					other_key_vals = other_key.split(" && ")
					for other_key_val in other_key_vals:
						if other_key_val not in ks:
							break
					else:
						break
				else:
					redirects_vals[(v, redirects[k][v])].append(k)
			else:
				redirects_vals[(v, redirects[k][v])] = [k]
	redirects = {}
	redirects_idxs = []
	for k, v in redirects_vals:
		key = "(" + (") || (".join(redirects_vals[(k, v)])) + ")"
		if key in redirects:
			redirects[key].append([k, v])
		else:
			redirects[key] = [[k, v]]
		if (key not in redirects_idxs) and (key != "()"):
			redirects_idxs.append(key)
	redirects_idxs.sort(key=lambda v: splitdef(v, defines))
	
	# Sort the tables
	for k in gbl:
		gbl[k].sort(key=lambda v: splitchar(v))
	values = values + ['0', '1']
	for k in redirects:
		redirects[k].sort(key=lambda v: splitchar(v[0]) + [0] + splitchar(v[1]))
	values = values[:-2]
	
	# Check if there was any new functions
	functions_list = ""
	for k in ["()"] + gbl_idxs:
		for v in gbl[k]:
			functions_list = functions_list + "#" + k + " " + v + "\n"
	for k in ["()"] + redirects_idxs:
		for v in redirects[k]:
			functions_list = functions_list + "#" + k + " " + v[0] + " -> " + v[1] + "\n"
	
	# functions_list is a unique string, compare it with the last run
	try:
		last_run = ""
		with open(os.path.join(root, "src", "wrapped", "generated", "functions_list.txt"), 'r') as file:
			last_run = file.read()
		if last_run == functions_list:
			# Mark as OK for CMake
			with open(os.path.join(root, "src", "wrapped", "generated", "functions_list.txt"), 'w') as file:
				file.write(functions_list)
			#return 0
	except IOError:
		# The file does not exist yet, first run
		pass
	
	# Now the files rebuilding part
	# File headers and guards
	files_headers = {
		"wrapper.c": """/*****************************************************************
 * File automatically generated by rebuild_wrappers.py (v%s)
 *****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "wrapper.h"
#include "emu/x64emu_private.h"
#include "emu/x87emu_private.h"
#include "regs.h"
#include "x64emu.h"

typedef union ui64_s {
    int64_t     i;
    uint64_t    u;
    uint32_t    d[2];
} ui64_t;

typedef struct _2uint_struct_s {
	uint32_t	a;
	uint32_t	b;
} _2uint_struct_t;

extern void* my__IO_2_1_stderr_;
extern void* my__IO_2_1_stdin_ ;
extern void* my__IO_2_1_stdout_;

static void* io_convert(void* v)
{
	if(!v)
		return v;
	if(v==my__IO_2_1_stderr_)
		return stderr;
	if(v==my__IO_2_1_stdin_)
		return stdin;
	if(v==my__IO_2_1_stdout_)
		return stdout;
	return v;
}

typedef struct my_GValue_s
{
  int         g_type;
  union {
    int        v_int;
    int64_t    v_int64;
    uint64_t   v_uint64;
    float      v_float;
    double     v_double;
    void*      v_pointer;
  } data[2];
} my_GValue_t;

static void alignGValue(my_GValue_t* v, void* value)
{
    v->g_type = *(int*)value;
    memcpy(v->data, value+4, 2*sizeof(double));
}
static void unalignGValue(void* value, my_GValue_t* v)
{
    *(int*)value = v->g_type;
    memcpy(value+4, v->data, 2*sizeof(double));
}

void* VulkanFromx86(void* src, void** save);
void VulkanTox86(void* src, void* save);

#define ST0val ST0.d

int of_convert(int);
""",
		"wrapper.h": """/*****************************************************************
 * File automatically generated by rebuild_wrappers.py (v%s)
 *****************************************************************/
#ifndef __WRAPPER_H_
#define __WRAPPER_H_
#include <stdint.h>
#include <string.h>

typedef struct x64emu_s x64emu_t;

// the generic wrapper pointer functions
typedef void (*wrapper_t)(x64emu_t* emu, uintptr_t fnc);

// list of defined wrapper
// v = void, i = int32, u = uint32, U/I= (u)int64
// l = signed long, L = unsigned long (long is an int with the size of a pointer)
// p = pointer, P = callback
// f = float, d = double, D = long double, K = fake long double
// V = vaargs, E = current x86emu struct, e = ref to current x86emu struct
// 0 = constant 0, 1 = constant 1
// o = stdout
// C = unsigned byte c = char
// W = unsigned short w = short
// O = libc O_ flags bitfield
// S = _IO_2_1_stdXXX_ pointer (or FILE*)
// Q = ...
// 2 = struct of 2 uint
// P = Vulkan struture pointer
// G = a single GValue pointer
// N = ... automatically sending 1 arg
// M = ... automatically sending 2 args

"""
	}
	files_guards = {"wrapper.c": """""",
		"wrapper.h": """
#endif //__WRAPPER_H_
"""
	}
	
	# Rewrite the wrapper.h file:
	with open(os.path.join(root, "src", "wrapped", "generated", "wrapper.h"), 'w') as file:
		file.write(files_headers["wrapper.h"] % ver)
		for v in gbl["()"]:
			file.write("void " + v + "(x64emu_t *emu, uintptr_t fnc);\n")
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			for v in gbl[k]:
				file.write("void " + v + "(x64emu_t *emu, uintptr_t fnc);\n")
			file.write("#endif\n")
		file.write("\n")
		for v in redirects["()"]:
			file.write("void " + v[0] + "(x64emu_t *emu, uintptr_t fnc);\n")
		for k in redirects_idxs:
			file.write("\n#if " + k + "\n")
			for v in redirects[k]:
				file.write("void " + v[0] + "(x64emu_t *emu, uintptr_t fnc);\n")
			file.write("#endif\n")
		file.write(files_guards["wrapper.h"])
	
	# Rewrite the wrapper.c file:
	with open(os.path.join(root, "src", "wrapped", "generated", "wrapper.c"), 'w') as file:
		file.write(files_headers["wrapper.c"] % ver)
		
		# First part: typedefs
		for v in gbl["()"]:
			#         E            e             v       c         w          i          I          C          W           u           U           f        d         D              K         l           L            p        V        O          S        2         		 P        G        N      M
			types = ["x64emu_t*", "x64emu_t**", "void", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "float", "double", "long double", "double", "intptr_t", "uintptr_t", "void*", "void*", "int32_t", "void*", "_2uint_struct_t", "void*", "void*", "...", "..."]
			if len(values) != len(types):
					raise NotImplementedError("len(values) = {lenval} != len(types) = {lentypes}".format(lenval=len(values), lentypes=len(types)))
			
			file.write("typedef " + types[values.index(v[0])] + " (*" + v + "_t)"
				+ "(" + ', '.join(types[values.index(t)] for t in v[2:]) + ");\n")
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			for v in gbl[k]:
				#         E            e             v       c         w          i          I          C          W           u           U           f        d         D              K         l           L            p        V        O          S        2      			 P        G        N      M
				types = ["x64emu_t*", "x64emu_t**", "void", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "float", "double", "long double", "double", "intptr_t", "uintptr_t", "void*", "void*", "int32_t", "void*", "_2uint_struct_t", "void*", "void*", "...", "..."]
				if len(values) != len(types):
						raise NotImplementedError("len(values) = {lenval} != len(types) = {lentypes}".format(lenval=len(values), lentypes=len(types)))
				
				file.write("typedef " + types[values.index(v[0])] + " (*" + v + "_t)"
					+ "(" + ', '.join(types[values.index(t)] for t in v[2:]) + ");\n")
			file.write("#endif\n")
		
		file.write("\n")
		
		# Next part: function definitions
		
		# Helper variables
		reg_arg = ["R_RDI", "R_RSI", "R_RDX", "R_RCX", "R_R8", "R_R9"]
		# vreg: value is in a general register
		#         E  e  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  2  P  G  N, M
		vreg   = [0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 2]
		# vxmm: value is in a XMM register
		#         E  e  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  2  P  G  N, M
		vxmm   = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
		# vother: value is elsewere
		#         E  e  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  2  P  G  N, M
		vother = [1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0]
		# vstack: value is on the stack (or out of register)
		#         E  e  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  2  P  G  N, M
		vstack = [0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 2]
		arg_s = [
			"",											# E
			"",											# e
			"",											# v
			"*(int8_t*)(R_RSP + {p}), ",				# c
			"*(int16_t*)(R_RSP + {p}), ",	 			# w
			"*(int32_t*)(R_RSP + {p}), ",				# i
			"*(int64_t*)(R_RSP + {p}), ",				# I
			"*(uint8_t*)(R_RSP + {p}), ",				# C
			"*(uint16_t*)(R_RSP + {p}), ",				# W
			"*(uint32_t*)(R_RSP + {p}), ",				# u
			"*(uint64_t*)(R_RSP + {p}), ",				# U
			"*(float*)(R_RSP + {p}), ",					# f
			"*(double*)(R_RSP + {p}), ",				# d
			"*(long double*)(R_RSP + {p}), ",			# D
			"FromLD((void*)(R_RSP + {p})), ",			# K
			"*(intptr_t*)(R_RSP + {p}), ",				# l
			"*(uintptr_t*)(R_RSP + {p}), ",				# L
			"*(void**)(R_RSP + {p}), ",					# p
			"",					# V
			"of_convert(*(int32_t*)(R_RSP + {p})), ",	# O
			"io_convert(*(void**)(R_RSP + {p})), ",		# S
			"(_2uint_struct_t){{*(uintptr_t*)(R_RSP + {p}),*(uintptr_t*)(R_RSP + {p} + 4)}}, ",	# 2
			"",									# P
			"",								# G
			"*(void**)(R_RSP + {p}), ",					# N
			"*(void**)(R_RSP + {p}),*(void**)(R_RSP + {p} + 8), ",	# M
		]
		arg_r = [
			"",                                  		# E
			"",                                 		# e
			"",                                       # v
			"(int8_t){p}, ",              # c
			"(int16_t){p}, ",             # w
			"(int32_t){p}, ",             # i
			"(int64_t){p}, ",             # I
			"(uint8_t){p}, ",             # C
			"(uint16_t){p}, ",            # W
			"(uint32_t){p}, ",            # u
			"(uint64_t){p}, ",            # U
			"",               # f
			"",              # d
			"",         # D
			"",         # K
			"(intptr_t){p} ",            # l
			"(uintptr_t){p}, ",           # L
			"(void*){p}, ",               # p
			"",                 # V
			"of_convert((int32_t){p}), ", # O
			"io_convert((void*){p}), ",   # S
			"",	# 2
			"",                               # P
			"",                              # G
			"(void*){p}, ",				  # N
			"(void*){p}, ",	# M
		]
		arg_x = [
			"",                                  # E
			"",                                 # e
			"",                                       # v
			"",              # c
			"",             # w
			"",             # i
			"",             # I
			"",             # C
			"",            # W
			"",            # u
			"",            # U
			"emu->xmm[{p}].f[0]), ",               # f
			"emu->xmm[{p}].d[0]), ",              # d
			"",         # D
			"",         # K
			"",            # l
			"",           # L
			"",               # p
			"",                 # V
			"", # O
			"",   # S
			"",	# 2
			"",                               # P
			"",                              # G
			"",				  # N
			"",	# M
		]
		arg_o = [
			"emu, ",                                  # E
			"&emu, ",                                 # e
			"",                                       # v
			"",              # c
			"",             # w
			"",             # i
			"",             # I
			"",             # C
			"",            # W
			"",            # u
			"",            # U
			"",               # f
			"",              # d
			"",         # D
			"",         # K
			"",            # l
			"",           # L
			"",               # p
			"(void*)(R_RSP + {p}), ",                 # V
			"", # O
			"",   # S
			"",	# 2
			"arg{p}, ",                               # P
			"&arg{p}, ",                              # G
			"",					# N
			"",	# M
		]

		vals = [
			"\n#error Invalid return type: emulator\n",                     # E
			"\n#error Invalid return type: &emulator\n",                    # e
			"fn({0});",                                                     # v
			"R_RAX=fn({0});",                                               # c
			"R_RAX=fn({0});",                                               # w
			"R_RAX=fn({0});",                                               # i
			"R_RAX=(uint64_t)fn({0});",           							# I
			"R_RAX=(unsigned char)fn({0});",                                # C
			"R_RAX=(unsigned short)fn({0});",                               # W
			"R_RAX=(uint32_t)fn({0});",                                     # u
			"R_RAX=fn({0});", 												# U
			"emu->xmm[0].f[0]=fn({0});",             						# f
			"emu->xmm[0].d[0]=fn({0});",            						# d
			"long double ld=fn({0}); fpu_do_push(emu); ST0val = ld;",       # D
			"double db=fn({0}); fpu_do_push(emu); ST0val = db;",            # K
			"R_RAX=(intptr_t)fn({0});",                                     # l
			"R_RAX=(uintptr_t)fn({0});",                                    # L
			"R_RAX=(uintptr_t)fn({0});",                                    # p
			"\n#error Invalid return type: va_list\n",                      # V
			"\n#error Invalid return type: at_flags\n",                     # O
			"\n#error Invalid return type: _io_file*\n",                    # S
			"\n#error Invalid return type: _2uint_struct\n",                # 2
			"\n#error Invalid return type: Vulkan Struct\n",                # P
			"\n#error Invalid return type: GValue Pointer\n",               # G
			"\n#error Invalid return type: ... with 1 arg\n",               # N
			"\n#error Invalid return type: ... with 2 args\n",              # M
		]
		# Asserts
		if len(values) != len(arg_s):
			raise NotImplementedError("len(values) = {lenval} != len(arg_s) = {lenargs}".format(lenval=len(values), lenargr=len(arg_s)))
		if len(values) != len(arg_r):
			raise NotImplementedError("len(values) = {lenval} != len(arg_r) = {lenargr}".format(lenval=len(values), lenargr=len(arg_r)))
		if len(values) != len(vals):
			raise NotImplementedError("len(values) = {lenval} != len(vals) = {lenvals}".format(lenval=len(values), lenvals=len(vals)))
		
		# Helper functions to write the function definitions
		def function_args(args, d=8, r=0, x=0):
			if len(args) == 0:
				return ""
			
			if args[0] == "0":
				return "0, " + function_args(args[1:], d, r, x)
			elif args[0] == "1":
				return "1, " + function_args(args[1:], d, r, x)
			
			idx = values.index(args[0])
			if r<6 and vreg[idx]>0:
					if vreg[idx]==2 and r==6:
						return arg_r[idx-1].format(p=reg_arg[r]) + arg_s[idx-1].format(p=d) + function_args(args[1:], d + vother[idx-1]*8, r+1, x)
					if vreg[idx]==2 and r<6:
						return arg_r[idx].format(p=reg_arg[r]) + arg_r[idx].format(p=reg_arg[r+1]) + function_args(args[1:], d, r+2, x)
					return arg_r[idx].format(p=reg_arg[r]) + function_args(args[1:], d, r+1, x)
			if x<8 and vxmm[idx]>0:
					return arg_x[idx].format(p=x) + function_args(args[1:], d, r, x+1)
			if vstack[idx]>0:
    				return arg_s[idx].format(p=d) + function_args(args[1:], d+8*vstack[idx], r, x)
			return arg_o[idx].format(p=d) + function_args(args[1:], d + vother[idx]*8, r, x)
		
		def function_writer(f, N, W, rettype, args):
			f.write("void {0}(x64emu_t *emu, uintptr_t fcn) {2} {1} fn = ({1})fcn; ".format(N, W, "{"))
			if any(cc in 'PG' for cc in args):
				# Vulkan struct or GValue pointer, need to unwrap functions at the end
				delta = 4
				for c in args:
					if c == 'P':
						f.write("void* save{d}=NULL; void *arg{d} = VulkanFromx86(*(void**)(R_RSP + {d}), &save{d}); ".format(d=delta))
					if c == 'G':
    						f.write("my_GValue_t arg{d}; alignGValue(&arg{d}, *(void**)(R_RSP + {d})); ".format(d=delta))
					delta = delta + deltas[values.index(c)]
				f.write(vals[values.index(rettype)].format(function_args(args)[:-2]) + " ")
				delta = 4
				for c in args:
					if c == 'P':
						f.write("VulkanTox86(arg{d}, save{d}); ".format(d=delta))
					if c == 'G':
						f.write("unalignGValue(*(void**)(R_RSP + {d}), &arg{d}); ".format(d=delta))
					delta = delta + deltas[values.index(c)]
				f.write("}\n")
			else:
				# Generic function
				f.write(vals[values.index(rettype)].format(function_args(args)[:-2]) + " }\n")
		
		for v in gbl["()"]:
			function_writer(file, v, v + "_t", v[0], v[2:])
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			for v in gbl[k]:
				function_writer(file, v, v + "_t", v[0], v[2:])
			file.write("#endif\n")
		file.write("\n")
		for v in redirects["()"]:
			function_writer(file, v[0], v[1] + "_t", v[0][0], v[0][2:])
		for k in redirects_idxs:
			file.write("\n#if " + k + "\n")
			for v in redirects[k]:
				function_writer(file, v[0], v[1] + "_t", v[0][0], v[0][2:])
			file.write("#endif\n")
		
		file.write(files_guards["wrapper.c"])
	
	# Save the string for the next iteration, writing was successful
	with open(os.path.join(root, "src", "wrapped", "generated", "functions_list.txt"), 'w') as file:
		file.write(functions_list)
	
	return 0

if __name__ == '__main__':
	limit = []
	for i, v in enumerate(sys.argv):
		if v == "--":
			limit.append(i)
	if main(sys.argv[1], sys.argv[2:limit[0]], sys.argv[limit[0]+1:], "1.2.0.09") != 0:
		exit(2)
	exit(0)
