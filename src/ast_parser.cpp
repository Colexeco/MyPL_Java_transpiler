//----------------------------------------------------------------------
// FILE: ast_parser.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Evan Shoemaker
// DESC: Generates Abstract Syntax Tree from file
//----------------------------------------------------------------------

#include "ast_parser.h"
#include <iostream>

using namespace std;


ASTParser::ASTParser(const Lexer& a_lexer)
  : lexer {a_lexer}
{}


void ASTParser::advance()
{
  curr_token = lexer.next_token();
}


void ASTParser::eat(TokenType t, const string& msg)
{
  if (!match(t))
    error(msg);
  advance();
}


bool ASTParser::match(TokenType t)
{
  return curr_token.type() == t;
}


bool ASTParser::match(initializer_list<TokenType> types)
{
  for (auto t : types)
    if (match(t))
      return true;
  return false;
}


void ASTParser::error(const string& msg)
{
  string s = msg + " found '" + curr_token.lexeme() + "' ";
  s += "at line " + to_string(curr_token.line()) + ", ";
  s += "column " + to_string(curr_token.column());
  throw MyPLException::ParserError(s);
}


bool ASTParser::bin_op()
{
  return match({TokenType::PLUS, TokenType::MINUS, TokenType::TIMES,
      TokenType::DIVIDE, TokenType::AND, TokenType::OR, TokenType::EQUAL,
      TokenType::LESS, TokenType::GREATER, TokenType::LESS_EQ,
      TokenType::GREATER_EQ, TokenType::NOT_EQUAL});
}


Program ASTParser::parse()
{
  Program p;
  advance();
  while (!match(TokenType::EOS)) {
    if (match(TokenType::STRUCT))
      struct_def(p);
    else
      fun_def(p);
  }
  eat(TokenType::EOS, "expecting end-of-file");
  return p;
}


void ASTParser::struct_def(Program& p)
{
  StructDef s;
  // TODO: finish this
  eat(TokenType::STRUCT, "expecting a struct");
  s.struct_name = curr_token;
  eat(TokenType::ID, "expecting an ID");
  eat(TokenType::LBRACE, "expecting '{'");
  fields(s);
  eat(TokenType::RBRACE, "expecting '}'");
  p.struct_defs.push_back(s);
}


void ASTParser::fun_def(Program& p)
{
  FunDef f;
  if (match(TokenType::VOID_TYPE))
  {
    f.return_type.is_array = false;
    f.return_type.type_name = "void";
    advance();
  } else {
    VarDef v;
    data_type(v);
    f.return_type.is_array = v.data_type.is_array;
    f.return_type.type_name = v.data_type.type_name;
  }
  f.fun_name = curr_token;
  eat(TokenType::ID, "expecting an ID");
  eat(TokenType::LPAREN, "expecting '('");
  while (!match(TokenType::RPAREN))
  {
    params(f);
  }
  eat(TokenType::RPAREN, "expecting ')'");
  eat(TokenType::LBRACE, "expecting '{'");
  while (!match(TokenType::RBRACE))
  {
    statement(f);
  }
  eat(TokenType::RBRACE, "expecting '}'");
  p.fun_defs.push_back(f);
}


