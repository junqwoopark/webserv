#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "HttpConfig.hpp"

using namespace std;

struct Node {
  void* data;
  bool isEndOfWord;
  unordered_map<char, Node*> children;
};

class Trie {
 public:
  Trie() : root(NULL) {}

  void insert(string path, void* data) {
    if (root == NULL) {
      root = new Node();
    }

    Node* cur = root;

    for (size_t i = 0; i < path.size(); i++) {
      char name = path[i];
      if (cur->children.find(name) == cur->children.end()) {
        cur->children[name] = new Node();
      }
      cur = cur->children[name];
    }
    cur->data = data;
    cur->isEndOfWord = true;
  }

  pair<void*, string> search(string path) {
    Node* cur = root;
    void* data = NULL;
    int idx = 0;

    for (size_t i = 0; i < path.size(); i++) {
      char name = path[i];
      if (cur->children.find(name) == cur->children.end()) {
        break;
      }
      cur = cur->children[name];
      if (cur->isEndOfWord) {
        data = cur->data;
        idx = i;
      }
    }
    return make_pair(data, '/' + path.substr(idx + 1));
  }

 private:
  Node* root;
};
