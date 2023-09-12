# arguments checking
if( NOT TEST_PROGRAM )
  message( FATAL_ERROR "Require TEST_PROGRAM to be defined" )
endif( NOT TEST_PROGRAM )
if( NOT TEST_ARGS )
  message( FATAL_ERROR "Require TEST_ARGS to be defined" )
endif( NOT TEST_ARGS )
if( NOT TEST_OUTPUT )
  message( FATAL_ERROR "Require TEST_OUTPUT to be defined" )
endif( NOT TEST_OUTPUT )
if( NOT TEST_REFERENCE )
  message( FATAL_ERROR "Require TEST_REFERENCE to be defined" )
endif( NOT TEST_REFERENCE )
if(NOT DEFINED ENV{INTERPRETER})
    set(ENV{INTERPRETER} "")
endif()

set(ENV{BOX64_LOG} 0)
set(ENV{BOX64_NOBANNER} 1)
if( EXISTS ${CMAKE_SOURCE_DIR}/x64lib )
  # we are inside box64 folder
  set(ENV{LD_LIBRARY_PATH} ${CMAKE_SOURCE_DIR}/x64lib)
else()
  # we are inside build folder
  set(ENV{LD_LIBRARY_PATH} ${CMAKE_SOURCE_DIR}/../x64lib)
endif( EXISTS ${CMAKE_SOURCE_DIR}/x64lib )

# run the test program, capture the stdout/stderr and the result var
execute_process(
  COMMAND $ENV{INTERPRETER} ${TEST_PROGRAM} ${TEST_ARGS} ${TEST_ARGS2}
  OUTPUT_FILE ${TEST_OUTPUT}
  ERROR_VARIABLE TEST_ERROR
  RESULT_VARIABLE TEST_RESULT
  )

# if the return value is !=0 bail out
if( TEST_RESULT )
  get_filename_component(TESTNAME "${TEST_ARGS}" NAME)
  file(RENAME "${TEST_OUTPUT}" "${CMAKE_BINARY_DIR}/${TESTNAME}.out")
  file(WRITE  "${CMAKE_BINARY_DIR}/${TESTNAME}.err" ${TEST_ERROR})
  message( FATAL_ERROR "Failed: Test program ${TEST_PROGRAM} exited != 0.\n${TEST_ERROR}" )
endif( TEST_RESULT )

# now compare the output with the reference
execute_process(
  COMMAND ${CMAKE_COMMAND} -E compare_files ${TEST_OUTPUT} ${TEST_REFERENCE}
  RESULT_VARIABLE TEST_RESULT
  )

# again, if return value is !=0 scream and shout
if( TEST_RESULT )
  get_filename_component(TESTNAME "${TEST_ARGS}" NAME)
  file(RENAME "${TEST_OUTPUT}" "${CMAKE_BINARY_DIR}/${TESTNAME}.out")
  file(WRITE  "${CMAKE_BINARY_DIR}/${TESTNAME}.err" ${TEST_ERROR})
  message( FATAL_ERROR "Failed: The output of ${TEST_PROGRAM} did not match ${TEST_REFERENCE}")
endif( TEST_RESULT )

# remove the temporary files if they exist
if( EXISTS ${TEST_OUTPUT} )
  file(REMOVE "${TEST_OUTPUT}")
endif()

# everything went fine...
message( "Passed: The output of ${TEST_PROGRAM} matches ${TEST_REFERENCE}" )
