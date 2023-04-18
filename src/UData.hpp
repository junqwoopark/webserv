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

    this->serverConfig = serverConfig;

    this->request = Request();
    this->response = Response();
  }

  int serverFd;  // 파일, cgi 를 읽고 쓰는 fd
  int clientFd;  // 클라이언트와 통신하는 fd
  int cgiFd[2];  // cgi와 통신하는 fd
  int cgiPid;

  Request request;
  Response response;
  eIoEventState ioEventState;

  ServerConfig *serverConfig;
};
