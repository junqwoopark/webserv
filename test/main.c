#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <unistd.h>

/*
#!/usr/bin/env python3

import os, sys

len = os.getenv('CONTENT_LENGTH')
query = sys.stdin.read(int(len))

print("Content-type: text/html\r\n\r\n")
print("<html><head><title>CGI Test</title></head><body>")
print("<h1>CGI Test</h1>")
print("<p>Query: %s</p>" % query)
print("</body></html>")

*/

// int main() {
//   int pid = fork();
//   if (pid == 0) {
//     if (execve("./cgi.py", NULL, NULL) == -1) {
//       perror("execve");
//       exit(EXIT_FAILURE);
//     }
//   }
//   else {
//     write
//   }

// example for kqueue read event data

int main() {
  int p[2];

  pipe(p);
  ;
  // close(p[0]);
  ;
  int fd = p[1];
  int kq = kqueue();
  struct kevent event;
  EV_SET(&event, fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
  kevent(kq, &event, 1, NULL, 0, NULL);
  struct kevent event2;
  kevent(kq, NULL, 0, &event2, 1, NULL);

  printf("event2.flags: %d\n", event2.flags);
  printf("event2.fflags: %d\n", event2.fflags);
  printf("event2.data: %d\n", event2.data);
}
