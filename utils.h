#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "constants.h"


int task_id;
pthread_mutex_t mutex_lock, mutex_lock_directory;
struct directory *dir;
struct thread_node **threads_head;

void daemon_message(const char *message) {
  int fd = open(debug_daemon, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU,
                S_IRWXG | S_IRWXO);
  if (fd < 0) {
    perror("Error at opening the debugging file\n");
  }
  write(fd, message, strlen(message));
  close(fd);
}

// fctia get_next_task_id() returneaza un id unic pentru fiecare task - folosita
// in functia Add
int get_next_task_id() { return ++task_id; }

void create_directory(const char *given_path) {
  struct stat st = {0}; // a stat structure gives us info about a file or // directory found at a given_path
  if (stat(given_path, &st) == -1) { // if the file from the path doesn't exist
    mode_t oldmask = umask(0);
    mkdir("/tmp/disk-analyzer",0777); // create the directory with 0777 permission =
    // read(4)+write(2)+execute(1) => to all users
     umask(oldmask);
  }
}

int compare(const FTSENT **one, const FTSENT **two) {
  return (strcmp((*one)->fts_name, (*two)->fts_name));
}

int count_dirs(char *path) {
  int count = 0;
  FTS *file_system = NULL;
  FTSENT *node = NULL;

  char *paths[] = {path, NULL};
  if (!(file_system = fts_open(paths, FTS_COMFOLLOW | FTS_NOCHDIR, &compare))) {
    perror(NULL);
    return errno;
  }
  if (file_system != NULL) {
    while ((node = fts_read(file_system)) != NULL) {
      switch (node->fts_info) {
      case FTS_D:;
        count++;
        break;
      default:
        break;
      }
    }
    fts_close(file_system);
  }
  return count;
}

/// STRUCTURI AICI----------------------------------------------------------------------------------------------
// a sort of a hash table association between a file and its data
struct file_directory {
  int id;        // ino number associated in fts
  void *fd_path; // file_directory path
  struct file_directory *next; // pointer to the next file/directory following in a ierarchy
};

struct directory {
  int size;
  struct file_directory **content;
};

void dir_hash_init(struct directory *m, int size) {
  m->size = size;
  m->content = (struct file_directory **)malloc(size * sizeof(struct file_directory *));
  for (int i = 0; i < size; i++) {
    m->content[i] = NULL;
  }
}

// functia *map_find returneaza nodul (task-ul) cu id-ul key - folosita in
// functia Remove si Info
struct file_directory *dir_hash_find(struct directory *m, int key) {
  int mod = key % m->size; // se parcurge arborele si se cauta nivelul din arbore
  if (m->content[mod] == NULL)
    return NULL;
  for (struct file_directory *nod = m->content[mod]; nod != NULL; nod = nod->next) {
    if (nod->id == key)
      return nod;
  }
  return NULL;
}

// functia find_task returneaza id-ul task-ului cu path-ul path, daca exista, si
// -1 altfel - folosita in functia Add
int find_task(struct directory *m, char *path) {
  // se parcurge lista de taskuri
  for (int i = 0; i < m->size; i++) {
    if (m->content[i] != NULL) { // daca lista are elemente nenule(daca fiecare
                                 // lista fd_node contine macar un element)
      for (struct file_directory *nod = m->content[i]; nod != NULL; nod = nod->next) { // se parcurge lista de fd_node
        if (strcmp((char *)nod->fd_path, path) == 0) // daca path-ul din nod este egal cu path-ul dat ca parametru
          return nod->id; // se returneaza id-ul task-ului (inseamna ca task-ul
                          // exista deja)
      }
    }
  }
  return -1;
}


// functia map_insert insereaza in map un nou task - folosita in functia Add
void dir_hash_insert(struct directory *m, int key, void *val) {
  int mod = key % m->size;
  if (m->content[mod] == NULL) {
    m->content[mod] = (struct file_directory *)malloc(sizeof(struct file_directory *));
    m->content[mod]->fd_path = val;
    m->content[mod]->id = key;
    m->content[mod]->next = NULL;
  } else {
    struct file_directory *nod;
    for (nod = m->content[mod]; nod->next != NULL; nod = nod->next) {
      if (nod->id == key) {
        nod->fd_path = val;
        return;
      }
    }
    if (nod->id == key) {
      nod->fd_path = val;
      return;
    }
    nod->next = (struct file_directory *)malloc(sizeof(struct file_directory));
    nod->next->next = NULL;
    nod->next->id = key;
    nod->next->fd_path = val;
  }
}

