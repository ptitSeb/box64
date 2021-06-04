#!/usr/bin/env python3

# TODO: same as for box86, flac can't be auto-generated yet

try:
	# Python 3.5.2+ (NewType)
	from typing import Union, List, Sequence, Dict, Tuple, NewType, TypeVar
except ImportError:
	#print("Your Python version does not have the typing module, fallback to empty 'types'")
	# Dummies
	class GTDummy:
		def __getitem__(self, t):
			return self
	Union = GTDummy() # type: ignore
	List = GTDummy() # type: ignore
	Sequence = GTDummy() # type: ignore
	Dict = GTDummy() # type: ignore
	Tuple = GTDummy() # type: ignore
	def NewType(T, b): return b # type: ignore
	def TypeVar(T): return object # type: ignore
try:
	# Python 3.8+
	from typing import final
	import typing
except ImportError:
	#print("Your Python version does not have all typing utilities, fallback to dummy ones")
	def _overload_dummy(*args, **kwds):
		raise NotImplementedError(
				"You should not call an overloaded function. "
				"A series of @overload-decorated functions "
				"outside a stub module should always be followed "
				"by an implementation that is not @overload-ed.")
	def overload(fun): return _overload_dummy # type: ignore
	class Typing:
		pass
	typing = Typing() # type: ignore
	typing.overload = overload # type: ignore
	final = lambda fun: fun # type: ignore

import os
import sys

class FunctionType(str):
	values: List[str] = ['E', 'e', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'V', 'O', 'S', 'N', 'M', 'H', 'P']
	
	@staticmethod
	def validate(s: str, post: str) -> bool:
		if len(s) < 3:
			raise NotImplementedError("Type {0} too short{1}".format(s, post))
		if "E" in s:
			if ("E" in s[:2]) or ("E" in s[3:]):
				raise NotImplementedError(
					"emu64_t* not as the first parameter{0}".format(post))
			if len(s) < 4:
				raise NotImplementedError("Type {0} too short{1}".format(s, post))
		
		if s[1] not in ["F"]:
			raise NotImplementedError("Bad middle letter {0}{1}".format(s[1], post))
		
		return all(c in FunctionType.values for c in s[2:]) and (('v' not in s[2:]) or (len(s) == 3))
	
	def splitchar(self) -> List[int]:
		"""
		splitchar -- Sorting key function for function signatures
		
		The longest strings are at the end, and for identical length, the string
		are sorted using a pseudo-lexicographic sort, where characters have a value
		of `values.index`.
		"""
		try:
			ret = [len(self), FunctionType.values.index(self[0])]
			for c in self[2:]:
				ret.append(FunctionType.values.index(c))
			return ret
		except ValueError as e:
			raise ValueError("Value is " + self) from e
	
	def __getitem__(self, i: Union[int, slice]) -> 'FunctionType':
		return FunctionType(super().__getitem__(i))

assert(all(c not in FunctionType.values[:i] for i, c in enumerate(FunctionType.values)))

RedirectType = NewType('RedirectType', FunctionType)
DefineType = NewType('DefineType', str)

T = TypeVar('T')
U = TypeVar('U')

Filename = str
ClausesStr = str

@final
class Define:
	name: DefineType
	inverted_: bool
	
	defines: List[DefineType] = []
	
	def __init__(self, name: str, inverted_: bool):
		# All values of "name" are included in defines (throw otherwise)
		if DefineType(name) not in self.defines:
			raise KeyError(name)
		self.name = DefineType(name)
		self.inverted_ = inverted_
	def copy(self) -> "Define":
		return Define(self.name, self.inverted_)
	
	def value(self) -> int:
		return self.defines.index(self.name)*2 + (1 if self.inverted_ else 0)
	
	def invert(self) -> None:
		"""
		invert -- Transform a `defined()` into a `!defined()` and vice-versa, in place.
		"""
		self.inverted_ = not self.inverted_
	def inverted(self) -> "Define":
		"""
		inverted -- Transform a `defined()` into a `!defined()` and vice-versa, out-of-place.
		"""
		return Define(self.name, not self.inverted_)
	
	def __str__(self) -> str:
		if self.inverted_:
			return "!defined(" + self.name + ")"
		else:
			return "defined(" + self.name + ")"
