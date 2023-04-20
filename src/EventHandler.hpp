#pragma once

#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "HttpConfig.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "String.hpp"
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
      throw "500";
    } else if (event.filter == EVFILT_TIMER) {
      throw "408";
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
        writeClientEventHandler(event);
        break;
      case WriteFile:
        writeFileEventHandler(kq, event);
        break;
      case WriteCgi:
        writeCgiEventHandler(kq, event);
        break;
      case CloseSocket:
        closeSocketEventHandler(event);
        break;
      default:
        break;
    }
  };

  void handleError(int kq, struct kevent &event, string errorCode) {
    cout << "handleError" << errorCode << endl;
    UData *udata = (UData *)event.udata;

    string &root = udata->serverConfig->rootPath;
    vector<string> &errorPageList = udata->serverConfig->errorPageList;

    for (size_t i = 0; i < errorPageList.size(); i++) {
      if (errorPageList[i].substr(0, errorCode.length()) == errorCode) {
        string errorFilePath = root + '/' + errorPageList[i].substr(errorCode.length() + 1);
        cout << "errorFilePath: " << errorFilePath << endl;
        int errorFileFd = open(errorFilePath.c_str(), O_RDONLY);
        if (errorFileFd < 0) continue;

        udata->serverFd = errorFileFd;
        udata->ioEventState = ReadFile;
        EV_SET(&event, udata->serverFd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, udata);
        kevent(kq, &event, 1, NULL, 0, NULL);

        struct stat fileStat;
        fstat(udata->serverFd, &fileStat);
        udata->readFileSize = fileStat.st_size;
        return;
      }
    }

    udata->response.setError(atoi(errorCode.c_str()));
    udata->ioEventState = WriteClient;

    EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);
  }

 private:
  void connectClientEventHandler(int kq, struct kevent &event) {
    cout << "connectClientEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    ServerConfig *serverConfig = udata->serverConfig;

    int clientFd = accept(udata->serverFd, NULL, NULL);

    if (clientFd == -1) {
      perror("accept error");
      return;
    }

    fcntl(clientFd, F_SETFL, O_NONBLOCK);
    UData *newUdata = new UData(-1, clientFd, serverConfig, ReadClient);  // serverConfig 상속
    struct kevent readEvent;

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

    udata->request.append(buffer, readSize, udata->serverConfig->maxClientBodySize);
    cout << string(buffer, readSize) << endl;
    if (!udata->request.isComplete()) return;

    deleteTimer(kq, udata); /* 타이머 삭제 */

    EV_SET(&event, udata->clientFd, EVFILT_READ, EV_DELETE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);

    udata->ioEventState = routingRequest(udata);
    fcntl(udata->serverFd, F_SETFL, O_NONBLOCK);

    if (udata->ioEventState == ReadFile) {
      struct stat fileStat;
      fstat(udata->serverFd, &fileStat);
      udata->readFileSize = fileStat.st_size;
    }

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
  }

  void readFileEventHandler(int kq, struct kevent &event) {
    cout << "readFileEventHandler" << endl;
    UData *udata = (UData *)event.udata;

    char buffer[1024];
    int readSize = read(udata->serverFd, buffer, 1024);

    udata->response.append(buffer, readSize);

    udata->readFileSize -= readSize;

    if (udata->readFileSize == 0) {
      udata->ioEventState = WriteClient;

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

  void writeClientEventHandler(struct kevent &event) {
    cout << "writeClientEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    vector<char> mResponse = udata->response.getResponse();

    size_t responseLength = mResponse.size();

    size_t writeSize = write(udata->clientFd, &mResponse[udata->writeOffset],
                             min(responseLength - udata->writeOffset, (size_t)event.data));

    udata->writeOffset += writeSize;

    if (udata->writeOffset == responseLength) {
      udata->ioEventState = CloseSocket;
    }
  }

  // Post 요청일 때
  void writeFileEventHandler(int kq, struct kevent &event) {
    cout << "writeFileEventHandler" << endl;
    UData *udata = (UData *)event.udata;

    size_t writeSize = write(udata->serverFd, udata->request.getBody().c_str(), udata->request.getBody().size());
    close(udata->serverFd);

    if (writeSize != udata->request.getBody().size()) {
      throw "500";
    }

    EV_SET(&event, udata->clientFd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, udata);
    kevent(kq, &event, 1, NULL, 0, NULL);
    udata->ioEventState = WriteClient;
  }

  void writeCgiEventHandler(int kq, struct kevent &event) {
    cout << "writeCgiEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    intptr_t data = event.data;
    String &body = udata->request.getBody();

    if (body.size() > static_cast<size_t>(data)) {
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

  void closeSocketEventHandler(struct kevent &event) {
    cout << "closeSocketEventHandler" << endl;
    UData *udata = (UData *)event.udata;
    close(udata->clientFd);
    delete udata;
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

    Trie &trie = serverConfig->locationConfigTrie;

    pair<void *, string> result = trie.search(uri.c_str());
    LocationConfig *locationConfig = (LocationConfig *)result.first;

    if (locationConfig == NULL) {
      throw "404";
    }

    // 메소드가 있는지 확인
    vector<std::string> &limitExceptList = locationConfig->limitExceptList;
    vector<std::string>::iterator it = find(limitExceptList.begin(), limitExceptList.end(), method);
    if (it == limitExceptList.end()) throw "405";

    string path = locationConfig->rootPath + result.second;
    cout << "path: " << path << endl;

    // 파일이 .py로 끝나면 cgi로 처리
    if (path.find(".py") != string::npos) {
      if (access(path.c_str(), F_OK) == -1) throw "404";
      if (method == "GET") {
        // read pipe하고, 읽기 부분 non_block하고, fork뜨고 안쓰는 부분 닫고, dup2때리고, execve에 query_string
        // 넣어주면 될듯? read를 이벤트 등록까지
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
        throw "405";
      }
    }

    // ext에 따라 Content-Type 설정
    string ext = path.substr(path.find_last_of(".") + 1);
    udata->response.setContentType(ext);

    // if (path.find(".php") != string::npos) {
    //   if (method == "GET") {
    //     udata->response.setStatusCode(200);
    //     return WriteCgi;
    //   } else if (method == "POST") {
    //     udata->response.setStatusCode(200);
    //     return WriteCgi;
    //   } else {
    //     throw "405";
    //   }
    // }

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
          for (size_t i = 0; i < locationConfig->indexList.size(); i++) {
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
          throw "403";
        }
      } else if (errno == ENOENT) {
        throw "404";
      }
      return ReadFile;
    } else if (method == "POST") {
      udata->serverFd = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
      if (errno == EISDIR) throw "403";
      return WriteFile;
    } else if (method == "DELETE") {
      if (unlink(path.c_str()) == -1) throw "404";

      return WriteClient;
    }

    return ReadFile;
  }
};