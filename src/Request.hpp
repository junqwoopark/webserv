#pragma once

#include <map>
#include <sstream>
#include <string>

using namespace std;

class Request {
 public:
  Request() {}

  void append(char *buffer, size_t size) { mRawRequest.append(buffer, size); }
  bool isComplete() {
    if (mRawRequest.find("\r\n\r\n") != string::npos) {
      parse();
      return true;
    }
    return false;
  }

  string getMethod() { return mMethod; }
  string getUri() { return mUri; }
  string getHttpVersion() { return mHttpVersion; }
  string getHeader(const string &key) { return mHeaders[key]; }
  string getBody() { return mBody; }

 private:
  void parse() {
    stringstream ss(mRawRequest);
    string line;
    getline(ss, line);
    parseRequestLine(line);
    parseHeaders(ss);
    parseBody(ss);
  }
  void parseRequestLine(const string &line) {
    stringstream ss(line);
    ss >> mMethod >> mUri >> mHttpVersion;
  }

  void parseHeaders(stringstream &ss) {
    string line;
    while (getline(ss, line) && line != "\r") {
      int colonIndex = line.find(":");
      string key = line.substr(0, colonIndex);
      string value = line.substr(colonIndex + 1);
      mHeaders[key] = value;
    }
  }

  void parseBody(stringstream &ss) {
    string line;
    while (getline(ss, line)) {
      mBody += line;
    }
  }

  string mRawRequest;
  string mMethod;
  string mUri;
  string mHttpVersion;
  map<string, string> mHeaders;
  string mBody;
};
