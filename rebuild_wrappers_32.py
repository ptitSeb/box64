#!/usr/bin/env python3

import os
import sys

try:
	assert(sys.version_info.major == 3)
	if sys.version_info.minor >= 9:
		# Python 3.9+
		from typing import Any, Generic, NewType, Optional, TypeVar, Union, final
		from collections.abc import Callable, Iterable, Sequence
		Dict = dict
		List = list
		Type = type
		Tuple = tuple
	elif sys.version_info.minor >= 8:
		# Python [3.8, 3.9)
		from typing import Any, Callable, Dict, Generic, Iterable, List, NewType, Optional, Sequence, Tuple, Type, TypeVar, Union, final
	elif (sys.version_info.minor >= 5) and (sys.version_info.micro >= 2):
		# Python [3.5.2, 3.8)
		from typing import Any, Callable, Dict, Generic, Iterable, List, NewType, Optional, Sequence, Tuple, Type, TypeVar, Union
		final = lambda fun: fun # type: ignore
	elif sys.version_info.minor >= 5:
		# Python [3.5, 3.5.2)
		from typing import Any, Callable, Dict, Generic, Iterable, List, Optional, Sequence, Tuple, Type, TypeVar, Union
		def NewType(_, b): return b # type: ignore
		final = lambda fun: fun # type: ignore
	else:
		# Python < 3.5
		#print("Your Python version does not have the typing module, fallback to empty 'types'")
		# Dummies
		class GTDummy:
			def __getitem__(self, _):
				return self
		Any = GTDummy() # type: ignore
		Callable = GTDummy() # type: ignore
		Dict = GTDummy() # type: ignore
		Generic = GTDummy() # type: ignore
		Iterable = GTDummy() # type: ignore
		List = GTDummy() # type: ignore
		def NewType(_, b): return b # type: ignore
		Optional = GTDummy() # type: ignore
		Sequence = GTDummy() # type: ignore
		Tuple = GTDummy() # type: ignore
		Type = GTDummy() # type: ignore
		def TypeVar(T): return object # type: ignore
		Union = GTDummy() # type: ignore
		final = lambda fun: fun # type: ignore
except ImportError:
	print("It seems your Python version is quite broken...")
	assert(False)

"""
Generates all files in src/wrapped/generated
===

TL;DR: Automagically creates type definitions (/.F.+/ functions/typedefs...).
       All '//%' in the headers are used by the script.

Reads each lines of each "_private.h" headers (plus wrappedd3dadapter9_genvate.h, derived from wrappedd3dadapter9_gen.h).
For each of them:
- If if starts with a #ifdef, #else, #ifndef, #endif, it memorizes which definition is required
- If it starts with a "GO", it will do multiple things:
  - It memorizes the type used by the function (second macro argument)
  - It memorizes the type it is mapped to, if needed (eg, iFvp is mapped to iFp: the first argument is dropped)
  - It checks if the type given (both original and mapped to) are valid
  - If the signature contains a 'E' but it is not a "GOM" command, it will throw an error* (and vice-versa)
  - If the line also contains '//%', the script will parse what's attached to this comment start:
    - If it is attached to a '%', the function will be skipped when generating the 'SUPER' macro in the *types.h
	- *If it is attached to a 'noE' or attached to something that ends with ',noE', it will ignore functions that
	  don't have the emulator as an argument but are still GOM functions
  To know more about the signatures, see also box86.org's article (TODO FOR NOW).
- If the line starts with a '//%S', it will memorize a structure declaration.
  The structure of it is: "//%S <letter> <structure name> <signature equivalent>"
  NOTE: Those structure letters are "fake types" that are accepted in the macros.

`gbl` contains the first list, `redirects` the second and
 `filespec` constains file specific informations (eg, structures, typedefs required...).

After sorting the data, it generates:

wrapper32.c
---------
(Private) type definitions (/.F.+_t/)
Function definitions (/.F.+/ functions, that actually execute the function given as argument)

wrapper32.h
---------
Generic "wrapper32_t" type definition
Function declarations (/.F._32+/ functions)

*types32.h
--------
Local types definition, for the original signatures
The SUPER() macro definition, used to generate and initialize the `*_my_t` library structure
(TODO: also automate this declaration/definition? It would require more metadata,
 and may break sometime in the future due to the system changing...)

*defs32.h
-------
Local `#define`s, for signature mapping

*undefs32.h
---------
Local `#undefine`s, for signature mapping


Example:
========
In wrappedtest_private.h:
   ----------------------
//%S X TestLibStructure ppu

GO(superfunction, pFX)
GOM(superFunction2, pFEpX)
GOM(functionWithoutE, pFppu) //%noE
GOM(functionWithoutEAndNotInTypes, pFpppu) //%%,noE
GOM(functionNotInTypes, pFEpppu) //%%

[No output]
Generated files:
wrapper32.c: [snippet]
----------
typedef void *(*pFppu_t)(void*, void*, uint32_t);
typedef void *(*pFpppu_t)(void*, void*, void*, uint32_t);
typedef void *(*pFEpppu_t)(x64emu_t*, void*, void*, void*, uint32_t);

void pFppu_32(x64emu_t *emu, uintptr_t fcn) { pFppu_t *fn = (pFppu_t)fn; R_RAX=...; }
void pFpppu_32(x64emu_t *emu, uintptr_t fcn) { pFpppu_t *fn = (pFpppu_t)fn; R_RAX=...; }
void pFEpppu_32(x64emu_t *emu, uintptr_t fcn) { pFpppu_t *fn = (pFpppu_t)fn; R_RAX=...; }

wrapper32.h: [snippet]
----------
typedef void (*wrapper_t)(x64emu_t*, uintptr_t);

void pFppu_32(x64emu_t *emu, uintptr_t fcn);
void pFpppu_32(x64emu_t *emu, uintptr_t fcn);
void pFEpppu_32(x64emu_t *emu, uintptr_t fcn);

wrappedtesttypes32.h:
-------------------
typedef void *(*pFpX_32_t)(void*, TestLibStructure);
typedef void *(*pFppu_32_t)(void*, void*, uint32_t);
typedef void *(*pFpppu_32_t)(void*, void*, void*, uint32_t);

#define SUPER() ADDED_FUNCTIONS() \\
	GO(superFunction2, pFpX) \\
	GO(functionWithoutE, pFppu)

wrappedtestdefs32.h:
------------------
#define pFX pFppu
#define pFpX pFpppu

wrappedtestundefs32.h:
--------------------
#undef pFX
#undef pFpX
"""

# Free characters:
#      FG  J      QR T   XYZ    e g  jk mno q      xyz01 3456789

T = TypeVar('T')
U = TypeVar('U')

Filename = str

class CustOrderedDict(Generic[T, U], Iterable[T]):
	__keys__: List[T]
	__actdict__: Dict[T, U]
	
	def __init__(self, src: Optional[Dict[T, U]] = None) -> None:
		if src is None:
			self.__keys__ = []
			self.__actdict__ = {}
		else:
			self.__keys__ = list(src.keys())
			self.__actdict__ = src
	
	def sort(self, key: Callable[[T], Any] = lambda x: x) -> None:
		self.__keys__.sort(key=key)
	
	def __iter__(self):
		return iter(self.__keys__)
	def __contains__(self, k: T) -> bool:
		return k in self.__actdict__
	def __getitem__(self, k: T) -> U:
		return self.__actdict__[k]
	def __setitem__(self, k: T, v: U) -> None:
		if k not in self.__keys__: self.__keys__.append(k)
		self.__actdict__[k] = v
class CustOrderedDictList(CustOrderedDict[T, List[U]]):
	def __getitem__(self, k: T) -> List[U]:
		if k not in self: self[k] = []
		return super().__getitem__(k)

class FirstArgumentSingletonMeta(Generic[T], type):
	_singletons: Dict[T, Type['FirstArgumentSingletonMeta']]
	
	@classmethod
	def __prepare__(metacls, __name: str, __bases: Tuple[type, ...], **kwds: Any) -> Dict[str, Any]:
		return { "_singletons": {} }
	
	def __contains__(cls, k):
		return k in cls._singletons
	
	def getSingletons(cls):
		return cls._singletons
	def __getitem__(cls, k):
		return cls._singletons[k]
	
	def __call__(cls, fstarg, *largs, **kwargs):
		if fstarg not in cls._singletons:
			cls._singletons[fstarg] = super().__call__(fstarg, *largs, **kwargs)
		return cls._singletons[fstarg]

DefineType = NewType('DefineType', str)
@final
class Define:
	name: DefineType
	inverted_: bool
	
	defines: List[DefineType] = []
	
	def __init__(self, name: DefineType, inverted_: bool) -> None:
		# All values for "name" are in defines (throw otherwise)
		if name not in Define.defines:
			raise KeyError(name)
		
		self.name = name
		self.inverted_ = inverted_
	def copy(self) -> "Define":
		return Define(self.name, self.inverted_)
	
	def value(self) -> int:
		return Define.defines.index(self.name)*2 + (1 if self.inverted_ else 0)
	
	def invert(self) -> "Define":
		"""
		invert -- Transform a `defined()` into a `!defined()` and vice-versa, in place.
		"""
		self.inverted_ = not self.inverted_
		return self
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
	def __eq__(self, o) -> bool:
		return isinstance(o, Define) and (self.name == o.name) and (self.inverted_ == o.inverted_)
