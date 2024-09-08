//----------------------------------------------------------------------
// FILE: lexer.cpp
// DATE: CPSC 326, Spring 2023
// NAME: Evan Shoemaker
// DESC: Lexical analyzer responsible for parsing tokens from mypl file
//       Grabs tokens character by character and maps them to correct type.
//----------------------------------------------------------------------

#include "lexer.h"
#include <iostream>
#include <vector>

using namespace std;


Lexer::Lexer(istream& input_stream)
  : input {input_stream}, column {0}, line {1}
{}


char Lexer::read()
{
  ++column;
  return input.get();
}


char Lexer::peek()
{
  return input.peek();
}


void Lexer::error(const string& msg, int line, int column) const
{
  throw MyPLException::LexerError(msg + " at line " + to_string(line) +
                                  ", column " + to_string(column));
}

//parses input stream character by character 
//until a valid token is found,
//then returns that token
Token Lexer::next_token()
{ 
  char ch = read();
  if (isspace(ch) || ch == '#') {
    while (isspace(ch) || ch == '#')
    {
      if (ch == '#') {
        while (ch != '\n') {
          ch = read();
          if (ch == EOF) break;
        }
      }

      if (ch == '\n') {
      ++line;
      column = 0;
      }

      if (ch == EOF) break;
      ch = read();
    }

    if (ch == EOF)
    {
      return Token(TokenType::EOS, "end-of-stream", line, column);
    }
  }
  if (ch == '(') return Token(TokenType::LPAREN, "(", line, column);
  else if (ch == ')') return Token(TokenType::RPAREN, ")", line, column);
  else if (ch == '{') return Token(TokenType::LBRACE, "{", line, column);
  else if (ch == '}') return Token(TokenType::RBRACE, "}", line, column);
  else if (ch == '[') return Token(TokenType::LBRACKET, "[", line, column);
  else if (ch == ']') return Token(TokenType::RBRACKET, "]", line, column);
  else if (ch == ';') return Token(TokenType::SEMICOLON, ";", line, column);
  else if (ch == '.') return Token(TokenType::DOT, ".", line, column);
  else if (ch == ',') return Token(TokenType::COMMA, ",", line, column);
  else if (ch == '<') {
    if (peek() == '=')
    {
      read();
      return Token(TokenType::LESS_EQ, "<=", line, column);
    } else {
      return Token(TokenType::LESS, "<", line, column);
    }
  }
  else if (ch == '>') {
    if (peek() == '=')
    {
      read();
      return Token(TokenType::GREATER_EQ, ">=", line, column);
    } else {
      return Token(TokenType::GREATER, ">", line, column);
    }
  }
  else if (ch == '!') {
    if (peek() == '=')
    {
      read();
      column--;
      return Token(TokenType::NOT_EQUAL, "!=", line, column);
    } else {
      error("expecting '!=' found '!>'", line, column);
    }
  }
  else if (ch == '=') {
    if (peek() == '=')
    {
      read();
      return Token(TokenType::EQUAL, "==", line, column);
    } else {
      return Token(TokenType::ASSIGN, "=", line, column);
    }
  }
  else if (ch == '+') return Token(TokenType::PLUS, "+", line, column);
  else if (ch == '-') return Token(TokenType::MINUS, "-", line, column);
  else if (ch == '*') return Token(TokenType::TIMES, "*", line, column);
  else if (ch == '/') return Token(TokenType::DIVIDE, "/", line, column);
  else if (isdigit(ch)) {
    string num = {};
    num += ch;
    int count = 0;
    if (ch =='0')
    {
      if (isdigit(peek()))
      {
        error("leading zero in number", line, column);
      }
    }
    
    while (isdigit(peek()) || peek() == '.')
    { 
      ch = read();
      count++;
      if (ch == '.') {
        num += ch;
        if (!isdigit(peek()))
        {
          ch = read();
          error("missing digit in '"+num+"'", line, column);
        }
        while(isdigit(peek())) {
          ch = read();
          num += ch;
          count++;
        }
        return Token(TokenType::DOUBLE_VAL, num, line, column - count);
      }
      num += ch;
    }
    return Token(TokenType::INT_VAL, num, line, column - count);
  }
  else if (isalpha(ch)) {
      string word = {};
      word += ch;
      int num = 0;
      while (isalpha(peek()) || peek() == '_' || (word.find('_') && isdigit(peek())))//until something besides a letter is reached, we have a "word"
      //to compare to reserved words
      {
        ch = read();
        word += ch;
        num++;
      }
      if (word == "and") {
        return Token(TokenType::AND, "and", line, column - num);
      } else if (word == "or") {
        return Token(TokenType::OR, "or", line, column - num);
      } else if (word == "not") {
        return Token(TokenType::NOT, "not", line, column - num);
      } else if (word == "true") {
        return Token(TokenType::BOOL_VAL, "true", line, column - num);
      } else if (word == "false") {
        return Token(TokenType::BOOL_VAL, "false", line, column - num);
      } else if (word == "null") {
        return Token(TokenType::NULL_VAL, "null", line, column - num);
      } else if (word == "int") {
        return Token(TokenType::INT_TYPE, "int", line, column - num);
      } else if (word == "bool") {
        return Token(TokenType::BOOL_TYPE, "bool", line, column - num);
      } else if (word == "double") {
        return Token(TokenType::DOUBLE_TYPE, "double", line, column - num);
      } else if (word == "char") {
        return Token(TokenType::CHAR_TYPE, "char", line, column - num);
      } else if (word == "string") {
        return Token(TokenType::STRING_TYPE, "string", line, column - num);
      } else if (word == "void") {
        return Token(TokenType::VOID_TYPE, "void", line, column - num);
      } else if (word == "struct") {
        return Token(TokenType::STRUCT, "struct", line, column - num);
      } else if (word == "while") {
        return Token(TokenType::WHILE, "while", line, column - num);
      } else if (word == "for") {
        return Token(TokenType::FOR, "for", line, column - num);
      } else if (word == "array") {
        return Token(TokenType::ARRAY, "array", line, column - num);
      } else if (word == "if") {
        return Token(TokenType::IF, "if", line, column - num);
      } else if (word == "elseif") {
        return Token(TokenType::ELSEIF, "elseif", line, column - num);
      } else if (word == "else") {
        return Token(TokenType::ELSE, "else", line, column - num);
      } else if (word == "new") {
        return Token(TokenType::NEW, "new", line, column - num);
      } else if (word == "return") {
        return Token(TokenType::RETURN, "return", line, column - num);
      } else {
        return Token(TokenType::ID, word, line, column - num);
      }

  } else if (ch == '"')
  {
    string str = {};
    if (peek() == '"')
    {
      ch = read();
      return Token(TokenType::STRING_VAL, str, line, column - str.size() - 1);
    }
    
    while (peek() != '"' && peek() != '\n' && peek() != EOF)
    { 
      ch = read();
      str += ch;
    }
    if (peek() == '\n')
      {
        ch = read();
        error("found end-of-line in string", line, column);
      }
    if (peek() == '"')
    {
      ch = read();
      return Token(TokenType::STRING_VAL, str, line, column - str.size() - 1);
    } else {
      ch = read();
      error("found end-of-file in string", line, column);
    }
    ch = read();
    error("found end-of-line in string", line, column);
  } else if (ch == '\'')
  {
    if (peek() == '\'') {
      error("empty character", line, column + 1);
    } else {
      string str;
      ch = read();
      if (ch == '\n')
        {
          error("found end-of-line in character", line, column);
        }
      if (ch == EOF)
      {
        error("found end-of-file in character", line, column);
      }
      
      if (ch == '\\')
      {
        if (peek() == 'n')
        {
          ch = read();
          if (peek() == '\'')
          {
            ch = read();
        
          string str(1, '\\n');
          return Token(TokenType::CHAR_VAL, "\\" + str, line, column - 3);
          } else {
            error("Chars must be one character long.", line, column);
          }
        }else if (isalpha(peek()))
        {
          ch = read();
        
          if (peek() == '\'')
          {
            ch = read();
          string str(1, '\\t');
          return Token(TokenType::CHAR_VAL, "\\" + str, line, column - 3);
          } else {
            error("Chars must be one character long.", line, column);
          }
        }
      }
    
      str += ch;
      ch = read();
      if (peek() == '\'')
        {
        str = ch;
        error("expecting ' found " + str, line, column);
        }
      return Token(TokenType::CHAR_VAL, str, line, column - 2);
    }
  }
  else if (ch == EOF) return Token(TokenType::EOS, "end-of-stream", line, column);
  else  {
    error("unexpected character '?'", line, column);
  }
}