#pragma once

#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

#include "EventHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"

using namespace std;

class ClientEventHandler : public EventHandler {
public:
  ClientEventHandler() {}

  ClientEventHandler(int readFd, int writeFd, string buffer = "") {}
  virtual ~ClientEventHandler() {}

  virtual void read(udata *data){
      // 요청들을 mBuffer에 담는다.
      // mBuffer에 \r\n\r\n이 들어오면, Request를 생성한다. -> Uri, Method, Header, Body
      // Request를 파싱한다.
      // Request를 Router에 넘겨준다. -> 각 서버의 실제 경로? 파일이면 (/home/index.html)하고, cgi면
      // (./cgi-bin/test.cgi)하고 -> fileFd 열어줍니다.
      // FileEventHandler를 생성한다.
      // fileEventHandler = FileEventHandler(fildFd, clientSocket);
  };
  virtual void write(){};
  virtual void close(){};

  // ServerClientEventHandler.hpp
  // mReadFd = serverSocket
  // mWriteFd = clientSocket
  // mBuffer = request를 담는 객체
};
