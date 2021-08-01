#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <utility>
#include <vector>

#include <llvm/Support/raw_ostream.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>

clang::MangleContext *mangler = nullptr;

std::unordered_map<std::string, std::pair<std::string, bool>> funMap;
std::vector<std::string> funList;

bool isTypeTrivial(const clang::QualType &q, const clang::QualType *qorig);
clang::QualType getPointedType(const clang::QualType q) {
	if (const clang::PointerType *p = q->getAs<clang::PointerType>())
		return getPointedType(p->getPointeeType());
	else if (const clang::ReferenceType *r = q->getAs<clang::ReferenceType>())
		return getPointedType(r->getPointeeType());
	else
		return q;
}

std::string record2name(const clang::RecordDecl &q, const clang::QualType *qorig) {
	std::string s = q.getNameAsString();
	if (s == "") {
		if (!qorig) return "????.!";
		if (const clang::TypedefType *tt = (*qorig)->getAs<clang::TypedefType>()) {
			// Typedef
			if (clang::TypedefNameDecl *td = tt->getDecl()) {
				return td->getNameAsString();
			} else {
				return "<typedef with no declaration>";
			}
		} else {
			return std::string("<unknown type ") + (*qorig)->getTypeClassName() + ">";
		}
	} else {
		return s;
	}
}

