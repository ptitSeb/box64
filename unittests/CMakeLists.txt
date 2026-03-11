file(GLOB UNITTEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.asm")

foreach(TEST_FILE ${UNITTEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)

    add_test(NAME ${TEST_NAME}
        COMMAND ${CMAKE_COMMAND}
            -D TEST_PROGRAM=${CMAKE_BINARY_DIR}/box64
            -D TEST_ARGS=-t
            -D TEST_ARGS2=${TEST_FILE}
            -D TEST_OUTPUT=${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.out
            -D TEST_REFERENCE=/dev/null
            -P ${CMAKE_SOURCE_DIR}/runTest.cmake
    )

    set_tests_properties(${TEST_NAME} PROPERTIES
        ENVIRONMENT "BOX64_DYNAREC_FASTNAN=0;BOX64_DYNAREC_FASTROUND=0;BOX64_LOG=0;BOX64_NOBANNER=1"
    )
endforeach()
