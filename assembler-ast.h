#ifndef ASSEMBLER_AST_H
#define ASSEMBLER_AST_H

#include "vm.h"

#include <map>
#include <string>
#include <vector>

using std::string;
using std::map;
using std::vector;

// line label and variable location thing
class asm_context
{
 public:
  typedef string symbol;
  map<symbol,int> knowns;
  vm &machine;
  
  asm_context( vm &machine )
    : machine(machine)
  {}
  
  
  
};

class asm_statement
{
 public:
  int value;

  

  // load this instruction into machine memory
  void
    load( asm_context &c  )
  {
    c.machine << vm::to_instruction(value);
  }
  
};

class asm_program
{
  vector<asm_statement> statements;
  asm_context c;

  void
    load( vm &machine )
  {
    
  }
};



#endif