char ptr2char(const std::string &str) __attribute__((const));
const char *ptr2str(const std::string &str) __attribute__((const));
char type2char(const clang::QualType &qual /* Canonical */, const clang::QualType *qorig) {
	if (qual->isBuiltinType()) {
		switch (static_cast<const clang::BuiltinType&>(*qual).getKind()) {
		case clang::BuiltinType::Kind::Void:
			return 'v';
		case clang::BuiltinType::Kind::Bool:
			return 'i';
		case clang::BuiltinType::Kind::Char_U:
			return 'C';
		case clang::BuiltinType::Kind::Char_S:
			return 'c';
		case clang::BuiltinType::Kind::Char8:
			return 'c';
		case clang::BuiltinType::Kind::UChar:
			return 'C';
		case clang::BuiltinType::Kind::SChar:
			return 'c';
		case clang::BuiltinType::Kind::WChar_U:
			return 'W';
		case clang::BuiltinType::Kind::UShort:
			return 'W';
		case clang::BuiltinType::Kind::WChar_S:
			return 'w';
		case clang::BuiltinType::Kind::Char16:
			return 'w';
		case clang::BuiltinType::Kind::Short:
			return 'w';
		case clang::BuiltinType::Kind::UInt:
			return 'u';
		case clang::BuiltinType::Kind::Char32:
			return 'i';
		case clang::BuiltinType::Kind::Int:
			return 'i';
		case clang::BuiltinType::Kind::ULong:
			return 'L';
		case clang::BuiltinType::Kind::Long:
			return 'l';
		case clang::BuiltinType::Kind::ULongLong:
			return 'U';
		case clang::BuiltinType::Kind::LongLong:
			return 'I';
		case clang::BuiltinType::Kind::UInt128:
			return 'H';
		case clang::BuiltinType::Kind::Int128:
			return 'H';
		case clang::BuiltinType::Kind::Float:
			return 'f';
		case clang::BuiltinType::Kind::Double:
			return 'd';
		case clang::BuiltinType::Kind::LongDouble:
			return 'D';
		case clang::BuiltinType::Kind::NullPtr:
			return 'p'; // nullptr_t
		
		case clang::BuiltinType::Kind::Half:
		case clang::BuiltinType::Kind::BFloat16:
		case clang::BuiltinType::Kind::ShortAccum:
		case clang::BuiltinType::Kind::Accum:
		case clang::BuiltinType::Kind::LongAccum:
		case clang::BuiltinType::Kind::UShortAccum:
		case clang::BuiltinType::Kind::UAccum:
		case clang::BuiltinType::Kind::ULongAccum:
		case clang::BuiltinType::Kind::ShortFract:
		case clang::BuiltinType::Kind::Fract:
		case clang::BuiltinType::Kind::LongFract:
		case clang::BuiltinType::Kind::UShortFract:
		case clang::BuiltinType::Kind::UFract:
		case clang::BuiltinType::Kind::ULongFract:
		case clang::BuiltinType::Kind::SatShortAccum:
		case clang::BuiltinType::Kind::SatAccum:
		case clang::BuiltinType::Kind::SatLongAccum:
		case clang::BuiltinType::Kind::SatUShortAccum:
		case clang::BuiltinType::Kind::SatUAccum:
		case clang::BuiltinType::Kind::SatULongAccum:
		case clang::BuiltinType::Kind::SatShortFract:
		case clang::BuiltinType::Kind::SatFract:
		case clang::BuiltinType::Kind::SatLongFract:
		case clang::BuiltinType::Kind::SatUShortFract:
		case clang::BuiltinType::Kind::SatUFract:
		case clang::BuiltinType::Kind::SatULongFract:
		case clang::BuiltinType::Kind::Float16:
		case clang::BuiltinType::Kind::Float128:
		case clang::BuiltinType::Kind::Overload:
		case clang::BuiltinType::Kind::BoundMember:
		case clang::BuiltinType::Kind::PseudoObject:
		case clang::BuiltinType::Kind::Dependent:
		case clang::BuiltinType::Kind::UnknownAny:
		case clang::BuiltinType::Kind::ARCUnbridgedCast:
		case clang::BuiltinType::Kind::BuiltinFn:
		case clang::BuiltinType::Kind::ObjCId:
		case clang::BuiltinType::Kind::ObjCClass:
		case clang::BuiltinType::Kind::ObjCSel:
#define IMAGE_TYPE(it, id, si, a, s) case clang::BuiltinType::Kind::id:
#include <clang/Basic/OpenCLImageTypes.def>
#undef IMAGE_TYPE
		case clang::BuiltinType::Kind::OCLSampler:
		case clang::BuiltinType::Kind::OCLEvent:
		case clang::BuiltinType::Kind::OCLClkEvent:
		case clang::BuiltinType::Kind::OCLQueue:
		case clang::BuiltinType::Kind::OCLReserveID:
		case clang::BuiltinType::Kind::IncompleteMatrixIdx:
		case clang::BuiltinType::Kind::OMPArraySection:
		case clang::BuiltinType::Kind::OMPArrayShaping:
		case clang::BuiltinType::Kind::OMPIterator:
#define EXT_OPAQUE_TYPE(et, id, e) case clang::BuiltinType::Kind::id:
#include <clang/Basic/OpenCLExtensionTypes.def>
#define SVE_TYPE(n, id, si) case clang::BuiltinType::Kind::id:
#include <clang/Basic/AArch64SVEACLETypes.def>
#define PPC_VECTOR_TYPE(n, id, s) case clang::BuiltinType::Kind::id:
#include <clang/Basic/PPCTypes.def>
#undef EXT_OPAQUE_TYPE
#undef SVE_TYPE
#undef PPC_VECTOR_TYPE
			return '!';
		default:
			return ':';
		}
	} else if (qual->isEnumeralType()) {
		const clang::EnumDecl *ed = qual->getAs<clang::EnumType>()->getDecl();
		if (!ed) {
			return 'i';
		} else {
			return type2char(ed->getIntegerType().getCanonicalType(), qorig);
		}
	} else if (qual->isFunctionPointerType()) {
		return '@';
	} else if (qual->isAnyPointerType() || qual->isReferenceType()) {
		const clang::QualType &pointed = getPointedType(qual);
		if (isTypeTrivial(pointed, qorig)) {
			return 'p';
		} else if (const clang::RecordType *rct = pointed->getAs<clang::RecordType>()) {
			clang::RecordDecl *rc = rct->getDecl();
			if (!rc) {
				return '!';
			} else if (!rc->isCompleteDefinition()) {
				return 'p';
			} else {
				std::string str;
				if (qorig) {
					const clang::QualType qpted = getPointedType(*qorig);
					str = record2name(*rc, &qpted);
				} else {
					str = record2name(*rc, nullptr);
				}
				char ret = ptr2char(str);
				if (ret) return ret;
				else {
					return '!';
				}
			}
		} else {
			return '!';
		}
	} else if (const clang::RecordType *rct = qual->getAs<clang::RecordType>()) {
		clang::RecordDecl *rc = rct->getDecl();
		if (!rc) {
			return '?';
		} else if (rc->getNameAsString() == "__builtin_va_list") {
			// va_list
			return 'A';
		} else {
			return '?';
		}
	} else {
		return '?';
	}
}
bool isTypeTrivial(const clang::QualType &q, const clang::QualType *qorig) {
	const char c = type2char(q, qorig);
#define GO(chr) || (c == chr)
	return (c == 'v')
		GO('i') GO('u')
		GO('I') GO('U')
		GO('l') GO('L')
		GO('f') GO('d')
		GO('D') GO('K')
		GO('0') GO('1')
		GO('C') GO('c')
		GO('W') GO('w')
		GO('H')
		GO('p');
#undef GO
}
bool isTypeValid(const clang::QualType &q, const clang::QualType *qorig) {
	const char c = type2char(q, qorig);
	if (c == 'A') return false;
	if (c == 'V') return false;
	return ((c >= '0') && (c <= '9')) || ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}

