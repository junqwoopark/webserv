#pragma once

#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

#include "Request.hpp"
#include "Response.hpp"

using namespace std;

struct Connection {
  Connection() {}

  Connection(int serverSocket, int clientSocket, int cgiSocket, bool isServer) {
    mServerSocket = serverSocket;
    mClientSocket = clientSocket;
    mCgiSocket = cgiSocket;
    mIsServer = isServer;
  }
  ~Connection() {}

  void readRequest(int kq) { // fd로 넘겨주고, 밖에서 이벤트 등록
    char buffer[1024];
    int bytesRead = ::read(mClientSocket, buffer, 1024);
    mRequest.append(buffer, bytesRead);

    if (mRequest.find("\r\n\r\n") == string::npos)
      return;

    Request request(mRequest);
    request.parse();

    cout << request << endl;

    // router->setRequest(request);
    // Response reponse = router->getResponse(); // kq에 이벤트를 등록해준다?

    struct kevent event;
    memset(&event, 0, sizeof(event));
    EV_SET(&event, mClientSocket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
    kevent(kq, &event, 1, NULL, 0, NULL);

    mResponse = "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: 12\r\n"
                "\r\n"
                "Hello World!";
    // 파일 입출력 신경 안쓰고 바로 writeEvent만 추가해서 쓸 수 있게만 해주면 되는거 아닌가?
  }
  void writeResponse() { ::write(mClientSocket, mResponse.c_str(), mResponse.size()); }

  void close() {
    if (mIsServer) {
      ::close(mServerSocket);
    } else {
      ::close(mClientSocket);
    }
  }

  Connection makeConnection(int kq) {
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    mClientSocket = ::accept(mServerSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);

    // router->setRouter(mServerSocket);

    struct kevent event;
    memset(&event, 0, sizeof(event));
    EV_SET(&event, mClientSocket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    kevent(kq, &event, 1, NULL, 0, NULL);

    return Connection(mServerSocket, mClientSocket, mCgiSocket, false);
  }

  bool mIsServer;
  int mServerSocket;
  int mClientSocket;
  int mCgiSocket;
  string mRequest;
  string mResponse;
  // void *router;
};
