#pragma once

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct RouteConfig {
  bool isAutoIndex;
  string httpRedirection;
  string rootPath;
  string index; // default file to answer if the request is a directory
  string fileExtension;
  string fastcgiPass;
};

struct Config {
  string serverName;
  string serverHost;
  size_t serverPort;
  string defaultErrorPage;
  size_t maxClientBodySize;
  unordered_map<string, RouteConfig> routeConfig;
};

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
