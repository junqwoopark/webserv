#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

class EchoServer {
 public:
  EchoServer(int port) : port(port) {}

  void handle_connections(int kq, int fd, int event_filter) {
    if (fd == server_socket && event_filter == EVFILT_READ) {
      // 클라이언트 연결 수락
      struct sockaddr_in client_address;
      socklen_t client_address_len = sizeof(client_address);
      int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);

      std::cout << "Client connected: " << inet_ntoa(client_address.sin_addr) << std::endl;

      // kqueue에 클라이언트 소켓 등록
      struct kevent kev;
      EV_SET(&kev, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
      if (kevent(kq, &kev, 1, NULL, 0, NULL) < 0) {
        std::cerr << "Failed to add client socket to kqueue" << std::endl;
        close(client_socket);
      }

    } else if (event_filter == EVFILT_READ) {
      // HTTP 요청 파싱 및 처리
      char buffer[1024];
      ssize_t bytes_received = recv(fd, buffer, sizeof(buffer), 0);
      if (bytes_received < 0) {
        std::cerr << "Failed to receive message" << std::endl;
        close(fd);
        return;
      }
      if (bytes_received == 0) {
        std::cout << "Client disconnected" << std::endl;
        close(fd);
        return;
      }

      // HTTP 요청 파싱 Request 객체에서 처리
      /* max_client_body_size 체크, request_max_size 체크, timeout 체크 */
      std::string request(buffer, bytes_received);
      std::cout << "request: " << request << std::endl;
      size_t pos = request.find("\r\n\r\n");
      if (pos == std::string::npos) {
        std::cerr << "Invalid HTTP request" << std::endl;
        close(fd);
        return;
      }
      std::string header = request.substr(0, pos);
      std::string body = request.substr(pos + 4);

      std::vector<std::string> lines;
      size_t prev = 0;
      while (true) {
        size_t next = header.find("\r\n", prev);
        if (next == std::string::npos) {
          break;
        }
        lines.push_back(header.substr(prev, next - prev));
        prev = next + 2;
      }

      std::string method, path, version;
      if (lines.empty()) {
        std::cerr << "Invalid HTTP request" << std::endl;
        close(fd);
        return;
      }
      {
        std::istringstream iss(lines[0]);
        iss >> method >> path >> version;
      }

      // HTTP Response 객체에서 처리
      std::ostringstream oss;
      if (method == "GET" && path == "/hello") {
        body = "hello";
        oss << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: text/plain\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "\r\n"
            << body;
      } else if (method == "POST" && path == "/echo") {
        body = "echo";
        oss << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: text/plain\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "\r\n"
            << body;
      } else {
        oss << "HTTP/1.1 404 Not Found\r\n"
            << "Content-Type: text/plain\r\n"
            << "Content-Length: 0\r\n"
            << "\r\n";
      }

      // HTTP 응답 전송
      std::string response = oss.str();
      ssize_t bytes_sent = send(fd, response.c_str(), response.size(), 0);
      if (bytes_sent < 0) {
        std::cerr << "Failed to send message" << std::endl;
      }
      close(fd);
    }
  }

 private:
  int server_socket;
  int port;

 public:
  void start(int kq) {
    // 소켓 생성
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
      std::cerr << "Failed to create server socket" << std::endl;
      return;
    }

    // 소켓 주소 설정
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // 소켓 바인딩
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
      std::cerr << "Failed to bind server socket to port " << port << std::endl;
      close(server_socket);
      return;
    }

    // 클라이언트 연결 대기
    if (listen(server_socket, SOMAXCONN) < 0) {
      std::cerr << "Failed to listen on server socket" << std::endl;
      close(server_socket);
      return;
    }

    std::cout << "Echo server started on port " << port << std::endl;

    // kqueue에 서버 소켓 등록
    struct kevent kev;
    EV_SET(&kev, server_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
    if (kevent(kq, &kev, 1, NULL, 0, NULL) < 0) {
      std::cerr << "Failed to add server socket to kqueue" << std::endl;
      close(server_socket);
    }
  }
};

int main() {
  // kqueue 생성
  int kq = kqueue();
  if (kq < 0) {
    std::cerr << "Failed to create kqueue" << std::endl;
    return 1;
  }

  // 서버 생성
  // initHTTPServers로 여러 서버 생성하고;
  EchoServer server(80);
  server.start(kq);

  // 이벤트 루프
  struct kevent events[10];
  while (true) {
    int nevents = kevent(kq, NULL, 0, events, 10, NULL);
    if (nevents < 0) {
      std::cerr << "Failed to wait for events" << std::endl;
      break;
    }

    for (int i = 0; i < nevents; i++) {
      server.handle_connections(kq, events[i].ident, events[i].filter);
    }
  }

  close(kq);
  return 0;
}
