#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

class String : public vector<char> {
 public:
  String() : vector<char>() {}
  String(string str) : vector<char>(str.begin(), str.end()) {}
  String(vector<char> vec) : vector<char>(vec) {}
  String(const char* str) : vector<char>(str, str + strlen(str)) {}

  void append(const String& other) {
    size_t oldSize = size();
    resize(oldSize + other.size());
    copy(other.begin(), other.end(), begin() + oldSize);
  }

  void append(const char* str, size_t len) {
    size_t oldSize = size();
    resize(oldSize + len);
    copy(str, str + len, begin() + oldSize);
  }

  size_t find(const String& target) const {
    const_iterator it = std::search(begin(), end(), target.begin(), target.end());
    if (it != end()) {
      return distance(begin(), it);
    }
    return string::npos;
  }

  String substr(size_t pos, size_t len = string::npos) const {
    if (len == string::npos) {
      len = size() - pos;
    }
    return String(vector<char>(begin() + pos, begin() + pos + len));
  }

  const char* c_str() const { return data(); }

  size_t length() const { return size(); }

  String& operator+=(const String& other) {
    append(other.data(), other.size());
    return *this;
  }

  String operator+(const String& other) const {
    String result(*this);
    result.append(other.data(), other.size());
    return result;
  }

  friend ostream& operator<<(ostream& os, const String& str) {
    os.write(str.data(), str.size());
    return os;
  }
};
