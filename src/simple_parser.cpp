//----------------------------------------------------------------------
// FILE: simple_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Evan Shoemaker
// DESC: Simple parser checks for syntactically well-formed token stream
// (no abstract syntax tree yet)
//----------------------------------------------------------------------

#include "simple_parser.h"
#include <iostream>

using namespace std;

SimpleParser::SimpleParser(const Lexer& a_lexer)
  : lexer {a_lexer}
{}


void SimpleParser::advance()
{
  curr_token = lexer.next_token();
}


void SimpleParser::eat(TokenType t, const std::string& msg)
{
  if (!match(t))
    error(msg);
  advance();
}


bool SimpleParser::match(TokenType t)
{
  return curr_token.type() == t;
}


bool SimpleParser::match(std::initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}


void SimpleParser::error(const std::string& msg)
{
  std::string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + std::to_string(curr_token.line()) + ", ";
  s += "column " + std::to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}


bool SimpleParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
      TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
      TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
      TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}


void SimpleParser::parse()
{
  advance();
  while (!match(TokenType::EOS)) {
    if (match(TokenType::STRUCT))
      struct_def();
    else
      fun_def();
  }
  eat(TokenType::EOS, "expecting end-of-file");
}


void SimpleParser::struct_def()
{
  // TODO: finish this
  eat(TokenType::STRUCT, "expecting a struct");
  eat(TokenType::ID, "expecting an ID");
  eat(TokenType::LBRACE, "expecting '{'");
  fields();
  eat(TokenType::RBRACE, "expecting '}'");
}


void SimpleParser::fun_def()
{
  // TODO: finish this
  if (match(TokenType::VOID_TYPE))
  {
    advance();
  } else {
    data_type();
  }
  eat(TokenType::ID, "expecting an ID");
  eat(TokenType::LPAREN, "expecting '('");
  while (!match(TokenType::RPAREN))
  {
    params();
  }
  eat(TokenType::RPAREN, "expecting ')'");
  eat(TokenType::LBRACE, "expecting '{'");
  while (!match(TokenType::RBRACE))
  {
    statement();
  }
  eat(TokenType::RBRACE, "expecting '}'");
}


// TODO: Implement the rest of your recursive descent functions
//       here. See simple_parser.h
void SimpleParser::fields() {
  while (!match(TokenType::RBRACE))
  {
    data_type();
    eat(TokenType::ID, "expecting an ID");
    if (match(TokenType::COMMA))
    {
      advance();
      if (match(TokenType::RBRACE)) {
        error("Too many commas, there should be one after each field except the last");
      }
    } else {
      break;
    }
  }
}

void SimpleParser::data_type() {
  if (base_type())
  {
    advance();
  } else if(match(TokenType::ID)) {
    eat(TokenType::ID, "expecting an ID");
  } else {
    eat(TokenType::ARRAY, "expecting an array");
    if (base_type())
    {
      advance();
    } else {
      eat(TokenType::ID, "expecting an ID");
    }
  }
}

bool SimpleParser::base_type() {
  if (match(TokenType::INT_TYPE) || match(TokenType::DOUBLE_TYPE) || 
  match(TokenType::BOOL_TYPE) || match(TokenType::CHAR_TYPE) || 
  match(TokenType::STRING_TYPE))
  {
    return true;
  } else {
    return false;
  }
}

void SimpleParser::params() {
  data_type();
  eat(TokenType::ID, "expecting an ID");
  if (match(TokenType::COMMA))
  {
    advance();
    if (match(TokenType::RPAREN))
    {
      error("Too many commas, there should be one after each parameter except the last");
    }
  }
}

void SimpleParser::statement() {
  if(match(TokenType::RETURN)) {
  return_statement();
  } else if (match(TokenType::IF))
  {
    if_statment();
  } else if (match(TokenType::FOR))
  {
    for_statement();
  } else if (match(TokenType::WHILE))
  {
    while_statement();
  } else if (match(TokenType::ID))
  {
    advance();
    if (match(TokenType::LPAREN))
    {
      call_expr();
    } else if (match({TokenType::ASSIGN,TokenType::DOT,TokenType::LBRACKET})) {
      assign_statement();
    } else if (match(TokenType::ID)) {
      vdecl_statement();
    } else {
      error("Invalid token in statement");
    }
  } else
  {
    data_type();
    vdecl_statement();
  }
}

void SimpleParser::return_statement() {
  eat(TokenType::RETURN, "expecting 'return'");
  expression();
}

void SimpleParser::expression() {
  // advance();
  if (match(TokenType::NOT))
  {
    eat(TokenType::NOT, "expecting 'not'");
    expression();
  } else if (match(TokenType::LPAREN)) {
    eat(TokenType::LPAREN, "expecting '('");
    expression();
    eat(TokenType::RPAREN, "expecting a ')'");
  } else {
    rvalue();
  }
  if (bin_op()) {
    advance();
    expression();
  }
}

void SimpleParser::rvalue() {
  if (match(TokenType::INT_VAL) || match(TokenType::DOUBLE_VAL)
  || match(TokenType::BOOL_VAL) || match(TokenType::CHAR_VAL)
  || match(TokenType::STRING_VAL))
  {
    advance();
  } else if (match(TokenType::NEW)) {
    new_rvalue();
  } else if ((match(TokenType::NULL_VAL))) {
    eat(TokenType::NULL_VAL, "expecting 'null'");
  } else {
    eat(TokenType::ID, "expecting an ID");
    if (match(TokenType::LPAREN)) {
      call_expr();
    } else {
      var_rvalue();
    }
  }
  if (bin_op())
  {
    advance();
    expression();
  }
}

