#!/usr/bin/env python

import os
import sys

values = ['E', 'e', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'V', 'O', 'S', 'N', 'M', 'H', 'P']
assert(all(c not in values[:i] for i, c in enumerate(values)))
def splitchar(s):
	"""
	splitchar -- Sorting key function for function signatures
	
	The longest strings are at the end, and for identical length, the string
	are sorted using a pseudo-lexicographic sort, where characters have a value
	of `values.index`.
	"""
	try:
		ret = [len(s), values.index(s[0])]
		for c in s[2:]:
			ret.append(values.index(c))
		return ret
	except ValueError as e:
		raise ValueError("Value is " + s) from e

def value(define):
	"""
	value -- Get the tested name of a defined( . )/!defined( . )
	"""
	return define[9:-1] if define.startswith("!") else define[8:-1]

def splitdef(dnf, defines):
	"""
	splitdef -- Sorting key function for #ifdefs
	
	All #if defined(...) are sorted first by the number of clauses, then by the
	number of '||' in each clause and then by the length of the tested names
	(left to right, inverted placed after non-inverted).
	"""
	cunjs = dnf.split(" || ")
	clauses = [c.split(" && ") for c in cunjs]
	
	ret = [len(cunjs)]
	
	for cunj in clauses:
		ret.append(len(cunj))
	for cunj in clauses:
		for c in cunj:
			# All values of "c" are included in defines (throw otherwise)
			ret.append(defines.index(value(c)) * 2 + (1 if c.startswith("!") else 0))
	ret.append(0)
	return ret

def invert(define):
	"""
	invert -- Transform a `defined()` into a `!defined()` and vice-versa.
	"""
	return define[1:] if define.startswith("!") else ("!" + define)

