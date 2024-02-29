#!/usr/bin/env python3

import os
import sys

try:
	assert(sys.version_info.major == 3)
	if sys.version_info.minor >= 9:
		# Python 3.9+
		from typing import Generic, NewType, Optional, TypeVar, Union, final
		from collections.abc import Iterable
		Dict = dict
		List = list
		Tuple = tuple
	elif sys.version_info.minor >= 8:
		# Python [3.8, 3.9)
		from typing import Dict, List, Tuple, Generic, Iterable, NewType, Optional, TypeVar, Union, final
	elif (sys.version_info.minor >= 5) and (sys.version_info.micro >= 2):
		# Python [3.5.2, 3.8)
		from typing import Dict, List, Tuple, Generic, Iterable, NewType, Optional, TypeVar, Union
		final = lambda fun: fun # type: ignore
	elif sys.version_info.minor >= 5:
		# Python [3.5, 3.5.2)
		from typing import Dict, List, Tuple, Generic, Iterable, Optional, TypeVar, Union
		class GTDummy:
			def __getitem__(self, _): return self
		final = lambda fun: fun # type: ignore
		def NewType(_, b): return b # type: ignore
	else:
		# Python < 3.5
		#print("Your Python version does not have the typing module, fallback to empty 'types'")
		# Dummies
		class GTDummy:
			def __getitem__(self, _):
				return self
		Dict = GTDummy() # type: ignore
		List = GTDummy() # type: ignore
		Generic = GTDummy() # type: ignore
		Iterable = GTDummy() # type: ignore
		Optional = GTDummy() # type: ignore
		def NewType(_, b): return b # type: ignore
		Tuple = GTDummy() # type: ignore
		def TypeVar(T): return object # type: ignore
		Union = GTDummy() # type: ignore
except ImportError:
	print("It seems your Python version is quite broken...")
	assert(False)

"""
Generates all files in src/wrapped/generated
===

TL;DR: Automagically creates type definitions (/.F.+/ functions/typedefs...).
       All '//%' in the headers are used by the script.

Reads each lines of each "_private.h" headers.
For each of them:
- If if starts with a #ifdef, #else, #ifndef, #endif, it memorizes which definition is required
- If it starts with a "GO", it will do multiple things:
  - It memorizes the type used by the function (second macro argument)
  - It memorizes the type it is mapped to, if needed (eg, iFEvp is mapped to iFEp: the first "real" argument is dropped)
  - It checks if the type given (both original and mapped to) are valid
  - If the signature contains a 'E' but it is not a "GOM" command, it will throw an error
- If the line starts with a '//%S', it will memorize a structure declaration.
  The structure of it is: "//%S <letter> <structure name> <signature equivalent>"
  NOTE: Those structure letters are "fake types" that are accepted in the macros.

After sorting the data, it generates:

wrapper.c
---------
(Private) type definitions (/.F.+_t/)
Function definitions (/.F.+/ functions, that actually execute the function given as argument)
isSimpleWrapper definition

wrapper.h
---------
Generic "wrapper_t" type definition
Function declarations (/.F.+/ functions)

*types.h
--------
Local types definition, for the original signatures
The SUPER() macro definition, used to generate and initialize the `*_my_t` library structure
(TODO: also automate this declaration/definition? It would require more metadata,
 and may break sometime in the future due to the system changing...)

*defs.h
-------
Local `#define`s, for signature mapping

*undefs.h
---------
Local `#undefine`s, for signature mapping


Example:
========
In wrappedtest_private.h:
   ----------------------
//%S X TestLibStructure ppu

GO(superfunction, pFX)
GOM(superFunction2, pFpX)

Generated files:
wrapper.c: [snippet]
----------
typedef void *(*pFppu_t)(void*, void*, uint32_t);
typedef void *(*pFpppu_t)(void*, void*, void*, uint32_t);

void pFppu(x64emu_t *emu, uintptr_t fcn) { pFppu_t *fn = (pFppu_t)fn; R_RAX=...; }
void pFpppu(x64emu_t *emu, uintptr_t fcn) { pFpppu_t *fn = (pFpppu_t)fn; R_RAX=...; }

int isSimpleWrapper(wrapper_t fun) {
	if (fcn == pFppu) return 1;
	if (fcn == pFpppu) return 1;
	return 0;
}

wrapper.h: [snippet]
----------
void pFppu(x64emu_t *emu, uintptr_t fcn);
void pFpppu(x64emu_t *emu, uintptr_t fcn);

int isSimpleWrapper(wrapper_t fun);

wrappedtesttypes.h:
-------------------
typedef void *(*pFpX_t)(void*, TestLibStructure);

#define SUPER() \\
	GO(superFunction2, pFpX)

wrappedtestdefs.h:
------------------
#define pFX pFppu
#define pFpX pFpppu

wrappedtestundefs.h:
--------------------
#undef pFX
#undef pFpX
"""

# TODO: Add /.F.*A/ automatic generation (and suppression)

class FunctionConvention(object):
	def __init__(self, ident: str, convname: str, valid_chars: List[str]) -> None:
		self.ident = ident
		self.name = convname
		self.values = valid_chars
