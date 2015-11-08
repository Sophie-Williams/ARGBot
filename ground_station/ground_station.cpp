#include <unistd.h>
#include "httplink.h"
#include <signal.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>

static int stopsig;

void stopprog(int x) {
  stopsig = 1;
}

int main() {
  signal(SIGINT, stopprog);
  int pid1 = fork();
  if (pid1 == 0) {
    // this is the oculus rift single camera process
    execlp("./single_cam", "./single_cam", NULL);
    return 1;
  }
  int pid2 = fork();
  if (pid2 == 0) {
    // this is the robot camera stream
    execlp("./get_xBot.sh", "./get_xBot.sh", NULL);
    kill(pid1, SIGINT);
    return 1;
  }
  int pid3 = fork();
  if (pid3 == 0) {
    // this is the oculus rift orientation send program?
    return 1;
  }

  while (!stopsig);

  kill(pid1, SIGKILL);
  kill(pid2, SIGKILL);
  kill(pid3, SIGKILL);

  return 0;
}