@final
class Clause:
	defines: List[Define]
	
	def __init__(self, defines: Union[List[Define], str] = []):
		if isinstance(defines, str):
			if defines == "":
				self.defines = []
			else:
				self.defines = list(
					map(
						lambda x:
							Define(x[9:-1] if x[0] == '!' else x[8:-1], x[0] == '!')
						, defines.split(" && ")
					)
				)
		else:
			self.defines = [d.copy() for d in defines]
	def copy(self) -> "Clause":
		return Clause(self.defines)
	
	def append(self, define: Define) -> None:
		self.defines.append(define)
	def invert_last(self) -> None:
		self.defines[-1].invert()
	def pop_last(self) -> None:
		if len(self.defines) > 0: self.defines.pop()
	
	def __str__(self) -> str:
		return " && ".join(map(str, self.defines))
@final
class Clauses:
	"""
	Represent a list of clauses, aka a list of or-ed together and-ed "defined"
	conditions
	"""
	definess: List[Clause]
	
	def __init__(self, definess: Union[List[Clause], str] = []):
		if isinstance(definess, str):
			if definess == "()":
				self.definess = []
			elif ") || (" in definess:
				self.definess = list(map(Clause, definess[1:-1].split(") || (")))
			else:
				self.definess = [Clause(definess)]
		else:
			self.definess = definess[:]
	def copy(self) -> "Clauses":
		return Clauses(self.definess[:])
	
	def add(self, defines: Clause) -> None:
		self.definess.append(defines)
	
	def splitdef(self) -> List[int]:
		"""
		splitdef -- Sorting key function for #ifdefs
		
		All #if defined(...) are sorted first by the number of clauses, then by the
		number of '&&' in each clause and then by the "key" of the tested names
		(left to right, inverted placed after non-inverted).
		"""
		
		ret = [len(self.definess)]
		for cunj in self.definess:
			ret.append(len(cunj.defines))
		for cunj in self.definess:
			for d in cunj.defines:
				ret.append(d.value())
		ret.append(0)
		return ret
	
	def __str__(self) -> ClausesStr:
		if len(self.definess) == 1:
			return str(self.definess[0])
		else:
			return "(" + ") || (".join(map(str, self.definess)) + ")"

