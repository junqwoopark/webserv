#pragma once

#include <map>
#include <string>

using namespace std;

class StatusCode : public std::map<int, string> {
 public:
  StatusCode() {
    (*this)[200] = "OK";
    (*this)[201] = "Created";
    (*this)[202] = "Accepted";
    (*this)[204] = "No Content";
    (*this)[301] = "Moved Permanently";
    (*this)[302] = "Found";
    (*this)[304] = "Not Modified";
    (*this)[400] = "Bad Request";
    (*this)[401] = "Unauthorized";
    (*this)[403] = "Forbidden";
    (*this)[404] = "Not Found";
    (*this)[405] = "Method Not Allowed";
    (*this)[406] = "Not Acceptable";
    (*this)[408] = "Request Timeout";
    (*this)[413] = "Payload Too Large";
    (*this)[414] = "URI Too Long";
    (*this)[415] = "Unsupported Media Type";
    (*this)[416] = "Range Not Satisfiable";
    (*this)[417] = "Expectation Failed";
    (*this)[418] = "I'm a teapot";
    (*this)[500] = "Internal Server Error";
    (*this)[501] = "Not Implemented";
    (*this)[502] = "Bad Gateway";
    (*this)[503] = "Service Unavailable";
    (*this)[504] = "Gateway Timeout";
    (*this)[505] = "HTTP Version Not Supported";
  }
};
