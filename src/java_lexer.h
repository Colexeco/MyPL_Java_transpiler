//----------------------------------------------------------------------
// FILE: java_lexer.h
// DATE: CPSC 326, Spring 2023
// NAME: Evan Shoemaker
// DESC: Class file for lexical analyzer. A lexer has an input object,
//       line, column, read(), peek(), and error() methods  
//----------------------------------------------------------------------

#ifndef JAVA_LEXER_H
#define JAVA_LEXER_H

#include <istream>
#include <string>
#include "mypl_exception.h"
#include "token.h"


class JavaLexer {
public:

  // Construct a new lexer from the given input stream
  JavaLexer(std::istream& input_stream);

  // Return the next available token in the input stream. Returns the
  // EOS (end of stream) token if no more tokens exist in the input
  // stream.
  Token next_token();
  
private:

  // input stream
  std::istream& input;

  // current line
  int line;
  
  // current column
  int column;

  // returns single character from input stream, advances stream, and
  // increments column number
  char read();

  // returns single character from input stream without advancing and
  // without incrementing column number
  char peek();

  // create and throw a MyPLException object (exits lexer)
  void error(const std::string& msg, int line, int column) const;
  
};

#endif