def readFiles(files: Sequence[Filename]) -> \
		Tuple[Dict[ClausesStr, List[FunctionType]],
		      Dict[ClausesStr, Dict[RedirectType, FunctionType]],
		      Dict[Filename,   Dict[RedirectType, List[str]]]]:
	"""
	readFiles
	
	This function is the one that parses the files.
	It returns the jumbled (gbl, redirects, mytypedefs) tuple.
	"""
	
	# Initialize variables: gbl for all values, redirects for redirections
	# mytypedefs is a list of all "*FE*" types per filename
	gbl       : Dict[ClausesStr, List[FunctionType]]               = {}
	redirects : Dict[ClausesStr, Dict[RedirectType, FunctionType]] = {}
	mytypedefs: Dict[Filename,   Dict[RedirectType, List[str]]]    = {}
	
	halt_required = False # Is there a GO(*, .FE*)?
	
	# First read the files inside the headers
	for filepath in files:
		filename: Filename = filepath.split("/")[-1]
		dependants: Clause = Clause()
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
							dependants.pop_last()
						elif preproc_cmd.startswith("ifdef"):
							dependants.append(Define(preproc_cmd[5:].strip(), False))
						elif preproc_cmd.startswith("ifndef"):
							dependants.append(Define(preproc_cmd[6:].strip(), True))
						elif preproc_cmd.startswith("else"):
							dependants.invert_last()
						else:
							raise NotImplementedError("Unknown preprocessor directive: {0} ({1}:{2})".format(
								preproc_cmd.split(" ")[0], filename, line[:-1]
							))
					except KeyError as k:
						raise NotImplementedError("Unknown key: {0} ({1}:{2})".format(
							k.args[0], filename, line[:-1]
						)) from k
				# If the line is a `GO...' line (GO/GOM/GO2/...)...
				elif ln.startswith("GO"):
					# ... then look at the second parameter of the line
					try:
						gotype = ln.split("(")[0].strip()
						funname = ln.split(",")[0].split("(")[1].strip()
						ln = ln.split(",")[1].split(")")[0].strip()
					except IndexError as e:
						raise NotImplementedError("Invalid GO command: {0}:{1}".format(
							filename, line[:-1]
						))
					
					if not FunctionType.validate(ln, " ({0}:{1})".format(filename, line[:-1])):
						old = RedirectType(FunctionType(ln))
						# This needs more work
						acceptables = ['v', '0', '1'] + FunctionType.values
						if any(c not in acceptables for c in ln[2:]):
							raise NotImplementedError("{0} ({1}:{2})".format(ln[2:], filename, line[:-1]))
						# Ok, this is acceptable: there is 0, 1 and/or void
						ln = ln[:2] + (ln[2:]
							.replace("v", "")   # void   -> nothing
							.replace("0", "p")  # 0      -> pointer
							.replace("1", "i")) # 1      -> integer
						assert(len(ln) >= 3)
						redirects.setdefault(str(dependants), {})
						redirects[str(dependants)][old] = FunctionType(ln)
					# Simply append the function type if it's not yet existing
					gbl.setdefault(str(dependants), [])
					if ln not in gbl[str(dependants)]:
						gbl[str(dependants)].append(FunctionType(ln))
					
					if ln[2] == "E":
						if (gotype != "GOM"):
							if (gotype != "GO2") or not (line.split(',')[2].split(')')[0].strip().startswith('my_')):
								print("\033[91mThis is probably not what you meant!\033[m ({0}:{1})".format(filename, line[:-1]))
								halt_required = True
						# filename isn't stored with the '_private.h' part
						if len(ln) > 3:
							funtype = RedirectType(FunctionType(ln[:2] + ln[3:]))
						else:
							funtype = RedirectType(FunctionType(ln[:2] + "v"))
						mytypedefs.setdefault(filename[:-10], {})
						mytypedefs[filename[:-10]].setdefault(funtype, [])
						mytypedefs[filename[:-10]][funtype].append(funname)
					# OK on box64
					# elif gotype == "GOM":
					# 	print("\033[94mAre you sure of this?\033[m ({0}:{1})".format(filename, line[:-1]))
					# 	halt_required = True
	
	if halt_required:
		raise ValueError("Fix all previous errors before proceeding")
	
	if ("" not in gbl) or ("" not in redirects):
		print("\033[1;31mThere is suspiciously not many types...\033[m")
		print("Check the CMakeLists.txt file. If you are SURE there is nothing wrong"
		      " (as a random example, `set()` resets the variable...), then comment out the following exit.")
		print("(Also, the program WILL crash later if you proceed.)")
		sys.exit(2) # Check what you did, not proceeding
	
	return gbl, redirects, mytypedefs

