#include <string>
#include <iostream>
#include <sstream>

#include "vm.h"
#include "vm-default.h"

using std::stringstream;
using std::string;

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
  else if(argc==2)
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
  else if(argc==3)
    {
      string fn;
      if( string(argv[1])=="debug")
	{
	  fn = argv[2];
	}
      else if( string(argv[2])=="debug")
	{
	  fn = argv[1];
	}
      machine.deserialize(fn);

      string cmd;
      while( machine.HALTED() != 1 )
	{
	  machine.dump_regs();
	  std::cin >> cmd;
	  if( cmd=="step" )
	    {
	      machine *= vm::assemble(10);
	    }
	  else if( cmd=="stepn" )
	    {
	      int n(0);
	      std::cin >> n;
	      for(int i=0; i<n; ++i)
		{
		  machine *= vm::assemble(10);
		  machine.dump_regs();
		}
	    }
	}

    }

  return 0;
}
