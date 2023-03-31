#pragma once
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <clang/Tooling/Tooling.h>
#include <clang/AST/RecordLayout.h>
#include <clang/AST/Decl.h>

#include <llvm/ADT/Triple.h>
#include <llvm/Support/Casting.h>

#include <cstddef>
#include <cstring>
#include <iostream>

enum Triple {
    X86,
    X64,
    ARM32,
    ARM64,
    RISCV64,
    TripleCnt,
};

static const char* TripleName[TripleCnt] = {
    "i386-pc-linux-gnu",
    "x86_64-pc-linux-gnu",
    "armv7-unknown-linux-gnueabihf",
    "aarch64-unknown-linux-gnu",
    "riscv64-unknown-linux-gnu"
};

static const clang::Type* StripTypedef(clang::QualType type) {
    if (type->isTypedefNameType()) {
        return StripTypedef(type->getAs<clang::TypedefType>()->getDecl()->getUnderlyingType());
    } else {
        return type.getTypePtr();
    }
}

static int GetRecordSize(const std::string& Code, const std::string& Triple) {
    std::vector<std::string> Args = {"-target", Triple};
    std::unique_ptr<clang::ASTUnit> AST = clang::tooling::buildASTFromCodeWithArgs(Code, Args);
    auto& Ctx = AST->getASTContext();
    auto TranslateDecl = Ctx.getTranslationUnitDecl();
    for (const auto& Decl : TranslateDecl->decls()) {
        if (const auto RecordDecl = clang::dyn_cast<clang::RecordDecl>(Decl)) {
            return Ctx.getTypeSize(RecordDecl->getTypeForDecl()) / 8;
        }
    }
    return 0;
}

static std::vector<int> GetRecordFieldOff(const std::string& Code, const std::string& Triple) {
    std::vector<int> FieldOff;
    std::vector<std::string> Args = {"-target", Triple};
    std::unique_ptr<clang::ASTUnit> AST = clang::tooling::buildASTFromCodeWithArgs(Code, Args);
    auto& Ctx = AST->getASTContext();
    auto TranslateDecl = Ctx.getTranslationUnitDecl();
    for (const auto& Decl : TranslateDecl->decls()) {
        if (const auto RecordDecl = clang::dyn_cast<clang::RecordDecl>(Decl)) {
            auto& RecordLayout = Ctx.getASTRecordLayout(RecordDecl);
            for (int i = 0; i < RecordLayout.getFieldCount(); i++) {
                FieldOff.push_back(RecordLayout.getFieldOffset(i) / 8);
            }
            break;
        }
    }
    return FieldOff;
}

static int GetRecordAlign(const std::string& Code, const std::string& Triple) {
    std::vector<std::string> Args = {"-target", Triple};
    std::unique_ptr<clang::ASTUnit> AST = clang::tooling::buildASTFromCodeWithArgs(Code, Args);
    auto& Ctx = AST->getASTContext();
    auto TranslateDecl = Ctx.getTranslationUnitDecl();
    for (const auto& Decl : TranslateDecl->decls()) {
        if (const auto RecordDecl = clang::dyn_cast<clang::RecordDecl>(Decl)) {
            auto& RecordLayout = Ctx.getASTRecordLayout(RecordDecl);
            for (int i = 0; i < RecordLayout.getFieldCount(); i++) {
                return RecordLayout.getAlignment().getQuantity() / 8;
            }
            break;
        }
    }
    return 0;
}

static std::vector<int> GetRecordFieldOffDiff(const std::string& Code, const std::string& GuestTriple, const std::string& HostTriple, std::vector<int>& GuestFieldOff, std::vector<int>& HostFieldOff) {
    std::vector<int> OffsetDiff;
    GuestFieldOff = GetRecordFieldOff(Code, GuestTriple);
    HostFieldOff = GetRecordFieldOff(Code, HostTriple);
    if (GuestFieldOff.size() != HostFieldOff.size()) {
        return OffsetDiff;
    }
    for (int i = 0; i < GuestFieldOff.size(); i++) {
        OffsetDiff.push_back(GuestFieldOff[i] - HostFieldOff[i]);
    }
    return OffsetDiff;
}

static int GetTypeSize(const clang::Type* Type, const std::string& Triple) {
    std::string Code = Type->getCanonicalTypeInternal().getAsString() + " dummy;";
    std::vector<std::string> Args = {"-target", Triple};
    std::unique_ptr<clang::ASTUnit> AST = clang::tooling::buildASTFromCodeWithArgs(Code, Args);
    auto& Ctx = AST->getASTContext();
    auto TranslateDecl = Ctx.getTranslationUnitDecl();
    for (const auto& Decl : TranslateDecl->decls()) {
        if (const auto VarDecl = clang::dyn_cast<clang::VarDecl>(Decl)) {
            return Ctx.getTypeSize(VarDecl->getType()) / 8;
        }
    }
    return 0;
}

static int GetTypeAlign(const clang::Type* Type, const std::string& Triple) {
    std::string Code = Type->getCanonicalTypeInternal().getAsString() + " dummy;";
    std::vector<std::string> Args = {"-target", Triple};
    std::unique_ptr<clang::ASTUnit> AST = clang::tooling::buildASTFromCodeWithArgs(Code, Args);
    auto& Ctx = AST->getASTContext();
    auto TranslateDecl = Ctx.getTranslationUnitDecl();
    for (const auto& Decl : TranslateDecl->decls()) {
        if (const auto VarDecl = clang::dyn_cast<clang::VarDecl>(Decl)) {
            return Ctx.getTypeAlign(VarDecl->getType());;
        }
    }
    return 0;
}

static std::string TypeToSig(clang::ASTContext* Ctx, const clang::Type* Type) {
    if (Type->isPointerType()) {
        return "p";
    } else if (Type->isVoidType()) {
        return "v";
    } else if (Type->isUnsignedIntegerOrEnumerationType()) {
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
                std::cout << "Unsupported UnSignedInteger Type: " << Type->getCanonicalTypeInternal().getAsString() << std::endl;
        }
    } else if (Type->isSignedIntegerOrEnumerationType()) {
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
                std::cout << "Unsupported SignedInteger Type: " << Type->getCanonicalTypeInternal().getAsString() << std::endl;
        }
    } else if (Type->isCharType()) {
        return "c";
    } else if (Type->isFloatingType()) {
        switch(Ctx->getTypeSizeInChars(Type).getQuantity()) {
            case 4:
                return "f";
            case 8:
                return  "d";
            default:
                std::cout << "Unsupported Floating Type: " << Type->getCanonicalTypeInternal().getAsString() << std::endl;
        }
    } else {
        std::cout << "Unsupported Type: " << Type->getCanonicalTypeInternal().getAsString() << std::endl;
    }
    return "?";
}
