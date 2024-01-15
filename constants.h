#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED
#include <pthread.h>
#define HELP "0"
#define ADD "1"
#define SUSPEND "2"
#define RESUME "3"
#define REMOVE "4"
#define INFO "5"
#define LIST "6"
#define PRINT "7"

#define INSTR_LENGTH 100

const char *daemon_pid = "/tmp/disk-analyzer/daemon.pid";
const char *da_pid = "/tmp/disk-analyzer/da.pid";
const char *input_from_user = "/tmp/disk-analyzer/input.txt";
const char *output_from_daemon_prefix = "/tmp/disk-analyzer/output";
const char *output_from_daemon = "/tmp/disk-analyzer/output.txt";
const char *debug_daemon = "/tmp/disk-analyzer/debug.txt";

struct file_directory {
  int id;
  void *fd_path;
  struct file_directory *next;
};

struct thread_details {
  char *path;
  int priority;
  int id;
};

// o structura de tipul lungime lista cu elemente de tip fd_node, lista
// cu toate elementele fd_node.
struct directory {
  int size;
  struct file_directory **content;
};

struct thread_node {
  int id;
  int priority;
  pthread_t *thr;
  char *status;
  int no_files, no_dirs, no_all_dirs;
  struct thread_node *next;
};
// capul listei de threaduri
struct thread_node **threads_head;
#endif
