#pragma once
#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>
#include <cstdint>
#include <string>
#include <vector>
#include <set>

struct FuncDefinition {
    std::vector<const clang::Type*> arg_types;
    std::vector<std::string> arg_types_str;
    std::vector<std::string> arg_names;
    const clang::Type* ret;
    std::string ret_str;
    int arg_size;
    bool is_variadaic;
};

struct FuncInfo {
    const clang::Type* type;
    clang::FunctionDecl* decl;
    std::string func_name;
    bool is_weak;
    bool is_variadaic;
    bool has_special_arg;
    bool has_special_ret;
    bool has_callback_arg;
    std::vector<const clang::Type*> callback_args;
};

struct RecordInfo {
    const clang::Type* type;
    clang::RecordDecl* decl;
    std::string type_name;
    bool is_union;
    bool is_special;

    uint64_t guest_size;
    uint64_t host_size;
    std::vector<const clang::Type*> callback_fields;
};

struct ObjectInfo {
    const clang::Type* type;
    std::string object_name;
};

struct WrapperGenerator {

    void Init(const std::string& libname, const std::string& host_triple, const std::string& guest_triple) {
        this->host_triple = host_triple;
        this->guest_triple = guest_triple;
        this->libname = libname;
        this->my_lib_type = libname + "_my_t";
        this->my_lib = "my_lib";
    }

    void Prepare(clang::ASTContext* Ctx);

    std::string GenCallbackTypeDefs(clang::ASTContext* Ctx);

    std::string GenFuncDeclare(clang::ASTContext* Ctx) {
        std::string res{};
        for (const auto& func : funcs) {
            res += GenDeclare(Ctx, func.second);
        }
        return res;
    }
    std::string GenRecordDeclare(clang::ASTContext* Ctx) {
        std::string res{};
        for (const auto& st : records) {
            if (st.second.host_size == st.second.guest_size)
                res += GenDeclare(Ctx, st.second);
            else {
                res += GenDeclareDiffTriple(Ctx, st.second, guest_triple, host_triple);
            }
        }
        return res;
    }

    std::string GenFuncDefine(clang::ASTContext* Ctx) {
        std::string res{};
        for (const auto& func : funcs) {
            res += GenDefine(Ctx, func.second);
        }
        return res;
    }

    std::string GenCallbackWrap(clang::ASTContext* Ctx) {
        std::string res{};
        for (const auto& func : funcs) {
            res += GenCallbackWrap(Ctx, func.second);
        }
        for (const auto& st : records) {
            res += GenCallbackWrap(Ctx, st.second);
        }
        return res;
    }
    
    std::string GenRecordConvert(clang::ASTContext* Ctx) {
        (void)Ctx;
        std::string res;
        for (const auto& record : records) {
            if (record.second.host_size != record.second.guest_size) {
                res += GenRecordConvert(record.second);
            }
        }
        return res;
    }

    std::map<const clang::Type*, FuncInfo> funcs;
    std::map<const clang::Type*, RecordInfo> records;
    std::map<const clang::Type*, ObjectInfo> objects;

    std::map<const clang::Type*, std::string> callbacks;

    std::string host_triple;
    std::string guest_triple;
    std::string libname;
    std::string my_lib_type;
    std::string my_lib;
private:
    std::string GenRecordConvert(const RecordInfo& Record);
    std::string GenDeclare(clang::ASTContext* Ctx, const FuncInfo& Func);
    std::string GenDefine(clang::ASTContext* Ctx, const FuncInfo& Func);
    std::string GenCallbackWrap(clang::ASTContext* Ctx, const FuncInfo& Func);
    std::string GenDeclareDiffTriple(clang::ASTContext* Ctx, const RecordInfo& Record, const std::string& GuestTriple, const std::string& HostTriple);
    std::string GenDeclare(clang::ASTContext* Ctx, const RecordInfo& Struct);
    std::string GenCallbackWrap(clang::ASTContext* Ctx, const RecordInfo& Struct);

    void ParseRecordRecursive(clang::ASTContext* Ctx, const clang::Type* Type, bool& Special, std::set<const clang::Type*>& Visited);
    std::string TypeStringify(const clang::Type* Type, clang::FieldDecl* FieldDecl, clang::ParmVarDecl* ParmDecl, std::string& PreDecl, std::string indent = "", std::string Name = "");
    std::string SimpleTypeStringify(const clang::Type* Type, clang::FieldDecl* FieldDecl, clang::ParmVarDecl* ParmDecl, std::string indent = "", std::string Name = "");
    std::string AnonRecordDecl(const clang::RecordType* Type, std::string& PreDecl, std::string indent);
    std::string SimpleAnonRecordDecl(const clang::RecordType* Type, std::string indent);
    FuncDefinition GetFuncDefinition(const clang::Type* Type);
    FuncDefinition GetFuncDefinition(clang::FunctionDecl* Decl);
    uint64_t GetRecordSize(const clang::Type* Type, const std::string& Triple);
    std::vector<uint64_t> GetRecordFieldOffDiff(const clang::Type* Type, const std::string& GuestTriple, const std::string& HostTriple, std::vector<uint64_t>& GuestFieldOff, std::vector<uint64_t>& HostFieldOff);
    clang::CharUnits::QuantityType GetRecordAlign(const clang::Type* Type, const std::string& Triple);

    std::string GetFuncSig(clang::ASTContext* CTX, const FuncInfo& Decl);
    std::string GetFuncSig(clang::ASTContext* CTX, const clang::Type* Type);
};
