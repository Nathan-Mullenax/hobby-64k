#include <iostream>
#include <functional>
#include <stdexcept>
#include <gmpxx.h>
#include <string>
#include <fstream>
#include <set>
#include <map>
#include <cstdlib>
#include <ctime>
#include <cctype>

using std::function;
using std::string;
using std::runtime_error;
using std::set;
using std::map;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::rand;
using std::time;
using std::islower;

class dfa { 
public: 
  typedef unsigned long long state;
private:
  state new_state_counter;
public:
  typedef char symbol;
  typedef function< state(state,symbol) > tfunc;
  static const int START= 0;
  static const int REJECT=-1;
 
  state current;
  set<state> accept_states;


  class CFunc {
  private:
    
  public:
    class Index {
 
    public:
      
      state s; symbol c;
      Index( state s, symbol c ) : s(s), c(c) {}

      bool operator< ( Index const & b ) const {
	return s < b.s || ( (s==b.s) && (c<b.c) );
      }

      bool operator== ( Index const & b ) const {
	return (s==b.s) && (c==b.c);
      }
    };
    // stores all transitions
    map<Index,state> t;
    state operator() ( state s, symbol c ) {
      if( t.count(Index(s,c)) > 0 ) {
	return t[Index(s,c)];
      } else {
	return dfa::REJECT;
      }
    }
    // define a transition from s to p via symbol c
    void augment( state s, symbol c, state p ) {
      Index i(s,c);
      t[i] = p;
    }
  } transition;

  dfa( ) {
    current = START;
    
    new_state_counter = 1;
  }

  
  // determine whether this dfa accepts string s                          .
  bool p_accepts( string const &s ) {
    current = START;
    
    for( char c : s ) {
      current = transition( current, c );
      if( current==REJECT ) {
	return false;
      }
    }
    return accept_states.count( current )>0;
  }
private:
  // modify this DFA to define a transition to a new or existing state from
  // the current state
  void accept( char c ) {
    if( current==REJECT ) {
      throw runtime_error("Cannot transition to acceptance from rejection. :(");
    } else {
      state nextp = transition(current,c);
      if( nextp==REJECT ) {
	// create a new state, update counter
	state old_state = current;
	current = new_state_counter++;
	state new_state = current;
	
	transition.augment( old_state, c, new_state );
      }
      else {
	current = transition(current,c);
      }
    }
  }
public:
  state size () { return new_state_counter; }


  // modify the DFA's transition function
  // to accept string s, starting from the initial state
  void accept( string const &s ) {
    current = START;
    for( char c:s ) {
      accept(c);
    }
    accept_states.insert(current);
  }
};


typedef function<bool(string const &)> filter_fn;

bool lowercase_first( string const &w ) {
  return w.size() > 0 ? islower(w[0]) : false;
}

filter_fn begins_with( string const &prefix ) {
  return [prefix] ( string const &w ) {
    if( prefix.size() > w.size() ) {
      return false;
    } else {
      for( size_t i=0; i<prefix.size(); ++i ) {
	if( prefix[i] != w[i] ) { 
	  return false; 
	} 
      }
      return true;
    }
  };
}

filter_fn And( filter_fn a, filter_fn b ) {
  return [a,b]( string const &w ) {
    return a(w) && b(w);
  };
}

filter_fn Or( filter_fn a, filter_fn b ) {
  return [a,b]( string const &w ) {
    return a(w) && b(w);
  };
}

void write_gv( dfa &d, ostream &out ) {
  out << "digraph words {\n";
  out << "\t" << "size=\"20,20\";\n";
  for( auto it=d.transition.t.begin(); it != d.transition.t.end(); ++it ) {
    out << "\t\"" << (it->first.s) << "\" -> \"" << (it->second) << "\"\t [label=\"" << it->first.c << "\"];\n";
  }
  out << "}\n";
}