void SimpleParser::new_rvalue() {
  eat(TokenType::NEW, "expecting 'new'");
  if(match(TokenType::ID)) {
    advance();
    if (match(TokenType::LBRACKET))
    {
      eat(TokenType::LBRACKET, "expecting a '['");
      expression();
      eat(TokenType::RBRACKET, "expecting a ']'");
    }
  } else if (base_type()) {
    advance();
    eat(TokenType::LBRACKET, "expecting a '['");
    expression();
    eat(TokenType::RBRACKET, "expecting a ']'");
  }
}

//STARTS ON SECOND TOKEN '{'
void SimpleParser::call_expr() {
  eat(TokenType::LPAREN, "expecting '{'");
  if (match(TokenType::RPAREN))
  {
    eat(TokenType::RPAREN, "expecting '}'");
  } else {
    while (!match(TokenType::RPAREN))
    {
      expression();
      if (match(TokenType::COMMA))
      {
        advance();
      }
    }
    eat(TokenType::RPAREN, "expecting '}'");
  }
}

//STARTS ON SECOND TOKEN '.' or '{'
void SimpleParser::var_rvalue() {
  while (match({TokenType::DOT, TokenType::LBRACKET}))
  {
    if (match(TokenType::DOT))
    {
      eat(TokenType::DOT, "expecting '.'");
      eat(TokenType::ID, "expecting an ID");
    } else if (match(TokenType::LBRACKET))
    {
      advance();
      expression();
      eat(TokenType::RBRACKET, "expecting ']'");
    }
  }
}

//STARTS ON SECOND TOKEN '.' or nothing
void SimpleParser::lvalue() {
  while (match({TokenType::DOT,TokenType::LBRACKET}))
  {
    if(match(TokenType::DOT)) {
    advance();
    eat(TokenType::ID, "expecting an ID");
    } else if (match(TokenType::LBRACKET))
    {
      advance();
      expression();
      eat(TokenType::RBRACKET, "expecting ']'");
    }
  }
}

//ASSUMES ID HAS ALREADY BEEN EATEN (from lvalue)
void SimpleParser::assign_statement() {
  while (!match(TokenType::ASSIGN))
  {
    //eat(TokenType::ID, "expecting an ID");
    lvalue();
  }
  eat(TokenType::ASSIGN, "expecting '='");
  expression();
}

void SimpleParser::if_statment() {
  eat(TokenType::IF, "expecting 'if'");
  eat(TokenType::LPAREN, "expecting '('");
  expression();
  eat(TokenType::RPAREN, "expecting ')'");
  eat(TokenType::LBRACE, "expecting '{'");
  if (match(TokenType::RBRACE))
  {
    advance();
  } else {
    while (!match(TokenType::RBRACE))
    {
      statement();
    }
    eat(TokenType::RBRACE, "expecting '}'");
  }
  if_statement_t();
}

void SimpleParser::if_statement_t() {
  if (match(TokenType::ELSEIF))
  {
    advance();
    eat(TokenType::LPAREN, "expecting '('");
    expression();
    eat(TokenType::RPAREN, "expecting a ')'");
    eat(TokenType::LBRACE, "expecting a '{'");
    while (!match(TokenType::RBRACE))
    {
      statement();
    }
    eat(TokenType::RBRACE, "expecting a '}'"); 
    if_statement_t();
  } else if (match(TokenType::ELSE)) {
    advance();
    eat(TokenType::LBRACE, "expecting a '{'");
    while (!match(TokenType::RBRACE))
    {
      statement();
    }
    eat(TokenType::RBRACE, "expecting a '}'");
  }
}

void SimpleParser::for_statement() {
  eat(TokenType::FOR, "expecting 'for'");
  eat(TokenType::LPAREN, "expecting a '('");
  data_type();
  vdecl_statement();
  eat(TokenType::SEMICOLON, "expecting a ';'");
  expression();
  eat(TokenType::SEMICOLON, "expecting a ';'");
  eat(TokenType::ID, "expecting an ID");
  assign_statement();
  eat(TokenType::RPAREN, "expecting a ')'");
  eat(TokenType::LBRACE, "expecting a'{'");
  while (!match(TokenType::RBRACE))
  {
    statement();
  }
  eat(TokenType::RBRACE, "expecting a '}'");
}

void SimpleParser::while_statement() {
  eat(TokenType::WHILE, "expecting 'while'");
  eat(TokenType::LPAREN, "expecting a '('");
  expression();
  eat(TokenType::RPAREN, "expecting ')'");
  eat(TokenType::LBRACE, "expecting '{'");
  while (!match(TokenType::RBRACE))
  {
    statement();
  }
  eat(TokenType::RBRACE, "expecting a '}'");
}

//STARTS ON SECOND TOKEN, DOESNT CHECK DATA TYPE
void SimpleParser::vdecl_statement() {
  eat(TokenType::ID, "expecting an ID");
  eat(TokenType::ASSIGN, "expecting '='");
  expression();
}