#pragma once

#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Mime.hpp"
#include "StatusCode.hpp"

using namespace std;

class Response {
 public:
  // Response() {}
  Response(int statusCode = 200) : mStatusCode(statusCode) {}
  ~Response() {}

  void setStatusCode(int statusCode) { mStatusCode = statusCode; }
  void setBody(const string &body) { mBody = vector<char>(body.begin(), body.end()); }
  void setError(int errorCode) {
    mStatusCode = errorCode;
    mBody = vector<char>(mStatusCodes[errorCode].begin(), mStatusCodes[errorCode].end());
  }
  void setContentType(const string &contentType) { mContentType = contentType; }
  void setLocation(const string &location) { mLocation = location; }
  void append(char *buffer, size_t size) { mBody.insert(mBody.end(), buffer, buffer + size); }

  vector<char> getResponse() {
    stringstream response;
    response << "HTTP/1.1 " << mStatusCode << " " << mStatusCodes[mStatusCode] << "\r\n";
    response << "Content-Length: " << mBody.size() << "\r\n";
    response << "Content-Type: " << (mMime[mContentType] == "" ? "text/html" : mMime[mContentType]) << "\r\n";

    if (mLocation != "") response << "Location: " << mLocation << "\r\n";

    response << "\r\n";

    string header = response.str();
    vector<char> result(header.begin(), header.end());
    result.insert(result.end(), mBody.begin(), mBody.end());

    return result;
  }

 private:
  int mStatusCode;
  string mContentType;
  string mLocation;
  vector<char> mBody;
  static StatusCode mStatusCodes;
  static Mime mMime;
};

StatusCode Response::mStatusCodes;
Mime Response::mMime;
