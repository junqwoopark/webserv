#pragma once

#include <fcntl.h>
#include <netinet/in.h>
#include <sys/_types/_timespec.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "EventHandler.hpp"
#include "HttpConfig.hpp"
#include "HttpServer.hpp"
#include "UData.hpp"

typedef int FD;

using namespace std;

class WebServ {  // 역할: kqueue 이벤트를 받아서 각각 요청이 들어온 http 서버로 이벤트를 전달.
 public:
  WebServ(HttpConfig &httpConfig) { initHttpServers(httpConfig); }
  virtual ~WebServ() {
    for (map<FD, HttpServer>::iterator it = mHttpServerMap.begin(); it != mHttpServerMap.end(); it++) {
      close(it->first);
    }
  }

  void run() {
    int kq = kqueue();

    if (kq == -1) {
      throw runtime_error("kqueue error");
    }

    struct kevent event;

    // kqueue에 등록
    for (map<FD, HttpServer>::iterator it = mHttpServerMap.begin(); it != mHttpServerMap.end(); it++) {
      int fd = it->second.getServerSocketFd();
      ServerConfig *serverConfig = &it->second.getServerConfig();
      serverConfig->buildLocationConfigTrie();

      UData *udata = new UData(fd, -1, serverConfig, ConnectClient);  // serverConfig 설정
      EV_SET(&event, fd, EVFILT_READ, EV_ADD, 0, 0, udata);
      if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
        throw runtime_error("kevent error");
      }
      // (serverSocket : 설정)이 매핑되게 해야할 것 같거든?
    }

    EventHandler eventHandler;
    while (42) {
      struct kevent eventList[1024];
      int eventCount = kevent(kq, NULL, 0, eventList, 1024, NULL);

      if (eventCount == -1) {
        throw runtime_error("kevent error");
      }

      for (int i = 0; i < eventCount; i++) {
        if (eventList[i].flags & EV_ERROR) {
          throw runtime_error("kevent error");
        }

        UData *udata = (UData *)eventList[i].udata;
        // cout << "eventList[i].ident: " << eventList[i].ident << endl;
        // cout << "eventList[i].filter: " << eventList[i].filter << endl;
        // cout << "eventList[i].flags: " << eventList[i].flags << endl;
        // cout << "eventList[i].fflags: " << eventList[i].fflags << endl;
        // cout << "eventList[i].data: " << eventList[i].data << endl;
        // cout << "eventList[i].udata: " << eventList[i].udata << endl;
        // cout << "udata->serverFd: " << udata->serverFd << endl;
        eventHandler.handle(kq, eventList[i]);
      }
    }
  }

  void stop() {
    for (map<FD, HttpServer>::iterator it = mHttpServerMap.begin(); it != mHttpServerMap.end(); it++) {
      close(it->first);
    }
  }

 private:
  void initHttpServers(HttpConfig &httpConfig) {
    vector<ServerConfig> &serverConfigList = httpConfig.getServerConfigList();
    for (int i = 0; i < serverConfigList.size(); i++) {
      HttpServer httpServer(serverConfigList[i]);
      mHttpServerMap[httpServer.getServerSocketFd()] = httpServer;
    }
  }

 private:
  HttpConfig mConfig;
  map<FD, HttpServer> mHttpServerMap;
  vector<UData *> udataList;
};
