#ifndef INSTRUCTIONS_H_INCLUDED
#define INSTRUCTIONS_H_INCLUDED

#include <cstdio> //de ce nu merg bibliotecile?
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <fts.h>
#include <cerrno>
#include <pthread.h>
#include <cstdlib>
#include <climits>
#include "utils.h"

void help(char *res) {
    // Your function implementation here
}

void add(char *path, char *priority, char *res) {
    // Your function implementation here
}

void suspend(int id, char *res){
}

void resume(int id, char *res){
}

void remove(int id, char *res){
}

void info(int id, char *res){
}

void list(char *res){
}

void print_id(int id, char *res){
}

void handling_instructions(char *instruction, char *res){
    /*
    char *command = strtok(instruction, "\n");
    int id;
    switch (atoi(command)){
        case 0: // HELP command
            //help(res);
            break;
        case 1: // ADD command
            //;
            //add(path, priority, res);
            break;
        case 2: // SUSPEND command
            //;
            //id = atoi(strtok(NULL, "\n"));
            //suspend(id, res);
            break;
        case 3: // RESUME commnad
            //;
            //id = atoi(strtok(NULL, "\n"));
            //resume(id, res);
            break;
        case 4: // REMOVE commnad
            //;
            //id = atoi(strtok(NULL, "\n"));
            //remove(id, res);
            break;
        case 5: // INFO commnad
            //;
            //id = atoi(strtok(NULL, "\n"));
            //info(id, res);
            break;
        case 6: // LIST commnad
            //list(res);
            break;
        case 7: // PRINT commnad
            //;
            //id = atoi(strtok(NULL, "\n"));
            //print_id(id, res);
            break;
        default:
            break;
    }
    */
}

#endif // INSTRUCTIONS_H_INCLUDED