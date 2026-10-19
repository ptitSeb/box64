set(BOX64_MUSL FALSE)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(CMAKE_C_COMPILER_TARGET MATCHES "musl")
        set(BOX64_MUSL TRUE)
    else()
        execute_process(
            COMMAND "${CMAKE_C_COMPILER}" -dumpmachine
            OUTPUT_VARIABLE BOX64_CC_MACHINE
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        if(BOX64_CC_MACHINE MATCHES "musl")
            set(BOX64_MUSL TRUE)
        endif()
    endif()
endif()

if(BOX64_MUSL)
    set(MUSL_COMPAT_LIBS
        argp
        obstack
        fts
        ucontext
    )

    if(HAVE_ERROR_LIB)
        list(APPEND MUSL_COMPAT_LIBS error)
    endif()

    foreach(lib IN LISTS MUSL_COMPAT_LIBS)
        string(TOUPPER "${lib}" LIB_UPPER)
        set(LIB_VAR "MUSL_${LIB_UPPER}_LIBRARY")
        
        message(STATUS "Looking for lib${lib}")
        find_library(${LIB_VAR} NAMES ${lib})

        if(${LIB_VAR})
            message(STATUS "Looking for lib${lib} - found: ${${LIB_VAR}}")
        else()
            message(FATAL_ERROR "Looking for lib${lib} - not found.")
        endif()

        target_link_libraries(${BOX64} "${MUSL_${LIB_UPPER}_LIBRARY}")
    endforeach()
endif()
