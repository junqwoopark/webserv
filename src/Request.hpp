#pragma once

#include <map>
#include <sstream>
#include <string>

using namespace std;

class Request {
 public:
  Request() {
    mRawRequest = "";
    mMethod = "";
    mUri = "";
    mHttpVersion = "";
    mHeaders = map<string, string>();
    mBody = "";
  }

  void append(char *buffer, size_t size) { mRawRequest.append(buffer, size); }
  bool isComplete() {
    // body까지 다 받았는지 확인
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
  string &getBody() { return mBody; }
  string getQueryString() {
    if (mQueryString.empty()) {
      return "";
    }
    return mQueryString;
  }

 private:
  void parse() {
    stringstream ss(mRawRequest);
    string line;
    getline(ss, line);
    parseRequestLine(line);
    parseHeaders(ss);
    parseBody(ss);
    parseUri();
  }
  void parseRequestLine(const string &line) {
    stringstream ss(line);
    getline(ss, mMethod, ' ');
    getline(ss, mUri, ' ');
    getline(ss, mHttpVersion, ' ');
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

  void parseUri() {
    int questionIndex = mUri.find("?");
    if (questionIndex == string::npos) {
      return;
    }
    mQueryString = mUri.substr(questionIndex + 1);
    mUri = mUri.substr(0, questionIndex);
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
  string mQueryString;
};
