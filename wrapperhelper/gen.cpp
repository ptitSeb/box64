#include "gen.h"
#include "utils.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/Basic/LLVM.h>

// Prepare for generation, collect the structures and functions that need to be prcessed
void WrapperGenerator::Prepare(clang::ASTContext *Ctx) {
  for (const auto &func_pair : funcs) {
    for (auto Type : func_pair.second.callback_args) {
      if (Type && Type->isTypedefNameType()) {
        callbacks[StripTypedef(Type->getPointeeType())] =
            Type->getAs<clang::TypedefType>()->getDecl()->getNameAsString();
      } else if (Type) {
        callbacks[StripTypedef(Type->getPointeeType())] =
            GetFuncSig(Ctx, Type->getPointeeType().getTypePtr()) + "_t";
      }
    }
  }
  std::vector<const clang::Type *> Types;
  for (const auto &record_pair : records) {
    auto Record = &record_pair.second;
    Types.push_back(record_pair.first);
  }
  for (auto Type : Types) {
    std::set<const clang::Type *> Visited{Type};
    bool Special = false;
    ParseRecordRecursive(Ctx, Type, Special, Visited);
  }
  for (auto it = records.begin(); it != records.end();) {
    if (!it->second.is_special) {
      it = records.erase(it);
    } else {
      for (auto Type : it->second.callback_fields) {
        if (Type->isTypedefNameType()) {
          callbacks[StripTypedef(Type->getPointeeType())] =
              Type->getAs<clang::TypedefType>()->getDecl()->getNameAsString();
        } else {
          callbacks[StripTypedef(Type->getPointeeType())] =
              GetFuncSig(Ctx, Type->getPointeeType().getTypePtr()) + "_t";
        }
      }
      ++it;
    }
  }
  for (auto &func_pair : funcs) {
    for (int i = 0; i < func_pair.second.decl->getNumParams(); i++) {
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
std::string WrapperGenerator::GenCallbackTypeDefs(clang::ASTContext *Ctx) {
  std::string res{};
  for (auto callback : callbacks) {
    auto Type = callback.first;
    auto Definiton = GetFuncDefinition(Type);
    res += "typedef " + Definiton.ret_str + "(*" + callback.second + ")(";
    for (int i = 0; i < Definiton.arg_size - 1; i++) {
      res += Definiton.arg_types_str[i];
      res += " ";
      res += Definiton.arg_names[i];
      res += ", ";
    }
    if (Definiton.arg_size) {
      res += Definiton.arg_types_str[Definiton.arg_size - 1];
      res += " ";
      res += Definiton.arg_names[Definiton.arg_size - 1];
    }
    res += ");";
    res += "\n";
  }
  return res;
}

// Gen function declare
std::string WrapperGenerator::GenDeclare(clang::ASTContext *Ctx,
                                         const FuncInfo &Func) {
  std::string res{};
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
std::string WrapperGenerator::GenDeclare(clang::ASTContext *Ctx,
                                         const RecordInfo &Record) {
  std::string RecordStr{};
  RecordStr += "typedef ";
  RecordStr +=
      (Record.is_union ? "union " : "struct ") + Record.type_name + " {\n";
  for (const auto &Field : Record.decl->fields()) {
    auto Type = Field->getType();
    std::string Name = Field->getNameAsString();
    RecordStr += "  ";
    if (Type->isFunctionPointerType()) {
      auto FuncType = StripTypedef(Type->getPointeeType());
      if (callbacks.count(FuncType)) {
        std::string FieldStr = callbacks[FuncType];
        FieldStr += " ";
        FieldStr += Name;
        RecordStr += FieldStr;
      } else {
        std::cout << "Err: "
                  << "FuncPtr(" << Record.type_name << "." << Name
                  << ") is not supported\n";
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
          RecordStr += "void * " + Name;
        }
      } else {
        RecordStr += "void * " + Name;
      }
    } else if (Type->isRecordType()) {
      if (records.count(Type.getTypePtr())) {
        std::string FieldStr = records[Type.getTypePtr()].type_name;
        FieldStr += " ";
        FieldStr += Name;
        RecordStr += FieldStr;
      } else {
        RecordStr += TypeStrify(StripTypedef(Type), Field, nullptr);
      }
    } else {
      RecordStr += TypeStrify(StripTypedef(Type), Field, nullptr);
    }
    RecordStr += ";\n";
  }
  RecordStr += "}";
  RecordStr += Record.type_name + ", *" + Record.type_name + "_ptr;\n";
  return RecordStr;
}

std::string WrapperGenerator::GenCallbackWrap(clang::ASTContext *Ctx,
                                              const FuncInfo &Func) {
  std::string res{};

  for (int i = 0; i < Func.decl->getNumParams(); i++) {
    auto ParamType = Func.decl->getParamDecl(i)->getType();
    if (ParamType->isFunctionPointerType()) {

      auto PointeeType = ParamType->getPointeeType();
      auto Definition = GetFuncDefinition(PointeeType.getTypePtr());
      std::string my_funcname =
          std::string("my_") + Func.decl->getParamDecl(i)->getNameAsString();
      std::string funcname =
          std::string("my_") + Func.decl->getParamDecl(i)->getNameAsString();
      res += "#define GO(A) \\\n";
      res +=
          std::string("static uintptr_t ") + my_funcname + "_fct_##A = 0; \\\n";
      res += Definition.ret_str + " " + my_funcname + "(";
      int arg_size = Definition.arg_names.size();
      if (arg_size) {
        for (int i = 0; i < arg_size - 1; i++) {
          res += Definition.arg_types_str[i];
          res += " ";
          res += Definition.arg_names[i];
          res += ", ";
        }
        res += Definition.arg_types_str[arg_size - 1];
        res += " ";
        res += Definition.arg_names[arg_size - 1];
      }
      res += ") {\\\n";
      res += "    ";
      res +=
          "return RunFunction(my_context, " + my_funcname + "_fct_##A" + ", ",
          std::to_string(arg_size);
      if (arg_size) {
        for (int i = 0; i < arg_size; i++) {
          res += ", " + Definition.arg_names[i];
        }
      }
      res += ");\\\n";
      res += "}\n";
      res += "#undef GO\n";
      res += "static void* find" + funcname + "Fct(void* fct) {\n";
      res += "  if(!fct) return fct;\n";
      res += "  if(GetNativeFnc((uintptr_t)fct))  return "
             "GetNativeFnc((uintptr_t)fct);\n";
      res += "  #define GO(A) if(" + my_funcname +
             "_fct_##A == (uintptr_t)fct) return " + my_funcname + "_##A;}\n";
      res += "  SUPER()\n";
      res += "  #undef GO\n";
      res += "  #define GO(A) if(" + my_funcname + "_fct_##A == 0) {" +
             my_funcname + "_fct_##A = (uintptr_t)fct;" + "return " +
             my_funcname + "_##A;}\n";
      res += "  SUPER()\n";
      res += "  #undef GO\n";
      res += "  return NULL;\n";
      res += "}\n";
    }
  }
  return res;
}

std::string WrapperGenerator::GenCallbackWrap(clang::ASTContext *Ctx,
                                              const RecordInfo &Struct) {
  std::string res{};
  auto Type = Struct.type;
  auto RecordType = Type->getAs<clang::RecordType>();
  for (const auto &field : Struct.decl->fields()) {
    auto FieldType = field->getType();
    if (FieldType->isFunctionPointerType()) {
      auto PointeeType = FieldType->getPointeeType();
      auto Definition = GetFuncDefinition(PointeeType.getTypePtr());
      std::string my_funcname = std::string("my_") + field->getNameAsString();
      std::string funcname = field->getNameAsString();
      res += "#define GO(A) \\\n";
      res +=
          std::string("static uintptr_t ") + my_funcname + "_fct_##A = 0;\\\n";
      res += Definition.ret_str + " " + my_funcname + "_##A(";
      int arg_size = Definition.arg_names.size();
      if (arg_size) {
        for (int i = 0; i < arg_size - 1; i++) {
          res += Definition.arg_types_str[i];
          res += " ";
          res += Definition.arg_names[i];
          res += ", ";
        }
        res += Definition.arg_types_str[arg_size - 1];
        res += " ";
        res += Definition.arg_names[arg_size - 1];
      }
      res += ") {\\\n";
      res += "    ";
      res += "return RunFunction(my_context, " + my_funcname + "_fct_##A" +
             ", " + std::to_string(arg_size);
      if (arg_size) {
        for (int i = 0; i < arg_size; i++) {
          res += ", " + Definition.arg_names[i];
        }
      }
      res += ");\\\n";
      res += "}\n";
      res += "#undef GO\n";
      res += "static void* find" + funcname + "Fct(void* fct) {\n";
      res += "  if(!fct) return fct;\n";
      res += "  if(GetNativeFnc((uintptr_t)fct))  return "
             "GetNativeFnc((uintptr_t)fct);\n";
      res += "  #define GO(A) if(" + my_funcname +
             "_fct_##A == (uintptr_t)fct) return " + my_funcname + "_##A;}\n";
      res += "  SUPER()\n";
      res += "  #undef GO\n";
      res += "  #define GO(A) if(" + my_funcname + "_fct_##A == 0) {" +
             my_funcname + "_fct_##A = (uintptr_t)fct;" + "return " +
             my_funcname + "_##A;}\n";
      res += "  SUPER()\n";
      res += "  #undef GO\n";
      res += "  return NULL;\n";
      res += "}\n";
    }
  }
  return res;
}

std::string WrapperGenerator::GenDefine(clang::ASTContext *Ctx,
                                        const FuncInfo &Func) {
  std::string Res{};
  auto Definition = GetFuncDefinition(Func.decl);
  std::string Sig = GetFuncSig(Ctx, Func.type);
  Res += "EXPORT ";
  Res += Definition.ret_str;
  Res += "my_" + Func.func_name + "(";
  if (Sig.find('E')) {
    Res += "void* emu, ";
  }
  int arg_size = Definition.arg_names.size();
  if (arg_size) {
    for (int i = 0; i < arg_size - 1; i++) {
      if (Definition.arg_types[i]->isPointerType()) {
        auto PointeeType = Definition.arg_types[i]->getPointeeType();
        if (records.count(
                Definition.arg_types[i]->getPointeeType().getTypePtr())) {
          Res +=
              Definition.arg_types[i]->getCanonicalTypeInternal().getAsString();
        } else {
          Res += Definition.arg_types_str[i];
        }
      } else {
        Res += Definition.arg_types_str[i];
      }
      Res += " ";
      Res += Definition.arg_names[i];
      Res += ", ";
    }
    if (Definition.arg_types[arg_size - 1]->isPointerType()) {
      auto PointeeType = Definition.arg_types[arg_size - 1]->getPointeeType();
      if (records.count(Definition.arg_types[arg_size - 1]
                            ->getPointeeType()
                            .getTypePtr())) {
        Res += Definition.arg_types[arg_size - 1]
                   ->getCanonicalTypeInternal()
                   .getAsString();
      } else {
        Res += Definition.arg_types_str[arg_size - 1];
      }
    } else {
      Res += Definition.arg_types_str[arg_size - 1];
    }
    Res += " ";
    Res += Definition.arg_names[arg_size - 1];
  }
  Res += ") {\n";
  std::string FuncBodyStr{};
  if (Func.has_special_arg) {
    FuncBodyStr += "  // WARN: This function's arg has structure ptr which is "
                   "special, may be need wrap it for host\n";

  } else if (Func.has_special_ret) {
    FuncBodyStr += "  // WARN: This function's ret structure ptr which is "
                   "special, may be need wrap it for guest\n";
  }
  if (Func.has_callback_arg) {
    FuncBodyStr += "  " + my_lib_type + "my = " + "(" + my_lib_type + ")" +
                   my_lib + "->priv.w.p2;\n";
    FuncBodyStr += "  my->" + Func.func_name + "(";
    if (arg_size) {
      for (int i = 0; i < arg_size - 1; i++) {
        if (Func.callback_args[i]) {
          if (!Func.callback_args[i]->isTypedefNameType()) {
            FuncBodyStr +=
                "find" + Func.func_name + "_arg" + std::to_string(i) + "Fct";
          } else {
            FuncBodyStr += "find" +
                           Func.callback_args[i]
                               ->getAs<clang::TypedefType>()
                               ->getDecl()
                               ->getNameAsString() +
                           "Fct";
          }
          FuncBodyStr += "(" + Definition.arg_names[i] + ")";
        } else {
          FuncBodyStr += Definition.arg_names[i];
        }
        FuncBodyStr += ", ";
      }
      if (Func.callback_args[arg_size - 1]) {
        if (!Func.callback_args[arg_size - 1]->isTypedefNameType()) {
          FuncBodyStr += "find" + Func.func_name + "_arg" +
                         std::to_string(arg_size - 1) + "Fct";
        } else {
          FuncBodyStr += "find" +
                         Func.callback_args[arg_size - 1]
                             ->getAs<clang::TypedefType>()
                             ->getDecl()
                             ->getNameAsString() +
                         "Fct";
        }
        FuncBodyStr += "(" + Definition.arg_names[arg_size - 1] + ")";
      } else {
        FuncBodyStr += Definition.arg_names[arg_size - 1];
      }
      FuncBodyStr += ")\n";
    }
  } else {
    FuncBodyStr += "  " + my_lib_type + "my = " + "(" + my_lib_type + ")" +
                   my_lib + "->priv.w.p2;\n";
    FuncBodyStr += "  my->" + Func.func_name + "(";
    if (arg_size) {
      for (int i = 0; i < arg_size - 1; i++) {
        FuncBodyStr += Definition.arg_names[i];
        FuncBodyStr += ", ";
      }
      FuncBodyStr += Definition.arg_names[arg_size - 1];
      FuncBodyStr += ");\n";
    }
  }

  Res += FuncBodyStr;
  Res += "}\n";
  return Res;
}

std::string WrapperGenerator::GenDeclareDiffTriple(
    clang::ASTContext *Ctx, const RecordInfo &Record,
    const std::string &GuestTriple, const std::string &HostTriple) {
  std::string GuestRecord{};
  std::string HostRecord{};
  std::vector<int> GuestFieldOff;
  std::vector<int> HostFieldOff;
  GuestRecord += "typedef ";
  HostRecord += "typedef ";
  GuestRecord +=
      (Record.is_union ? "union " : "struct ") + Record.type_name + " {\n";
  HostRecord += (Record.is_union ? "union " : "struct ") +
                std::string("host_") + Record.type_name + " {\n";
  auto OffDiff = GetRecordFieldOffDiff(Record.type, GuestTriple, HostTriple,
                                       GuestFieldOff, HostFieldOff);
  int GuestRecordSize = GetRecordSize(Record.type, GuestTriple);
  int HostRecordSize = GetRecordSize(Record.type, HostTriple);
  int SizeDiff = GuestRecordSize - HostRecordSize;
  int FieldIndex = 0;
  std::set<clang::FieldDecl *> AlignDiffFields;
  for (const auto &Field : Record.decl->fields()) {
    if (OffDiff[FieldIndex] == 0) {
      FieldIndex++;
      continue;
    }
    auto Type = Field->getType();
    std::string Name = Field->getNameAsString();
    if (OffDiff[FieldIndex] != SizeDiff) {
      auto Diff = OffDiff[FieldIndex];
      AlignDiffFields.insert(Field);
      for (int i = FieldIndex; i < OffDiff.size(); i++) {
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
      switch (GetTypeSize(StripTypedef(Field->getType()), guest_triple)) {
      // FIXME: should test more case in different triple
      case 4:
        GuestRecord += "int " + Name;
      case 8:
        GuestRecord += "int " + Name + "[2]";
      default:
        break;
      }
      HostRecord += TypeStrify(StripTypedef(Type), Field, nullptr);
    } else if (Type->isFunctionPointerType()) {
      auto FuncType = StripTypedef(Type->getPointeeType());
      if (callbacks.count(FuncType)) {
        std::string FieldStr = callbacks[FuncType];
        FieldStr += " ";
        FieldStr += Name;
        GuestRecord += FieldStr;
        HostRecord += FieldStr;
      } else {
        std::cout << "Err: "
                  << "FuncPtr(" << Record.type_name << "." << Name
                  << ") is not supported\n";
      }
    } else if (Type->isPointerType()) {
      auto PointeeType = Type->getPointeeType();
      if (PointeeType->isRecordType()) {
        if (records.count(PointeeType.getTypePtr())) {
          std::string FieldStr = records[PointeeType.getTypePtr()].type_name;
          FieldStr += "_ptr ";
          FieldStr += Name;
          GuestRecord += FieldStr;
          HostRecord += "host_" + FieldStr;
        } else {
          GuestRecord += "void * " + Name;
          HostRecord += "void * " + Name;
        }
      } else {
        GuestRecord += "void * " + Name;
        HostRecord += "void * " + Name;
      }
    } else if (Type->isRecordType()) {
      if (records.count(Type.getTypePtr())) {
        std::string FieldStr = records[Type.getTypePtr()].type_name;
        FieldStr += " ";
        FieldStr += Name;
        GuestRecord += FieldStr;
        HostRecord += "host_" + FieldStr;
      } else {
        GuestRecord += TypeStrify(StripTypedef(Type), Field, nullptr);
        HostRecord += TypeStrify(StripTypedef(Type), Field, nullptr);
      }
    } else {
      HostRecord += TypeStrify(StripTypedef(Type), Field, nullptr);
      GuestRecord += TypeStrify(StripTypedef(Type), Field, nullptr);
    }
    GuestRecord += ";\n";
    HostRecord += ";\n";
  }
  GuestRecord += "}";
  GuestRecord += Record.type_name + ", *" + Record.type_name + "_ptr;\n";

  HostRecord += "}";
  HostRecord +=
      "host_" + Record.type_name + ", *host_" + Record.type_name + "_ptr;\n";
  return GuestRecord + HostRecord;
}

// Gen record convert function between host and guest
std::string WrapperGenerator::GenRecordConvert(const RecordInfo &Record) {
  std::string res{};
  if (Record.guest_size != Record.host_size) {
    auto RecordDecl = Record.decl;
    std::vector<int> GuestFieldOff;
    std::vector<int> HostFieldOff;
    auto OffDiff = GetRecordFieldOffDiff(Record.type, guest_triple, host_triple,
                                         GuestFieldOff, HostFieldOff);
    int FieldIndex = 0;
    std::vector<clang::FieldDecl *> AlignDiffFields;
    int SizeDiff = Record.guest_size - Record.host_size;
    for (const auto &Field : RecordDecl->fields()) {
      if (OffDiff[FieldIndex] == 0) {
        FieldIndex++;
        continue;
      }
      auto Type = Field->getType();
      std::string Name = Field->getNameAsString();
      if (OffDiff[FieldIndex] != SizeDiff) {
        auto Diff = OffDiff[FieldIndex];
        AlignDiffFields.push_back(Field);
        for (int i = FieldIndex; i < OffDiff.size(); i++) {
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
    res += "void g2h_" + Record.type_name + "(" + "struct host_" +
           Record.type_name + "* d, struct" + Record.type_name + "* s) {\n";
    std::string body = "  memcpy(d, s, offsetof(struct " + Record.type_name +
                       ", " + AlignDiffFields[0]->getNameAsString() + "));\n";
    std::string offstr = "offsetof(struct " + Record.type_name + ", " +
                         AlignDiffFields[0]->getNameAsString() + ")";
    for (int i = 1; i < AlignDiffFields.size() - 1; i++) {
      body += "  memcpy(d->" + AlignDiffFields[i]->getNameAsString() + ", " +
              "s->" + AlignDiffFields[i]->getNameAsString() + ", " +
              "offsetof(struct " + Record.type_name + ", " +
              AlignDiffFields[i + 1]->getNameAsString() + ") - " + offstr +
              ");\n";
      offstr = "offsetof(struct " + Record.type_name + ", " +
               AlignDiffFields[i + 1]->getNameAsString() + ")";
    }
    body += "  memcpy(d->" +
            AlignDiffFields[AlignDiffFields.size() - 1]->getNameAsString() +
            ", " + "s->" +
            AlignDiffFields[AlignDiffFields.size() - 1]->getNameAsString() +
            ", " + std::to_string(GetRecordSize(Record.type, guest_triple)) +
            " - " + offstr + ");\n";
    res += body;
    res += "}\n";

    res += "void h2g_" + Record.type_name + "(struct" + Record.type_name +
           "* d, " + "struct host_" + Record.type_name + "* s) {\n";
    res += body;
    res += "}\n";
  }
  return res;
}

void WrapperGenerator::ParseRecordRecursive(
    clang::ASTContext *Ctx, const clang::Type *Type, bool &Special,
    std::set<const clang::Type *> &Visited) {
  auto RecordType = Type->getAs<clang::RecordType>();
  auto RecordDecl = RecordType->getDecl();
  for (const auto &field : RecordDecl->fields()) {
    auto FieldType = field->getType();
    if (FieldType->isFunctionPointerType()) {
      auto Record = &records[Type];
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
  int GuestSize = GetRecordSize(Type, guest_triple);
  int HostSize = GetRecordSize(Type, host_triple);

  auto Record = &records[Type];
  if (GuestSize != HostSize) {
    Special = 1;
  }
  if (Type->isUnionType()) {
    Record->is_union = true;
  }
  if (!Record->decl) {
    Record->type = Type;
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
std::string WrapperGenerator::TypeStrify(const clang::Type *Type,
                                         clang::FieldDecl *FieldDecl,
                                         clang::ParmVarDecl *ParmDecl,
                                         std::string Name) {
  std::string res{};
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
        res += PointeeType->isUnionType() ? "union {\n" : "struct ";
        res += records[StripTypedef(PointeeType)].type_name;
      } else {
        res += "void";
      }
    } else {
      res += "void";
    }
    res += " * " + name;
  } else if (Type->isEnumeralType()) {
    res += "int ";
    res += name;
  } else if (Type->isRecordType()) {
    if (records.find(StripTypedef(Type->getCanonicalTypeInternal())) !=
            records.end() &&
        records[StripTypedef(Type->getCanonicalTypeInternal())].is_special) {
      res += Type->isUnionType() ? "union {\n" : "struct ";
      res += records[StripTypedef(Type->getCanonicalTypeInternal())].type_name;
      res += " ";
    } else {
      res += AnonRecordDecl(Type->getAs<clang::RecordType>());
    }
    res += name;
  } else if (Type->isConstantArrayType()) {
    auto ArrayType =
        clang::dyn_cast<clang::ConstantArrayType>(Type->getAsArrayTypeUnsafe());
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
  return res;
}

// Type to String, less detail
std::string WrapperGenerator::SimpleTypeStrify(const clang::Type *Type,
                                               clang::FieldDecl *FieldDecl,
                                               clang::ParmVarDecl *ParmDecl,
                                               std::string Name) {
  std::string res{};
  std::string name = FieldDecl
                         ? FieldDecl->getNameAsString()
                         : (ParmDecl ? ParmDecl->getNameAsString() : Name);
  if (Type->isPointerType()) {
    res += "void * " + name;
  } else if (Type->isEnumeralType()) {
    res += "int ";
    res += name;
  } else if (Type->isRecordType()) {
    if (records.find(StripTypedef(Type->getCanonicalTypeInternal())) !=
        records.end()) {
      res += Type->isUnionType() ? "union {\n" : "struct ";
      res += records[StripTypedef(Type->getCanonicalTypeInternal())].type_name;
      res += " ";
    } else {
      res += SimpleAnonRecordDecl(Type->getAs<clang::RecordType>());
    }
    res += name;
  } else if (Type->isConstantArrayType()) {
    auto ArrayType =
        clang::dyn_cast<clang::ConstantArrayType>(Type->getAsArrayTypeUnsafe());
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
  return res;
}

std::string WrapperGenerator::AnonRecordDecl(const clang::RecordType *Type) {
  auto RecordDecl = Type->getDecl();
  std::string res{};
  res += Type->isUnionType() ? "union {\n" : "struct {\n";
  for (const auto &field : RecordDecl->fields()) {
    auto FieldType = field->getType();
    res += "    ";
    res += TypeStrify(StripTypedef(FieldType), field, nullptr);
    res += ";\n";
  }
  res += "    } ";
  return res;
}

std::string
WrapperGenerator::SimpleAnonRecordDecl(const clang::RecordType *Type) {
  auto RecordDecl = Type->getDecl();
  std::string res{};
  res += Type->isUnionType() ? "union {\n" : "struct {\n";
  for (const auto &field : RecordDecl->fields()) {
    auto FieldType = field->getType();
    res += "    ";
    res += SimpleTypeStrify(StripTypedef(FieldType), field, nullptr);
    res += ";\n";
  }
  res += "    } ";
  return res;
}

// Get func info from FunctionType
FuncDefinition WrapperGenerator::GetFuncDefinition(const clang::Type *Type) {
  FuncDefinition res;
  auto ProtoType = Type->getAs<clang::FunctionProtoType>();
  res.ret = StripTypedef(ProtoType->getReturnType());
  res.ret_str =
      TypeStrify(StripTypedef(ProtoType->getReturnType()), nullptr, nullptr);
  for (int i = 0; i < ProtoType->getNumParams(); i++) {
    auto ParamType = ProtoType->getParamType(i);
    res.arg_types.push_back(StripTypedef(ParamType));
    res.arg_types_str.push_back(
        TypeStrify(StripTypedef(ParamType), nullptr, nullptr));
    res.arg_names.push_back(std::string("a") + std::to_string(i));
  }
  if (ProtoType->isVariadic()) {
    res.is_variadaic = true;
  }
  res.arg_size = ProtoType->getNumParams();
  return res;
}

// Get funcdecl info from FunctionDecl
FuncDefinition WrapperGenerator::GetFuncDefinition(clang::FunctionDecl *Decl) {
  FuncDefinition res;
  auto RetType = Decl->getReturnType();
  res.ret = RetType.getTypePtr();
  res.ret_str = TypeStrify(StripTypedef(RetType), nullptr, nullptr, "");
  for (int i = 0; i < Decl->getNumParams(); i++) {
    auto ParamDecl = Decl->getParamDecl(i);
    auto ParamType = ParamDecl->getType();
    res.arg_types.push_back(ParamType.getTypePtr());
    res.arg_types_str.push_back(
        TypeStrify(StripTypedef(ParamType), nullptr, nullptr, ""));
    res.arg_names.push_back(ParamDecl->getNameAsString());
  }
  if (Decl->isVariadic()) {
    res.is_variadaic = true;
  }
  return res;
}

// Get the offset diff between two different triple
std::vector<int> WrapperGenerator::GetRecordFieldOffDiff(
    const clang::Type *Type, const std::string &GuestTriple,
    const std::string &HostTriple, std::vector<int> &GuestFieldOff,
    std::vector<int> &HostFieldOff) {
  std::string Code = TypeStrify(Type, nullptr, nullptr, "dummy;");
  return ::GetRecordFieldOffDiff(Code, GuestTriple, HostTriple, GuestFieldOff,
                                 HostFieldOff);
}

// Get the size under a specific triple
int WrapperGenerator::GetRecordSize(const clang::Type *Type,
                                    const std::string &Triple) {
  std::string Code = TypeStrify(Type, nullptr, nullptr, "dummy;");
  return ::GetRecordSize(Code, Triple);
}

// Get the align under a specific triple
int WrapperGenerator::GetRecordAlign(const clang::Type *Type,
                                     const std::string &Triple) {
  std::string Code = TypeStrify(Type, nullptr, nullptr, "dummy;");
  return ::GetRecordAlign(Code, Triple);
}

// Generate the func sig by type, used for export func
std::string WrapperGenerator::GetFuncSig(clang::ASTContext *CTX,
                                         const FuncInfo &Func) {
  std::string sig{};
  auto Decl = Func.decl;
  auto Type = Decl->getType().getTypePtr();
  auto ProtoType = Type->getAs<clang::FunctionProtoType>();
  auto RetType = ProtoType->getReturnType();

  sig += TypeToSig(CTX, RetType.getTypePtr());
  sig += "F";
  if (Func.has_special_arg || Func.has_special_ret || Func.has_callback_arg) {
    sig += "E";
  }
  if (ProtoType->getNumParams()) {
    for (int i = 0; i < ProtoType->getNumParams(); i++) {
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
std::string WrapperGenerator::GetFuncSig(clang::ASTContext *CTX,
                                         const clang::Type *Type) {
  std::string sig{};
  auto ProtoType = Type->getAs<clang::FunctionProtoType>();
  auto RetType = ProtoType->getReturnType();
  sig += TypeToSig(CTX, RetType.getTypePtr());
  sig += "F";
  if (ProtoType->getNumParams()) {
    for (int i = 0; i < ProtoType->getNumParams(); i++) {
      sig += TypeToSig(CTX, ProtoType->getParamType(i).getTypePtr());
    }
  } else {
    sig += "v";
  }
  return sig;
}