#include <__nullptr>
#include <fstream>
#include <iostream>
#include <string_view>
class Token {
 public:
  enum Kind {
    Number,
    Identifier,
    LeftCurly,
    RightCurly,
    SemiColon,
    Unexpected,
    End,
  };

  Token(Kind kind) : mKind(kind) {}
  Token(Kind kind, const char *beg, std::size_t len)
      : mKind(kind), mLexeme(beg, len) {}
  Token(Kind kind, const char *beg, const char *end)
      : mKind(kind), mLexeme(beg, std::distance(beg, end)) {}

  Kind kind() const { return mKind; }
  void kind(Kind kind) { mKind = kind; }
  std::string_view lexeme() const { return mLexeme; }

 private:
  Kind mKind;
  std::string_view mLexeme;  // 각각 스트링 들고 잇음.
};

class Lexer {
 public:
  Lexer(const char *beg) : mBeg(beg){};
  Token next() {
    while (std::isspace(peek())) {
      get();
    }  // 공백 무시
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
    if (std::isdigit(peek())) {
      return number();
    }
    if (is_identifier_char(peek())) {
      return identifier();
    }
    return Token(Token::Unexpected);
  }

 private:
  bool is_identifier_char(char c) {
    return std::isalnum(c) || '/' == c || '_' == c || '-' == c || '.' == c ||
           '*' == c || ':' == c;
  }
  Token identifier() {
    const char *beg = mBeg;
    while (is_identifier_char(peek())) {
      get();
    }
    return Token(Token::Identifier, beg, mBeg);
  }
  Token number() {
    const char *beg = mBeg;
    while (std::isdigit(peek())) get();
    return Token(Token::Number, beg, mBeg);
  }
  Token atom(Token::Kind kind) {
    const char *beg = mBeg;
    get();
    return Token(kind, beg, mBeg);
  }

  char peek() const { return *mBeg; }  // 보는거
  char get() { return *mBeg++; }       // 보고 넘기는거
  const char *mBeg;
};

int main() {
  std::ifstream file("nginx.conf");
  std::string str((std::istreambuf_iterator<char>(file)),
                  std::istreambuf_iterator<char>());

  Lexer lexer(str.c_str());
  for (auto token = lexer.next(); token.kind() != Token::End;
       token = lexer.next()) {
    switch (token.kind()) {
      case Token::Number:
        std::cout << "Number: " << token.lexeme() << std::endl;
        break;
      case Token::Identifier:
        std::cout << "Identifier: " << token.lexeme() << std::endl;
        break;
      case Token::LeftCurly:
        printf("LeftCurly: {\n");
        break;
      case Token::RightCurly:

        printf("RightCurly: }\n");
        break;
      case Token::SemiColon:
        printf("SemiColon: ;\n");
        break;
      case Token::Unexpected:
        printf("Unexpected\n");
        break;
      case Token::End:
        printf("End\n");
        break;
    }
  }
}
