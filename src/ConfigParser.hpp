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
    GetServer,
    OpenServerBlock,
    GetKey,
    GetValue,
    GetLocation,
    GetPath,
    OpenLocationBlock,
    GetLocationKey,
    GetLocationValue,
    CloseLocationBlock,
    CloseServerBlock,
    Finish,
  };

  ConfigParser() : mStatus(GetServer) {
    // mFuncMap initialization
    mFuncMap[make_pair(GetServer, Token::Identifier)] = &ConfigParser::GetServerFunc;
    mFuncMap[make_pair(OpenServerBlock, Token::LeftCurly)] = &ConfigParser::OpenServerBlockFunc;
    mFuncMap[make_pair(GetKey, Token::Identifier)] = &ConfigParser::GetKeyFunc;
    mFuncMap[make_pair(GetValue, Token::SemiColon)] = &ConfigParser::GetValueFunc;
    mFuncMap[make_pair(GetValue, Token::Identifier)] = &ConfigParser::GetValueFunc;
    mFuncMap[make_pair(GetLocation, Token::Identifier)] = &ConfigParser::GetLocationFunc;
    mFuncMap[make_pair(GetPath, Token::Identifier)] = &ConfigParser::GetPathFunc;
    mFuncMap[make_pair(OpenLocationBlock, Token::LeftCurly)] = &ConfigParser::OpenLocationBlockFunc;
    mFuncMap[make_pair(GetLocationKey, Token::Identifier)] = &ConfigParser::GetLocationKeyFunc;
    mFuncMap[make_pair(GetLocationValue, Token::SemiColon)] = &ConfigParser::GetLocationValueFunc;
    mFuncMap[make_pair(GetLocationValue, Token::Identifier)] = &ConfigParser::GetLocationValueFunc;
    mFuncMap[make_pair(CloseLocationBlock, Token::RightCurly)] = &ConfigParser::CloseLocationBlockFunc;
    mFuncMap[make_pair(CloseLocationBlock, Token::Identifier)] = &ConfigParser::CloseLocationBlockFunc;
    mFuncMap[make_pair(CloseServerBlock, Token::RightCurly)] = &ConfigParser::CloseServerBlockFunc;
    mFuncMap[make_pair(CloseServerBlock, Token::Identifier)] = &ConfigParser::CloseServerBlockFunc;
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
  }

 private:
  void GetServerFunc(Token token);
  void OpenServerBlockFunc(Token token);
  void GetKeyFunc(Token token);
  void GetValueFunc(Token token);
  void GetLocationFunc(Token token);
  void GetPathFunc(Token token);
  void OpenLocationBlockFunc(Token token);
  void GetLocationKeyFunc(Token token);
  void GetLocationValueFunc(Token token);
  void CloseLocationBlockFunc(Token token);
  void CloseServerBlockFunc(Token token);

 private:
  eStatus mStatus;

  map<pair<int, Token::Kind>, void (ConfigParser::*)(Token)> mFuncMap;
};

void ConfigParser::GetServerFunc(Token token) {
  if (token.lexeme() == "server") {
    mStatus = OpenServerBlock;
  } else {
    throw "Error: " + token.lexeme() + " is not server";
  }
}

void ConfigParser::OpenServerBlockFunc(Token token) {
  if (token.lexeme() == "{") {
    mStatus = GetKey;
  } else {
    throw "Error: " + token.lexeme() + " is not {";
  }
}

void ConfigParser::GetKeyFunc(Token token) {
  if (token.lexeme() == "server_name") {
    mStatus = GetValue;
  } else if (token.lexeme() == "listen") {
    mStatus = GetValue;
  } else if (token.lexeme() == "location") {
    mStatus = GetLocation;
  } else {
    throw "Error: " + token.lexeme() + " is not server_name or listen or location";
  }
}
void ConfigParser::GetValueFunc(Token token) {
  if (token.lexeme() == ";") {
    mStatus = GetLocation;
  } else {
    mStatus = GetValue;
  }

  // if (token.lexeme() == "server_name") {
  //   mStatus = GetValue;
  // } else if (token.lexeme() == "listen") {
  //   mStatus = GetValue;
  // } else if (token.lexeme() == "location") {
  //   mStatus = GetLocation;
  // } else {
  //   throw "Error: " + token.lexeme() + " is not server_name or listen or location";
  // }
}
void ConfigParser::GetLocationFunc(Token token) {
  if (token.lexeme() == "location") {
    mStatus = GetPath;
  } else {
    mStatus = GetValue;
  }
}
void ConfigParser::GetPathFunc(Token token) { mStatus = OpenLocationBlock; }
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
  } else if (token.lexeme() == "}") {
    mStatus = CloseLocationBlock;
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
    throw "Error: " + token.lexeme() + " is not }";
  }
}

void ConfigParser::CloseServerBlockFunc(Token token) {
  if (token.lexeme() == "}") {
    mStatus = Finish;
  } else if (token.lexeme() == "location") {
    mStatus = GetPath;
  }
}
