//----------------------------------------------------------------------
// FILE: code_generator.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Evan Shoemaker
// DESC: Generates MyPL VM instructions for MyPL code
//----------------------------------------------------------------------

#include <iostream>             // for debugging
#include "code_generator.h"
#include <unordered_set>

using namespace std;

const unordered_set<string> BUILT_INS {"print", "input", "to_string",  "to_int",
  "to_double", "length", "get", "concat"};

// helper function to replace all occurrences of old string with new
void replace_all(string& s, const string& old_str, const string& new_str)
{
  while (s.find(old_str) != string::npos)
    s.replace(s.find(old_str), old_str.size(), new_str);
}


CodeGenerator::CodeGenerator(VM& vm)
  : vm(vm)
{
}


void CodeGenerator::visit(Program& p)
{
  for (auto& struct_def : p.struct_defs)
    struct_def.accept(*this);
  for (auto& fun_def : p.fun_defs)
    fun_def.accept(*this);
}


void CodeGenerator::visit(FunDef& f)
{
  curr_frame = {f.fun_name.lexeme(), (int) f.params.size()};
  var_table.push_environment();

  for (int i = 0; i < f.params.size(); i++)
  {
    this->var_table.add(f.params[i].var_name.lexeme());
    curr_frame.instructions.push_back(VMInstr::STORE(i));
  }

  if (f.stmts.size() == 0)
    {
      curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
      curr_frame.instructions.push_back(VMInstr::RET());
    } else {
      for (int i = 0; i < f.stmts.size(); i++)
      {
        f.stmts.at(i)->accept(*this);

        if (i == (f.stmts.size()))
        {
          if (curr_frame.instructions.at(i).opcode() != OpCode::RET) 
          {
            curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
            curr_frame.instructions.push_back(VMInstr::RET());
          }
        }
      }
    }
  
  vm.add(curr_frame);
  var_table.pop_environment();
}


void CodeGenerator::visit(StructDef& s)
{
  struct_defs[s.struct_name.lexeme()] = s;
}


void CodeGenerator::visit(ReturnStmt& s)
{
  s.expr.accept(*this);
  curr_frame.instructions.push_back(VMInstr::RET());
}


void CodeGenerator::visit(WhileStmt& s)
{
  int top = curr_frame.instructions.size();

  s.condition.accept(*this);

  curr_frame.instructions.push_back(VMInstr::JMPF(-1));

  int jmpf = curr_frame.instructions.size() - 1;

  this->var_table.push_environment();

  for (int i = 0; i < s.stmts.size(); i++) {
    s.stmts[i]->accept(*this);
  }

  this->var_table.pop_environment();

  curr_frame.instructions.push_back(VMInstr::JMP(top));
  curr_frame.instructions.push_back(VMInstr::NOP());

  curr_frame.instructions.at(jmpf) = VMInstr::JMPF(curr_frame.instructions.size());
}


void CodeGenerator::visit(ForStmt& s)
{
  this->var_table.push_environment(); //push env for vardecl

  s.var_decl.accept(*this);

  int top = curr_frame.instructions.size();

  s.condition.accept(*this);

  curr_frame.instructions.push_back(VMInstr::JMPF(-1));

  int jmpf = curr_frame.instructions.size() - 1;

  this->var_table.push_environment();

  for (int i = 0; i < s.stmts.size(); i++) {
    s.stmts[i]->accept(*this);
  }

  this->var_table.pop_environment();

  s.assign_stmt.accept(*this);

  this->var_table.pop_environment();

  curr_frame.instructions.push_back(VMInstr::JMP(top));
  curr_frame.instructions.push_back(VMInstr::NOP());

  curr_frame.instructions.at(jmpf) = VMInstr::JMPF(curr_frame.instructions.size());
}


void CodeGenerator::visit(IfStmt& s)
{
  vector<int> jmp_indices = {}; //save size of instructions at bottom of else-if and if clauses to skip else
  // clause if needed (the case where else-if or if is true and gets executed)

  //if part

  s.if_part.condition.accept(*this);

  curr_frame.instructions.push_back(VMInstr::JMPF(-1));

  int jmpf = curr_frame.instructions.size() - 1;

  this->var_table.push_environment();

  for (int i = 0; i < s.if_part.stmts.size(); i++)
  {
    s.if_part.stmts[i]->accept(*this);
  }

  this->var_table.pop_environment();

  curr_frame.instructions.at(jmpf) = VMInstr::JMPF(curr_frame.instructions.size() + 1);

  jmp_indices.push_back(curr_frame.instructions.size());

  curr_frame.instructions.push_back(VMInstr::JMP(-1));

  //else ifs

  for (int i = 0; i < s.else_ifs.size(); i++)
  {
    s.else_ifs[i].condition.accept(*this);

    curr_frame.instructions.push_back(VMInstr::JMPF(-1));

    jmpf = curr_frame.instructions.size() - 1;

    this->var_table.push_environment();

    for (int j = 0; j < s.else_ifs[i].stmts.size(); j++)
    {
      s.else_ifs[i].stmts[j]->accept(*this);
    }

    this->var_table.pop_environment();

    curr_frame.instructions.at(jmpf) = VMInstr::JMPF(curr_frame.instructions.size() + 1);
  }

  jmp_indices.push_back(curr_frame.instructions.size());

  curr_frame.instructions.push_back(VMInstr::JMP(-1));

  //else stmts

  for (int i = 0; i < s.else_stmts.size(); i++)
  {
    s.else_stmts[i]->accept(*this);
  }

  for (int i = 0; i < jmp_indices.size(); i++)
  {
    curr_frame.instructions.at(jmp_indices.at(i)) = VMInstr::JMP(curr_frame.instructions.size());
  }
}