COrderedDict = Tuple[Dict[T, U], List[T]]
def sortArrays(
	gbl_tmp   : Dict[str,      List[FunctionType]],
	red_tmp   : Dict[str,      Dict[RedirectType, FunctionType]],
	mytypedefs: Dict[Filename, Dict[RedirectType, List[str]]]) -> \
		Tuple[
			COrderedDict[ClausesStr, List[FunctionType]],
			COrderedDict[ClausesStr, List[Tuple[RedirectType, FunctionType]]],
			Dict[Filename, COrderedDict[RedirectType, List[str]]]
		]:
	# Now, take all function types, and make a new table gbl_vals
	# This table contains all #if conditions for when a function type needs to
	# be generated. There is also a filter to avoid duplicate/opposite clauses.
	gbl_vals: Dict[FunctionType, Clauses] = {}
	for k1 in gbl_tmp:
		ks = Clause(k1)
		for v in gbl_tmp[k1]:
			if k1 == "":
				# Unconditionally define v
				gbl_vals[v] = Clauses()
				
			elif v in gbl_vals:
				if gbl_vals[v].definess == []:
					# v already unconditionally defined
					continue
				
				for other_key in gbl_vals[v].definess:
					for other_key_val in other_key.defines:
						if other_key_val not in ks.defines:
							# Not a duplicate or more specific case
							# (could be a less specific one though)
							break
					else:
						break
				else:
					gbl_vals[v].add(ks)
				
			else:
				gbl_vals[v] = Clauses([Clause(k1)])
	
	for v in gbl_vals:
		strdefines = list(map(str, gbl_vals[v].definess))
		for k2 in gbl_vals[v].definess:
			for i in range(len(k2.defines)):
				if " && ".join(map(str, k2.defines[:i] + [k2.defines[i].inverted()] + k2.defines[i+1:])) in strdefines:
					# Opposite clauses detected
					gbl_vals[v] = Clauses()
					break
			else:
				continue
			break
	
	# Now create a new gbl and gbl_idxs
	# gbl will contain the final version of gbl (without duplicates, based on
	# gbl_vals)
	# gbl_idxs will contain all #if clauses
	gbl: Dict[ClausesStr, List[FunctionType]] = {}
	gbl_idxs: List[ClausesStr] = []
	for k1 in gbl_vals:
		clauses = gbl_vals[k1]
		key = str(clauses)
		gbl.setdefault(key, [])
		gbl[key].append(k1)
		if (key not in gbl_idxs) and (clauses.definess != []):
			gbl_idxs.append(key)
	# Sort the #if clauses as defined in `splitdef`
	gbl_idxs.sort(key=lambda c: Clauses(c).splitdef())
	
	# This map will contain all additional function types that are "redirected"
	# to an already defined type (with some remapping).
	redirects_vals: Dict[Tuple[RedirectType, FunctionType], Clauses] = {}
	for k1 in red_tmp:
		ks = Clause(k1)
		for v in red_tmp[k1]:
			if k1 == "":
				# Unconditionally define v
				redirects_vals[(v, red_tmp[k1][v])] = Clauses()
			
			elif (v, red_tmp[k1][v]) in redirects_vals:
				if redirects_vals[(v, red_tmp[k1][v])].definess == []:
					# v already unconditionally defined
					continue
				
				for other_key in redirects_vals[(v, red_tmp[k1][v])].definess:
					for other_key_val in other_key.defines:
						if other_key_val not in ks.defines:
							# Not a duplicate or more specific case
							# (could be a less specific one though)
							break
					else:
						break
				else:
					redirects_vals[(v, red_tmp[k1][v])].add(ks)
			
			else:
				redirects_vals[(v, red_tmp[k1][v])] = Clauses([Clause(k1)])
	# Also does the same trick as before (also helps keep the order
	# in the file deterministic)
	redirects: Dict[ClausesStr, List[Tuple[RedirectType, FunctionType]]] = {}
	redirects_idxs: List[ClausesStr] = []
	for k1, v in redirects_vals:
		clauses = redirects_vals[(k1, v)]
		key = str(clauses)
		redirects.setdefault(key, [])
		redirects[key].append((k1, v))
		if (key not in redirects_idxs) and (clauses.definess != []):
			redirects_idxs.append(key)
	redirects_idxs.sort(key=lambda c: Clauses(c).splitdef())
	
	# Sort the function types as defined in `splitchar`
	for k3 in gbl:
		gbl[k3].sort(key=FunctionType.splitchar)
	
	FunctionType.values = FunctionType.values + ['0', '1']
	for k3 in redirects:
		redirects[k3].sort(key=lambda v: v[0].splitchar() + v[1].splitchar())
	FunctionType.values = FunctionType.values[:-2]
	
	mytypedefs_vals: Dict[Filename, List[RedirectType]] = dict((fn, sorted(mytypedefs[fn].keys(), key=FunctionType.splitchar)) for fn in mytypedefs)
	for fn in mytypedefs:
		for v in mytypedefs_vals[fn]:
			mytypedefs[fn][v].sort()
	
	return (gbl, gbl_idxs), (redirects, redirects_idxs), \
		dict((fn, (mytypedefs[fn], mytypedefs_vals[fn])) for fn in mytypedefs)
	
