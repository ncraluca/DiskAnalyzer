#include<stdlib.h>
#include<sys/stat.h>
#include "constants.h"



void deamon_message(const char *message){
    int fd = open(debug_daemon, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU, S_IRWXG | S_IRWXO);
    if(fd < 0){
        perror("Error at opening the debugging file\n");
    }
    write(fd, message, strlen(message));
    close(fd);
}

void create_directory(const char * given_path){
    struct stat st = {0}; //a stat structure gives us info about a file or directory found at a given_path
    if(stat(given_path, &st)==-1){  //if the file from the path doesn't exist
        mkdir("/tmp/disk-analyzer",0777); //create the directory with 0777 permission = read(4)+write(2)+execute(1) => to all users
    }
}

void dir_hash_init(struct directory *m, int size){
    m->size = size;
    m->content = (struct file_directory **)malloc(size * sizeof(struct file_directory *));
    for(int i=0; i<size; i++){
        m->content[i]=NULL;
    }
}

void thread_list_init(){
    threads_head = (struct thread_node **)malloc(sizeof(struct thread_node *));
    *threads_head=NULL;
}