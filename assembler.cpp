#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <stdexcept>
#include <functional>

#include "vm.h"
#include "vm-default.h"
#include "lexer.h"
#include "assembler-ast.h"

using std::istream;
using std::stringstream;
using std::map;
using std::set;
using std::string;
using std::vector;
using std::function;

// it's an assembler for no particular 
// implementation of the virtual machine
class assembler
{
private:
  // these are types of statements that are legal
  // in an assembly file.
  typedef enum 
    {
      MNEM, DW, DS, XADDR, REGS, SCALAR, 
      CHARS, LABEL, SHORT, NOARGS, CTRLD 
    } form_type;

  // maps instruction name to grammatical form.
  map< string,form_type> mnemonics;

  // as labels are encountered, there are given
  // line numbers.
  map< string,int > line_labels;

  // as mnemonics are defined, their corresponding
  // codes are stored in this table.
  map< string,int > codes;

  
  int mnem_count;
  
  // transformations that should happen to
  // a machine once a line label is defined.
  // So, this functor template will capture
  // W 
  typedef function< void(vm &, string, int) > dep_fn;
  map<string,vector<dep_fn> > deps;


  lexer lex;
  
  static string error_prefix( token t )
  {
    stringstream s;
    s << t.line << "," << t.col << " : ";
    return s.str();
  }

  string expect( token t, string value )
  {
    if( t.content != value )
      {
	stringstream ss( error_prefix(t) );
	ss << "Expected " << value << ".";
	throw runtime_error( ss.str());
      }
    return t.content;
  }

  string expect( token t, token_type type )
  {
    if( t.type != type )
      {
	stringstream ss; ss <<  error_prefix(t) ;
	ss << "Expected " << token_type_name(type) << " but got " << "'" << t.content << "'";
	throw runtime_error(ss.str());
      }
    return t.content;
  }

  // take a type name and return a corresponding form_type
  static form_type
  typify( token t )
  {
    string tn(t.content);

    if( tn=="xaddr" )     return XADDR;
    if( tn=="regs" )      return REGS;
    if( tn=="scalar" )    return SCALAR;
    if( tn=="chars" )     return CHARS;
    if( tn=="short" )     return SHORT;
    if( tn=="noargs" )    return NOARGS;

    stringstream ss( error_prefix(t) );
    ss << "Expected form type name.";
    throw runtime_error(ss.str());
  }

  

  int
  intify( string n )
  {
    stringstream s(n);
    int v(0); s >> v;
    return v;
  }

  template<class T>
  vector<T>
  list_of( function< T(token) > f, std::istream &s )
  {
    vector<T> ts;
    token t = lex.next_token(s);
    ts.push_back(f(t));
    t = lex.peek(s,0);
    while( t.content == "," )
      {
	lex.next_token(s); // comma
	
	t = lex.next_token(s);
	ts.push_back( f(t) );

	t = lex.peek(s,0);
      }
    expect( lex.next_token(s), ";" );
    return ts;
  }

  // Mnem ::= "mnem" kw(CODE) FORM0, FORM1, FORM2 ;
  void parse_mnem( vm &machine, std::istream &s )
  {
    vector<form_type> fs;
    string name;
    int code;

    expect( lex.next_token(s), "mnem" );
    name = expect( lex.next_token(s), ID );
    expect( lex.next_token(s), "(" );
    
    token cp_p( lex.next_token(s) );
    if(cp_p.content==")")
      {
	code = machine.extensions.size() + mnem_count;
	++mnem_count;
      }
    else
      {
	code = intify( expect(cp_p, INTEGER ) );
	expect( lex.next_token(s), ")" );
      }
    
    
    //    fs = list_of<form_type>( typify,s );
    
   
    mnemonics[name] = typify( lex.next_token(s) );
    expect( lex.next_token(s), ";" );
    codes[ name ] = code;

    //    std::cout << name << " is now defined.\n";
  }

