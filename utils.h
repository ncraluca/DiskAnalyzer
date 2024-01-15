#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fts.h>
#include <pthread.h>
#include "constants.h"

//functie care are rol de debugger (scrie in fisierul log_file_path)
void daemon_message(const char *msg){
    int fd = open(debug_daemon, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0){
        perror("Couldn't open debug file\n");
    }
    write(fd, msg, strlen(msg));
    close(fd);
}

//fctia get_next_task_id() returneaza un id unic pentru fiecare task - folosita in functia Add
int get_next_task_id(){
    return ++task_id;
}       

//functia find_task returneaza id-ul task-ului cu path-ul path, daca exista, si -1 altfel - folosita in functia Add    
int find_task(struct directory *m, char *path){
    //se parcurge lista de taskuri
    for(int i = 0; i < m->size; i++){
        if(m->content[i] != NULL){ //daca lista are elemente nenule(daca fiecare lista fd_node contine macar un element)
            for (struct file_directory *nod = m->content[i]; nod != NULL; nod = nod->next) { //se parcurge lista de fd_node
                if (strcmp((char*)nod->fd_path, path) == 0) //daca path-ul din nod este egal cu path-ul dat ca parametru
                    return nod->id; //se returneaza id-ul task-ului (inseamna ca task-ul exista deja)
            }
        }
    }
    return -1;
    }   

//functia map_insert insereaza in map un nou task - folosita in functia Add
 void dir_hash_insert(struct directory *m, int key, void *val) {
        int mod = key % m->size;
        if (m->content[mod] == NULL) {
            m->content[mod] = (struct file_directory *) malloc(sizeof(struct file_directory *));
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
            nod->next = (struct file_directory *) malloc(sizeof(struct file_directory));
            nod->next->next = NULL;
            nod->next->id = key;
            nod->next->fd_path = val;
        }
}

//functia list_find_by_key returneaza threadul cu id-ul key - folosita in functia Remove
struct thread_node* find_thread_id(struct thread_node** head_ref, int key){
    daemon_message("In find_thread_id\n");
    struct thread_node *current_thread = (struct thread_node*)malloc(sizeof(struct thread_node));
    current_thread = *head_ref;
    while(current_thread != NULL){
        if(current_thread->id == key){
            return current_thread;
        }
        current_thread = current_thread->next;
    }
    return NULL;
}

//functia *map_find returneaza nodul (task-ul) cu id-ul key - folosita in functia Remove si Info
struct file_directory *dir_hash_find(struct directory *m, int key) {
    int mod = key % m->size; //se parcurge arborele si se cauta nivelul din arbore
    if (m->content[mod] == NULL)
        return NULL;
    for (struct file_directory *nod = m->content[mod]; nod != NULL; nod = nod->next) {
        if (nod->id == key)
            return nod;
    }
    return NULL;
}

//functia list_delete sterge threadul cu id-ul key - folosita in functia Remove
void thread_list_delete(struct thread_node **head_ref, int key){
    struct thread_node *current_thread = (struct thread_node*)malloc(sizeof(struct thread_node));
    current_thread = *threads_head;
    struct thread_node *previous = (struct thread_node*)malloc(sizeof(struct thread_node));

    if(current_thread != NULL && current_thread->id == key){
        *head_ref = current_thread->next;
        free(current_thread);
    } else {
        while(current_thread != NULL && current_thread->id != key){
            previous = current_thread;
            current_thread = current_thread->next;
        }

        if(current_thread != NULL){
            previous->next = current_thread->next;
            free(current_thread);
        }
    }
}

//functia map_delete sterge nodul(task-ul) cu id-ul key - folosita in functia Remove
void dir_hash_delete(struct directory *m, int id){
    struct file_directory *nod = (struct file_direcory*)malloc(sizeof(struct file_directory));
    nod = m->content[id%m->size];
    struct file_directory *prev = (struct file_directory*)malloc(sizeof(struct file_directory));
    if(nod != NULL && nod->id == id){
        m->content[id%m->size] = nod->next;
        free(nod);
    } else {
        while(nod != NULL && nod->id != id){
            prev = nod;
            nod = nod->next;
        }

        if(nod != NULL){
            prev->next = nod->next;
            free(nod);
        }
    }
}

void* disk_analyzer(void *args){  
}
#endif