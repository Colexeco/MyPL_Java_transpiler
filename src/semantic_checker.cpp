//----------------------------------------------------------------------
// FILE: semantic_checker.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Evan Shoemaker
// DESC: Performs Semantic Analysis of MyPL programs
//----------------------------------------------------------------------

#include <unordered_set>
#include "mypl_exception.h"
#include "semantic_checker.h"
#include <iostream>

using namespace std;

// hash table of names of the base data types and built-in functions
const unordered_set<string> BASE_TYPES {"int", "double", "char", "string", "bool"};
const unordered_set<string> BUILT_INS {"print", "input", "to_string",  "to_int",
  "to_double", "length", "get", "concat"};


// helper functions

optional<VarDef> SemanticChecker::get_field(const StructDef& struct_def,
                                            const string& field_name)
{
  for (const VarDef& var_def : struct_def.fields)
    if (var_def.var_name.lexeme() == field_name)
      return var_def;
  return nullopt;
}


void SemanticChecker::error(const string& msg, const Token& token)
{
  string s = msg;
  s += " near line " + to_string(token.line()) + ", ";
  s += "column " + to_string(token.column());
  throw MyPLException::StaticError(s);
}


void SemanticChecker::error(const string& msg)
{
  throw MyPLException::StaticError(msg);
}


// visitor functions


void SemanticChecker::visit(Program& p)
{
  // record each struct def
  for (StructDef& d : p.struct_defs) {
    string name = d.struct_name.lexeme();
    if (struct_defs.contains(name))
      error("multiple definitions of '" + name + "'", d.struct_name);
    struct_defs[name] = d;
  }
  // record each function def (need a main function)
  bool found_main = false;
  for (FunDef& f : p.fun_defs) {
    string name = f.fun_name.lexeme();
    if (BUILT_INS.contains(name))
      error("redefining built-in function '" + name + "'", f.fun_name);
    if (fun_defs.contains(name))
      error("multiple definitions of '" + name + "'", f.fun_name);
    if (name == "main") {
      if (f.return_type.type_name != "void")
        error("main function must have void type", f.fun_name);
      if (f.params.size() != 0)
        error("main function cannot have parameters", f.params[0].var_name);
      found_main = true;
    }
    fun_defs[name] = f;
  }
  if (!found_main)
    error("program missing main function");
  // check each struct
  for (StructDef& d : p.struct_defs)
    d.accept(*this);
  // check each function
  for (FunDef& d : p.fun_defs)
    d.accept(*this);
}


void SemanticChecker::visit(SimpleRValue& v)
{
  if (v.value.type() == TokenType::INT_VAL)
    curr_type = DataType {false, "int"};
  else if (v.value.type() == TokenType::DOUBLE_VAL)
    curr_type = DataType {false, "double"};    
  else if (v.value.type() == TokenType::CHAR_VAL)
    curr_type = DataType {false, "char"};    
  else if (v.value.type() == TokenType::STRING_VAL)
    curr_type = DataType {false, "string"};    
  else if (v.value.type() == TokenType::BOOL_VAL)
    curr_type = DataType {false, "bool"};    
  else if (v.value.type() == TokenType::NULL_VAL)
    curr_type = DataType {false, "void"};    
}

void SemanticChecker::visit(FunDef& f)
{
  DataType return_type = {f.return_type.is_array,f.return_type.type_name}; //store return type for later checking in ReturnStmt
  symbol_table.push_environment(); //track all vars associated with this fun def
  symbol_table.add("return", return_type);

  if (!BASE_TYPES.contains(f.return_type.type_name) && !struct_defs.contains(f.return_type.type_name) && (f.return_type.type_name != "void"))
  //checks that function return type is a base type or defined struct
  {
    error(f.return_type.type_name + " not defined");
  }

  for (const VarDef& param:f.params)
  {
    curr_type = DataType {param.data_type.is_array, param.data_type.type_name};
    if (!BASE_TYPES.contains(param.data_type.type_name) && !struct_defs.contains(param.data_type.type_name))
    //checks that every parameter is a base type or defined struct type to ensure no undfined structs are used
    {
      error(param.data_type.type_name + "not defined");
    }
    if (symbol_table.name_exists_in_curr_env(param.var_name.lexeme()))
    {
      error("multiple definitions of '" + param.var_name.lexeme() + "'", param.var_name);
    } else {
      symbol_table.add(param.var_name.lexeme(), param.data_type);
    }
  }

  for (const shared_ptr<Stmt>& stmt:f.stmts)
  {
    stmt->accept(*this);
  }
  symbol_table.pop_environment();
}