  // we can do reg:n, [reg:n], stack:n, [stack:n]
  void parse_reg( istream &s, unsigned char &seg, unsigned short &ind )
  {
    token t0 = lex.peek(s,0);
    token t1 = lex.peek(s,1);
    token t2 = lex.peek(s,3);
    if( t0.type == ID )
      {
	if( t0.content=="reg" )
	  {
	    lex.next_token(s); // "reg"
	    expect(lex.next_token(s),":"); // ":"
	    seg = mod_rv;
	    ind = intify( expect( lex.next_token(s), INTEGER ) );
	    return;
	  }
	else if( t0.content=="stack" )
	  {
	    lex.next_token(s); // "stack"
	    expect(lex.next_token(s),":"); // ":"
	    seg = mod_sv;
	    ind = intify( expect( lex.next_token(s), INTEGER ) );
	    return;
	  }
      }
    else if( t0.content == "[" )
      {
	lex.next_token(s); // "["
	if( t1.content=="reg" )
	  {
	    lex.next_token(s); // "reg"
	    expect(lex.next_token(s), ":" );
	    seg = mod_ra;
	    ind = intify( expect( lex.next_token(s), INTEGER ) );
	    expect(lex.next_token(s),"]");
	    return;
	  }
	else if( t1.content=="stack" )
	  {
	    lex.next_token(s); // "stack";
	    expect( lex.next_token(s), ":" );
	    seg = mod_sa;
	    ind = intify( expect( lex.next_token(s), INTEGER ) );
	    expect( lex.next_token(s), "]");
	    return;
	  }
      }
    throw runtime_error("Invalid register specification.");
  }

  void parse_regs( vm &machine, istream &s )
  {
    unsigned char mod;
    unsigned short addr;
    vm::instruction code;
    
    code.instr = codes[ expect(lex.next_token(s),ID) ];

    parse_reg( s, mod, addr );
    code.src = addr;
    code.src_mod = mod;
    expect(lex.next_token(s), "," );
    parse_reg( s, mod, addr );
    code.dst = addr;
    code.dst_mod = mod;
    expect( lex.next_token(s), ";" );

    machine << code;
  }

  void define_label( vm &machine, string name, int value )
  {
    // for future reference:
    line_labels[name] = machine.W();

    // for past references:
    // list of functions that are waiting for 'name'
    auto fns = deps[name];
    
    
    for(unsigned i=0; i<fns.size(); ++i)
      {
	// invoke each one, in hopes that it knows
	// how to patch the machine.
	fns[i](machine,name,value);
      }
    
    // these dependencies have been satisfied,
    // delete them.
    deps[name] = vector<dep_fn>();
  }

  void parse_label( vm &machine, istream &s )
  {
    token name = lex.next_token(s);
    expect( lex.next_token(s), ":" );
    define_label( machine, name.content, machine.W() );
  }

  void parse_short( vm &machine, istream &s )
  {
    token name = lex.next_token(s);
    short v = intify(expect( lex.next_token(s), INTEGER ) );
    expect( lex.next_token(s), ";" );
    machine << vm::assemble( codes[name.content ], v );
    
  }

  unsigned char  
  parse_mod( istream &s )
  {
    token mod = lex.peek(s,0);
    char segno = 0;
    if( mod.content=="code" ) segno=mod_code;
    else if( mod.content=="stack" ) segno=mod_sv;
    else if( mod.content=="reg" ) segno=mod_rv;
    else
      throw runtime_error( "Invalid addressing mode" );
    lex.next_token(s);
    return segno;
  }

  void 
  parse_xaddr( vm &machine, istream &s )
  {
    token name = lex.next_token(s);
    bool address(false);
    
    if( lex.peek(s,0).content=="[" )
      {
	lex.next_token(s);
	address=true;
      }
    
    

    unsigned char segno(parse_mod(s));
   
    token p_plus = lex.peek(s,0);
    
    while( p_plus.content=="+" )
      {
	// eat "+"
	lex.next_token(s);
	segno |= parse_mod(s);
	p_plus = lex.peek(s,0);
      }
    
    expect( lex.next_token(s), ":" );
    short v = intify( expect( lex.next_token(s), INTEGER ) );
    

    
    if( address )
      {
	segno += mod_ra;
	expect( lex.next_token(s), "]" );
      }
    expect( lex.next_token(s), ";" );    
    machine << vm::assemble_xaddr( codes[name.content], segno, v );
  }