# Free letters:  B   FG  J      QR T    YZa   e gh jk mno qrst    z
conventions = {
	'F': FunctionConvention('F', "System V", ['E', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'V', 'O', 'S', 'N', 'M', 'H', 'P', 'A', 'x', 'X', 'Y', 'y', 'b']),
	'W': FunctionConvention('W', "Windows",  ['E', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd',      'K', 'l', 'L', 'p', 'V', 'O', 'S', 'N', 'M',      'P', 'A'])
}
sortedvalues = ['E', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'V', 'O', 'S', 'N', 'M', 'H', 'P', 'A', 'x', 'X', 'Y', 'y', 'b', '0', '1']
assert(all(all(c not in conv.values[:i] and c in sortedvalues for i, c in enumerate(conv.values)) for conv in conventions.values()))

class FunctionType(str):
	@staticmethod
	def validate(s: str, post: str) -> bool:
		if len(s) < 3:
			raise NotImplementedError("Type {0} too short{1}".format(s, post))
		chk_type = s[0] + s[2:]
		if "E" in s:
			if ("E" in s[:2]) or ("E" in s[3:]):
				raise NotImplementedError(
					"emu64_t* not as the first parameter{0}".format(post))
			if len(s) < 4:
				raise NotImplementedError("Type {0} too short{1}".format(s, post))
			# TODO: change *FEv into true functions (right now they are redirected to *FE)
			#chk_type = s[0] + s[3:]
		
		if s[1] not in conventions:
			raise NotImplementedError("Bad middle letter {0}{1}".format(s[1], post))
		
		return all(c in conventions[s[1]].values for c in chk_type) and (('v' not in chk_type[1:]) or (len(chk_type) == 2))
	
	def get_convention(self) -> FunctionConvention:
		return conventions[self[1]]
	
	def splitchar(self) -> List[int]:
		"""
		splitchar -- Sorting key function for function signatures
		
		The longest strings are at the end, and for identical length, the string
		are sorted using a pseudo-lexicographic sort, where characters have a value
		of `values.index`.
		"""
		try:
			ret = [len(self), ord(self.get_convention().ident), self.get_convention().values.index(self[0])]
			for c in self[2:]:
				ret.append(self.get_convention().values.index(c))
			return ret
		except ValueError as e:
			raise ValueError("Value is " + self) from e
	
	def __getitem__(self, i: Union[int, slice]) -> 'FunctionType': # type: ignore [override]
		return FunctionType(super().__getitem__(i))

RedirectType = NewType('RedirectType', FunctionType)
DefineType = NewType('DefineType', str)

StructType = NewType('StructType', str)

T = TypeVar('T')
U = TypeVar('U')

# TODO: simplify construction of this (add an 'insert' method?...)
class CustOrderedDict(Generic[T, U], Iterable[T]):
	def __init__(self, dict: Dict[T, U], keys: List[T]):
		self.__indict__ = dict
		self.__inkeys__ = keys
	
	def __iter__(self):
		return iter(self.__inkeys__)
	
	def __getitem__(self, k: T) -> U:
		return self.__indict__[k]

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
		return ret
	
	def __str__(self) -> ClausesStr:
		if len(self.definess) == 1:
			return str(self.definess[0])
		else:
			return "(" + ") || (".join(map(str, self.definess)) + ")"

JumbledGlobals       = Dict[ClausesStr, List[FunctionType]]
JumbledRedirects     = Dict[ClausesStr, Dict[RedirectType, FunctionType]]
JumbledTypedefs      = Dict[RedirectType, List[str]]
JumbledStructures    = Dict[str, Tuple[StructType, str]]
JumbledStructUses    = Dict[RedirectType, FunctionType]
JumbledFilesSpecific = Dict[Filename, Tuple[JumbledTypedefs, JumbledStructures, JumbledStructUses]]
SortedGlobals       = CustOrderedDict[ClausesStr, List[FunctionType]]
SortedRedirects     = CustOrderedDict[ClausesStr, List[Tuple[RedirectType, FunctionType]]]
SortedTypedefs      = CustOrderedDict[RedirectType, List[str]]
SortedStructUses    = CustOrderedDict[RedirectType, FunctionType]
SortedFilesSpecific = Dict[Filename, Tuple[SortedTypedefs, SortedStructUses]]

def readFiles(files: Iterable[Filename]) -> Tuple[JumbledGlobals, JumbledRedirects, JumbledFilesSpecific]:
	"""
	readFiles
	
	This function is the one that parses the files.
	It returns the jumbled (gbl, redirects, {file: (typedefs, mystructs)}) tuple.
	"""
	
	# Initialize variables: gbl for all values, redirects for redirections
	gbl      : JumbledGlobals       = {}
	redirects: JumbledRedirects     = {}
	filespec : JumbledFilesSpecific = {}
	
	functions: Dict[str, Filename] = {}
	halt_required = False # Is there a GO(*, .FE*) or similar in-batch error(s)?
	# First read the files inside the headers
	for filepath in files:
		filename: Filename = filepath.split("/")[-1]
		dependants: Clause = Clause()
		
		# typedefs is a list of all "*FE*" types for the current file
		# mystructs  is a map  of all char -> (structure C name, replacement) for structures
		typedefs  : JumbledTypedefs   = {}
		mystructs   : JumbledStructures = {}
		mystructuses: JumbledStructUses = {}
		filespec[filename[:-10]] = (typedefs, mystructs, mystructuses)
		
		def add_symbol_name(funname: Union[str, None], funsname: Dict[ClausesStr, List[str]] = {"": []}):
			# Optional arguments are evaluated only once!
			nonlocal halt_required
			if funname is None:
				for k in funsname:
					if (k != "") and (len(funsname[k]) != 0):
						# Note: if this condition ever raises, check the wrapper pointed by it.
						# If you find no problem, comment the error below, add a "pass" line (so python is happy)
						# and open a ticket so I can fix this.
						raise NotImplementedError("Some functions are only implemented under one condition (probably) ({0}:{1})"
							.format(k, filename) + " [extra note in the script]\nProblematic function{}: {}".format(("" if len(funsname[k]) == 1 else "s"), funsname[k]))
					for f in funsname[k]:
						if f in ['_fini', '_init', '__bss_start', '__data_start', '_edata', '_end']:
							continue # Always allow those symbols [TODO: check if OK]
						if f in functions:
							# Check for resemblances between functions[f] and filename
							if filename.startswith(functions[f][:-12]) or functions[f].startswith(filename[:-12]):
								# Probably OK
								continue
							# Manual compatible libs detection
							match = lambda l, r: (filename[7:-10], functions[f][7:-10]) in [(l, r), (r, l)]
							if match("sdl1image", "sdl2image") \
							 or match("sdl1mixer", "sdl2mixer") \
							 or match("sdl1net", "sdl2net") \
							 or match("sdl1ttf", "sdl2ttf") \
							 or match("libgl", "libegl") \
							 or match("libgl", "glesv2") \
							 or match("libegl", "glesv2") \
							 or match("libc", "tcmallocminimal") \
							 or match("libc", "tbbmallocproxy") \
							 or match("libc", "androidshmem") \
							 or match("tcmallocminimal", "tbbmallocproxy"):
								continue
							
							# Note: this test is very (too) simple. If it ever raises, comment
							# `halt_required = True` and open an issue.
							print("The function or data {0} is declared in multiple files ({1}/{2})"
								.format(f, functions[f], filename) + " [extra note in the script]")
							halt_required = True
						functions[f] = filename
			else:
				if funname == "":
					raise NotImplementedError("This function name (\"\") is suspicious... ({0})".format(filename))
				l = len(dependants.defines)
				already_pst = funname in funsname[""]
				if l > 1:
					return
				elif l == 1:
					funsname.setdefault(str(dependants), [])
					already_pst = already_pst or (funname in funsname[str(dependants)])
				if already_pst:
					print("Function or data {0} is duplicated! ({1})".format(funname, filename))
					halt_required = True
					return
				if l == 1:
					s = str(dependants.defines[0].inverted())
					if (s in funsname) and (funname in funsname[s]):
						funsname[s].remove(funname)
						funsname[""].append(funname)
					else:
						funsname[str(dependants)].append(funname)
				else:
					funsname[""].append(funname)
		
		with open(filepath, 'r') as file:
			line: str # Because VSCode really struggles with files
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
						if not filename.endswith("_genvate.h"):
							add_symbol_name(funname)
					except IndexError:
						raise NotImplementedError("Invalid GO command: {0}:{1}".format(
							filename, line[:-1]
						))
					
					hasFlatStructure = False
					origLine = ln
					if not FunctionType.validate(ln, " ({0}:{1})".format(filename, line[:-1])):
						# This needs more work
						old = RedirectType(FunctionType(ln))
						if (ln[0] in old.get_convention().values) \
						 and ('v' not in ln[2:]) \
						 and all((c in old.get_convention().values) or (c in mystructs) for c in ln[2:]):
							hasFlatStructure = True
							
							for sn in mystructs:
								ln = ln.replace(sn, mystructs[sn][1])
							ln = ln[0] + 'F' + ln[2:] # In case a structure named 'F' is used
							mystructuses[RedirectType(FunctionType(origLine))] = FunctionType(ln)
						else:
							if old.get_convention().name == "System V":
								acceptables = ['0', '1'] + old.get_convention().values
								if any(c not in acceptables for c in ln[2:]):
									raise NotImplementedError("{0} ({1}:{2})".format(ln[2:], filename, line[:-1]))
								# Ok, this is acceptable: there is 0, 1 and/or void
								ln = ln[:2] + (ln[2:]
									.replace("v", "")   # void   -> nothing
									.replace("0", "i")  # 0      -> integer
									.replace("1", "i")) # 1      -> integer
								assert(len(ln) >= 3)
							else:
								acceptables = ['0', '1', 'D', 'H'] + old.get_convention().values
								if any(c not in acceptables for c in ln[2:]):
									raise NotImplementedError("{0} ({1}:{2})".format(ln[2:], filename, line[:-1]))
								# Ok, this is acceptable: there is 0, 1 and/or void
								ln = ln[:2] + (ln[2:]
									.replace("v", "")   # void              -> nothing
									.replace("D", "p")  # long double       -> pointer
									.replace("H", "p")  # unsigned __int128 -> pointer
									.replace("0", "i")  # 0                 -> integer
									.replace("1", "i")) # 1                 -> integer
								assert(len(ln) >= 3)
							redirects.setdefault(str(dependants), {})
							redirects[str(dependants)][old] = FunctionType(ln)
							
							origLine = ln
					
					# Simply append the function type if it's not yet existing
					gbl.setdefault(str(dependants), [])
					if ln not in gbl[str(dependants)]:
						gbl[str(dependants)].append(FunctionType(ln))
					
					if origLine[2] == "E":
						if (gotype != "GOM") and (gotype != "GOWM"):
							if (gotype != "GO2") or not (line.split(',')[2].split(')')[0].strip().startswith('my_')):
								print("\033[91mThis is probably not what you meant!\033[m ({0}:{1})".format(filename, line[:-1]))
								halt_required = True
						if len(origLine) > 3:
							funtype = RedirectType(FunctionType(origLine[:2] + origLine[3:]))
						else:
							funtype = RedirectType(FunctionType(origLine[:2] + "v"))
						# filename isn't stored with the '_private.h' part
						typedefs.setdefault(funtype, [])
						typedefs[funtype].append(funname)
					elif (gotype == "GOM") or (gotype == "GOWM"):
						# OK on box64 for a GOM to not have emu...
						funtype = RedirectType(FunctionType(origLine))
						typedefs.setdefault(funtype, [])
						typedefs[funtype].append(funname)
						# print("\033[94mAre you sure of this?\033[m ({0}:{1})".format(filename, line[:-1]))
						# halt_required = True
					elif hasFlatStructure:
						# Still put the type in typedefs, but don't add the function name
						typedefs.setdefault(RedirectType(FunctionType(origLine)), [])
				
				# If the line is a structure metadata information...
				# FIXME: what happens with e.g. a Windows function?
				elif ln.startswith("//%S"):
					metadata = [e for e in ln.split() if e]
					if len(metadata) != 4:
						raise NotImplementedError("Invalid structure metadata supply (too many fields) ({0}:{1})".format(filename, line[:-1]))
					if metadata[0] != "//%S":
						raise NotImplementedError("Invalid structure metadata supply (invalid signature) ({0}:{1})".format(filename, line[:-1]))
					if len(metadata[1]) != 1:
						# If you REALLY need it, consider opening a ticket
						# Before you do, consider that everything that is a valid in a C token is valid here too
						raise NotImplementedError("Structure names cannot be of length greater than 1 ({0}:{1})".format(filename, line[:-1]))
					if metadata[3] == "":
						# If you need this, please open an issue (this is never actually called, empty strings are removed)
						raise NotImplementedError("Invalid structure metadata supply (empty replacement) ({0}:{1})".format(filename, line[:-1]))
					if any(c not in conventions['F'].values for c in metadata[3]):
						# Note that replacement cannot be another structure type
						raise NotImplementedError("Invalid structure metadata supply (invalid replacement) ({0}:{1})".format(filename, line[:-1]))
					if metadata[1] in mystructs:
						raise NotImplementedError("Invalid structure nickname {0} (duplicate) ({1}/{2})".format(metadata[1], filename, line[:-1]))
					if (metadata[1] in conventions['F'].values) or (metadata[1] in ['0', '1']):
						raise NotImplementedError("Invalid structure nickname {0} (reserved) ({1}/{2})".format(metadata[1], filename, line[:-1]))
					
					# OK, add into the database
					mystructs[metadata[1]] = (StructType(metadata[2]), metadata[3])
				
				# If the line contains any symbol name...
				elif ("GO" in ln) or ("DATA" in ln):
					# Probably "//GO(..., " or "DATA(...," at least
					try:
						funname = ln.split('(')[1].split(',')[0].strip()
						add_symbol_name(funname)
					except IndexError:
						# Oops, it wasn't...
						pass
		
		add_symbol_name(None)
	
	if halt_required:
		raise ValueError("Fix all previous errors before proceeding")
	
	if ("" not in gbl) or ("" not in redirects):
		print("\033[1;31mThere is suspiciously not many types...\033[m")
		print("Check the CMakeLists.txt file. If you are SURE there is nothing wrong"
		      " (as a random example, `set()` resets the variable...), then comment out the following exit.")
		print("(Also, the program WILL crash later if you proceed.)")
		sys.exit(2) # Check what you did, not proceeding
	
	return gbl, redirects, filespec

def sortArrays(gbl_tmp : JumbledGlobals, red_tmp : JumbledRedirects, filespec: JumbledFilesSpecific) \
 -> Tuple[SortedGlobals, SortedRedirects, SortedFilesSpecific]:
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
		if key not in gbl_idxs:
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
	# Also do the same trick as before (it also helps keep the order
	# in the file deterministic)
	redirects: Dict[ClausesStr, List[Tuple[RedirectType, FunctionType]]] = {}
	redirects_idxs: List[ClausesStr] = []
	for k1, v in redirects_vals:
		clauses = redirects_vals[(k1, v)]
		key = str(clauses)
		redirects.setdefault(key, [])
		redirects[key].append((k1, v))
		if key not in redirects_idxs:
			redirects_idxs.append(key)
	redirects_idxs.sort(key=lambda c: Clauses(c).splitdef())
	
	# Sort the function types as defined in `splitchar`
	for k3 in gbl:
		gbl[k3].sort(key=FunctionType.splitchar)
	
	oldvals = { k: conventions[k].values for k in conventions }
	for k in conventions:
		conventions[k].values = sortedvalues
	for k3 in redirects:
		redirects[k3].sort(key=lambda v: v[0].splitchar() + v[1].splitchar())
	for k in conventions:
		conventions[k].values = oldvals[k]
	
	sortedfilespec: SortedFilesSpecific = {}
	for fn in filespec:
		# Maybe do better?
		mystructs_vals: List[str] = sorted(filespec[fn][1].keys())
		if mystructs_vals != []:
			for k in conventions:
				conventions[k].values = conventions[k].values + list(mystructs_vals)
		
		mytypedefs_vals: List[RedirectType] = sorted(filespec[fn][0].keys(), key=FunctionType.splitchar)
		sortedfilespec[fn] = (
			CustOrderedDict(dict((v, sorted(filespec[fn][0][v])) for v in mytypedefs_vals), mytypedefs_vals),
			CustOrderedDict(filespec[fn][2], sorted(filespec[fn][2], key=FunctionType.splitchar))
		)
		
		if mystructs_vals != []:
			for k in conventions:
				conventions[k].values = conventions[k].values[:-len(mystructs_vals)]
	
	return CustOrderedDict(gbl, gbl_idxs), CustOrderedDict(redirects, redirects_idxs), sortedfilespec

def checkRun(root: str, jumbled: JumbledFilesSpecific, \
  gbls: SortedGlobals, redirects: SortedRedirects, filesspec: SortedFilesSpecific) -> Optional[str]:
	# Check if there was any new functions compared to last run
	functions_list: str = ""
	for k in gbls:
		for v in gbls[k]:
			functions_list = functions_list + "#" + k + " " + v + "\n"
	for k in redirects:
		for vr, vf in redirects[k]:
			functions_list = functions_list + "#" + k + " " + vr + " -> " + vf + "\n"
	for filename in sorted(filesspec.keys()):
		functions_list = functions_list + filename + ":\n"
		for st in sorted(jumbled[filename][1].keys()):
			functions_list = functions_list + \
				"% " + st + " " + jumbled[filename][1][st][0] + " " + jumbled[filename][1][st][1] + "\n"
		for vr in filesspec[filename][0]:
			functions_list = functions_list + "- " + vr + ":\n"
			for fn in filesspec[filename][0][vr]:
				functions_list = functions_list + "  - " + fn + "\n"
		for defined in filesspec[filename][1]:
			functions_list = functions_list + "% " + defined + "\n"
	
	# functions_list is a unique string, compare it with the last run
	try:
		last_run = ""
		with open(os.path.join(root, "src", "wrapped", "generated", "functions_list.txt"), 'r') as file:
			last_run = file.read()
		if last_run == functions_list:
			# Mark as OK for CMake
			with open(os.path.join(root, "src", "wrapped", "generated", "functions_list.txt"), 'w') as file:
				file.write(functions_list)
			return None
	except IOError:
		# The file does not exist yet, first run
		pass
	
	return functions_list

def main(root: str, files: Iterable[Filename], ver: str):
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
	# fsp_tmp:
	#  "filename" -> (
	#   [vFEv -> fopen, ...],
	#   [G -> ("SDL_J...", UU), ...],
	#   [vFGppp -> vFUUppp, ...]
	#  )
	gbl_tmp: JumbledGlobals
	red_tmp: JumbledRedirects
	fsp_tmp: JumbledFilesSpecific
	
	gbl_tmp, red_tmp, fsp_tmp = readFiles(files)
	
	# gbls: sorted gbl_tmp
	# redirects: sorted red_tmp
	# filesspec:
	#  "filename" -> (
	#   sorted [vFEv -> fopen, ...],
	#   sorted [vFGppp -> vFUUppp, ...]
	#  )
	gbls     : SortedGlobals
	redirects: SortedRedirects
	filesspec: SortedFilesSpecific
	
	gbls, redirects, filesspec = sortArrays(gbl_tmp, red_tmp, fsp_tmp)
	
	functions_list = checkRun(root, fsp_tmp, gbls, redirects, filesspec)
	if functions_list is None:
		print("Detected same build as last run, skipping")
		return 0
	
	# Detect simple wrappings
	allowed_conv_ident = "F"
	allowed_conv = conventions[allowed_conv_ident]
	
	# H could be allowed maybe?
	allowed_simply: Dict[str, str] = {"ARM64": "v", "RV64": "v"}
	allowed_regs  : Dict[str, str] = {"ARM64": "cCwWiuIUlLp", "RV64": "CWuIUlLp"}
	allowed_fpr   : Dict[str, str] = {"ARM64": "fd", "RV64": "fd"}
	
	# Detect functions which return in an x87 register
	retx87_wraps: Dict[ClausesStr, List[FunctionType]] = {}
	return_x87: str = "DK"
	
	# Sanity checks
	forbidden_simple: Dict[str, str] = {"ARM64": "EDKVOSNMHPAxXYyb", "RV64": "EcwiDKVOSNMHPAxXYyb"}
	assert(all(k in allowed_simply for k in forbidden_simple))
	assert(all(k in allowed_regs for k in forbidden_simple))
	assert(all(k in allowed_fpr for k in forbidden_simple))
	for k1 in forbidden_simple:
		assert(len(allowed_simply[k1]) + len(allowed_regs[k1]) + len(allowed_fpr[k1]) + len(forbidden_simple[k1]) == len(allowed_conv.values))
		assert(all(c not in allowed_regs[k1] for c in allowed_simply[k1]))
		assert(all(c not in allowed_simply[k1] + allowed_regs[k1] for c in allowed_fpr[k1]))
		assert(all(c not in allowed_simply[k1] + allowed_regs[k1] + allowed_fpr[k1] for c in forbidden_simple[k1]))
		assert(all(c in allowed_simply[k1] + allowed_regs[k1] + allowed_fpr[k1] + forbidden_simple[k1] for c in allowed_conv.values))
	assert(all(c in allowed_conv.values for c in return_x87))
	
	simple_wraps: Dict[str, Dict[ClausesStr, List[Tuple[FunctionType, int]]]] = {
		k1: {} for k1 in forbidden_simple
	}
	
	def check_simple(v: FunctionType) -> Dict[str, int]:
		regs_count: int = 0
		fpr_count : int = 0
		
		ret = {}
		for k in forbidden_simple:
			if v.get_convention() is not allowed_conv:
				continue
			if v[0] in forbidden_simple[k]:
				continue
			for c in v[2:]:
				if c in allowed_regs[k]:
					regs_count = regs_count + 1
				elif c in allowed_fpr[k]:
					fpr_count = fpr_count + 1
				elif c in allowed_simply[k]:
					continue
				else:
					break
			else:
				# No character in forbidden_simply
				if (regs_count <= 6) and (fpr_count <= 8):
					# All checks passed!
					ret_val = 1 + fpr_count
					if v[0] in allowed_fpr[k]:
						ret_val = -ret_val
					ret[k] = ret_val
				# Else, too many arguments
		return ret
	
	# Only search in real wrappers (mapped ones are nearly always not simple)
	for k in gbls:
		for v in gbls[k]:
			simples = check_simple(v)
			for k1, i in simples.items():
				if k in simple_wraps[k1]:
					simple_wraps[k1][k].append((v, i))
				else:
					simple_wraps[k1][k] = [(v, i)]
	simple_idxs = { k1: sorted(v1.keys(), key=lambda x: Clauses(x).splitdef()) for k1, v1 in simple_wraps.items() }
	
	def check_return_x87(v: FunctionType) -> bool:
		return v[0] in return_x87
    
	for k in gbls:
		tmp = [v for v in gbls[k] if check_return_x87(v)]
		if tmp:
			retx87_wraps[k] = tmp
	retx87_idxs = sorted(retx87_wraps.keys(), key=lambda x: Clauses(x).splitdef())
	
	# Now the files rebuilding part
	# File headers and guards
	files_header = {
		"wrapper.c": """
		#include <stdio.h>
		#include <stdlib.h>
		#include <stdint.h>
		
		#include "wrapper.h"
		#include "emu/x64emu_private.h"
		#include "emu/x87emu_private.h"
		#include "regs.h"
		#include "x64emu.h"
		#define COMPLEX_IMPL
		#include "complext.h"
		
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
		void* align_xcb_connection(void* src);
		void unalign_xcb_connection(void* src, void* dst);
		
		""",
		"wrapper.h": """
		#ifndef __WRAPPER_H_
		#define __WRAPPER_H_
		#include <stdint.h>
		#include <string.h>
		#include "complext.h"
		
		typedef struct x64emu_s x64emu_t;
		
		// the generic wrapper pointer functions
		typedef void (*wrapper_t)(x64emu_t* emu, uintptr_t fnc);

	
		// list of defined wrapper
		// E = current x86emu struct
		// v = void
		// C = unsigned byte c = char
		// W = unsigned short w = short
		// u = uint32, i = int32
		// U = uint64, I = int64
		// L = unsigned long, l = signed long (long is an int with the size of a pointer)
		// H = Huge 128bits value/struct
		// p = pointer, P = void* on the stack
		// f = float, d = double, D = long double, K = fake long double
		// V = vaargs
		// O = libc O_ flags bitfield
		// o = stdout
		// S = _IO_2_1_stdXXX_ pointer (or FILE*)
		// N = ... automatically sending 1 arg
		// M = ... automatically sending 2 args
		// A = va_list
		// 0 = constant 0, 1 = constant 1
		// x = float complex
		// X = double complex
		// b = xcb_connection_t*
		
		""",
		"fntypes.h": """
		#ifndef __{filename}TYPES_H_
		#define __{filename}TYPES_H_
		
		#ifndef LIBNAME
		#error You should only #include this file inside a wrapped*.c file
		#endif
		#ifndef ADDED_FUNCTIONS
		#define ADDED_FUNCTIONS() 
		#endif
		
		""",
		"fndefs.h": """
		#ifndef __{filename}DEFS_H_
		#define __{filename}DEFS_H_
		
		""",
		"fnundefs.h": """
		#ifndef __{filename}UNDEFS_H_
		#define __{filename}UNDEFS_H_
		
		"""
	}
	files_guard = {
		"wrapper.c": """
		""",
		"wrapper.h": """
		
		int isSimpleWrapper(wrapper_t fun);
		
		#endif // __WRAPPER_H_
		""",
		"fntypes.h": """
		
		#endif // __{filename}TYPES_H_
		""",
		"fndefs.h": """
		
		#endif // __{filename}DEFS_H_
		""",
		"fnundefs.h": """
		
		#endif // __{filename}UNDEFS_H_
		"""
	}
	
	banner = "/********************************************************" + ('*'*len(ver)) + "***\n" \
	         " * File automatically generated by rebuild_wrappers.py (v" + ver + ") *\n" \
	         " ********************************************************" + ('*'*len(ver)) + "***/\n"
	trim = lambda string: '\n'.join(line[2:] for line in string.splitlines())[1:]
	# Yes, the for loops are inversed. This is because both dicts should have the same keys.
	for fhdr in files_guard:
		files_header[fhdr] = banner + trim(files_header[fhdr])
	for fhdr in files_header:
		files_guard[fhdr] = trim(files_guard[fhdr])
	
	# Rewrite the wrapper.c file:
	# i and u should only be 32 bits
	td_types = {
		#      E            v       c         w          i          I          C          W           u           U           f        d         D              K         l           L            p        V        O          S        N      M      H                    P        A			x				X           Y              y         b
		'F': ["x64emu_t*", "void", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "float", "double", "long double", "double", "intptr_t", "uintptr_t", "void*", "void*", "int32_t", "void*", "...", "...", "unsigned __int128", "void*", "void*", "complexf_t", "complex_t", "complexl_t", "complex_t", "void*"],
		#      E            v       c         w          i          I          C          W           u           U           f        d         K         l           L            p        V        O          S        N      M      P        A
		'W': ["x64emu_t*", "void", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "float", "double", "double", "intptr_t", "uintptr_t", "void*", "void*", "int32_t", "void*", "...", "...", "void*", "void*"]
	}
	assert(all(k in conventions for k in td_types))
	for k in conventions:
		if len(conventions[k].values) != len(td_types[k]):
			raise NotImplementedError("len(values) = {lenval} != len(td_types) = {lentypes}".format(lenval=len(conventions[k].values), lentypes=len(td_types[k])))
	
	def generate_typedefs(arr: Iterable[FunctionType], file) -> None:
		for v in arr:
			file.write("typedef " + td_types[v.get_convention().ident][v.get_convention().values.index(v[0])] + " (*" + v + "_t)"
				+ "(" + ', '.join(td_types[v.get_convention().ident][v.get_convention().values.index(t)] for t in v[2:]) + ");\n")
	
	with open(os.path.join(root, "src", "wrapped", "generated", "wrapper.c"), 'w') as file:
		file.write(files_header["wrapper.c"].format(lbr="{", rbr="}", version=ver))
		
		# First part: typedefs
		for k in gbls:
			if k != str(Clauses()):
				file.write("\n#if " + k + "\n")
			generate_typedefs(gbls[k], file)
			if k != str(Clauses()):
				file.write("#endif\n")
		
		file.write("\n")
		
		# Next part: function definitions
		
		# Helper variables
		# Return type template
		vals = {
			conventions['F']: [
				"\n#error Invalid return type: emulator\n",                # E
				"fn({0});",                                                # v
				"R_RAX=fn({0});",                                          # c
				"R_RAX=fn({0});",                                          # w
				"R_RAX=(int32_t)fn({0});",                                 # i
				"R_RAX=(int64_t)fn({0});",                                 # I
				"R_RAX=(unsigned char)fn({0});",                           # C
				"R_RAX=(unsigned short)fn({0});",                          # W
				"R_RAX=(uint32_t)fn({0});",                                # u
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
				"\n#error Invalid return type: va_list\n",                 # A
				'from_complexf(emu, fn({0}));', 	                       # x
				'from_complex(emu, fn({0}));',                             # X
				'from_complexl(emu, fn({0}));', 	                       # Y
				'from_complexk(emu, fn({0}));',                            # y
				"\n#error Invalid return type: xcb_connection_t*\n",       # b
			],
			conventions['W']: [
				"\n#error Invalid return type: emulator\n",                # E
				"fn({0});",                                                # v
				"R_RAX=fn({0});",                                          # c
				"R_RAX=fn({0});",                                          # w
				"R_RAX=(int32_t)fn({0});",                                 # i
				"R_RAX=(int64_t)fn({0});",                                 # I
				"R_RAX=(unsigned char)fn({0});",                           # C
				"R_RAX=(unsigned short)fn({0});",                          # W
				"R_RAX=(uint32_t)fn({0});",                                # u
				"R_RAX=fn({0});",                                          # U
				"emu->xmm[0].f[0]=fn({0});",                               # f
				"emu->xmm[0].d[0]=fn({0});",                               # d
				"double db=fn({0}); fpu_do_push(emu); ST0val = db;",       # K
				"R_RAX=(intptr_t)fn({0});",                                # l
				"R_RAX=(uintptr_t)fn({0});",                               # L
				"R_RAX=(uintptr_t)fn({0});",                               # p
				"\n#error Invalid return type: va_list\n",                 # V
				"\n#error Invalid return type: at_flags\n",                # O
				"\n#error Invalid return type: _io_file*\n",               # S
				"\n#error Invalid return type: ... with 1 arg\n",          # N
				"\n#error Invalid return type: ... with 2 args\n",         # M
				"\n#error Invalid return type: pointer in the stack\n",    # P
				"\n#error Invalid return type: va_list\n",                 # A
			]
		}
		
		# vreg: value is in a general register
		#         E  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  N  M  H  P  A  x  X  Y  y  b
		vreg   = [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 2, 2, 0, 1, 0, 0, 0, 0, 1]
		# vxmm: value is in a XMM register
		#         E  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  N  M  H  P  A  x  X  Y  y  b
		vxmm   = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0]
		# vother: value is elsewere
		#         E  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  N  M  H  P  A  x  X  Y  y  b
		vother = [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
		# vstack: value is on the stack (or out of register)
		#         E  v  c  w  i  I  C  W  u  U  f  d  D  K  l  L  p  V  O  S  N  M  H  P  A  x  X  Y  y  b
		vstack = [0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 0, 1, 1, 1, 2, 2, 1, 1, 1, 2, 4, 4, 1]
		arg_r = [
			"",                            # E
			"",                            # v
			"(int8_t){p}, ",               # c
			"(int16_t){p}, ",              # w
			"(int32_t){p}, ",              # i
			"(int64_t){p}, ",              # I
			"(uint8_t){p}, ",              # C
			"(uint16_t){p}, ",             # W
			"(uint32_t){p}, ",             # u
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
			"(void*){p}, ",                # A
			"",	                           # x
			"",                            # X
			"",							   # Y
			"",							   # y
			"aligned_xcb, ",               # b
		]
		arg_x = [
			"",                       # E
			"",                       # v
			"",                       # c
			"",                       # w
			"",                       # i
			"",                       # I
			"",                       # C
			"",                       # W
			"",                       # u
			"",                       # U
			"emu->xmm[{p}].f[0], ",   # f
			"emu->xmm[{p}].d[0], ",   # d
			"",                       # D
			"",                       # K
			"",                       # l
			"",                       # L
			"",                       # p
			"",                       # V
			"",                       # O
			"",                       # S
			"",                       # N
			"",                       # M
			"",                       # H
			"",                       # P
			"",                       # A
			"to_complexf(emu, {p}), ", # x
			"to_complex(emu, {p}), ",  # X
			"", 				      # Y
			"", 				      # y
			"",                       # b
		]
		arg_o = [
			"emu, ",                  # E
			"",                       # v
			"",                       # c
			"",                       # w
			"",                       # i
			"",                       # I
			"",                       # C
			"",                       # W
			"",                       # u
			"",                       # U
			"",                       # f
			"",                       # d
			"",                       # D
			"",                       # K
			"",                       # l
			"",                       # L
			"",                       # p
			"(void*)(R_RSP + {p}), ", # V
			"",                       # O
			"",                       # S
			"",                       # N
			"",                       # M
			"",                       # H
			"",                       # P
			"",                       # A
			"",                       # x
			"",                       # X
			"",						  # Y
			"",						  # y
			"",                       # b
		]
		arg_s = [
			"",                                         # E
			"",                                         # v
			"*(int8_t*)(R_RSP + {p}), ",                # c
			"*(int16_t*)(R_RSP + {p}), ",               # w
			"*(int32_t*)(R_RSP + {p}), ",               # i
			"*(int64_t*)(R_RSP + {p}), ",               # I
			"*(uint8_t*)(R_RSP + {p}), ",               # C
			"*(uint16_t*)(R_RSP + {p}), ",              # W
			"*(uint32_t*)(R_RSP + {p}), ",              # u
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
			"*(unsigned __int128*)(R_RSP + {p}), ",     # H
			"*(void**)(R_RSP + {p}), ",                 # P
			"*(void**)(R_RSP + {p}), ",                 # A
			"*(complexf_t*)(R_RSP + {p}), ",            # x
			"*(complex_t*)(R_RSP + {p}), ",             # X
			"to_complexl(emu, R_RSP + {p}), ",		    # Y
			"to_complexk(emu, R_RSP + {p}), ",		    # y
			"aligned_xcb, ",                            # b
		]
		
		# Asserts
		for k in conventions:
			assert all(v in conventions['F'].values for v in conventions[k].values), "a convention is not a subset of System V"
		assert all(vr == vs for (vr, vs) in zip(vreg, vstack) if vr != 0), "vreg and vstack are inconsistent"
		assert all(vx == vs for (vx, vs) in zip(vxmm, vstack) if vx != 0), "vxmm and vstack are inconsistent"
		assert all((vo == 0) == (vs != 0) for (vo, vs) in zip(vother, vstack)), "vother and vstack are inconsistent"
		if len(conventions['F'].values) != len(vstack):
			raise NotImplementedError("len(values) = {lenval} != len(vstack) = {lenvstack}".format(lenval=len(conventions['F'].values), lenvstack=len(vstack)))
		if len(conventions['F'].values) != len(vreg):
			raise NotImplementedError("len(values) = {lenval} != len(vreg) = {lenvreg}".format(lenval=len(conventions['F'].values), lenvreg=len(vreg)))
		if len(conventions['F'].values) != len(vxmm):
			raise NotImplementedError("len(values) = {lenval} != len(vxmm) = {lenvxmm}".format(lenval=len(conventions['F'].values), lenvxmm=len(vxmm)))
		if len(conventions['F'].values) != len(vother):
			raise NotImplementedError("len(values) = {lenval} != len(vother) = {lenvother}".format(lenval=len(conventions['F'].values), lenvother=len(vother)))
		if len(conventions['F'].values) != len(arg_s):
			raise NotImplementedError("len(values) = {lenval} != len(arg_s) = {lenargs}".format(lenval=len(conventions['F'].values), lenargs=len(arg_s)))
		if len(conventions['F'].values) != len(arg_r):
			raise NotImplementedError("len(values) = {lenval} != len(arg_r) = {lenargr}".format(lenval=len(conventions['F'].values), lenargr=len(arg_r)))
		if len(conventions['F'].values) != len(arg_x):
			raise NotImplementedError("len(values) = {lenval} != len(arg_x) = {lenargx}".format(lenval=len(conventions['F'].values), lenargx=len(arg_x)))
		if len(conventions['F'].values) != len(arg_o):
			raise NotImplementedError("len(values) = {lenval} != len(arg_o) = {lenargo}".format(lenval=len(conventions['F'].values), lenargo=len(arg_o)))
		for k in conventions:
			c = conventions[k]
			if c not in vals:
				raise NotImplementedError("convention {k} not in vals".format(k=k))
			if len(c.values) != len(vals[c]):
				raise NotImplementedError("len([{k}]values) = {lenval} != len(vals[...]) = {lenvals}".format(k=k, lenval=len(c.values), lenvals=len(vals[c])))
		# When arg_* is not empty, v* should not be 0
		if any(map(lambda v, a: (a != "") and (v == 0), vstack, arg_s)):
			raise NotImplementedError("Something in the stack has a null offset and a non-empty arg string")
		if any(map(lambda v, a: (a != "") and (v == 0), vreg, arg_r)):
			raise NotImplementedError("Something in the stack has a null offset and a non-empty arg string")
		if any(map(lambda v, a: (a != "") and (v == 0), vxmm, arg_x)):
			raise NotImplementedError("Something in the stack has a null offset and a non-empty arg string")
		if any(map(lambda v, a: (a != "") and (v == 0), vother, arg_o)):
			raise NotImplementedError("Something in the stack has a null offset and a non-empty arg string")
		# Everything is either in the stack or somewhere else, it cannot be in a GPr and in an XMMr, etc
		if any(map(lambda o, s: (o == 0) == (s == 0), vother, vstack)):
			raise NotImplementedError("Something cannot be in exactly one of the stack and somewhere else")
		if any(map(lambda r, x: (r > 0) and (x > 0), vreg, vxmm)):
			raise NotImplementedError("Something can be in both a general purpose register and in an XMM register")
		if any(map(lambda r, s: (r > 0) and (s == 0), vreg, vstack)):
			raise NotImplementedError("Something can be in a general purpose register but not in the stack")
		if any(map(lambda x, s: (x > 0) and (s == 0), vxmm, vstack)):
			raise NotImplementedError("Something can be in an XMM register but not in the stack")
		
		# Helper functions to write the function definitions
		systemVconv = conventions['F']
		def function_pre_systemV(args: FunctionType, d: int = 8, r: int = 0, x: int = 0) -> Tuple[Optional[str], str]:
			# args: string of argument types
			# d: delta (in the stack)
			# r: general register no
			# x: XMM register no
			if len(args) == 0:
				return None, ""
			
			# Redirections
			if args[0] == "0":
				return function_pre_systemV(args[1:], d, r, x)
			elif args[0] == "1":
				return function_pre_systemV(args[1:], d, r, x)
			
			idx = systemVconv.values.index(args[0])
			# Name of the registers
			reg_arg = ["R_RDI", "R_RSI", "R_RDX", "R_RCX", "R_R8", "R_R9"]
			if args[0] == "b":
				if 'b' in args[1:]:
					raise NotImplementedError("Multiple XCB connections unsupported")
				content = ""
				if r < len(reg_arg):
					content = reg_arg[r]
				else:
					content = "(R_RSP + " + str(d) + ")"
				return "(void*)" + content, f"void *aligned_xcb = align_xcb_connection((void*){content}); "
			elif (r < len(reg_arg)) and (vreg[idx] > 0):
				for _ in range(vreg[idx]):
					if r < len(reg_arg):
						r = r + 1
					else:
						d = d + 8
				return function_pre_systemV(args[1:], d, r, x)
			elif (x < 8) and (vxmm[idx] > 0):
				return function_pre_systemV(args[1:], d, r, x+1)
			elif vstack[idx] > 0:
				return function_pre_systemV(args[1:], d+8*vstack[idx], r, x)
			else:
				return function_pre_systemV(args[1:], d, r, x)
		def function_post_systemV(content: Optional[str]) -> str:
			if content is not None:
				return f" unalign_xcb_connection(aligned_xcb, {content});"
			else:
				return ""
		def function_args_systemV(args: FunctionType, d: int = 8, r: int = 0, x: int = 0) -> str:
			# args: string of argument types
			# d: delta (in the stack)
			# r: general register no
			# x: XMM register no
			if len(args) == 0:
				return ""
			
			# Redirections
			if args[0] == "0":
				return "0, " + function_args_systemV(args[1:], d, r, x)
			elif args[0] == "1":
				return "1, " + function_args_systemV(args[1:], d, r, x)
			
			idx = systemVconv.values.index(args[0])
			# Name of the registers
			reg_arg = ["R_RDI", "R_RSI", "R_RDX", "R_RCX", "R_R8", "R_R9"]
			if (r < len(reg_arg)) and (vreg[idx] > 0):
				ret = ""
				for _ in range(vreg[idx]):
					# There may be values in multiple registers
					if r < len(reg_arg):
						# Value is in a general register
						ret = ret + arg_r[idx].format(p=reg_arg[r])
						r = r + 1
					else:
						# Remaining is in the stack
						ret = ret + arg_s[idx].format(p=d)
						d = d + 8
				return ret + function_args_systemV(args[1:], d, r, x)
			elif (x < 8) and (vxmm[idx] > 0):
				# Value is in an XMM register
				return arg_x[idx].format(p=x) + function_args_systemV(args[1:], d, r, x+vxmm[idx])
			elif vstack[idx] > 0:
				# Value is in the stack
				return arg_s[idx].format(p=d) + function_args_systemV(args[1:], d+8*vstack[idx], r, x)
			else:
				# Value is somewhere else
				return arg_o[idx].format(p=d) + function_args_systemV(args[1:], d, r, x)
		# windowsconv = conventions['W']
		def function_args_windows(args: FunctionType, d: int = 40, r: int = 0) -> str:
			# args: string of argument types
			# d: delta (in the stack)
			# r: general register no
			# We can re-use vstack to know if we need to put a pointer or the value
			if len(args) == 0:
				return ""
			
			# Redirections
			if args[0] == "0":
				return "0, " + function_args_windows(args[1:], d, r)
			elif args[0] == "1":
				return "1, " + function_args_windows(args[1:], d, r)
			
			idx = systemVconv.values.index(args[0]) # Little hack to be able to re-use
			# Name of the registers
			reg_arg = ["R_RCX", "R_RDX", "R_R8", "R_R9"]
			if (r < len(reg_arg)) and (vstack[idx] == 1):
				# We use a register
				if vreg[idx] == 1:
					# Value is in a general register
					return arg_r[idx].format(p=reg_arg[r]) + function_args_windows(args[1:], d, r+1)
				else:
					# Remaining is in an XMM register
					return arg_x[idx].format(p=r) + function_args_windows(args[1:], d, r+1)
			elif vstack[idx] > 0:
				# Value is in the stack
				return arg_s[idx].format(p=d) + function_args_windows(args[1:], d+8*vstack[idx], r)
			else:
				# Value is somewhere else
				return arg_o[idx].format(p=d) + function_args_windows(args[1:], d, r)
		
		def function_writer(f, N: FunctionType, W: str) -> None:
			# Write to f the function type N (real type W)
			
			f.write("void {0}(x64emu_t *emu, uintptr_t fcn) {2} {1} fn = ({1})fcn; ".format(N, W, "{"))
			# Generic function
			conv = N.get_convention()
			if conv is systemVconv:
				prepost, pre = function_pre_systemV(N[2:])
				f.write(pre + vals[conv][conv.values.index(N[0])].format(function_args_systemV(N[2:])[:-2]) + function_post_systemV(prepost))
			else:
				f.write(vals[conv][conv.values.index(N[0])].format(function_args_windows(N[2:])[:-2]))
			f.write(" }\n")
		
		for k in gbls:
			if k != str(Clauses()):
				file.write("\n#if " + k + "\n")
			for v in gbls[k]:
				if v == FunctionType("vFv"):
					# Suppress all warnings...
					file.write("void vFv(x64emu_t *emu, uintptr_t fcn) { vFv_t fn = (vFv_t)fcn; fn(); (void)emu; }\n")
				else:
					function_writer(file, v, v + "_t")
			if k != str(Clauses()):
				file.write("#endif\n")
		file.write("\n")
		for k in redirects:
			if k != str(Clauses()):
				file.write("\n#if " + k + "\n")
			for vr, vf in redirects[k]:
				function_writer(file, vr, vf + "_t")
			if k != str(Clauses()):
				file.write("#endif\n")
		
		# Write the isSimpleWrapper function
		inttext = ""
		file.write("\n")
		for k1 in simple_idxs:
			file.write("#{inttext}if defined({k1})\nint isSimpleWrapper(wrapper_t fun) {{\n".format(inttext=inttext, k1=k1))
			inttext = "el"
			for k in simple_idxs[k1]:
				if k != str(Clauses()):
					file.write("#if " + k + "\n")
				for vf, val in simple_wraps[k1][k]:
					file.write("\tif (fun == &" + vf + ") return " + str(val) + ";\n")
				if k != str(Clauses()):
					file.write("#endif\n")
			file.write("\treturn 0;\n}\n")
		file.write("\n#else\nint isSimpleWrapper(wrapper_t fun) {\n\treturn 0;\n}\n#endif\n")
		
		# Write the isRetX87Wrapper function
		file.write("\nint isRetX87Wrapper(wrapper_t fun) {\n")
		for k in retx87_idxs:
			if k != str(Clauses()):
				file.write("#if " + k + "\n")
			for vf in retx87_wraps[k]:
				file.write("\tif (fun == &" + vf + ") return 1;\n")
			if k != str(Clauses()):
				file.write("#endif\n")
		file.write("\treturn 0;\n}\n")
		
		file.write(files_guard["wrapper.c"].format(lbr="{", rbr="}", version=ver))
	
	# Rewrite the wrapper.h file:
	with open(os.path.join(root, "src", "wrapped", "generated", "wrapper.h"), 'w') as file:
		file.write(files_header["wrapper.h"].format(lbr="{", rbr="}", version=ver))
		# Normal function types
		for k in gbls:
			if k != str(Clauses()):
				file.write("\n#if " + k + "\n")
			for v in gbls[k]:
				file.write("void " + v + "(x64emu_t *emu, uintptr_t fnc);\n")
			if k != str(Clauses()):
				file.write("#endif\n")
		file.write("\n")
		# Redirects
		for k in redirects:
			if k != str(Clauses()):
				file.write("\n#if " + k + "\n")
			for vr, _ in redirects[k]:
				file.write("void " + vr + "(x64emu_t *emu, uintptr_t fnc);\n")
			if k != str(Clauses()):
				file.write("#endif\n")
		file.write(files_guard["wrapper.h"].format(lbr="{", rbr="}", version=ver))
	
	# Rewrite the *types.h files:
	for k in conventions:
		td_types[k][conventions[k].values.index('A')] = "va_list"
		td_types[k][conventions[k].values.index('V')] = "..."
	orig_val_len = {k: len(conventions[k].values) for k in conventions}
	for fn in filesspec:
		for strc in fsp_tmp[fn][1]:
			for k in conventions:
				conventions[k].values.append(strc)
				td_types[k].append(fsp_tmp[fn][1][strc][0])
		
		with open(os.path.join(root, "src", "wrapped", "generated", fn + "types.h"), 'w') as file:
			file.write(files_header["fntypes.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
			generate_typedefs(filesspec[fn][0], file)
			file.write("\n#define SUPER() ADDED_FUNCTIONS()")
			for r in filesspec[fn][0]:
				for f in filesspec[fn][0][r]:
					file.write(" \\\n\tGO({0}, {1}_t)".format(f, r))
			file.write("\n")
			file.write(files_guard["fntypes.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
		
		with open(os.path.join(root, "src", "wrapped", "generated", fn + "defs.h"), 'w') as file:
			file.write(files_header["fndefs.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
			for defined in filesspec[fn][1]:
				file.write("#define {defined} {define}\n".format(defined=defined, define=filesspec[fn][1][defined]))
			file.write(files_guard["fndefs.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
		
		with open(os.path.join(root, "src", "wrapped", "generated", fn + "undefs.h"), 'w') as file:
			file.write(files_header["fnundefs.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
			for defined in filesspec[fn][1]:
				file.write("#undef {defined}\n".format(defined=defined))
			file.write(files_guard["fnundefs.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
		
		for k in conventions:
			conventions[k].values = conventions[k].values[:orig_val_len[k]]
			td_types[k] = td_types[k][:orig_val_len[k]]
	
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
	if main(sys.argv[1], sys.argv[limit[0]+1:], "2.4.0.21") != 0:
		exit(2)
	exit(0)
