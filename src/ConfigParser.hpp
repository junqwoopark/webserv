#pragma once

#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "Config.hpp"
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

  void parse(vector<Token> tokens) {
    for (auto token : tokens) {
      auto func = mFuncMap[make_pair(mStatus, token.kind())];
      if (func) {
        (this->*func)(token);
      } else {
        throw "Error: " + token.lexeme() + " is not expected";
      }
    }
    if (mStatus != Finish) {
      throw "Error: " + tokens.back().lexeme() + " is not expected";
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
  } else {
    throw "Error: " + token.lexeme() + " is not {";
  }
}

void ConfigParser::GetServerKeyFunc(Token token) {
  if (token.lexeme() == "server_name") {
    mStatus = GetServerValue;
  } else if (token.lexeme() == "listen") {
    mStatus = GetServerValue;
  } else if (token.lexeme() == "location") {
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
  }

  // if (token.lexeme() == "server_name") {
  //   mStatus = GetServerValue;
  // } else if (token.lexeme() == "listen") {
  //   mStatus = GetServerValue;
  // } else if (token.lexeme() == "location") {
  //   mStatus = GetLocation;
  // } else {
  //   throw "Error: " + token.lexeme() + " is not server_name or listen or location";
  // }
}
void ConfigParser::GetLocationFunc(Token token) {
  if (token.lexeme() == "location") {
    mStatus = GetLocationPath;
  } else {
    mStatus = GetServerValue;
  }
}
void ConfigParser::GetLocationPathFunc(Token token) { mStatus = OpenLocationBlock; }
void ConfigParser::OpenLocationBlockFunc(Token token) {
  if (token.lexeme() == "{") {
    mStatus = GetLocationKey;
  } else {
    throw "Error: " + token.lexeme() + " is not location";
  }
}

void ConfigParser::GetLocationKeyFunc(Token token) {
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
  } else if (token.lexeme() == "cgi") {
    mStatus = GetLocationValue;
  } else {
    throw "Error: " + token.lexeme() +
        " is not root or index or autoindex or limit_except or return or "
        "error_page or cgi";
  }
}
void ConfigParser::GetLocationValueFunc(Token token) {
  if (token.lexeme() == ";")
    mStatus = CloseLocationBlock;
  else
    mStatus = GetLocationValue;
}
void ConfigParser::CloseLocationBlockFunc(Token token) {
  if (token.lexeme() == "}") {
    mStatus = CloseServerBlock;
  } else {
    mStatus = GetLocationValue;
  }
}

void ConfigParser::CloseServerBlockFunc(Token token) {
  if (token.lexeme() == "}") {
    mStatus = CloseHttpBlock;
  } else if (token.lexeme() == "location") {
    mStatus = GetLocationPath;
  }
}

void ConfigParser::CloseHttpBlockFunc(Token token) {
  if (token.lexeme() == "}") {
    mStatus = Finish;
  } else if (token.lexeme() == "server") {
    mStatus = OpenServerBlock;
  }
}
