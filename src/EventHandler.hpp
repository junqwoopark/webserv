#pragma once

#include <arpa/inet.h>
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
      case Error:
        errorHandler(kq, udata);
        break;
    }
  };

 private:
  void connectClientEventHandler(int kq, UData *udata) {
    // cout << "connectClientEventHandler" << endl;
    ServerConfig *serverConfig = udata->serverConfig;

    // cout << "serverFd: " << udata->serverFd << endl;

    int clientFd = accept(udata->serverFd, NULL, NULL);
    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    // cout << "clientFd: " << clientFd << endl;
    if (clientFd == -1) {
      cout << "accept error" << endl;
      return;
    }

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
    struct kevent event;
    EV_SET(&event, udata->clientFd, EVFILT_READ, EV_DELETE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);

    udata->ioEventState = routingRequest(udata);

    // 파일 또는 CGI 이벤트 등록
    // struct kevent event;
    if (udata->ioEventState == ReadFile) {
      EV_SET(&event, udata->serverFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);
    } else if (udata->ioEventState == WriteFile || udata->ioEventState == WriteCgi) {
      EV_SET(&event, udata->serverFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);
    } else if (udata->ioEventState == Error || udata->ioEventState == WriteClient) {
      EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);
    }
  }

  void readFileEventHandler(int kq, UData *udata) {
    char buffer[1024];
    int readSize = read(udata->serverFd, buffer, 1024);  // NON_BLOCK

    // if (readSize > 0)
    udata->response.append(buffer, readSize);
    // cout << udata->response << endl;
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
    cout << "readCgiEventHandler" << endl;
    char buffer[1024];
    int readSize = read(udata->serverFd, buffer, 1024);  // NON_BLOCK
    cout << readSize << endl;
    // readSize = 0;

    // if (readSize > 0)
    udata->response.append(buffer, readSize);

    // else {
    udata->ioEventState = WriteClient;
    struct kevent event;
    EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);
    // }

    close(udata->serverFd);
  }

  void writeClientEventHandler(int kq, UData *udata) {
    cout << "writeClientEventHandler" << endl;
    string mResponse =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: " +
        to_string(udata->response.size()) + "\r\n\r\n" + udata->response;

    // cout << mResponse << endl;

    int x = write(udata->clientFd, mResponse.c_str(), mResponse.size());
    if (x < 0) {
      cout << "write error" << endl;
    }

    udata->ioEventState = CloseSocket;
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

  void writeCgiEventHandler(int kq, UData *udata) {
    cout << "writeCgiEventHandler" << endl;
    // get request to nodejs /hello
    string request = "GET /hello HTTP/1.1\r\nHost: localhost:9000\r\nConnection: Keep-Alive\r\n\r\n";

    cout << write(udata->serverFd, request.c_str(), request.size()) << endl;

    struct kevent event;

    // serverFD -> 쓰기 요청 -> 다 쓰이면 write이벤트가 또 발생함.
    EV_SET(&event, udata->serverFd, EVFILT_WRITE, EV_DELETE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);
    EV_SET(&event, udata->serverFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);

    udata->ioEventState = ReadCgi;
  }

  void closeSocketEventHandler(int kq, UData *udata) {
    cout << "closeSocketEventHandler" << endl;
    close(udata->clientFd);
    delete udata;
  }

  void deleteFileEventHandler(int kq, UData *udata) {}

  void errorHandler(int kq, UData *udata) {
    cout << "error handler" << endl;
    string mResponse =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 0\r\n\r\n";

    write(udata->clientFd, mResponse.c_str(), mResponse.size());
    udata->ioEventState = CloseSocket;
  }

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

    // cout << "uri: " << uri << endl;
    // cout << "method: " << method << endl;

    Trie &trie = serverConfig->locationConfigTrie;

    LocationConfig *locationConfig = (LocationConfig *)trie.search(uri.c_str());

    if (locationConfig == NULL) {
      udata->response = "404 Not Found";
      // cout << "404 Not Found" << endl;
      return Error;
    }

    // 메소드가 있는지 확인
    vector<std::string> &limitExceptList = locationConfig->limitExceptList;
    vector<std::string>::iterator it = find(limitExceptList.begin(), limitExceptList.end(), method);
    if (it == limitExceptList.end()) {
      udata->response = "405 Method Not Allowed";
      cout << "405 Method Not Allowed" << endl;
      return Error;
    }

    string path = locationConfig->rootPath + uri;

    // 1. cgi pass 가 있으면  다 걸로 보내고,,,
    if (locationConfig->fastcgiPass != "") {
      udata->serverFd = createCgiSocket(locationConfig->fastcgiPass);
      if (udata->serverFd == -1) {
        udata->response = "500 Internal Server Error";
        return Error;
      }
      return WriteCgi;
    } else if (!locationConfig->returnRedirectList.empty()) {
      // cout << "redirect" << endl;
      udata->response = "302 Found";
      return Error;
    }

    // 2. return 있으면 다 리다이렉트로 보내고,,,
    // 3. 없으면 그냥 보내고,,,

    /* cgi 인지 확인 */

    if (method == "GET") {
      udata->serverFd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
      if (errno == EISDIR && locationConfig->isAutoIndex) {
        // autoindex
      } else if (errno == EISDIR) {
        udata->response = "403 Forbidden";
        errno = 0;
        return Error;
      } else if (errno == ENOENT) {
        udata->response = "404 Not Found";
        errno = 0;
        // cout << "404 Not Found" << endl;
        return Error;
      }
      return ReadFile;
    } else if (method == "POST") {
      udata->serverFd = open(path.c_str(), O_RDWR | O_CREAT | O_NONBLOCK | O_TRUNC, 0644);
      if (errno == EISDIR) {
        udata->response = "403 Forbidden";
        // cout << "403 Forbidden" << endl;
        errno = 0;
        return Error;
      }
      return WriteFile;
    } else if (method == "DELETE") {
      if (unlink(path.c_str()) == -1) {
        udata->response = "404 Not Found";
        cout << "DELETE ERROR" << endl;
        return Error;
      }
      return WriteClient;
    }

    return ReadFile;
  }

  int createCgiSocket(std::string &fastCgiPass) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
      cout << "socket error" << endl;
      return -1;
    }

    cout << "serverFd: " << serverFd << endl;

    // fastCgiPass = 127.0.0.1:9000
    std::string ip = fastCgiPass.substr(0, fastCgiPass.find(":"));
    std::string port = fastCgiPass.substr(fastCgiPass.find(":") + 1, fastCgiPass.size());
    cout << ip << endl;
    cout << port << endl;

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port.c_str()));
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
      cout << "connect error" << endl;
      close(serverFd);
      return -1;
    }
    cout << "connect success" << endl;
    fcntl(serverFd, F_SETFL, O_NONBLOCK);

    return serverFd;
  }
};
