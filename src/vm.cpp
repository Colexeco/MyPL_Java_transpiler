//----------------------------------------------------------------------
// FILE: vm.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Evan Shoemaker
// DESC: MyPL virtual machine implementation
//----------------------------------------------------------------------

#include <iostream>
#include "vm.h"
#include "mypl_exception.h"


using namespace std;


void VM::error(string msg) const
{
  throw MyPLException::VMError(msg);
}


void VM::error(string msg, const VMFrame& frame) const
{
  int pc = frame.pc - 1;
  VMInstr instr = frame.info.instructions[pc];
  string name = frame.info.function_name;
  msg += " (in " + name + " at " + to_string(pc) + ": " +
    to_string(instr) + ")";
  throw MyPLException::VMError(msg);
}


string to_string(const VM& vm)
{
  string s = "";
  for (const auto& entry : vm.frame_info) {
    const string& name = entry.first;
    s += "\nFrame '" + name + "'\n";
    const VMFrameInfo& frame = entry.second;
    for (int i = 0; i < frame.instructions.size(); ++i) {
      VMInstr instr = frame.instructions[i];
      s += "  " + to_string(i) + ": " + to_string(instr) + "\n"; 
    }
  }
  return s;
}


void VM::add(const VMFrameInfo& frame)
{
  frame_info[frame.function_name] = frame;
}


