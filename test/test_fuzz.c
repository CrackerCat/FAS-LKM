#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "libfas.h"

/* Actions:
    - get a filename
    - open r
    - open rw
    - unlink
    - write
    - close
    - open a sysfs fd
  Data:
    - filenames stack
    - fds stack
*/

// undefine thsi to run FUZZ_ITERATIONS times only
#define FUZZ_FOREVAH 1

#define THREADS_NUM 8
#define FUZZ_ITERATIONS 1000000

struct fd_node {

  int             fd;
  int             is_w;
  struct fd_node* next;

};

struct filename_node {

  char*                 str;
  int                   is_w;
  struct filename_node* next;

};

struct fd_node* fd_push(struct fd_node* head, int fd, int is_w) {

  struct fd_node* tmp = malloc(sizeof(struct fd_node));
  tmp->fd = fd;
  tmp->is_w = is_w;
  tmp->next = head;
  return tmp;

}

struct filename_node* filename_push(struct filename_node* head, char* str,
                                    int is_w) {

  struct filename_node* tmp = malloc(sizeof(struct filename_node));
  tmp->str = str;
  tmp->is_w = is_w;
  tmp->next = head;
  return tmp;

}

struct fd_node* fd_pop(struct fd_node* head) {

  struct fd_node* tmp = head;
  head = head->next;
  free(tmp);
  return head;

}

struct filename_node* filename_pop(struct filename_node* head) {

  struct filename_node* tmp = head;
  head = head->next;
  free(tmp);
  return head;

}

#define FILENAMES_ROSTER_SIZE 6
static char* filenames_roster[FILENAMES_ROSTER_SIZE] = {

    "/tmp/fuzzy_fas_0",
    "/tmp/fuzzy_fas_1",
    "/tmp/fuzzy_fas_2",
    "/tmp/fuzzy_fas_3",
    "/tmp/fuzzy_fas_4",
    "/tmp/fuzzy_fas_5"

};

#define SYSFS_FILENAMES_ROSTER_SIZE 3
static char* sysfs_filenames_roster[SYSFS_FILENAMES_ROSTER_SIZE] = {

    "/sys/kernel/fas/sessions_num", "/sys/kernel/fas/sessions_each_file",
    "/sys/kernel/fas/processes"

};

static char* A_get_filename() {

  return filenames_roster[rand() % FILENAMES_ROSTER_SIZE];

}

static char* A_get_sysfs_filename() {

  return sysfs_filenames_roster[rand() % SYSFS_FILENAMES_ROSTER_SIZE];

}

static int A_open_rdonly(char* filename) {

  return fas_open(filename, O_RDONLY);

}

static int A_open_rw(char* filename) {

  return fas_open(filename, O_RDWR);

}

static int A_unlink(char* filename) {

  int r = unlink(filename);
  int fd = open(filename, O_CREAT | O_WRONLY, 0777);
  if (fd > 0) close(fd);
  return r;

}

#define MAX_WRITE_SIZE 12
static int A_write(int fd) {

  size_t size = rand() % MAX_WRITE_SIZE;
  char*  buf = malloc(size);
  int    r = write(fd, buf, size);
  free(buf);
  return r;

}

#define MAX_WRITE_SIZE 12
static int A_read(int fd) {

  size_t size = rand() % MAX_WRITE_SIZE;
  char*  buf = malloc(size);
  int    r = read(fd, buf, size);
  free(buf);
  return r;

}

static int A_close(int fd) {

  return close(fd);

}

void do_action(int actid, struct fd_node** fd_stack,
               struct filename_node** filenames_stack) {

  // fprintf(stderr, "action: %d\n", actid);

  switch (actid) {

    case 0: {

      *filenames_stack = filename_push(*filenames_stack, A_get_filename(), 1);
      break;

    }

    case 1: {

      if (!(*filenames_stack)) break;
      if (!(*filenames_stack)->is_w) goto open_rdonly_action;
      char* fn = (*filenames_stack)->str;
      *filenames_stack = filename_pop(*filenames_stack);
      int fd = A_open_rw(fn);
      A_write(fd);
      *fd_stack = fd_push(*fd_stack, fd, 1);
      break;

    }

    case 2: {

    open_rdonly_action:
      if (!(*filenames_stack)) break;
      char* fn = (*filenames_stack)->str;
      *filenames_stack = filename_pop(*filenames_stack);
      int fd = A_open_rdonly(fn);
      *fd_stack = fd_push(*fd_stack, fd, 0);
      break;

    }

    case 3: {

      if (!(*filenames_stack)) break;
      char* fn = (*filenames_stack)->str;
      *filenames_stack = filename_pop(*filenames_stack);
      A_unlink(fn);
      break;

    }

    case 4: {

      if (!(*fd_stack)) break;
      int fd = (*fd_stack)->fd;
      A_read(fd);
      break;

    }

    case 5 ... 6: {  // more probability to close

      if (!(*fd_stack)) break;
      int fd = (*fd_stack)->fd;
      *fd_stack = fd_pop(*fd_stack);
      A_close(fd);
      break;

    }

    case 7: {

      *filenames_stack =
          filename_push(*filenames_stack, A_get_sysfs_filename(), 0);
      break;

    }

  }

}

#define NUM_ACTIONS 8

void* fuzz(void* __a) {

  srand(time(NULL));
  struct fd_node*       fd_stack = NULL;
  struct filename_node* filenames_stack = NULL;

  int i = 0;
#ifndef FUZZ_FOREVAH
  while (i < FUZZ_ITERATIONS) {

#else
  while (1) {

#endif

    do_action(rand() % NUM_ACTIONS, &fd_stack, &filenames_stack);
    if (i % 1000 == 0) {

      fprintf(stderr, "[%ld] iteration #%d (%p, %p)\n", syscall(__NR_gettid), i,
              fd_stack, filenames_stack);

    }

    ++i;

  }

  return NULL;

}

void pipe_handler(int s) {

  fprintf(stderr, "SIGPIPE\n");

}

int main() {

  fas_init();

  // signal(SIGPIPE, pipe_handler);
  signal(SIGPIPE, SIG_IGN);

  pthread_t threads[THREADS_NUM];

  int i;
  for (i = 0; i < FILENAMES_ROSTER_SIZE; ++i) {

    int fd = open(filenames_roster[i], O_CREAT | O_WRONLY, 0777);
    if (fd < 0) {

      fprintf(
          stderr,
          "You don't have the permissions to write in /tmp, are u kidding?\n");
      return 1;

    }

    close(fd);

  }

  for (i = 0; i < THREADS_NUM; ++i) {

    pthread_create(&threads[i], NULL, &fuzz, NULL);

  }

  for (i = 0; i < THREADS_NUM; ++i) {

    pthread_join(threads[i], NULL);

  }

  return 0;

}

