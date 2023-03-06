#pragma once

#include "Config.hpp"
#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class HttpServer {
public:
  HttpServer(const Config &config) : mConfig(config) {
    // socket
    // bind
    // listen
  }
  virtual ~HttpServer();

private:
  Config mConfig;
};
