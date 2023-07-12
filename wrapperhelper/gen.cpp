#include "gen.h"
#include "utils.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/Basic/LLVM.h>

using namespace clang;
using namespace clang::tooling;

static std::vector<uint64_t> GetRecordFieldOff(const std::string& Code, const std::string& Triple) {
    std::vector<uint64_t> FieldOff;
    std::vector<std::string> Args = {"-target", Triple};
    std::unique_ptr<clang::ASTUnit> AST = clang::tooling::buildASTFromCodeWithArgs(Code, Args);
    auto& Ctx = AST->getASTContext();
    auto TranslateDecl = Ctx.getTranslationUnitDecl();
    for (const auto& Decl : TranslateDecl->decls()) {
        if (const auto RecordDecl = clang::dyn_cast<clang::RecordDecl>(Decl)) {
            auto& RecordLayout = Ctx.getASTRecordLayout(RecordDecl);
            for (unsigned i = 0; i < RecordLayout.getFieldCount(); i++) {
                FieldOff.push_back(RecordLayout.getFieldOffset(i) / 8);
            }
            break;
        }
    }
    return FieldOff;
}

static uint64_t GetRecordSize(const std::string& Code, const std::string& Triple) {
    std::vector<std::string> Args = {"-target", Triple};
    std::unique_ptr<ASTUnit> AST = buildASTFromCodeWithArgs(Code, Args);
    auto& Ctx = AST->getASTContext();
    auto TranslateDecl = Ctx.getTranslationUnitDecl();
    for (const auto& Decl : TranslateDecl->decls()) {
        if (const auto recordDecl = dyn_cast<RecordDecl>(Decl)) {
            return Ctx.getTypeSize(recordDecl->getTypeForDecl()) / 8;
        }
    }
    return 0;
}

static CharUnits::QuantityType GetRecordAlign(const std::string& Code, const std::string& Triple) {
    std::vector<std::string> Args = {"-target", Triple};
    std::unique_ptr<ASTUnit> AST = buildASTFromCodeWithArgs(Code, Args);
    auto& Ctx = AST->getASTContext();
    auto TranslateDecl = Ctx.getTranslationUnitDecl();
    for (const auto& Decl : TranslateDecl->decls()) {
        if (const auto recordDecl = dyn_cast<RecordDecl>(Decl)) {
            auto& RecordLayout = Ctx.getASTRecordLayout(recordDecl);
            for (unsigned i = 0; i < RecordLayout.getFieldCount(); i++) {
                return RecordLayout.getAlignment().getQuantity() / 8;
            }
            break;
        }
    }
    return 0;
}

static uint64_t GetTypeSize(const Type* Type, const std::string& Triple) {
    std::string Code = Type->getCanonicalTypeInternal().getAsString() + " dummy;";
    std::vector<std::string> Args = {"-target", Triple};
    std::unique_ptr<ASTUnit> AST = buildASTFromCodeWithArgs(Code, Args);
    auto& Ctx = AST->getASTContext();
    auto TranslateDecl = Ctx.getTranslationUnitDecl();
    for (const auto& Decl : TranslateDecl->decls()) {
        if (const auto varDecl = dyn_cast<VarDecl>(Decl)) {
            return Ctx.getTypeSize(varDecl->getType()) / 8;
        }
    }
    return 0;
}

static std::string TypeToSig(ASTContext* Ctx, const Type* Type) {
    if (Type->isBuiltinType()) {
        switch (Type->getAs<clang::BuiltinType>()->getKind()) {
        case clang::BuiltinType::Kind::Void:
            return "v";
        case clang::BuiltinType::Kind::Bool:
            return "i";
        case clang::BuiltinType::Kind::Char_U:
            return "C";
        case clang::BuiltinType::Kind::Char_S:
            return "c";
        case clang::BuiltinType::Kind::Char8:
            return "c";
        case clang::BuiltinType::Kind::UChar:
            return "C";
        case clang::BuiltinType::Kind::SChar:
            return "c";
        case clang::BuiltinType::Kind::WChar_U:
            return "W";
        case clang::BuiltinType::Kind::UShort:
            return "W";
        case clang::BuiltinType::Kind::WChar_S:
            return "w";
        case clang::BuiltinType::Kind::Char16:
            return "w";
        case clang::BuiltinType::Kind::Short:
            return "w";
        case clang::BuiltinType::Kind::UInt:
            return "u";
        case clang::BuiltinType::Kind::Char32:
            return "i";
        case clang::BuiltinType::Kind::Int:
            return "i";
        case clang::BuiltinType::Kind::ULong:
            return "L";
        case clang::BuiltinType::Kind::Long:
            return "l";
        case clang::BuiltinType::Kind::ULongLong:
            return "U";
        case clang::BuiltinType::Kind::LongLong:
            return "I";
        case clang::BuiltinType::Kind::UInt128:
            return "H";
        case clang::BuiltinType::Kind::Int128:
            return "H";
        case clang::BuiltinType::Kind::Float:
            return "f";
        case clang::BuiltinType::Kind::Double:
            return "d";
        case clang::BuiltinType::Kind::LongDouble:
            return "D";
        case clang::BuiltinType::Kind::NullPtr:
            return "p"; // nullptr_t
        default:
            std::cout << "Unsupported BuiltinType: " << Type->getCanonicalTypeInternal().getAsString() << std::endl;
        }
    } else {
        if (Type->isPointerType()) {
            return "p";
        } else if (Type->isVoidType()) {
            return "v";
        } else if (Type->isUnsignedIntegerOrEnumerationType()) {
            switch(Ctx->getTypeSizeInChars(Type).getQuantity()) {
                case 1:
                    return "C";
                case 2:
                    return  "W";
                case 4:
                    return "u";
                case 8:
                    return "U";
                default:
                    std::cout << "Unsupported UnsignedInteger Type: " << Type->getCanonicalTypeInternal().getAsString() << std::endl;
            }
        } else if (Type->isSignedIntegerOrEnumerationType()) {
            switch(Ctx->getTypeSizeInChars(Type).getQuantity()) {
                case 1:
                    return "c";
                case 2:
                    return  "w";
                case 4:
                    return "i";
                case 8:
                    return "I";
                default:
                    std::cout << "Unsupported SignedInteger Type: " << Type->getCanonicalTypeInternal().getAsString() << std::endl;
            }
        } else if (Type->isCharType()) {
            return "c";
        } else if (Type->isFloatingType()) {
            switch(Ctx->getTypeSizeInChars(Type).getQuantity()) {
                case 4:
                    return "f";
                case 8:
                    return "d";
                case 16:
                    return "D";
                default:
                    std::cout << "Unsupported Floating Type: " << Type->getCanonicalTypeInternal().getAsString()
                              << " (quantity = " << Ctx->getTypeSizeInChars(Type).getQuantity() << ")" << std::endl;
            }
        } else {
            std::cout << "Unsupported Type: " << Type->getCanonicalTypeInternal().getAsString() << std::endl;
        }
    }
    return "?";
}

