#include <stdio.h>
#include <unistd.h>

int main() {
  // ~/Project/webserv/hello.js
  if (execve("/Project/webserv/hello.js", NULL, NULL) == -1) {
    perror("execve");
  }
}
