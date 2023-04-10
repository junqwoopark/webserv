#pragma once

#include <iostream>

#include "HttpConfig.hpp"
#include "Request.hpp"

using namespace std;

enum eIoEventState {
  ConnectClient,
  ReadClient,
  ReadFile,
  ReadCgi,
  WriteClient,
  WriteFile,
  WriteCgi,
  DeleteFile,
  CloseSocket,
  Error
};

struct UData {
  UData(int serverFd, int clientFd, ServerConfig *serverConfig, eIoEventState ioEventState) {
    this->serverFd = serverFd;
    this->clientFd = clientFd;
    this->ioEventState = ioEventState;

    this->serverConfig = serverConfig;
  }

  int serverFd;  // 파일, cgi 를 읽고 쓰는 fd
  int clientFd;  // 클라이언트와 통신하는 fd
  Request request;
  string response;
  eIoEventState ioEventState;

  ServerConfig *serverConfig;
};