// Prepare for generation, collect the structures and functions that need to be prcessed
void WrapperGenerator::Prepare(ASTContext *Ctx) {
  for (const auto &func_pair : funcs) {
    for (auto Type : func_pair.second.callback_args) {
      if (Type && Type->isTypedefNameType()) {
        callbacks[StripTypedef(Type->getPointeeType())] =
            Type->getAs<TypedefType>()->getDecl()->getNameAsString();
      } else if (Type) {
        callbacks[StripTypedef(Type->getPointeeType())] =
            GetFuncSig(Ctx, Type->getPointeeType().getTypePtr()) + "_t";
      }
    }
  }
  std::vector<const Type *> Types;
  for (const auto &record_pair : records) {
    Types.push_back(record_pair.first);
  }
  for (auto type : Types) {
    std::set<const Type*> Visited{type};
    bool Special = false;
    ParseRecordRecursive(Ctx, type, Special, Visited);
  }
  for (auto it = records.begin(); it != records.end();) {
    if (!it->second.is_special) {
      it = records.erase(it);
    } else {
      for (auto field : it->second.callback_fields) {
        if (field->isTypedefNameType()) {
          callbacks[StripTypedef(field->getPointeeType())] =
              field->getAs<TypedefType>()->getDecl()->getNameAsString();
        } else {
          callbacks[StripTypedef(field->getPointeeType())] =
              GetFuncSig(Ctx, field->getPointeeType().getTypePtr()) + "_t";
        }
      }
      ++it;
    }
  }
  for (auto &func_pair : funcs) {
    for (unsigned i = 0; i < func_pair.second.decl->getNumParams(); i++) {
      auto ParamDecl = func_pair.second.decl->getParamDecl(i);
      auto ParamType = ParamDecl->getType();
      if (ParamType->isPointerType() &&
          ParamType->getPointeeType()->isRecordType()) {
        if (records.find(StripTypedef(ParamType->getPointeeType())) !=
            records.end()) {
          func_pair.second.has_special_arg = true;
          break;
        }
      } else if (ParamType->isRecordType()) {
        if (records.find(StripTypedef(ParamType)) != records.end()) {
          func_pair.second.has_special_arg = true;
          break;
        }
      }
    }
    auto RetType = func_pair.second.decl->getReturnType();
    if (RetType->isPointerType() && RetType->getPointeeType()->isRecordType()) {
      if (records.find(StripTypedef(RetType->getPointeeType())) !=
          records.end()) {
        func_pair.second.has_special_ret = true;
      }
    } else if (RetType->isRecordType()) {
      if (records.find(StripTypedef(RetType)) != records.end()) {
        func_pair.second.has_special_ret = true;
      }
    }
  }
}

// Gen callback typedef
std::string WrapperGenerator::GenCallbackTypeDefs(ASTContext *Ctx) {
  (void)Ctx;
  std::string res;
  for (auto callback : callbacks) {
    auto Type = callback.first;
    auto Definition = GetFuncDefinition(Type);
    res += "typedef " + Definition.ret_str + "(*" + callback.second + ")(";
    for (int i = 0; i < Definition.arg_size - 1; i++) {
      res += Definition.arg_types_str[i] + Definition.arg_names[i] + ", ";
    }
    if (Definition.arg_size) {
      res += Definition.arg_types_str[Definition.arg_size - 1] + Definition.arg_names[Definition.arg_size - 1];
    }
    res += ");\n";
  }
  return res;
}