@final
class Clause:
	defines: List[Define]
	
	def __init__(self, defines: Union[List[Define], str] = []) -> None:
		if isinstance(defines, str):
			if defines == "":
				self.defines = []
			else:
				self.defines = list(
					map(
						lambda x:
							Define(DefineType(x[9:-1] if x[0] == '!' else x[8:-1]), x[0] == '!')
						, defines.split(" && ")
					)
				)
		else:
			self.defines = [d.copy() for d in defines]
	def copy(self) -> "Clause":
		return Clause(self.defines)
	
	def append(self, define: Define) -> "Clause":
		if any((define2.name == define.name) and (define2.inverted_ != define.inverted_) for define2 in self.defines):
			raise ValueError("Tried to append an incompatible clause")
		
		self.defines.append(define)
		return self
	def invert_last(self) -> "Clause":
		self.defines[-1].invert()
		return self
	def pop_last(self) -> "Clause":
		if len(self.defines) > 0: self.defines.pop()
		return self
	
	def empty(self) -> bool:
		return self.defines == []
	
	def __str__(self) -> str:
		return " && ".join(map(str, self.defines))
	def __hash__(self):
		return hash(str(self))
	def __eq__(self, o) -> bool:
		return isinstance(o, Clause) and (self.defines == o.defines)
ClausesStr = str
@final
class Clauses:
	"""
	Represent a list of clauses, aka a list of or-ed together and-ed "defined()"
	conditions
	"""
	clauses: List[Clause]
	
	def __init__(self, clauses: Union[List[Clause], str] = []) -> None:
		if isinstance(clauses, str):
			if clauses == "()":
				self.clauses = []
			elif ") || (" in clauses:
				self.clauses = list(map(Clause, clauses[1:-1].split(") || (")))
			else:
				self.clauses = [Clause(clauses)]
		else:
			self.clauses = clauses[:]
	def copy(self) -> "Clauses":
		return Clauses(self.clauses[:])
	
	def add(self, defines: Clause) -> "Clauses":
		self.clauses.append(defines)
		return self
	
	def empty(self) -> bool:
		return self.clauses == []
	
	def splitdef(self) -> Sequence[int]:
		"""
		splitdef -- Sorting key function for #ifdefs
		
		All #if defined(...) are sorted first by the length of its string
		representation, then by the number of clauses, then by the number of
		'&&' in each clause and then by the "key" of the tested names (left to
		right, inverted placed after non-inverted).
		"""
		
		ret = [len(str(self)), len(self.clauses)] if len(self.clauses) > 0 else [-1]
		for cunj in self.clauses:
			ret.append(len(cunj.defines))
		for cunj in self.clauses:
			for d in cunj.defines:
				ret.append(d.value())
		return ret
	
	def reduce(self) -> None:
		"""
		reduce -- Reduces the number of clauses in-place
		
		Removes the most possible number of conditions, both by removing
		conditions and by removing entire clauses.
		
		As a side effect, sorts itself.
		"""
		# Early breaks
		if any(c.empty() for c in self.clauses):
			self.clauses = []
			return
		if len(self.clauses) == 0:
			return
		elif len(self.clauses) == 1:
			clause = Clause()
			for define in self.clauses[0].defines:
				if define in clause.defines:
					continue
				elif define.inverted() in clause.defines:
					clause = Clause(',') # This should never happen (and never happens without breaking encapsulation)
				else:
					clause.append(define)
			clause.defines.sort(key=lambda d: Define.defines.index(d.name))
			self.clauses = [clause]
			return
		elif len(self.clauses) == 2:
			if len(self.clauses[0].defines) == len(self.clauses[1].defines) == 1:
				if self.clauses[0].defines[0].inverted() == self.clauses[1].defines[0]:
					self.clauses = []
					return
		
		# Quine-McCluskey algorithm
		# matches: list of (matches, inverted_mask)
		needed: List[Tuple[int, int]] = [
			(i, 0)
			for i in range(1<<len(Define.defines))
			if any( # i matches any clause
				all( # i matches all conditions in the clause
					(i & (1<<Define.defines.index(define.name)) == 0) == define.inverted_
					for define in clause.defines)
				for clause in self.clauses)
		]
		
		last_combined = needed[:]
		uncombinable: List[Tuple[int, int]] = []
		while len(last_combined) > 0:
			combined: List[Tuple[int, int]] = []
			combinable: List[bool] = [False] * len(last_combined)
			while len(last_combined) > 0:
				attempt = last_combined[-1]
				for idx, (i, m) in enumerate(last_combined):
					if idx == len(last_combined) - 1:
						if not combinable[idx]:
							uncombinable.append(attempt)
					elif m == attempt[1]:
						if (i ^ attempt[0]) & ((i ^ attempt[0]) - 1) != 0:
							continue # More than 1 bit of difference
						
						combinable[idx] = True
						combinable[len(last_combined) - 1] = True
						add = (i | attempt[0], m | (i ^ attempt[0]))
						if add in combined:
							continue # Aleady added
						combined.append(add)
				last_combined.pop()
			last_combined = combined
		
		matches: Dict[int, List[Tuple[int, int]]] = {
			i: [combination for combination in uncombinable if (i | combination[1]) == combination[0]] for i, _ in needed
		}
		self.clauses = []
		matches_size: int = 1
		while len(matches) != 0:
			match_found = True
			while match_found:
				match_found = False
				for i in matches:
					if len(matches[i]) < matches_size:
						raise NotImplementedError("There seems to be an error in the algorithm")
					elif len(matches[i]) == matches_size:
						match_found = True
						self.clauses.append(
							Clause([
								Define(
									n,
									matches[i][0][0] & (1 << j) == 0
								) for j, n in enumerate(Define.defines) if matches[i][0][1] & (1 << j) == 0
							]))
						self.clauses[-1].defines.sort(key=lambda d: Define.defines.index(d.name))
						to_erase: List[int] = []
						for j in matches:
							if matches[i][0] in matches[j]:
								to_erase.append(j)
						for j in to_erase:
							del matches[j]
						break
			matches_size = matches_size + 1
		self.clauses.sort(key=lambda c: (len(c.defines), [Define.defines.index(d.name) for d in c.defines]))
	
	def __str__(self) -> ClausesStr:
		if len(self.clauses) == 1:
			return str(self.clauses[0])
		else:
			return "(" + ") || (".join(map(str, self.clauses)) + ")"
	def __hash__(self):
		return hash(str(self))
	def __eq__(self, o) -> bool:
		return isinstance(o, Clauses) and (self.clauses == o.clauses)