void CodeGenerator::visit(VarDeclStmt& s)
{
  s.expr.accept(*this);
  this->var_table.add(s.var_def.var_name.lexeme());
  curr_frame.instructions.push_back(VMInstr::STORE(this->var_table.get(s.var_def.var_name.lexeme())));
}


void CodeGenerator::visit(AssignStmt& s)
{ 
  for (int i = 0; i < s.lvalue.size() - 1; i++)
  {
    if (i > 0) {
      curr_frame.instructions.push_back(VMInstr::GETF(s.lvalue[i].var_name.lexeme()));
    } else {
      curr_frame.instructions.push_back(VMInstr::LOAD(var_table.get(s.lvalue[i].var_name.lexeme())));
    }

    if (s.lvalue[i].array_expr.has_value()) //any field could be an array access
    {
      s.lvalue[i].array_expr->accept(*this);
      curr_frame.instructions.push_back(VMInstr::GETI());
    }
  }

  if (s.lvalue[s.lvalue.size() - 1].array_expr.has_value()) //last field could be an array access
  {
    if (s.lvalue.size() == 1) {
      curr_frame.instructions.push_back(VMInstr::LOAD(this->var_table.get(s.lvalue[s.lvalue.size() - 1].var_name.lexeme())));
    } else {
      curr_frame.instructions.push_back(VMInstr::GETF(s.lvalue[s.lvalue.size() - 1].var_name.lexeme()));
    }
    s.lvalue[s.lvalue.size() - 1].array_expr->accept(*this);
    s.expr.accept(*this);
    curr_frame.instructions.push_back(VMInstr::SETI());
  } else if (s.lvalue.size() > 1) { //last field could be a simple variable
    s.expr.accept(*this);
    curr_frame.instructions.push_back(VMInstr::SETF(s.lvalue[s.lvalue.size() - 1].var_name.lexeme()));
  } else {
    s.expr.accept(*this);
    curr_frame.instructions.push_back(VMInstr::STORE(this->var_table.get(s.lvalue[s.lvalue.size() - 1].var_name.lexeme())));
  }
}


void CodeGenerator::visit(CallExpr& e)
{
  if (BUILT_INS.contains(e.fun_name.lexeme()))
  {

    for (auto& arg : e.args)
    {
      arg.accept(*this);
    }

    if (e.fun_name.lexeme() == "print")
    {
      curr_frame.instructions.push_back(VMInstr::WRITE());
    } else if (e.fun_name.lexeme() == "concat") {
      curr_frame.instructions.push_back(VMInstr::CONCAT());
    } else if (e.fun_name.lexeme() == "to_string") {
      curr_frame.instructions.push_back(VMInstr::TOSTR());
    } else if (e.fun_name.lexeme() == "to_int") {
      curr_frame.instructions.push_back(VMInstr::TOINT());
    } else if (e.fun_name.lexeme() == "to_double") {
      curr_frame.instructions.push_back(VMInstr::TODBL());
    } else if (e.fun_name.lexeme() == "length") {
      curr_frame.instructions.push_back(VMInstr::SLEN());
    } else if (e.fun_name.lexeme() == "length@array") {
      curr_frame.instructions.push_back(VMInstr::ALEN());
    } else if (e.fun_name.lexeme() == "get") {
      curr_frame.instructions.push_back(VMInstr::GETC());
    } else if (e.fun_name.lexeme() == "input") {
      curr_frame.instructions.push_back(VMInstr::READ());
    }
    
  } else {

    for (auto& arg : e.args)
    {
      arg.accept(*this);
    }

    curr_frame.instructions.push_back(VMInstr::CALL(e.fun_name.lexeme()));
  }
}


