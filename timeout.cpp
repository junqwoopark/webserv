#include <stdio.h>
#include <stdlib.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>

int main() {
  int kq, nev;
  struct kevent change;
  struct kevent event;
  struct timespec timeout;

  // kqueue 생성
  kq = kqueue();
  if (kq == -1) {
    perror("kqueue");
    exit(EXIT_FAILURE);
  }

  // 이벤트 등록
  EV_SET(&change, 1, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, 5000, 0);

  // 이벤트 대기
  timeout.tv_sec = 5;
  timeout.tv_nsec = 0;
  nev = kevent(kq, &change, 1, &event, 1, NULL);
  if (nev == -1) {
    perror("kevent");
    exit(EXIT_FAILURE);
  } else if (nev == 0) {
    printf("timeout\n");
  } else {
    if (event.flags & EV_ERROR) {
      printf("EV_ERROR: 에러임\n");
      exit(EXIT_FAILURE);
    }

    if (event.flags & EVFILT_TIMER) {
      printf("EVFILT_TIMER\n");
    }
  }

  return 0;
}

// #include <sys/event.h>
// #include <sys/time.h>
// #include <sys/types.h>
// #include <unistd.h>
//
// #include <iostream>
//
// int main() {
//   int kq = kqueue();
//   if (kq == -1) {
//     std::cerr << "Failed to create kqueue." << std::endl;
//     return 1;
//   }
//
//   struct kevent event;
//   int timer_id = 1;
//   int timer_interval_ms = 5000;  // 5 seconds
//
//   EV_SET(&event, timer_id, EVFILT_TIMER, EV_ADD, 0, timer_interval_ms, NULL);
//
//   if (kevent(kq, &event, 1, NULL, 0, NULL) == -1) {
//     std::cerr << "Failed to add timer event." << std::endl;
//     return 1;
//   }
//
//   struct kevent triggered_event;
//
//   while (true) {
//     int num_triggered_events = kevent(kq, NULL, 0, &triggered_event, 1, NULL);
//
//     if (num_triggered_events > 0) {
//       std::cout << "Timer event triggered!" << std::endl;
//       break;
//     }
//   }
//
//   close(kq);
//   return 0;
// }
//
