#pragma once

#include <netinet/in.h>
#include <sys/event.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "HttpConfig.hpp"

using namespace std;

class HttpServer {
 public:
  HttpServer() {}
  HttpServer(ServerConfig &serverConfig) : mServerConfig(serverConfig) {
    mServerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (mServerSocketFd == -1) {
      throw runtime_error("socket error");
    }

    fcntl(mServerSocketFd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(serverConfig.serverPort);

    if (bind(mServerSocketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
      throw runtime_error("bind error");
    }

    if (listen(mServerSocketFd, SOMAXCONN) == -1) {
      throw runtime_error("listen error");
    }
  }

  int getServerSocketFd() { return mServerSocketFd; }

  ~HttpServer() {}

 private:
  ServerConfig mServerConfig;
  int mServerSocketFd;
};