void SemanticChecker::visit(StructDef& s)
{
  unordered_set<string> unique_names = {};
  symbol_table.push_environment(); //enter Struct def environment
  for (const VarDef& field:s.fields)
  {
    curr_type = DataType {field.data_type.is_array, field.data_type.type_name};

    if (!BASE_TYPES.contains(field.data_type.type_name) && !struct_defs.contains(field.data_type.type_name))
    //checks that every field is a base type or defined struct type to ensure no undefined structs are used
    {
      error(field.data_type.type_name + "not defined");
    }
    string name = field.var_name.lexeme();

    if (unique_names.contains(name))
    {
      error("multiple definitions of '" + name + "'", field.var_name);
    } else {
      unique_names.insert(name);
    }
  }
  symbol_table.pop_environment();
}


void SemanticChecker::visit(ReturnStmt& s)
{
  DataType return_type = symbol_table.get("return").value();
  s.expr.accept(*this);

  if (((curr_type.type_name != return_type.type_name) || (curr_type.is_array != return_type.is_array)) && (curr_type.type_name != "void"))
  {
    if (!fun_defs.contains(curr_type.type_name)) {
      error("Mismatch between function return type and type in function return statement", s.expr.first_token());
    }
    
  }
}


void SemanticChecker::visit(WhileStmt& s)
{
  s.condition.accept(*this);

  if (curr_type.type_name != "bool") //conditions must evaluate to boolean
  {
    error("While condition must be a boolean expression", s.condition.first_token());
  }

  symbol_table.push_environment();

  for (const shared_ptr<Stmt>& stmt:s.stmts)
  {
    stmt->accept(*this);
  }
  symbol_table.pop_environment();
}


void SemanticChecker::visit(ForStmt& s)
{
  symbol_table.push_environment();
  s.var_decl.accept(*this);
  s.condition.accept(*this);

  if (curr_type.type_name != "bool") //conditions must evaluate to boolean
  {
    error("For condition must be a boolean expression", s.condition.first_token());
  }

  s.assign_stmt.accept(*this);
  symbol_table.push_environment();

  for (const shared_ptr<Stmt>& stmt:s.stmts)
  {
    stmt->accept(*this);
  }
  symbol_table.pop_environment();
  symbol_table.pop_environment();
}


void SemanticChecker::visit(IfStmt& s)
{
  s.if_part.condition.accept(*this);

  if (curr_type.type_name != "bool") //conditions must evaluate to boolean
  {
    error("If condition must be a boolean expression", s.if_part.condition.first_token());
  }

  if (curr_type.is_array) //ensure one element of boolean array is accessed properly
  {
    error("If condition cannot be an array", s.if_part.condition.first_token());
  }

  symbol_table.push_environment();
  for (const shared_ptr<Stmt>& stmt:s.if_part.stmts)
  {
    stmt->accept(*this);
  }

  symbol_table.pop_environment();
  for (BasicIf elseif:s.else_ifs)
  {
    elseif.condition.accept(*this);

    if (curr_type.type_name != "bool") //conditions must evaluate to boolean
    {
      error("Else if condition must be a boolean expression", elseif.condition.first_token());
    }

    symbol_table.push_environment();

    for (const shared_ptr<Stmt>& stmt:elseif.stmts)
    {
      stmt->accept(*this);
    }
    symbol_table.pop_environment();
  }
  symbol_table.push_environment();
  for (shared_ptr<Stmt>& elsestmt:s.else_stmts)
  {
    elsestmt->accept(*this);
  }
  symbol_table.pop_environment();
}


