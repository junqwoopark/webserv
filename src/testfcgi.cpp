#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>
#include <string>

int main() {
  const std::string socket_path = "/opt/homebrew/var/run/php-fpm.sock";  // Replace with your PHP-FPM socket path
  const std::string script_filename = "/Users/junkpark/goinfre/webserv/phpinfo.php";

  int sock = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    return 1;
  }

  struct sockaddr_un addr;
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

  if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("connect");
    close(sock);
    return 1;
  }

  // Prepare the request
  std::string request = "GET /" + script_filename +
                        " HTTP/1.1\r\n"
                        "Host: localhost\r\n"
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
