#include <vector>

#include "Config.hpp"
#include "ConfigParser.hpp"
#include "Lexer.hpp"
#include "Token.hpp"

using namespace std;

int main(void) {
  std::ifstream file("nginx.conf");
  std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  Lexer lexer(str.c_str());
  ConfigParser parser;
  vector<Token> tokens;
  for (Token token = lexer.next(); token.kind() != Token::End; token = lexer.next()) {
    tokens.push_back(token);
  }
  try {
    HttpConfig config = parser.parse(tokens);
    cout << config << endl;
    cout << "OKAY" << endl;
  } catch (...) {
    cout << "ERROR!!!";
  }

  return 0;
}