void SemanticChecker::visit(VarDeclStmt& s)
{
  if (symbol_table.name_exists_in_curr_env(s.var_def.var_name.lexeme())) 
  //ensures there can't be two variables with the same name
  {
    error("'" + s.var_def.var_name.lexeme() + "' already exists");
  }

  curr_type = {s.var_def.data_type.is_array, s.var_def.data_type.type_name};
  symbol_table.add(s.var_def.var_name.lexeme(), curr_type);
  s.expr.accept(*this);

  if ((curr_type.type_name != s.var_def.data_type.type_name) && (curr_type.type_name != "void") && (s.var_def.data_type.type_name != "void"))
  {
    error("Type mismatch between left and right side of VarDeclStatement", s.var_def.first_token());
  }

  if ((curr_type.is_array != s.var_def.data_type.is_array) && (curr_type.type_name != "void"))
  {
    cout << s.var_def.data_type.is_array<< " " << curr_type.is_array << endl;
    cout << s.var_def.data_type.type_name << " " << curr_type.type_name << endl;
    error("Array type mismatch between left and right side of VarDeclStatement", s.expr.first_token());
  }
}


void SemanticChecker::visit(AssignStmt& s)
{
  VarRef ref;
  DataType curr, prev;
  for (int i = 0; i < s.lvalue.size(); i++)
  {
    ref = s.lvalue[i];
    if (i == 0) { //if we're on a field, every previous 
    //VarRef in the path must be a struct
      if (symbol_table.name_exists(s.lvalue[i].var_name.lexeme())) {
        curr = {symbol_table.get(s.lvalue[i].var_name.lexeme()).value().is_array, 
        symbol_table.get(s.lvalue[i].var_name.lexeme()).value().type_name};
      } else {
        error("Variable not defined " + s.lvalue[i].var_name.lexeme(), s.expr.first_token());
      }
    }

    if (i > 0)
    {
      prev = curr;
      if (struct_defs.contains(prev.type_name)) {
        StructDef st = struct_defs[prev.type_name];
        if (get_field(st, s.lvalue[i].var_name.lexeme())) {
          curr = get_field(st, s.lvalue[i].var_name.lexeme()).value().data_type;
        } else {
          error("Field is not part of struct definition", s.expr.first_token());
        }
      } else {
        error("Path expressions must consist of structs until last element", s.expr.first_token());
      }
    }
    if (s.lvalue.back().array_expr.has_value()) { //evaluate array expression
      if (curr.is_array == false) {
        error("Array expression used outside of an array", s.expr.first_token());
      }

      s.lvalue.back().array_expr->accept(*this);

      if (curr_type.type_name != "int") {
        error("Array expressions must evaluate to int", s.expr.first_token());
      }
      curr.is_array = false;
    }
  }   
  curr_type = curr;
  s.expr.accept(*this);
  if ((curr.type_name != curr_type.type_name) && (curr_type.type_name != "null") && (curr.type_name != "null"))
  {
    error("Type mismatch between struct field access and assigned value", s.expr.first_token());
  }
}


