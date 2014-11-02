#ifndef VM_DEFAULT_H
#define VM_DEFAULT_H

#include <sstream>
#include <ncurses.h>
#include "vm.h"

using std::stringstream;

void RESET ( vm &machine, vm::instruction instr )
{
  for( int i=0; i<VM_SIZE; ++i)
    {
      machine.stack[i]=0;
      machine.program[i]=0;
    }
  machine.SP() = VM_SIZE-1;
  machine.IP() = 0;
  machine.W() = 0;
  machine.X() = 1;
  machine.ZF() = 0;
  
}

void PUSH_LITERAL ( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  machine.stack[machine.SP()--] = c.s_arg;
  ++machine.IP();
}

void PUSH_ADDRESS ( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  machine.stack[machine.SP()--] = machine.stack[c.s_arg%(8*1024)];
  ++machine.IP();
}


void POP_ADDRESS( vm &machine, vm::instruction instr )
{
  unsigned short addr(vm::convert(instr).s_arg);
  machine.stack[addr%(8*1024)] = machine.stack[++machine.SP()];
  ++machine.IP();
}

void OUCH2( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  std::cout << c.c_args.c0;
  std::cout << c.c_args.c1;
  ++machine.IP();
}

void ADD( vm &machine, vm::instruction instr )
{
  machine.lookup(instr.dst,instr.dst_mod)
    += machine.lookup(instr.src,instr.src_mod);
  ++machine.IP();
}

void SUB( vm &machine, vm::instruction instr )
{
  machine.lookup( instr.dst, instr.dst_mod )
    -= machine.lookup( instr.src, instr.src_mod );
  ++machine.IP();
}

void TIMES( vm &machine, vm::instruction instr )
{
  machine.lookup( instr.dst, instr.dst_mod )
    *= machine.lookup( instr.src, instr.src_mod );
  ++machine.IP();
}



void CMP( vm &machine, vm::instruction instr )
{
  int lhs = machine.lookup( instr.src, instr.src_mod );
  int rhs = machine.lookup( instr.dst, instr.dst_mod );
  machine.ZF() = (lhs==rhs)?1:0;
  ++machine.IP();
}

void JE( vm &machine, vm::instruction instr )
{
  if( machine.ZF()==1 )
    {
      vm::conversion c(vm::convert(instr));
      machine.IP() = c.s_arg;
    }
  else
    {
      ++machine.IP();
    }
  machine.ZF() = 0;
}

// an x-address can refer to any address, including
// those in the code segment, on the stack, in registers.
void JX( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  machine.X() = ((c.x_args.a_mod & mod_code) > 0)?1:0;
  machine.IP() = c.x_args.a_loc;
}

void STEP( vm &machine, vm::instruction instr )
{
  if( machine.HALTED()==0 )
    { 
      if( machine.X()==1 )
	machine *= vm::to_instruction(machine.program[machine.IP()%VM_SIZE]);
      else
	machine *= vm::to_instruction(machine.stack[machine.IP()%VM_SIZE]);
    }
}

void HALT( vm &machine, vm::instruction instr )
{
  machine.HALTED()=1;
}

void RUN( vm &machine, vm::instruction instr )
{
  machine.HALTED() = 0;
  while( machine.HALTED() != 1 )
    {
      STEP(machine, instr);
    }
}

void RUN_TRACE( vm &machine, vm::instruction instr )
{
  machine.HALTED() = 0;
  while( machine.HALTED() != 1 )
    {
      machine.dump_regs();
      STEP(machine, instr );
    }
}

void JUMP_LITERAL( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  machine.IP() = c.s_arg;
}

void SETW_LITERAL( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  machine.W() = c.s_arg;
  ++machine.IP();
}


void CALL_LITERAL( vm &machine, vm::instruction instr )
{
  // make space for return value
  --machine.SP();
  
  // current IP onto stack
  machine.stack[machine.SP()] = machine.IP() + 1;
  --machine.SP();
  
  vm::conversion c(vm::convert(instr));

  // transfer execution to specified address
  machine.IP() = c.s_arg;
  
}

// invoke a function whose location is stored in an extended address
void CALL_ADDRESS( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  // get address stored at specified location
  int address = machine.lookup( c.x_args.a_loc, c.x_args.a_mod );
  
  // make space for return value
  --machine.IP();
  
  // save current IP + 1
  machine.stack[machine.SP()] = machine.IP()+1;
  --machine.IP();
  
  // jump to routine
  machine.IP() = address;
}

void RETURN_LITERAL( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr) );
  // set return value
  machine.stack[machine.SP()+2] = c.s_arg;
  // jump to return address and remove it from the stack
  machine.IP() = machine.stack[++machine.SP()];
}

void RETURN_ADDRESS( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr) );
  // set return value to whatever is pointed by xaddress
  machine.stack[machine.SP()+2] = machine.lookup( c.x_args.a_loc, c.x_args.a_mod );
  // jump to return address and remove it from the stack
  machine.IP() = machine.stack[++machine.SP()];
}

