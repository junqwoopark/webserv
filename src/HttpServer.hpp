#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Config.hpp"

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
