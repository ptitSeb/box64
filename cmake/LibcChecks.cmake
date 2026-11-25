include(CheckIncludeFile)

set(PROJECT_HEADERS
    execinfo.h
)

foreach(header IN LISTS PROJECT_HEADERS)
    string(MAKE_C_IDENTIFIER "${header}" header_ident)
    string(TOUPPER "${header_ident}" header_ident)

    set(var "HAVE_${header_ident}")

    check_include_file("${header}" "${var}")

    if(${var})
        add_compile_definitions("${var}=1")
    endif()
endforeach()

if(HAVE_EXECINFO_H AND NOT ANDROID)
    add_compile_definitions(USE_BACKTRACE)
endif()

include(CheckSymbolExists)

set(PROJECT_SYMBOL_CHECKS
    mallopt                             malloc.h
    malloc_trim                         malloc.h
    mallinfo                            malloc.h
    mallinfo2                           malloc.h
    _pthread_cleanup_push               pthread.h
    _pthread_cleanup_pop                pthread.h
    pthread_attr_getaffinity_np         pthread.h
    pthread_attr_setaffinity_np         pthread.h
    pthread_mutexattr_setprioceiling    pthread.h
    dlvsym                              dlfcn.h
    obstack_vprintf                     stdio.h
    getprotobyname_r                    netdb.h
    getprotobynumber_r                  netdb.h
    scandirat                           dirent.h
)

list(LENGTH PROJECT_SYMBOL_CHECKS checks_len)
math(EXPR checks_last "${checks_len} - 1")

foreach(i RANGE 0 ${checks_last} 2)
    math(EXPR j "${i} + 1")

    list(GET PROJECT_SYMBOL_CHECKS ${i} symbol)
    list(GET PROJECT_SYMBOL_CHECKS ${j} header)

    string(MAKE_C_IDENTIFIER "${symbol}" symbol_ident)
    string(TOUPPER "${symbol_ident}" symbol_ident)

    set(var "HAVE_${symbol_ident}")

    set(headers "${header}")
    check_symbol_exists("${symbol}" "${headers}" "${var}")

    if(${var})
        add_compile_definitions("${var}=1")
    endif()
endforeach()

include(CheckFunctionExists)

set(PROJECT_FUNCTION_CHECKS
    __chk_fail
)

foreach(function IN LISTS PROJECT_FUNCTION_CHECKS)
    string(MAKE_C_IDENTIFIER "${function}" function_ident)
    string(TOUPPER "${function_ident}" function_ident)

    set(var "HAVE_${function_ident}")

    check_function_exists("${function}" "${var}")

    if(${var})
        add_compile_definitions("${var}=1")
    endif()
endforeach()

include(CheckCSourceCompiles)