void ASTParser::fields(StructDef& s) {
  VarDef v;
  while (!match(TokenType::RBRACE))
  {
    data_type(v);
    v.var_name = curr_token;
    s.fields.push_back(v);
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

void ASTParser::data_type(VarDef& v) {
  if (base_type())
  {
    v.data_type.is_array = false;
    v.data_type.type_name = curr_token.lexeme();
    advance();
  } else if(match(TokenType::ID)) {
    v.data_type.is_array = false;
    v.data_type.type_name = curr_token.lexeme();
    eat(TokenType::ID, "expecting an ID");
  } else if(match(TokenType::VOID_TYPE)) {
    v.data_type.type_name = curr_token.lexeme();
  } else {
    eat(TokenType::ARRAY, "expecting an array");
    v.data_type.is_array = true;
    if (base_type())
    {
      v.data_type.type_name = curr_token.lexeme();
      advance();
    } else {
      v.data_type.type_name = curr_token.lexeme();
      eat(TokenType::ID, "expecting an ID");
    }
  }
}

bool ASTParser::base_type() {
  if (match(TokenType::INT_TYPE) || match(TokenType::DOUBLE_TYPE) || 
  match(TokenType::BOOL_TYPE) || match(TokenType::CHAR_TYPE) || 
  match(TokenType::STRING_TYPE))
  {
    return true;
  } else {
    return false;
  }
}

void ASTParser::params(FunDef& f) {
  VarDef v;
  data_type(v);
  v.var_name = curr_token;
  f.params.push_back(v);
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

void ASTParser::statement(FunDef& f) {
  if(match(TokenType::RETURN)) {
  shared_ptr <ReturnStmt> r = make_shared <ReturnStmt>();
  return_statement(*r);
  f.stmts.push_back(r);
  } else if (match(TokenType::IF))
  {
    shared_ptr <IfStmt> i = make_shared <IfStmt>();
    if_statement(*i);
    f.stmts.push_back(i);
  } else if (match(TokenType::FOR))
  {
    shared_ptr <ForStmt> fo = make_shared <ForStmt>();
    for_statement(*fo);
    f.stmts.push_back(fo);
  } else if (match(TokenType::WHILE))
  {
    shared_ptr <WhileStmt> w = make_shared <WhileStmt>();
    while_statement(*w);
    f.stmts.push_back(w);
  } else if (match(TokenType::ID))
  {
    Token ID = curr_token;
    advance();
    if (match(TokenType::LPAREN))
    {
      CallExpr c;
      c.fun_name = ID;
      call_expr(c);
      shared_ptr <CallExpr> ca = make_shared <CallExpr>(c);
      f.stmts.push_back(ca);
    } else if (match({TokenType::ASSIGN,TokenType::DOT,TokenType::LBRACKET})) {
      AssignStmt as;
      VarRef ref;
      ref.var_name = ID;
      as.lvalue.push_back(ref);
      assign_statement(as);
      shared_ptr <AssignStmt> a = make_shared <AssignStmt>(as);
      f.stmts.push_back(a);
    } else if (match(TokenType::ID)) {
      VarDeclStmt va;
      VarDef v;
      if (match(TokenType::ARRAY))
      {
        v.data_type.is_array = true;
        advance();
      }
      v.data_type.type_name = ID.lexeme(); //grab datatype since vdecl_statement starts at ID
      va.var_def = v;
      vdecl_statement(va);
      shared_ptr <VarDeclStmt> var = make_shared <VarDeclStmt>(va);
      f.stmts.push_back(var);
    } else {
      error("Invalid token in statement");
    }
  } else {
    VarDeclStmt va;
    VarDef v;
    data_type(v);
    va.var_def = v;
    vdecl_statement(va);
    shared_ptr <VarDeclStmt> var = make_shared <VarDeclStmt>(va);
    f.stmts.push_back(var);
  }
}

void ASTParser::return_statement(ReturnStmt& r) {
  eat(TokenType::RETURN, "expecting 'return'");
  Expr e;
  expression(e);
  r.expr = e;
}

void ASTParser::expression(Expr& e) {
  if (match(TokenType::NOT))
  {
    e.negated = true;
    eat(TokenType::NOT, "expecting 'not'");
    expression(e);
  } else if (match(TokenType::LPAREN)) {
    shared_ptr <ComplexTerm> t = make_shared <ComplexTerm>();
    eat(TokenType::LPAREN, "expecting '('");
    expression(t->expr);
    e.first = t;
    eat(TokenType::RPAREN, "expecting a ')'");
  } else {
    SimpleTerm st;
    rvalue(st);
    shared_ptr <SimpleTerm> str = make_shared <SimpleTerm>(st);
    e.first = str;
  }
  if (bin_op()) {
    e.op = curr_token;
    advance();
    e.rest = make_shared <Expr>();
    expression(*e.rest);
  }
}

void ASTParser::rvalue(SimpleTerm& st) {
  if (match(TokenType::INT_VAL) || match(TokenType::DOUBLE_VAL)
  || match(TokenType::BOOL_VAL) || match(TokenType::CHAR_VAL)
  || match(TokenType::STRING_VAL))
  {
    SimpleRValue r;
    r.value = curr_token;
    shared_ptr <SimpleRValue> rv = make_shared <SimpleRValue>(r);
    st.rvalue = rv;
    advance();
  } else if (match(TokenType::NEW)) {
    NewRValue n;
    new_rvalue(n);
    shared_ptr <NewRValue> ne = make_shared <NewRValue>(n);
    st.rvalue = ne;
  } else if ((match(TokenType::NULL_VAL))) {
    SimpleRValue r;
    r.value = curr_token;
    shared_ptr <SimpleRValue> rv = make_shared <SimpleRValue>(r);
    st.rvalue = rv;
    advance();
  } else {
    CallExpr c;
    c.fun_name = curr_token;
    VarRValue v;
    VarRef va;
    va.var_name = curr_token;
    v.path.push_back(va);
    eat(TokenType::ID, "expecting an ID");
    if (match(TokenType::LPAREN)) {
      call_expr(c);
      shared_ptr <CallExpr> ca = make_shared <CallExpr>(c);
      st.rvalue = ca;
    } else {
      var_rvalue(v);
      shared_ptr <VarRValue> var = make_shared <VarRValue>(v);
      st.rvalue = var;
    }
  }
}

void ASTParser::new_rvalue(NewRValue& n) {
  eat(TokenType::NEW, "expecting 'new'");
  n.type = curr_token;
  if(match(TokenType::ID)) {
    advance();
    if (match(TokenType::LBRACKET))
    {
      eat(TokenType::LBRACKET, "expecting a '['");
      Expr e;
      expression(e);
      n.array_expr = e;
      eat(TokenType::RBRACKET, "expecting a ']'");
    }
  } else if (base_type()) {
    advance();
    eat(TokenType::LBRACKET, "expecting a '['");
    Expr e;
    expression(e);
    n.array_expr = e;
    eat(TokenType::RBRACKET, "expecting a ']'");
  }
}

//STARTS ON SECOND TOKEN '{'
void ASTParser::call_expr(CallExpr& c) {
  eat(TokenType::LPAREN, "expecting '('");
  if (match(TokenType::RPAREN))
  {
    eat(TokenType::RPAREN, "expecting ')'");
  } else {
    while (!match(TokenType::RPAREN))
    {
      Expr e;
      expression(e);
      c.args.push_back(e);
      if (match(TokenType::COMMA))
      {
        advance();
      }
    }
    eat(TokenType::RPAREN, "expecting ')'");
  }
}

//STARTS ON SECOND TOKEN '.' or '{'
void ASTParser::var_rvalue(VarRValue& v) {
  while (match({TokenType::DOT, TokenType::LBRACKET}))
  {
    if (match(TokenType::DOT))
    {
      eat(TokenType::DOT, "expecting '.'");
      VarRef va;
      va.var_name = curr_token;
      v.path.push_back(va);
      eat(TokenType::ID, "expecting an ID");
    } else if (match(TokenType::LBRACKET))
    {
      advance();
      Expr e;
      expression(e);
      v.path.back().array_expr = e;
      eat(TokenType::RBRACKET, "expecting ']'");
    }
  }
}

//STARTS ON SECOND TOKEN '.' or nothing
void ASTParser::lvalue(AssignStmt& as) {
  int curr = 0;
  while (match({TokenType::DOT,TokenType::LBRACKET}))
  {
    if(match(TokenType::DOT)) {
      VarRef ref;
      advance();
      ref.var_name = curr_token;
      as.lvalue.push_back(ref);
      eat(TokenType::ID, "expecting an ID");
      curr++;
    } else if (match(TokenType::LBRACKET))
    {
      advance();
      Expr e;
      expression(e);
      as.lvalue[curr].array_expr = e;
      eat(TokenType::RBRACKET, "expecting ']'");
    }
  }
}

//ASSUMES ID HAS ALREADY BEEN EATEN (from lvalue)
void ASTParser::assign_statement(AssignStmt& as) {
  while (!match(TokenType::ASSIGN))
  {
    //eat(TokenType::ID, "expecting an ID");
    lvalue(as);
  }
  eat(TokenType::ASSIGN, "expecting '='");
  Expr e;
  expression(e);
  as.expr = e;
}

void ASTParser::if_statement(IfStmt& i) {
  BasicIf ba;
  FunDef f;
  eat(TokenType::IF, "expecting 'if'");
  eat(TokenType::LPAREN, "expecting '('");
  Expr e;
  expression(e);
  ba.condition = e;
  eat(TokenType::RPAREN, "expecting ')'");
  eat(TokenType::LBRACE, "expecting '{'");
  if (match(TokenType::RBRACE))
  {
    advance();
  } else {
    while (!match(TokenType::RBRACE))
    {
      statement(f);
      ba.stmts = f.stmts;
    }
    eat(TokenType::RBRACE, "expecting '}'");
  }
  i.if_part = ba;
  if_statement_t(i);
}

void ASTParser::if_statement_t(IfStmt& i) {
  FunDef f;
  BasicIf ba;
  if (match(TokenType::ELSEIF))
  {
    advance();
    eat(TokenType::LPAREN, "expecting '('");
    Expr e;
    expression(e);
    ba.condition = e;
    eat(TokenType::RPAREN, "expecting a ')'");
    eat(TokenType::LBRACE, "expecting a '{'");
    while (!match(TokenType::RBRACE))
    {
      statement(f);
      ba.stmts = f.stmts;
    }
    eat(TokenType::RBRACE, "expecting a '}'"); 
    i.else_ifs.push_back(ba);
    if_statement_t(i);
  } else if (match(TokenType::ELSE)) {
    advance();
    eat(TokenType::LBRACE, "expecting a '{'");
    while (!match(TokenType::RBRACE))
    {
      statement(f);
      i.else_stmts = f.stmts;
    }
    eat(TokenType::RBRACE, "expecting a '}'");
  }
}

void ASTParser::for_statement(ForStmt& fo) {
  FunDef f;
  eat(TokenType::FOR, "expecting 'for'");
  eat(TokenType::LPAREN, "expecting a '('");
  VarDef v;
  data_type(v);
  VarDeclStmt va;
  va.var_def = v;
  vdecl_statement(va);
  fo.var_decl = va;
  eat(TokenType::SEMICOLON, "expecting a ';'");
  Expr e;
  expression(e);
  fo.condition = e;
  eat(TokenType::SEMICOLON, "expecting a ';'");
  AssignStmt a;
  VarRef vr;
  vr.var_name = curr_token;
  a.lvalue.push_back(vr);
  eat(TokenType::ID, "expecting an ID");
  assign_statement(a);
  fo.assign_stmt = a;
  eat(TokenType::RPAREN, "expecting a ')'");
  eat(TokenType::LBRACE, "expecting a'{'");
  while (!match(TokenType::RBRACE))
  {
    statement(f);
  }
   fo.stmts = f.stmts;
  eat(TokenType::RBRACE, "expecting a '}'");
}

void ASTParser::while_statement(WhileStmt& w) {
  FunDef f;
  eat(TokenType::WHILE, "expecting 'while'");
  eat(TokenType::LPAREN, "expecting a '('");
  Expr e;
  expression(e);
  w.condition = e;
  eat(TokenType::RPAREN, "expecting ')'");
  eat(TokenType::LBRACE, "expecting '{'");
  while (!match(TokenType::RBRACE))
  {
    statement(f);
    w.stmts = f.stmts;
  }
  eat(TokenType::RBRACE, "expecting a '}'");
}
//STARTS ON SECOND TOKEN, DOESNT CHECK DATA TYPE
void ASTParser::vdecl_statement(VarDeclStmt& v) {
  v.var_def.var_name = curr_token;
  eat(TokenType::ID, "expecting an ID");
  eat(TokenType::ASSIGN, "expecting '='");
  Expr e;
  expression(e);
  v.expr = e;
}
