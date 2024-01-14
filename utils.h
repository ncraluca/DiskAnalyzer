#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "constants.h"
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
int task_id;
struct my_map *tasks;
struct thr_node **list_head;
pthread_mutex_t mtx_lock, mtx_lock_map;

void log_daemon(const char *msg) {
  int fd = open(output_from_daemon, O_CREAT | O_APPEND | O_WRONLY,
                S_IRWXU | S_IRWXG | S_IRWXO);
  if (fd < -1) {
    perror("Couldn't open log file\n");
  }
  write(fd, msg, strlen(msg));
  close(fd);
}
int get_next_task_id() { return ++task_id; }

void create_dir_if_not_exists(const char *path) {
  struct stat st = {0};
  if (stat(path, &st) == -1) {
    mode_t oldmask = umask(0);
    mkdir("/tmp/disk-analyzer", 0777);
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

// hash
struct fd_node {
  int id;
  void *val;
  struct fd_node *next;
};

struct my_map {
  int length;
  struct fd_node **lista;
};

void map_init(struct my_map *m, int n) {
  m->length = n;
  m->lista = (struct fd_node **)malloc(n * sizeof(struct fd_node *));
  for (int i = 0; i < n; i++)
    m->lista[i] = NULL;
}

struct fd_node *map_find(struct my_map *m, int key) {
  int mod = key % m->length;
  if (m->lista[mod] == NULL)
    return NULL;
  for (struct fd_node *nod = m->lista[mod]; nod != NULL; nod = nod->next) {
    if (nod->id == key)
      return nod;
  }
  return NULL;
}

int map_find_task(struct my_map *m, char *path) {
  for (int i = 0; i < m->length; i++) {
    if (m->lista[i] != NULL) {
      for (struct fd_node *nod = m->lista[i]; nod != NULL; nod = nod->next) {
        if (strcmp((char *)nod->val, path) == 0)
          return nod->id;
      }
    }
  }
  return -1;
}

void map_insert(struct my_map *m, int key, void *val) {
  int mod = key % m->length;
  if (m->lista[mod] == NULL) {
    m->lista[mod] = (struct fd_node *)malloc(sizeof(struct fd_node *));
    m->lista[mod]->val = val;
    m->lista[mod]->id = key;
    m->lista[mod]->next = NULL;
  } else {
    struct fd_node *nod;
    for (nod = m->lista[mod]; nod->next != NULL; nod = nod->next) {
      if (nod->id == key) {
        nod->val = val;
        return;
      }
    }
    if (nod->id == key) {
      nod->val = val;
      return;
    }
    nod->next = (struct fd_node *)malloc(sizeof(struct fd_node));
    nod->next->next = NULL;
    nod->next->id = key;
    nod->next->val = val;
  }
}
// for debugging
void map_print_int(struct my_map *m) {
  for (int i = 0; i < m->length; i++) {
    printf("%d: ", i);
    if (m->lista[i] != NULL) {
      struct fd_node *nod;
      for (nod = m->lista[i]; nod != NULL; nod = nod->next) {
        printf("(%d, %d) ", nod->id, *((int *)nod->val));
      }
    }
    printf("\n");
  }
}

void map_print_char(struct my_map *m, char *res) {
  for (int i = 0; i < m->length; i++) {
    sprintf(res, "%d: ", i);
    if (m->lista[i] != NULL) {
      struct fd_node *nod;
      for (nod = m->lista[i]; nod != NULL; nod = nod->next) {
        sprintf(res, "(%d, %s) ", nod->id, (char *)nod->val);
      }
    }
    sprintf(res, "\n");
  }
}

void map_delete(struct my_map *m, int id) {
  struct fd_node *nod = (struct fd_node *)malloc(sizeof(struct fd_node));
  nod = m->lista[id % m->length];
  struct fd_node *prev = (struct fd_node *)malloc(sizeof(struct fd_node));
  if (nod != NULL && nod->id == id) {
    m->lista[id % m->length] = nod->next;
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

void map_clear(struct my_map *m) {
  for (int i = 0; i < m->length; i++)
    free(m->lista[i]);
  free(m->lista);
}

struct thr_node {
  int id;
  int priority;
  pthread_t *thr;
  char *done_status;
  int files, dirs, total_dirs;
  struct thr_node *next;
};

void list_init() {
  list_head = (struct thr_node **)malloc(sizeof(struct thr_node *));

  *list_head = NULL;
}

void list_insert(struct thr_node **head_ref, int id, int priority,
                 pthread_t *thr) {

  struct thr_node *new_node =
      (struct thr_node *)malloc(sizeof(struct thr_node));

  new_node->id = id;
  new_node->priority = priority;
  new_node->thr = thr;
  new_node->done_status = (char *)malloc(30);
  strcpy(new_node->done_status, "preparing");
  new_node->files = 0;
  new_node->dirs = 0;
  // find out total number of subdirectories
  struct fd_node *node = map_find(tasks, id);
  char *path = (char *)(node->val);
  new_node->total_dirs = count_dirs(path);

  new_node->next = *head_ref;
  *head_ref = new_node;
}

void list_delete(struct thr_node **head_ref, int key) {
  struct thr_node *aux = (struct thr_node *)malloc(sizeof(struct thr_node));
  aux = *list_head;
  struct thr_node *prev = (struct thr_node *)malloc(sizeof(struct thr_node));

  if (aux != NULL && aux->id == key) {
    *head_ref = aux->next;
    free(aux);
  } else {
    while (aux != NULL && aux->id != key) {
      prev = aux;
      aux = aux->next;
    }

    if (aux != NULL) {
      prev->next = aux->next;
      free(aux);
    }
  }
}

struct thr_node *list_find_by_key(struct thr_node **head_ref, int key) {
  log_daemon("In list_find_by_key\n");
  struct thr_node *current = (struct thr_node *)malloc(sizeof(struct thr_node));
  current = *head_ref;
  while (current != NULL) {
    if (current->id == key) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

struct thr_node *list_find_by_thr(struct thr_node **head_ref, pthread_t thr) {
  struct thr_node *current = (struct thr_node *)malloc(sizeof(struct thr_node));
  current = *head_ref;
  while (current != NULL) {
    if (pthread_equal(*(current->thr), thr)) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

void list_print(struct thr_node **head_ref, char *res) {
  res[0] = '\0';
  struct thr_node *current = (struct thr_node *)malloc(sizeof(struct thr_node));
  current = *list_head;
  while (current != NULL) {
    sprintf(res + strlen(res), "Id: %d, ", current->id);
    current = current->next;
  }
  sprintf(res + strlen(res), "\n");
}

void *reverse(char *v) {
  char *str = (char *)v, *s = str, *f = str + strlen(str) - 1;
  while (s < f) {
    *s ^= *f;
    *f ^= *s;
    *s ^= *f;
    s++;
    f--;
  }
  return str;
}

void itoa(int n, char *s) {
  int i = 0;
  // generate digits in reverse order
  do {
    s[i++] = n % 10 + '0';
  } while ((n /= 10) > 0);

  s[i] = '\0';
  reverse(s);
}

char *get_progress(float percent) {
  char *res = (char *)malloc(51);
  res = "##################################################";
  int number_of_hashtags = percent * 50 / 100;
  res += (50 - number_of_hashtags);
  return res;
}

char *convert_size_to_standard_unit(float bytes) {
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

int get_file_size(const char *file_name) {
  log_daemon("Inainte de fopen\n");
  FILE *fp = fopen(file_name, "r");
  log_daemon("Dupa fopen\n");
  if (fp == NULL) {
    printf("File Not Found!\n");
    log_daemon("Dupa get file size\n");
    return -1;
  }
  fseek(fp, 0, SEEK_END);
  int res = ftell(fp);
  fclose(fp);

  return res;
}

void read_from_file(const char *path, char *error_msg, char *res) {
  log_daemon("In read_from_file\n");
  log_daemon(path);
  int size = 10000000; // get_file_size(path);
  log_daemon("Dupa get file size\n");
  int fd = open(path, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
  if (fd < 0) {
    log_daemon("Eroare deschidere fisier\n");
    // exit(-1);
  }
  log_daemon("Inainte de malloc\n");

  log_daemon("Dupa malloc\n");
  read(fd, res, size);
  log_daemon("Dupa read\n");
  close(fd);
}

void update_done_status(struct thr_node *node) {
  float percent;

  if (node->total_dirs == 0) {
    percent = 100;
  } else {
    percent = (1.0 * node->dirs * 100) / node->total_dirs;
  }

  if (abs(100 - percent) <= 0.00000000001) {
    strcpy(node->done_status, "done");
  } else {
    char *aux = (char *)malloc(30);

    sprintf(aux, "%.1f%% in progress\n", percent);
    strcpy(node->done_status, aux);
    free(aux);
  }
}

struct thread_args {
  char *path;
  int priority, id;
};

#endif
