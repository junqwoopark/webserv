#pragma once

#include <ios>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

#include "String.hpp"

#define MAX_HEADER_SIZE 8092 * 1024

using namespace std;

class Request {
 public:
  Request() {
    mBuffer = "";
    mMethod = "";
    mUri = "";
    mHttpVersion = "";
    mHeaders = map<string, string>();
    mBody = "";
    mIsChunked = false;
  }

  void append(char *buffer, size_t size, size_t maxClientBodySize) {
    if (!mIsHeaderComplete) {
      mBuffer.append(buffer, size);
      size = 0;
      size_t headerEnd = mBuffer.find("\r\n\r\n");
      if (headerEnd == string::npos && mBuffer.size() > MAX_HEADER_SIZE) {
        throw "400";
      } else if (headerEnd != string::npos && headerEnd > MAX_HEADER_SIZE) {
        throw "400";
      } else if (headerEnd != string::npos) {
        parseHeader();

        mBuffer = mBuffer.substr(headerEnd + 4);
        mIsHeaderComplete = true;
      }
    }

    if (mMethod == "POST") {
      if (mIsChunked) {
        mBuffer.append(buffer, size);
        while (true) {
          size_t chunkEnd = mBuffer.find("\r\n");
          if (chunkEnd == string::npos) {
            break;
          }

          int chunkSize = strtol(mBuffer.substr(0, chunkEnd).c_str(), nullptr, 16);
          if (chunkSize == 0) {
            mIsBodyComplete = true;
            mBuffer = mBuffer.substr(chunkEnd + 2);
            break;
          }

          if (mBody.size() + chunkSize > maxClientBodySize) {
            throw "413";
          }

          size_t chunkStart = chunkEnd + 2;
          size_t chunkDataEnd = chunkStart + chunkSize;
          if (chunkDataEnd + 2 > mBuffer.size()) {
            break;
          }
          mBody.append(mBuffer.substr(chunkStart, chunkSize));
          mBuffer = mBuffer.substr(chunkDataEnd + 2);
        }
      } else {
        mBuffer.append(buffer, size);
        size_t contentLength = atoi(mHeaders["Content-Length"].c_str());

        if (contentLength > maxClientBodySize) {
          throw "413";
        }
        if (mBuffer.length() > maxClientBodySize) {
          throw "413";
        }

        if (mBuffer.length() == contentLength) {
          mBody.append(mBuffer);
          mIsBodyComplete = true;
        }
      }
    } else {
      mIsBodyComplete = true;
    }
  }
  bool isComplete() { return mIsHeaderComplete && mIsBodyComplete; }

  string getMethod() { return mMethod; }
  string getUri() { return mUri; }
  string getHttpVersion() { return mHttpVersion; }
  string getHeader(const string &key) { return mHeaders[key]; }
  String &getBody() { return mBody; }
  string getServerName() { return mHeaders["Host"].substr(0, mHeaders["Host"].find(":")); }
  string getQueryString() {
    if (mQueryString.empty()) {
      return "";
    }
    return mQueryString;
  }

 private:
  void parseHeader() {
    stringstream ss(mBuffer.c_str());
    string line;
    getline(ss, line);
    parseRequestLine(line);
    parseHeaders(ss);

    parseUri();
  }
  void parseRequestLine(const string &line) {
    stringstream ss(line);
    getline(ss, mMethod, ' ');
    getline(ss, mUri, ' ');
    getline(ss, mHttpVersion, ' ');
  }

  void parseUri() {
    size_t questionIndex = mUri.find("?");
    if (questionIndex == string::npos) {
      return;
    }
    mQueryString = mUri.substr(questionIndex + 1);
    mUri = mUri.substr(0, questionIndex);
  }

  void parseHeaders(stringstream &ss) {
    string line;
    while (getline(ss, line) && line != "\r") {
      int colonIndex = line.find(":");
      string key = line.substr(0, colonIndex);
      string value = line.substr(colonIndex + 2);

      value = value.substr(0, value.length() - 1);

      mHeaders[key] = value;

      if (key == "Transfer-Encoding" && value == "chunked") {
        mIsChunked = true;
      }
    }
  }

  String mBuffer;
  string mMethod;
  string mUri;
  string mHttpVersion;
  map<string, string> mHeaders;
  String mBody;
  string mQueryString;
  bool mIsChunked;
  bool mIsHeaderComplete;
  bool mIsBodyComplete;
};
