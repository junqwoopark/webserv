#pragma once
#include "EventHandler.hpp"

class ServerEventHandler : public EventHandler {

public:
  ServerEventHandler() {}

  void read(udata *udata) {} // 소켓 연결 생성 + kq랑, clientEventHandler에 등록
  void write() { cout << "server write" << endl; }
  void close() {}
};
