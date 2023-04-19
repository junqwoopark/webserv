#pragma once

#include <ios>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>

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

  // 헤더가 잘 들어왔는지 판단 rnrn, header_max_size = 8k
  // timeout 혹은 ioState = error

  // body가 size 에 맞게 잘들어왔는지 판단
  // ioState = error

  void append(char *buffer, size_t size, size_t maxClientBodySize) {
    if (!mIsHeaderComplete) {
      mBuffer.append(buffer, size);
      size = 0;
      size_t headerEnd = mBuffer.find("\r\n\r\n");
      if (headerEnd == string::npos && mBuffer.size() > MAX_HEADER_SIZE) {  // 헤더가 완성되지 않았을 때
        cout << "header not complete" << endl;
        // error handle
      } else if (headerEnd != string::npos && headerEnd > MAX_HEADER_SIZE) {  // 헤더가 완성되었는데 너무 클 때,
        cout << "header too big" << endl;
        // error handle
      } else if (headerEnd != string::npos) {  // 찾은 경우...
        parseHeader();

        cout << mBuffer << endl;
        mBuffer = mBuffer.substr(headerEnd + 4);  // 헤더 뒤에 남은 body
        mIsHeaderComplete = true;
        // cout << "mBuffer: " << mBuffer << endl;
        cout << "header complete" << endl;
      }
    }

    if (mMethod == "POST") {
      if (mIsChunked) {
        mBuffer.append(buffer, size);
        while (true) {
          cout << "Chunked mBuffer: " << mBuffer << endl;
          size_t chunkEnd = mBuffer.find("\r\n");
          if (chunkEnd == string::npos) {
            break;
          }
          // 숫자\r\n데이터\r\n

          // Extract the chunk size and convert it from hex to int
          int chunkSize = strtol(mBuffer.substr(0, chunkEnd).c_str(), nullptr, 16);
          if (chunkSize == 0) {  // End of chunked encoding
            mIsBodyComplete = true;
            mBuffer = mBuffer.substr(chunkEnd + 2);  // Move past the final \r\n
            break;
          }

          // Check if the chunk size exceeds the maximum allowed body size
          if (mBody.size() + chunkSize > maxClientBodySize) {
            // error handle
            break;
          }

          // Extract the current chunk and append it to the body
          size_t chunkStart = chunkEnd + 2;
          size_t chunkDataEnd = chunkStart + chunkSize;
          if (chunkDataEnd + 2 > mBuffer.size()) {  // Incomplete chunk
            break;
          }
          mBody.append(mBuffer.substr(chunkStart, chunkSize));
          mBuffer = mBuffer.substr(chunkDataEnd + 2);  // Move past the \r\n after the chunk data
          cout << "mBody: " << mBody << endl;
        }
      } else {
        mBuffer.append(buffer, size);
        size_t contentLength = stoi(mHeaders["Content-Length"]);
        if (contentLength > maxClientBodySize) {
          // error handle
        }
        if (mBuffer.length() > maxClientBodySize) {
          // error handle
        }
        cout << "mBuffer.length : " << mBuffer.length() << endl;
        cout << "contentLength : " << contentLength << endl;
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
  string &getBody() { return mBody; }
  string getServerName() { return mHeaders["Host"].substr(0, mHeaders["Host"].find(":")); }
  string getQueryString() {
    if (mQueryString.empty()) {
      return "";
    }
    return mQueryString;
  }

 private:
  void parseHeader() {
    stringstream ss(mBuffer);
    string line;
    getline(ss, line);
    parseRequestLine(line);
    parseHeaders(ss);
    // parseBody(ss);
    parseUri();
  }
  void parseRequestLine(const string &line) {
    stringstream ss(line);
    getline(ss, mMethod, ' ');
    getline(ss, mUri, ' ');
    getline(ss, mHttpVersion, ' ');
  }

  // void parseHeaders(stringstream &ss) {
  //   string line;
  //   while (getline(ss, line) && line != "\r") {
  //     int colonIndex = line.find(":");
  //     string key = line.substr(0, colonIndex);
  //     string value = line.substr(colonIndex + 1);
  //     mHeaders[key] = value;
  //   }
  // }

  void parseUri() {
    int questionIndex = mUri.find("?");
    if (questionIndex == string::npos) {
      return;
    }
    mQueryString = mUri.substr(questionIndex + 1);
    mUri = mUri.substr(0, questionIndex);
  }

  // void parseBody(stringstream &ss) {
  //   string line;
  //   while (getline(ss, line)) {
  //     mBody += line;
  //   }
  // }

  void parseHeaders(stringstream &ss) {
    string line;
    while (getline(ss, line) && line != "\r") {
      int colonIndex = line.find(":");
      string key = line.substr(0, colonIndex);
      string value = line.substr(colonIndex + 2);

      // Remove the \r from the end of the value
      value = value.substr(0, value.length() - 1);

      mHeaders[key] = value;

      if (key == "Transfer-Encoding" && value == "chunked") {
        cout << "chunked" << endl;
        mIsChunked = true;
      }
    }
  }

  // Add a new method to parse chunked body
  void parseChunkedBody(stringstream &ss) {
    string line;
    size_t chunkSize = 0;
    while (getline(ss, line)) {
      if (chunkSize == 0) {
        chunkSize = std::stoi(line, nullptr, 16);
        if (chunkSize == 0) {
          break;
        }
        continue;
      }

      mBody += line.substr(0, chunkSize);
      chunkSize = 0;
    }
  }

  // Update the parseBody method
  void parseBody(stringstream &ss) {
    if (mIsChunked) {
      parseChunkedBody(ss);
    } else {
      string line;
      while (getline(ss, line)) {
        mBody += line;
      }
    }
  }

  string mBuffer;
  string mMethod;
  string mUri;
  string mHttpVersion;
  map<string, string> mHeaders;
  string mBody;
  string mQueryString;
  bool mIsChunked;
  bool mIsHeaderComplete;
  bool mIsBodyComplete;
};
