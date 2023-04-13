#ifndef FAST_CGI_REQUEST_H
#define FAST_CGI_REQUEST_H

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

class FastCgiRequest {
 public:
  FastCgiRequest(const std::string &host, int port);
  ~FastCgiRequest();

  std::string sendRequest(const std::string &requestUri);

 private:
  std::string host_;
  int port_;
};

FastCgiRequest::FastCgiRequest(const std::string &host, int port) : host_(host), port_(port) {}

FastCgiRequest::~FastCgiRequest() {}

std::string FastCgiRequest::sendRequest(const std::string &requestUri) {
  int sockfd;
  struct sockaddr_in server_addr;
  struct hostent *server;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    throw std::runtime_error("Failed to open socket");
  }

  server = gethostbyname(host_.c_str());
  if (server == NULL) {
    throw std::runtime_error("Host not found");
  }

  memset((char *)&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  memcpy((char *)&server_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
  server_addr.sin_port = htons(port_);

  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    throw std::runtime_error("Failed to connect to server");
  }

  // 여기서 FastCGI 요청을 만들고, 인코딩한 후 서버로 전송하세요.
  // 이 예제에서는 실제 FastCGI 요청을 만들지 않았습니다.
  std::string dummyRequest = "Dummy FastCGI request for " + requestUri;
  send(sockfd, dummyRequest.c_str(), dummyRequest.size(), 0);

  char buffer[4096];
  int bytesRead = recv(sockfd, buffer, sizeof(buffer), 0);
  if (bytesRead < 0) {
    throw std::runtime_error("Failed to read from server");
  }

  std::string response(buffer, bytesRead);
  close(sockfd);

  return response;
}
struct FCGI_Header {
  unsigned char version;
  unsigned char type;
  unsigned char requestIdB1;
  unsigned char requestIdB0;
  unsigned char contentLengthB1;
  unsigned char contentLengthB0;
  unsigned char paddingLength;
  unsigned char reserved;
};

struct FCGI_BeginRequestBody {
  unsigned char roleB1;
  unsigned char roleB0;
  unsigned char flags;
  unsigned char reserved[5];
};

struct FCGI_EndRequestBody {
  unsigned char appStatusB3;
  unsigned char appStatusB2;
  unsigned char appStatusB1;
  unsigned char appStatusB0;
  unsigned char protocolStatus;
  unsigned char reserved[3];
};
// 함수: createFastCgiHeader
FCGI_Header createFastCgiHeader(unsigned char type, int requestId, int contentLength, int paddingLength) {
  FCGI_Header header;
  header.version = 1;
  header.type = type;
  header.requestIdB1 = (requestId >> 8) & 0xff;
  header.requestIdB0 = requestId & 0xff;
  header.contentLengthB1 = (contentLength >> 8) & 0xff;
  header.contentLengthB0 = contentLength & 0xff;
  header.paddingLength = paddingLength;
  header.reserved = 0;
  return header;
}

// 함수: createFastCgiBeginRequestBody
FCGI_BeginRequestBody createFastCgiBeginRequestBody(int role, int keepConnection) {
  FCGI_BeginRequestBody body;
  body.roleB1 = (role >> 8) & 0xff;
  body.roleB0 = role & 0xff;
  body.flags = keepConnection ? 1 : 0;
  memset(body.reserved, 0, sizeof(body.reserved));
  return body;
}

// 함수: sendFastCgiRecord
void sendFastCgiRecord(int sockfd, const FCGI_Header &header, const void *content) {
  send(sockfd, &header, sizeof(header), 0);
  if (content != NULL) {
    int contentLength = (header.contentLengthB1 << 8) + header.contentLengthB0;
    send(sockfd, content, contentLength, 0);
  }
  if (header.paddingLength > 0) {
    char padding[8] = {0};
    send(sockfd, padding, header.paddingLength, 0);
  }
}
// BEGIN_REQUEST 레코드 생성 및 전송
FCGI_Header beginRequestHeader = createFastCgiHeader(1, 1, sizeof(FCGI_BeginRequestBody), 0);
FCGI_BeginRequestBody beginRequestBody = createFastCgiBeginRequestBody(0, 0);
sendFastCgiRecord(sockfd, beginRequestHeader, &beginRequestBody);

// PARAMS 레코드 생성 및 전송
std::string paramsData = "QUERY_STRING=" + requestUri + "\0" + "REQUEST_METHOD=GET\0";
int paramsPaddingLength = (8 - (paramsData.size() % 8)) % 8;
FCGI_Header paramsHeader = createFastCgiHeader(4, 1, paramsData.size(), paramsPaddingLength);
sendFastCgiRecord(sockfd, paramsHeader, paramsData.c_str());

// PARAMS 종료 레코드 전송
FCGI_Header emptyParamsHeader = createFastCgiHeader(4, 1, 0, 0);
sendFastCgiRecord(sockfd, emptyParamsHeader, NULL);

// STDIN 레코드 전송 (GET 요청이므로 비어있음)
FCGI_Header emptyStdinHeader = createFastCgiHeader(5, 1, 0, 0);
sendFastCgiRecord(sockfd, emptyStdinHeader, NULL);

// 응답 처리
std::string response;

while (true) {
  // 응답 헤더를 읽습니다.
  FCGI_Header responseHeader;
  int bytesRead = recv(sockfd, &responseHeader, sizeof(responseHeader), 0);
  if (bytesRead <= 0) {
    break;
  }

  // 응답 데이터를 읽습니다.
  int contentLength = (responseHeader.contentLengthB1 << 8) + responseHeader.contentLengthB0;
  char contentBuffer[contentLength];
  bytesRead = recv(sockfd, contentBuffer, contentLength, 0);
  if (bytesRead <= 0) {
    break;
  }

  // 패딩 데이터를 건너뜁니다.
  int paddingLength = responseHeader.paddingLength;
  char paddingBuffer[paddingLength];
  bytesRead = recv(sockfd, paddingBuffer, paddingLength, 0);
  if (bytesRead <= 0) {
    break;
  }

  // 응답 데이터를 처리합니다.
  if (responseHeader.type == 6) {  // STDOUT
    response.append(contentBuffer, contentLength);
  } else if (responseHeader.type == 7) {  // STDERR
    std::cerr.write(contentBuffer, contentLength);
  } else if (responseHeader.type == 3) {  // END_REQUEST
    break;
  }
}

close(sockfd);

return response;

#endif
