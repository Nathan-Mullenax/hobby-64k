#ifndef VM_H
#define VM_H

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <functional>


const int mod_rv(0); // register value             000
const int mod_ra(1); // register address           001
const int mod_sv(2); // stack value [sp-addr]      010
const int mod_sa(3); // stack address [[sp-addr]]  011
const int mod_code(4); // program segment relative 100
const int VM_SIZE(8*1024);

using std::string;
using std::vector;
using std::runtime_error;

class vm
{
 public:
  typedef struct
  {
    unsigned int src:        6;
    unsigned int dst:        6;
    unsigned int src_mod:    2;
    unsigned int dst_mod:    2;
    unsigned int instr:      16;
  } instruction;

  typedef struct
  {
    unsigned int src:     6;
    unsigned int dst:     6;
    unsigned int src_mod: 2;
    unsigned int dst_mod: 2;
  } instruction_args;

  typedef struct
  {
    unsigned char c0;
    unsigned char c1;
  } instruction_cargs;

  // Extended address format. For instructions
  // that take a single address argument that
  // may be relative to the program or stack
  // segment.
  typedef struct
  {
    unsigned int a_mod: 3;  // 3 mode bits hence 'extended'
    unsigned int a_loc: 13; // the rest store the address
  } instruction_xaddress;

  // scalar format: location and length
  typedef struct
  {
    unsigned int a_mod: 3; // same as x address
    unsigned int a_loc: 6; // 0..63  .  names a register
    unsigned int len: 7;   // 0..127 .  length
  } instruction_scalar;

  
  typedef union
  {
    instruction_args i_args;
    instruction_cargs c_args;
    instruction_xaddress x_args;
    instruction_scalar s_args;
    unsigned short s_arg;
  } conversion;
  
  static conversion convert( instruction n )
  {
    conversion c;
    c.i_args.src = n.src;
    c.i_args.dst = n.dst;
    c.i_args.src_mod = n.src_mod;
    c.i_args.dst_mod = n.dst_mod;
    return c;
  }

  static int int32( instruction n )
  {
    conversion c(convert(n));
    return (n.instr << 16) | c.s_arg;
  }

  static instruction 
    to_instruction( int v )
  {
    conversion c;
    c.s_arg = static_cast<unsigned short>( v & 0xFFFF );
    instruction i;
    i.instr = static_cast<unsigned short>( v >> 16 );
    i.src = c.i_args.src;
    i.dst = c.i_args.dst;
    i.src_mod = c.i_args.src_mod;
    i.dst_mod = c.i_args.dst_mod;
    return i;
  }
    

  //  typedef void (*exec) ( vm &machine, instruction bits );
  typedef std::function<void(vm &machine, instruction instr) > exec;

  typedef void (&exec_ref) ( vm &machine, instruction bits );

  typedef struct
  {
    exec fun;
    unsigned short instr;
  } extension;

  // 64kb machine image
  int stack[VM_SIZE];
  int program[VM_SIZE];

  vector<extension> extensions;
  
  vm()
    {
      IP() = VM_SIZE-1;
      X() = 1;
    }

  void
    serialize( string name )
  {
    std::ofstream fs(name, std::ios::binary);
    for(int i=0; i<VM_SIZE; ++i)
      {
	char *s = reinterpret_cast<char*>(&stack[i]);
	char *p = reinterpret_cast<char*>(&program[i]);

	fs.put( *(s+0) );
	fs.put( *(s+1) );
	fs.put( *(s+2) );
	fs.put( *(s+3) );		

	fs.put( *(p+0) );
	fs.put( *(p+1) );
	fs.put( *(p+2) );
	fs.put( *(p+3) );		
      }
    fs.flush();
    fs.close();
  }

  void
    deserialize( string name )
  {
    std::ifstream fs(name, std::ios::binary);
    for(int i=0; i<VM_SIZE; ++i)
      {
	char *s = reinterpret_cast<char*>(&stack[i]);
	char *p = reinterpret_cast<char*>(&program[i]);
	
	*(s+0) = fs.get();
	*(s+1) = fs.get();
	*(s+2) = fs.get();
	*(s+3) = fs.get();
	*(p+0) = fs.get();
	*(p+1) = fs.get();
	*(p+2) = fs.get();
	*(p+3) = fs.get();
      
	//	std::cout << "Program[" << i << "]=" << program[i] << "\n";
      }
    fs.close();
  }

  // check if an instruction corresponds to a function
  bool
    is_op( unsigned short instr )
  {
    return instr < extensions.size();
  }

  // execute (immediately) an instruction on a virtual machine
  friend vm & 
    operator*= ( vm &machine, instruction ins );

  

  // augment a virtual machine with a new instruction type
  friend vm &
    operator+= ( vm &machine, extension &ext );

  // augment a virtual machine with a new instruction type
  // based on a function
  friend vm &
    operator+= ( vm &machine, exec_ref ext );
  
  // store an instruction as program[W] and increment W
  friend vm &
    operator<< ( vm &machine, instruction ins );
  
  
  // registers are at the bottom of the stack.
  // some of them have names

