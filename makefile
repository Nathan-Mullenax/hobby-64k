all:	h64k-vm h64k-as h64k-c example.b64

h64k-vm:	vm.cpp vm.h vm-default.h
	g++ -lncurses -std=c++11 -Wall ./vm.cpp -O -oh64k-vm

h64k-as:	assembler.cpp assembler.h vm.h lexer.h vm-default.h
	g++ -lncurses -std=c++11 -Wall ./assembler.cpp -O -oh64k-as

h64k-c:	compiler.cpp language.h ast.h vm.h
	g++ -lncurses -std=c++11 -Wall ./compiler.cpp -O -oh64k-c

example.b64: example.s64 h64k-as
	./h64k-as ./example.s64

clean:
	rm ./h64k-vm ./h64k-as ./h64k-c ./*.b64
