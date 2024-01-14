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

//rutele catre fisierele folosite de daemon
const char *daemon_pid = "/tmp/disk-analyzer/daemon.pid";
const char *da_pid = "/tmp/disk-analyzer/da.pid";
const char *input_from_user = "/tmp/disk-analyzer/input.txt";
const char *output_from_daemon_prefix = "/tmp/disk-analyzer/output";
const char *output_from_daemon = "/tmp/disk-analyzer/output.txt";
const char *debug_daemon = "/tmp/disk-analyzer/debug.txt";

//variabile folosite de daemon

//folosite in Add
int task_id;

// hash 
//Acesta contine id-ul, valoarea de tip void *-> adica path-ul, si un pointer la urmatorul element din lista.
//un nod = un task
struct fd_node {
    int id;
    void *val;
    struct fd_node *next;
};

//o structura de tipul lungime lista cu elemente de tip fd_node, lista
//cu toate elementele fd_node.
struct my_map {
    int length;
    struct fd_node **lista;
};

//lista de taskuri
struct my_map *tasks;

 struct thread_args {
            char *path;
            int priority;
            int id;
};

//folosite in Remove
//lista de threaduri
struct thr_node {
    int id;
    int priority;
    pthread_t *thr;
    char *done_status;
    int files, dirs, total_dirs;
    struct thr_node *next;
};

//capul listei de threaduri
struct thr_node **list_head;



#endif
