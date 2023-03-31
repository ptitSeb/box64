#include "ast.h"
#include "utils.h"


void dump_usage() {
    std::string Usage = R"usage(
        usage: command <filename> <libname> [guest_triple] [host_triple] -- <clang_flags>
                <filename> : set the header file to be parsed
                <libname>  : set libname required for wrapping func
                [guest_triple]: set guest triple arm32/arm64/x86/x64, default is x86
                [host_triple]: set host tripe arm32/arm64/x86/x64, default is arm32
                -- : is necessary
    )usage";
    std::cerr << Usage << std::endl;
}

std::string parse_triple(const char* arg) {
    if (strcmp(arg, "arm32") == 0) {
        return TripleName[ARM32];
    } else if (strcmp(arg, "arm64") == 0) {
        return TripleName[ARM64];
    } else if (strcmp(arg, "x86") == 0) {
        return TripleName[X86];
    } else if (strcmp(arg, "x64") == 0) {
        return TripleName[X64];
    } else {
        return "";
    }
}

using namespace clang::tooling;
int main(int argc, const char* argv[]) {
    if (argc < 4) {
        dump_usage();
        return 0;
    }
    std::string libname = argv[2];
    std::string guest_triple = TripleName[X64];
    std::string host_triple = TripleName[ARM64];
    if (argc >= 5) {
        guest_triple = parse_triple(argv[3]);
    }
    if (argc >= 6) {
        host_triple = parse_triple(argv[4]);
    }
    bool has_nessary_tag = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            has_nessary_tag = true;
            break;
        }
    }
    if (!has_nessary_tag) {
        std::cerr << "Please add '--' after triple arg" << std::endl;
        return 0;
    }
    std::string err;
    auto compile_db = FixedCompilationDatabase::loadFromCommandLine(argc, argv, err);
    ClangTool Tool(*compile_db, {argv[1]});
    return Tool.run(std::make_unique<MyFrontendActionFactory>(libname, host_triple, guest_triple).get());
}
