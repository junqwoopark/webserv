#pragma once

#include <iostream>

#include "HttpConfig.hpp"
#include "Request.hpp"
#include "Response.hpp"

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
  Error,
};

struct UData {
  UData(int serverFd, int clientFd, ServerConfig *serverConfig, eIoEventState ioEventState) {
    this->serverFd = serverFd;
    this->clientFd = clientFd;
    this->ioEventState = ioEventState;
    this->cgiPid = 0;
    this->writeOffset = 0;

    this->serverConfig = serverConfig;

    this->request = Request();
    this->response = Response();
  }

  int serverFd;
  int clientFd;
  int cgiFd[2];
  int cgiPid;

  size_t readFileSize;
  size_t writeOffset;

  Request request;
  Response response;
  eIoEventState ioEventState;

  ServerConfig *serverConfig;
};