def main(root: str, files: Sequence[Filename], ver: str):
	"""
	main -- The main function
	
	root: the root path (where the CMakeLists.txt is located)
	files: a list of files to parse (wrapped*.h)
	ver: version number
	"""
	
	# gbl_tmp:
	#  "defined() && ..." -> [vFv, ...]
	# red_tmp:
	#  "defined() && ..." -> [vFEv -> vFv, ...]
	# tdf_tmp:
	#  "filename" -> [vFEv -> fopen, ...]
	gbl_tmp: Dict[ClausesStr, List[FunctionType]]
	red_tmp: Dict[ClausesStr, Dict[RedirectType, FunctionType]]
	tdf_tmp: Dict[Filename,   Dict[RedirectType, List[str]]]
	
	gbl_tmp, red_tmp, tdf_tmp = readFiles(files)
	
	gbls      : COrderedDict[ClausesStr, List[FunctionType]]
	redirects_: COrderedDict[ClausesStr, List[Tuple[RedirectType, FunctionType]]]
	mytypedefs: Dict[Filename, COrderedDict[RedirectType, List[str]]]
	
	gbls, redirects_, mytypedefs = \
		sortArrays(gbl_tmp, red_tmp, tdf_tmp)
	
	gbl, gbl_idxs = gbls
	redirects, redirects_idxs = redirects_
	
	# Check if there was any new functions compared to last run
	functions_list: str = ""
	for k in [str(Clauses())] + gbl_idxs:
		for v in gbl[k]:
			functions_list = functions_list + "#" + k + " " + v + "\n"
	for k in [str(Clauses())] + redirects_idxs:
		for vr, vf in redirects[k]:
			functions_list = functions_list + "#" + k + " " + vr + " -> " + vf + "\n"
	for filename in sorted(mytypedefs.keys()):
		functions_list = functions_list + filename + ":\n"
		for vr in mytypedefs[filename][1]:
			functions_list = functions_list + "- " + vr + ":\n"
			for fn in mytypedefs[filename][0][vr]:
				functions_list = functions_list + "  - " + fn + "\n"
	
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
	simple_wraps: Dict[ClausesStr, List[FunctionType]] = {}
	
	# H could be allowed maybe?
	allowed_simply: str = "v"
	allowed_regs  : str = "cCwWiuIUlLp"
	allowed_fpr   : str = "fd"
	
	# Sanity checks
	forbidden_simple: str = "EeDKVOSNMHP"
	assert(len(allowed_simply) + len(allowed_regs) + len(allowed_fpr) + len(forbidden_simple) == len(FunctionType.values))
	assert(all(c not in allowed_regs for c in allowed_simply))
	assert(all(c not in allowed_simply + allowed_regs for c in allowed_fpr))
	assert(all(c not in allowed_simply + allowed_regs + allowed_fpr for c in forbidden_simple))
	assert(all(c in allowed_simply + allowed_regs + allowed_fpr + forbidden_simple for c in FunctionType.values))
	
	# Only search on real wrappers
	for k in [str(Clauses())] + gbl_idxs:
		for v in gbl[k]:
			regs_count: int = 0
			fpr_count : int = 0
			
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
	simple_idxs.sort(key=lambda x: Clauses(x).splitdef())
	
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
	def generate_typedefs(arr: Sequence[FunctionType], file) -> None:
		# i and u should only be 32 bits
		#         E            e             v       c         w          i          I          C          W           u           U           f        d         D              K         l           L            p        V        O          S        N      M      H                    P
		types = ["x64emu_t*", "x64emu_t**", "void", "int8_t", "int16_t", "int64_t", "int64_t", "uint8_t", "uint16_t", "uint64_t", "uint64_t", "float", "double", "long double", "double", "intptr_t", "uintptr_t", "void*", "void*", "int32_t", "void*", "...", "...", "unsigned __int128", "void*"]
		if len(FunctionType.values) != len(types):
			raise NotImplementedError("len(values) = {lenval} != len(types) = {lentypes}".format(lenval=len(FunctionType.values), lentypes=len(types)))
		
		for v in arr:
			file.write("typedef " + types[FunctionType.values.index(v[0])] + " (*" + v + "_t)"
				+ "(" + ', '.join(types[FunctionType.values.index(t)] for t in v[2:]) + ");\n")
	
	with open(os.path.join(root, "src", "wrapped", "generated", "wrapper.c"), 'w') as file:
		file.write(files_header["wrapper.c"].format(lbr="{", rbr="}", version=ver))
		
		# First part: typedefs
		generate_typedefs(gbl[str(Clauses())], file)
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			generate_typedefs(gbl[k], file)
			file.write("#endif\n")
		
		file.write("\n")
		
		# Next part: function definitions
		
		# Helper variables
		# Return type template
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

		# Asserts
		if len(FunctionType.values) != len(vstack):
			raise NotImplementedError("len(values) = {lenval} != len(vstack) = {lenvstack}".format(lenval=len(FunctionType.values), lenvstack=len(vstack)))
		if len(FunctionType.values) != len(vreg):
			raise NotImplementedError("len(values) = {lenval} != len(vreg) = {lenvreg}".format(lenval=len(FunctionType.values), lenvreg=len(vreg)))
		if len(FunctionType.values) != len(vxmm):
			raise NotImplementedError("len(values) = {lenval} != len(vxmm) = {lenvxmm}".format(lenval=len(FunctionType.values), lenvxmm=len(vxmm)))
		if len(FunctionType.values) != len(vother):
			raise NotImplementedError("len(values) = {lenval} != len(vother) = {lenvother}".format(lenval=len(FunctionType.values), lenvother=len(vother)))
		if len(FunctionType.values) != len(arg_s):
			raise NotImplementedError("len(values) = {lenval} != len(arg_s) = {lenargs}".format(lenval=len(FunctionType.values), lenargs=len(arg_s)))
		if len(FunctionType.values) != len(arg_r):
			raise NotImplementedError("len(values) = {lenval} != len(arg_r) = {lenargr}".format(lenval=len(FunctionType.values), lenargr=len(arg_r)))
		if len(FunctionType.values) != len(arg_x):
			raise NotImplementedError("len(values) = {lenval} != len(arg_x) = {lenargx}".format(lenval=len(FunctionType.values), lenargx=len(arg_x)))
		if len(FunctionType.values) != len(arg_o):
			raise NotImplementedError("len(values) = {lenval} != len(arg_o) = {lenargo}".format(lenval=len(FunctionType.values), lenargo=len(arg_o)))
		if len(FunctionType.values) != len(vals):
			raise NotImplementedError("len(values) = {lenval} != len(vals) = {lenvals}".format(lenval=len(FunctionType.values), lenvals=len(vals)))
		
		# Helper functions to write the function definitions
		def function_args(args: FunctionType, d: int = 8, r: int = 0, x: int = 0) -> str:
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
			
			idx = FunctionType.values.index(args[0])
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
		
		def function_writer(f, N: FunctionType, W: str) -> None:
			# Write to f the function type N (real type W)
			# rettype is a single character, args is the string of argument types
			# (those could actually be deduced from N)
			
			f.write("void {0}(x64emu_t *emu, uintptr_t fcn) {2} {1} fn = ({1})fcn; ".format(N, W, "{"))
			# Generic function
			f.write(vals[FunctionType.values.index(N[0])].format(function_args(N[2:])[:-2]) + " }\n")
		
		for v in gbl[str(Clauses())]:
			if v == FunctionType("vFv"):
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
		for vr, vf in redirects[str(Clauses())]:
			function_writer(file, vr, vf + "_t")
		for k in redirects_idxs:
			file.write("\n#if " + k + "\n")
			for vr, vf in redirects[k]:
				function_writer(file, vr, vf + "_t")
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
		for v in gbl[str(Clauses())]:
			file.write("void " + v + "(x64emu_t *emu, uintptr_t fnc);\n")
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			for v in gbl[k]:
				file.write("void " + v + "(x64emu_t *emu, uintptr_t fnc);\n")
			file.write("#endif\n")
		file.write("\n")
		# Redirects
		for vr, _ in redirects[str(Clauses())]:
			file.write("void " + vr + "(x64emu_t *emu, uintptr_t fnc);\n")
		for k in redirects_idxs:
			file.write("\n#if " + k + "\n")
			for vr, _ in redirects[k]:
				file.write("void " + vr + "(x64emu_t *emu, uintptr_t fnc);\n")
			file.write("#endif\n")
		file.write(files_guard["wrapper.h"].format(lbr="{", rbr="}", version=ver))
	
	# Rewrite the *types.h files:
	for fn in mytypedefs:
		with open(os.path.join(root, "src", "wrapped", "generated", fn + "types.h"), 'w') as file:
			file.write(files_header["fntypes.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
			generate_typedefs(mytypedefs[fn][1], file)
			file.write("\n#define SUPER() ADDED_FUNCTIONS()")
			for v in mytypedefs[fn][1]:
				for f in mytypedefs[fn][0][v]:
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
	Define.defines = list(map(DefineType, sys.argv[2:limit[0]]))
	if main(sys.argv[1], sys.argv[limit[0]+1:], "2.0.1.14") != 0:
		exit(2)
	exit(0)
