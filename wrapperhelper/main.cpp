#include "ast.h"
#include "utils.h"

static void dump_usage() {
    std::string Usage = R"usage(
        usage: command <filename> <libname> [guest_triple] [host_triple] -- <clang_flags>
                <filename>    : set the header file to be parsed
                <libname>     : set libname required for wrapping func
                [guest_triple]: set guest triple: can be arm32/arm64/x86/x64, default is x64
                [host_triple] : set host triple: can be arm32/arm64/x86/x64, default is arm64
                --            : mandatory
                <clang_flags> : extra compiler flags
    )usage";
    std::cerr << Usage << std::endl;
}

std::string parse_triple(const char* arg) {
    if (strcmp(arg, "x86") == 0) {
        return "i386-pc-linux-gnu";
    } else if (strcmp(arg, "x64") == 0) {
        return "x86_64-pc-linux-gnu";
    } else if (strcmp(arg, "arm32") == 0) {
        return "armv7-unknown-linux-gnueabihf";
    } else if (strcmp(arg, "arm64") == 0) {
        return "aarch64-unknown-linux-gnu";
    } else {
        std::cerr << "Invalid triple: '" << arg << "'\n";
        dump_usage();
        return "";
    }
}

int main(int argc, const char* argv[]) {
    if (argc < 4) {
        dump_usage();
        return 0;
    }
    std::string libname = argv[2];
    std::string guest_triple = parse_triple("x64");
    std::string host_triple = parse_triple("arm64");
    if (argc >= 5) {
        guest_triple = parse_triple(argv[3]);
    }
    if (argc >= 6) {
        host_triple = parse_triple(argv[4]);
    }
    bool has_necessary_tag = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "--") == 0) {
            has_necessary_tag = true;
            break;
        }
    }
    if (!has_necessary_tag) {
        std::cerr << "Please add '--' after the triples" << std::endl;
        dump_usage();
        return 0;
    }
    std::string err;
    auto compile_db = clang::tooling::FixedCompilationDatabase::loadFromCommandLine(argc, argv, err);
    clang::tooling::ClangTool Tool(*compile_db, {argv[1]});
    Tool.appendArgumentsAdjuster([&guest_triple](const clang::tooling::CommandLineArguments &args, clang::StringRef) {
        clang::tooling::CommandLineArguments adjusted_args = args;
        adjusted_args.push_back(std::string{"-target"});
        adjusted_args.push_back(guest_triple);
        return adjusted_args;
    });
    return Tool.run(std::make_unique<MyFrontendActionFactory>(libname, host_triple, guest_triple).get());
}
