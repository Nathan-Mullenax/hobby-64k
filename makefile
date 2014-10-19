all:	vm assembler compiler

vm:	vm.cpp vm.h
	g++ -std=c++11 -Wall ./vm.cpp -O -ovm

assembler:	assembler.cpp assembler.h vm.h lexer.h
	g++ -std=c++11 -Wall ./assembler.cpp -O -oassembler

compiler:	compiler.cpp
	g++ -std=c++11 -Wall ./compiler.cpp -O -ocompiler

clean:
	rm ./vm ./assembler ./compiler