#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

int main() {
  const std::string server_ip =
      "127.0.0.1";              // Replace with your PHP-FPM server IP
  const int server_port = 9000; // Replace with your PHP-FPM server port
  const std::string scriptFilename = "phpinfo.php";

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    return 1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  if (inet_pton(AF_INET, server_ip.c_str(), &addr.sin_addr) <= 0) {
    perror("inet_pton");
    close(sock);
    return 1;
  }

  if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("connect");
    close(sock);
    return 1;
  }

  // Prepare the request
  std::string request = "GET /" + scriptFilename +
                        " HTTP/1.1\r\n"
                        "Host: " +
                        server_ip + ":" + std::to_string(server_port) +
                        "\r\n"
                        "\r\n";

  // Send the request to PHP-FPM
  if (write(sock, request.c_str(), request.size()) == -1) {
    perror("write");
    close(sock);
    return 1;
  }

  // Read and print the PHP response
  char buffer[4096];
  ssize_t bytesRead;
  while ((bytesRead = read(sock, buffer, sizeof(buffer))) > 0) {
    std::cout.write(buffer, bytesRead);
  }
  if (bytesRead == -1) {
    perror("read");
    close(sock);
    return 1;
  }

  close(sock);
  return 0;
}
