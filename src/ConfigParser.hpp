#pragma once

#include "Config.hpp"
#include <cstddef>
#include <string>

using namespace std;

Parser(table);

config : parser;

class ConfigParser {
public:
  ConfigParser(const string &configFilePath)
      : mConfigFilePath(configFilePath) {}
  virtual ~ConfigParser() {}

  vector<Config> parse() const;
};
