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

// struct Config {
//   string serverName;
//   string serverHost;
//   size_t serverPort;
//   string defaultErrorPage;
//   size_t maxClientBodySize;
//   unordered_map<string, RouteConfig> routeConfig;
// };

class Config {
public:
  Config(const string &configFile) {
  } // config 파일 하나 쓴단 말이야? 어디 까지 읽었는지 그걸 하가 좀 어려운 부분

private:
  string mServerName;
  string mServerHost;
  size_t mServerPort;
  string mDefaultErrorPage;
  size_t mMaxClientBodySize;
  unordered_map<string, RouteConfig> mRouteConfig;
};
