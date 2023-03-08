#include <string>

class Token {
 public:
  enum Kind {
    Identifier,
    LeftCurly,
    RightCurly,
    SemiColon,
    Unexpected,
    End,
  };

  Token(Kind kind) : mKind(kind) {}
  Token(Kind kind, const char *beg, std::size_t len) : mKind(kind), mLexeme(beg, len) {}
  Token(Kind kind, const char *beg, const char *end) : mKind(kind), mLexeme(beg, std::distance(beg, end)) {}

  Kind kind() const { return mKind; }
  void kind(Kind kind) { mKind = kind; }
  std::string lexeme() const { return mLexeme; }

 private:
  Kind mKind;
  std::string mLexeme;
};
