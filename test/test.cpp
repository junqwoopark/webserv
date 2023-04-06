#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdlib.h>
#include <sys/event.h>
#include <unistd.h>

int main(void) {
  int kq = kqueue();

  int fd = open("main.cpp", O_RDONLY);
  fcntl(fd, F_SETFL, O_NONBLOCK);

  struct kevent event;
  memset(&event, 0, sizeof(event));
  EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

  kevent(kq, &event, 1, NULL, 0, NULL);

  while (true) {
    struct kevent event;
    memset(&event, 0, sizeof(event));
    kevent(kq, NULL, 0, &event, 1, NULL);

    if (event.filter == EVFILT_READ) {
      char buffer[1024];
      int bytesRead = ::read(event.ident, buffer, 1024);
      buffer[bytesRead] = '\0';
      std::cout << buffer << std::endl;
    }
  }
}

#include <fcntl.h>
#include <iostream>
#include <sys/event.h>
#include <unistd.h>

using namespace std;

// int main() {
//   const char *file_path = "main.cpp";

//   // Open the file
//   int fd = open(file_path, O_RDONLY | O_NONBLOCK);
//   if (fd == -1) {
//     std::cerr << "Failed to open the file." << std::endl;
//     return 1;
//   }

//   // Create kqueue instance
//   int kq = kqueue();
//   if (kq == -1) {
//     std::cerr << "Failed to create kqueue." << std::endl;
//     return 1;
//   }

//   // Register the file descriptor with kqueue
//   struct kevent event;
//   EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

//   int ret = kevent(kq, &event, 1, NULL, 0, NULL);
//   if (ret == -1) {
//     std::cerr << "Failed to register the file descriptor with kqueue." << std::endl;
//     return 1;
//   }

//   // Wait for events
//   struct kevent triggered_event;
//   struct timespec timeout = {0, 0}; // 1-second timeout

//   while (true) {
//     int num_events = kevent(kq, NULL, 0, &triggered_event, 1, NULL);
//     if (num_events > 0) {
//       char buffer[1024];
//       ssize_t bytes_read = read(fd, buffer, 1023);
//       buffer[bytes_read] = '\0'; // Null-terminate the string [strncpy doesn't do this
//       if (bytes_read > 0) {
//         cout << buffer << endl;
//         std::cout << "Read " << bytes_read << " bytes from the file." << std::endl;
//       } else {
//         std::cout << "Reached the end of the file." << std::endl;
//         break;
//       }
//     } else {
//       std::cout << "No events triggered." << std::endl;
//     }
//   }

//   close(fd);
//   close(kq);

//   return 0;
// }