void INC( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr) );
  ++machine.lookup( c.x_args.a_loc, c.x_args.a_mod );
  ++machine.IP();
}

void DEC( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr) );
  --machine.lookup( c.x_args.a_loc, c.x_args.a_mod );
  ++machine.IP();
}

void DIV( vm &machine, vm::instruction instr )
{
  int divisor(machine.lookup(instr.src,instr.src_mod));
  if( divisor==0 ) throw runtime_error("Division by zero.");
  machine.lookup(instr.dst,instr.dst_mod)
    /= divisor;
  ++machine.IP();
}

// print an integer pointed to by an extended address
void PRINT_ADDRESS_DEC( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  int v = machine.lookup( c.x_args.a_loc, c.x_args.a_mod );
  std::cout << v;
  ++machine.IP();
}

// create a new instruction takes one argument (length),
// which is the number of instructions that follow.
void LAMBDA( vm &machine, vm::instruction instr )
{
  //std::cout << "Lambda defined.\n";
  vm::conversion c(vm::convert(instr));
  int len = c.s_arg;
  int start = machine.IP()+1;
  //  std::cout << "start=" << start << "\n";
  //std::cout << "len=" << len << "\n";
  
  auto fun = [len,start]( vm &machine, vm::instruction instr )
    {
      //  std::cout << "Lambda called.\n";
      // save IP address plus 1 ('next line')
      machine.stack[machine.SP()] = machine.IP()+1;
      
      --machine.SP();
      
      // argument to instruction (and instruction code) passed via register A
      machine.A() = vm::int32( instr );

      machine.IP() = start; // jump to function

      // function is 'running' as long as the line number
      // is within the function body.
      while( (machine.IP() >= start) && (machine.IP() < start+len) )
	{
	  machine *= vm::to_instruction(machine.program[machine.IP()]);
	}      
      
    };
  vm::extension x;
  x.instr = machine.extensions.size();
  x.fun = fun;
  machine += x;
  // jump past function definition
  machine.IP() = start + len;
}


void LSH( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  machine.lookup( c.s_args.a_loc, c.s_args.a_mod )
    <<= c.s_args.len;
  ++machine.IP();
}

void RSH( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  machine.lookup( c.s_args.a_loc, c.s_args.a_mod )
    >>= c.s_args.len;
  ++machine.IP();
} 


void AND( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  machine.lookup( c.i_args.dst, c.i_args.dst_mod )
    &= machine.lookup( c.i_args.src, c.i_args.src_mod );
  ++machine.IP();
}

void OR( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  machine.lookup( c.i_args.dst, c.i_args.dst_mod )
    |= machine.lookup( c.i_args.src, c.i_args.src_mod );
  ++machine.IP();
}

// pops IP off of the stack and jumps to it.
void RETURN_NOTHING( vm &machine, vm::instruction instr )
{
  machine.IP() = machine.stack[machine.SP()+1];
  ++machine.SP(); 
}

// curses IO

void CURSES_INITSCR( vm &machine, vm::instruction instr )
{
  initscr();
  ++machine.IP();
}

void CURSES_CBREAK( vm &machine, vm::instruction instr )
{
  cbreak();
  ++machine.IP();
}

void CURSES_NOECHO( vm &machine, vm::instruction instr )
{
  noecho();
  ++machine.IP();
}

void CURSES_KEYPAD( vm &machine, vm::instruction instr )
{
  keypad( stdscr, TRUE );
  ++machine.IP();
}

void CURSES_ENDWIN( vm &machine, vm::instruction instr )
{
  endwin();
  ++machine.IP();
}

void CURSES_GETCH( vm &machine, vm::instruction instr )
{
  machine.stack[ machine.SP() ] = getch();
  --machine.SP();
  ++machine.IP();
}

void CURSES_WAITCH( vm &machine, vm::instruction instr )
{
  int c;
  while( (c=getch())==ERR )
    ;
  machine.stack[ machine.SP() ] = c;
  --machine.SP();
  ++machine.IP();
}

void CURSES_START_COLOR( vm &machine, vm::instruction instr )
{
  start_color();
  ++machine.IP();
}

void CURSES_REFRESH( vm &machine, vm::instruction instr )
{
  refresh();
  ++machine.IP();
}

// move cursor to y,x
void CURSES_MOVE( vm &machine, vm::instruction instr )
{
  vm::conversion c( vm::convert(instr));
  move( c.c_args.c0, c.c_args.c1 );
  ++machine.IP();
}

// same as move with registers instead of tiny ints
void CURSES_MOVE_R( vm &machine, vm::instruction instr )
{
  vm::conversion c( vm::convert(instr) );
  move( machine.lookup( instr.src, instr.src_mod ), 
	machine.lookup( instr.dst, instr.dst_mod ) );
  ++machine.IP();
}

void CURSES_ADDCH( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  addch( c.c_args.c1 );
  ++machine.IP();
}