  int & IP()     { return stack[0]; } // instruction pointer
  int & SP()     { return stack[1]; } // stack pointer
  int & W()      { return stack[2]; } // instruction encoding pointer
  int & ZF()     { return stack[3]; } // zero flag
  int & HALTED() { return stack[4]; } // halted
  
  
  // general purpose registers
  int & A() { return stack[5];  } 
  int & B() { return stack[6];  }
  int & C() { return stack[7];  }
  int & D() { return stack[8];  }
  int & E() { return stack[9];  }
  int & G() { return stack[10]; }
  int & H() { return stack[11]; }
  int & I() { return stack[12]; }
  int & J() { return stack[13]; }
  int & K() { return stack[14]; }
  int & L() { return stack[15]; }
  int & M() { return stack[16]; }
  int & N() { return stack[17]; }
  int & O() { return stack[18]; }
  int & P() { return stack[19]; }
  int & Q() { return stack[20]; }
  int & R() { return stack[21]; }
  int & S() { return stack[22]; } 
  int & T() { return stack[23]; }
  int & U() { return stack[24]; }
  int & V() { return stack[25]; }
  // there is no general-purpose W
  
  // X chooses segment or eXecution
  // segment 0 = stack, 1 = program
  int & X() { return stack[26]; } // segment 0 = stack, 1 = program
  int & Y() { return stack[27]; }
  int & Z() { return stack[28]; }
  
  void
    dump_regs()
  {
    std::cout << "\nIP=" << IP() << "\tSP=" << SP() << "\tW=" << W() << "\tZF=" << ZF() <<
      "\tHALTED=" << HALTED() << " X=" << X() << "\n";
  }

  void 
    dump_debug()
  {
    std::cout << "SP=" << SP() << "\tIP=" << IP() << "\t"
	      << "W=" << W() << "\n";
  }

  // an instruction with no argument
  static instruction
    assemble( unsigned short code )
  {
    instruction ins;    
    ins.instr = code;
    ins.src = 0;
    ins.src_mod = 0;
    ins.dst = 0;
    ins.dst_mod = 0;
    return ins;
  }

  static instruction
    assemble( unsigned short code, unsigned short arg )
  {
    instruction ins;
    ins.instr = code;
    conversion c;
    c.s_arg = arg;
    ins.src = c.i_args.src;
    ins.dst = c.i_args.dst;
    ins.src_mod = c.i_args.src_mod;
    ins.dst_mod = c.i_args.dst_mod;
    return ins;
  }

  // instruction with two chars
  static instruction
    assemble( unsigned short code, char arg0, char arg1 )
  {
    instruction ins;
    ins.instr = code;
    conversion c;
    c.c_args.c0 = arg0;
    c.c_args.c1 = arg1;
    ins.src = c.i_args.src;
    ins.dst = c.i_args.dst;
    ins.src_mod = c.i_args.src_mod;
    ins.dst_mod = c.i_args.dst_mod;
    return ins;
  }

  // instruction with xaddress
  static instruction
    assemble_xaddr( unsigned short code, unsigned char mod, unsigned short ind )
  {
    instruction ins;
    ins.instr = code;
    conversion c;
    c.x_args.a_mod = mod;
    c.x_args.a_loc = ind;
    ins.src_mod = c.i_args.src_mod;
    ins.dst_mod = c.i_args.dst_mod;
    ins.src = c.i_args.src;
    ins.dst = c.i_args.dst;
    return ins;
  }

  static instruction
    dw( unsigned char c0, unsigned char c1, unsigned char c2, unsigned char c3 )
  {    
    return to_instruction( (c0<<24)|(c1<<16)|(c2<<8)|c3 );
  }
    
  int & lookup( unsigned short address, unsigned char mode )
  {  
    switch(mode)
      {
      case mod_rv: // look up a register value
	return stack[address];
      case mod_ra: // look up the value of an address via register
	return stack[stack[address]%VM_SIZE];
      case mod_sv: // look up a stack value
	return stack[(SP()+address)%VM_SIZE];
      case mod_sa: // look up a value pointed to by stack value
	return stack[stack[(SP()+address)%VM_SIZE]];
      case mod_code + mod_rv:
	return program[address%VM_SIZE];
      case mod_code + mod_ra:
	return program[stack[address%VM_SIZE]%VM_SIZE];
      case mod_code + mod_sv:
	return program[(SP()+address)%VM_SIZE];
      case mod_code + mod_sa:
	return program[stack[(SP()+address)%VM_SIZE]%VM_SIZE];
      default:   
	throw runtime_error("Invalid addressing mode.");
      }
  }
};



vm &
operator *= (vm &machine, vm::instruction ins )
{
  if( !machine.HALTED() )
    {
      if( machine.is_op( ins.instr ) )
	{
	  //      machine.program[machine.IP()%8*1024] = vm::int32(ins);
	  machine.extensions[ins.instr].fun( machine, ins );
	}
      else
	{
	  std::stringstream s;
	  s << machine.W() << ": " << "(" << ins.instr << ")" << " not a valid instruction.";
	  throw runtime_error(s.str());
	}
    }
  return machine;
  
}

vm &
operator += (vm &machine, vm::extension &ext )
{
  machine.extensions.push_back(ext);
  ext.instr = machine.extensions.size()-1;
  return machine;
}

vm &
operator += (vm &machine, vm::exec_ref fn )
{
  vm::extension ext;
  ext.fun = &fn;
  machine += ext;
  return machine;
}

vm &
operator << (vm &machine, vm::instruction ins )
{
  machine.program[machine.W()] = vm::int32(ins);
  machine.W() = (machine.W()+1) % (8*1024);
  return machine;
}



#endif