// Gen function declare
std::string WrapperGenerator::GenDeclare(ASTContext *Ctx,
                                         const FuncInfo &Func) {
  std::string res;
  std::string sig = GetFuncSig(Ctx, Func);
  res += "GO";
  if (Func.is_weak) {
    res += "W";
  }
  if (sig.find('E') != std::string::npos) {
    res += "M";
  }
  res += "(" + Func.func_name + ", " + sig + ")\n";
  ;
  return res;
}

// Gen structure declare
std::string WrapperGenerator::GenDeclare(ASTContext *Ctx,
                                         const RecordInfo &Record) {
  (void)Ctx;
  std::string RecordStr;
  std::string PreDecl;
  RecordStr += "\ntypedef ";
  RecordStr +=
      (Record.is_union ? "union " : "struct ") + Record.type_name + " {\n";
  for (const auto &Field : Record.decl->fields()) {
    auto Type = Field->getType();
    std::string Name = Field->getNameAsString();
    RecordStr += "    ";
    if (Type->isFunctionPointerType()) {
      auto FuncType = StripTypedef(Type->getPointeeType());
      if (callbacks.count(FuncType)) {
        std::string FieldStr = callbacks[FuncType];
        FieldStr += " ";
        FieldStr += Name;
        RecordStr += FieldStr;
      } else {
        std::cout << "Err: FuncPtr(" << Record.type_name << "." << Name << ") is not supported\n";
      }
    } else if (Type->isPointerType()) {
      auto PointeeType = Type->getPointeeType();
      if (PointeeType->isRecordType()) {
        if (records.count(PointeeType.getTypePtr())) {
          std::string FieldStr = records[PointeeType.getTypePtr()].type_name;
          FieldStr += "_ptr ";
          FieldStr += Name;
          RecordStr += FieldStr;
        } else {
          RecordStr += "void *" + Name;
        }
      } else {
        RecordStr += "void *" + Name;
      }
    } else if (Type->isRecordType()) {
      if (records.count(Type.getTypePtr())) {
        std::string FieldStr = records[Type.getTypePtr()].type_name;
        FieldStr += " ";
        FieldStr += Name;
        RecordStr += FieldStr;
      } else {
        RecordStr += TypeStringify(StripTypedef(Type), Field, nullptr, PreDecl);
      }
    } else {
      RecordStr += TypeStringify(StripTypedef(Type), Field, nullptr, PreDecl);
    }
    RecordStr += ";\n";
  }
  RecordStr += "} ";
  RecordStr += Record.type_name + ", *" + Record.type_name + "_ptr;\n";
  return RecordStr;
}

std::string WrapperGenerator::GenCallbackWrap(ASTContext *Ctx,
                                              const FuncInfo &Func) {
  (void)Ctx;
  std::string res;

  for (unsigned i = 0; i < Func.decl->getNumParams(); i++) {
    auto ParamType = Func.decl->getParamDecl(i)->getType();
    if (ParamType->isFunctionPointerType()) {

      auto PointeeType = ParamType->getPointeeType();
      auto Definition = GetFuncDefinition(PointeeType.getTypePtr());
      std::string my_funcname =
          std::string("my_") + Func.decl->getParamDecl(i)->getNameAsString();
      std::string funcname = Func.decl->getParamDecl(i)->getNameAsString();
      res += "\n#define GO(A) \\\n"
             "static uintptr_t " + my_funcname + "_fct_##A = 0; \\\n" +
             Definition.ret_str + " " + my_funcname + "(";
      int arg_size = Definition.arg_names.size();
      if (arg_size) {
        for (int i = 0; i < arg_size - 1; i++) {
          res += Definition.arg_types_str[i] + " " + Definition.arg_names[i] + ", ";
        }
        res += Definition.arg_types_str[arg_size - 1] + " " + Definition.arg_names[arg_size - 1];
      }
      res += ") { \\\n"
             "    return RunFunction(my_context, " + my_funcname + "_fct_##A" + ", " + std::to_string(arg_size);
      for (int i = 0; i < arg_size; i++) {
        res += ", " + Definition.arg_names[i];
      }
      res += "); \\\n"
             "}\n"
             "SUPER()\n"
             "#undef GO\n"
             "static void* find" + funcname + "Fct(void* fct) {\n"
             "    if (!fct) return fct;\n"
             "    if (GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);\n"
             "    #define GO(A) if (" + my_funcname + "_fct_##A == (uintptr_t)fct) return " + my_funcname + "_##A;\n"
             "    SUPER()\n"
             "    #undef GO\n"
             "    #define GO(A) if (" + my_funcname + "_fct_##A == 0) { " + my_funcname + "_fct_##A = (uintptr_t)fct; return " + my_funcname + "_##A; }\n"
             "    SUPER()\n"
             "    #undef GO\n"
             "    return NULL;\n"
             "}\n";
    }
  }
  return res;
}

