
#pragma once

#include "ClientEventHandler.hpp"
#include "EventHandler.hpp"

class FileEventHandler : public EventHandler {
public:
  FileEventHandler(ClientEventHandler &clientEventHandler) : mBuffer(clientEventHandler.mBuffer) {}

  //   FileEventHandler(int serverSocket, int clientSocket, int cgiSocket) {
  //     mServerSocket = serverSocket;
  //     mClientSocket = clientSocket;
  //     mCgiSocket = cgiSocket;
  //   }
  void read() {
    // read from file
    mBuffer.append("file read\n");
  }
  virtual void write() { cout << "file write" << endl; }
  void close() {}

private:
  int mServerSocket;
  int mClientSocket;
  int mCgiSocket;
  string mRequest;
  string mResponse;
  string &mBuffer;
};