void VM::run(bool DEBUG)
{
  // grab the "main" frame if it exists
  if (!frame_info.contains("main"))
    error("No 'main' function");
  shared_ptr<VMFrame> frame = make_shared<VMFrame>();
  frame->info = frame_info["main"];
  call_stack.push(frame);

  // run loop (keep going until we run out of instructions)
  while (!call_stack.empty() and frame->pc < frame->info.instructions.size()) {

    // get the next instruction
    VMInstr& instr = frame->info.instructions[frame->pc];

    // increment the program counter
    ++frame->pc;

    // for debugging
    if (DEBUG) {
      // TODO
      cerr << endl << endl;
      cerr << "\t FRAME.........: " << frame->info.function_name << endl;
      cerr << "\t PC............: " << (frame->pc - 1) << endl;
      cerr << "\t INSTR.........: " << to_string(instr) << endl;
      cerr << "\t NEXT OPERAND..: ";
      if (!frame->operand_stack.empty())
        cerr << to_string(frame->operand_stack.top()) << endl;
      else
        cerr << "empty" << endl;
      cerr << "\t NEXT FUNCTION.: ";
      if (!call_stack.empty())
        cerr << call_stack.top()->info.function_name << endl;
      else
        cerr << "empty" << endl;
    }

    //----------------------------------------------------------------------
    // Literals and Variables
    //----------------------------------------------------------------------

    if (instr.opcode() == OpCode::PUSH) {
      frame->operand_stack.push(instr.operand().value());
    }


    else if (instr.opcode() == OpCode::POP) {
      frame->operand_stack.pop();
    }


    else if (instr.opcode() == OpCode::LOAD)
    {
      VMValue val = instr.operand().value();
      ensure_not_null(*frame, val);
      if (holds_alternative<int>(val)){
        int index = get<int>(val);

        VMValue x = frame->variables.at(index); //grab from memory
        frame->operand_stack.push(x);
      } else {
        error("LOAD only accepts integers for memory addresses");
      }
    }

    else if (instr.opcode() == OpCode::STORE)
    {
      VMValue x = frame->operand_stack.top(); //grab from stack
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      
      if (holds_alternative<int>(instr.operand().value())) {
        int index = get<int>(instr.operand().value());

        if (index == frame->variables.size())
        {
          frame->variables.push_back(x);
        } else {
          frame->variables[index] = x; //add to memory
        }

      } else {
        error("STORE only accepts integers for memory addresses");
      }
    }
    
    //----------------------------------------------------------------------
    // Operations
    //----------------------------------------------------------------------

    else if (instr.opcode() == OpCode::ADD) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(add(y, x));
    }

    
    else if (instr.opcode() == OpCode::SUB) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(sub(y, x));
    }


    else if (instr.opcode() == OpCode::MUL) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(mul(y, x));
    }


    else if (instr.opcode() == OpCode::DIV) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(div(y, x));
    }


    else if (instr.opcode() == OpCode::AND) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      if (holds_alternative<bool>(x) && holds_alternative<bool>(y)) {
        frame->operand_stack.push(get<bool>(y) && get<bool>(x));
      }
    }


    else if (instr.opcode() == OpCode::OR) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      if (holds_alternative<bool>(x) && holds_alternative<bool>(y)) {
        frame->operand_stack.push(get<bool>(y) || get<bool>(x));
      }
    }


    else if (instr.opcode() == OpCode::NOT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      if (holds_alternative<bool>(x)) {
        frame->operand_stack.push(!get<bool>(x));
      }
    }


    else if (instr.opcode() == OpCode::CMPLT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(lt(y, x));
    }


    else if (instr.opcode() == OpCode::CMPLE) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(le(y, x));
    }


    else if (instr.opcode() == OpCode::CMPGT) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(gt(y, x));
    }


    else if (instr.opcode() == OpCode::CMPGE) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      frame->operand_stack.push(ge(y, x));
    }


    else if (instr.opcode() == OpCode::CMPEQ) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(eq(y, x));
    }


    else if (instr.opcode() == OpCode::CMPNE) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(!get<bool>(eq(y, x)));
    }
    //----------------------------------------------------------------------
    // Branching
    //----------------------------------------------------------------------

    
    else if (instr.opcode() == OpCode::JMP) {
      if (holds_alternative<int>(instr.operand().value())) {
        int instruction_number = get<int>(instr.operand().value());
          frame->pc = instruction_number; //jump to next instruction
      } else {
        error("JMPF only accepts integers for instruction numbers to jump to");
      }
    }

    else if (instr.opcode() == OpCode::JMPF) {
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();

      int instruction_number = get<int>(instr.operand().value());
      
      if (holds_alternative<bool>(x))
      {
        if (get<bool>(x) == false)
        {
          frame->pc = instruction_number; //jump to next instruction
        }
      } else {
        error("Last element on stack must be bool type when calling JMPF");
      }
    }

    //----------------------------------------------------------------------
    // Functions
    //----------------------------------------------------------------------


    else if (instr.opcode() == OpCode::CALL) {
      //get name
      string name = get<string>(instr.operand().value());
      //instantiate new frame and set frame info
      shared_ptr<VMFrame> new_frame = make_shared<VMFrame>();
      new_frame->info = frame_info[name];
      //push new frame on to call stack
      call_stack.push(new_frame);
      //copy number of arguments into stack
      for (int i = 0; i < new_frame->info.arg_count; i++)
      {
        VMValue x = frame->operand_stack.top();
        new_frame->operand_stack.push(x);
        frame->operand_stack.pop();
      }
      //set new frame to the current frame
      frame = new_frame;
    }

    else if (instr.opcode() == OpCode::RET) {
      //grab return value
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      //pop frame
      call_stack.pop();
      if (!call_stack.empty())
      {
        frame = call_stack.top();
        //if frame exists, push return value
        frame->operand_stack.push(x);
      }
    }

    //----------------------------------------------------------------------
    // Built in functions
    //----------------------------------------------------------------------


    else if (instr.opcode() == OpCode::WRITE) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      cout << to_string(x);
    }

    else if (instr.opcode() == OpCode::READ) {
      string val = "";
      getline(cin, val);
      frame->operand_stack.push(val);
    }

    
    else if (instr.opcode() == OpCode::SLEN) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, x);
      int size = get<string>(x).size();
      frame->operand_stack.push(size);
    }

    else if (instr.opcode() == OpCode::ALEN) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, x);
      int size = array_heap[get<int>(x)].size();
      frame->operand_stack.push(size);
    }

    else if (instr.opcode() == OpCode::GETC) {
      //pop x
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, x);
      //pop y
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, y);
      //push x[y]
      if (get<int>(y) >= get<string>(x).size())
      {
        error("out-of-bounds string index", *frame);
      } else if (get<int>(y) < 0)
      {
        error("out-of-bounds string index", *frame);
      } else {
        string s = "";
        s += get<string>(x).at(get<int>(y));
        frame->operand_stack.push(s);
      }
    }

    else if (instr.opcode() == OpCode::TOINT) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, x);
      if (holds_alternative<double>(x))
      {
        frame->operand_stack.push(int(get<double>(x)));
      }
      if (holds_alternative<string>(x))
      {
        try {
          stoi(get<string>(x));
        } catch (exception& err) {
          error("cannot convert string to int", *frame);
        }
        static_cast<int>(stoi(get<string>(x)));
        frame->operand_stack.push(stoi(get<string>(x)));
      }
    }

    else if (instr.opcode() == OpCode::TODBL) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, x);
      if (holds_alternative<int>(x))
      {
        frame->operand_stack.push(double(get<int>(x)));
      }
      if (holds_alternative<string>(x))
      {
        try {
          stod(get<string>(x));
        } catch (exception& err) {
          error("cannot convert string to double", *frame);
        }
        frame->operand_stack.push(stod(get<string>(x)));
      }
    }

    else if (instr.opcode() == OpCode::TOSTR) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, x);
      frame->operand_stack.push(to_string(x));
    }

    else if (instr.opcode() == OpCode::CONCAT) {
      //grab x
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, x);
      //grab y
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, y);
      frame->operand_stack.push(to_string(y) + to_string(x));
    }

    //----------------------------------------------------------------------
    // heap
    //----------------------------------------------------------------------


    else if (instr.opcode() == OpCode::ALLOCS) {
      struct_heap[next_obj_id] = {};
      frame->operand_stack.push(next_obj_id);
      ++next_obj_id;
    }

    else if (instr.opcode() == OpCode::ALLOCA) {
     VMValue val = frame->operand_stack.top();
     frame->operand_stack.pop();
     int size = get<int>(frame->operand_stack.top());
     frame->operand_stack.pop();
     array_heap[next_obj_id] = vector<VMValue>(size, val);
     frame->operand_stack.push(next_obj_id);
     ++next_obj_id;
    }
    
    else if (instr.opcode() == OpCode::ADDF) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, x);
      struct_heap[get<int>(x)][get<string>(instr.operand().value())] = nullptr;
    }

    else if (instr.opcode() == OpCode::SETF) {
      //grab x
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      //ensure_not_null(*frame, x);
      //grab y
      VMValue y = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, y);
      //obj(y).f = x
      struct_heap[get<int>(y)][get<string>(instr.operand().value())] = x;
    }

    else if (instr.opcode() == OpCode::GETF) {
      //grab x
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      ensure_not_null(*frame, x);
      //push obj(y).f
      frame->operand_stack.push(struct_heap[get<int>(x)][get<string>(instr.operand().value())]);
    }

    else if (instr.opcode() == OpCode::SETI) {
      //grab x
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      //grab y
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      //grab z
      VMValue z = frame->operand_stack.top();
      ensure_not_null(*frame, z);
      frame->operand_stack.pop();
      //obj(z)[y] = x
      if (((get<int>(y)) >= array_heap[get<int>(z)].size()) || (get<int>(y) < 0))
      {
        error("out-of-bounds array index", *frame);
      } else {
        array_heap[get<int>(z)][get<int>(y)] = x;
      }
    }

    else if (instr.opcode() == OpCode::GETI) {
      //grab x
      VMValue x = frame->operand_stack.top();
      ensure_not_null(*frame, x);
      frame->operand_stack.pop();
      //grab y
      VMValue y = frame->operand_stack.top();
      ensure_not_null(*frame, y);
      frame->operand_stack.pop();
      //obj(y)[x]
      if ((get<int>(x) >= array_heap[get<int>(y)].size()) || (get<int>(x) < 0))
      {
        error("out-of-bounds array index", *frame);
      } else {
        frame->operand_stack.push(array_heap[get<int>(y)][get<int>(x)]);
      }
    }
    //----------------------------------------------------------------------
    // special
    //----------------------------------------------------------------------

    
    else if (instr.opcode() == OpCode::DUP) {
      VMValue x = frame->operand_stack.top();
      frame->operand_stack.pop();
      frame->operand_stack.push(x);
      frame->operand_stack.push(x);      
    }

    else if (instr.opcode() == OpCode::NOP) {
      // do nothing
    }
    
    else {
      error("unsupported operation " + to_string(instr));
    }
  }
}


