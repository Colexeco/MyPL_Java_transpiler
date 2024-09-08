//----------------------------------------------------------------------
// FILE: MyPL_to_Java_Transpiler_Tests.cpp
// DATE: CPSC 326, Spring 2023
// AUTH: Evan Shoemaker
// DESC: Non-comprehensive set of basics tests for MyPL to Java transpiler
//----------------------------------------------------------------------

#include <gtest/gtest.h>
#include <string>
#include <vector>
#include "mypl_exception.h"
#include "java_lexer.h"
#include "java_ast_parser.h"
#include "mypl_to_java_transpiler.h"
#include <iostream>

using namespace std;

streambuf* stream_buffer;

//------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------

string build_string(initializer_list<string> strs)
{
  string result = "";
  for (string s : strs)
    result += s;
  return result;
}

void change_cout(stringstream& out)
{
  stream_buffer = cout.rdbuf();
  cout.rdbuf(out.rdbuf());
}

void restore_cout()
{
  cout.rdbuf(stream_buffer);
}

//------------------------------------------------------------
// Basic definitions/instantiations
//------------------------------------------------------------

TEST (MyPLtoJavaTranspilerTests, EmptyMain) {
    stringstream in("void main() {}");
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n}",
        "\n}"
    }),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, Semicolon) {
    stringstream in("void main() {int a = 10}");
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  int a = 10;;",
        "\n}",
        "\n}"
    }),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, SimpleStruct) {
    stringstream in(build_string({
    "struct SimpleType {",
    "int v1,",
    "double v2,",
    "string v3",
    "}",
    "void main() {}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\nclass SimpleType {",
        "\n  public int v1;",
        "\n  public double v2;",
        "\n  public String v3;",
        "\n}",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n}",
        "\n}"
    }),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, SimpleVarDecl) {
    stringstream in(build_string({
    "void main() {",
    "int a  = 10",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  int a = 10;;",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

//------------------------------------------------------------
// Helper function conversions
//------------------------------------------------------------


TEST (MyPLtoJavaTranspilerTests, print) {
    stringstream in(build_string({
    "void main() {",
    "print()",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  System.out.println();",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, toString) {
    stringstream in(build_string({
    "void main() {",
    "to_string(4)",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  toString(4);",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, toInt) {
    stringstream in(build_string({
    "void main() {",
    "to_int(6.7)",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  toInt(6.7);",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, toDouble) {
    stringstream in(build_string({
    "void main() {",
    "to_double(9)",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  toDouble(9);",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, get) {
    stringstream in(build_string({
    "void main() {",
    "get(0, \"ab\")",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  \"ab\".charAt(0);",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, getWithVariable) {
    stringstream in(build_string({
    "void main() {",
    "string s = \"Words!\"",
    "get(0, s)",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  String s = \"Words!\";;",
        "\n  s.charAt(0);",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, concat) {
    stringstream in(build_string({
    "void main() {",
    "concat(\"ab\", \"bc\")",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  \"ab\".concat(\"bc\");",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, input) {
    stringstream in(build_string({
    "void main() {",
    "input()",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  input.nextLine();",
        "\n;;",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}
//------------------------------------------------------------
// Arrays
//------------------------------------------------------------

TEST (MyPLtoJavaTranspilerTests, baseTypeArray) {
    stringstream in(build_string({
    "void main() {",
    "int xs = new int [10]",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  int[] xs = new int[10];;",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, ArrayOfStructs) {
    stringstream in(build_string({
    "struct T {",
    "}",
    "void main() {",
    "T ts = new T [10]",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\nclass T {",
        "\n}",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  T[] ts = new T()[10];;",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

//------------------------------------------------------------
// Functions
//------------------------------------------------------------

TEST (MyPLtoJavaTranspilerTests, simpleSum) {
    stringstream in(build_string({
    "int sum (int op1, int op2) {",
    "return op1 + op2",
    "}",
    "void main() {",
    "int first = 7",
    "int second = 4",
    "int result = sum(first, second)",
    "print(result)",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic int sum(int op1, int op2) {",
        "\n  return op1 + op2;;",
        "\n}",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  int first = 7;;",
        "\n  int second = 4;;",
        "\n  int result = sum(first, second);;",
        "\n  System.out.println(result);",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

//------------------------------------------------------------
// Simple syntax conversions
//------------------------------------------------------------

TEST (MyPLtoJavaTranspilerTests, stringToString) {
    stringstream in(build_string({
    "void main() {",
    "string s = \"hello\"",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  String s = \"hello\";;",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, boolToBoolean) {
    stringstream in(build_string({
    "void main() {",
    "bool b = false",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  boolean b = false;;",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, and) {
    stringstream in(build_string({
    "void main() {",
    "bool result = 1 and 0",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  boolean result = 1 && 0;;",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, or) {
    stringstream in(build_string({
    "void main() {",
    "bool result = 1 or 0",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  boolean result = 1 || 0;;",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

TEST (MyPLtoJavaTranspilerTests, not) {
    stringstream in(build_string({
    "void main() {",
    "bool result = not 1",
    "}"
    }));
    stringstream out;
    change_cout(out);
    MyPLtoJavaTranspiler transpiler(out);
    JavaASTParser(JavaLexer(in)).parse().accept(transpiler);
    EXPECT_EQ(build_string({
        "import java.util.*;",
        "\nimport java.util.Scanner;\n",
        "\nclass Program {",
        "\nScanner input = new Scanner(System.in);",
        "\n\npublic static void main(String[] args) {",
        "\nProgram p = new Program();",
        "\n  boolean result = !(1);;",
        "\n}",
        "\n}"}),out.str());
    restore_cout();
}

//----------------------------------------------------------------------
// main
//----------------------------------------------------------------------

int main(int argc, char** argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}