#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

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
