#!/bin/bash

run_test() {
  id=$1
  append=$2

  echo "Test $id"
  ./box64 ../tests/test$id $append > /tmp/test$id.log
  if [ $? -ne 0 ]; then
    echo "Test $id exited with non-zero status"
    exit 1
  fi
  diff /tmp/test$id.log ../tests/ref$id.txt
  if [ $? -ne 0 ]; then
    echo "Test $id output mismatch"
    exit 1
  fi
  echo "Test $id passed"
}

plain_tests=(01 02 03 06 07 08 09 11 12 13 15 18 19 20 21 22)
for i in ${plain_tests[@]}; do
  run_test $i
done

run_test 04 "yeah"
run_test 05 "7"

LD_LIBRARY_PATH=../x64lib run_test 10
BOX64_DYNAREC_FASTROUND=0 run_test 16
BOX64_DYNAREC_FASTNAN=0 BOX64_DYNAREC_FASTROUND=0 run_test 17
