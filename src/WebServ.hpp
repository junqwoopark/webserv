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

enum eFdIdentifier {
  ServerSocketFd,
  ClientSocketFd,
  CgiSocketFd,
};

typedef int FD;

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
    for (map<FD, HttpServer>::iterator it = mHttpServerMap.begin(); it != mHttpServerMap.end(); it++) {
      int fd = it->second.getServerSocketFd();

      mFdMap[fd] = ServerSocketFd;
      EV_SET(&event, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
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

        int fd = eventList[i].ident;
        int filter = eventList[i].filter;
        int fdType = mFdMap[fd];
        switch (filter) {
          case EVFILT_READ: {
            switch (fdType) {
              case ServerSocketFd: {
                struct sockaddr_in clientAddr;
                socklen_t clientAddrLen = sizeof(clientAddr);
                int clientFd = accept(fd, (struct sockaddr *)&clientAddr, &clientAddrLen);
                if (clientFd == -1) {
                  throw runtime_error("accept error");
                }
                cout << "client connected: " << clientFd << endl;
                fcntl(clientFd, F_SETFL, O_NONBLOCK);
                mFdMap[clientFd] = ClientSocketFd;
                EV_SET(&event, clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
                  throw runtime_error("kevent error");
                }
                break;
              }  // 1. read 이벤트가 논블록으로 fd 를 읽을수 있을때 방생하는 것인지 => 소켓 이 평생 논블락으로
                 // 못읽을경우가 생길수도??
                 // 2. buf로 읽어 들인 데이터를 조인해서 들고 있어야함 언제까지? read 한 내용이 0 이 될때까지? rnrn?
              case ClientSocketFd: {
                char buf[1024];  // TODO: 버퍼 크기 고려
                int readLen = read(eventList[i].ident, buf, 1024);
                cout << "readLen: " << readLen << endl;  // 이 부분이 안나옴 ㅠ
                // rnrn 있으면 파싱
                // rnrn 없으면 join 후 continue; 굿!
                // 만약 rnrn을 계속 안보내면?
                if (readLen == -1) {
                  throw runtime_error("read error");
                } else if (readLen == 0) {
                  // EOF 인 경우는 이제 실제 파싱해서 들어가면 되는거고?
                  break;
                }
                EV_SET(&event, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
                if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
                  throw runtime_error("kevent error");
                }
                break;
              }
              case CgiSocketFd: {
                break;
              }
            }
            break;
          }
          case EVFILT_WRITE: {
            switch (fdType) {
              case ClientSocketFd: {
                cout << "write" << endl;
                ifstream file("index.html");
                string str((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

                string response = "HTTP/1.1 200 OK\r\n";
                response += "Content-Type: text/html\r\n";
                response += "Content-Length: " + to_string(str.length()) + "\r\n\r\n";
                response += str;
                write(fd, response.c_str(), response.length());
                close(fd);
                break;
              }
              case CgiSocketFd: {
                /* 파싱해서 CgiSocket으로 보내주면 되고 */
                break;
              }
              default:
                break;
            }
            break;
          }
        }
        if (eventList[i].filter == EVFILT_READ) {
        } else if (eventList[i].filter == EVFILT_WRITE) {
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
  // mHttpServerMap[fd]
  map<FD, HttpServer> mHttpServerMap;
  map<FD, eFdIdentifier> mFdMap;
};
