#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

char *message[5] = {"[^-^]", "[T_T]", "[*.*]", "[-_-]", "quit"};

struct msgbuf {
  long mtype;
  char mtext[64];
};

int main() {
  srand((unsigned int)time(NULL));
  key_t key = ftok(".", rand() % 1000);
  int msgid = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
  assert(msgid >= 0);
  pid_t pid = fork();
  assert(pid >= 0);
  if (pid == 0) {
    printf("I'm child process B.\n");
    struct msgbuf buf;
    while (1) {
      memset(buf.mtext, 0, sizeof(buf.mtext));
      msgrcv(msgid, &buf, 64, 1, 0);
      printf("B reveive from A:   %s\n", buf.mtext);
      if (strncmp(buf.mtext, "quit", 4) == 0) {
        break;
      }
      sleep(1);
      memset(buf.mtext, 0, sizeof(buf.mtext));
      buf.mtype = 2;
      strcpy(buf.mtext, message[rand() % 5]);
      printf("B send to A:        %s\n", buf.mtext);
      msgsnd(msgid, &buf, strlen(buf.mtext), 0);
      if (strncmp(buf.mtext, "quit", 4) == 0) {
        break;
      }
    }
  } else {
    printf("I'm parent process A.\n");
    struct msgbuf buf;
    while (1) {
      memset(buf.mtext, 0, sizeof(buf.mtext));
      buf.mtype = 1;
      strcpy(buf.mtext, message[rand() % 5]);
      printf("A send to B:        %s\n", buf.mtext);
      msgsnd(msgid, &buf, strlen(buf.mtext), 0);
      if (strncmp(buf.mtext, "quit", 4) == 0) {
        wait(NULL);
        break;
      }
      sleep(1);
      memset(buf.mtext, 0, sizeof(buf.mtext));
      msgrcv(msgid, &buf, 64, 2, 0);
      printf("A reveive from B:   %s\n", buf.mtext);
      if (strncmp(buf.mtext, "quit", 4) == 0) {
        wait(NULL);
        break;
      }
    }
  }
}