void CURSES_ADD2CH( vm &machine, vm::instruction instr )
{
  vm::conversion c(vm::convert(instr));
  addch( c.c_args.c0 );
  addch( c.c_args.c1 );
  ++machine.IP();
}

// pushes the curses global 'COLORS' onto the stack
void CURSES_COLORS( vm &machine, vm::instruction instr )
{
  machine.stack[machine.SP()--] = COLORS;
  ++machine.IP();
}

void CURSES_COLOR_PAIRS( vm &machine, vm::instruction instr )
{
  machine.stack[machine.SP()--] = COLOR_PAIRS;
  ++machine.IP();
}




vm
create_default_vm( stringstream &s )
{
  vm machine;
  machine += RESET;             s << "mnem reset(0)        noargs;" "\n";
  machine += PUSH_LITERAL;      s << "mnem push-l(1)        short;" "\n";
  machine += PUSH_ADDRESS;      s << "mnem push-a(2)        short;" "\n";
  machine += POP_ADDRESS;       s << "mnem pop-a(3)         short;" "\n";
  machine += OUCH2;             s << "mnem ouch2(4)         chars;" "\n";
  machine += ADD;               s << "mnem add-r(5)          regs;" "\n";
  machine += SUB;               s << "mnem sub-r(6)          regs;" "\n";
  machine += TIMES;             s << "mnem mul-r(7)          regs;" "\n";
  machine += CMP;               s << "mnem cmp-r(8)          regs;" "\n";
  machine += JE;                s << "mnem j-e(9)          noargs;" "\n";
  machine += STEP;              s << "mnem step(10)        noargs;" "\n";
  machine += HALT;              s << "mnem halt(11)        noargs;" "\n";
  machine += RUN;               s << "mnem run(12)         noargs;" "\n";
  machine += RUN_TRACE;         s << "mnem run-trace(13)   noargs;" "\n";
  machine += JUMP_LITERAL;      s << "mnem jmp-l(14)        short;" "\n";
  machine += SETW_LITERAL;      s << "mnem setw-l(15)       short;" "\n";
  machine += CALL_LITERAL;      s << "mnem call-l(16)       short;" "\n";
  machine += CALL_ADDRESS;      s << "mnem call-x(17)       xaddr;" "\n";
  machine += RETURN_LITERAL;    s << "mnem return-l(18)     short;" "\n";
  machine += RETURN_ADDRESS;    s << "mnem return-x(19)     xaddr;" "\n";
  machine += INC;               s << "mnem inc-x(20)        xaddr;" "\n";
  machine += DEC;               s << "mnem dec-x(21)        xaddr;" "\n";
  machine += DIV;               s << "mnem div-r(22)         regs;" "\n";
  machine += PRINT_ADDRESS_DEC; s << "mnem print-a-d(23)    xaddr;" "\n";
  machine += LAMBDA;            s << "mnem lambda-l(24)     short;" "\n";
  machine += AND;               s << "mnem and-r(25)         regs;" "\n";
  machine += OR;                s << "mnem or-r(26)          regs;" "\n";
  machine += RETURN_NOTHING;    s << "mnem return(27)      noargs;" "\n";
  machine += JX;                s << "mnem j-x(28)          xaddr;" "\n";
  machine += RSH;               s << "mnem rsh(29)         scalar;" "\n";
  machine += LSH;               s << "mnem lsh(30)         scalar;" "\n";

  machine += CURSES_INITSCR;     s << "mnem curses-initscr(31)     noargs;" "\n";
  machine += CURSES_CBREAK;      s << "mnem curses-cbreak(32)      noargs;" "\n";
  machine += CURSES_NOECHO;      s << "mnem curses-noecho(33)      noargs;" "\n";
  machine += CURSES_KEYPAD;      s << "mnem curses-keypad(34)      noargs;" "\n";
  machine += CURSES_ENDWIN;      s << "mnem curses-endwin(35)      noargs;" "\n";
  machine += CURSES_GETCH;       s << "mnem curses-getch(36)       noargs;" "\n";
  machine += CURSES_WAITCH;      s << "mnem curses-waitch(37)      noargs;" "\n";
  machine += CURSES_START_COLOR; s << "mnem curses-start-color(38) noargs;" "\n";
  machine += CURSES_REFRESH;     s << "mnem curses-refresh(39)     noargs;" "\n";
  machine += CURSES_MOVE;        s << "mnem curses-move(40)         chars;" "\n";
  machine += CURSES_ADDCH;       s << "mnem curses-addch(41)        chars;" "\n";
  machine += CURSES_ADD2CH;      s << "mnem curses-add2ch(42)       chars;" "\n";
  machine += CURSES_COLORS;      s << "mnem curses-colors(43)      noargs;" "\n";
  machine += CURSES_COLOR_PAIRS; s << "mnem curses-color-pairs(44) noargs;" "\n";
  machine += CURSES_MOVE_R;      s << "mnem curses-move-r(45)        regs;" "\n";

  return machine;
}

#endif