void SemanticChecker::visit(CallExpr& e)
{
  if (BUILT_INS.contains(e.fun_name.lexeme())) { 
    if (e.fun_name.lexeme() == "print")
    {
      for (Expr arg:e.args)
      {
        arg.accept(*this);
      }

      if ((curr_type.type_name != "string") && (curr_type.type_name != "bool") 
      && (curr_type.type_name != "int") && (curr_type.type_name != "double")
      && (curr_type.type_name != "char"))
      {
        error("Print() only accepts base types as arguments", e.first_token());
      }

      if (curr_type.is_array)
      {
        error("Print() doesn't accept arrays", e.first_token());
      }

      if (e.args.size() != 1)
      {
        error("Print() only  takes one argument", e.first_token());
      }

    } else if (e.fun_name.lexeme() == "concat") {
      for (Expr arg:e.args)
      {
        arg.accept(*this);
      }

      if (curr_type.type_name != "string")
      {
        error("Concat() only accepts strings as arguments", e.first_token());
      }

      if (e.args.size() != 2)
      {
        error("Concat() only takes two string as arguments", e.first_token());
      }

    } else if (e.fun_name.lexeme() == "to_string") {
      for (Expr arg:e.args)
      {
        arg.accept(*this);
      }

      if ((curr_type.type_name == "bool") || (curr_type.type_name == "void"))
      {
        error("to_string() only accepts int, double, and char types as arguments", e.first_token());
      }

      if (e.args.size() != 1)
      {
        error("to_string() only  takes one argument", e.first_token());
      }

      curr_type = {false, "string"};
    } else if (e.fun_name.lexeme() == "to_int") {
      for (Expr arg:e.args)
      {
        arg.accept(*this);
        if ((curr_type.type_name == "int") || (curr_type.type_name == "bool"))
        {
          error("Cannot convert " + curr_type.type_name + " to int", e.first_token());
        }
      }

      if (e.args.size() != 1)
      {
        error("to_int() only takes one argument", e.first_token());
      }

      curr_type = {false, "int"};
    } else if (e.fun_name.lexeme() == "to_double") {
      for (Expr arg:e.args)
      {
        arg.accept(*this);
      }

      if ((curr_type.type_name == "double") || (curr_type.type_name == "bool"))
      {
        error("Cannot convert " + curr_type.type_name + " to double", e.first_token());
      }

      if (e.args.size() != 1)
      {
        error("to_double() only  takes one argument", e.first_token());
      }

      curr_type = {false, "double"};
    } else if (e.fun_name.lexeme() == "input") {
      for (Expr arg:e.args)
      {
        arg.accept(*this);
      }
      curr_type = {false, "string"};
    } else if (e.fun_name.lexeme() == "get") {

      for (int i = 0; i < e.args.size(); i++)
      {
        e.args[i].accept(*this);

        if (i == 0)
        {
          if (curr_type.type_name != "int")
          {
            error("get() takes an int for string index as the first argument", e.first_token());
          }
        }
      }

      if (e.args.size() != 2)
      {
        error("Wrong number of arguments", e.first_token());
      }

      curr_type = {false, "char"};
    } else if (e.fun_name.lexeme() == "length") {
      for (Expr arg:e.args)
      {
        arg.accept(*this);

        if (curr_type.is_array == false)
        {
          if ((curr_type.type_name == "double") || (curr_type.type_name == "bool") 
          || (curr_type.type_name == "int") || (curr_type.type_name == "char"))
          {
            error("Invalid argument type " + curr_type.type_name, e.first_token());
          }
        }

        if (e.args.size() != 1)
        {
          error("length() only  takes one argument", e.first_token());
        }
      }
      curr_type = {false, "int"};
    }
  } else if (fun_defs.contains(e.fun_name.lexeme())) { 
  // checking for function use before definition

    for (Expr arg:e.args)
    {
      arg.accept(*this);
    }
    if (e.args.size() != fun_defs[e.fun_name.lexeme()].params.size())
    {
      error("Wrong number of arguments in function call", e.first_token());
    }

  } else {
    error("Function " + e.fun_name.lexeme() + " used before definition.", e.first_token());
  }
}


