#include <vector>

#include "Config.hpp"
#include "ConfigParser.hpp"
#include "Lexer.hpp"
#include "Token.hpp"

using namespace std;

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " [config file]" << endl;
    return 1;
  }
  ConfigParser configParser;
  HttpConfig config = configParser.parse(argv[1]);
  cout << config << endl;
  // Webserv webserv(config);
  // webserv.run();

  // html 에코 서버?

  return 0;
}
