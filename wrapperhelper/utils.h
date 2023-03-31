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

static const clang::Type* StripTypedef(clang::QualType type) {
    if (type->isTypedefNameType()) {
        return StripTypedef(type->getAs<clang::TypedefType>()->getDecl()->getUnderlyingType());
    } else {
        return type.getTypePtr();
    }
}
