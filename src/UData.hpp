#pragma once

#include <iostream>

#include "HttpConfig.hpp"

using namespace std;

enum eIoEventState { ConnectClient, ReadClient, ReadFile, ReadCgi, WriteClient, WriteFile, WriteCgi, CloseSocket };

struct UData {
  UData(int readFd, int writeFd, int clientFd, ServerConfig *serverConfig, eIoEventState ioEventState) {
    this->readFd = readFd;
    this->writeFd = writeFd;
    this->clientFd = clientFd;
    this->ioEventState = ioEventState;

    this->serverConfig = serverConfig;
  }

  int readFd;
  int writeFd;
  int clientFd;
  string request;
  string response;
  eIoEventState ioEventState;

  ServerConfig *serverConfig;
};
