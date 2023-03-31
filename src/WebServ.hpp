#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "HttpConfig.hpp"
#include "HttpServer.hpp"

typedef int SERVER_SOCKET_FD;

using namespace std;

class WebServ {
 public:
  WebServ(HttpConfig &httpConfig) { initHttpServers(httpConfig); }
  virtual ~WebServ() {}

  void run() {
    int kq = kqueue();
    if (kq == -1) {
      throw runtime_error("kqueue error");
    }

    struct kevent event;

    // kqueue에 등록
    for (map<SERVER_SOCKET_FD, HttpServer>::iterator it = mHttpServerMap.begin(); it != mHttpServerMap.end(); it++) {
      EV_SET(&event, it->second.getServerSocketFd(), EVFILT_READ, EV_ADD, 0, 0, NULL);
      if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
        throw runtime_error("kevent error");
      }
    }

    while (42) {
      struct kevent eventList[10];
      int eventCount = kevent(kq, NULL, 0, eventList, 10, NULL);
      if (eventCount == -1) {
        throw runtime_error("kevent error");
      }

      for (int i = 0; i < eventCount; i++) {
        if (eventList[i].flags & EV_ERROR) {
          throw runtime_error("kevent error");
        }

        /* TODO: 내부 논리 수정 필요! */
        if (eventList[i].filter == EVFILT_READ) {
          if (eventList[i].ident == mHttpServerMap.begin()->second.getServerSocketFd() /* 수정 해야함 */) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int clientFd = accept(mHttpServerMap.begin()->second.getServerSocketFd(), (struct sockaddr *)&clientAddr,
                                  &clientAddrLen);
            if (clientFd == -1) {
              throw runtime_error("accept error");
            }

            cout << "client connected: " << clientFd << endl;

            EV_SET(&event, clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
            if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
              throw runtime_error("kevent error");
            }
          } else {
            char buf[1024];
            int readLen = read(eventList[i].ident, buf, 1024);
            if (readLen == -1) {
              throw runtime_error("read error");
            }

            cout << buf << endl;

            ifstream file("index.html");
            string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

            string response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " +
                to_string(content.length()) + "\r\n\r\n" + content;
            write(eventList[i].ident, response.c_str(), response.length());
            close(eventList[i].ident);
          }
        }
      }
    }
  }

 private:
  void initHttpServers(HttpConfig &httpConfig) {
    vector<ServerConfig> serverConfigList = httpConfig.getServerConfigList();
    for (int i = 0; i < serverConfigList.size(); i++) {
      HttpServer httpServer(serverConfigList[i]);
      mHttpServerMap[httpServer.getServerSocketFd()] = httpServer;
    }
  }

 private:
  HttpConfig mConfig;
  map<SERVER_SOCKET_FD, HttpServer> mHttpServerMap;
};
