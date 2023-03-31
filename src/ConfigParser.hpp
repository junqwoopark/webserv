#pragma once

#include <cstddef>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "HttpConfig.hpp"
#include "Lexer.hpp"
#include "Token.hpp"

using namespace std;

class ConfigParser {
 public:
  enum eStatus {
    GetHttp,
    OpenHttpBlock,
    GetServer,
    OpenServerBlock,
    GetServerKey,
    GetServerValue,
    GetLocation,
    GetLocationPath,
    OpenLocationBlock,
    GetLocationKey,
    GetLocationValue,
    CloseLocationBlock,
    CloseServerBlock,
    CloseHttpBlock,
    Finish,
  };

  ConfigParser() : mStatus(GetHttp) {
    // mFuncMap initialization
    mFuncMap[make_pair(GetHttp, Token::Identifier)] = &ConfigParser::GetHttpFunc;
    mFuncMap[make_pair(OpenHttpBlock, Token::LeftCurly)] = &ConfigParser::OpenHttpBlockFunc;
    mFuncMap[make_pair(GetServer, Token::Identifier)] = &ConfigParser::GetServerFunc;
    mFuncMap[make_pair(OpenServerBlock, Token::LeftCurly)] = &ConfigParser::OpenServerBlockFunc;
    mFuncMap[make_pair(GetServerKey, Token::Identifier)] = &ConfigParser::GetServerKeyFunc;
    mFuncMap[make_pair(GetServerValue, Token::SemiColon)] = &ConfigParser::GetServerValueFunc;
    mFuncMap[make_pair(GetServerValue, Token::Identifier)] = &ConfigParser::GetServerValueFunc;
    mFuncMap[make_pair(GetLocation, Token::Identifier)] = &ConfigParser::GetLocationFunc;
    mFuncMap[make_pair(GetLocationPath, Token::Identifier)] = &ConfigParser::GetLocationPathFunc;
    mFuncMap[make_pair(OpenLocationBlock, Token::LeftCurly)] = &ConfigParser::OpenLocationBlockFunc;
    mFuncMap[make_pair(GetLocationKey, Token::Identifier)] = &ConfigParser::GetLocationKeyFunc;
    mFuncMap[make_pair(GetLocationValue, Token::SemiColon)] = &ConfigParser::GetLocationValueFunc;
    mFuncMap[make_pair(GetLocationValue, Token::Identifier)] = &ConfigParser::GetLocationValueFunc;
    mFuncMap[make_pair(CloseLocationBlock, Token::RightCurly)] = &ConfigParser::CloseLocationBlockFunc;
    mFuncMap[make_pair(CloseLocationBlock, Token::Identifier)] = &ConfigParser::CloseLocationBlockFunc;
    mFuncMap[make_pair(CloseServerBlock, Token::RightCurly)] = &ConfigParser::CloseServerBlockFunc;
    mFuncMap[make_pair(CloseServerBlock, Token::Identifier)] = &ConfigParser::CloseServerBlockFunc;
    mFuncMap[make_pair(CloseHttpBlock, Token::RightCurly)] = &ConfigParser::CloseHttpBlockFunc;
    mFuncMap[make_pair(CloseHttpBlock, Token::Identifier)] = &ConfigParser::CloseHttpBlockFunc;
  }

