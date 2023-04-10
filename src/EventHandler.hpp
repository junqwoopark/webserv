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
      close(udata->serverFd);
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

    int clientFd = accept(udata->serverFd, NULL, NULL);
    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    UData *newUdata = new UData(-1, clientFd, serverConfig, ReadClient);  // serverConfig 상속
    struct kevent readEvent;

    // Set up read event
    EV_SET(&readEvent, clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, newUdata);
    kevent(kq, &readEvent, 1, NULL, 0, NULL);

    setTimer(kq, newUdata);
  }

  void readClientEventHandler(int kq, UData *udata) {
    char buffer[1024];
    int readSize = read(udata->clientFd, buffer, 1024);

    setTimer(kq, udata); /* 타이머 초기화 */

    udata->request.append(buffer, readSize);
    if (!udata->request.isComplete()) return;

    deleteTimer(kq, udata); /* 타이머 삭제 */
    udata->ioEventState = routingRequest(udata);

    // 파일 또는 CGI 이벤트 등록
    struct kevent event;
    if (udata->ioEventState == ReadFile)
      EV_SET(&event, udata->serverFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
    else if (udata->ioEventState == WriteFile || udata->ioEventState == WriteCgi)
      EV_SET(&event, udata->serverFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);
  }

  void readFileEventHandler(int kq, UData *udata) {
    char buffer[1024];
    int readSize = read(udata->serverFd, buffer, 1024);  // NON_BLOCK

    // if (readSize > 0)
    udata->response.append(buffer, readSize);
    cout << udata->response << endl;
    // else {
    udata->ioEventState = WriteClient;

    struct kevent event;
    EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);

    close(udata->serverFd);
    // }
  }

  // readFileEventHandler와 동일
  void readCgiEventHandler(int kq, UData *udata) {
    char buffer[1024];
    int readSize = read(udata->serverFd, buffer, 1024);  // NON_BLOCK

    if (readSize > 0)
      udata->response.append(buffer, readSize);
    else {
      udata->ioEventState = WriteClient;
      struct kevent event;
      EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);
    }
  }

  void writeClientEventHandler(int kq, UData *udata) {
    cout << "writeClientEventHandler" << endl;
    string mResponse =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " +
        to_string(udata->response.size()) + "\r\n\r\n" + udata->response;

    // read, write 하는 애들이 있음.
    // I/O 작업 -> 느림
    // I/O 작업 느리니까 그냥 요청만 해야지
    // NON_BLOCK FD니까 write하면 바로 리턴됨. -> 커널이 write하고 있음. -> 이 결과를 kqueue가 받음.
    write(udata->clientFd, mResponse.c_str(), mResponse.size());
    udata->ioEventState = CloseSocket;
    close(udata->clientFd);  // 닫아버려도 되나?
  }

  // Post 요청일 때
  void writeFileEventHandler(int kq, UData *udata) {
    cout << "writeFileEventHandler" << endl;
    write(udata->serverFd, udata->request.getBody().c_str(), udata->request.getBody().size());
    close(udata->serverFd);

    struct kevent event;
    EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);

    udata->ioEventState = WriteClient;
  }

  // request Line
  // header Line\r\n\r\n
  // body \r\n\r\n
  void writeCgiEventHandler(int kq, UData *udata) {}
  void closeSocketEventHandler(int kq, UData *udata) {
    close(udata->clientFd);
    cout << "closed client: " << udata->clientFd << endl;
    delete udata;
  }

  void deleteFileEventHandler(int kq, UData *udata) {}

 private:
  void setTimer(int kq, UData *udata) {
    struct kevent timerEvent;
    int timer_interval_ms = 5000;
    EV_SET(&timerEvent, udata->clientFd, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, timer_interval_ms, udata);
    kevent(kq, &timerEvent, 1, NULL, 0, NULL);
  }

  void deleteTimer(int kq, UData *udata) {
    struct kevent timerEvent;
    EV_SET(&timerEvent, udata->clientFd, EVFILT_TIMER, EV_DELETE, 0, 0, udata);
    kevent(kq, &timerEvent, 1, NULL, 0, NULL);
  }

  eIoEventState routingRequest(UData *udata) {
    string uri = udata->request.getUri();
    string method = udata->request.getMethod();
    ServerConfig *serverConfig = udata->serverConfig;

    cout << "uri: " << uri << endl;
    cout << "method: " << method << endl;

    vector<LocationConfig> &locationConfig = udata->serverConfig->locationConfigList;

    int i = 0;
    string path;
    for (i = 0; i < locationConfig.size(); i++) {
      if (locationConfig[i].path == uri) {
        path = locationConfig[i].rootPath + locationConfig[i].path;

        // 메소드가 있는지 확인
        vector<std::string> &limitExceptList = locationConfig[i].limitExceptList;
        vector<std::string>::iterator it = find(limitExceptList.begin(), limitExceptList.end(), method);
        if (it == limitExceptList.end()) {
          udata->response = "405 Method Not Allowed";
          cout << "405 Method Not Allowed" << endl;
          return Error;
        }

        break;
      }
    }

    /* cgi 인지 확인 */

    if (method == "GET") {
      udata->serverFd = open(path.c_str(), O_RDONLY);
      fcntl(udata->serverFd, F_SETFL, O_NONBLOCK);

      if (errno == EISDIR && locationConfig[i].isAutoIndex) {
        // autoindex
      } else if (errno == EISDIR) {
        udata->response = "403 Forbidden";
        return Error;
      } else if (errno == ENOENT) {
        udata->response = "404 Not Found";
        return Error;
      }
      return ReadFile;
    } else if (method == "POST") {
      udata->serverFd = open(path.c_str(), O_RDWR | O_CREAT);
      fcntl(udata->serverFd, F_SETFL, O_NONBLOCK);
      if (errno == EISDIR) {
        udata->response = "403 Forbidden";
        cout << "403 Forbidden" << endl;
        return Error;
      }

      return WriteFile;
    } else if (method == "DELETE") {
      return DeleteFile;
    }

    return ReadFile;
  }
};
