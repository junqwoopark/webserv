#pragma once

#include "HttpServer.hpp"
#include <cstddef>
#include <string>
#include <vector>

using namespace std;

class WebServ {
public:
  WebServ(const string &configFile) {
    // configFile 파싱 -> vector<Config> config만들고
  }
  virtual ~WebServ() {}

  void run() {
    initHttpServers();
    eventLoop();
  }

private:
  void initHttpServers() {
    for (int i = 0; i < mConfigs.size(); i++)
      mHttpServers.push_back(HttpServer(mConfigs[i]));
  }
  void eventLoop() {
    // kevent
    // accept
    // read
    // write
    // close
  }

private:
  vector<Config> mConfigs;
  vector<HttpServer> mHttpServers;
};