class CType(metaclass=FirstArgumentSingletonMeta):
	class ReadWrite:
		none: 'CType.ReadWrite'
		readonly: 'CType.ReadWrite'
		writeonly: 'CType.ReadWrite'
		readwrite: 'CType.ReadWrite'
		def __init__(self, pre: bool, post: bool) -> None:
			# pre is "need to convert before the call", post is "need to convert after the call"
			self.pre = pre
			self.post = post
	
	@staticmethod
	def find_next(name: str) -> Tuple[str, Optional[Tuple['CType.ReadWrite', str, str]]]:
		"""
		Returns (pre, (type, str, post))
		 where name == pre.('r' if type is readonly, 'b' if type is writeonly else 'B').str.'_'.post
		Aka, pre contains no structure; type, str is the structure characterization (b fo writeonly).
		If it returns (pre, None), it is guaranteed to have no structure in pre === name.
		"""
		beg: Optional[int] = None
		t: Optional[CType.ReadWrite] = None
		depth = 0
		
		for i in range(len(name)):
			if (name[i] == 'r') or (name[i] == 'b') or (name[i] == 'B'):
				if beg is None:
					beg = i
					t = CType.ReadWrite.readonly  if name[i] == 'r' else \
					    CType.ReadWrite.writeonly if name[i] == 'B' else CType.ReadWrite.readwrite
				depth = depth + 1
			elif name[i] == '_':
				if depth == 0:
					raise ValueError(f"Invalid type {name}")
				elif depth == 1:
					assert beg is not None, "Unreachable"
					assert t is not None, "Unreachable"
					return name[:beg], (t, name[beg+1:i], name[i+1:])
				depth = depth - 1
		
		return name, None
	
	def __init__(self, name_t: Tuple[str, 'CType.ReadWrite'], clause: Clause, filespec: 'FileSpec') -> None:
		self.name = name_t[0]
		self.type = name_t[1]
		if self.type is CType.ReadWrite.none:
			self.structname = self.name
			self.structname2 = self.name
		elif self.type is CType.ReadWrite.readonly:
			self.structname = "r" + self.name + "_"
			self.structname2 = "struct_" + self.name
		elif self.type is CType.ReadWrite.writeonly:
			self.structname = "B" + self.name + "_"
			self.structname2 = "struct_" + self.name
		elif self.type is CType.ReadWrite.readwrite:
			self.structname = "b" + self.name + "_"
			self.structname2 = "struct_" + self.name
		else:
			self.structname = "//" + self.name
			self.structname2 = "//" + self.name
		self.recursive: List[CType] = []
		self.replaced = self.name
		
		if len(name_t[0]) != 1:
			replaced = []
			pre, tmp = CType.find_next(self.name)
			while tmp is not None:
				self.recursive.extend(CType((c, CType.ReadWrite.none), clause, filespec) for c in pre)
				self.recursive.append(CType((tmp[1], tmp[0]), clause, filespec))
				replaced.append(pre)
				pre, tmp = CType.find_next(tmp[2])
			self.recursive.extend(CType((c, CType.ReadWrite.none), clause, filespec) for c in pre)
			replaced.append(pre)
			self.replaced = 'B'.join(replaced)
		
		self.asret: Optional[str] = None
		self.aspre: Optional[str] = None
		self.asarg: Optional[str] = None
		self.aspost: Optional[str] = None
	
	def describe(self, spacer=""):
		nl = "" if len(self.recursive) == 0 else f"\n{spacer}+-> "
		if self.type is CType.ReadWrite.none:
			t = ""
		elif self.type is CType.ReadWrite.readonly:
			t = " (r-)"
		elif self.type is CType.ReadWrite.writeonly:
			t = " (-w)"
		elif self.type is CType.ReadWrite.readwrite:
			t = " (rw)"
		else:
			t = " ?!?!"
		return f"{self.structname} => {self.replaced}+{len(self.recursive)}{t}{nl}" + \
			f"\n{spacer}+-> ".join(r.describe(spacer + "    ") for r in self.recursive)
	
	def generate_converters(self) -> None:
		if self.asret is not None:
			return # Already done: probably a base type
		
		self.asret = "\n#error TODO? Cannot return custom structure\n"
		self.aspre = f"struct_{self.name}_t arg_{{p}}={{{{0}}}}; " + ("" if not self.type.pre else \
			f"if (*(ptr_t*)(from_ptr((R_ESP + {{p}})))) from_{self.structname2}(&arg_{{p}}, *(ptr_t*)(from_ptr((R_ESP + {{p}})))); ")
		self.asarg = "*(ptr_t*)(from_ptr((R_ESP + {p}))) ? &arg_{p} : NULL, "
		self.aspost = "" if not self.type.post else \
			(" if (*(ptr_t*)(from_ptr((R_ESP + {p})))) to_" + self.structname2 + \
				"(*(ptr_t*)(from_ptr((R_ESP + {p}))), &arg_{p});")
		
		for rec in self.recursive:
			rec.generate_converters()
class CTypeNone(CType, metaclass=FirstArgumentSingletonMeta):
	def __init__(self, name: str, clause: Clause, filespec: 'FileSpec') -> None:
		super().__init__((name, CType.ReadWrite.none), clause, filespec)

CType.ReadWrite.none = CType.ReadWrite(False, False) # Uppermost type
CType.ReadWrite.readonly = CType.ReadWrite(True, False)
CType.ReadWrite.writeonly = CType.ReadWrite(False, True)
CType.ReadWrite.readwrite = CType.ReadWrite(True, True)

