//----------------------------------------------------------------------
// FILE: simple_parser.h
// DATE: CPSC 326, Spring 2023
// AUTH: Evan Shoemaker
// DESC: Basic interface for our Simple Parser (syntax checker)
//----------------------------------------------------------------------

#ifndef SIMPLE_PARSER_H
#define SIMPLE_PARSER_H

#include "mypl_exception.h"
#include "lexer.h"


class SimpleParser
{
public:

  // crate a new recursive descent parer
  SimpleParser(const Lexer& lexer);

  // run the parser
  void parse();
  
private:
  
  Lexer lexer;
  Token curr_token;
  
  // helper functions
  void advance();
  void eat(TokenType t, const std::string& msg);
  bool match(TokenType t);
  bool match(std::initializer_list<TokenType> types);
  void error(const std::string& msg);
  bool bin_op();

  // recursive descent functions
  void struct_def();
  void fun_def();
  
  // TODO:
  //   -- Add remaining recursive descent function declarations here.
  //   -- Note that you need to follow the general flow of the MyPL
  //      syntax specification, but there may be a few places you will
  //      want to simplify and/or diverge.
  //   -- Be sure to remove this comment block when you are finished.
  void fields();
  void data_type();
  bool base_type();
  void params();
  void statement();
  void return_statement();
  void expression();
  void rvalue();
  void new_rvalue();
  void call_expr();
  void var_rvalue();
  void assign_statement();
  void vdecl_statement();
  void if_statment();
  void if_statement_t();
  void while_statement();
  void for_statement();
  void lvalue();
};


#endif
