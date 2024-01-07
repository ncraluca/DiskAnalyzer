#include<stdlib.h>
#include<sys/stat.h>
#include "constants.h"

void debug_message(const char *message){
    int fd = open(debug_daemon, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU, S_IRWXG | S_IRWXO);
    if(fd < 0){
        perror("Error at opening the debugging file\n");
    }
    write(fd, message, strlen(message));
    close(fd);
}


void read_from_file(const char *file_name, char *error_message, char *result){
    debug_message("Reading from the file \n");
    int size = RESULT_SIZE;
    int fd = open(file_name, O_RDONLY, S_IRWXU|S_IRWXG|S_IRWXO); //opening file in read-only mode, giving read, write, execute permissions to the current user, all groups and other users
    if(fd<0){
        debug_message("Error reading from file\n");
    }
    read(fd, result, size);
    close(fd);
}

void write_to_file(const char* file_name, char *error_message, char *string_to_write){
    debug_message("Writing in the file\n");
    int fd = open(file_name, O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU,S_IRWXG|S_IRWXO);
    if(fd<0){
        debug_message("Error writing to file\n");

    }
    write(fd, string_to_write, sizeof(string_to_write));
    close(fd);
}

void create_dir_if_needed(const char * given_path){
    struct stat st = {0}; //a stat structure gives us info about a file or directory found at a given_path
    if(stat(given_path, &st)==-1){  //if the file from the path doesn't exist
        mkdir("/tmp/disk-analyzer",0777); //create the directory with 0777 permission = read(4)+write(2)+execute(1) => to all users
    }
}

void dir_hash_init(struct directory *m, int size){
    m->size = size;
    m->content = (struct file **)malloc(size * sizeof(struct file *));
    for(int i=0; i<size; i++){
        m->content[i]=NULL;
    }
}

void thread_list_init(){
    threads_list_head = malloc(sizeof(struct thread*))
    *threads_list_head=NULL;
}