std::string WrapperGenerator::GenCallbackWrap(ASTContext *Ctx,
                                              const RecordInfo &Struct) {
  (void)Ctx;
  std::string res;
  for (const auto &field : Struct.decl->fields()) {
    auto FieldType = field->getType();
    if (FieldType->isFunctionPointerType()) {
      auto PointeeType = FieldType->getPointeeType();
      auto Definition = GetFuncDefinition(PointeeType.getTypePtr());
      std::string my_funcname = std::string("my_") + field->getNameAsString();
      std::string funcname = field->getNameAsString();
      res += "\n#define GO(A) \\\n"
             "static uintptr_t " + my_funcname + "_fct_##A = 0; \\\n" +
             Definition.ret_str + " " + my_funcname + "_##A(";
      int arg_size = Definition.arg_names.size();
      if (arg_size) {
        for (int i = 0; i < arg_size - 1; i++) {
          res += Definition.arg_types_str[i] + " " + Definition.arg_names[i] + ", ";
        }
        res += Definition.arg_types_str[arg_size - 1] + " " + Definition.arg_names[arg_size - 1];
      }
      res += ") { \\\n"
             "    return RunFunction(my_context, " + my_funcname + "_fct_##A" + ", " + std::to_string(arg_size);
      for (int i = 0; i < arg_size; i++) {
        res += ", " + Definition.arg_names[i];
      }
      res += "); \\\n"
             "}\n"
             "SUPER()\n"
             "#undef GO\n"
             "static void* find" + funcname + "Fct(void* fct) {\n"
             "    if(!fct) return fct;\n"
             "    if(GetNativeFnc((uintptr_t)fct)) return GetNativeFnc((uintptr_t)fct);\n"
             "    #define GO(A) if(" + my_funcname + "_fct_##A == (uintptr_t)fct) return " + my_funcname + "_##A;}\n"
             "    SUPER()\n"
             "    #undef GO\n"
             "    #define GO(A) if(" + my_funcname + "_fct_##A == 0) {" + my_funcname + "_fct_##A = (uintptr_t)fct;" + "return " + my_funcname + "_##A;}\n"
             "    SUPER()\n"
             "    #undef GO\n"
             "    return NULL;\n"
             "}\n";
    }
  }
  return res;
}

std::string WrapperGenerator::GenDefine(ASTContext *Ctx,
                                        const FuncInfo &Func) {
  std::string res;
  auto Definition = GetFuncDefinition(Func.decl);
  std::string Sig = GetFuncSig(Ctx, Func.type);
  res += "\nEXPORT " + Definition.ret_str + "my_" + Func.func_name + "(";
  if (Sig.find('E')) {
    res += "void *emu, ";
  }
  int arg_size = Definition.arg_names.size();
  if (arg_size) {
    for (int i = 0; i < arg_size - 1; i++) {
      if (Definition.arg_types[i]->isPointerType()) {
        auto PointeeType = Definition.arg_types[i]->getPointeeType();
        if (records.count(PointeeType.getTypePtr())) {
          res +=
              Definition.arg_types[i]->getCanonicalTypeInternal().getAsString();
        } else {
          res += Definition.arg_types_str[i];
        }
      } else {
        res += Definition.arg_types_str[i];
      }
      res += " " + Definition.arg_names[i] + ", ";
    }
    if (Definition.arg_types[arg_size - 1]->isPointerType()) {
      auto PointeeType = Definition.arg_types[arg_size - 1]->getPointeeType();
      if (records.count(PointeeType.getTypePtr())) {
        res += Definition.arg_types[arg_size - 1]
                   ->getCanonicalTypeInternal()
                   .getAsString();
      } else {
        res += Definition.arg_types_str[arg_size - 1];
      }
    } else {
      res += Definition.arg_types_str[arg_size - 1];
    }
    res += " ";
    res += Definition.arg_names[arg_size - 1];
  }
  res += ") {\n";
  if (Func.has_special_arg) {
    res += "    // WARN: This function's arg has a structure ptr which is "
                   "special, may need to wrap it for the host\n";

  } else if (Func.has_special_ret) {
    res += "    // WARN: This function's ret is a structure ptr which is "
                   "special, may need to wrap it for the guest\n";
  }
  if (Func.has_callback_arg) {
    res += "    " + my_lib_type + " *my = " + "(" + my_lib_type + "*)" +
                   my_lib + "->priv.w.p2;\n"
                   "    my->" + Func.func_name + "(";
    if (arg_size) {
      for (int i = 0; i < arg_size - 1; i++) {
        if (Func.callback_args[i]) {
          if (!Func.callback_args[i]->isTypedefNameType()) {
            res +=
                "find" + Func.func_name + "_arg" + std::to_string(i) + "Fct";
          } else {
            res += "find" +
                           Func.callback_args[i]
                               ->getAs<TypedefType>()
                               ->getDecl()
                               ->getNameAsString() +
                           "Fct";
          }
          res += "(" + Definition.arg_names[i] + ")";
        } else {
          res += Definition.arg_names[i];
        }
        res += ", ";
      }
      if (Func.callback_args[arg_size - 1]) {
        if (!Func.callback_args[arg_size - 1]->isTypedefNameType()) {
          res += "find" + Func.func_name + "_arg" +
                         std::to_string(arg_size - 1) + "Fct";
        } else {
          res += "find" +
                         Func.callback_args[arg_size - 1]
                             ->getAs<TypedefType>()
                             ->getDecl()
                             ->getNameAsString() +
                         "Fct";
        }
        res += "(" + Definition.arg_names[arg_size - 1] + ")";
      } else {
        res += Definition.arg_names[arg_size - 1];
      }
      res += ")\n";
    }
  } else {
    res += "    " + my_lib_type + " *my = (" + my_lib_type + "*)" + my_lib + "->priv.w.p2;\n"
                   "    my->" + Func.func_name + "(";
    if (arg_size) {
      for (int i = 0; i < arg_size - 1; i++) {
        res += Definition.arg_names[i] + ", ";
      }
      res += Definition.arg_names[arg_size - 1];
    }
      res += ");\n";
  }

  res += "}\n";
  return res;
}

