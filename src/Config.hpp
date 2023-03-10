#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>

using namespace std;

struct LocationConfig {
  bool isAutoIndex;
  string httpRedirection;
  string rootPath;
  vector<string> index;
  string fileExtension;
  string fastcgiPass;
};

struct ServerConfig {
  vector<string> serverName;
  string serverHost;
  string defaultErrorPage;
  size_t serverPort;
  size_t maxClientBodySize;
  map<string, LocationConfig> locationConfig;
};

map<string, ServerConfig> httpConfig;  // 이것만 parse()에서 리턴하게 끔
// stirng : server_name
// ServerConfig : ServerConfig
