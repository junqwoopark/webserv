#pragma once

#include <sstream>
#include <string>

using namespace std;

class Response {
public:
  Response() {}
  ~Response() {}

  void setStatusCode(int statusCode) { mStatusCode = statusCode; }
  void setBody(const string &body) { mBody = body; }

  string getResponse() {
    string response = "HTTP/1.1 " + to_string(mStatusCode) + " OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + to_string(mBody.size()) + "\r\n";
    response += "\r\n";
    response += mBody;
    return response;
  }

private:
  int mStatusCode;
  string mBody;
};
