#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "libfas.h"

int main() {

  int fd = open("/tmp/pippo.txt", O_CREAT | O_WRONLY, 0777);
  write(fd, "pippo", 5);
  close(fd);

  fas_init();

  fd = fas_open("/tmp/pippo.txt", O_RDWR);
  printf("fd: %d\n", fd);

  char buf[32] = {0};
  read(fd, buf, 5);

  printf("buf: %s\n", buf);

  int r = write(fd, " <3\n", 4);
  printf("r: %d\n", r);

  return 0;

}