  HttpConfig parse(const char *fileName) {
    try {
      ifstream file(fileName);
      if (!file.is_open()) {
        throw invalid_argument("Error: " + string(fileName) + " is not found");
      }
      string str((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
      Lexer lexer(str.c_str());
      Token token(Token::Start);

      for (token = lexer.next(); token.kind() != Token::End; token = lexer.next()) {
        void (ConfigParser::*func)(Token) = mFuncMap[make_pair(mStatus, token.kind())];
        if (func) {
          (this->*func)(token);
        } else {
          throw invalid_argument("Error: " + token.lexeme() + " is not expected");
        }
      }
      if (mStatus != Finish) {
        throw invalid_argument("Error: " + token.lexeme() + " is not expected");
      }
      return mHttpConfig;
    } catch (exception &e) {
      cerr << e.what() << endl;
      exit(1);
    }
  }

 private:
  void GetHttpFunc(Token token);
  void OpenHttpBlockFunc(Token token);
  void GetServerFunc(Token token);
  void OpenServerBlockFunc(Token token);
  void GetServerKeyFunc(Token token);
  void GetServerValueFunc(Token token);
  void GetLocationFunc(Token token);
  void GetLocationPathFunc(Token token);
  void OpenLocationBlockFunc(Token token);
  void GetLocationKeyFunc(Token token);
  void GetLocationValueFunc(Token token);
  void CloseLocationBlockFunc(Token token);
  void CloseServerBlockFunc(Token token);
  void CloseHttpBlockFunc(Token token);

 private:
  eStatus mStatus;  // mFinish인지만 확인해주면 됨.

  map<pair<int, Token::Kind>, void (ConfigParser::*)(Token)> mFuncMap;
  HttpConfig mHttpConfig;
};

void ConfigParser::GetHttpFunc(Token token) {
  if (token.lexeme() == "http") {
    mStatus = OpenHttpBlock;
  } else {
    throw "Error: " + token.lexeme() + " is not server";
  }
}

void ConfigParser::OpenHttpBlockFunc(Token token) {
  if (token.lexeme() == "{") {
    mStatus = GetServer;
  } else {
    throw "Error: " + token.lexeme() + " is not {";
  }
}

void ConfigParser::GetServerFunc(Token token) {
  if (token.lexeme() == "server") {
    mStatus = OpenServerBlock;
  } else {
    throw "Error: " + token.lexeme() + " is not server";
  }
}

void ConfigParser::OpenServerBlockFunc(Token token) {
  if (token.lexeme() == "{") {
    mStatus = GetServerKey;
    mHttpConfig.GetServer();
  } else {
    throw "Error: " + token.lexeme() + " is not {";
  }
}

void ConfigParser::GetServerKeyFunc(Token token) {
  mHttpConfig.GetServerKey(token.lexeme());
  if (token.lexeme() == "server_name") {  // key
    mStatus = GetServerValue;
  } else if (token.lexeme() == "listen") {  // key
    mStatus = GetServerValue;
  } else if (token.lexeme() == "error_page") {  // key
    mStatus = GetServerValue;
  } else if (token.lexeme() == "client_max_body_size") {  // key
    mStatus = GetServerValue;
  } else if (token.lexeme() == "location") {  // key
    mStatus = GetLocation;
  } else {
    throw "Error: " + token.lexeme() + " is not server_name or listen or location";
  }
}
void ConfigParser::GetServerValueFunc(Token token) {
  if (token.lexeme() == ";") {
    mStatus = GetLocation;
  } else {
    mStatus = GetServerValue;
    mHttpConfig.GetServerValue(token.lexeme());
  }
}

void ConfigParser::GetLocationFunc(Token token) {
  if (token.lexeme() == "location") {
    mStatus = GetLocationPath;
  } else {
    mStatus = GetServerValue;
    mHttpConfig.GetServerKey(token.lexeme());
  }
}

void ConfigParser::GetLocationPathFunc(Token token) {
  mStatus = OpenLocationBlock;
  mHttpConfig.GetLocationPath(token.lexeme());
}

void ConfigParser::OpenLocationBlockFunc(Token token) {
  if (token.lexeme() == "{") {
    mStatus = GetLocationKey;
  } else {
    throw "Error: " + token.lexeme() + " is not location";
  }
}

void ConfigParser::GetLocationKeyFunc(Token token) {
  mHttpConfig.GetLocationKey(token.lexeme());
  if (token.lexeme() == "root") {
    mStatus = GetLocationValue;
  } else if (token.lexeme() == "index") {
    mStatus = GetLocationValue;
  } else if (token.lexeme() == "autoindex") {
    mStatus = GetLocationValue;
  } else if (token.lexeme() == "limit_except") {
    mStatus = GetLocationValue;
  } else if (token.lexeme() == "return") {
    mStatus = GetLocationValue;
  } else if (token.lexeme() == "error_page") {
    mStatus = GetLocationValue;
  } else if (token.lexeme() == "fastcgi_pass") {
    mStatus = GetLocationValue;
  } else if (token.lexeme() == "fastcgi_param") {
    mStatus = GetLocationValue;
  } else {
    throw "Error: " + token.lexeme() +
        " is not root or index or autoindex or limit_except or return or "
        "error_page or cgi";
  }
}

void ConfigParser::GetLocationValueFunc(Token token) {
  if (token.lexeme() == ";") {
    mStatus = CloseLocationBlock;
  } else {
    mStatus = GetLocationValue;
    mHttpConfig.GetLocationValue(token.lexeme());
  }
}

void ConfigParser::CloseLocationBlockFunc(Token token) {
  if (token.lexeme() == "}") {
    mStatus = CloseServerBlock;
  } else {
    mStatus = GetLocationValue;
    mHttpConfig.GetLocationKey(token.lexeme());
  }
}

void ConfigParser::CloseServerBlockFunc(Token token) {
  if (token.lexeme() == "}") {
    mStatus = CloseHttpBlock;
  } else if (token.lexeme() == "location") {
    mStatus = GetLocationPath;
  } else {
    throw "Error: " + token.lexeme() + " is not } or location";
  }
}

void ConfigParser::CloseHttpBlockFunc(Token token) {
  if (token.lexeme() == "}") {
    mStatus = Finish;
  } else if (token.lexeme() == "server") {
    mStatus = OpenServerBlock;
  }
}
