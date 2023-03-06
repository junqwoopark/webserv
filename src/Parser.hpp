#pragma once

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// class Parser {
// public:
//   Parser(unordered_map<class Key, class Tp> table) : mTable(table) {}
//   virtual ~Parser() {}
//   virtual vector<class Tp> parse(const string &configFilePath) const = 0;

// private:
//   unordered_map<class Key, class Tp> mTable;
// };

/*
    nginx.conf
    http {
            server {
                    location {
                    };
            };
            server {
                    location {
                    };
            };
            server {
                    location {
                    };
            };
    };
*/

/*
        http
    method: GET /home.html HTTP/1.1
    Host: developer.mozilla.org
    User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:50.0l
        Gecko/20100101 Firefox/50.0 Accept-Language: en-US,en;q=0.5
   Accept-Encoding: gzip, deflate, br Referer:
   https://developer.mozilla.org/testpage.html Connection: keep-alive
    Upgrade-Insecure-Requests: 1
    If-Modified-Since: Mon, 18 Jul 2016 02:36:04 GMT
    If-None-Match: "c561c68d0ba92bbeb8b0fff2a9199f722e3a621a"
    Cache-Control: max-age=0
*/

class Parser {
public:
  Parser(std::string input_str) : input_str_(input_str), pos_(0) {}

  // Parse method to be implemented in derived classes
  virtual void parse() = 0;

protected:
  // Input string and current position in string
  std::string input_str_;
  std::size_t pos_;

  // Helper functions for parsing
  char current_char() { return input_str_[pos_]; }
  char next_char() { return input_str_[pos_ + 1]; }
  bool has_next_char() { return pos_ < input_str_.size() - 1; }
  bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
  }
  void skip_whitespace() {
    while (is_whitespace(current_char()))
      pos_++;
  }
  bool match(char c) {
    if (current_char() == c) {
      pos_++;
      return true;
    }
    return false;
  }
};

#include <iostream>
#include <string>
#include <utility>
#include <vector>

class httpMethodParser : public Parser {
public:
  httpMethodParser(std::string input_str) : Parser(input_str) {}

  // Parse method to be implemented in derived classes
  void parse() override;

private:
  // Parsing table
  std::vector<std::pair<std::string, std::string>> parsing_table_ = {
      {"method", ""},
      {"path", ""},
      {"http_version", ""},
      {"header_key", ""},
      {"header_value", ""}};
};

#include <iostream>
#include <string>
#include <utility>
#include <vector>

class nginxConfigParser : public Parser {
public:
  nginxConfigParser(std::string input_str) : Parser(input_str) {}

  // Parse method to be implemented in derived classes
  void parse() override;

private:
  // Parsing table
  std::vector<std::pair<std::string, std::string>> parsing_table_ = {
      {"server", ""},
      {"location", ""},
      {"setting_key", ""},
      {"setting_value", ""}};
};

void httpMethodParser::parse() {
  // Parse the method
  std::string method = "";
  while (has_next_char() && !is_whitespace(current_char())) {
    method += current_char();
    pos_++;
  }
  parsing_table_[0].second = method;

  // Skip whitespace
  skip_whitespace();

  // Parse the path
  std::string path = "";
  while (has_next_char() && !is_whitespace(current_char())) {
    path += current_char();
    pos_++;
  }
  parsing_table_[1].second = path;

  // Skip whitespace
  skip_whitespace();

  // Parse the HTTP version
  std::string http_version = "";
  while (has_next_char() && !is_whitespace(current_char())) {
    http_version += current_char();
    pos_++;
  }
  parsing_table_[2].second = http_version;

  // Skip whitespace
  skip_whitespace();

  // Parse the headers
  while (has_next_char()) {
    // Parse the header key
    std::string header_key = "";
    while (has_next_char() && current_char() != ':') {
      header_key += current_char();
      pos_++;
    }
    if (!has_next_char())
      break;
    match(':');
    parsing_table_[3].second = header_key;

    // Skip whitespace
    skip_whitespace();

    // Parse the header value
    std::string header_value = "";
    while (has_next_char() && current_char() != '\r' &&
           current_char() != '\n') {
      header_value += current_char();
      pos_++;
    }
    parsing_table_[4].second = header_value;

    // Skip whitespace and newlines
    while (has_next_char() &&
           (current_char() == '\r' || current_char() == '\n')) {
      pos_++;
    }
  }
}

// void nginxConfigParser::parse() {
//   // Parse the server blocks
//   while (pos_ < input_str_.size()) {
//     skip_whitespace();
//
//     // Check for end of input
//     if (!has_next_char())
//       break;
//
//     // Check for start of a new server block
//     if (current_char() == 's' && input_str_.substr(pos_, 6) == "server") {
//       pos_ += 6;
//       skip_whitespace();
//
//       // Parse server block
//       match('{');
//       while (has_next_char() && current_char() != '}') {
//         skip_whitespace();
//
//         // Check for start of a new location block
//                 if (current_char() == 'l' && input_str_.substr(pos_, 8) ==
//

std::unordered_map<std::string, std::function<void(std::string)>> parsingTable =
    {
        {"http", [](std::string arg) { httpBlockStartHandler(arg); }},
        {"server", [](std::string arg) { serverBlockStartHandler(arg); }},
        {"location", [](std::string arg) { locationBlockStartHandler(arg); }},
        {"}", [](std::string arg) { blockEndHandler(arg); }},
        // ...
};

std::unordered_map<std::string, std::function<void(std::string)>> parsingTable =
    {
        {"GET", [](std::string arg) { httpGetHandler(arg); }},
        {"POST", [](std::string arg) { httpPostHandler(arg); }},
        {"PUT", [](std::string arg) { httpPutHandler(arg); }},
        {"DELETE", [](std::string arg) { httpDeleteHandler(arg); }},
        // ...
};

void Parser::parse(std::string input) {
  std::istringstream iss(input);
  std::string token;
  while (iss >> token) {
    auto it = parsingTable.find(token);
    if (it != parsingTable.end()) {
      std::string arg;
      std::getline(iss, arg);
      it->second(arg);
    } else {
      std::cerr << "Unknown token: " << token << std::endl;
    }
  }
}
