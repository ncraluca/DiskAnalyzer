#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<errno.h>
#include<sys/statfs.h>
#include<sys/types.h>
#include<pthread.h>
#include<fts.h>

#ifndef CONSTANTS_H_INCLUDED
#define CONSTANTS_H_INCLUDED

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

pthread_mutex_t mutex_lock, mutex_lock_direct;

//a sort of a hash table association between a file and its data
struct file{
    int id; //ino number associated in fts 
    void *val; //type of file
    struct file *next; //pointer to the next file/directory following in a ierarchy
};

struct directory {
    int size;
    struct file **content;
};
struct thread {
    int id; //job ID;   
    int priority;
    pthread_t; *thr;
    char* status;
    int no_file, no_dirs, no_all_dirs;
    struct thread *next;
};
struct directory *dir;
struct thread **threads_list_head;

#endif