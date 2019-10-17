#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "libfas.h"

/* Actions:
    - get a filename
    - open r
    - open rw
    - unlink
    - write
    - close
    - fork
  Data:
    - filenames stack
    - fds stack
*/

struct fd_node {
    int fd;
    struct fd_node* next;
};

struct filename_node {
    char* str;
    struct filename_node* next;
};

struct fd_node* fd_push(struct fd_node* head, int fd) {

    struct fd_node* tmp = malloc(sizeof(struct fd_node));
    tmp->fd = fd;
    tmp->next = head;
    return tmp;

}

struct filename_node* filename_push(struct filename_node* head, char* str) {

    struct filename_node* tmp = malloc(sizeof(struct filename_node));
    tmp->str = str;
    tmp->next = head;
    return tmp;

}

struct fd_node* fd_pop(struct fd_node *head) {

    struct fd_node* tmp = head;
    head = head->next;
    free(tmp);
    return head;

}

struct filename_node* filename_pop(struct filename_node *head) {

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

#define NUM_ACTIONS 7

static char* A_get_filename() {

  return filenames_roster[rand() % FILENAMES_ROSTER_SIZE];
  
}

static int A_open_rdonly(char* filename) {

  return fas_open(filename, O_RDONLY, 0);

}

static int A_open_rw(char* filename) {

  return fas_open(filename, O_RDWR | O_CREAT, 0);

}

static int A_unlink(char* filename) {

  return unlink(filename);

}

#define MAX_WRITE_SIZE 1000
static int A_write(int fd) {

  size_t size = rand() % MAX_WRITE_SIZE;
  char* buf = malloc(size);
  int r = write(fd, buf, size);
  free(buf);
  return r;

}

static int A_close(int fd) {

  return close(fd);

}

static int fork_depth = 6;

static int A_fork() {

  fork_depth--;
  
  if (fork_depth > 0)
    return fork();

  return -1;
}


void do_action(int actid, struct fd_node** fd_stack, struct filename_node** filenames_stack) {

  fprintf(stderr, "action: %d\n", actid);

  switch(actid) {
  
    case 0: {
    
      *filenames_stack = filename_push(*filenames_stack, A_get_filename());
      break;
    
    }
    case 1: {
    
      if (!(*filenames_stack)) break;
      char* fn = (*filenames_stack)->str;
      *filenames_stack = filename_pop(*filenames_stack);
      int fd = A_open_rdonly(fn);
      *fd_stack = fd_push(*fd_stack, fd);
      break;

    }
    case 2: {
    
      if (!(*filenames_stack)) break;
      char* fn = (*filenames_stack)->str;
      *filenames_stack = filename_pop(*filenames_stack);
      int fd = A_open_rw(fn);
      *fd_stack = fd_push(*fd_stack, fd);
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
      A_write(fd);
      break;

    }
    case 5: {
    
      if (!(*fd_stack)) break;
      int fd = (*fd_stack)->fd;
      *fd_stack = fd_pop(*fd_stack);
      A_close(fd);
      break;

    }
    case 6: {
    
      A_fork();
      break;

    }
      
  }

}

void fuzz() {

  srand(time(NULL));
  struct fd_node* fd_stack = NULL;
  struct filename_node* filenames_stack = NULL;
  
  int i = 0;
  while (i < 1000) {
    do_action(rand() % NUM_ACTIONS, &fd_stack, &filenames_stack);
    ++i;
  }

}
