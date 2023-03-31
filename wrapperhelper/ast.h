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

using namespace clang;
using namespace clang::tooling;

static void ParseParameter(ASTContext* AST, WrapperGenerator* Gen, ParmVarDecl* Decl, FuncInfo* Func) {
    auto ParmType = Decl->getType();
    if (ParmType->isPointerType()) {
        auto PointeeType = ParmType->getPointeeType();
        if (PointeeType->isRecordType()) {
            if (Gen->records.find(StripTypedef(PointeeType)) == Gen->records.end()) {
                auto Record = &Gen->records[StripTypedef(PointeeType)];
                if (PointeeType->isUnionType()) {
                    Record->is_union = true;
                }
                Record->type = StripTypedef(PointeeType);
                Record->decl = PointeeType->getAs<RecordType>()->getDecl();
                Record->type_name = Record->decl->getIdentifier()->getName().str();
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
                    Record->type_name = Record->decl->getIdentifier()->getName().str();
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
            Record->type_name = Record->decl->getIdentifier()->getName().str();
        }
    }
}

static void ParseFunction(ASTContext* AST, WrapperGenerator* Gen, FunctionDecl* Decl) {
    auto Type = Decl->getType().getTypePtr();
    auto FuncInfo = &Gen->funcs[Type];
    FuncInfo->type = Type;
    FuncInfo->func_name = Decl->getNameAsString();
    FuncInfo->decl = Decl;
    FuncInfo->callback_args.resize(Decl->getNumParams());
    if (Decl->getAttr<clang::WeakRefAttr>()) {
        FuncInfo->is_weak = true;
    }
    if (Decl->isVariadic()) {
        FuncInfo->is_variadaic = true;
    }
    for (int i = 0; i < Decl->getNumParams(); i++) {
        auto ParmDecl = Decl->getParamDecl(i);
        if (ParmDecl->getType()->isFunctionPointerType()) {
            FuncInfo->callback_args[i] = ParmDecl->getType().getTypePtr();
            FuncInfo->has_callback_arg = true;
        } else {
            FuncInfo->callback_args[i] = nullptr;
        }
        ParseParameter(AST, Gen, ParmDecl, FuncInfo);
    }
}

class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor> {
public:
    MyASTVisitor(ASTContext* ctx) : Ctx(ctx) {}
    MyASTVisitor(ASTContext* ctx, WrapperGenerator* gen) : Ctx(ctx), Gen(gen) {}

    bool VisitFunctionDecl(FunctionDecl* Decl) {
        ParseFunction(Ctx, Gen, Decl);
        return true;
    }
private:
    ASTContext* Ctx;
    WrapperGenerator* Gen;
};

class MyASTConsumer : public clang::ASTConsumer {
public:
    MyASTConsumer(ASTContext* Context, const std::string& libname, const std::string& host_triple, const std::string& guest_triple)
        : Visitor(Context, &Generator) {
        Generator.Init(libname, host_triple, guest_triple);
    }
    void HandleTranslationUnit(clang::ASTContext &Ctx) override {
        Visitor.TraverseDecl(Ctx.getTranslationUnitDecl());
        std::cout << "--------------- Libclangtooling parse complete -----------------\n";
        Generator.Prepare(&Ctx);
        std::cout << "--------------- Generator prepare complete -----------------\n";
        std::ofstream FuncDeclFile(Generator.libname + "_private.h", std::ios::out);
        FuncDeclFile << Generator.GenFuncDeclare(&Ctx) << std::endl;
        FuncDeclFile.close();
        std::ofstream FuncDefinelFile("wrapped" + Generator.libname + ".c", std::ios::out);
        FuncDefinelFile << Generator.GenCallbackTypeDefs(&Ctx) << std::endl;
        FuncDefinelFile << Generator.GenRecordDeclare(&Ctx) << std::endl;
        FuncDefinelFile << Generator.GenRecordConvert(&Ctx) << std::endl;
        FuncDefinelFile << Generator.GenCallbackWrap(&Ctx) << std::endl;
        FuncDefinelFile << Generator.GenFuncDefine(&Ctx) << std::endl;
        FuncDefinelFile.close();
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