//----------------------------------------------------------------------
// FILE: print_visitor.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Evan Shoemaker
// DESC: Pretty-Prints MyPL file
//----------------------------------------------------------------------

#include "print_visitor.h"
#include <iostream>

using namespace std;


PrintVisitor::PrintVisitor(ostream& output)
  : out(output)
{
  
}


void PrintVisitor::inc_indent()
{
  indent += INDENT_AMT;
}


void PrintVisitor::dec_indent()
{
  indent -= INDENT_AMT;
}


void PrintVisitor::print_indent()
{
  out << string(indent, ' ');
}


void PrintVisitor::visit(Program& p)
{
  cout << endl;
  for (auto struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto fun_def : p.fun_defs)
    fun_def.accept(*this);
}

void PrintVisitor::visit(FunDef& f) {
  cout << endl;
  cout << f.return_type.type_name << " " << f.fun_name.lexeme() << "(";
  for (int i = 0; i < f.params.size(); i++)
  {
    cout << f.params.at(i).data_type.type_name << " " << f.params.at(i).var_name.lexeme();
    if (i < (f.params.size() - 1))
    {
      cout << ", ";
    }
  }
  cout << ") {" << endl;
  inc_indent();
  for (auto statement : f.stmts)
  {
    print_indent();
    statement->accept(*this);
    cout << endl;
  }
  dec_indent();
  cout << "}" << endl;
}

void PrintVisitor::visit(StructDef& s) {
  cout << "struct " << s.struct_name.lexeme() << " {" << endl;
  inc_indent();
  for (int i = 0; i < s.fields.size(); i++)
  {
    print_indent();
    cout << s.fields.at(i).data_type.type_name << " " << s.fields.at(i).var_name.lexeme();
    if (i <= (s.fields.size() - 2))
    {
      cout << ",";
    } 
    cout << endl;
  }
  dec_indent();
  cout << "}" << endl;
}

void PrintVisitor::visit(ReturnStmt& s) {
  cout << "return ";
  s.expr.accept(*this);
}

void PrintVisitor::visit(WhileStmt& s) {
  cout << "while" << " (";
  s.condition.accept(*this);
  cout << ")" << " {" << endl;
  inc_indent();
  for (auto stmt : s.stmts)
  {
    print_indent();
    stmt->accept(*this);
    cout << endl;
  }
  dec_indent();
  print_indent();
  cout << "}";
}
void PrintVisitor::visit(ForStmt& s) {
  cout << "for (";
  s.var_decl.accept(*this);
  cout << "; ";
  s.condition.accept(*this);
  cout << "; ";
  s.assign_stmt.accept(*this);
  cout << ") {" << endl;
  inc_indent();
  print_indent();
  for (auto stmt : s.stmts)
  {
    stmt->accept(*this);
  }
  dec_indent();
  cout << endl;
  print_indent();
  cout << "}";
}

void PrintVisitor::visit(IfStmt& s) {
  cout << "if (";
  s.if_part.condition.accept(*this);
  cout << ") {" << endl;
  inc_indent();
  for (auto stmt : s.if_part.stmts)
  {
    print_indent();
    stmt->accept(*this);
    cout << endl;
  }
  dec_indent();
  print_indent();
  cout << "}";
  if (s.else_ifs.size() > 0 || s.else_stmts.size() > 0)
  {
    cout << endl;
  }
  
  if (s.else_ifs.size() > 0)
  {
    for (auto elseif : s.else_ifs)
    {
    print_indent();
    cout << "elseif (";
    elseif.condition.accept(*this);
    cout << ") {" << endl;
    inc_indent();
    for (auto stmt : elseif.stmts)
    {
      print_indent();
      stmt->accept(*this);
      cout << endl;
    }
    dec_indent();
    print_indent();
    cout << "}";
    cout << endl;
    }
  }
  if (s.else_stmts.size() > 0)
  {
    print_indent();
    cout << "else {" << endl;
    inc_indent();
    for (auto stmt : s.else_stmts)
    {
      print_indent();
      stmt->accept(*this);
      cout << endl;
    }
    dec_indent();
    print_indent();
    cout << "}";
  }
}

void PrintVisitor::visit(VarDeclStmt& s) {
  cout << s.var_def.data_type.type_name << " " << s.var_def.var_name.lexeme();
  cout << " = ";
  s.expr.accept(*this);
}

void PrintVisitor::visit(AssignStmt& s) {
  for (int i = 0; i < s.lvalue.size(); i++)
  {
    cout << s.lvalue.at(i).var_name.lexeme();
    if (s.lvalue.at(i).array_expr.has_value())
    {
      s.lvalue.at(i).array_expr.value().accept(*this);
    }
    if (i < (s.lvalue.size() - 1))
    {
      cout << ".";
    }
  }
  cout << " = ";
  s.expr.accept(*this);
}

void PrintVisitor::visit(CallExpr& e) {
  cout << e.fun_name.lexeme() << "(";
  for (int i = 0; i < e.args.size(); i++)
  {
    e.args.at(i).accept(*this);
    if (i < (e.args.size() - 1))
    {
      cout << ", ";
    }
    
  }
  cout << ")";
}

void PrintVisitor::visit(Expr& e) {
  if(e.negated) {
    cout << "not ";
    cout << "(";
  }
  e.first->accept(*this);
  if (e.op.has_value())
  {
    cout << " " << e.op.value().lexeme() << " ";
    e.rest->accept(*this);
  }
  if(e.negated) {
    cout << ")";
  }
}

void PrintVisitor::visit(SimpleTerm& t) {
  t.rvalue->accept(*this);
}

void PrintVisitor::visit(ComplexTerm& t) {
  cout << "(";
  t.expr.accept(*this);
  cout << ")";
}

void PrintVisitor::visit(SimpleRValue& v) {
  if (v.value.type() == TokenType::CHAR_VAL)
  {
    cout << "'" << v.value.lexeme() << "'";
  } else if (v.value.type() == TokenType::STRING_VAL) {
    cout << "\"" << v.value.lexeme() << "\"";
  } else {
    cout << v.value.lexeme();
  }
}

void PrintVisitor::visit(NewRValue& v) {
  cout << "new " << v.type.lexeme();
}

void PrintVisitor::visit(VarRValue& v) {
  for (int i = 0; i < v.path.size(); i++)
  {
    cout << v.path.at(i).var_name.lexeme();
    if (v.path.at(i).array_expr.has_value())
    {
      v.path.at(i).array_expr->accept(*this);
    }
    if (i < (v.path.size() - 1))
    {
      cout << ".";
    }
  }
}