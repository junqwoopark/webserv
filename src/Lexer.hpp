#pragma once

#include <fstream>
#include <iostream>

#include "Token.hpp"

class Lexer {
 public:
  Lexer(const char *beg) : mBeg(beg){};

  Token next() {
    while (std::isspace(peek())) {
      get();
    }
    switch (peek()) {
      case '{':
        return atom(Token::LeftCurly);
      case '}':
        return atom(Token::RightCurly);
      case ';':
        return atom(Token::SemiColon);
      case '\0':
        return Token(Token::End);
    }
    if (is_identifier_char(peek())) {
      return identifier();
    }
    return Token(Token::Unexpected);
  }

 private:
  bool is_identifier_char(char c) {
    return std::isalnum(c) || '/' == c || '_' == c || '-' == c || '.' == c || '*' == c || ':' == c;
  }
  Token identifier() {
    const char *beg = mBeg;
    while (is_identifier_char(peek())) {
      get();
    }
    return Token(Token::Identifier, beg, mBeg);
  }

  Token atom(Token::Kind kind) {
    const char *beg = mBeg;
    get();
    return Token(kind, beg, mBeg);
  }

 private:
  char peek() const { return *mBeg; }  // 보는거
  char get() { return *mBeg++; }       // 보고 넘기는거
  const char *mBeg;
};
