#pragma once

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "HttpConfig.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "UData.hpp"

#define READ 0
#define WRITE 1

class EventHandler {
 public:
  EventHandler() {}
  ~EventHandler() {}

  void handle(int kq, struct kevent &event) {
    UData *udata = (UData *)event.udata;

    if (event.filter == EVFILT_TIMER && udata->cgiPid) {
      cout << "cgi timeout!!!!!!!!!!!!!!!!!!" << endl;
      close(udata->cgiFd[READ]);
      kill(udata->cgiPid, SIGKILL);
      close(udata->serverFd);
      close(udata->clientFd);  // todo: 에러페이지 리턴
      delete udata;
      return;
    } else if (event.filter == EVFILT_TIMER) {
      cout << "timeout: " << udata->clientFd << endl;
      close(udata->serverFd);
      close(udata->clientFd);  // todo: 에러페이지 리턴
      delete udata;
      return;
    }

    switch (udata->ioEventState) {
      case ConnectClient:
        connectClientEventHandler(kq, event);
        break;
      case ReadClient:
        readClientEventHandler(kq, event);
        break;
      case ReadFile:
        readFileEventHandler(kq, event);
        break;
      case ReadCgi:
        readCgiEventHandler(kq, event);
        break;
      case WriteClient:
        writeClientEventHandler(kq, event);
        break;
      case WriteFile:
        writeFileEventHandler(kq, event);
        break;
      case WriteCgi:
        writeCgiEventHandler(kq, event);
        break;
      case CloseSocket:
        closeSocketEventHandler(kq, event);
        break;
      case Error:
        errorHandler(kq, event);
        break;
    }
  };

 private:
  void connectClientEventHandler(int kq, struct kevent &event) {
    cout << "connectClientEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    ServerConfig *serverConfig = udata->serverConfig;

    cout << "serverFd: " << udata->serverFd << endl;
    int clientFd = accept(udata->serverFd, NULL, NULL);
    cout << "clientFd: " << clientFd << endl;

    if (clientFd == -1) {
      // cout << "accept error" << endl;
      perror("accept error");
      return;
    }

    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    UData *newUdata = new UData(-1, clientFd, serverConfig, ReadClient);  // serverConfig 상속
    struct kevent readEvent;

    // Set up read event
    EV_SET(&readEvent, clientFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, newUdata);
    kevent(kq, &readEvent, 1, NULL, 0, NULL);

    setTimer(kq, newUdata);
  }

