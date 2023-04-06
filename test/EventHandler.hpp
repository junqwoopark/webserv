#pragma once

#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

#include "Request.hpp"
#include "Response.hpp"

using namespace std;

enum eType { ConnectClient, ReadClient, ReadFile, ReadCgi, WriteClient, WriteFile, WriteCgi, CloseSocket };

class udata {
public:
  udata(int readFd, int writeFd, eType type, string buffer = "") {
    this->readFd = readFd;
    this->writeFd = writeFd;
    this->buffer = buffer;
    this->type = type;
  }

  udata *clone() { return new udata(readFd, writeFd, type); }

  int readFd;
  int writeFd;
  string buffer;
  void *handler;
  eType type;
};

class ClientEventHandler {
public:
  ClientEventHandler() {}

  virtual ~ClientEventHandler() {}

  virtual void read(int kq, udata *udata){};
  virtual void write(int kq, udata *udata){};
  virtual void close(){};
};

// void readClientClientEventHandler(udata *udata) -> post 면 writeFile ,cgi 면 writeCgi, file 이면 readFile
// void readFileClientEventHandler(udata *udata) { -> writeClient
// void readCgiClientEventHandler(udata *udata) { -> writeClient
// void writeClientClientEventHandler(udata *udata) { -> close
// void writeFileClientEventHandler(udata *udata) { -> writeClient
// void writeCgiClientEventHandler(udata *udata) { -> readCgi

void connectClientEventHandler(int kq, udata *_udata) {
  cout << "connectClientEventHandler" << endl;
  int clientFd = accept(_udata->readFd, NULL, NULL);
  udata *newUdata = new udata(clientFd, clientFd, ReadClient);
  struct kevent event;
  EV_SET(&event, clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, newUdata);
  kevent(kq, &event, 1, NULL, 0, NULL);
}

void readClientEventHandler(int kq, udata *udata) {
  cout << "readClientEventHandler" << endl;
  char buffer[1024];
  int readSize = read(udata->readFd, buffer, 1024);
  udata->buffer.append(buffer, readSize);

  if (udata->buffer.find("\r\n\r\n") == string::npos)
    return;

  Request request(udata->buffer);
  request.parse();

  // 라우팅 후 new fd 찾아오기 -> 여기서 cgi 면 cgiToClientClientEventHandler, file이면 fileToClientClientEventHandler
  // 해줘야함. if (cgi라면) {
  //   커넥션 만들고, udata->writeFd = cgiFd; udata->type = ClientToCgi;
  //   struct kevent event;
  //   EV_SET(&event, udata->writeFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
  //   kevent(kq, &event, 1, NULL, 0, NULL);
  //} else if (file이라면 && post라면) {
  //   udata->writeFd = fileFd; udata->type = ClientToFile;
  //   struct kevent event;
  //   EV_SET(&event, udata->writeFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
  //   kevent(kq, &event, 1, NULL, 0, NULL);
  //} else if (file이라면 && get이라면) {
  //   udata->readFd = fileFd; udata->type = ClientToFile;
  //  struct kevent event;
  //  EV_SET(&event, udata->readFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
  //  kevent(kq, &event, 1, NULL, 0, NULL);
  //}

  udata->type = ReadFile;

  // 이벤트 등록
  struct kevent event;
  EV_SET(&event, udata->writeFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
  kevent(kq, &event, 1, NULL, 0, NULL);
}

void readFileEventHandler(int kq, udata *udata) {
  udata->type = WriteClient;
  struct kevent event;
  EV_SET(&event, udata->writeFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
  kevent(kq, &event, 1, NULL, 0, NULL);
}

void readCgiEventHandler(int kq, udata *udata) {}

void writeClientEventHandler(int kq, udata *udata) {
  string mResponse = "HTTP/1.1 200 OK\r\n"
                     "Content-Type: text/html\r\n"
                     "Content-Length: 12\r\n"
                     "\r\n"
                     "Hello World!";

  write(udata->writeFd, mResponse.c_str(), mResponse.size());
  udata->type = CloseSocket;
}
void writeFileEventHandler(int kq, udata *udata) {}
void writeCgiEventHandler(int kq, udata *udata) {}
void closeSocketEventHandler(int kq, udata *udata) { close(udata->writeFd); }
