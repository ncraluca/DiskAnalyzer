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

}

#endif // INSTRUCTIONS_H_INCLUDED