  void
  parse_chars( vm &machine, istream &s )
  {
    token name = lex.next_token(s);
    string v = expect( lex.next_token(s), S_STRING );
    if( v.size() != 2 )
      throw runtime_error("Too many characters in character constant." );
    char c0 = v[0];
    char c1 = v[1];
    expect( lex.next_token(s), ";" );
    machine << vm::assemble( codes[name.content], c0, c1 );
  }

  void
  parse_noargs( vm &machine, istream &s )
  {
    token name = lex.next_token(s);
    expect( lex.next_token(s), ";" );
    machine << vm::assemble( codes[name.content] );
  }

public:
  assembler()
  {
    mnem_count = 0;
  }

  form_type 
  classify( istream &s )
  {
    token t0 = lex.peek(s,0); if( t0.type == END_OF_FILE ) return CTRLD;
    token t1 = lex.peek(s,1);

    //    std::cout << "Classify (" << t0.content << "," << t1.content << ")\n";

    if( t0.content=="mnem" )
      {
	return MNEM;
      }
    else if( (t0.type == ID) && (t1.content == ":" ))
      {
	return LABEL;
      }
    else if( t0.type == ID )
      {
	if( mnemonics.count(t0.content)==0 )
	  {
	    std::cout << "\"" << t0.content << "\"\n";
	    throw runtime_error("Undefined instruction.");
	  }
	else
	  return mnemonics[t0.content];
	
      }
    else if( t0.type == END_OF_FILE )
      {
	return CTRLD;
      }
    throw runtime_error("Could not classify your statement.");
  }

  

  void
  assemble( vm &machine, istream &s )
  {
    bool running(true);

    while( running )
      {
       
	switch( classify(s) )
	  {
	  case MNEM:
	    parse_mnem( machine, s );
	    break;
	  case LABEL:
	    {
	      parse_label( machine, s);
	      
	    }
	    break;
	  case REGS:
	    parse_regs( machine, s );
	    break;
	  case CTRLD:	   
	    running = false;
	    break;
	  case SHORT:
	    parse_short( machine, s );
	    break;
	  case XADDR:
	    parse_xaddr( machine, s );
	    break;
	  case CHARS:
	    parse_chars( machine, s );
	    break;
	  case NOARGS:
	    parse_noargs( machine, s );
	    break;
	  default:
	    throw runtime_error("I didn't get that.");
	  }
      }
    lex.reset();
  }
};

int 
last_dot( string &s )
{
  int i(s.size()-1);
  while(s[i]!='.')
    {
      --i;
      if( i==0 ) return s.size()-1;
    }
  return i;
}

string 
out_file_name( string fn )
{
  
  string o;
  return fn.substr(0, last_dot(fn)) + ".b64";
  
}

int
main(int argc, char **argv )
{
  assembler basic_asm;
  

  try
    {
      stringstream ss;
      // create a machine with some basic instructions
      vm machine( create_default_vm(ss) );
      // load default mnemonics
      basic_asm.assemble(machine, ss );
      // read and assemble source

      if( argc > 1 )
	{
	  std::ifstream fin(argv[1], std::ios::binary);
	  basic_asm.assemble(machine, fin);
	  machine.IP() = 0;
	  machine.SP() = VM_SIZE-1;
	  machine.serialize(out_file_name(argv[1]));
	}
      else
	{
	  basic_asm.assemble(machine, std::cin);
	}

      machine.serialize("a.b64");
    }
  catch( std::runtime_error e )
    {
      std::cout << e.what() << "\n";
      std::cout << "Assembly aborted.\n";
    }
  
}