void VM::ensure_not_null(const VMFrame& f, const VMValue& x) const
{
  if (holds_alternative<nullptr_t>(x))
    error("null reference", f);
}


VMValue VM::add(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) + get<int>(y);
  else
    return get<double>(x) + get<double>(y);
}


VMValue VM::sub(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) - get<int>(y);
  else
    return get<double>(x) - get<double>(y);
}

VMValue VM::mul(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) * get<int>(y);
  else
    return get<double>(x) * get<double>(y);
}

VMValue VM::div(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<int>(x)) 
    return get<int>(x) / get<int>(y);
  else
    return get<double>(x) / get<double>(y);
}


VMValue VM::eq(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) == get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) == get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) == get<string>(y);
  else
    return get<bool>(x) == get<bool>(y);
}

// TODO: Finish the rest of the comparison operators

VMValue VM::lt(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) < get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) < get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) < get<string>(y);
  else
    return get<bool>(x) < get<bool>(y);
}

VMValue VM::le(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) <= get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) <= get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) <= get<string>(y);
  else
    return get<bool>(x) <= get<bool>(y);
}

VMValue VM::gt(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) > get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) > get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) > get<string>(y);
  else
    return get<bool>(x) > get<bool>(y);
}

VMValue VM::ge(const VMValue& x, const VMValue& y) const
{
  if (holds_alternative<nullptr_t>(x) and not holds_alternative<nullptr_t>(y)) 
    return false;
  else if (not holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return false;
  else if (holds_alternative<nullptr_t>(x) and holds_alternative<nullptr_t>(y))
    return true;
  else if (holds_alternative<int>(x)) 
    return get<int>(x) >= get<int>(y);
  else if (holds_alternative<double>(x))
    return get<double>(x) >= get<double>(y);
  else if (holds_alternative<string>(x))
    return get<string>(x) >= get<string>(y);
  else
    return get<bool>(x) >= get<bool>(y);
}

