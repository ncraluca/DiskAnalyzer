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
void debug_daemon(const char *msg){
    int fd = open(log_file_path, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
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

//functia map_find_task returneaza id-ul task-ului cu path-ul path, daca exista, si -1 altfel - folosita in functia Add    
int map_find_task(struct my_map *m, char *path){
    //se parcurge lista de taskuri
    for(int i = 0; i < m->length; i++){
        if(m->lista[i] != NULL){ //daca lista are elemente nenule(daca fiecare lista fd_node contine macar un element)
            for (struct fd_node *nod = m->lista[i]; nod != NULL; nod = nod->next) { //se parcurge lista de fd_node
                if (strcmp((char*)nod->val, path) == 0) //daca path-ul din nod este egal cu path-ul dat ca parametru
                    return nod->id; //se returneaza id-ul task-ului (inseamna ca task-ul exista deja)
            }
        }
    }
    return -1;
    }   

//functia map_insert insereaza in map un nou task - folosita in functia Add
 void map_insert(struct my_map *m, int key, void *val) {
        int mod = key % m->length;
        if (m->lista[mod] == NULL) {
            m->lista[mod] = (struct fd_node *) malloc(sizeof(struct fd_node *));
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
            nod->next = (struct fd_node *) malloc(sizeof(struct fd_node));
            nod->next->next = NULL;
            nod->next->id = key;
            nod->next->val = val;
        }
}

//functia list_find_by_key returneaza threadul cu id-ul key - folosita in functia Remove
struct thr_node* list_find_by_key(struct thr_node** head_ref, int key){
    log_daemon("In list_find_by_key\n");
    struct thr_node *current = (struct thr_node*)malloc(sizeof(struct thr_node));
    current = *head_ref;
    while(current != NULL){
        if(current->id == key){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

//functia *map_find returneaza nodul (task-ul) cu id-ul key - folosita in functia Remove si Info
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

//functia list_delete sterge threadul cu id-ul key - folosita in functia Remove
void list_delete(struct thr_node **head_ref, int key){
    struct thr_node *aux = (struct thr_node*)malloc(sizeof(struct thr_node));
    aux = *list_head;
    struct thr_node *prev = (struct thr_node*)malloc(sizeof(struct thr_node));

    if(aux != NULL && aux->id == key){
        *head_ref = aux->next;
        free(aux);
    } else {
        while(aux != NULL && aux->id != key){
            prev = aux;
            aux = aux->next;
        }

        if(aux != NULL){
            prev->next = aux->next;
            free(aux);
        }
    }
}

//functia map_delete sterge nodul(task-ul) cu id-ul key - folosita in functia Remove
void map_delete(struct my_map *m, int id){
    struct fd_node *nod = (struct fd_node*)malloc(sizeof(struct fd_node));
    nod = m->lista[id%m->length];
    struct fd_node *prev = (struct fd_node*)malloc(sizeof(struct fd_node));
    if(nod != NULL && nod->id == id){
        m->lista[id%m->length] = nod->next;
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