std::string WrapperGenerator::GenDeclareDiffTriple(
    ASTContext *Ctx, const RecordInfo &Record,
    const std::string &GuestTriple, const std::string &HostTriple) {
  (void)Ctx;
  std::string GuestRecord;
  std::string HostRecord;
  std::string PreDecl;
  std::vector<uint64_t> GuestFieldOff;
  std::vector<uint64_t> HostFieldOff;
  GuestRecord += "typedef ";
  HostRecord += "typedef ";
  GuestRecord +=
      (Record.is_union ? "union " : "struct ") + Record.type_name + " {\n";
  HostRecord += (Record.is_union ? "union " : "struct ") +
                std::string("host_") + Record.type_name + " {\n";
  auto OffDiff = GetRecordFieldOffDiff(Record.type, GuestTriple, HostTriple,
                                       GuestFieldOff, HostFieldOff);
  uint64_t GuestRecordSize = GetRecordSize(Record.type, GuestTriple);
  uint64_t HostRecordSize = GetRecordSize(Record.type, HostTriple);
  uint64_t SizeDiff = GuestRecordSize - HostRecordSize;
  int FieldIndex = 0;
  std::set<FieldDecl *> AlignDiffFields;
  for (const auto &Field : Record.decl->fields()) {
    if (OffDiff[FieldIndex] == 0) {
      FieldIndex++;
      continue;
    }
    std::string Name = Field->getNameAsString();
    if (OffDiff[FieldIndex] != SizeDiff) {
      auto Diff = OffDiff[FieldIndex];
      AlignDiffFields.insert(Field);
      for (size_t i = FieldIndex; i < OffDiff.size(); i++) {
        if (OffDiff[i] == Diff) {
          OffDiff[i] = 0;
        } else {
          break;
        }
      }
    } else {
      AlignDiffFields.insert(Field);
      break;
    }
    FieldIndex++;
  }
  for (const auto &Field : Record.decl->fields()) {
    auto Type = Field->getType();
    std::string Name = Field->getNameAsString();
    GuestRecord += "  ";
    HostRecord += "  ";
    if (AlignDiffFields.find(Field) != AlignDiffFields.end()) {
      auto typeSize = GetTypeSize(StripTypedef(Field->getType()), guest_triple);
      switch (typeSize) {
      // FIXME: should test more case in different triple
      case 4: GuestRecord += "int " + Name        ; break;
      case 8: GuestRecord += "int " + Name + "[2]"; break;
      default:
        std::cout << "Err: unknown type size " << typeSize << std::endl;
        break;
      }
      HostRecord += TypeStringify(StripTypedef(Type), Field, nullptr, PreDecl);
    } else if (Type->isFunctionPointerType()) {
      auto FuncType = StripTypedef(Type->getPointeeType());
      if (callbacks.count(FuncType)) {
        std::string FieldStr = callbacks[FuncType];
        FieldStr += " ";
        FieldStr += Name;
        GuestRecord += FieldStr;
        HostRecord += FieldStr;
      } else {
        std::cout << "Err: FuncPtr(" << Record.type_name << "." << Name << ") is not supported" << std::endl;
      }
    } else if (Type->isPointerType()) {
      auto PointeeType = Type->getPointeeType();
      if (PointeeType->isRecordType()) {
        if (records.count(PointeeType.getTypePtr())) {
          std::string FieldStr = records[PointeeType.getTypePtr()].type_name;
          FieldStr += "_ptr " + Name;
          GuestRecord += FieldStr;
          HostRecord += "host_" + FieldStr;
        } else {
          GuestRecord += "void *" + Name;
          HostRecord += "void *" + Name;
        }
      } else {
        GuestRecord += "void *" + Name;
        HostRecord += "void *" + Name;
      }
    } else if (Type->isRecordType()) {
      if (records.count(Type.getTypePtr())) {
        std::string FieldStr = records[Type.getTypePtr()].type_name;
        FieldStr += " " + Name;
        GuestRecord += FieldStr;
        HostRecord += "host_" + FieldStr;
      } else {
        GuestRecord += TypeStringify(StripTypedef(Type), Field, nullptr, PreDecl);
        HostRecord += TypeStringify(StripTypedef(Type), Field, nullptr, PreDecl);
      }
    } else {
      HostRecord += TypeStringify(StripTypedef(Type), Field, nullptr, PreDecl);
      GuestRecord += TypeStringify(StripTypedef(Type), Field, nullptr, PreDecl);
    }
    GuestRecord += ";\n";
    HostRecord += ";\n";
  }
  GuestRecord += "} " + Record.type_name + ", *" + Record.type_name + "_ptr;\n";
  HostRecord += "} host_" + Record.type_name + ", *host_" + Record.type_name + "_ptr;\n";
  return GuestRecord + HostRecord;
}

