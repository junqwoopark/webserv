#include "EventHandler.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

// #include "ClientEventHandler.hpp"
// #include "FileEventHandler.hpp"
// #include "ServerEventHandler.hpp"

using namespace std;

typedef void (*handler_function)(int, udata *);

int main() {
  int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in serverAddress;

  memset(&serverAddress, 0, sizeof(serverAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddress.sin_port = htons(9000);

  bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
  listen(serverSocket, 10);

  int kq = kqueue();

  struct kevent event;
  memset(&event, 0, sizeof(event));

  udata *_udata = new udata(serverSocket, serverSocket, ConnectClient);
  EV_SET(&event, serverSocket, EVFILT_READ, EV_ADD, 0, 0, _udata);
  kevent(kq, &event, 1, NULL, 0, NULL);

  std::map<eType, handler_function> handler_map;
  handler_map[ConnectClient] = connectClientEventHandler;
  handler_map[ReadClient] = readClientEventHandler;
  handler_map[ReadFile] = readFileEventHandler;
  handler_map[ReadCgi] = readCgiEventHandler;
  handler_map[WriteClient] = writeClientEventHandler;
  handler_map[WriteFile] = writeFileEventHandler;
  handler_map[WriteCgi] = writeCgiEventHandler;
  handler_map[CloseSocket] = closeSocketEventHandler;

  struct kevent events[10];
  while (true) {
    int nevents = kevent(kq, NULL, 0, events, 10, NULL);
    cout << "nevents: " << nevents << endl;
    for (int i = 0; i < nevents; i++) {
      int fd = events[i].ident;
      struct udata *udata = (struct udata *)events[i].udata;

      handler_map[udata->type](kq, udata);
    }
  }
}
