#ifndef LEXER
#define LEXER
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cctype>
#include <exception>
#include <stdexcept>
#include <deque>

using std::string;
using std::istream;
using std::isspace;
using std::isalpha;
using std::runtime_error;
using std::stringstream;
using std::deque;

typedef enum
  {
    ID,OP,INTEGER,FLOAT,D_STRING,S_STRING,END_OF_FILE
  } token_type;

string 
token_type_name( token_type t )
{
  if( t==ID) return "identifier";
  if( t==OP) return "operator";
  if( t==INTEGER ) return "integer";
  if( t==FLOAT ) return "float";
  if( (t==D_STRING) || (t==S_STRING) ) return "string";
  if( t==END_OF_FILE ) return "end-of-file";
  
  throw runtime_error("Token type has no name.\n");

}

typedef struct
{
  string content;
  token_type type;
  int line;
  int col;
  int offset;
} token;

class lexer
{
 private:
  int line;
  int col;
  int offset;

 private:
  // parser's dead ends may need to put tokens back into the lexing stream.
  // it's up to the user to make what is "put back" is the same token
  // that had last been extracted.
  deque<token> q;

  // report that a character has been used
  // updates stream, line, column, and offset
  void
    consume(istream &s  )
  {
    char c(s.get());
    ++offset;
    if( c=='\n' )
      { ++line; col=0; }
    else
      { ++col; }
  }

  void 
    eat_ws( istream &s )
  {
    char c = s.peek();
    while( isspace(c) )
      {
	consume(s);
	c = s.peek();
      }
  }

  bool
    isoperator( char c )
  {
    return c==':'
      || c=='+'
      || c=='-'
      || c==','
      || c=='('
      || c==')'
      || c=='['
      || c==']'
      || c==';';
  }

  string
    lex_id( istream &s, stringstream &w  )
  {
    char c(s.peek());
    while( isalpha(c) || isdigit(c) || c=='_' || c=='-' )
      {
	w << c;
	consume(s);
	c = s.peek();
      }
    return w.str();
  }

  string
    lex_qstring( istream &s, stringstream &w )
  {
    char c(s.peek());
    char toMatch(c);
    if( !(c == '"' || c == '\'') )
      {
	throw runtime_error("Expected single or double quote.");
      }
    
    consume(s);
    c = s.peek();
    while( c != toMatch )
      {
	w << c;
	consume(s);
	c = s.peek();
	if( s.eof() )
	  {
	    throw runtime_error("Unterminated string.");
	  }
      }
    consume(s);
    return w.str();
  }

  string
    lex_number( istream &s, stringstream &w, bool & was_float )
  {
    was_float = false;
    char c(s.peek());
    while( isdigit(c) )
      {
	w << c;
	consume(s);
	c = s.peek();
      }
    if( c=='.' )
      {
	was_float = true;
	consume(s);
	w << c;
	c = s.peek();
	while( isdigit(c) )
	  {
	    w << c;
 	    consume(s);
	    c = s.peek();
	  }
      }
    return w.str();
  }

  void
    remove_eofs()
  {
    deque<token> q_new;
    for(std::size_t i=0;
	i < q.size(); ++i )
      {
	if( q[i].type != END_OF_FILE )
	  {
	    q_new.push_back( q[i] );
	  }
      }
    q = q_new;
  }
  
  token 
    extract_next_token( istream &s )
  {
    eat_ws(s); // get rid of leading spaces.
    char c = s.peek();
    std::stringstream word("");
    token t;
    t.col = col;
    t.line = line;
    t.offset = offset;

    if( s.eof() )
      {
	t.type = END_OF_FILE;	
      }
    else if( isdigit(c) )
      {
	bool was_float(false);
	lex_number( s, word, was_float );
	t.type = was_float?FLOAT:INTEGER;
      }
    else if( isoperator(c) )
      {
	t.type = OP;
	word << c;
	consume(s);
      }
    else if( isalpha(c) )
      {
	t.type = ID;
	lex_id( s, word );       
      }
    else if( c=='"' )
      {
	t.type = D_STRING;
	lex_qstring( s, word );
      }
    else if( c=='\'' )
      {
	t.type = S_STRING;
	lex_qstring( s, word );
      }
    else
      {
	throw runtime_error("Invalid token.");
      }
    t.content = word.str();
    return t;
  }

 public:
 
 lexer()
   : line(0),col(0),offset(0)
    {
    }
  
  void
    reset()
  {
    q.clear();
    line = 0;
    col = 0;
    offset = 0;
  }


  void
    put_back( token t )
  {
    q.push_back(t);
  }
    
  void
    look_ahead( istream &s, unsigned int n )
    {
      if( n==0 ) return;
      
      for(unsigned i=0; q.size()<n; ++i)
	{
	  q.push_back( extract_next_token(s) );
	}
    }

  token
    peek( istream &s, unsigned int where )
  {
    if( q.size() <= where )
      look_ahead(s,where+1);

    return q[where];
  }
    

  token
    next_token( istream &s)
  {
    
    if( q.size() > 0 )
      {
	token t = q.front();
	q.pop_front();
	return t;
      }
    else
      {
	return extract_next_token(s);
      }
  }		     
};

#endif
