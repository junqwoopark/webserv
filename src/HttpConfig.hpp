#pragma once

#include <cctype>
#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Trie.hpp"

using namespace std;

struct LocationConfig {
  bool isAutoIndex;
  string path;
  string rootPath;
  vector<string> returnRedirectList;
  vector<string> limitExceptList;
  vector<string> indexList;
  string fastcgiPass;
};

struct ServerConfig {
  vector<string> serverName;
  string rootPath;
  vector<string> errorPageList;
  size_t serverPort;
  size_t maxClientBodySize;
  vector<LocationConfig> locationConfigList;
  Trie locationConfigTrie;

  ServerConfig() {
    serverName = vector<string>();
    errorPageList = vector<string>();
    serverPort = 0;
    maxClientBodySize = 1 * 1024 * 1024;
    locationConfigList = vector<LocationConfig>();
    locationConfigTrie = Trie();
  }

  void buildLocationConfigTrie() {
    for (size_t i = 0; i < locationConfigList.size(); i++) {
      locationConfigTrie.insert(locationConfigList[i].path, &locationConfigList[i]);
    }
  }
};

ostream &operator<<(ostream &os, const ServerConfig &serverConfig) {
  os << "server_name: ";
  for (size_t i = 0; i < serverConfig.serverName.size(); i++) {
    os << serverConfig.serverName[i] << ' ';
  }
  os << endl;
  os << "server_port: " << serverConfig.serverPort << endl;
  os << "default_error_page: ";
  for (size_t i = 0; i < serverConfig.errorPageList.size(); i++) {
    os << serverConfig.errorPageList[i] << ' ';
  }
  os << endl;
  os << "max_client_body_size: " << serverConfig.maxClientBodySize << endl;
  os << "location_config_list: " << endl;
  for (size_t i = 0; i < serverConfig.locationConfigList.size(); i++) {
    os << "  path: " << serverConfig.locationConfigList[i].path << endl;
    os << "  root_path: " << serverConfig.locationConfigList[i].rootPath << endl;
    os << "  return_redirect: ";
    for (size_t j = 0; j < serverConfig.locationConfigList[i].returnRedirectList.size(); j++) {
      os << serverConfig.locationConfigList[i].returnRedirectList[j] << ' ';
    }
    os << endl;
    os << "  limit_except_list: ";
    for (size_t j = 0; j < serverConfig.locationConfigList[i].limitExceptList.size(); j++) {
      os << serverConfig.locationConfigList[i].limitExceptList[j] << ' ';
    }
    os << endl;
    os << "  index_list: ";
    for (size_t j = 0; j < serverConfig.locationConfigList[i].indexList.size(); j++) {
      os << serverConfig.locationConfigList[i].indexList[j] << ' ';
    }
    os << endl;
    os << "  fastcgi_pass: " << serverConfig.locationConfigList[i].fastcgiPass << endl;
    os << "  is_auto_index: " << serverConfig.locationConfigList[i].isAutoIndex << endl;
  }
  return os;
};

class HttpConfig {
 public:
  HttpConfig() {}

  vector<ServerConfig> &getServerConfigList() { return mServerConfigList; }

  void GetHttp();
  void OpenHttpBlock();
  void GetServer();
  void OpenServerBlock();
  void GetServerKey(string &key);
  void GetServerValue(string &value);
  void GetLocation();
  void GetLocationPath(string &path);
  void OpenLocationBlock();
  void GetLocationKey(string &key);
  void GetLocationValue(string &value);
  void CloseLocationBlock();
  void CloseServerBlock();
  void CloseHttpBlock();

  friend ostream &operator<<(ostream &os, const HttpConfig &httpConfig) {
    for (size_t i = 0; i < httpConfig.mServerConfigList.size(); i++) {
      os << httpConfig.mServerConfigList[i] << endl;
    }
    return os;
  }

 private:
  vector<ServerConfig> mServerConfigList;
  string mKey;
};

void HttpConfig::GetHttp() {}

void HttpConfig::OpenHttpBlock() {}

void HttpConfig::GetServer() { mServerConfigList.push_back(ServerConfig()); }

void HttpConfig::OpenServerBlock() {}

void HttpConfig::GetServerKey(string &key) { mKey = key; }

void HttpConfig::GetServerValue(string &value) {
  if (mKey == "server_name") {
    mServerConfigList.back().serverName.push_back(value);
  } else if (mKey == "listen") {
    mServerConfigList.back().serverPort = atoi(value.c_str());
  } else if (mKey == "root") {
    mServerConfigList.back().rootPath = value;
  } else if (mKey == "error_page") {
    if (value.size() < 3) {
      throw invalid_argument("error_page is not valid");
    }

    for (int i = 0; i < 3; ++i) {
      if (!std::isdigit(value[i])) {
        throw invalid_argument("error_page is not valid");
      }
    }

    if (value[3] != '/') {
      throw invalid_argument("error_page is not valid");
    }
    mServerConfigList.back().errorPageList.push_back(value);
  } else if (mKey == "client_max_body_size") {
    for (size_t i = 0; i < value.length(); i++) {
      if (!isdigit(value[i])) throw invalid_argument("client_max_body_size is not valid");
    }
    mServerConfigList.back().maxClientBodySize = atoi(value.c_str()) * 1024 * 1024;
  }
}

void HttpConfig::GetLocation() {}

void HttpConfig::GetLocationPath(string &path) {
  mServerConfigList.back().locationConfigList.push_back(LocationConfig());
  mServerConfigList.back().locationConfigList.back().path = path;
}

void HttpConfig::OpenLocationBlock() {}

void HttpConfig::GetLocationKey(string &key) { mKey = key; }

void HttpConfig::GetLocationValue(string &value) {
  LocationConfig &locationConfig = mServerConfigList.back().locationConfigList.back();
  if (mKey == "root") {
    locationConfig.rootPath = value;
  } else if (mKey == "index") {
    locationConfig.indexList.push_back(value);
  } else if (mKey == "autoindex") {
    if (!value.compare("on") || !value.compare("off"))
      locationConfig.isAutoIndex = (value == "on");
    else
      throw invalid_argument("autoindex is not valid");
  } else if (mKey == "limit_except") {
    if (!value.compare("GET") || !value.compare("POST") || !value.compare("DELETE"))
      locationConfig.limitExceptList.push_back(value);
    else
      throw invalid_argument("limit_except is not valid");
  } else if (mKey == "return") {
    if (locationConfig.returnRedirectList.size() == 0) {
      for (size_t i = 0; i < value.length(); i++) {
        if (!isdigit(value[i])) throw invalid_argument("return is not valid");
      }
      locationConfig.returnRedirectList.push_back(value);
    } else if (locationConfig.returnRedirectList.size() == 1) {
      locationConfig.returnRedirectList.push_back(value);
    } else {
      throw invalid_argument("return is not valid");
    }
  } else if (mKey == "fastcgi_pass") {
    locationConfig.fastcgiPass = value;
  }
}

void HttpConfig::CloseLocationBlock() {}

void HttpConfig::CloseServerBlock() {}

void HttpConfig::CloseHttpBlock() {}
