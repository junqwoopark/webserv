#include <vector>

#include "ConfigParser.hpp"
#include "HttpConfig.hpp"
#include "Lexer.hpp"
#include "Token.hpp"
#include "WebServ.hpp"

using namespace std;

int main(int argc, char **argv) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " [config file]" << endl;
    return 1;
  }
  ConfigParser configParser;
  HttpConfig config = configParser.parse(argv[1]);
  cout << config << endl;
  WebServ webserv(config);
  webserv.run();

  return 0;
}
