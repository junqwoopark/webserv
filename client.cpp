// send to 127.0.0.1 8080

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>

using namespace std;

int main(void) {
  int sock;
  struct sockaddr_in addr;
  char buf[1024];

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    exit(1);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(8080);
  addr.sin_addr.s_addr = inet_addr("127.0.0.1");
  if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("connect");
    exit(2);
  }

  std::string s =
      "GET / HTTP/1.1\r\n"
      "Host: localhost:8080\r\n"
      "Connection: keep-alive\r\n"
      "Cache-Control: max-age=0\r\n";

  write(sock, s.c_str(), s.length() + 1);

  while (true) {
    int n = read(sock, buf, sizeof(buf));  // 여기서 계속 읽고 있어서 그런가?
    if (n <= 0) {
      break;
    }
    write(1, buf, n);
  }
}
