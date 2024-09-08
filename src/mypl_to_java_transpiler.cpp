//----------------------------------------------------------------------
// FILE: print_visitor.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Evan Shoemaker
// DESC: MyPL to Java transpiler
//----------------------------------------------------------------------

#include "mypl_to_java_transpiler.h"
#include <iostream>

using namespace std;


MyPLtoJavaTranspiler::MyPLtoJavaTranspiler(ostream& output)
  : out(output)
{
  
}


void MyPLtoJavaTranspiler::inc_indent()
{
  indent += INDENT_AMT;
}


void MyPLtoJavaTranspiler::dec_indent()
{
  indent -= INDENT_AMT;
}


void MyPLtoJavaTranspiler::print_indent()
{
  out << string(indent, ' ');
}


void MyPLtoJavaTranspiler::visit(Program& p)
{
  cout << "import java.util.*;" << endl;
  cout << "import java.util.Scanner;" << endl;
  cout << endl;
  cout << "class Program {" << endl;
  // inc_indent();
  // cout << "public static void main(String args[]) {" << endl;
  cout << "Scanner input = new Scanner(System.in);"<< endl; //FIXME, temporary solution for input(), should be placed closer to call
  for (auto struct_def : p.struct_defs) {
    struct_def.accept(*this);
  }

  for (auto fun_def : p.fun_defs) {
    fun_def.accept(*this);
  }
  // cout << "}" << endl;
  // dec_indent();
  cout << "}";
}

void MyPLtoJavaTranspiler::visit(FunDef& f) {
  cout << endl;

  cout << "public ";

  if (f.fun_name.lexeme() == "main") {
    cout << "static ";
  }

  cout << f.return_type.type_name << " " << f.fun_name.lexeme() << "(";
  if (f.fun_name.lexeme() == "main") {
    cout << "String[] args";
  } else {
    for (int i = 0; i < f.params.size(); i++)
    {
      cout << f.params.at(i).data_type.type_name << " " << f.params.at(i).var_name.lexeme();
      if (i < (f.params.size() - 1))
      {
        cout << ", ";
      }
    }
  }
  cout << ") {" << endl;
  inc_indent();
  if (f.fun_name.lexeme() == "main")
  {
    cout << "Program p = new Program();" << endl;
  }
  
  for (auto statement : f.stmts)
  {
    print_indent();
    statement->accept(*this);
    cout << ";";
    cout << endl;
  }
  dec_indent();
  cout << "}" << endl;
}

void MyPLtoJavaTranspiler::visit(StructDef& s) {
  cout << "class " << s.struct_name.lexeme() << " {" << endl;
  inc_indent();
  for (int i = 0; i < s.fields.size(); i++)
  {
    print_indent();

    cout << "public " << s.fields.at(i).data_type.type_name;
    cout << " " << s.fields.at(i).var_name.lexeme();
    if (i <= (s.fields.size() - 1))
    {
      cout << ";";
    } 
    cout << endl;
  }
  dec_indent();
  cout << "}" << endl;
}

void MyPLtoJavaTranspiler::visit(ReturnStmt& s) {
  cout << "return ";
  s.expr.accept(*this);
  cout << ";";
}

void MyPLtoJavaTranspiler::visit(WhileStmt& s) {
  cout << "while" << " (";
  s.condition.accept(*this);
  cout << ")" << " {" << endl;
  inc_indent();
  for (auto stmt : s.stmts)
  {
    print_indent();
    stmt->accept(*this);
    cout << ";";
    cout << endl;
  }
  dec_indent();
  print_indent();
  cout << "}";
}
void MyPLtoJavaTranspiler::visit(ForStmt& s) {
  cout << "for (";
  s.var_decl.accept(*this);
  // cout << "; ";
  s.condition.accept(*this);
  cout << "; ";
  s.assign_stmt.accept(*this);
  cout << ") {" << endl;
  inc_indent();
  print_indent();
  for (auto stmt : s.stmts)
  {
    stmt->accept(*this);
    cout << ";";
  }
  dec_indent();
  cout << endl;
  print_indent();
  cout << "}";
}

void MyPLtoJavaTranspiler::visit(IfStmt& s) {
  cout << "if (";
  s.if_part.condition.accept(*this);
  cout << ") {" << endl;
  inc_indent();
  for (auto stmt : s.if_part.stmts)
  {
    print_indent();
    stmt->accept(*this);
    cout << ";";
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
    cout << "else if (";
    elseif.condition.accept(*this);
    cout << ") {" << endl;
    inc_indent();
    for (auto stmt : elseif.stmts)
    {
      print_indent();
      stmt->accept(*this);
      cout << ";";
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
      cout << ";";
      cout << endl;
    }
    dec_indent();
    print_indent();
    cout << "}";
  }
}