const std::string type2string(const clang::QualType &qual, const clang::QualType *qorig) {
	if (qual->isBuiltinType()) {
		return std::string("(builtin) ") + static_cast<const clang::BuiltinType&>(*qual).getName(clang::PrintingPolicy{{}}).data();
	} else if (qual->isFunctionPointerType()) {
		return "Callback (function pointer)";
	} else if (qual->isAnyPointerType() || qual->isReferenceType()) {
		std::string prefix = qual->isAnyPointerType() ? "Pointer to " : "Reference to ";
		const clang::QualType &pointed = getPointedType(qual);
		if (isTypeTrivial(pointed, qorig)) {
			return prefix + "trivial object " + type2string(pointed, qorig) + " (" + type2char(pointed, qorig) + ")";
		} else if (const clang::RecordType *rct = pointed->getAs<clang::RecordType>()) {
			clang::RecordDecl *rc = rct->getDecl();
			if (!rc) {
				return prefix + "unknown record";
			} else if (!rc->isCompleteDefinition()) {
				return prefix + "incomplete record " + rc->getNameAsString();
			} else {
				std::string str;
				if (qorig) {
					const clang::QualType qpted = getPointedType(*qorig);
					str = record2name(*rc, &qpted);
				} else {
					str = record2name(*rc, nullptr);
				}
				const char *ret = ptr2str(str);
				if (ret[0] != '\0') {
					return prefix + ret;
				} else {
					if (mangler && mangler->shouldMangleDeclName(rc)) {
						std::string mangled;
						{
							llvm::raw_string_ostream strstr{mangled};
							mangler->mangleName(rc, strstr);
						}
						return prefix + "unknown record " + str + " (=== " + mangled + ")";
					} else {
						return prefix + "unknown record " + str;
					}
				}
			}
		} else {
			return prefix + "non-trivial or typedef'ed object " + type2string(pointed, qorig) + " (" + type2char(pointed, qorig) + ")";
			//return "Pointer (maybe to callback)";
		}
	} else if (qual->isEnumeralType()) {
		const clang::EnumDecl *ed = qual->getAs<clang::EnumType>()->getDecl();
		if (!ed) {
			return "Enumeration with unknown underlying integer type (assuming int)";
		} else {
			return "Enumeration with underlying type " + type2string(ed->getIntegerType().getCanonicalType(), nullptr);
		}
	} else if (const clang::RecordType *rct = qual->getAs<clang::RecordType>()) {
		clang::RecordDecl *rc = rct->getDecl();
		if (!rc) {
			return "Unknown record";
		} else if (rc->getNameAsString() == "__builtin_va_list") {
			return "va_list";
		} else {
			return "Unknown record " + std::string(rc->getName().data());
		}
	} else {
		return std::string("??? ") + qual->getTypeClassName();
	}
}

class Visitor : public clang::RecursiveASTVisitor<Visitor> {
public:
	clang::ASTContext &context;
	
	bool shouldVisitTemplateInstantiations() const /* override */ { return true; }
	
	Visitor(clang::CompilerInstance &ci) : context(ci.getASTContext()) {
		if (!mangler) {
			mangler = clang::ItaniumMangleContext::create(context, ci.getDiagnostics());
		}
	}
	
	~Visitor() {
		if (mangler) {
			delete mangler;
			mangler = nullptr;
		}
	}
	
