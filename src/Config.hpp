#pragma once

#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

struct LocationConfig {
  bool isAutoIndex;
  string path;
  string rootPath;
  string returnRedirect;
  vector<string> limitExceptList;
  vector<string> indexList;
  string fileExtension;
  string fastcgiPass;
};

// 출력 연산자 오버로딩
struct ServerConfig {
  vector<string> serverName; // 여러개 받을려고 해놓은 것 같긴 함
  string serverHost;
  string defaultErrorPage;
  size_t serverPort;
  size_t maxClientBodySize;
  vector<LocationConfig> locationConfigList; // 시간..?

};

ostream& operator<<(ostream& os, const ServerConfig& serverConfig) {
    os << "server_name: ";
    for (int i = 0; i < serverConfig.serverName.size(); i++) {
      os << serverConfig.serverName[i] << ' ';
    }
    os << endl;
    os << "server_host: " << serverConfig.serverHost << endl;
    os << "server_port: " << serverConfig.serverPort << endl;
    os << "default_error_page: " << serverConfig.defaultErrorPage << endl;
    os << "max_client_body_size: " << serverConfig.maxClientBodySize << endl;
    os << "location_config_list: " << endl;
    for (int i = 0; i < serverConfig.locationConfigList.size(); i++) {
      os << "  path: " << serverConfig.locationConfigList[i].path << endl;
      os << "  root_path: " << serverConfig.locationConfigList[i].rootPath << endl;
      os << "  return_redirect: " << serverConfig.locationConfigList[i].returnRedirect << endl;
      os << "  limit_except_list: ";
      for (int j = 0; j < serverConfig.locationConfigList[i].limitExceptList.size(); j++) {
        os << serverConfig.locationConfigList[i].limitExceptList[j] << ' ';
      }
      os << endl;
      os << "  index_list: ";
      for (int j = 0; j < serverConfig.locationConfigList[i].indexList.size(); j++) {
        os << serverConfig.locationConfigList[i].indexList[j] << ' ';
      }
      os << endl;
      os << "  file_extension: " << serverConfig.locationConfigList[i].fileExtension << endl;
      os << "  fastcgi_pass: " << serverConfig.locationConfigList[i].fastcgiPass << endl;
      os << "  is_auto_index: " << serverConfig.locationConfigList[i].isAutoIndex << endl;
    }
    return os;
  };

// map<string, ServerConfig> httpConfig;  // 이것만 parse()에서 리턴하게 끔
// stirng : server_name
// ServerConfig : ServerConfig

// vector<ServerConfig> httpConfigList;



// url = kakao.com/hello/world
// naver.com 
// google.com
// kakao.com



class HttpConfig {
  public :
    HttpConfig() {
    }

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

  friend ostream& operator<<(ostream& os, const HttpConfig& httpConfig) {
    for (int i = 0; i < httpConfig.mServerConfigList.size(); i++) {
      os << httpConfig.mServerConfigList[i] << endl;
    }
    return os;
  }
  
  private:
    vector<ServerConfig> mServerConfigList;
    string mKey;
  
};

void HttpConfig::GetHttp() {
}

void HttpConfig::OpenHttpBlock() {
}

void HttpConfig::GetServer() {
  mServerConfigList.push_back(ServerConfig());
}

void HttpConfig::OpenServerBlock() {
}

void HttpConfig::GetServerKey(string &key) {
  mKey = key;
}

void HttpConfig::GetServerValue(string &value) { // 이거 짜다가 호스트 궁금해짐.
  if (mKey == "server_name") {
    mServerConfigList.back().serverName.push_back(value);
  } else if (mKey == "listen") {
    mServerConfigList.back().serverPort = atoi(value.c_str());
  } else if (mKey == "error_page") {
    mServerConfigList.back().defaultErrorPage = value;
  } else if (mKey == "client_max_body_size") {
    mServerConfigList.back().maxClientBodySize = atoi(value.c_str());
  }
}

void HttpConfig::GetLocation() {
}

void HttpConfig::GetLocationPath(string &path) {
  mServerConfigList.back().locationConfigList.push_back(LocationConfig());
  mServerConfigList.back().locationConfigList.back().path = path;
}


void HttpConfig::OpenLocationBlock() {
}

void HttpConfig::GetLocationKey(string &key) {
  mKey = key;
}

void HttpConfig::GetLocationValue(string &value) {
  if (mKey == "root") {
    mServerConfigList.back().locationConfigList.back().rootPath = value;
  } else if (mKey == "index") {
    mServerConfigList.back().locationConfigList.back().indexList.push_back(value);
  } else if (mKey == "autoindex") {
    mServerConfigList.back().locationConfigList.back().isAutoIndex = (value == "on");
  } else if (mKey == "limit_except") {
    mServerConfigList.back().locationConfigList.back().limitExceptList.push_back(value);
  } else if (mKey == "return") {
    mServerConfigList.back().locationConfigList.back().returnRedirect = value;
  } else if (mKey == "fastcgi_pass") {
    mServerConfigList.back().locationConfigList.back().fastcgiPass = value;
  } else if (mKey == "fastcgi_param") {
    mServerConfigList.back().locationConfigList.back().fileExtension = value;
  }
}

void HttpConfig::CloseLocationBlock() {
}

void HttpConfig::CloseServerBlock() {
}

void HttpConfig::CloseHttpBlock() {
}