void MyPLtoJavaTranspiler::visit(VarDeclStmt& s) {
  cout << s.var_def.data_type.type_name;
  if ((s.expr.first_token().type() == TokenType::STRING_TYPE) ||
  (s.expr.first_token().type() == TokenType::INT_TYPE) ||
  (s.expr.first_token().type() == TokenType::DOUBLE_TYPE) ||
  (s.expr.first_token().type() == TokenType::BOOL_TYPE) ||
  (s.expr.first_token().type() == TokenType::CHAR_TYPE)) {
    //Java arrays require datatype to have "[]", as in String[] strings = new String[];
    cout << "[]";
  }

  cout<< " " << s.var_def.var_name.lexeme();
  cout << " = ";
  s.expr.accept(*this);
  cout << ";";
}

void MyPLtoJavaTranspiler::visit(AssignStmt& s) {
  for (int i = 0; i < s.lvalue.size(); i++)
  {
    cout << s.lvalue.at(i).var_name.lexeme();
    if (s.lvalue.at(i).array_expr.has_value())
    {
      cout << "[";
      s.lvalue.at(i).array_expr.value().accept(*this);
      cout << "]";
    }
    if (i < (s.lvalue.size() - 1))
    {
      cout << ".";
    }
  }
  cout << " = ";
  s.expr.accept(*this);
  //cout << ";";
}

void MyPLtoJavaTranspiler::visit(CallExpr& e) {
  if (e.fun_name.lexeme() == "get")
  {
    e.args.at(1).accept(*this);
    cout << "." << "charAt" << "(";
    e.args.at(0).accept(*this);
    cout << ")";
  } else if (e.fun_name.lexeme() == "length") {
    e.args.at(0).accept(*this);
    cout << "." << "length";
    if (e.args.at(0).first_token().type() == TokenType::STRING_VAL)
    {
      cout << "()";
    }
  } else if (e.fun_name.lexeme() == "input") {
      cout << "input.nextLine();" << endl;
  } else if (e.fun_name.lexeme() == "concat") {
    e.args.at(0).accept(*this);
    cout << ".concat(" ;
    e.args.at(1).accept(*this);
    cout << ")";
  } else {
    if (e.fun_name.lexeme() == "print") {
      cout << "System.out.println" << "(";
    } else if (e.fun_name.lexeme() == "to_string") {
      cout << "toString" << "(";
    } else if (e.fun_name.lexeme() == "to_double") {
      cout << "toDouble" << "(";
    } else if (e.fun_name.lexeme() == "to_int") {
      cout << "toInt" << "(";
    } else {
      cout << e.fun_name.lexeme() << "(";
    }
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
}

void MyPLtoJavaTranspiler::visit(Expr& e) {
  if(e.negated) {
    cout << "!";
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

void MyPLtoJavaTranspiler::visit(SimpleTerm& t) {
  t.rvalue->accept(*this);
}

void MyPLtoJavaTranspiler::visit(ComplexTerm& t) {
  cout << "(";
  t.expr.accept(*this);
  cout << ")";
}

void MyPLtoJavaTranspiler::visit(SimpleRValue& v) {
  if (v.value.type() == TokenType::CHAR_VAL)
  {
    cout << "'" << v.value.lexeme() << "'";
  } else if (v.value.type() == TokenType::STRING_VAL) {
    cout << "\"" << v.value.lexeme() << "\"";
  } else {
    cout << v.value.lexeme();
  }
}

void MyPLtoJavaTranspiler::visit(NewRValue& v) {
  cout << "new " << v.type.lexeme();

  if ((v.type.lexeme() != "String") && (v.type.lexeme() != "double")
  && (v.type.lexeme() != "bool") && (v.type.lexeme() != "char") 
  && (v.type.lexeme() != "int")) //then it must be a struct, (or a class in Java), which needs '()'
  {
    cout << "()";
  }
  
  if (v.array_expr.has_value())
  {
    cout << "[";
    v.array_expr->accept(*this);
    cout << "]";
  }
  
}

void MyPLtoJavaTranspiler::visit(VarRValue& v) {
  for (int i = 0; i < v.path.size(); i++)
  {
    cout << v.path.at(i).var_name.lexeme();
    if (v.path.at(i).array_expr.has_value())
    {
      cout << "[";
      v.path.at(i).array_expr->accept(*this);
      cout << "]";
    }
    if (i < (v.path.size() - 1))
    {
      cout << ".";
    }
  }
}