// Gen record convert function between host and guest
std::string WrapperGenerator::GenRecordConvert(const RecordInfo &Record) {
  std::string res;
  if (Record.guest_size != Record.host_size) {
    auto RecordDecl = Record.decl;
    std::vector<uint64_t> GuestFieldOff;
    std::vector<uint64_t> HostFieldOff;
    auto OffDiff = GetRecordFieldOffDiff(Record.type, guest_triple, host_triple,
                                         GuestFieldOff, HostFieldOff);
    int FieldIndex = 0;
    std::vector<FieldDecl *> AlignDiffFields;
    uint64_t SizeDiff = Record.guest_size - Record.host_size;
    for (const auto &Field : RecordDecl->fields()) {
      if (OffDiff[FieldIndex] == 0) {
        FieldIndex++;
        continue;
      }
      if (OffDiff[FieldIndex] != SizeDiff) {
        auto Diff = OffDiff[FieldIndex];
        AlignDiffFields.push_back(Field);
        for (size_t i = FieldIndex; i < OffDiff.size(); i++) {
          if (OffDiff[i] == Diff) {
            OffDiff[i] = 0;
          } else {
            break;
          }
        }
      } else {
        AlignDiffFields.push_back(Field);
        break;
      }
      FieldIndex++;
    }
    if (!AlignDiffFields.size()) {
      return res;
    }
    res += "void g2h_" + Record.type_name + "(" + "struct host_" + Record.type_name + " *d, struct " + Record.type_name + " *s) {\n";
    std::string body = "    memcpy(d, s, offsetof(struct " + Record.type_name +
                       ", " + AlignDiffFields[0]->getNameAsString() + "));\n";
    std::string offstr = "offsetof(struct " + Record.type_name + ", " +
                         AlignDiffFields[0]->getNameAsString() + ")";
    for (size_t i = 0; i < AlignDiffFields.size() - 1; i++) {
      body += "    memcpy(d->" + AlignDiffFields[i]->getNameAsString() + ", " +
              "s->" + AlignDiffFields[i]->getNameAsString() + ", " +
              "offsetof(struct " + Record.type_name + ", " +
              AlignDiffFields[i + 1]->getNameAsString() + ") - " + offstr +
              ");\n";
      offstr = "offsetof(struct " + Record.type_name + ", " +
               AlignDiffFields[i + 1]->getNameAsString() + ")";
    }
    body += "    memcpy(d->" +
            AlignDiffFields[AlignDiffFields.size() - 1]->getNameAsString() +
            ", " + "s->" +
            AlignDiffFields[AlignDiffFields.size() - 1]->getNameAsString() +
            ", " + std::to_string(GetRecordSize(Record.type, guest_triple)) +
            " - " + offstr + ");\n";
    res += body + "}\n";

    res += "void h2g_" + Record.type_name + "(struct " + Record.type_name + " *d, " + "struct host_" + Record.type_name + " *s) {\n";
    res += body;
    res += "}\n";
  }
  return res;
}

void WrapperGenerator::ParseRecordRecursive(
    ASTContext *Ctx, const Type *type, bool &Special,
    std::set<const Type *> &Visited) {
  auto recordType = type->getAs<RecordType>();
  auto RecordDecl = recordType->getDecl();
  for (const auto &field : RecordDecl->fields()) {
    auto FieldType = field->getType();
    if (FieldType->isFunctionPointerType()) {
      auto Record = &records[type];
      Record->callback_fields.push_back(field->getType().getTypePtr());
      // Record->type_name =
      Special = true;
    } else if (FieldType->isPointerType() &&
               FieldType->getPointeeType()->isRecordType()) {
      auto FieldRecordType = StripTypedef(FieldType->getPointeeType());
      if (Visited.find(FieldRecordType) != Visited.end())
        continue;
      Visited.insert(FieldRecordType);
      bool _Special = false;
      ParseRecordRecursive(Ctx, FieldRecordType, _Special, Visited);
      if (_Special)
        Special = true;
    } else if (FieldType->isRecordType()) {
      auto FieldRecordType = StripTypedef(FieldType);
      if (Visited.find(FieldRecordType) != Visited.end())
        continue;
      Visited.insert(FieldRecordType);
      bool _Special = false;
      ParseRecordRecursive(Ctx, FieldRecordType, _Special, Visited);
      if (_Special)
        Special = true;
    }
  }
  uint64_t GuestSize = GetRecordSize(type, guest_triple);
  uint64_t HostSize = GetRecordSize(type, host_triple);

  auto Record = &records[type];
  if (GuestSize != HostSize) {
    Special = 1;
  }
  if (type->isUnionType()) {
    Record->is_union = true;
  }
  if (!Record->decl) {
    Record->type = type;
    Record->decl = RecordDecl;
    if (RecordDecl->getIdentifier())
      Record->type_name = RecordDecl->getIdentifier()->getName().str();
  }
  Record->guest_size = GuestSize;
  Record->host_size = HostSize;
  if (Record->type_name.empty()) {
    Record->is_special = false;
  } else
    Record->is_special = Special;
}

