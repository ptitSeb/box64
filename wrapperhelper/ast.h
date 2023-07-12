#pragma once
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Attr.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include <ios>
#include <llvm/Support/Casting.h>

#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <map>

#include "gen.h"
#include "utils.h"

static void ParseParameter(clang::ASTContext* AST, WrapperGenerator* Gen, clang::QualType ParmType, FuncInfo* Func) {
    using namespace clang;
    (void)AST; (void)Func;
    if (ParmType->isFunctionPointerType()) {
        auto ProtoType = ParmType->getPointeeType()->getAs<FunctionProtoType>();
        for (unsigned i = 0; i < ProtoType->getNumParams(); i++) {
            ParseParameter(AST, Gen, ProtoType->getParamType(i), Func);
        }
    } else if (ParmType->isPointerType()) {
        auto PointeeType = ParmType->getPointeeType();
        if (PointeeType->isRecordType()) {
            if (Gen->records.find(StripTypedef(PointeeType)) == Gen->records.end()) {
                auto Record = &Gen->records[StripTypedef(PointeeType)];
                if (PointeeType->isUnionType()) {
                    Record->is_union = true;
                }
                Record->type = StripTypedef(PointeeType);
                Record->decl = PointeeType->getAs<RecordType>()->getDecl();
                Record->type_name = Record->decl->getIdentifier() ? Record->decl->getIdentifier()->getName().str() : "<null identifier>";
            }
        } else if (PointeeType->isPointerType()) {
            PointeeType = PointeeType->getPointeeType();
            if (PointeeType->isRecordType()) {
                if (Gen->records.find(StripTypedef(PointeeType)) == Gen->records.end()) {
                    auto Record = &Gen->records[StripTypedef(PointeeType)];
                    if (PointeeType->isUnionType()) {
                        Record->is_union = true;
                    }
                    
                    Record->type = StripTypedef(PointeeType);
                    Record->decl = PointeeType->getAs<RecordType>()->getDecl();
                    Record->type_name = Record->decl->getIdentifier() ? Record->decl->getIdentifier()->getName().str() : "<null identifier>";
                }
            }
        }
    } else if (ParmType->isRecordType()) {
        if (Gen->records.find(StripTypedef(ParmType)) == Gen->records.end()) {
            auto Record = &Gen->records[StripTypedef(ParmType)];
            if (ParmType->isUnionType()) {
                Record->is_union = true;
            }
            Record->type = StripTypedef(ParmType);
            Record->decl = ParmType->getAs<RecordType>()->getDecl();
            Record->type_name = Record->decl->getIdentifier() ? Record->decl->getIdentifier()->getName().str() : "<null identifier>";
        }
    }
}

static void ParseFunction(clang::ASTContext* AST, WrapperGenerator* Gen, clang::FunctionDecl* Decl) {
    using namespace clang;
    auto Type = Decl->getType().getTypePtr();
    auto FuncInfo = &Gen->funcs[Type];
    FuncInfo->type = Type;
    FuncInfo->func_name = Decl->getNameAsString();
    FuncInfo->decl = Decl;
    FuncInfo->callback_args.resize(Decl->getNumParams());
    if (Decl->getAttr<WeakRefAttr>()) {
        FuncInfo->is_weak = true;
    }
    if (Decl->isVariadic()) {
        FuncInfo->is_variadaic = true;
    }
    for (unsigned i = 0; i < Decl->getNumParams(); i++) {
        auto ParmDecl = Decl->getParamDecl(i);
        if (ParmDecl->getType()->isFunctionPointerType()) {
            FuncInfo->callback_args[i] = ParmDecl->getType().getTypePtr();
            FuncInfo->has_callback_arg = true;
        } else {
            FuncInfo->callback_args[i] = nullptr;
        }
        ParseParameter(AST, Gen, ParmDecl->getType(), FuncInfo);
    }
}

class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor> {
public:
    MyASTVisitor(clang::ASTContext* ctx) : Ctx(ctx) {}
    MyASTVisitor(clang::ASTContext* ctx, WrapperGenerator* gen) : Ctx(ctx), Gen(gen) {}

    bool VisitFunctionDecl(clang::FunctionDecl* Decl) {
        ParseFunction(Ctx, Gen, Decl);
        return true;
    }
private:
    clang::ASTContext* Ctx;
    WrapperGenerator* Gen;
};

class MyASTConsumer : public clang::ASTConsumer {
public:
    MyASTConsumer(clang::ASTContext* Context, const std::string& libname, const std::string& host_triple, const std::string& guest_triple)
        : Visitor(Context, &Generator) {
        Generator.Init(libname, host_triple, guest_triple);
    }
    void HandleTranslationUnit(clang::ASTContext &Ctx) override {
        Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
        std::cout << "--------------- Libclangtooling parse complete -----------------\n";
        Generator.Prepare(&Ctx);
        std::cout << "--------------- Generator prepare complete -----------------\n";
        std::ofstream FuncDeclFile("wrapped" + Generator.libname + "_private.h", std::ios::out);
        FuncDeclFile << Generator.GenFuncDeclare(&Ctx);
        FuncDeclFile.close();
        std::ofstream FuncDefineFile("wrapped" + Generator.libname + ".c", std::ios::out);
        FuncDefineFile << "#include <stdio.h>\n"
                          "#include <stdlib.h>\n"
                          "#include <string.h>\n"
                          "#define _GNU_SOURCE         /* See feature_test_macros(7) */\n"
                          "#include <dlfcn.h>\n"
                          "\n"
                          "#include \"wrappedlibs.h\"\n"
                          "\n"
                          "#include \"debug.h\"\n"
                          "#include \"wrapper.h\"\n"
                          "#include \"bridge.h\"\n"
                          "#include \"x64emu.h\"\n"
                          "#include \"box64context.h\"\n"
                          "\n"
                          "const char* " + Generator.libname + "Name = \"" + Generator.libname + "\";\n"
                          "#define LIBNAME " + Generator.libname + "\n"
                          "\n"
                          "#define ADDED_FUNCTIONS()           \\\n"
                          "\n"
                          "#include \"generated/wrapped" + Generator.libname + "types.h\"\n";
        FuncDefineFile << Generator.GenRecordDeclare(&Ctx);
        FuncDefineFile << Generator.GenRecordConvert(&Ctx);
        FuncDefineFile << Generator.GenCallbackWrap(&Ctx);
        FuncDefineFile << Generator.GenFuncDefine(&Ctx);
        FuncDefineFile.close();
        std::cout << "--------------- Generator gen complete -----------------\n";
    }
private:
    MyASTVisitor Visitor;
    WrapperGenerator Generator;
};

class MyGenAction : public clang::ASTFrontendAction {
public:
    MyGenAction(const std::string& libname, const std::string& host_triple, const std::string& guest_triple) :
        libname(libname), host_triple(host_triple), guest_triple(guest_triple)  {}
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& Compiler, clang::StringRef file) override {
        (void)file;
        return std::make_unique<MyASTConsumer>(&Compiler.getASTContext(), libname, host_triple, guest_triple);
    }
private:
    std::string libname;
    std::string host_triple;
    std::string guest_triple;
};

class MyFrontendActionFactory : public clang::tooling::FrontendActionFactory {
public:
    MyFrontendActionFactory(const std::string& libname, const std::string& host_triple, const std::string& guest_triple) : 
        libname(libname), host_triple(host_triple), guest_triple(guest_triple)  {}
private:
    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<MyGenAction>(libname, host_triple, guest_triple);
    }
private:
    std::string libname;
    std::string host_triple;
    std::string guest_triple;
};