void CodeGenerator::visit(Expr& e)
{
  e.first->accept(*this);

  if (e.negated == true)
  {
    curr_frame.instructions.push_back(VMInstr::NOT());
  }
  
  if (e.op.has_value()) {
    e.rest->accept(*this);
    if(e.op.value().lexeme() == "+") {
      curr_frame.instructions.push_back(VMInstr::ADD());
    } else if (e.op.value().lexeme() == "-") {
      curr_frame.instructions.push_back(VMInstr::SUB());
    } else if (e.op.value().lexeme() == "*") {
      curr_frame.instructions.push_back(VMInstr::MUL());
    } else if (e.op.value().lexeme() == "/") {
      curr_frame.instructions.push_back(VMInstr::DIV());
    } else if (e.op.value().lexeme() == "==") {
      curr_frame.instructions.push_back(VMInstr::CMPEQ());
    } else if (e.op.value().lexeme() == "!=") {
      curr_frame.instructions.push_back(VMInstr::CMPNE());
    } else if (e.op.value().lexeme() == "<") {
      curr_frame.instructions.push_back(VMInstr::CMPLT());
    } else if (e.op.value().lexeme() == "<=") {
      curr_frame.instructions.push_back(VMInstr::CMPLE());
    } else if (e.op.value().lexeme() == ">") {
      curr_frame.instructions.push_back(VMInstr::CMPGT());
    } else if (e.op.value().lexeme() == ">=") {
      curr_frame.instructions.push_back(VMInstr::CMPGE());
    } else if (e.op.value().lexeme() == "and") {
      curr_frame.instructions.push_back(VMInstr::AND());
    } else if (e.op.value().lexeme() == "or") {
      curr_frame.instructions.push_back(VMInstr::OR());
    }
  }
}


void CodeGenerator::visit(SimpleTerm& t)
{
  t.rvalue->accept(*this);
}
 

void CodeGenerator::visit(ComplexTerm& t)
{
  t.expr.accept(*this);
}


void CodeGenerator::visit(SimpleRValue& v)
{
  if (v.value.type() == TokenType :: INT_VAL) 
  { 
    int val = stoi(v.value.lexeme ()); 
    curr_frame.instructions.push_back(VMInstr ::PUSH(val ));
  } else if (v.value.type() == TokenType :: DOUBLE_VAL) 
  { 
    double val = stod(v.value.lexeme ()); 
    curr_frame.instructions.push_back(VMInstr ::PUSH(val ));
  } else if (v.value.type() == TokenType :: NULL_VAL) 
  { 
    curr_frame.instructions.push_back(VMInstr ::PUSH(nullptr ));
  } else if (v.value.type() == TokenType :: BOOL_VAL) 
  { 
    if (v.value.lexeme () == "true") curr_frame.instructions.push_back(VMInstr ::PUSH(true ));
    else curr_frame.instructions.push_back(VMInstr ::PUSH(false ));
  } else if (v.value.type() == TokenType::STRING_VAL) 
  { 
    string s = v.value.lexeme (); replace_all(s, "\\n", "\n"); replace_all(s, "\\t", "\t"); // could do more here curr_frame.instructions.push_back(VMInstr ::PUSH(s));
    curr_frame.instructions.push_back(VMInstr ::PUSH(s));
  } else if (v.value.type() == TokenType::CHAR_VAL) 
  { 
    string s = v.value.lexeme ();
    curr_frame.instructions.push_back(VMInstr ::PUSH(s));
    replace_all(s, "\\n", "\n"); replace_all(s, "\\t", "\t"); // could do more here curr_frame.instructions.push_back(VMInstr ::PUSH(s));
  }
}


void CodeGenerator::visit(NewRValue& v)
{
  if (v.array_expr.has_value())
  {
    v.array_expr->accept(*this);
    curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
    curr_frame.instructions.push_back(VMInstr::ALLOCA());
  } else {
    curr_frame.instructions.push_back(VMInstr::ALLOCS());
    for (VarDef field : struct_defs[v.type.lexeme()].fields)
    {
      curr_frame.instructions.push_back(VMInstr::DUP());
      curr_frame.instructions.push_back(VMInstr::ADDF(field.var_name.lexeme()));
      curr_frame.instructions.push_back(VMInstr::DUP());
      curr_frame.instructions.push_back(VMInstr::PUSH(nullptr));
      curr_frame.instructions.push_back(VMInstr::SETF(field.var_name.lexeme()));
    }
  }
}


void CodeGenerator::visit(VarRValue& v)
{
  curr_frame.instructions.push_back(VMInstr::LOAD(this->var_table.get(v.path[0].var_name.lexeme())));
  //could be an array: x[0], etc.
  if (v.path[0].array_expr.has_value())
  {
    v.path[0].array_expr->accept(*this);
    curr_frame.instructions.push_back(VMInstr::GETI());
  }
  
  for (int i = 1; i < v.path.size(); i++)
  {
    curr_frame.instructions.push_back(VMInstr::GETF(v.path[i].var_name.lexeme()));
    if (v.path[i].array_expr.has_value())
    {
      v.path[i].array_expr->accept(*this);
      curr_frame.instructions.push_back(VMInstr::GETI());
    }
  }
  
}
    

