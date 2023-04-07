#pragma once

#include <map>
#include <sstream>
#include <string>

using namespace std;

class Request {
 public:
  Request(const std::string &rawRequest) {
    mRawRequest = rawRequest;
    parse();
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

  friend ostream &operator<<(ostream &os, const Request &request) {
    os << "Method: " << request.mMethod << endl;
    os << "Uri: " << request.mUri << endl;
    os << "HttpVersion: " << request.mHttpVersion << endl;
    for (auto &header : request.mHeaders) {
      os << header.first << ": " << header.second << endl;
    }
    os << "Body: " << request.mBody << endl;
    return os;
  }
};
