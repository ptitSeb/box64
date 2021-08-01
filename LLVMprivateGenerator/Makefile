all: dumpSigs

dumpSigs: main.o registered_structs.o
	g++ -g3 -std=gnu++17 -fno-rtti main.o registered_structs.o -o dumpSigs "-L$(LLVM_install_dir)/lib" -lclang-cpp -lclangTooling -Wl,-rpath "-Wl,$(LLVM_install_dir)/lib"

main.o: main.cpp
	g++ -g3 -std=gnu++17 -fno-rtti -c main.cpp -Wfatal-errors "-I$(LLVM_install_dir)/include" -o main.o
registered_structs.o: registered_structs.cpp
	g++ -g3 -std=gnu++17 -fno-rtti -c registered_structs.cpp -Wfatal-errors -o registered_structs.o

clean:
	$(RM) dumpSigs main.o registered_structs.o
