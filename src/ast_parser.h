//----------------------------------------------------------------------
// FILE: ast_parser.h
// DATE: CPSC 326, Spring 2023
// AUTH: Evan Shoemaker
// DESC: Generates Abstract Syntax Tree from file
//----------------------------------------------------------------------

#ifndef AST_PARSER_H
#define AST_PARSER_H

#include "mypl_exception.h"
#include "lexer.h"
#include "ast.h"


class ASTParser
{
public:

  // crate a new recursive descent parer
  ASTParser(const Lexer& lexer);

  // run the parser
  Program parse();
  
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
  void struct_def(Program& p);
  void fun_def(Program& s);
  void fields(StructDef& s);
  void data_type(VarDef& v);
  bool base_type();
  void params(FunDef& f);
  void statement(FunDef& f);
  void return_statement(ReturnStmt& r);
  void expression(Expr& e);
  void rvalue(SimpleTerm& st);
  void new_rvalue(NewRValue& n);
  void call_expr(CallExpr& c);
  void var_rvalue(VarRValue& v);
  void assign_statement(AssignStmt& a);
  void vdecl_statement(VarDeclStmt& v);
  void if_statement(IfStmt& i);
  void if_statement_t(IfStmt& i);
  void while_statement(WhileStmt& w);
  void for_statement(ForStmt& fo);
  void lvalue(AssignStmt& as);
};


#endif
