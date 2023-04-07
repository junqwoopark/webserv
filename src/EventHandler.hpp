#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "HttpConfig.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "UData.hpp"

// void readClientClientEventHandler(udata *udata) -> post 면 writeFile ,cgi 면 writeCgi, file 이면 readFile
// void readFileClientEventHandler(udata *udata) { -> writeClient
// void readCgiClientEventHandler(udata *udata) { -> writeClient
// void writeClientClientEventHandler(udata *udata) { -> close
// void writeFileClientEventHandler(udata *udata) { -> writeClient
// void writeCgiClientEventHandler(udata *udata) { -> readCgi

class EventHandler {
 public:
  EventHandler() {}
  ~EventHandler() {}

  void handle(int kq, UData *udata, struct kevent &event) {
    if (event.filter == EVFILT_TIMER) {
      cout << "timeout" << endl;
      close(udata->readFd);
      close(udata->writeFd);
      close(udata->clientFd);
      delete udata;
      return;
    }

    switch (udata->ioEventState) {
      case ConnectClient:
        connectClientEventHandler(kq, udata);
        break;
      case ReadClient:
        readClientEventHandler(kq, udata);
        break;
      case ReadFile:
        readFileEventHandler(kq, udata);
        break;
      case ReadCgi:
        readCgiEventHandler(kq, udata);
        break;
      case WriteClient:
        writeClientEventHandler(kq, udata);
        break;
      case WriteFile:
        writeFileEventHandler(kq, udata);
        break;
      case WriteCgi:
        writeCgiEventHandler(kq, udata);
        break;
      case CloseSocket:
        closeSocketEventHandler(kq, udata);
        break;
    }
  };

 private:
  void connectClientEventHandler(int kq, UData *udata) {
    cout << "connectClientEventHandler" << endl;
    ServerConfig *serverConfig = udata->serverConfig;

    int clientFd = accept(udata->readFd, NULL, NULL);
    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    UData *newUdata = new UData(clientFd, clientFd, clientFd, serverConfig, ReadClient);  // serverConfig 상속
    struct kevent readEvent;
    struct kevent timerEvent;

    // Set up read event
    EV_SET(&readEvent, clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, newUdata);

    // Set up timer event
    int timer_interval_ms = 5000;  // 5 seconds
    EV_SET(&timerEvent, clientFd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timer_interval_ms, newUdata);

    // Add events to the kqueue
    kevent(kq, &readEvent, 1, NULL, 0, NULL);
    kevent(kq, &timerEvent, 1, NULL, 0, NULL);
  }

  void readClientEventHandler(int kq, UData *udata) {
    cout << "readClientEventHandler" << endl;
    char buffer[8192];
    int readSize = read(udata->readFd, buffer, 1024);

    udata->request.append(buffer, readSize);

    cout << "buffer: " << buffer << endl;
    if (udata->request.find("\r\n\r\n") == string::npos) return;  // 헤더 사이즈 체크..
    // 다음 \r\n\r\n까지 읽어서 bodysize 체크?

    // 수정 필요
    Request request(udata->request);  // httpRequest 유효성검사
    // 만약 유효하지 않은 요청이다! -> Response 생성하고 -> writeClientEventHandler로 보내서 보내주면 됨!

    // cout << *(udata->serverConfig) << endl;

    // serverConfig로 라우팅!

    // 여기서 request를 보고 cgi인지 file인지 알아내야함
    // 다 라우터가 해야할 일...

    // 라우팅 후 new fd 찾아오기 -> 여기서 cgi 면 cgiToClientClientEventHandler, file이면 fileToClientClientEventHandler
    // 해줘야함. if (cgi라면) {
    //   커넥션 만들고, udata->writeFd = cgiFd; udata->ioEventState = ClientToCgi;
    //   struct kevent event;
    //   EV_SET(&event, udata->writeFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    //   kevent(kq, &event, 1, NULL, 0, NULL);
    //} else if (file이라면 && post라면) {
    //   udata->writeFd = fileFd; udata->ioEventState = ClientToFile;
    //   struct kevent event;
    //   EV_SET(&event, udata->writeFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    //   kevent(kq, &event, 1, NULL, 0, NULL);
    //} else if (file이라면 && get이라면) {
    //   udata->readFd = fileFd; udata->ioEventState = ClientToFile;
    //  struct kevent event;
    //  EV_SET(&event, udata->readFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
    //  kevent(kq, &event, 1, NULL, 0, NULL);
    //}

    udata->ioEventState = ReadFile;

    // 이벤트 등록
    struct kevent event;
    EV_SET(&event, udata->writeFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);
  }

  void readFileEventHandler(int kq, UData *udata) {
    char buffer[1024];
    int readSize = read(udata->readFd, buffer, 1024);  // NON_BLOCK

    if (readSize > 0)
      udata->response.append(buffer, readSize);
    else {
      udata->ioEventState = WriteClient;
      struct kevent event;
      EV_SET(&event, udata->writeFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);
    }
  }

  // readFileEventHandler와 동일
  void readCgiEventHandler(int kq, UData *udata) {
    char buffer[1024];
    int readSize = read(udata->readFd, buffer, 1024);  // NON_BLOCK

    if (readSize > 0)
      udata->response.append(buffer, readSize);
    else {
      udata->ioEventState = WriteClient;
      struct kevent event;
      EV_SET(&event, udata->writeFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);
    }
  }

  void writeClientEventHandler(int kq, UData *udata) {
    string mResponse =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 12\r\n"
        "\r\n"
        "Hello World!";

    write(udata->writeFd, mResponse.c_str(), mResponse.size());
    udata->ioEventState = CloseSocket;
  }
  void writeFileEventHandler(int kq, UData *udata) {
    write(udata->writeFd, udata->response.c_str(), udata->response.size());
    udata->ioEventState = CloseSocket;
  }

  // request Line
  // header Line\r\n\r\n
  // body \r\n\r\n
  void writeCgiEventHandler(int kq, UData *udata) {}
  void closeSocketEventHandler(int kq, UData *udata) {
    close(udata->clientFd);
    cout << "closed client: " << udata->writeFd << endl;
    delete udata;
  }

 private:
  unordered_map<string, string> mRouter;
};