	bool VisitDecl(clang::Decl *decl) /* override */ {
		std::cerr << std::flush;
		if (!decl) return true;
		
		if ((decl->getKind() >= clang::Decl::Kind::firstFunction) && (decl->getKind() <= clang::Decl::Kind::lastFunction)) {
			clang::DeclaratorDecl *ddecl = static_cast<clang::DeclaratorDecl*>(decl);
			std::cout << "Function detected!\n";
			
			std::string funName{ddecl->getName()};
			
			auto niceprint = [](const std::string &infotype, const auto &dat){ std::cout << "    " << infotype << ": " << dat << "\n"; };
			niceprint("Function name", funName);
			if (mangler && mangler->shouldMangleDeclName(ddecl)) {
				std::string mangled;
				{
					llvm::raw_string_ostream strstr{mangled};
					mangler->mangleName(ddecl, strstr);
				}
				niceprint("Function mangled name", mangled);
				funName = std::move(mangled);
			}
			
			bool valid;
			std::string funTypeStr{""};
			if (ddecl->getFunctionType()->isFunctionNoProtoType()) {
				const clang::FunctionNoProtoType *funType = static_cast<const clang::FunctionNoProtoType*>(ddecl->getFunctionType());
				const auto &retType = funType->getReturnType();
				
				niceprint("Function return type", type2string(retType, &retType));
				niceprint("Canonical function return type",
					type2string(retType.getCanonicalType(), &retType) +
					" (" + type2char(retType.getCanonicalType(), &retType) + ")");
				niceprint("Is sugared", funType->isSugared());
				if (funType->isSugared()) {
					clang::QualType qft{funType, 0};
					niceprint("Desugared", type2string(funType->desugar(), &qft));
				}
				
				funTypeStr = type2char(retType.getCanonicalType(), &retType) + std::string("Fv");
				valid = isTypeValid(retType.getCanonicalType(), &retType);
			} else {
				const clang::FunctionProtoType *funType = static_cast<const clang::FunctionProtoType*>(ddecl->getFunctionType());
				const auto &retType = funType->getReturnType();
				
				niceprint("Function return type", type2string(retType, &retType));
				niceprint("Canonical function return type",
					type2string(retType.getCanonicalType(), &retType)
					+ " (" + type2char(retType.getCanonicalType(), &retType) + ")");
				niceprint("Parameter count", funType->getNumParams());
				for (const clang::QualType &type : funType->getParamTypes()) {
					niceprint("  " + type2string(type, &type),
						type2string(type.getCanonicalType(), &type) + " (" + type2char(type.getCanonicalType(), &type) + ")");
				}
				niceprint("Variadic function", funType->isVariadic() ? "yes" : "no");
				
				funTypeStr =
					type2char(retType.getCanonicalType(), &retType) +
					((funType->getNumParams() == 0)
					 ? std::string("Fv") : std::accumulate(funType->getParamTypes().begin(), funType->getParamTypes().end(), std::string("F"),
											[](const std::string &acc, const clang::QualType &qual){ return acc + type2char(qual.getCanonicalType(), &qual); }));
				if (funType->isVariadic()) funTypeStr += "V";
				valid = !funType->isVariadic() &&
					std::accumulate(funType->getParamTypes().begin(), funType->getParamTypes().end(), isTypeValid(retType.getCanonicalType(), &retType),
						[](bool acc, const clang::QualType &qual){ return acc && isTypeValid(qual.getCanonicalType(), &qual); });
			}
			
			niceprint("Conclusion", "");
			niceprint("Function final name", funName);
			niceprint("Function type", funTypeStr);
			niceprint("Valid function type", valid ? "yes" : "no");
			std::cout << "\n";
			
			funMap[funName] = std::make_pair(funTypeStr, valid);
			funList.push_back(funName);
		}
		
		return true;
	}
};

class Consumer : public clang::ASTConsumer {
public:
	Visitor visitor;
	
	Consumer(clang::CompilerInstance &ci) : visitor(ci) {
	}
	
	void HandleTranslationUnit(clang::ASTContext &context) override {
		visitor.TraverseDecl(context.getTranslationUnitDecl());
	}
};

class Action : public clang::ASTFrontendAction {
public:
	virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef inFile) override {
		return std::make_unique<Consumer>(ci);
	}
};

int main(int argc, const char **argv) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " (filenames) -- [-I...]" << std::endl;
		return 2;
	}
	
	/*int fakeargc = argc + 1;
	const char **fakeargv = new const char*[fakeargc];
	memcpy(fakeargv, argv, argc * sizeof(char*));
	fakeargv[fakeargc - 1] = "--";*/
	llvm::cl::OptionCategory opcat{""};
	clang::tooling::CommonOptionsParser op{argc, argv, opcat};
	std::vector<std::string> paths; for (int i = 1; i < argc; ++i) paths.push_back(argv[i]);
	
	clang::tooling::ClangTool tool{op.getCompilations(), paths};
	
	tool.run(clang::tooling::newFrontendActionFactory<Action>().get());
	
	std::cout << "Done, outputing output.h" << std::endl;
	std::sort(funList.begin(), funList.end());
	std::fstream file{"output.h", std::ios_base::out};
	for (const std::string &funName : funList) {
		if (!funMap[funName].second) {
			file << "//";
		}
		file << "GO(" << funName << ", " << funMap[funName].first << ")\n";
	}
	
	return 0;
}
