cmake_minimum_required(VERSION 3.10)
project("helper")
find_package(Clang REQUIRED CONFIG)

add_executable(helper main.cpp gen.cpp)
target_include_directories(helper SYSTEM PUBLIC ${CLANG_INCLUDE_DIRS})
target_link_libraries(helper PUBLIC clang-cpp LLVM)
