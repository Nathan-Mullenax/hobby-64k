#include <iostream>
#include <sstream>

#include "vm.h"
#include "vm-default.h"

using std::stringstream;

// example

int main(int argc, char **argv)
{
  stringstream preamble;
  vm machine(create_default_vm(preamble));
  machine *= vm::assemble(0); // reset.
  if( argc<2 )
    {
      // describe the virtual machine
      std::cout << preamble.str();
    }
  else
    {
      try
	{
	  // deserialize binary image and run it
	  machine.deserialize(argv[1]);
	  machine *= vm::assemble(12); // RUN
	 
	}
      catch( runtime_error e )
	{
	  std::cout << std::endl;
	  std::cout << e.what() << "\n";
	}
    }
  


  

  // RESET the machine
  /*
  machine *= vm::assemble(Reset);
  */

  // define a custom instruction in assembly using LAMBDA
  /*
  machine << vm::assemble(Lambda,4);
  machine << vm::assemble(Ouch2,'l','a');
  machine << vm::assemble(Ouch2,'m','b');
  machine << vm::assemble(Ouch2,'d','a');
  machine << vm::assemble(Ouch2,'!','\n');
  */

  // print "Hello world!"
  /*
  machine << vm::assemble(Ouch2,'H','e');
  machine << vm::assemble(Ouch2,'l','l');
  machine << vm::assemble(Ouch2,'o',' ');
  machine << vm::assemble(Ouch2,'w','o');
  machine << vm::assemble(Ouch2,'r','l');
  machine << vm::assemble(Ouch2,'d','!');
  machine << vm::assemble(Ouch2,' ','\n');
  */


  // invoke our lambda
  /*
  machine << vm::assemble(26);
  */

  /* int hello_string = machine.W(); // address of string */
  // store a string in the code segment
  // call function
  /* machine << vm::assemble(CallLiteral, hello_string+6 ); 
  
  machine << vm::assemble(Halt);

  machine << vm::to_instruction(12);
  machine << vm::dw( 'H', 'e', 'l', 'l' );
  machine << vm::dw( 'o', ' ', 'a', 'g' );
  machine << vm::dw( 'a', 'i', 'n', '\n' );
  */
  // how do we jump past data without knowing a future label?
  // tables i guess...

  /*
  int a_function = machine.W();
  machine << vm::assemble( Ouch2, 'A', 'B' );
  machine << vm::assemble( ReturnLiteral, 42 );
  */

  // run the program
  /*
  machine *= vm::assemble( Run );
  */

  // go to first line and run again with debug output
  /*
  machine *= vm::assemble( JumpLiteral, 0 );
  machine *= vm::assemble( RunTrace );
  */

  return 0;
}