def main(root, defines, files, ver):
	"""
	main -- The main function
	
	root: the root path (where the CMakeLists.txt is located)
	defines: which `defined( . )` are allowed
	files: a list of files to parse (wrapped*.h)
	ver: version number
	"""
	
	global values
	
	# Initialize variables: gbl for all values, redirects for redirections
	# mytypedefs is a list of all "*FE*" types per filename
	gbl        = {}
	redirects  = {}
	mytypedefs = {}
	
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
							if dependants != []: # If the previous 2 lines were ignored, skip
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
					try:
						funname = ln.split(",")[0].split("(")[1].strip()
						ln = ln.split(",")[1].split(")")[0].strip()
					except IndexError as e:
						raise NotImplementedError("Invalid GO command: {0}:{1}".format(
							filename, line[:-1]
						))
					
					if len(ln) < 3:
						raise NotImplementedError("Type {0} too short ({1}:{2})".format(ln, filename, line[:-1]))
					if "E" in ln:
						if ("E" in ln[:2]) or ("E" in ln[3:]):
							raise NotImplementedError(
								"emu64_t* not as the first parameter ({0}:{1})".format(filename, line[:-1]))
						if len(ln) < 4:
							raise NotImplementedError("Type {0} too short ({1}:{2})".format(ln, filename, line[:-1]))
					
					if ln[1] not in ["F"]:
						raise NotImplementedError("Bad middle letter {0} ({1}:{2})".format(ln[1], filename, line[:-1]))
					
					if any(c not in values for c in ln[2:]) or (('v' in ln[2:]) and (len(ln) > 3)):
						old = ln
						# This needs more work
						acceptables = ['v', '0', '1'] + values
						if any(c not in acceptables for c in ln[2:]):
							raise NotImplementedError("{0} ({1}:{2})".format(ln[2:], filename, line[:-1]))
						# Ok, this is acceptable: there is 0, 1 and/or void
						ln = ln[:2] + (ln[2:]
							.replace("v", "")   # void   -> nothing
							.replace("0", "p")  # 0      -> pointer
							.replace("1", "i")) # 1      -> integer
						assert(len(ln) >= 3)
						redirects.setdefault(" && ".join(dependants), {})
						redirects[" && ".join(dependants)][old] = ln
					# Simply append the function type if it's not yet existing
					gbl.setdefault(" && ".join(dependants), [])
					if ln not in gbl[" && ".join(dependants)]:
						gbl[" && ".join(dependants)].append(ln)
					
					if ln[2] == "E":
						# filename isn't stored with the '_private.h' part
						if len(ln) > 3:
							ln = ln[:2] + ln[3:]
						else:
							ln = ln[:2] + "v"
						mytypedefs.setdefault(filename[:-10], {})
						mytypedefs[filename[:-10]].setdefault(ln, [])
						mytypedefs[filename[:-10]][ln].append(funname)
	
	if ("" not in gbl) or ("" not in redirects):
		print("\033[1;31mThere is suspiciously not many types...\033[m")
		print("Check the CMakeLists.txt file. If you are SURE there is nothing wrong"
		      " (as a random example, `set()` resets the variable...), then comment out the following return.")
		print("(Also, the program WILL crash later if you proceed.)")
		return 2 # Check what you did, not proceeding
	
	# Now, take all function types, and make a new table gbl_vals
	# This table contains all #if conditions for when a function type needs to
	# be generated. There is also a filter to avoid duplicate/opposite clauses.
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
							# Not a duplicate or more specific case
							# (could be a less specific one though)
							break
					else:
						break
				else:
					gbl_vals[v].append(k)
			else:
				gbl_vals[v] = [k]
	for v in gbl_vals:
		for k in gbl_vals[v]:
			vs = k.split(" && ")
			for i in range(len(vs)):
				if " && ".join(vs[:i] + [invert(vs[i])] + vs[i+1:]) in gbl_vals[v]:
					# Opposite clauses detected
					gbl_vals[v] = []
					break
	# Now create a new gbl and gbl_idxs
	# gbl will contain the final version of gbl (without duplicates, based on
	# gbl_vals)
	# gbl_idxs will contain all #if clauses
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
	# Sort the #if clauses as defined in `splitdef`
	gbl_idxs.sort(key=lambda v: splitdef(v, defines))
	
	# This map will contain all additional function types that are "redirected"
	# to an already defined type (with some remapping).
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
	# Also does the same trick as before (also helps keep the order
	# in the file deterministic)
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
	
	# Sort the function types as defined in `splitchar`
	for k in gbl:
		gbl[k].sort(key=lambda v: splitchar(v))
	values = values + ['0', '1']
	for k in redirects:
		redirects[k].sort(key=lambda v: splitchar(v[0]) + [-1] + splitchar(v[1]))
	values = values[:-2]
	mytypedefs_vals = dict((fn, sorted(mytypedefs[fn].keys(), key=lambda v: splitchar(v))) for fn in mytypedefs)
	for fn in mytypedefs:
		for v in mytypedefs_vals[fn]:
			mytypedefs[fn][v].sort()
	
	# Check if there was any new functions compared to last run
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
			print("Detected same build as last run, skipping")
			return 0
	except IOError:
		# The file does not exist yet, first run
		pass
	
	# Detect simple wrappings
	simple_wraps = {}
	
	# H could be allowed maybe?
	allowed_simply = "v"
	allowed_regs = "cCwWiuIUlLp"
	allowed_fpr = "fd"
	
	# Sanity checks
	forbidden_simple = "EeDKVOSNMHP"
	assert(len(allowed_simply) + len(allowed_regs) + len(allowed_fpr) + len(forbidden_simple) == len(values))
	assert(all(c not in allowed_regs for c in allowed_simply))
	assert(all(c not in allowed_simply + allowed_regs for c in allowed_fpr))
	assert(all(c not in allowed_simply + allowed_regs + allowed_fpr for c in forbidden_simple))
	assert(all(c in allowed_simply + allowed_regs + allowed_fpr + forbidden_simple for c in values))
	
	# Only search on real wrappers
	for k in ["()"] + gbl_idxs:
		for v in gbl[k]:
			regs_count = 0
			fpr_count  = 0
			
			if v[0] in forbidden_simple:
				continue
			for c in v[2:]:
				if c in allowed_regs:
					regs_count = regs_count + 1
				elif c in allowed_fpr:
					fpr_count = fpr_count + 1
				elif c in allowed_simply:
					continue
				else:
					break
			else:
				# No character in forbidden_simply
				if (regs_count <= 6) and (fpr_count <= 4):
					# All checks passed!
					simple_wraps.setdefault(k, []).append(v)
	simple_idxs = list(simple_wraps.keys())
	simple_idxs.sort(key=lambda v: [-1] if v == "()" else splitdef(v, defines))
	
	# Now the files rebuilding part
	# File headers and guards
	files_header = {
		"wrapper.c": """/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v{version}) *
 *******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "wrapper.h"
#include "emu/x64emu_private.h"
#include "emu/x87emu_private.h"
#include "regs.h"
#include "x64emu.h"

extern void* my__IO_2_1_stdin_ ;
extern void* my__IO_2_1_stdout_;
extern void* my__IO_2_1_stderr_;

static void* io_convert(void* v)
{lbr}
	if(!v)
		return v;
	if(v == my__IO_2_1_stdin_)
		return stdin;
	if(v == my__IO_2_1_stdout_)
		return stdout;
	if(v == my__IO_2_1_stderr_)
		return stderr;
	return v;
{rbr}

#define ST0val ST0.d

int of_convert(int);

""",
		"wrapper.h": """/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v{version}) *
 *******************************************************************/
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
// p = pointer, P = void* on the stack
// f = float, d = double, D = long double, K = fake long double
// V = vaargs, E = current x86emu struct, e = ref to current x86emu struct
// 0 = constant 0, 1 = constant 1
// o = stdout
// C = unsigned byte c = char
// W = unsigned short w = short
// O = libc O_ flags bitfield
// S = _IO_2_1_stdXXX_ pointer (or FILE*)
// Q = ...
// N = ... automatically sending 1 arg
// M = ... automatically sending 2 args
// H = Huge 128bits value/struct

""",
		"fntypes.h": """/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v{version}) *
 *******************************************************************/
#ifndef __{filename}TYPES_H_
#define __{filename}TYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

"""
	}
	files_guard = {"wrapper.c": """""",
		"wrapper.h": """
int isSimpleWrapper(wrapper_t fun);

#endif // __WRAPPER_H_
""",
		"fntypes.h": """
#endif // __{filename}TYPES_H_
"""
	}
	
	# Rewrite the wrapper.c file:
	def generate_typedefs(arr, file):
		# i and u should only be 32 bits
		#         E            e             v       c         w          i          I          C          W           u           U           f        d         D              K         l           L            p        V        O          S        N      M      H                    P
		types = ["x64emu_t*", "x64emu_t**", "void", "int8_t", "int16_t", "int64_t", "int64_t", "uint8_t", "uint16_t", "uint64_t", "uint64_t", "float", "double", "long double", "double", "intptr_t", "uintptr_t", "void*", "void*", "int32_t", "void*", "...", "...", "unsigned __int128", "void*"]
		if len(values) != len(types):
			raise NotImplementedError("len(values) = {lenval} != len(types) = {lentypes}".format(lenval=len(values), lentypes=len(types)))
		
		for v in arr:
			file.write("typedef " + types[values.index(v[0])] + " (*" + v + "_t)"
				+ "(" + ', '.join(types[values.index(t)] for t in v[2:]) + ");\n")
	
	with open(os.path.join(root, "src", "wrapped", "generated", "wrapper.c"), 'w') as file:
		file.write(files_header["wrapper.c"].format(lbr="{", rbr="}", version=ver))
		
		# First part: typedefs
		generate_typedefs(gbl["()"], file)
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			generate_typedefs(gbl[k], file)
			file.write("#endif\n")
		
		file.write("\n")
		
		# Next part: function definitions
		
		# Helper variables
		# Name of the registers
		reg_arg = ["R_RDI", "R_RSI", "R_RDX", "R_RCX", "R_R8", "R_R9"]
		# vreg: value is in a general register
		#         E  e  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  N  M  H  P
		vreg   = [0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 2, 2, 0]
		# vxmm: value is in a XMM register
		#         E  e  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  N  M  H  P
		vxmm   = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
		# vother: value is elsewere
		#         E  e  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  N  M  H  P
		vother = [1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0]
		# vstack: value is on the stack (or out of register)
		#         E  e  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  N  M  H  P
		vstack = [0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 0, 1, 1, 1, 2, 2, 1]
		arg_s = [
			"",                                         # E
			"",                                         # e
			"",                                         # v
			"*(int8_t*)(R_RSP + {p}), ",                # c
			"*(int16_t*)(R_RSP + {p}), ",               # w
			"*(int64_t*)(R_RSP + {p}), ",               # i  should be int32_t
			"*(int64_t*)(R_RSP + {p}), ",               # I
			"*(uint8_t*)(R_RSP + {p}), ",               # C
			"*(uint16_t*)(R_RSP + {p}), ",              # W
			"*(uint64_t*)(R_RSP + {p}), ",              # u  should be uint32_t
			"*(uint64_t*)(R_RSP + {p}), ",              # U
			"*(float*)(R_RSP + {p}), ",                 # f
			"*(double*)(R_RSP + {p}), ",                # d
			"LD2localLD((void*)(R_RSP + {p})), ",       # D
			"FromLD((void*)(R_RSP + {p})), ",           # K
			"*(intptr_t*)(R_RSP + {p}), ",              # l
			"*(uintptr_t*)(R_RSP + {p}), ",             # L
			"*(void**)(R_RSP + {p}), ",                 # p
			"",                                         # V
			"of_convert(*(int32_t*)(R_RSP + {p})), ",   # O
			"io_convert(*(void**)(R_RSP + {p})), ",     # S
			"*(void**)(R_RSP + {p}), ",                 # N
			"*(void**)(R_RSP + {p}),*(void**)(R_RSP + {p} + 8), ", # M
			"*(unsigned __int128)(R_RSP + {p}), ",      # H
			"*(void**)(R_RSP + {p}), ",                 # P
		]
		arg_r = [
			"",                            # E
			"",                            # e
			"",                            # v
			"(int8_t){p}, ",               # c
			"(int16_t){p}, ",              # w
			"(int64_t){p}, ",              # i  should int32_t
			"(int64_t){p}, ",              # I
			"(uint8_t){p}, ",              # C
			"(uint16_t){p}, ",             # W
			"(uint64_t){p}, ",             # u  should uint32_t
			"(uint64_t){p}, ",             # U
			"",                            # f
			"",                            # d
			"",                            # D
			"",                            # K
			"(intptr_t){p}, ",             # l
			"(uintptr_t){p}, ",            # L
			"(void*){p}, ",                # p
			"",                            # V
			"of_convert((int32_t){p}), ",  # O
			"io_convert((void*){p}), ",    # S
			"(void*){p}, ",                # N
			"(void*){p}, ",                # M
			"\n#error Use pp instead\n",   # H
			"",                            # P
		]
		arg_x = [
			"",                      # E
			"",                      # e
			"",                      # v
			"",                      # c
			"",                      # w
			"",                      # i
			"",                      # I
			"",                      # C
			"",                      # W
			"",                      # u
			"",                      # U
			"emu->xmm[{p}].f[0], ",  # f
			"emu->xmm[{p}].d[0], ",  # d
			"",                      # D
			"",                      # K
			"",                      # l
			"",                      # L
			"",                      # p
			"",                      # V
			"",                      # O
			"",                      # S
			"",                      # N
			"",                      # M
			"",                      # H
			"",                      # P
		]
		arg_o = [
			"emu, ",                   # E
			"&emu, ",                  # e
			"",                        # v
			"",                        # c
			"",                        # w
			"",                        # i
			"",                        # I
			"",                        # C
			"",                        # W
			"",                        # u
			"",                        # U
			"",                        # f
			"",                        # d
			"",                        # D
			"",                        # K
			"",                        # l
			"",                        # L
			"",                        # p
			"(void*)(R_RSP + {p}), ",  # V
			"",                        # O
			"",                        # S
			"",                        # N
			"",                        # M
			"",                        # H
			"",                        # P
		]

		vals = [
			"\n#error Invalid return type: emulator\n",                # E
			"\n#error Invalid return type: &emulator\n",               # e
			"fn({0});",                                                # v
			"R_RAX=fn({0});",                                          # c
			"R_RAX=fn({0});",                                          # w
			"R_RAX=(int64_t)fn({0});",                                 # i  should be int32_t
			"R_RAX=(int64_t)fn({0});",                                 # I
			"R_RAX=(unsigned char)fn({0});",                           # C
			"R_RAX=(unsigned short)fn({0});",                          # W
			"R_RAX=(uint64_t)fn({0});",                                # u  should be uint32_t
			"R_RAX=fn({0});",                                          # U
			"emu->xmm[0].f[0]=fn({0});",                               # f
			"emu->xmm[0].d[0]=fn({0});",                               # d
			"long double ld=fn({0}); fpu_do_push(emu); ST0val = ld;",  # D
			"double db=fn({0}); fpu_do_push(emu); ST0val = db;",       # K
			"R_RAX=(intptr_t)fn({0});",                                # l
			"R_RAX=(uintptr_t)fn({0});",                               # L
			"R_RAX=(uintptr_t)fn({0});",                               # p
			"\n#error Invalid return type: va_list\n",                 # V
			"\n#error Invalid return type: at_flags\n",                # O
			"\n#error Invalid return type: _io_file*\n",               # S
			"\n#error Invalid return type: ... with 1 arg\n",          # N
			"\n#error Invalid return type: ... with 2 args\n",         # M
			"unsigned __int128 u128 = fn({0}); R_RAX=(u128&0xFFFFFFFFFFFFFFFFL); R_RDX=(u128>>64)&0xFFFFFFFFFFFFFFFFL;", # H
			"\n#error Invalid return type: pointer in the stack\n",    # P
		]
		# Asserts
		if len(values) != len(vstack):
			raise NotImplementedError("len(values) = {lenval} != len(vstack) = {lenvstack}".format(lenval=len(values), lenvstack=len(vstack)))
		if len(values) != len(vreg):
			raise NotImplementedError("len(values) = {lenval} != len(vreg) = {lenvreg}".format(lenval=len(values), lenvreg=len(vreg)))
		if len(values) != len(vxmm):
			raise NotImplementedError("len(values) = {lenval} != len(vxmm) = {lenvxmm}".format(lenval=len(values), lenvvxmm=len(vxmm)))
		if len(values) != len(vother):
			raise NotImplementedError("len(values) = {lenval} != len(vother) = {lenvother}".format(lenval=len(values), lenvother=len(vother)))
		if len(values) != len(arg_s):
			raise NotImplementedError("len(values) = {lenval} != len(arg_s) = {lenargs}".format(lenval=len(values), lenargs=len(arg_s)))
		if len(values) != len(arg_r):
			raise NotImplementedError("len(values) = {lenval} != len(arg_r) = {lenargr}".format(lenval=len(values), lenargr=len(arg_r)))
		if len(values) != len(arg_x):
			raise NotImplementedError("len(values) = {lenval} != len(arg_x) = {lenargx}".format(lenval=len(values), lenargx=len(arg_x)))
		if len(values) != len(arg_o):
			raise NotImplementedError("len(values) = {lenval} != len(arg_o) = {lenargo}".format(lenval=len(values), lenargo=len(arg_o)))
		if len(values) != len(vals):
			raise NotImplementedError("len(values) = {lenval} != len(vals) = {lenvals}".format(lenval=len(values), lenvals=len(vals)))
		
		# Helper functions to write the function definitions
		def function_args(args, d=8, r=0, x=0):
			# args: string of argument types
			# d: delta (in the stack)
			# r: general register no
			# x: XMM register no
			if len(args) == 0:
				return ""
			
			# Redirections
			if args[0] == "0":
				return "0, " + function_args(args[1:], d, r, x)
			elif args[0] == "1":
				return "1, " + function_args(args[1:], d, r, x)
			
			idx = values.index(args[0])
			if (r < 6) and (vreg[idx] > 0):
				# Value is in a general register (and there is still one available)
				if (vreg[idx] == 2) and (r == 6):
					return arg_r[idx-1].format(p=reg_arg[r]) + arg_s[idx-1].format(p=d) + function_args(args[1:], d + vother[idx-1]*8, r+1, x)
				elif (vreg[idx] == 2) and (r < 6):
					return arg_r[idx].format(p=reg_arg[r]) + arg_r[idx].format(p=reg_arg[r+1]) + function_args(args[1:], d, r+2, x)
				else:
					return arg_r[idx].format(p=reg_arg[r]) + function_args(args[1:], d, r+1, x)
			elif (x < 8) and (vxmm[idx] > 0):
				# Value is in an XMM register (and there is still one available)
				return arg_x[idx].format(p=x) + function_args(args[1:], d, r, x+1)
			elif vstack[idx] > 0:
				# Value is in the stack
				return arg_s[idx].format(p=d) + function_args(args[1:], d+8*vstack[idx], r, x)
			else:
				# Value is somewhere else
				return arg_o[idx].format(p=d) + function_args(args[1:], d, r, x)
		
		def function_writer(f, N, W):
			# Write to f the function type N (real type W)
			# rettype is a single character, args is the string of argument types
			# (those could actually be deduced from N)
			
			f.write("void {0}(x64emu_t *emu, uintptr_t fcn) {2} {1} fn = ({1})fcn; ".format(N, W, "{"))
			# Generic function
			f.write(vals[values.index(N[0])].format(function_args(N[2:])[:-2]) + " }\n")
		
		for v in gbl["()"]:
			if v == "vFv":
				# Suppress all warnings...
				file.write("void vFv(x64emu_t *emu, uintptr_t fcn) { vFv_t fn = (vFv_t)fcn; fn(); (void)emu; }\n")
			else:
				function_writer(file, v, v + "_t")
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			for v in gbl[k]:
				function_writer(file, v, v + "_t")
			file.write("#endif\n")
		file.write("\n")
		for v in redirects["()"]:
			function_writer(file, v[0], v[1] + "_t")
		for k in redirects_idxs:
			file.write("\n#if " + k + "\n")
			for v in redirects[k]:
				function_writer(file, v[0], v[1] + "_t")
			file.write("#endif\n")
		
		# Write the isSimpleWrapper function
		file.write("\nint isSimpleWrapper(wrapper_t fun) {\n")
		for k in simple_idxs:
			if k != "()":
				file.write("#if " + k + "\n")
			for v in simple_wraps[k]:
				file.write("\tif (fun == &" + v + ") return 1;\n")
			if k != "()":
				file.write("#endif\n")
		file.write("\treturn 0;\n}\n")
		
		file.write(files_guard["wrapper.c"].format(lbr="{", rbr="}", version=ver))
	
	# Rewrite the wrapper.h file:
	with open(os.path.join(root, "src", "wrapped", "generated", "wrapper.h"), 'w') as file:
		file.write(files_header["wrapper.h"].format(lbr="{", rbr="}", version=ver))
		# Normal function types
		for v in gbl["()"]:
			file.write("void " + v + "(x64emu_t *emu, uintptr_t fnc);\n")
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			for v in gbl[k]:
				file.write("void " + v + "(x64emu_t *emu, uintptr_t fnc);\n")
			file.write("#endif\n")
		file.write("\n")
		# Redirects
		for v in redirects["()"]:
			file.write("void " + v[0] + "(x64emu_t *emu, uintptr_t fnc);\n")
		for k in redirects_idxs:
			file.write("\n#if " + k + "\n")
			for v in redirects[k]:
				file.write("void " + v[0] + "(x64emu_t *emu, uintptr_t fnc);\n")
			file.write("#endif\n")
		file.write(files_guard["wrapper.h"].format(lbr="{", rbr="}", version=ver))
	
	# Rewrite the *types.h files:
	for fn in mytypedefs:
		with open(os.path.join(root, "src", "wrapped", "generated", fn + "types.h"), 'w') as file:
			file.write(files_header["fntypes.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
			generate_typedefs(mytypedefs_vals[fn], file)
			file.write("\n#define SUPER() ADDED_FUNCTIONS()")
			for v in mytypedefs_vals[fn]:
				for f in mytypedefs[fn][v]:
					file.write(" \\\n\tGO({0}, {1}_t)".format(f, v))
			file.write("\n")
			file.write(files_guard["fntypes.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
	
	# Save the string for the next iteration, writing was successful
	with open(os.path.join(root, "src", "wrapped", "generated", "functions_list.txt"), 'w') as file:
		file.write(functions_list)
	
	return 0

if __name__ == '__main__':
	limit = []
	for i, v in enumerate(sys.argv):
		if v == "--":
			limit.append(i)
	if main(sys.argv[1], sys.argv[2:limit[0]], sys.argv[limit[0]+1:], "1.3.0.12") != 0:
		exit(2)
	exit(0)