check_c_source_compiles("#include <obstack.h>
int main(void){ struct obstack o = { .temp.tempptr = NULL }; return 0; }" OBSTACK_TEMP_TEMPPTR)
check_c_source_compiles("#include <obstack.h>
int main(void){ struct obstack o = { .temp.p = NULL }; return 0; }"       OBSTACK_TEMP_P)

if(OBSTACK_TEMP_TEMPPTR) # glibc
  add_compile_definitions("STRUCT_OBSTACK_TEMP=tempptr")
  add_compile_definitions("STRUCT_OBSTACK_CHUNKFUN=chunkfun")
  add_compile_definitions("STRUCT_OBSTACK_FREEFUN=freefun")
elseif(OBSTACK_TEMP_P) # musl-obstack
  add_compile_definitions("STRUCT_OBSTACK_TEMP=p")
  add_compile_definitions("STRUCT_OBSTACK_CHUNKFUN=chunkfun.extra")
  add_compile_definitions("STRUCT_OBSTACK_FREEFUN=freefun.extra")
elseif(NOT ANDROID)
  message(FATAL_ERROR "Unknown struct obstack layout!")
endif()

check_c_source_compiles("#include <signal.h>
int main(void){ siginfo_t s = { ._sifields = { 0 } }; return 0; }"   SIGINFO_S__SIFIELDS)
check_c_source_compiles("#include <signal.h>
int main(void){ siginfo_t s = { .__si_fields = { 0 } }; return 0; }" SIGINFO_S___SI_FIELDS)

if(SIGINFO_S__SIFIELDS) # glibc
  add_compile_definitions("SIGINFO_S_SIFIELDS=_sifields")
elseif(SIGINFO_S___SI_FIELDS) # musl
  add_compile_definitions("SIGINFO_S_SIFIELDS=__si_fields")
elseif(NOT ANDROID)
  message(FATAL_ERROR "Unknown siginfo_t layout!")
endif()

check_c_source_compiles("#include <resolv.h>
int main(void){ struct __res_state r = { .__glibc_unused_qhook = NULL }; return 0; }"  RES_STATE_GLIBC_UNUSED_QHOOK)
check_c_source_compiles("#include <resolv.h>
int main(void){ struct __res_state r = { .qhook = NULL }; return 0; }"                 RES_STATE_QHOOK)

if(RES_STATE_GLIBC_UNUSED_QHOOK) # glibc
  add_compile_definitions("RES_STATE_QHOOK=__glibc_unused_qhook")
  add_compile_definitions("RES_STATE_RHOOK=__glibc_unused_rhook")
elseif(RES_STATE_QHOOK) # musl
  add_compile_definitions("RES_STATE_QHOOK=qhook")
  add_compile_definitions("RES_STATE_RHOOK=rhook")
elseif(NOT ANDROID)
  message(FATAL_ERROR "Unknown res_state layout!")
endif()

check_c_source_compiles("#include <ctype.h>
int main(void){ return &__ctype_b_loc == 0; }" CTYPE_B_LOC)
check_c_source_compiles("#include <ctype.h>
extern const unsigned short int **__ctype_b_loc(void);
int main(void){ __ctype_b_loc(); return 0; }" CTYPE_B_LOC_EXT)

if(CTYPE_B_LOC) # glibc
  add_compile_definitions("HAVE_CTYPE_B_LOC")
elseif(CTYPE_B_LOC_EXT) # musl
  add_compile_definitions("HAVE_CTYPE_B_LOC")
  add_compile_definitions("HAVE_CTYPE_B_LOC_EXT")
endif()

check_c_source_compiles("#include <glob.h>
int main(void){ glob_t g = { .gl_flags = 0 }; return 0; }" GLOB_T_GL_FLAGS)
check_c_source_compiles("#include <glob.h>
int main(void){ glob_t g = { .__dummy1 = 0 }; return 0; }" GLOB_T_DUMMY)

if(GLOB_T_GL_FLAGS) # glibc
  add_compile_definitions("GLOB_T_GL_FLAGS=gl_flags")
  add_compile_definitions("GLOB_T_GL_CLOSEDIR=gl_closedir")
  add_compile_definitions("GLOB_T_GL_READDIR=gl_readdir")
  add_compile_definitions("GLOB_T_GL_OPENDIR=gl_opendir")
  add_compile_definitions("GLOB_T_GL_LSTAT=gl_lstat")
  add_compile_definitions("GLOB_T_GL_STAT=gl_stat")
elseif(GLOB_T_DUMMY) # musl
  add_compile_definitions("GLOB_T_GL_FLAGS=__dummy1")
  add_compile_definitions("GLOB_T_GL_CLOSEDIR=__dummy2[0]")
  add_compile_definitions("GLOB_T_GL_READDIR=__dummy2[1]")
  add_compile_definitions("GLOB_T_GL_OPENDIR=__dummy2[2]")
  add_compile_definitions("GLOB_T_GL_LSTAT=__dummy2[3]")
  add_compile_definitions("GLOB_T_GL_STAT=__dummy2[4]")
elseif(NOT ANDROID)
  message(FATAL_ERROR "Unknown glob_t layout!")
endif()

check_c_source_compiles("#include <spawn.h>
int main(void){ posix_spawn_file_actions_t a = { .__allocated = 0 }; return 0; }" 
    POSIX_SPAWN_FILE_ACTIONS_T__ALLOCATED)
check_c_source_compiles("#include <spawn.h>
int main(void){ posix_spawn_file_actions_t a = { .__pad0 = {0} }; return 0; }" 
    POSIX_SPAWN_FILE_ACTIONS_T__PAD0)

if(POSIX_SPAWN_FILE_ACTIONS_T__ALLOCATED) # glibc
  add_compile_definitions("POSIX_SPAWN_FILE_ACTIONS_T__ALLOCATED=__allocated")
  add_compile_definitions("POSIX_SPAWN_FILE_ACTIONS_T__USED=__used")
  add_compile_definitions("POSIX_SPAWN_FILE_ACTIONS_T__ACTIONS=__actions")
  add_compile_definitions("POSIX_SPAWN_FILE_ACTIONS_T__PAD=__pad")
elseif(POSIX_SPAWN_FILE_ACTIONS_T__PAD0) # musl
  add_compile_definitions("POSIX_SPAWN_FILE_ACTIONS_T__ALLOCATED=__pad0[0]")
  add_compile_definitions("POSIX_SPAWN_FILE_ACTIONS_T__USED=__pad0[1]")
  add_compile_definitions("POSIX_SPAWN_FILE_ACTIONS_T__ACTIONS=__actions")
  add_compile_definitions("POSIX_SPAWN_FILE_ACTIONS_T__PAD=__pad")
elseif(NOT ANDROID)
  message(FATAL_ERROR "Unknown posix_spawn_file_actions_t layout!")
endif()
