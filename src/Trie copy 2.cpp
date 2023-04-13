#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

class ExtensionMap {
 public:
  void insert(string extension, void* data) { data_map[extension] = data; }

  void* searchByExtension(string path, string extension) {
    if (path.size() < extension.size()) {
      return nullptr;
    }

    size_t pos = path.rfind(extension);
    if (pos != string::npos && pos == path.size() - extension.size()) {
      return data_map[extension];
    }
    return nullptr;
  }

 private:
  unordered_map<string, void*> data_map;
};

int main(void) {
  ExtensionMap extension_map;
  void* data = 0;  // 이곳에 데이터를 할당하세요.
  extension_map.insert(".js", data);

  string requestPath = "example.js";
  void* result = extension_map.searchByExtension(requestPath, ".js");

  if (result != nullptr) {
    cout << "Found" << endl;
  } else {
    cout << "Not Found" << endl;
  }
}
