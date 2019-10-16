#include "libfas.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main() {

  int fd = creat("/tmp/pippo.txt", O_WRONLY);
  write(fd, "pippo", 5);
  close(fd);

  fas_init();

  fd = fas_open("/tmp/pippo.txt", O_RDWR, 0);
  printf("fd: %d\n", fd);
  char buf[32] = {0};
  read(fd, buf, 5);
  
  printf("buf: %s\n", buf);
  
  memset(buf, 0, 32);

  pid_t pid = fork();
  if (pid == 0) {
  
    sleep(3);
    
    fprintf(stderr, "Closing child.\n");
    close(fd);
    
    return 0;
  }

  int r = write(fd, " <3\n", 4);
  printf("r: %d\n", r);

  FILE* fp = fopen("/sys/kernel/fas/sessions_num", "r");
  fread(buf, 1, 32, fp);
  fprintf(stderr, "sessions_num: %s\n", buf);
  fclose(fp);
  
  fprintf(stderr, "Closing parent.\n");
  close(fd);

  return 0;

}

