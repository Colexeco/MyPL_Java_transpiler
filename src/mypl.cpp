//----------------------------------------------------------------------
// FILE: mypl.cpp
// DATE: 1/26/2023
// AUTH: Evan Shoemaker
// DESC: Skeleton for MyPL interpreter with six different modes. Works
// with both file and standard inputs.
//----------------------------------------------------------------------

#include <iostream>
#include <fstream>
#include <lexer.h>
#include <java_lexer.h>
#include <token.h>
#include <mypl_exception.h>
#include <simple_parser.h>
#include <ast_parser.h>
#include <java_ast_parser.h>
#include <print_visitor.h>
#include <semantic_checker.h>
#include <mypl_to_java_transpiler.h>
#include <optional>
#include <code_generator.h>

using namespace std;

void usage(const string& command);
void selector(const string& command, istream* input);
void help_options();

int main(int argc, char* argv[])
{
  istream* input = &cin;
  string response, args[argc];
  char ch1, ch2 = ' ';
  for (int i = 0; i < argc; i++)
  {
    args[i] = string(argv[i]);//convert from array of char* to array of string
  }
  
  switch (argc) {
    case 1: //no mode or file specified, open in "normal" mode for console input
    usage("");
    selector("", input);
    break;

    case 2: //mode specified but no file, open console input 
    //OR file with no mode, open file in "normal" mode
      ch1 = args[1][0];
      ch2 = args[1][1];
      if (ch1 == '-' && ch2 == '-')//checking for "--" to distinguish between mode or file path
      {
        //usage(args[1]);
        selector(args[1], input);
      } else {
        input = new ifstream(args[1]); //file name is only argument
        if (input->fail()) {
          cout << "Unable to open file '" << args[1] << "'" << endl;
          return 1;
        }
        //usage("");
        selector("", input);
      }
    break;

    case 3: //file and mode specified
      input = new ifstream(args[2]); //file name is second argument
      usage(args[1]);
      if (input->fail()) {
        cout << "Unable to open file '" << args[2] << "'" << endl;
        return 1;
      }
      selector(args[1], input);
    break;

    default: //invalid option
      help_options();
    return 1;
  }
  //input->clear();
  if (input != &cin)
  {
    delete input;
  }
  return 1;
}

//prints usage information for "--help flag" when incorrect argument are given
void usage(const string& command) {
  if (command == "--help") {
    cout << "Usage: ./mypl [option] [script-file]" << endl;
    cout << "Options:" << endl;
    cout << "   --help    prints this message" << endl;
    cout << "   --lex   displays token information" << endl;
    cout << "   --parse   checks for syntax errors" << endl;
    cout << "   --print   pretty prints program" << endl;
    cout << "   --check   statically checks program" << endl;
    cout << "   --ir     print intermediate (code) representation" << endl;
  } else if (command == "--lex") {
    cout << "[Lex Mode]" << endl;
  } else if (command == "--parse") {
    cout << "[Parse Mode]" << endl;
  } else if (command == "--print") {
    cout << "[Print Mode]" << endl;
  } else if (command == "--check") {
    cout << "[Check Mode]" << endl;
  } else if (command == "--ir") {
    cout << "[IR Mode]" << endl;
  } else if (command == "") {
    cout << "[Normal Mode]" << endl;
  } else if (command == "--java") {
    // cout << "[Java Mode]" << endl;
  } else {
    help_options();
  }
}

//implements behavior for each mode, accepts command and input stream
void selector(const string& command, istream* input) {
  char ch = ' ';
  Lexer lexer = Lexer(*input);
  JavaLexer jlexer = JavaLexer(*input);
  if (command == "--lex") {//call lexer
    try {
      Token t = lexer.next_token();
      cout << to_string(t) << endl;

      while (t.type() != TokenType::EOS)
      {
        t = lexer.next_token();
        cout << to_string(t) << endl;
      }
    } catch (MyPLException& ex) {
      cerr << ex.what() << endl;
    }
  } else if (command == "--parse") { //grab first two characters
    // There must be no lexer errors before attempting parsing
    // Create lexer and tokenize file before attempting parsing
    try {
      SimpleParser parser(lexer);
      parser.parse();
    } catch (MyPLException& ex) {
      cerr << ex.what() << endl;
    }
  } else if (command == "--print") {
    try {
      ASTParser parser(lexer); 
        Program p = parser.parse(); 
        PrintVisitor v(cout); 
        p.accept(v);
      } catch (MyPLException& ex) { 
        cerr << ex.what() << endl;
      }
    cout << endl;
  } else if (command == "--java") {
    try {
      JavaASTParser parser(jlexer); 
        Program p = parser.parse(); 
        MyPLtoJavaTranspiler j(cout);
        p.accept(j);
      } catch (MyPLException& ex) { 
        cerr << ex.what() << endl;
      }
    cout << endl;
  } else if (command == "--check") {
    try {
      ASTParser parser(lexer); 
      Program p = parser.parse(); 
      SemanticChecker v; 
      p.accept(v);
    } catch (MyPLException& ex) { 
      cerr << ex.what() << endl;
    }
  } else if (command == "--ir") {
    try {
        ASTParser parser(lexer); 
        Program p = parser.parse(); 
        SemanticChecker v; 
        p.accept(v);
        VM vm;
        CodeGenerator g(vm);
        p.accept(g);
        cout << to_string(vm) << endl;
      } catch (MyPLException& ex) { 
        cerr << ex.what() << endl;
      }
  } else if (command == "") {
    try {
        ASTParser parser(lexer); 
        Program p = parser.parse(); 
        SemanticChecker v; 
        p.accept(v);
        VM vm;
        CodeGenerator g(vm);
        p.accept(g);
        vm.run();
      } catch (MyPLException& ex) { 
        cerr << ex.what() << endl;
      }
  }
}

//simple helper function to output help message, avoids repeating code
void help_options() {
  cout << "Usage: ./mypl [option] [script-file]" << endl;
  cout << "Options:" << endl;
  cout << "   --help    prints this message" << endl;
  cout << "   --lex   displays token information" << endl;
  cout << "   --parse   checks for syntax errors" << endl;
  cout << "   --print   pretty prints program" << endl;
  cout << "   --check   statically checks program" << endl;
  cout << "   --ir     print intermediate (code) representation" << endl;
  cout << "   --java     Transpiles program to Java" << endl;

}