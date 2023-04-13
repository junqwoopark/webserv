// #include <iostream>
// #include <sstream>
// #include <string>
// #include <unordered_map>
// #include <vector>
//
// using namespace std;
//
// struct Node {
//   void* data;
//   bool isEndOfWord;
//   unordered_map<char, Node*> children;
// };
//
// class Trie {
//  public:
//   Trie() {}
//
//   void insert(string path, void* data) {
//     if (root == NULL) {
//       root = new Node();
//     }
//
//     Node* cur = root;
//
//     for (int i = 0; i < path.size(); i++) {
//       char name = path[i];
//       if (cur->children.find(name) == cur->children.end()) {
//         cur->children[name] = new Node();
//       }
//       cur = cur->children[name];
//     }
//     cur->data = data;
//     cur->isEndOfWord = true;
//   }
//
//   void* search(string path) {
//     Node* cur = root;
//     void* data = NULL;
//
//     for (int i = 0; i < path.size(); i++) {
//       char name = path[i];
//       // cout << name << endl;
//       if (cur->children.find(name) == cur->children.end()) {
//         break;
//       }
//       cur = cur->children[name];
//       if (cur->isEndOfWord) {
//         data = cur->data;
//       }
//     }
//     return data;
//   }
//
//  private:
//   Node* root;
// };

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

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

    for (int i = 0; i < path.size(); i++) {
      char name = path[i];
      if (cur->children.find(name) == cur->children.end()) {
        cur->children[name] = new Node();
      }
      cur = cur->children[name];
    }
    cur->data = data;
    cur->isEndOfWord = true;
  }

  void* search(string path) {
    Node* cur = root;
    void* data = NULL;

    for (int i = 0; i < path.size(); i++) {
      char name = path[i];
      cout << name << endl;
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

  void* searchByExtension(string path) {
    Node* cur = root;
    void* data = NULL;
    size_t pos = path.find_last_of(".");
    string extension;

    if (pos == string::npos) {
      return NULL;
    }
    extension = "*" + path.substr(pos);

    for (int i = 0; i < extension.size(); i++) {
      char name = extension[i];
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

int main(void) {
  Trie trie;
  void* data = (void*)1;  // 이곳에 데이터를 할당하세요.
  trie.insert("*.js", data);

  string requestPath = "examplejs";
  void* result = trie.searchByExtension(requestPath);

  if (result != NULL) {
    cout << "Found" << endl;
  } else {
    cout << "Not Found" << endl;
  }
}
