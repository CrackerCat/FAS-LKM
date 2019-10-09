#include "libfas.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main() {

  int fd = creat("./pippo.txt", O_WRONLY);
  write(fd, "pippo", 5);
  close(fd);
  
  fas_init();
  
  fd = fas_open("./pippo.txt", O_RDWR, 0);
  printf("fd: %d\n", fd);
  char buf[32] = {0};
  read(fd, buf, 5);

  printf("buf: %s\n", buf);
  return 0;

}
