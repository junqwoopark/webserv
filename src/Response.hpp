#pragma once

#include <map>
#include <sstream>
#include <string>
#include <unordered_map>

using namespace std;

class Response {
 public:
  // Response() {}
  Response(int statusCode = 200, const string &body = "") : mStatusCode(statusCode), mBody(body) {
    statusCodes[200] = "OK";
    statusCodes[201] = "Created";
    statusCodes[202] = "Accepted";
    statusCodes[204] = "No Content";
    statusCodes[301] = "Moved Permanently";
    statusCodes[302] = "Found";
    statusCodes[304] = "Not Modified";
    statusCodes[400] = "Bad Request";
    statusCodes[401] = "Unauthorized";
    statusCodes[403] = "Forbidden";
    statusCodes[404] = "Not Found";
    statusCodes[405] = "Method Not Allowed";
    statusCodes[406] = "Not Acceptable";
    statusCodes[408] = "Request Timeout";
    statusCodes[413] = "Payload Too Large";
    statusCodes[414] = "URI Too Long";
    statusCodes[415] = "Unsupported Media Type";
    statusCodes[416] = "Range Not Satisfiable";
    statusCodes[417] = "Expectation Failed";
    statusCodes[418] = "I'm a teapot";
    statusCodes[500] = "Internal Server Error";
    statusCodes[501] = "Not Implemented";
    statusCodes[502] = "Bad Gateway";
    statusCodes[503] = "Service Unavailable";
    statusCodes[504] = "Gateway Timeout";
    statusCodes[505] = "HTTP Version Not Supported";
  }
  ~Response() {}

  void setStatusCode(int statusCode) { mStatusCode = statusCode; }
  void setBody(const string &body) { mBody = body; }
  void setLocation(const string &location) { mLocation = location; }
  void append(char *buffer, size_t size) { mBody.append(buffer, size); }

  string getResponse() {
    stringstream response;
    response << "HTTP/1.1 " << mStatusCode << " " << Response::statusCodes[mStatusCode] << "\r\n";
    response << "Content-Length: " << mBody.length() << "\r\n";
    response << "Content-Type: text/html\r\n";
    response << "Location: " << mLocation << "\r\n";
    response << "\r\n";
    response << mBody;
    return response.str();
  }

 public:
 private:
  int mStatusCode;
  string mLocation;
  string mBody;
  unordered_map<int, string> statusCodes;
};
