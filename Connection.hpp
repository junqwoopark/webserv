#include <string>

using namespace std;

class Connection {
 private:
  int port;
  bool mIsServer;
  int mServerSocket;
  int mClientSocket;
  int mCgiSocket;
  string mRequest;
  string mResponse;
};
