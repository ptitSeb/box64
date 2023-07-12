#pragma once
#include <clang/AST/ASTContext.h>
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

static const clang::Type* StripTypedef(clang::QualType type) {
    if (type->isTypedefNameType()) {
        return StripTypedef(type->getAs<clang::TypedefType>()->getDecl()->getUnderlyingType());
    } else {
        return type.getTypePtr();
    }
}

// FIXME: Need to support other triple except default target triple
static std::string GetDeclHeaderFile(clang::ASTContext& Ctx, clang::Decl* Decl) {
    const auto& SourceManager = Ctx.getSourceManager();
    const clang::FileID FileID = SourceManager.getFileID(Decl->getBeginLoc());
    const clang::FileEntry *FileEntry = SourceManager.getFileEntryForID(FileID);
    if (FileEntry) {
        return FileEntry->getName().str();
    }
    return "";
}