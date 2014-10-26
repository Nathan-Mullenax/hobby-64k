#ifndef AST_H
#define AST_H

#include <string>
#include <sstream>
#include <memory>
#include <stdexcept>

using std::runtime_error;
using std::string;
using std::stringstream;
using std::unique_ptr;

class expr
{
 public:
  virtual string
    string_value() const =0;

  virtual int 
    int_value() const
  {
    throw runtime_error("Expression is not an integer.");
  }
  
};

class integer
: public expr
{
 public:
  int value;

  virtual string
    string_value() const
  {
    stringstream ss;
    ss << value;
    return ss.str();
  }

  virtual int
    int_value() const { return value; }
  
};

class binary_operator
: public expr
{
  unique_ptr<expr> lhs;
  unique_ptr<expr> rhs;
  
};



#endif