  void readClientEventHandler(int kq, struct kevent &event) {
    cout << "readClientEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    char buffer[event.data];
    int readSize = read(udata->clientFd, buffer, event.data);

    setTimer(kq, udata); /* 타이머 초기화 */

    // cout << udata->clientFd << " " << string(buffer, readSize) << endl;
    udata->request.append(buffer, readSize);
    cout << string(buffer, readSize) << endl;
    if (!udata->request.isComplete())
      return; /* Todo: header, body size 체크 및 body 계속 받을 수 있게 하기 data를 사용해야할듯*/

    deleteTimer(kq, udata); /* 타이머 삭제 */

    EV_SET(&event, udata->clientFd, EVFILT_READ, EV_DELETE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);

    udata->ioEventState = routingRequest(udata);
    fcntl(udata->serverFd, F_SETFL, O_NONBLOCK);

    // 파일 또는 CGI 이벤트 등록
    // struct kevent event;
    if (udata->ioEventState == ReadFile || udata->ioEventState == ReadCgi) {
      EV_SET(&event, udata->serverFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);
      if (udata->ioEventState == ReadCgi) setTimer(kq, udata);
    } else if (udata->ioEventState == WriteFile || udata->ioEventState == WriteCgi) {
      EV_SET(&event, udata->serverFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);
    } else if (udata->ioEventState == Error || udata->ioEventState == WriteClient) {
      EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);
    }
    cout << strerror(errno) << endl;
  }

  void readFileEventHandler(int kq, struct kevent &event) {
    cout << "readFileEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    char buffer[1024];
    int readSize = read(udata->serverFd, buffer, 1024);  // NON_BLOCK
    cout << readSize << endl;
    if (readSize == 1024) udata->response.append(buffer, readSize);

    // cout << udata->response << endl;
    else {
      udata->response.append(buffer, readSize);
      udata->ioEventState = WriteClient;

      struct kevent event;
      EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);

      close(udata->serverFd);
    }
  }

  // readFileEventHandler와 동일
  void readCgiEventHandler(int kq, struct kevent &event) {
    cout << "readCgiEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    intptr_t data = event.data;
    char buffer[data];
    int readSize = read(udata->serverFd, buffer, data);  // NON_BLOCK

    setTimer(kq, udata); /* 타이머 초기화 */

    cout << readSize << endl;
    // readSize = 0;

    // if (readSize > 0)
    udata->response.append(buffer, readSize);

    if (event.flags & EV_EOF) {
      deleteTimer(kq, udata); /* 타이머 삭제 */
      kill(udata->cgiPid, SIGKILL);
      udata->ioEventState = WriteClient;

      EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
      kevent(kq, &event, 1, NULL, 0, NULL);

      close(udata->serverFd);
    }
  }

  void writeClientEventHandler(int kq, struct kevent &event) {
    cout << "writeClientEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    string mResponse = udata->response.getResponse();

    int x = write(udata->clientFd, mResponse.c_str(), mResponse.size());
    if (x < 0) {
      cout << "write error" << endl;
    }

    udata->ioEventState = CloseSocket;
  }

  // Post 요청일 때
  void writeFileEventHandler(int kq, struct kevent &event) {
    cout << "writeFileEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    write(udata->serverFd, udata->request.getBody().c_str(), udata->request.getBody().size());
    close(udata->serverFd);

    EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);

    udata->ioEventState = WriteClient;
  }

  void writeCgiEventHandler(int kq, struct kevent &event) {
    cout << "writeCgiEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    intptr_t data = event.data;
    string &body = udata->request.getBody();

    if (body.size() > data) {
      write(udata->serverFd, body.c_str(), data);
      body = body.substr(data);
      return;
    }
    cout << udata->serverFd << endl;
    write(udata->serverFd, body.c_str(), body.size());
    cout << "write end" << endl;
    body.clear();

    // close(udata->cgiFd[1]);

    EV_SET(&event, udata->serverFd, EVFILT_WRITE, EV_DELETE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);

    udata->serverFd = udata->cgiFd[0];

    EV_SET(&event, udata->serverFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);

    udata->ioEventState = ReadCgi;
  }

  void closeSocketEventHandler(int kq, struct kevent &event) {
    cout << "closeSocketEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    close(udata->clientFd);
    delete udata;
  }

  void errorHandler(int kq, struct kevent &event) {
    UData *udata = (UData *)event.udata;
    cout << "error handler" << endl;
    string mResponse =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 15\r\n\r\n"
        "404 Not Found\r\n";

    // 에러 파일이 있으면 그 파일 보내주고, 없으면 우리가 지정한 에러 메시지 보내주기
    write(udata->clientFd, mResponse.c_str(), mResponse.size());
    udata->ioEventState = CloseSocket;
  }

 private:
  void setTimer(int kq, UData *udata) {
    struct kevent timerEvent;
    int timer_interval_ms = 5000;
    EV_SET(&timerEvent, udata->clientFd, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, timer_interval_ms, udata);
    kevent(kq, &timerEvent, 1, NULL, 0, NULL);
  }

  void deleteTimer(int kq, UData *udata) {
    struct kevent timerEvent;
    EV_SET(&timerEvent, udata->clientFd, EVFILT_TIMER, EV_DELETE, 0, 0, udata);
    kevent(kq, &timerEvent, 1, NULL, 0, NULL);
  }

  eIoEventState routingRequest(UData *udata) {
    string uri = udata->request.getUri();

    string method = udata->request.getMethod();
    ServerConfig *serverConfig = udata->serverConfig;

    // cout << "uri: " << uri << endl;
    // cout << "method: " << method << endl;

    Trie &trie = serverConfig->locationConfigTrie;

    pair<void *, string> result = trie.search(uri.c_str());
    LocationConfig *locationConfig = (LocationConfig *)result.first;

    if (locationConfig == NULL) {
      udata->response.setStatusCode(404);
      return Error;
    }

    // 메소드가 있는지 확인
    vector<std::string> &limitExceptList = locationConfig->limitExceptList;
    vector<std::string>::iterator it = find(limitExceptList.begin(), limitExceptList.end(), method);
    if (it == limitExceptList.end()) {
      udata->response.setStatusCode(405);
      return Error;
    }

    /* !Todo: path를 갈아끼우는 것으로 대체해야함. */
    string path = locationConfig->rootPath + result.second;
    cout << "path: " << path << endl;

    // 파일이 .py로 끝나면 cgi로 처리
    if (path.find(".py") != string::npos) {
      if (access(path.c_str(), F_OK) == -1) {
        udata->response.setStatusCode(404);
        return Error;
      }
      if (method == "GET") {
        cout << "cgi get" << endl;
        // read pipe하고, 읽기 부분 non_block하고, fork뜨고 안쓰는 부분 닫고, dup2때리고, execve에 query_string 넣어주면
        // 될듯? read를 이벤트 등록까지
        int fd[2];
        pipe(fd);
        fcntl(fd[0], F_SETFL, O_NONBLOCK);
        udata->cgiPid = fork();
        if (udata->cgiPid == 0) {
          close(fd[0]);
          string query_string = udata->request.getQueryString();
          query_string = "QUERY_STRING=" + query_string;
          dup2(fd[1], 1);
          char *argv[] = {(char *)path.c_str(), NULL};
          char *envp[] = {(char *)query_string.c_str(), NULL};
          execve(argv[0], argv, envp);
        } else {
          close(fd[1]);
          udata->serverFd = fd[0];
        }
        return ReadCgi;
      } else if (method == "POST") {
        cout << "post@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << endl;
        // read, write 파이프 만들고, read, write 논블럭으로 하니씩 바꾸고, fork 뜨고, 안쓰는 부분 닫고, dup2때리고,
        // execve에 content-length, query_string 넣어주고, execve하고 read, write 이벤트 등록
        int toCgi[2];
        int fromCgi[2];

        pipe(toCgi);
        pipe(fromCgi);

        int parent[2] = {fromCgi[READ], toCgi[WRITE]};
        int child[2] = {toCgi[READ], fromCgi[WRITE]};

        fcntl(parent[0], F_SETFL, O_NONBLOCK);
        fcntl(parent[1], F_SETFL, O_NONBLOCK);

        udata->cgiPid = fork();
        if (udata->cgiPid == 0) {
          // child
          close(parent[0]);
          close(parent[1]);
          dup2(child[0], 0);
          dup2(child[1], 1);

          string content_length = udata->request.getHeader("Content-Length");
          content_length = "CONTENT_LENGTH=" + content_length;
          string query_string = udata->request.getQueryString();
          query_string = "QUERY_STRING=" + query_string;
          cout << "query_string: " << query_string << endl;
          char *argv[] = {(char *)path.c_str(), NULL};
          char *envp[] = {(char *)query_string.c_str(), (char *)content_length.c_str(), NULL};
          execve(argv[0], argv, envp);
        } else {
          // parent
          close(child[0]);
          close(child[1]);
          udata->cgiFd[0] = parent[0];
          udata->cgiFd[1] = parent[1];
          udata->serverFd = udata->cgiFd[1];
        }

        udata->response.setStatusCode(200);
        return WriteCgi;
      } else {
        udata->response.setStatusCode(405);
        return Error;
      }
    }

    if (path.find(".php") != string::npos) {
      if (method == "GET") {
        udata->response.setStatusCode(200);
        return WriteCgi;
      } else if (method == "POST") {
        udata->response.setStatusCode(200);
        return WriteCgi;
      } else {
        udata->response.setStatusCode(405);
        return Error;
      }
    }

    // 2. redirect 가 있으면 redirect 처리
    if (!locationConfig->returnRedirectList.empty()) { /* !Todo: Redirect 처리 */
      cout << "redirect" << endl;
      udata->response.setStatusCode(atoi(locationConfig->returnRedirectList[0].c_str()));
      udata->response.setLocation(locationConfig->returnRedirectList[1]);
      return WriteClient;
    }

    // 3. 없으면 그냥 보내고,,,

    /* cgi 인지 확인 */
    cout << "method: " << method << endl;
    if (method == "GET") {
      cout << "path.c_str(): " << path.c_str() << endl;
      errno = 0;
      udata->serverFd = open(path.c_str(), O_RDWR);
      cout << "errno: " << errno << endl;
      if (errno == EISDIR) { /* Todo: Index 파일 체크 + autoindex 확인 */
        if (!locationConfig->indexList.empty()) {
          for (int i = 0; i < locationConfig->indexList.size(); i++) {
            string &index = locationConfig->indexList[i];
            string index_path = path + '/' + index;
            cout << "index_path: " << index_path << endl;
            udata->serverFd = open(index_path.c_str(), O_RDWR);
            if (udata->serverFd != -1) return ReadFile;
          }
        } else if (locationConfig->isAutoIndex) {
          udata->response.setStatusCode(200);
          DIR *dir = opendir(path.c_str());
          struct dirent *ent;
          string body = "";
          while ((ent = readdir(dir)) != NULL) {
            body += ent->d_name;
            if (ent->d_type == DT_DIR) body += "/";
            body += "<br>";
          }
          udata->response.setBody(body);
          closedir(dir);

          return WriteClient;
        } else {
          udata->response.setStatusCode(403);
          return Error;
        }
      } else if (errno == ENOENT) {
        udata->response.setStatusCode(404);
        errno = 0;
        // cout << "404 Not Found" << endl;
        return Error;
      }
      return ReadFile;
    } else if (method == "POST") {
      udata->serverFd = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
      if (errno == EISDIR) {
        udata->response.setStatusCode(403);
        errno = 0;
        return Error;
      }
      return WriteFile;
    } else if (method == "DELETE") {
      if (unlink(path.c_str()) == -1) {
        udata->response.setStatusCode(404);
        cout << "DELETE ERROR" << endl;
        return Error;
      }
      return WriteClient;
    }

    return ReadFile;
  }

  int createCgiSocket(std::string &fastCgiPass) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
      cout << "socket error" << endl;
      return -1;
    }

    cout << "serverFd: " << serverFd << endl;

    // fastCgiPass = 127.0.0.1:9000
    std::string ip = fastCgiPass.substr(0, fastCgiPass.find(":"));
    std::string port = fastCgiPass.substr(fastCgiPass.find(":") + 1, fastCgiPass.size());
    cout << ip << endl;
    cout << port << endl;

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port.c_str()));
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (connect(serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
      cout << "connect error" << endl;
      close(serverFd);
      return -1;
    }
    cout << "connect success" << endl;
    fcntl(serverFd, F_SETFL, O_NONBLOCK);

    return serverFd;
  }
};
