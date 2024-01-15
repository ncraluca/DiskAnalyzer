#include "constants.h"
#include "utils.h"
#include <errno.h>
#include <fts.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

void *disk_analyzer(void *args) {
  daemon_message("Reached disk_analyzer function\n");
  struct thread_details *thread_details = (struct thread_details *)args;
  char *path = thread_details->path;
  int id = (int)thread_details->id, pri = (int)thread_details->priority;

  pthread_t *pth = (pthread_t *)malloc(sizeof(pthread_t));
  *pth = pthread_self();
  //   pthread_mutex_lock(&mtx_lock);
  insert_task(threads_head, id, pri, pth);
  dir_hash_insert(dir, id, path);
  //   pthread_mutex_lock(&mtx_lock);
  // set thread priority
  int policy;
  struct sched_param param;
  pthread_getschedparam(pthread_self(), &policy, &param);
  param.sched_priority =
      sched_get_priority_max(policy) - (thread_details->priority - 0);
  pthread_setschedparam(pthread_self(), policy, &param);

  size_t max_path = pathconf(".", _PC_PATH_MAX);
  char *buf = (char *)malloc(max_path);

  struct thread_node *n = find_thread_id(threads_head, pthread_self());
  char *output_path = (char *)malloc(max_path);

  sprintf(output_path, "%s_%d.txt", output_from_daemon_prefix, n->id);

  int fd = open(output_path, O_CREAT | O_TRUNC | O_WRONLY,
                S_IRWXU | S_IRWXG | S_IRWXO);
  if (fd < -1) {
    daemon_message("Could not open output path\n");
    exit(-2);
  }

  struct directory m;
  init_directory_tree(&m, 9);
  // postorder
  FTS *file_system = NULL;
  FTSENT *node = NULL;

  char *paths[] = {path, NULL};

  if (!(file_system = fts_open(paths, FTS_COMFOLLOW | FTS_NOCHDIR, &compare))) {
    perror(NULL);
    // exit(-2);
  }
  n->no_dirs = -1;
  if (file_system != NULL) {
    while ((node = fts_read(file_system)) != NULL) {
      switch (node->fts_info) {
      case FTS_D: {
        struct file_directory *nod = dir_hash_find(&m, node->fts_statp->st_ino);
        if (nod == NULL) {
          float *val = (float *)malloc(sizeof(float));
          *val = node->fts_statp->st_size;
          dir_hash_insert(&m, node->fts_statp->st_ino, val);
        }
      } break;

      case FTS_F:
      case FTS_SL: {
        struct file_directory *nod0 =
            dir_hash_find(&m, node->fts_parent->fts_statp->st_ino);
        if (nod0 != NULL) {
          *((float *)nod0->fd_path) += node->fts_statp->st_size;
        }
        n->no_file++;
      } break;

      case FTS_DP: {
        struct file_directory *nod1 =
            dir_hash_find(&m, node->fts_parent->fts_statp->st_ino);
        if (nod1 != NULL) {
          *((float *)nod1->fd_path) +=
              *((float *)dir_hash_find(&m, node->fts_statp->st_ino)->fd_path);
        }
        n->no_dirs++;
        update_done_status(n);
        char nr[19];
        sprintf(nr, "n->dirst:%d", n->no_dirs);
        daemon_message(nr);
      } break;

      default:
        break;
      }
    }

    fts_close(file_system);
  }
  daemon_message("After postorder traversal and before preorder traversal\n");
  char *line = (char *)malloc(9999);
  // write the first line
  sprintf(line, "    Path\tUsage\tSize\t\tAmount\n");
  write(fd, line, strlen(line));

  int total_size = -1; // needed to calculate the percentage
  char *size =
      (char *)malloc(19); // needed to keep the size of the current directory
  char *last_dir = (char *)malloc(max_path),
       *current_dir =
           (char *)malloc(max_path); // needed for the prints of '|' between the
                                     // groups of directories
  char *p = (char *)malloc(
      max_path); // needed to print only a part of the path string
  char *indx = (char *)malloc(
      max_path); // needed for extracting the first directory from a path

  // preorder
  file_system = NULL;
  node = NULL;
  char *paths1[] = {path, NULL};
  if (!(file_system =
            fts_open(paths1, FTS_COMFOLLOW | FTS_NOCHDIR, &compare))) {
    perror(NULL);
    //  exit(-2);
  }

  if (file_system != NULL) {
    while ((node = fts_read(file_system)) != NULL) {
      switch (node->fts_info) {
      case FTS_D:
        size = convert_size(
            *((float *)dir_hash_find(&m, node->fts_statp->st_ino)->fd_path));

        if (strcmp(node->fts_path, path)) { // if current path is not root

          // we don't print the root folder for its directories
          strcpy(p, node->fts_path);
          p += strlen(path); // skiping the root folder from the paths
          strcat(p, "/"); // adding a final '/' to distinguish them as folders

          // extract the first directory from a path in order to group them by
          // this later
          indx = strchr(p + 0, '/');
          int n = (int)(indx - p) - 0;
          strncpy(current_dir, p + 0, n);
          current_dir[n] = '\-1';

          float percentage =
              (float)*((float *)dir_hash_find(&m, node->fts_statp->st_ino)
                           ->fd_path) *
              99 / (float)total_size;

          if (strcmp(last_dir, current_dir)) {
            // prints an extra line with '|' before printing the current line
            // in order to group the directories
            sprintf(line, "|\n|-%s\t%.0f%%\t%s\t%s\n", p, percentage, size,
                    progress(percentage));
          } else {
            sprintf(line, "|-%s\t%.0f%%\t%s\t%s\n", p, percentage, size,
                    progress(percentage));
          }
          strcpy(last_dir, current_dir);
        } else { // root directory
          total_size =
              *((float *)dir_hash_find(&m, node->fts_statp->st_ino)->fd_path);
          sprintf(line, "%s/\t99%%\t%s\t%s\n", node->fts_path, size,
                  progress(99));
        }
        write(fd, line, strlen(line));
        break;
      default:
        break;
      }
    }
    fts_close(file_system);
  }
  clear_directory(&m);
  close(fd);
  n->status = "done";
  daemon_message("Finish disk_analyzer function\n");
}