class FileSpec:
	class Struct:
		def __init__(self, name: str, repl: str) -> None:
			self.name = name
			self.repl = repl
	
	# CONSTANT- values: original set
	# CONSTANT- rvalues: valid replacement values (outside of structures)
	# CONSTANT- validrepl: valid replacement values (for structures)
	#           structs: structure ids and additional data
	values:    Sequence[str] = ['E', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'h', 'H', 'a', 'A', 'V', 'O', 'S', '2', 'P', 'N', 'M', 's', 'r', 'b', 'B', '_', 't', 'X', 'n']
	rvalues:   Sequence[str] = ['E', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'h', 'H', 'a', 'A', 'V', 'O', 'S', '2', 'P', 'N', 'M', 's', 'r', 'b', 'B', '_', 't', 'X', 'n']
	validrepl: Sequence[str] = ['c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'h', 'H', 'a', 'A', 'V', 'O', 'S', '2', 'P', 'N', 'M', 's', 'r', 'b', 'B', '_']
	
	def __init__(self) -> None:
		self.structs: CustOrderedDict[str, FileSpec.Struct] = CustOrderedDict()
		
		self.typedefs: CustOrderedDictList[_BareFunctionType, Function] = CustOrderedDictList()
		self.structsuses: List[FunctionType] = []
	
	def registerStruct(self, id: str, name: str, repl: str) -> None:
		if len(id) != 1:
			# If you REALLY need it, consider opening a ticket
			# Before you do, consider that everything that is a valid in a C token is valid here too
			raise ValueError("Type ID \"" + id + "\" is too long!")
		if id in self.rvalues:
			raise ValueError("Type " + id + " is already reserved!")
		if id in self.structs:
			raise ValueError("Type " + id + " is already used!")
		if any(c not in self.validrepl for c in repl):
			raise ValueError("Invalid structure replacement value \"" + repl + "\" (note: recursive replacements are not supported)")
		if repl == "":
			# If you need this, please open an issue (also, this is never actually called, empty strings are removed at the calling site)
			raise NotImplementedError("Invalid structure metadata supply (empty replacement)")
		
		self.structs[id] = FileSpec.Struct(name, repl)

class FunctionType(metaclass=FirstArgumentSingletonMeta):
	def __new__(cls, string: str, clause: Clause, filespec: FileSpec) -> 'FunctionType':
		if ((string[0] not in FileSpec.values) or any(c not in FileSpec.values for c in string[2:])) \
		 and ((string[0] in FileSpec.values) or (string[0] in filespec.structs)) \
		 and all((c != 'v') and (c in FileSpec.values) or (c in filespec.structs) for c in string[2:]):
			return super().__new__(StructFunctionType)
		else:
			return super().__new__(cls)
	
	def __init__(self, string: str, clause: Clause, filespec: FileSpec) -> None:
		# Early fail
		if 'VV' in string:
			raise ValueError("'V' can only be at the end of the type (use 's' instead)")
		
		self.orig = CTypeNone(string, clause, filespec)
		
		self.hasemu = 'E' in self.orig.replaced and (
			(self.orig.recursive[1].structname != 'E') or ('E' in self.orig.recursive[0].name) or any('E' in ct.name for ct in self.orig.recursive[2:]))
		if self.hasemu:
			if ("E" in self.orig.recursive[0].name) or any("E" in ct.name for ct in self.orig.recursive[3:]):
				raise NotImplementedError("x64emu_t* not as the first parameter")
			if len(self.orig.replaced) < 4:
				raise NotImplementedError("Type {0} too short".format(self.orig.replaced))
			chk_type = self.orig.recursive[0].structname + ''.join(map(lambda ct: ct.structname, self.orig.recursive[3:]))
		else:
			if len(self.orig.replaced) < 3:
				raise NotImplementedError("Type {0} too short".format(self.orig.replaced))
			chk_type = self.orig.recursive[0].structname + ''.join(map(lambda ct: ct.structname, self.orig.recursive[2:]))
		self.withoutE = _BareFunctionType(string[0:2] + chk_type[1:], clause, filespec, isinstance(self, StructFunctionType))
		self._bare = _BareFunctionType(self.orig.name, clause, filespec, isinstance(self, StructFunctionType))
		if len(chk_type) < 2:
			raise NotImplementedError("Type {0} too short".format(string))
		
		if self.orig.recursive[1].structname not in ['E', 'F']:
			raise NotImplementedError("Bad middle letter {0}".format(self.orig.recursive[1].structname))
		
		self.redirect = any(c not in FileSpec.values for c in chk_type) or (('v' in chk_type[1:]) and (len(chk_type) > 2))
		self.usestruct: bool = False
		self.redirected: Optional[FunctionType] = None
		if self.redirect:
			if all(((i == 0) or (c != 'v')) and (c in FileSpec.values) or (c in filespec.structs) for i, c in enumerate(chk_type)):
				# 'v' is never allowed here
				self.redirect = False
				self.usestruct = True
				return
			else:
				if any(c not in FileSpec.rvalues for c in chk_type):
					raise NotImplementedError("Invalid type {0}".format(string))
				
				# Ok, this is acceptable: there is void
				string = string[:2] + (string[2:]
					.replace("v", "")) # void -> nothing
				assert(len(string) >= 3) # If this raises, don't use 'vFvvvvvv' as a signature...
				self.redirected = FunctionType(string, clause, filespec)
				assert(not self.redirected.redirect and not self.redirected.usestruct)
	
	def getchar(self, c: str) -> int:
		return self._bare.getchar(c)
	def getcharidx(self, i: int) -> int:
		return self._bare.getcharidx(i)
	def splitchar(self) -> List[int]:
		return self._bare.splitchar()
	
	def __hash__(self) -> int:
		return str.__hash__(self.orig.name)
	def __eq__(self, o: object):
		return isinstance(o, FunctionType) and ((self.orig.name == o.orig.name) and (o is self or not isinstance(self, StructFunctionType)))
class StructFunctionType(FunctionType):
	def __init__(self, string: str, clause: Clause, filespec: FileSpec) -> None:
		super().__init__(string, clause, filespec)
		assert(self.usestruct)
		self.filespec = filespec
		self.filespec.structsuses.append(self)
		
		self.returnsstruct = string[0] in self.filespec.structs
		if self.returnsstruct:
			if self.hasemu:
				string = "pFEp" + string[3:]
			else:
				string = "pFp" + string[2:]
		
		for struct in self.filespec.structs:
			string = string.replace(struct, self.filespec.structs[struct].repl)
		self.redirected = FunctionType(string, clause, self.filespec)

class _BareFunctionType(FunctionType): # Fake derived
	def __new__(cls, *largs, **kwargs):
		return object.__new__(cls)
	def __init__(self, string: str, clause: Clause, filespec: FileSpec, isstruct: bool) -> None:
		self.orig = CTypeNone(string, clause, filespec)
		self.filespec = filespec
		self.isstruct = isstruct
	
	def getchar(self, c: str) -> int:
		if c in FileSpec.rvalues:
			return FileSpec.rvalues.index(c)
		else:
			assert(self.isstruct)
			return self.filespec.structs.__keys__.index(c) + len(FileSpec.rvalues)
	def getcharidx(self, i: int) -> int:
		return self.getchar(self.orig.replaced[i])
	
	def splitchar(self) -> List[int]:
		try:
			ret = [
				len(self.orig.replaced), len(self.orig.name), self.getcharidx(0),
				*map(self.getcharidx, range(2, len(self.orig.replaced)))
			]
			return ret
		except ValueError as e:
			raise ValueError("Value is " + self.orig.replaced + ":\n" + self.orig.describe()) from e
		except AssertionError as e:
			raise ValueError("Value is " + self.orig.replaced + ":\n" + self.orig.describe()) from e

# Allowed GOs: GO,GOM,GO2,GOS,GOW,GOWM,GOW2,GO2S
class Function:
	def __init__(self, name: str, funtype: FunctionType, gotype: str, filespec: FileSpec, filename: Filename, line: str) -> None:
		self._noE = False
		
		self.no_dlsym: bool = False
		if "//%" in line:
			additional_meta = line.split("//%")[1].split(" ")[0].strip()
			
			if additional_meta.endswith(",noE"):
				self._noE = True
				additional_meta = additional_meta[:-4]
			
			if additional_meta == 'noE':
				assert not self._noE, "Duplicated 'noE'"
				self._noE = True
			elif additional_meta == '%':
				self.no_dlsym = True
			else:
				raise NotImplementedError("Changing the function type 'on the fly' is not supported")
		
		funtypeerr = ValueError("Invalid function type " + gotype)
		if not gotype.startswith("GO"):
			raise funtypeerr
		gotype = gotype[2:]
		self.isweak = (len(gotype) > 0) and (gotype[0] == "W")
		if self.isweak:
			gotype = gotype[1:]
		self.ismy = (len(gotype) > 0) and (gotype[0] == "M")
		self.is2 = (len(gotype) > 0) and (gotype[0] == "2")
		self.retS = (len(gotype) > 0) and (gotype[0] == "S")
		if self.ismy or self.is2 or self.retS:
			gotype = gotype[1:]
		if self.retS:
			self.ismy = True
			assert((self.no_dlsym and (funtype.orig.name.startswith("pFp") or funtype.orig.name.startswith("pFEp") or funtype.orig.name.startswith("pEp") or funtype.orig.name.startswith("pEEp")))
			 or (isinstance(funtype, StructFunctionType) and funtype.returnsstruct)), \
				"Maybe TODO? (Returns unregistered structure)"
			self._noE = self._noE or self.no_dlsym
		if isinstance(funtype, StructFunctionType) and funtype.returnsstruct and not self.retS:
			gotype = "GO" + \
				("W" if self.isweak else "") + \
				("M" if self.ismy else "") + ("2" if self.is2 else "")
			raise ValueError("Function " + name + " of type " + funtype.orig.name + \
				" needs to return a structure, but doesn't (currently " + gotype + ")")
		if gotype != "":
			raise funtypeerr
		
		self.name = name
		self.type = funtype
		self.filespec = filespec
		assert(not isinstance(funtype, StructFunctionType) or filespec is funtype.filespec) # No reason why not, so assert()
		
		if self.is2:
			self.fun2 = line.split(',')[2].split(')')[0].strip()
			if (self.type.hasemu != self.fun2.startswith("my32_") and self.type.hasemu != self.fun2.startswith("my_")) and not self._noE:
				# If this raises because of a different prefix, open a pull request
				print("\033[91mThis is probably not what you meant!\033[m ({0}:{1})".format(filename, line[:-1]), file=sys.stderr)
				self.invalid = True
		
		if (self.ismy and not self.type.hasemu and not self.is2) and not self._noE:
			# Probably invalid on box86; if not so, remove/comment this whole 'if' (and also open an issue)
			print("\033[94mAre you sure of this?\033[m ({0}:{1})".format(filename, line[:-1]), file=sys.stderr)
			self.invalid = True
			return
		if self.type.hasemu and not self.ismy and not self.is2:
			# Certified invalid
			print("\033[91mThis is probably not what you meant!\033[m ({0}:{1})".format(filename, line[:-1]), file=sys.stderr)
			self.invalid = True
			return
		if self._noE and not self.ismy and not self.is2:
			raise ValueError("Invalid meta: 'no E' provided but function is not a GOM")
		
		if self.ismy or self.is2:
			# Add this to the typedefs
			self.filespec.typedefs[self.type.withoutE].append(self)

JumbledFunctions = CustOrderedDictList[Clause, Function]
FilesSpecific    = Dict[Filename, FileSpec]

SortedGlobals    = CustOrderedDictList[Clauses, FunctionType]
SortedRedirects  = CustOrderedDictList[Clauses, FunctionType]

def readFiles(files: Iterable[str]) -> Tuple[JumbledFunctions, JumbledFunctions, FilesSpecific]:
	"""
	readFiles
	
	This function is the one that parses the files.
	"""
	
	gbls:      JumbledFunctions = CustOrderedDictList()
	redirects: JumbledFunctions = CustOrderedDictList()
	filespecs: FilesSpecific    = {}
	
	symbols: Dict[str, Filename] = {}
	need_halt: bool = False
	
	for filepath in files:
		filename: Filename = filepath.split("/")[-1]
		dependants: Clause = Clause()
		
		filespec = FileSpec()
		filespecs[filename[:-10]] = filespec
		
		def add_symbol_name(symname: Optional[str], weak: bool = False, symsname: Dict[str, List[Tuple[str, bool]]] = {"": []}):
			# Optional arguments are evaluated only once!
			nonlocal need_halt
			if symname is None:
				for c in symsname:
					if (c != "") and (len(symsname[c]) != 0):
						# Note: if this condition ever raises, check the wrapper pointed by it.
						# If you find no problem, comment the error below, add a "pass" below (so python is happy)
						# and open a ticket so I can fix this.
						raise NotImplementedError("Some symbols are only implemented under one condition '{0}' (probably) ({1}/{2})"
								.format(c, symsname[c][0][0], filename) + " [extra note in the script]")
					for s, w in symsname[c]:
						if w: continue # Weak symbols never conflict with others in different libraries
						
						if s in (
						  '_init', '_fini',
						  '__bss_start', '__bss_start__', '__bss_end__', '_bss_end__',
						  '__data_start', '_edata',
						  '_end', '__end__'):
							continue # Always allow those symbols [TODO: check if OK]
						if s in symbols:
							# Check for resemblances between symbols[s] and filename
							# if filename.startswith(symbols[s][:-12]) or symbols[s].startswith(filename[:-12]):
							# 	# Probably OK
							# 	continue
							# Manual incompatible libs detection
							match = lambda l, r: (filename[7:-10], symbols[s][7:-10]) in [(l, r), (r, l)]
							if  match("gdkx112",     "gdk3")        \
							 or match("gtkx112",     "gtk3")        \
							 or match("libjpeg",     "libjpeg62")   \
							 or match("libncurses",  "libncurses6") \
							 or match("libncurses",  "libncursesw") \
							 or match("libncurses6", "libncursesw") \
							 or match("libtinfo6",   "libtinfo")    \
							 or match("png12",       "png16")       \
							 or match("sdl1",        "sdl2")        \
							 or match("sdl1image",   "sdl2image")   \
							 or match("sdl1mixer",   "sdl2mixer")   \
							 or match("sdl1net",     "sdl2net")     \
							 or match("sdl1ttf",     "sdl2ttf")     \
							 or match("smpeg",       "smpeg2")      \
							 or match("udev0",       "udev1")       \
							 or match("gstinterfaces010","gstvideo")\
							 or match("gstinterfaces010","gstaudio")\
							 or match("gstreamer010","gstreamer")	\
							 or match("appindicator","appindicator3")\
							 or match("libgl",        "libegl")     \
							 or match("libgl",        "libglx")     \
							 or match("libgl",        "libglxnvidia")\
							 \
							 or match("libc",        "tcmallocminimal") \
							 or match("libc",        "ldlinux") 	\
							:
								# libc and ldlinux have some "__libc_" data symbols in common... TODO check if ok
								continue
							
							# Note: this test is very (too) simple. If it ever raises, comment
							# `need_halt = True` and open an issue.
							print("The symbol {0} is declared in multiple files ({1}/{2})"
								.format(s, symbols[s], filename) + " [extra note in the script]", file=sys.stderr)
							need_halt = True
						symbols[s] = filename
			else:
				symname = symname.strip()
				if symname == "":
					raise ValueError("This symbol name (\"\") is suspicious... ({0})".format(filename))
				
				l = len(dependants.defines)
				already_pst = any(s == symname for s, _ in symsname[""])
				if l == 1:
					symsname.setdefault(str(dependants), [])
					already_pst = already_pst or any(s == symname for s, _ in symsname[str(dependants)])
				if already_pst:
					print("The symbol {0} is duplicated! ({1})".format(symname, filename), file=sys.stderr)
					need_halt = True
					return
				if l == 1:
					s = str(dependants.defines[0].inverted())
					if (s in symsname) and ((symname, weak) in symsname[s]):
						symsname[s].remove((symname, weak))
						symsname[""].append((symname, weak))
					elif (s in symsname) and ((symname, not weak) in symsname[s]):
						print("The symbol {0} doesn't have the same 'weakness' in different conditions! ({1})"
							.format(symname, filename), file=sys.stderr)
						need_halt = True
					else:
						symsname[str(dependants)].append((symname, weak))
				elif l == 0:
					symsname[""].append((symname, weak))
		
		with open(filepath, 'r') as file:
			for line in file:
				ln = line.strip()
				
				try:
					# If the line is a `#' line (#ifdef LD80BITS/#ifndef LD80BITS/header)
					if ln.startswith("#"):
						preproc_cmd = ln[1:].strip()
						if preproc_cmd.startswith("if defined(GO)"):
							continue #if defined(GO) && defined(GOM)...
						elif preproc_cmd.startswith("if !(defined(GO)"):
							continue #if !(defined(GO) && defined(GOM)...)
						elif preproc_cmd.startswith("error"):
							continue #error meh!
						elif preproc_cmd.startswith("include"):
							continue #inherit other library
						elif preproc_cmd.startswith("endif"):
							dependants.pop_last()
						elif preproc_cmd.startswith("ifdef"):
							dependants.append(Define(DefineType(preproc_cmd[5:].strip()), False))
						elif preproc_cmd.startswith("ifndef"):
							dependants.append(Define(DefineType(preproc_cmd[6:].strip()), True))
						elif preproc_cmd.startswith("else"):
							dependants.invert_last()
						else:
							raise NotImplementedError("Unknown preprocessor directive: {0}".format(preproc_cmd.split(" ")[0]))
					
					# If the line is a `GO...' line (GO/GOM/GO2/...)...
					elif ln.startswith("GO"):
						# ... then look at the second parameter of the line
						try:
							gotype = ln.split("(")[0].strip()
							funname = ln.split(",")[0].split("(")[1].strip()
							ln = ln.split(",")[1].split(")")[0].strip()
						except IndexError:
							raise NotImplementedError("Invalid GO command")
						
						fun = Function(funname, FunctionType(ln, dependants, filespec), gotype, filespec, filename, line)
						if not filename.endswith("_genvate.h"):
							add_symbol_name(fun.name, fun.isweak)
						
						if hasattr(fun, 'invalid'):
							need_halt = True
							continue
						
						if fun.type.redirect or fun.type.usestruct:
							redirects[dependants.copy()].append(fun)
						else:
							gbls[dependants.copy()].append(fun)
					
					# If the line is a structure metadata information...
					elif ln.startswith("//%S"):
						metadata = [e for e in ln.split() if e]
						if len(metadata) != 4:
							# If you need an empty replacement, please open a PR
							raise NotImplementedError("Invalid structure metadata supply (too many/not enough fields)")
						if metadata[0] != "//%S":
							raise NotImplementedError("Invalid structure metadata supply (invalid signature)")
						
						filespec.registerStruct(metadata[1], metadata[2], metadata[3])
					
					# If the line contains any symbol name...
					elif ("GO" in ln) or ("DATA" in ln):
						if filename.endswith("_genvate.h"):
							continue
						# Probably "//GO(..., " or "DATA(...," at least
						try:
							symname = ln.split('(')[1].split(',')[0].strip()
							add_symbol_name(symname)
						except IndexError:
							# Oops, it wasn't...
							pass
				except Exception as e:
					raise NotImplementedError("{0}:{1}".format(filename, line[:-1])) from e
		
		if filename.endswith("_genvate.h"):
			del filespecs[filename[:-10]]
		
		add_symbol_name(None)
		FunctionType.getSingletons().clear()
	
	if need_halt:
		raise ValueError("Fix all previous errors before proceeding")
	
	return gbls, redirects, filespecs

def sortArrays(gbl_funcs: JumbledFunctions, red_funcs: JumbledFunctions, filespecs: FilesSpecific) \
 -> Tuple[SortedGlobals, SortedRedirects]:
	# First sort file specific stuff
	for fn in filespecs:
		filespecs[fn].typedefs.sort(key=_BareFunctionType.splitchar)
		for funtype in filespecs[fn].typedefs:
			filespecs[fn].typedefs[funtype].sort(key=lambda f: f.name)
		
		filespecs[fn].structs.sort()
		filespecs[fn].structsuses.sort(key=FunctionType.splitchar)
	
	# Now, take all function types, and make a new table gbl_vals
	# This table contains all #if conditions for when a function type needs to
	# be generated.
	def add_to_vals(vals: Dict[FunctionType, Clauses], t: FunctionType, clause: Clause) -> None:
		vals.setdefault(t, Clauses())
		if clause in vals[t].clauses: return
		vals[t].add(clause)
	
	gbl_vals: Dict[FunctionType, Clauses] = {}
	for clause in gbl_funcs:
		for f in gbl_funcs[clause]:
			add_to_vals(gbl_vals, f.type, clause)
	for clause in red_funcs:
		for f in red_funcs[clause]:
			assert(f.type.redirected is not None)
			add_to_vals(gbl_vals, f.type.redirected, clause)
	
	# Remove duplicate/useless conditions (and sort)
	for t in gbl_vals:
		gbl_vals[t].reduce()
	
	# Now create a new gbls
	# gbls will contain the final version of gbls (without duplicates, based on
	# gbl_vals), meaning, a dict from clauses to function types to implement
	gbls: SortedGlobals = CustOrderedDictList()
	for funtype in gbl_vals:
		gbls[gbl_vals[funtype]].append(funtype)
	# Sort the #if clauses as defined in `splitdef`
	gbls.sort(key=Clauses.splitdef)
	
	# Sort the function types as defined in `splitchar`
	for clauses in gbls:
		gbls[clauses].sort(key=FunctionType.splitchar)
	
	# This map will contain all additional function types that are "redirected"
	# to an already defined type (with some remapping).
	red_vals: Dict[FunctionType, Clauses] = {}
	for clause in red_funcs:
		for f in red_funcs[clause]:
			if isinstance(f.type, StructFunctionType): continue
			assert(f.type.redirected is not None)
			add_to_vals(red_vals, f.type, clause)
	
	# Also do the same sorting as before (it also helps keep the order
	# in the file deterministic)
	for t in red_vals:
		red_vals[t].reduce()
	
	redirects: SortedRedirects = CustOrderedDictList()
	for funtype in red_vals:
		redirects[red_vals[funtype]].append(funtype)
	redirects.sort(key=Clauses.splitdef)
	
	def fail(): assert False, "value has no redirect"
	for clauses in redirects:
		redirects[clauses].sort(key=lambda v: fail() if v.redirected is None else v.splitchar() + v.redirected.splitchar())
	
	return gbls, redirects

def checkRun(root: str, gbls: SortedGlobals, redirects: SortedRedirects, filesspec: FilesSpecific) -> Optional[str]:
	# Check if there was any new functions compared to last run
	functions_list: str = ""
	for clauses in gbls:
		for v in gbls[clauses]:
			functions_list = functions_list + "#" + str(clauses) + " " + v.orig.name + " -> " + v.orig.replaced + "\n"
	for clauses in redirects:
		for v in redirects[clauses]:
			assert(v.redirected is not None)
			functions_list = functions_list + "#" + str(clauses) + " " + v.orig.name + " -> " + v.redirected.orig.name + "\n"
	for filename in sorted(filesspec.keys()):
		functions_list = functions_list + filename + ":\n"
		for st in filesspec[filename].structs:
			struct = filesspec[filename].structs[st]
			functions_list = functions_list + \
				"% " + st + " " + struct.name + " " + struct.repl + "\n"
		for _bare in filesspec[filename].typedefs:
			functions_list = functions_list + "- " + _bare.orig.name + ":\n"
			for fn in filesspec[filename].typedefs[_bare]:
				if fn.no_dlsym: continue
				functions_list = functions_list + "  - " + fn.name + "\n"
		for funtype in filesspec[filename].structsuses:
			assert(funtype.redirected is not None)
			functions_list = functions_list + "% " + funtype.orig.name + " -> " + funtype.redirected.orig.name + "\n"
	
	# functions_list is a unique string, compare it with the last run
	try:
		last_run = ""
		with open(os.path.join(root, "src", "wrapped32", "generated", "functions_list.txt"), 'r') as file:
			last_run = file.read()
		if last_run == functions_list:
			# Mark as OK for CMake
			with open(os.path.join(root, "src", "wrapped32", "generated", "functions_list.txt"), 'w') as file:
				file.write(functions_list)
			return None
	except IOError:
		# The file does not exist yet, first run
		pass
	
	return functions_list

def generate_files(root: str, files: Iterable[str], ver: str, gbls: SortedGlobals, redirects: SortedRedirects, \
 filespecs: FilesSpecific) -> None:
	# Generate converters
	asreturns = [
		"\n#error Invalid return type: emulator\n",                           # E
		"fn({0});",                                                           # v
		"R_EAX = fn({0});",                                                   # c
		"R_EAX = fn({0});",                                                   # w
		"R_EAX = fn({0});",                                                   # i
		"ui64_t r; r.i = fn({0}); R_EAX = r.d[0]; R_EDX = r.d[1];",           # I
		"R_EAX = (unsigned char)fn({0});",                                    # C
		"R_EAX = (unsigned short)fn({0});",                                   # W
		"R_EAX = (uint32_t)fn({0});",                                         # u
		"ui64_t r; r.u = (uint64_t)fn({0}); R_EAX = r.d[0]; R_EDX = r.d[1];", # U
		"float fl = fn({0}); fpu_do_push(emu); ST0val = fl;",                 # f
		"double db = fn({0}); fpu_do_push(emu); ST0val = db;",                # d
		"long double ld = fn({0}); fpu_do_push(emu); ST0val = ld;",           # D
		"double db = fn({0}); fpu_do_push(emu); ST0val = db;",                # K
		"R_EAX = to_long(fn({0}));",                                          # l
		"R_EAX = to_ulong(fn({0}));",                                         # L
		"R_EAX = to_ptrv(fn({0}));",                                          # p
		"R_EAX = to_hash(fn({0}));",                                          # h
		"R_EAX = to_hash_d(fn({0}));",                                        # H
		"R_EAX = to_locale(fn({0}));",                                        # a
		"R_EAX = to_locale_d(fn({0}));",                                      # A
		"\n#error Invalid return type: va_list\n",                            # V
		"\n#error Invalid return type: at_flags\n",                           # O
		"R_EAX = to_ptrv(io_convert_from(fn({0})));",                         # S
		"\n#error Invalid return type: _2uint_struct\n",                      # 2
		"\n#error Invalid return type: Vulkan Struct\n",                      # P
		"\n#error Invalid return type: ... with 1 arg\n",                     # N
		"\n#error Invalid return type: ... with 2 args\n",                    # M
		"\n#error Invalid return type: address on the stack\n",               # s
		"\n#error Invalid return type: ro structure declaration\n",           # r
		"\n#error Invalid return type: rw structure declaration\n",           # b
		"\n#error Invalid return type: wo structure declaration\n",           # B
		"\n#error Invalid return type: end of structure declaration\n",       # _
		"R_EAX = to_cstring(fn({0}));",                                       # t
		"R_EAX = to_ptrv(addDisplay(fn({0})));",           	  			  	  # X
		"\n#error Invalid return type: xcb_connection_t*\n",       			  # n
	]
	asargs = [
		"emu, ",                                              # E
		"",                                                   # v
		"from_ptri(int8_t, R_ESP + {p}), ",                   # c
		"from_ptri(int16_t, R_ESP + {p}), ",                  # w
		"from_ptri(int32_t, R_ESP + {p}), ",                  # i
		"from_ptri(int64_t, R_ESP + {p}), ",                  # I
		"from_ptri(uint8_t, R_ESP + {p}), ",                  # C
		"from_ptri(uint16_t, R_ESP + {p}), ",                 # W
		"from_ptri(uint32_t, R_ESP + {p}), ",                 # u
		"from_ptri(uint64_t, R_ESP + {p}), ",                 # U
		"from_ptri(float, R_ESP + {p}), ",                    # f
		"from_ptri(double, R_ESP + {p}), ",                   # d
		"LD2localLD(from_ptrv(R_ESP + {p})), ",       		  # D
		"FromLD(from_ptrv(R_ESP + {p})), ",           		  # K
		"from_long(from_ptri(long_t, R_ESP + {p})), ",        # l
		"from_ulong(from_ptri(ulong_t, R_ESP + {p})), ",      # L
		"from_ptriv(R_ESP + {p}), ",      					  # p
		"from_hash(from_ptri(ptr_t, R_ESP + {p})), ",         # h
		"from_hash_d(from_ptri(ptr_t, R_ESP + {p})), ",       # H
		"from_locale(from_ptri(ptr_t, R_ESP + {p})), ",       # a
		"from_locale_d(from_ptri(ptr_t, R_ESP + {p})), ",     # A
		"from_ptrv(R_ESP + {p}), ",                   		  # V
		"of_convert32(from_ptri(int32_t, R_ESP + {p})), ",    # O
		"io_convert32(from_ptriv(R_ESP + {p})), ",     		  # S
		"(_2uint_struct_t){{from_ptri(uint32_t, R_ESP + {p}),from_ptri(uint32_t, R_ESP + {p} + 4)}}, ", # 2
		"arg_{p}, ",                                          # P
		"from_ptriv(R_ESP + {p}), ",                 		  # N
		"from_ptriv(R_ESP + {p}),from_ptriv(R_ESP + {p} + 4), ", # M
		"from_ptrv(R_ESP + {p}), ",                   		  # s
		"\n#error Invalid argument type: ro structure declaration\n",     # r
		"\n#error Invalid argument type: rw structure declaration\n",     # b
		"\n#error Invalid argument type: wo structure declaration\n",     # B
		"\n#error Invalid argument type: end of structure declaration\n", # _
		"\n#error Invalid argument type: maybe-high string\n",# t
		"getDisplay(from_ptriv(R_ESP + {p})), ", 		      # X
		"aligned_xcb, ", 		      						  # n
	]
	if len(FileSpec.values) != len(asreturns):
		raise NotImplementedError("len(values) = {lenval} != len(asreturns) = {lenvals}".format(lenval=len(FileSpec.values), lenvals=len(asreturns)))
	if len(FileSpec.values) != len(asargs):
		raise NotImplementedError("len(values) = {lenval} != len(asargs) = {lenarg}".format(lenval=len(FileSpec.values), lenarg=len(asargs)))
	for value, asret, asarg in zip(FileSpec.values, asreturns, asargs):
		for ctrw in (CType.ReadWrite.none, CType.ReadWrite.readonly, CType.ReadWrite.writeonly, CType.ReadWrite.readwrite):
			if (value, CType.ReadWrite.none) not in CType:
				continue # TODO: remove this and fail if one base type is missing?
			CType[(value, CType.ReadWrite.none)].asret = asret
			CType[(value, CType.ReadWrite.none)].aspre = ""
			CType[(value, CType.ReadWrite.none)].asarg = asarg
			CType[(value, CType.ReadWrite.none)].aspost = ""
			if value == 'n':
				CType[(value, CType.ReadWrite.none)].aspre = f"void *aligned_xcb = align_xcb_connection32(from_ptriv(R_ESP + {{p}})); "
				CType[(value, CType.ReadWrite.none)].aspost = f" unalign_xcb_connection32(aligned_xcb, from_ptriv(R_ESP + {{p}}));"
	for ctn in CType.getSingletons():
		CType[ctn].generate_converters()
	
	# Detect functions which return in an x87 register
	return_x87: str = "DKdf"
	if any(c not in FileSpec.values for c in return_x87):
		raise NotImplementedError("Invalid character")
	
	# Files header and guard
	files_header = {
		"wrapper32.c": """
		#include <errno.h>
		#include <stdio.h>
		#include <stdlib.h>
		#include <stdint.h>
		
		#include "wrapper32.h"
		#include "emu/x64emu_private.h"
		#include "emu/x87emu_private.h"
		#include "regs.h"
		#include "x64emu.h"
		#include "box32.h"
		#include "converter32.h"
		
		typedef union ui64_s {lbr}
		    int64_t     i;
		    uint64_t    u;
		    uint32_t    d[2];
		{rbr} ui64_t;
		
		typedef struct _2uint_struct_s {lbr}
			uint32_t	a;
			uint32_t	b;
		{rbr} _2uint_struct_t;
		
		extern void* my__IO_2_1_stderr_;
		extern void* my__IO_2_1_stdin_ ;
		extern void* my__IO_2_1_stdout_;
		
		static void* io_convert32(void* v)
		{lbr}
			if(!v)
				return v;
			if(v==my__IO_2_1_stderr_)
				return stderr;
			if(v==my__IO_2_1_stdin_)
				return stdin;
			if(v==my__IO_2_1_stdout_)
				return stdout;
			return v;
		{rbr}

		static void* io_convert_from(void* v)
		{lbr}
			if(!v)
				return v;
			if(v==stderr)
				return my__IO_2_1_stderr_;
			if(v==stdin)
				return my__IO_2_1_stdin_;
			if(v==stdout)
				return my__IO_2_1_stdout_;
			return v;
		{rbr}
		
		typedef struct my_GValue_s
		{lbr}
		  int         g_type;
		  union {lbr}
		    int        v_int;
		    int64_t    v_int64;
		    uint64_t   v_uint64;
		    float      v_float;
		    double     v_double;
		    void*      v_pointer;
		  {rbr} data[2];
		{rbr} my_GValue_t;
		
		static void alignGValue(my_GValue_t* v, void* value)
		{lbr}
		    v->g_type = *(int*)value;
		    memcpy(v->data, value+4, 2*sizeof(double));
		{rbr}
		static void unalignGValue(void* value, my_GValue_t* v)
		{lbr}
		    *(int*)value = v->g_type;
		    memcpy(value+4, v->data, 2*sizeof(double));
		{rbr}
		
		void* VulkanFromx86(void* src, void** save);
		void VulkanTox86(void* src, void* save);
		
		#define ST0val ST0.d
		
		int of_convert32(int);

		void* getDisplay(void*);
		void* addDisplay(void*);
		
		void* align_xcb_connection32(void* src);
		void unalign_xcb_connection32(void* src, void* dst);

		""",
		"wrapper32.h": """
		#ifndef __WRAPPER32_H_
		#define __WRAPPER32_H_
		#include <stdint.h>
		#include <string.h>
		
		typedef struct x64emu_s x64emu_t;
		
		// the generic wrapper pointer functions
		typedef void (*wrapper_t)(x64emu_t* emu, uintptr_t fnc);
		
		// list of defined wrappers
		// E = current x64emu struct
		// v = void
		// C = unsigned byte c = char
		// W = unsigned short w = short
		// u = uint32, i = int32
		// U = uint64, I = int64
		// L = unsigned long, l = signed long (long is an int with the size of a pointer)
		// p = pointer
		// h = hash (32<->64bits)
		// H = hash (32<->64bits) that will be deleted from hashmaps
		// a = locale
		// A = locale that will be deleted from hashmaps
		// f = float, d = double, D = long double, K = fake long double
		// V = vaargs, s = address on the stack (doesn't move forward the pointer)
		// O = libc O_ flags bitfield
		// o = stdout
		// S = _IO_2_1_stdXXX_ pointer (or FILE*)
		// 2 = struct of 2 uint
		// N = ... automatically sending 1 arg
		// M = ... automatically sending 2 args
		// P = Vulkan struct pointer
		// r..._ = pointer to read-only structure
		// B..._ = pointer to write-only structure
		// b..._ = pointer to read-write structure
		// t = char* as a return value (copies to a lower address if the return address is too high)
		// X = Display*
		
		""",
		"converter32.c": """
		#include "converter32.h"
		""",
		"converter32.h": """
		#ifndef __CONVERTER32_H_
		#define __CONVERTER32_H_
		""",
		"fntypes32.h": """
		#ifndef __{filename}TYPES32_H_
		#define __{filename}TYPES32_H_
		
		#ifndef LIBNAME
		#error You should only #include this file inside a wrapped*.c file
		#endif
		#ifndef ADDED_FUNCTIONS
		#define ADDED_FUNCTIONS() 
		#endif
		
		""",
		"fndefs32.h": """
		#ifndef __{filename}DEFS32_H_
		#define __{filename}DEFS32_H_
		
		""",
		"fnundefs32.h": """
		#ifndef __{filename}UNDEFS32_H_
		#define __{filename}UNDEFS32_H_
		
		"""
	}
	files_guard = {
		"wrapper32.c": """
		""",
		"wrapper32.h": """
		#endif // __WRAPPER32_H_
		""",
		"converter32.c": """
		""",
		"converter32.h": """
		#endif // __CONVERTER32_H_
		""",
		"fntypes32.h": """
		#endif // __{filename}TYPES32_H_
		""",
		"fndefs32.h": """
		
		#endif // __{filename}DEFS32_H_
		""",
		"fnundefs32.h": """
		
		#endif // __{filename}UNDEFS32_H_
		"""
	}
	banner = "/***********************************************************" + ('*'*len(ver)) + "***\n" \
	         " * File automatically generated by rebuild_wrappers_32.py (v" + ver + ") *\n" \
	         " ***********************************************************" + ('*'*len(ver)) + "***/\n"
	trim: Callable[[str], str] = lambda string: '\n'.join(line[2:] for line in string.splitlines())[1:]
	# Yes, the for loops are inverted. This is because both dicts should have the same keys.
	for fhdr in files_guard:
		files_header[fhdr] = banner + trim(files_header[fhdr])
	for fhdr in files_header:
		files_guard[fhdr] = trim(files_guard[fhdr])
	
	# Typedefs
	#           E            v       c         w          i          I          C          W           u           U           f        d         D              K         l           L            p        h            H            a        A        V        O          S        2                  P        N      M      s        r               b               B               _               t          X       n
	tdtypes = ["x64emu_t*", "void", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "float", "double", "long double", "double", "intptr_t", "uintptr_t", "void*", "uintptr_t", "uintptr_t", "void*", "void*", "void*", "int32_t", "void*", "_2uint_struct_t", "void*", "...", "...", "void*", "\n#error _\n", "\n#error _\n", "\n#error _\n", "\n#error _\n", "char*", "void*", "void*"]
	if len(FileSpec.values) != len(tdtypes):
		raise NotImplementedError("len(values) = {lenval} != len(tdtypes) = {lentypes}".format(lenval=len(FileSpec.values), lentypes=len(tdtypes)))
	def generate_typedefs(funs: Iterable[FunctionType], file):
		for funtype in funs:
			def getstr(i: int, ct: CType) -> str:
				if ct.type != CType.ReadWrite.none:
					return ct.structname2 + "_t*"
				elif i < len(tdtypes):
					return tdtypes[i]
				else:
					# We are in a *types.h file
					assert(isinstance(funtype, _BareFunctionType) and funtype.isstruct)
					return funtype.filespec.structs[funtype.filespec.structs.__keys__[i - len(tdtypes)]].name
			if funtype.orig.name.endswith("Ev"):
				file.write("typedef " + getstr(funtype.getcharidx(0), funtype.orig.recursive[0])
				            + " (*" + funtype.orig.name + "_t)" + "("
				            + getstr(funtype.getcharidx(2), funtype.orig.recursive[2]) + ");\n")
			else:
				file.write("typedef " + getstr(funtype.getcharidx(0), funtype.orig.recursive[0])
				            + " (*" + funtype.orig.name + "_t)" + "("
				            + ', '.join(getstr(funtype.getcharidx(i), funtype.orig.recursive[i])
				                                   for i in range(2, len(funtype.orig.replaced))) + ");\n")
	
	# Wrappers
	#         E  v  c  w  i  I  C  W  u  U  f  d  D   K   l  L  p  h  H  a  A  V  O  S  2  P  N  M  s  r  b  B  _  t  X  n
	deltas = [0, 4, 4, 4, 4, 8, 4, 4, 4, 8, 4, 8, 12, 12, 4, 4, 4, 4, 4, 4, 4, 1, 4, 4, 8, 4, 0, 0, 0, 1, 1, 4, 1, 4, 4, 4]
	# Asserts
	if len(FileSpec.values) != len(deltas):
		raise NotImplementedError("len(values) = {lenval} != len(deltas) = {lendeltas}".format(lenval=len(FileSpec.values), lendeltas=len(deltas)))
	
	# Helper functions to write the function definitions
	def function_args(args: Iterable[Tuple[CType, str]], map: Callable[[CType], str], d: int = 4) -> str:
		d = 4
		ret = ""
		for arg, c in args:
			if d % 4 != 0:
				raise ValueError("{d} is not a multiple of 4. Did you try passing a V then something else?".format(d=d))
			
			try:
				ret = ret + map(arg).format(p=d)
				d = d + deltas[FileSpec.values.index(c)]
			except KeyError as e:
				raise ValueError(arg.describe()) from e
		return ret
	
	def function_writer(f, N: FunctionType, W: str) -> None:
		f.write("void {0}_32(x64emu_t *emu, uintptr_t fcn) {2} {1} fn = ({1})fcn; ".format(N.orig.name, W, "{"))
		
		args = (N.orig.recursive[2:], N.orig.replaced[2:])
		if len(args[0]) == 0:
			raise ValueError("Failed to properly parse {0}:\n{1}\nrecursive has length {2} < 3"
				.format(N.orig.name, N.orig.describe(), len(N.orig.recursive)))
		if len(args[1]) == 0:
			raise ValueError("Failed to properly parse {0}:\n{1}\nreplaced ({2}) has length {3} < 3"
				.format(N.orig.name, N.orig.describe(), N.orig.replaced, len(N.orig.replaced)))
		if len(args[0]) != len(args[1]):
			raise ValueError("Failed to properly parse {0}:\n{1}\nrecursive has length {2}, replaced has length {3}"
				.format(N.orig.name, N.orig.describe(), len(args[0])+2, len(args[1])+2))
		if (len(args[0]) == 2) and (args[0][0].name == 'E') and (args[0][1].name == 'v'): args = ([args[0][0]], args[1][0])
		
		#if any(c in 'PG' for c in args):
		#	# Vulkan struct or GValue pointer, need to unwrap functions at the end
		#	delta = 4
		#	for c in args:
		#		if c == 'P':
		#			f.write("void* save{d} = NULL; void *arg{d} = VulkanFromx86(*(void**)(R_ESP + {d}), &save{d}); ".format(d=delta))
		#		if c == 'G':
		#			f.write("my_GValue_t arg{d}; alignGValue(&arg{d}, *(void**)(R_ESP + {d})); ".format(d=delta))
		#		delta = delta + deltas[FileSpec.values.index(c)]
		#	f.write(vals[FileSpec.values.index(N.orig[0])].format(function_args(args)[:-2]))
		#	delta = 4
		#	for c in args:
		#		if c == 'P':
		#			f.write(" VulkanTox86(arg{d}, save{d});".format(d=delta))
		#		if c == 'G':
		#			f.write(" unalignGValue(*(void**)(R_ESP + {d}), &arg{d});".format(d=delta))
		#		delta = delta + deltas[FileSpec.values.index(c)]
		#	f.write(" }\n")
		#else:
		#	# Generic function
		#	f.write(vals[FileSpec.values.index(N.orig[0])].format(function_args(args)[:-2]) + " }\n")
		assert 'P' not in N.orig.name, "TODO: add back Vulkan compatibility"
		def assertex(v: Optional[T]) -> T:
			assert v is not None, "Value is None"
			return v
		if N.orig.recursive[1].structname == 'E':
			f.write("errno = emu->libc_err; ")
		f.write(function_args(zip(args[0], args[1]), lambda ct: assertex(ct.aspre)))
		f.write(assertex(N.orig.recursive[0].asret).format(function_args(zip(args[0], args[1]), lambda ct: assertex(ct.asarg))[:-2]))
		f.write(function_args(zip(args[0], args[1]), lambda ct: assertex(ct.aspost)))
		if N.orig.recursive[1].structname == 'E':
			f.write(" emu->libc_err = errno;")
		f.write(" }\n")
	
	# TODO: src/wrapped/generated32/converter32.c&h
	# Rewrite the wrapper.c file:
	with open(os.path.join(root, "src", "wrapped32", "generated", "wrapper32.c"), 'w') as file:
		file.write(files_header["wrapper32.c"].format(lbr="{", rbr="}", version=ver))
		
		# First part: typedefs
		for clauses in gbls:
			if not clauses.empty():
				file.write("\n#if " + str(clauses) + "\n")
			generate_typedefs(gbls[clauses], file)
			if not clauses.empty():
				file.write("#endif\n")
		
		file.write("\n")
		
		# Next part: function definitions
		
		for clauses in gbls:
			if not clauses.empty():
				file.write("\n#if " + str(clauses) + "\n")
			for funtype in gbls[clauses]:
				function_writer(file, funtype, funtype.orig.name + "_t")
			if not clauses.empty():
				file.write("#endif\n")
		file.write("\n")
		for clauses in redirects:
			if not clauses.empty():
				file.write("\n#if " + str(clauses) + "\n")
			for funtype in redirects[clauses]:
				assert(funtype.redirected is not None)
				function_writer(file, funtype, funtype.redirected.orig.name + "_t")
			if not clauses.empty():
				file.write("#endif\n")
		
		# Write the isRetX87Wrapper function
		# isRetX87Wrapper
		file.write("\nint isRetX87Wrapper32(wrapper_t fun) {\n")
		for clauses in gbls:
			empty = True
			for funtype in gbls[clauses]:
				if funtype.orig.name[0] in return_x87: # TODO: put this in a function (functions would request the ABI for more info)
					if empty and (not clauses.empty()):
						file.write("#if " + str(clauses) + "\n")
						empty = False
					file.write("\tif (fun == &" + funtype.orig.name + "_32) return 1;\n")
			if not empty:
				file.write("#endif\n")
		file.write("\treturn 0;\n}\n")
		
		file.write(files_guard["wrapper32.c"].format(lbr="{", rbr="}", version=ver))
	
	# Rewrite the wrapper32.h file:
	with open(os.path.join(root, "src", "wrapped32", "generated", "wrapper32.h"), 'w') as file:
		file.write(files_header["wrapper32.h"].format(lbr="{", rbr="}", version=ver))
		for clauses in gbls:
			if not clauses.empty():
				file.write("\n#if " + str(clauses) + "\n")
			for funtype in gbls[clauses]:
				file.write("void " + funtype.orig.name + "_32(x64emu_t *emu, uintptr_t fnc);\n")
			if not clauses.empty():
				file.write("#endif\n")
		file.write("\n")
		for clauses in redirects:
			if not clauses.empty():
				file.write("\n#if " + str(clauses) + "\n")
			for funtype in redirects[clauses]:
				file.write("void " + funtype.orig.name + "_32(x64emu_t *emu, uintptr_t fnc);\n")
			if not clauses.empty():
				file.write("#endif\n")
		file.write(files_guard["wrapper32.h"].format(lbr="{", rbr="}", version=ver))
	
	for fn in filespecs:
		tdtypes[FileSpec.values.index('V')] = "..."
		with open(os.path.join(root, "src", "wrapped32", "generated", fn + "types32.h"), 'w') as file:
			file.write(files_header["fntypes32.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
			generate_typedefs(filespecs[fn].typedefs, file)
			file.write("\n#define SUPER() ADDED_FUNCTIONS()")
			for _bare in filespecs[fn].typedefs:
				for fun in filespecs[fn].typedefs[_bare]:
					if fun.no_dlsym: continue
					file.write(" \\\n\tGO({0}, {1}_t)".format(fun.name, _bare.orig.name))
			file.write("\n\n")
			file.write(files_guard["fntypes32.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
		
		with open(os.path.join(root, "src", "wrapped32", "generated", fn + "defs32.h"), 'w') as file:
			file.write(files_header["fndefs32.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
			for defined in filespecs[fn].structsuses:
				assert defined.redirected is not None, "Unreachable?"
				file.write("#define {defined} {define}\n".format(defined=defined.orig.name, define=defined.redirected.orig.name))
				file.write("#define {defined}_32 {define}_32\n".format(defined=defined.orig.name, define=defined.redirected.orig.name))
			file.write(files_guard["fndefs32.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
		
		with open(os.path.join(root, "src", "wrapped32", "generated", fn + "undefs32.h"), 'w') as file:
			file.write(files_header["fnundefs32.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
			for defined in filespecs[fn].structsuses:
				file.write("#undef {defined}\n".format(defined=defined.orig.name))
				file.write("#undef {defined}_32\n".format(defined=defined.orig.name))
			file.write(files_guard["fnundefs32.h"].format(lbr="{", rbr="}", version=ver, filename=fn))

def main(root: str, files: Iterable[str], ver: str):
	"""
	main -- The main function
	
	root: the root path (where the CMakeLists.txt is located)
	files: a list of files to parse (wrapped*.h)
	ver: version number
	"""
	
	# First read the files inside the headers
	gbl_funcs, red_funcs, filespecs = readFiles(files)
	
	if all(not c.empty() for c in gbl_funcs) or all(not c.empty() for c in red_funcs):
		print("\033[1;31mThere is suspiciously not many types...\033[m", file=sys.stderr)
		print("Check the CMakeLists.txt file. If you are SURE there is nothing wrong"
			  " (as a random example, `set()` resets the variable...), then comment out the following return.", file=sys.stderr)
		print("(Also, the program WILL crash later if you proceed.)", file=sys.stderr)
		return 2 # Check what you did, not proceeding
	
	gbls, redirects = sortArrays(gbl_funcs, red_funcs, filespecs)
	
	# Check if there was any new functions
	functions_list = checkRun(root, gbls, redirects, filespecs)
	if functions_list is None:
		print("Detected same build as last run, skipping")
		return 0
	
	# Now the files rebuilding part
	generate_files(root, files, ver, gbls, redirects, filespecs)
	
	# Save the string for the next iteration, writing was successful
	with open(os.path.join(root, "src", "wrapped32", "generated", "functions_list.txt"), 'w') as file:
		file.write(functions_list)
	
	return 0

if __name__ == '__main__':
	limit = []
	for i, v in enumerate(sys.argv):
		if v == "--":
			limit.append(i)
	Define.defines = list(map(DefineType, sys.argv[2:limit[0]]))
	if main(sys.argv[1], sys.argv[limit[0]+1:], "0.0.2.2") != 0:
		exit(2)
	exit(0)
