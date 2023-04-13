#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct Node {
  void* data;
  bool isEndOfWord;
  unordered_map<string, Node*> children;
};

// vector<string>

//  /home
//  /home/index.html
//  /home/index.html/index2.html

class Trie {
 public:
  Trie() { root = new Node(); }

  void insert(vector<string> path, void* data) {
    Node* cur = root;

    for (int i = 0; i < path.size(); i++) {
      string name = path[i];
      if (cur->children.find(name) == cur->children.end()) {
        cur->children[name] = new Node();
      }
      cur = cur->children[name];
    }
    cur->data = data;
    cur->isEndOfWord = true;
  }

  void* search(vector<string> path) {
    Node* cur = root;
    void* data = NULL;

    for (int i = 0; i < path.size(); i++) {
      string name = path[i];
      if (cur->children.find(name) == cur->children.end()) {
        break;
      }
      cur = cur->children[name];
      if (cur->isEndOfWord) {
        data = cur->data;
      }
    }
    return data;
  }

 private:
  Node* root;
};

// implement split with sstream

vector<string> split(string str, string delimiter) {
  vector<string> result;
  size_t pos = 0;
  string token;
  while ((pos = str.find(delimiter)) != string::npos) {
    token = str.substr(0, pos);
    result.push_back(token);
    str.erase(0, pos + delimiter.length());
  }
  result.push_back(str);
  return result;
}

int main(void) {
  Trie trie;

  vector<string> path1 = split("/home", "/");
  vector<string> path2 = split("/home/index.html", "/");
  vector<string> path3 = split("/home/index.html/index2.html", "/");
  vector<string> path4 = split("/home/index.html/index2.html/index3.html", "/");

  trie.insert(path1, (void*)1);
  trie.insert(path2, (void*)2);
  trie.insert(path3, (void*)3);
  trie.insert(path4, (void*)4);

  vector<string> path5 = split("/home/", "/");
  pair<void*, string> result = trie.search(path5);
  cout << result.first << endl;
  cout << result.second << endl;
}