// Type to String
std::string WrapperGenerator::TypeStringify(const Type *Type,
                                         FieldDecl *FieldDecl,
                                         ParmVarDecl *ParmDecl,
                                         std::string& PreDecl,
                                         std::string indent,
                                         std::string Name) {
  std::string res;
  std::string name = FieldDecl
                         ? FieldDecl->getNameAsString()
                         : (ParmDecl ? ParmDecl->getNameAsString() : Name);
  if (Type->isPointerType()) {
    auto PointeeType = Type->getPointeeType();
    if (PointeeType->isBuiltinType()) {
      res +=
          StripTypedef(PointeeType)->getCanonicalTypeInternal().getAsString();
    } else if (PointeeType->isRecordType()) {
      if (records.find(StripTypedef(PointeeType)) != records.end() &&
          records[StripTypedef(PointeeType)].is_special) {
        res += (PointeeType->isUnionType() ? "union " : "struct ") + records[StripTypedef(PointeeType)].type_name;
      } else {
        res += "void";
      }
    } else {
      res += "void";
    }
    res += " *" + name;
  } else if (Type->isEnumeralType()) {
    res += "int " + name;
  } else if (Type->isRecordType()) {
    if (records.find(StripTypedef(Type->getCanonicalTypeInternal())) !=
            records.end() &&
        records[StripTypedef(Type->getCanonicalTypeInternal())].is_special) {
      res += Type->isUnionType() ? "union " : "struct ";
      res += records[StripTypedef(Type->getCanonicalTypeInternal())].type_name;
      res += " ";
    } else {
      res += AnonRecordDecl(Type->getAs<RecordType>(), PreDecl, indent + "  ");
    }
    res += name;
  } else if (Type->isConstantArrayType()) {
    auto ArrayType =
        dyn_cast<ConstantArrayType>(Type->getAsArrayTypeUnsafe());
    int EleSize = ArrayType->getSize().getZExtValue();
    if (ArrayType->getElementType()->isPointerType()) {
      res += "void *";
    } else if (ArrayType->getElementType()->isEnumeralType()) {
      res += "int ";
    } else if (ArrayType->getElementType()->isRecordType()) {
      auto RecordType = ArrayType->getElementType()->getAs<clang::RecordType>();
      auto RecordDecl = RecordType->getDecl();
      if (RecordDecl->isCompleteDefinition()) {
        auto& Ctx = RecordDecl->getDeclContext()->getParentASTContext();
        PreDecl += "#include \"";
        PreDecl += GetDeclHeaderFile(Ctx, RecordDecl);
        PreDecl += "\"";
        PreDecl += "\n";
      }
      res += StripTypedef(ArrayType->getElementType())
                ->getCanonicalTypeInternal()
                .getAsString();
    } else {
      res += StripTypedef(ArrayType->getElementType())
                 ->getCanonicalTypeInternal()
                 .getAsString();
    }
    res += " ";
    res += name;
    res += "[";
    res += std::to_string(EleSize);
    res += "]";
  } else {
    res += StripTypedef(Type->getCanonicalTypeInternal())
               ->getCanonicalTypeInternal()
               .getAsString();
    res += " ";
    res += name;
  }
  return indent + res;
}

// Type to String, less detail
std::string WrapperGenerator::SimpleTypeStringify(const Type *Type,
                                               FieldDecl *FieldDecl,
                                               ParmVarDecl *ParmDecl,
                                               std::string indent,
                                               std::string Name) {
  std::string res;
  std::string name = FieldDecl
                         ? FieldDecl->getNameAsString()
                         : (ParmDecl ? ParmDecl->getNameAsString() : Name);
  if (Type->isPointerType()) {
    res += "void *" + name;
  } else if (Type->isEnumeralType()) {
    res += "int ";
    res += name;
  } else if (Type->isRecordType()) {
    if (records.find(StripTypedef(Type->getCanonicalTypeInternal())) !=
        records.end()) {
      res += Type->isUnionType() ? "union " : "struct ";
      res += records[StripTypedef(Type->getCanonicalTypeInternal())].type_name;
      res += " ";
    } else {
      res += SimpleAnonRecordDecl(Type->getAs<RecordType>(), indent);
    }
    res += name;
  } else if (Type->isConstantArrayType()) {
    auto ArrayType =
        dyn_cast<ConstantArrayType>(Type->getAsArrayTypeUnsafe());
    int EleSize = ArrayType->getSize().getZExtValue();
    if (ArrayType->getElementType()->isPointerType()) {
      res += "void *";
    } else {
      res += StripTypedef(ArrayType->getElementType())
                 ->getCanonicalTypeInternal()
                 .getAsString();
    }
    res += " ";
    res += name;
    res += "[";
    res += std::to_string(EleSize);
    res += "]";
  } else {
    res += StripTypedef(Type->getCanonicalTypeInternal())
               ->getCanonicalTypeInternal()
               .getAsString();
    res += " ";
    res += name;
  }
  return indent + res;
}

std::string WrapperGenerator::AnonRecordDecl(const RecordType *Type, std::string& PreDecl, std::string indent) {
  auto RecordDecl = Type->getDecl();
  std::string res;
  res += Type->isUnionType() ? "union {\n" : "struct {\n";
  for (const auto &field : RecordDecl->fields()) {
    auto FieldType = field->getType();
    res += TypeStringify(StripTypedef(FieldType), field, nullptr, PreDecl, indent + "  ");
    res += ";\n";
  }
  res += indent + "} ";
  return res;
}

std::string
WrapperGenerator::SimpleAnonRecordDecl(const RecordType *Type, std::string indent) {
  auto RecordDecl = Type->getDecl();
  std::string res;
  res += Type->isUnionType() ? "union {\n" : "struct {\n";
  for (const auto &field : RecordDecl->fields()) {
    auto FieldType = field->getType();
    res += SimpleTypeStringify(StripTypedef(FieldType), field, nullptr, indent + "  ");
    res += ";\n";
  }
  res += indent + "} ";
  return res;
}

