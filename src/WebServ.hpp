#pragma once

#include "Config.hpp"
#include "HttpServer.hpp"
#include <cstddef>
#include <string>
#include <vector>

using namespace std;

class WebServ {
public:
  WebServ(const vector<Config> &configs) { initHttpServers(configs); }
  virtual ~WebServ() {}

  void run() {
    // kevent
    // accept
    // read
    // write
    // close
  }

private:
  void initHttpServers(vector<Config> configs) {
    for (const auto &config : configs) {
      mHttpServers.push_back(HttpServer(config));
    }
  }

private:
  vector<HttpServer> mHttpServers;
};