// functia map_delete sterge nodul(task-ul) cu id-ul key - folosita in functia
// Remove
void dir_hash_delete(struct directory *m, int id) {
  struct file_directory *nod = (struct file_directory *)malloc(sizeof(struct file_directory));
  nod = m->content[id % m->size];
  struct file_directory *prev = (struct file_directory *)malloc(sizeof(struct file_directory));
  if (nod != NULL && nod->id == id) {
    m->content[id % m->size] = nod->next;
    free(nod);
  } else {
    while (nod != NULL && nod->id != id) {
      prev = nod;
      nod = nod->next;
    }

    if (nod != NULL) {
      prev->next = nod->next;
      free(nod);
    }
  }
}


void clear_directory(struct directory *m) {
  for (int i = 0; i < m->size; i++)
    free(m->content[i]);
  free(m->content);
}

///-----------------STRUCTURA THREAD------------------
struct thread_node {
  int id; // job ID;
  int priority;
  pthread_t *thr;
  char *status;
  int no_file, no_dirs, no_all_dirs;
  struct thread_node *next;
};


void thread_list_init() {
  threads_head = (struct thread_node **)malloc(sizeof(struct thread_node *));
  *threads_head = NULL;
}



void insert_task(struct thread_node **threads_head, int id, int priority, pthread_t *thr) {

  struct thread_node *new_node = (struct thread_node *)malloc(sizeof(struct thread_node));

  new_node->id = id;
  new_node->priority = priority;
  new_node->thr = thr;
  new_node->status = (char *)malloc(30);
  strcpy(new_node->status, "preparing");
  new_node->no_file = 0;
  new_node->no_dirs = 0;
  // find out total number of subdirectories
  struct file_directory *node = dir_hash_find(dir, id);
  char *path = (char *)(node->fd_path);
  new_node->no_dirs = count_dirs(path);
  new_node->next = *threads_head;
  *threads_head = new_node;
}

// functia list_delete sterge threadul cu id-ul key - folosita in functia Remove
void thread_list_delete(struct thread_node **head_ref, int key) {
  struct thread_node *current_thread = (struct thread_node *)malloc(sizeof(struct thread_node));
  current_thread = *threads_head;
  struct thread_node *previous = (struct thread_node *)malloc(sizeof(struct thread_node));

  if (current_thread != NULL && current_thread->id == key) {
    *head_ref = current_thread->next;
    free(current_thread);
  } else {
    while (current_thread != NULL && current_thread->id != key) {
      previous = current_thread;
      current_thread = current_thread->next;
    }

    if (current_thread != NULL) {
      previous->next = current_thread->next;
      free(current_thread);
    }
  }
}

// functia list_find_by_key returneaza threadul cu id-ul key - folosita in
// functia Remove
struct thread_node* find_thread_id(struct thread_node** head_ref, int key) {
  daemon_message("In find_thread_id\n");
  struct thread_node *current_thread = (struct thread_node* )malloc(sizeof(struct thread_node));
  current_thread = *head_ref;
  while (current_thread != NULL) {
    if (current_thread->id == key) {
      return current_thread;
    }
    current_thread = current_thread->next;
  }
  return NULL;
}


struct thread_node *find_thread(struct thread_node **head_ref, pthread_t thr) {
  struct thread_node *current = (struct thread_node *)malloc(sizeof(struct thread_node));
  current = *head_ref;
  while (current != NULL) {
    if (pthread_equal(*(current->thr), thr)) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}


char *progress(float percent) {
  char *res = (char *)malloc(51);
  res = "##################################################";
  int number_of_hashtags = percent * 50 / 100;
  res += (50 - number_of_hashtags);
  return res;
}


char *convert_size(float bytes) {
  char *size = (char *)malloc(20);
  if (bytes >= 1024) { // KB
    bytes /= 1024;
    sprintf(size, "%.1f", bytes);
    strcat(size, "KB");
  }

  if (bytes >= 1024) { // MB
    bytes /= 1024;
    sprintf(size, "%.1f", bytes);
    strcat(size, "MB");
  }

  if (bytes >= 1024) { // GB
    bytes /= 1024;
    sprintf(size, "%.1f", bytes);
    strcat(size, "GB");
  }

  if (bytes >= 1024) { // TB
    bytes /= 1024;
    sprintf(size, "%.1f", bytes);
    strcat(size, "TB");
  }

  return size;
}

void update_done_status(struct thread_node *node) {
  float percent;

  if (node->no_all_dirs == 0) {
    percent = 100;
  } else {
    percent = (1.0 * node->no_dirs * 100) / node->no_all_dirs;
  }

  if (abs(100 - percent) <= 0.00000000001) {
    strcpy(node->status, "done");
  } else {
    char *aux = (char *)malloc(30);
    sprintf(aux, "%.1f%% in progress\n", percent);
    strcpy(node->status, aux);
    free(aux);
  }
}

// STRUCTURA THREAD DETAILS -------------------------------------------------------
struct thread_details {
  char *path;
  int priority;
  int id;
};

#endif