// Get func info from FunctionType
FuncDefinition WrapperGenerator::GetFuncDefinition(const Type *Type) {
  FuncDefinition res;
  std::string PreDecl;
  auto ProtoType = Type->getAs<FunctionProtoType>();
  res.ret = StripTypedef(ProtoType->getReturnType());
  res.ret_str =
      TypeStringify(StripTypedef(ProtoType->getReturnType()), nullptr, nullptr, PreDecl);
  for (unsigned i = 0; i < ProtoType->getNumParams(); i++) {
    auto ParamType = ProtoType->getParamType(i);
    res.arg_types.push_back(StripTypedef(ParamType));
    res.arg_types_str.push_back(
        TypeStringify(StripTypedef(ParamType), nullptr, nullptr, PreDecl));
    res.arg_names.push_back(std::string("a") + std::to_string(i));
  }
  if (ProtoType->isVariadic()) {
    res.is_variadaic = true;
  }
  res.arg_size = ProtoType->getNumParams();
  return res;
}

// Get funcdecl info from FunctionDecl
FuncDefinition WrapperGenerator::GetFuncDefinition(FunctionDecl *Decl) {
  FuncDefinition res;
  std::string PreDecl;
  auto RetType = Decl->getReturnType();
  res.ret = RetType.getTypePtr();
  res.ret_str = TypeStringify(StripTypedef(RetType), nullptr, nullptr, PreDecl);
  for (unsigned i = 0; i < Decl->getNumParams(); i++) {
    auto ParamDecl = Decl->getParamDecl(i);
    auto ParamType = ParamDecl->getType();
    res.arg_types.push_back(ParamType.getTypePtr());
    res.arg_types_str.push_back(
        TypeStringify(StripTypedef(ParamType), nullptr, nullptr, PreDecl));
    res.arg_names.push_back(ParamDecl->getNameAsString());
  }
  if (Decl->isVariadic()) {
    res.is_variadaic = true;
  }
  return res;
}

// Get the offset diff between two different triple
std::vector<uint64_t> WrapperGenerator::GetRecordFieldOffDiff(
    const Type *Type, const std::string &GuestTriple,
    const std::string &HostTriple, std::vector<uint64_t> &GuestFieldOff,
    std::vector<uint64_t> &HostFieldOff) {
  std::string PreDecl;
  std::string Code = TypeStringify(Type, nullptr, nullptr, PreDecl, "", "dummy;");
  std::vector<uint64_t> OffsetDiff;
  GuestFieldOff = GetRecordFieldOff(Code, GuestTriple);
  HostFieldOff = GetRecordFieldOff(Code, HostTriple);
  if (GuestFieldOff.size() != HostFieldOff.size()) {
    // Should not happen
    std::cout << "Greater field offsets in guest than in host" << std::endl;
    return OffsetDiff;
  }
  for (size_t i = 0; i < GuestFieldOff.size(); i++) {
    OffsetDiff.push_back(GuestFieldOff[i] - HostFieldOff[i]);
  }
  return OffsetDiff;
}

// Get the size under a specific triple
uint64_t WrapperGenerator::GetRecordSize(const Type *Type,
                                    const std::string &Triple) {
  std::string PreDecl;
  std::string Code = TypeStringify(Type, nullptr, nullptr, PreDecl, "", "dummy;");
  auto Size = ::GetRecordSize(PreDecl + Code, Triple);
  return Size;
}

// Get the align under a specific triple
CharUnits::QuantityType WrapperGenerator::GetRecordAlign(const Type *Type,
                                     const std::string &Triple) {
  std::string PreDecl{};
  std::string Code = TypeStringify(Type, nullptr, nullptr, PreDecl, "", "dummy;");
  return ::GetRecordAlign(PreDecl + Code, Triple);
}

// Generate the func sig by type, used for export func
std::string WrapperGenerator::GetFuncSig(ASTContext *CTX,
                                         const FuncInfo &Func) {
  std::string sig;
  auto Decl = Func.decl;
  auto Type = Decl->getType().getTypePtr();
  auto ProtoType = Type->getAs<FunctionProtoType>();
  auto RetType = ProtoType->getReturnType();

  sig += TypeToSig(CTX, RetType.getTypePtr());
  sig += "F";
  if (Func.has_special_arg || Func.has_special_ret || Func.has_callback_arg) {
    sig += "E";
  }
  if (ProtoType->getNumParams()) {
    for (unsigned i = 0; i < ProtoType->getNumParams(); i++) {
      sig += TypeToSig(CTX, ProtoType->getParamType(i).getTypePtr());
    }
  } else {
    sig += "v";
  }
  if (Decl->isVariadic()) {
    sig += "VV";
  }
  return sig;
}

// Generate the func sig by type, used for callbacks
std::string WrapperGenerator::GetFuncSig(ASTContext *CTX,
                                         const Type *Type) {
  std::string sig;
  auto ProtoType = Type->getAs<FunctionProtoType>();
  auto RetType = ProtoType->getReturnType();
  sig += TypeToSig(CTX, RetType.getTypePtr());
  sig += "F";
  if (ProtoType->getNumParams()) {
    for (unsigned i = 0; i < ProtoType->getNumParams(); i++) {
      sig += TypeToSig(CTX, ProtoType->getParamType(i).getTypePtr());
    }
  } else {
    sig += "v";
  }
  return sig;
}
