#include <errno.h>
#include <fts.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED
#define MAX_PID_LENGHT 10

#define HELP "0"
#define ADD "1"
#define SUSPEND "2"
#define RESUME "3"
#define REMOVE "4"
#define INFO "5"
#define LIST "6"
#define PRINT "7"

#define INSTR_LENGTH 100
#define RESULT_SIZE 1000000

const char *daemon_pid_path = "/tmp/disk-analyzer/daemon.pid";
const char *da_pid_path = "/tmp/disk-analyzer/da.pid";
const char *input_from_user = "/tmp/disk-analyzer/input.txt";
const char *output_from_daemon_prefix = "/tmp/disk-analyzer/output";
const char *output_from_daemon = "/tmp/disk-analyzer/output.txt";
const char *debug_daemon = "/tmp/disk-analyzer/debug.txt";
int task_id;
pthread_mutex_t mutex_lock, mutex_lock_directory;

// a sort of a hash table association between a file and its data
struct file_directory {
  int id;        // ino number associated in fts
  void *fd_path; // file_directory path
  struct file_directory
      *next; // pointer to the next file/directory following in a ierarchy
};

struct directory {
  int size;
  struct file_directory **content;
};
struct thread_details {
  char *path;
  int priority;
  int id;
};
struct thread_node {
  int id; // job ID;
  int priority;
  pthread_t *thr;
  char *status;
  int no_file, no_dirs, no_all_dirs;
  struct thread_node *next;
};

struct directory *dir;
struct thread_node **threads_head;

#endif