void SemanticChecker::visit(Expr& e)
{
  e.first->accept(*this);
  DataType lhs_type = curr_type;
  if (e.op.has_value())
  {
    e.rest->accept(*this);
    DataType rhs_type = curr_type;
    if ((e.op.value().lexeme() == "+") || (e.op.value().lexeme() == "*") ||
    (e.op.value().lexeme() == "-") || (e.op.value().lexeme() == "/"))
    //arithmetic operations mean type must be int or double
    {

      if ((lhs_type.type_name == "int") && (rhs_type.type_name == "int"))
      {
        if (rhs_type.type_name == "int")
        {
          if (rhs_type.is_array)
          {
            curr_type = {true, "int"};
          } else {
            curr_type = {false, "int"};
          }
        } else {
          error("Both operands must be of same type (int)", e.first_token());
        }

      } else if ((lhs_type.type_name == "double") && (rhs_type.type_name == "double")) {

        if (rhs_type.type_name == "double")
        {
          if (rhs_type.is_array)
          {
            curr_type = {true, "double"};
          } else {
            curr_type = {false, "double"};
          }
        } else {
          error("Both operands must be of same type (double)");
        }
      } else {
        error("Arithmetic expressions must consist of integers and/or doubles", e.first_token());
      }

    } else if ((e.op.value().lexeme() == "==") || (e.op.value().lexeme() == "!=")) 
    //operands must be of the same type or void
    {
      if (lhs_type.type_name == rhs_type.type_name)
      {
        if (rhs_type.is_array)
        {
          curr_type = {true, "bool"};
        } else {
          curr_type = {false, "bool"};
        }

      } else if ((lhs_type.type_name == "void") || (rhs_type.type_name == "void")) {
        if (lhs_type.type_name == "void")
        {
          if (rhs_type.is_array)
          {
            curr_type = {true, "bool"};
          } else {
            curr_type = {false, "bool"};
          }
        } else {
          if (rhs_type.is_array)
          {
            curr_type = {true, "bool"};
          } else {
            curr_type = {false, "bool"};
          }
        }
      } else {
        error("Both operands must be of same type or void", e.first_token());
      }

    } else if ((e.op.value().lexeme() == ">") || (e.op.value().lexeme() == "<") || 
    (e.op.value().lexeme() == "<=") || (e.op.value().lexeme() == ">="))
    //operands must be int, double, bool, char, or string
    {
      if (((lhs_type.type_name == "int") && (rhs_type.type_name == "int")) || 
      ((lhs_type.type_name == "double") && (rhs_type.type_name == "double"))
      || ((lhs_type.type_name == "char") && (rhs_type.type_name == "char")) ||
      ((lhs_type.type_name == "string") && (rhs_type.type_name == "string")))
      {
        curr_type = {false, "bool"};
      } else {
        error("Operands must be matching type and must be int, double, bool, char, or string.", e.first_token());
      }
      
    } else if ((e.op.value().lexeme() == "and") || (e.op.value().lexeme() == "or") || (e.op.value().lexeme() == "not")) {
      if (lhs_type.type_name == "bool")
      {
        curr_type = {false, "bool"};
      } else {
        error("Operands must be bool.", e.first_token());
      }
    }
  }
}


void SemanticChecker::visit(SimpleTerm& t)
{
  t.rvalue->accept(*this);
} 


void SemanticChecker::visit(ComplexTerm& t)
{
  t.expr.accept(*this);
}


void SemanticChecker::visit(NewRValue& v)
{
  curr_type.type_name = v.type.lexeme();
  //curr_type.is_array = true;
  if (v.array_expr.has_value())
  {
    curr_type.is_array = true;
  } else {
    curr_type.is_array = false;
  }
}


void SemanticChecker::visit(VarRValue& v)
{
  VarRef ref;
  DataType curr, prev;
  for (int i = 0; i < v.path.size(); i++)
  {
    ref = v.path[i];
    if (i == 0) { //if we're on a field, every previous 
    //VarRef in the path must be a struct
      if (symbol_table.name_exists(v.path[i].var_name.lexeme())) {
        curr = {symbol_table.get(v.path[i].var_name.lexeme()).value().is_array, 
        symbol_table.get(v.path[i].var_name.lexeme()).value().type_name};
      } else {
        error("Variable not defined " + v.path[i].var_name.lexeme(), v.first_token());
      }
    }

    if (i > 0)
    {
      prev = curr;
      if (struct_defs.contains(prev.type_name)) {
        StructDef st = struct_defs[prev.type_name];
        if (get_field(st, v.path[i].var_name.lexeme())) {
          curr = get_field(st, v.path[i].var_name.lexeme()).value().data_type;
        } else {
          error("Field is not part of struct definition", v.first_token());
        }
      } else {
        error("Path expressions must consist of structs until last element", v.first_token());
      }
    }
    if (v.path.back().array_expr.has_value()) { //evaluate array expression
      if (curr.is_array == false) {
        error("Array expression used outside of an array", v.first_token());
      }

      v.path.back().array_expr->accept(*this);

      if (curr_type.type_name != "int") {
        error("Array expressions must evaluate to int", v.first_token());
      }
      curr.is_array = false;
    }
  }   
  curr_type = curr;
}
