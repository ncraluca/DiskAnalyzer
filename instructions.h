#ifndef INSTRUCTIONS_H_INCLUDED
#define INSTRUCTIONS_H_INCLUDED

#include <stdio.h>  // for perror
#include <fcntl.h>  // for open flags
#include <unistd.h> // for write/close/sleep/pathconf functions
#include <string.h>
#include <fts.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h> // for malloc
#include <limits.h>
#include "utils.h"
#include <linux/limits.h>


//functie care returneaza un string cu toate instructiunile
void Help(char *buf) {
    daemon_message("We are in Help method...\n");
    sprintf(buf, "%s",
            "Usage: da [OPTION]... [DIR]...\n"
            "Analyze the space occupied by the directory ai [DIR]\n"
            "\t-a, --add\t\tanalyze the new directory path for disk usage\n"
            "\t-p, --priority\t\tset priority for the new analysis (works only with -a argument)\n"
            "\t-S, --suspend <id>\tsuspend task with id <id>\n"
            "\t-R, --resume <id>\tresume task with <id>\n"
            "\t-r, --remove <id>\tremove the analysis with the given <id>\n"
            "\t-i, --info <id>\t\tprint status about the analysis with <id> (pending, progress, done)\n"
            "\t-l, --list\t\tlist all analysis tasks, with their ID and the corresponding root path\n"
            "\t-p, --print <id>\tprint analysis report for those tasks that are 'done'\n");
}

//functie care adauga un task nou in lista de task-uri
void Add(char *path, char *priority, char *buf){
    daemon_message("We are in Add method...\n");

    //atribuim un id unic task-ului
    int task_id; 

    //verificam daca task-ul exista deja in lista de task-uri
    int existing_task = find_task(dir, path); //functia e in alt fisier

    //se aloca memorie pentru un nou thread
    pthread_t *new_thread = malloc(sizeof(pthread_t));

    //daca task-ul nu exista deja in lista de task-uri, il adaugam
    if(existing_task == -1){
        //adaugam task-ul in lista de task-uri si ii atribuim un id unic
        task_id = get_next_task_id(); //functia e in alt fisier

        //adaugam task-ul in lista de task-uri
        dir_hash_insert(dir, task_id, path); 
        
        daemon_message("The task was added in the directory hash tree...\n");

        // construim argumentele unui thread pentru task-ul adaugat
        struct thread_details *details = (struct thread_details*)malloc(sizeof(struct thread_details));
      
        details->path = path; //path-ul task-ului de analizat
        details->priority = atoi(priority); //prioritatea task-ului de analizat
        details->id = task_id; //id-ul task-ului de analizat, id-ul unui thread = id-ul task-ului

        daemon_message("Before the function pthread_create\n");
        
        //cream un nou thread pentru task-ul adaugat
        if (pthread_create(new_thread, NULL, disk_analyzer, details)){
            //disk_analyzer e in alt fisier
            char *buf_error = malloc(30);
            sprintf(buf_error, "Could not create a new thread: %d\n", errno);  //daca nu se poate crea un nou thread, se afiseaza eroarea          
            daemon_message(buf_error);
            perror(NULL);
        }
        daemon_message("After pthread_create executed\n");
        sprintf(buf, "Created analysis task with ID %d for '%s' and priority '%s'.\n", task_id, path, priority);
    } 
    else {
        daemon_message("This directory is already included in the analysis\n");
        sprintf(buf, "Directory '%s' is already included in analysis with ID '%d'", path, existing_task);
    }
}

void Suspend(int id, char *buf){
    daemon_message("We are in Suspend method...\n");
     //ideea e ca trebuie sa cautam in lista de threaduri threadul cu id-ul id si sa-l suspendam
    //eventual ii punem un status temporar de suspended
    //IDEE: cream un nou camp in structura thr_node care sa fie un bool, un int: int suspended = 0(0 inseamna ca nu e suspendat, 1 inseamna ca e suspendat)
    //cand facem suspend, punem suspended = 1, iar cand facem resume punem suspended = 0
    //cand facem suspend, verificam daca suspended == 0, daca da, atunci facem suspend, daca nu, nu facem nimic
    //cand facem resume, verificam daca suspended == 1, daca da, atunci facem resume, daca nu, nu facem nimic
    //in disk analyzer sau in programul care executa thread-urile ar trb sa se verifice mereu daca el e suspendat sau nu (are 
    //un while(1) in care se verifica daca e suspendat sau nu, daca nu e suspendat, face ce are de facut)

    /*//tot nu e f buna ca sterge de tot thread-ul
    struct thr_node *thread_node = list_find_by_key(list_head, id);
    if (thread_node == NULL) {
        sprintf(buf, "No existing analysis for task ID %d, there is nothing to suspend", id);
        debug_daemon(buf);
    } else {
        thread_node->suspended = 1;
        sprintf(buf, "Suspended analysis task with ID %d", id);
        debug_daemon(buf);
        } 
    }
    */

    /*gresita
    int existing_task = map_find_task(tasks, id);
    if(existing_task != -1){
        if (pthread_cancel(existing_task)){

            char *buf_error = malloc(30);
            sprintf(buf_error, "Could not cancel the thread: %d\n", errno);
            debug_daemon(buf_error);
            perror(NULL);
        }
        sprintf(buf, "Suspended analysis task with ID %d.\n", id);
    } else {
        sprintf(buf, "No analysis task with ID %d.\n", id);
    }
    */

}
    



void Resume(int id, char *buf){
    daemon_message("We are in Resume method...\n");
    /*ideea e ca trebuie sa cautam in lista de threaduri threadul cu id-ul id si sa-l resume(sa il scoata din suspend)
     struct thr_node *thread_node = list_find_by_key(list_head, id);
    if (thread_node == NULL) {
        sprintf(buf, "No existing analysis for task ID %d, there is nothing to suspend", id);
        debug_daemon(buf);
    } else {
        thread_node->suspended = 0;
        sprintf(buf, "Resumed analysis task with ID %d", id);
        debug_daemon(buf);
        } 
    }

*/
}

//functie care sterge un task din lista de task-uri
void Remove(int id, char *buf){
    daemon_mesaage("We are in Remove method...\n");
    //cautam threadul cu id-ul id in lista de threaduri
    struct thread_node *thread_node = find_thread_id(threads_head, id);
    
    //daca nu exista threadul cu id-ul id in lista de threaduri inseamna ca nu exista task-ul cu id-ul cautat
    if(thread_node == NULL){
        sprintf(buf, "No existing analysis for task ID %d, there is nothing to remove", id);
    } else {
        //daca exista se cauta in dir task-ul cu id-ul id
        struct file_directory *node = dir_hash_find(dir, thread_node->id);
        char *path = (char*)node->fd_path;
        //se sterge threadul 
        int existing_thread = pthread_cancel(*thread_node->thr);
        //daca threadul nu exista inseamna ca nu exista task-ul cu id-ul cautat
        if(existing_thread == -1){
            daemon_message(buf);
        } else {
            //daca threadul exista, se asteapta sa se termine
            void *thr_ret;
            pthread_join(*thread_node->thr, &thr_ret); //pune in thr_ret 0 daca s-a terminat, daca nu pune un cod de eroare
            if (thr_ret != PTHREAD_CANCELED){
                daemon_message(buf);
            }
        }


        //se sterge task-ul din lista de task-uri si din map
        thread_list_delete(threads_head, id);//functia e in alt fisier
        dir_hash_delete(dir, id);//functia e in alt fisier
       
        sprintf(buf, "Removed analysis task with ID %d, status %s for %s", id, thread_node->status, path);
    }
}

//functie care afiseaza informatii despre un task
void Info(int id, char *buf){
    daemon_message("We are in Info method...\n");
    //cautam threadul cu id-ul id in lista de threaduri
    struct thread_node *thread_node = find_thread_id(threads_head, id);
    //daca nu exista threadul cu id-ul id in lista de threaduri inseamna ca nu exista task-ul cu id-ul cautat
    if(thread_node == NULL){
        sprintf(buf, "No existing analysis for task ID %d, no info can be displayed", id);
    } else {
        //daca exista se cauta in map task-ul cu id-ul id
        sprintf(buf, "ID\tPRI\tPath\tDone Status\tDetails\n");
        char priority[3]="***"; //prioritatea e reprezentata de 3 stelute
        struct file_directory *node = dir_hash_find(dir, thread_node->id);
        char *path = (char*)node->fd_path;
        //se afiseaza informatii despre task
        sprintf(buf + strlen(buf), "%d\t%s\t%s\t%s\t%d files, %d dirs\n", 
                thread_node->id, priority+ (3-thread_node->priority), path, 
                thread_node->status, thread_node->no_files, thread_node->no_dirs);
    }
}
    

//functie care afiseaza informatii despre toate task-urile
void List(char *buf){
    daemon_message("We are in List method...\n");
    sprintf(buf, "ID\tPRI\tPath\tDone Status\tDetails\n");
    //se parcurge lista de task-uri
    for(int i = 0; i < dir->size; i++){
        if(dir->content[i] != NULL){ //daca lista de task-uri nu e goala
            struct file_directory *node; 
            for (node = dir->content[i]; node != NULL; node = node->next) { //se parcurge lista de file_directory
                daemon_message("Inainte de list_head\n"); 
                struct thread_node *thread_node = find_thread_id(threads_head, node->id); //se cauta threadul cu id-ul node->id in lista de threaduri
                daemon_message("Dupa thread_head\n");
                char priority[3]="***"; //prioritatea e reprezentata de 3 stelute
                daemon_message("Inainte de dir_hash_find\n");
                struct file_directory *node2 = dir_hash_find(dir, node->id); //se cauta taskul cu id-ul node->id in map
                daemon_message("Dupa dir_hash_find\n");
                char *path = (char*)node2->fd_path; //path-ul task-ului
                daemon_message("Dupa conversie path\n");
                sprintf(buf + strlen(buf), "%d\t%s\t%s\t%s\t%d files, %d dirs\n", node->id, priority+(3-thread_node->priority), path, 
                                    thread_node->status, thread_node->no_files, thread_node->no_dirs); //se afiseaza informatii despre task
                daemon_message("Eroare dupa sprintf\n");
            }
        }
    }
}

//functie care afiseaza raportul de analiza pentru task-ul cu id-ul id
void Print(int id, char *buf){
    daemon_message("We are in Print method...\n");
    //cautam threadul cu id-ul id in lista de threaduri
    struct thread_node *thread_node = find_thread_id(threads_head, id);
    if(thread_node == NULL){ //daca nu exista threadul cu id-ul id in lista de threaduri inseamna ca nu exista task-ul cu id-ul cautat
        sprintf(buf, "No existing analysis for task ID %d", id);
    } 
    else if(strcmp(thread_node->status, "done") == 0){ //daca task-ul este terminat
        char *output_path = malloc(PATH_MAX);
        char *msg = malloc(PATH_MAX+30); //se aloca memorie pentru path-ul fisierului de output si pentru un mesaj de eroare

        sprintf(output_path, "%s_%d.txt", output_from_daemon_prefix, id); //se construieste path-ul fisierului de output
        sprintf(msg, "Couldn't open %s file\n", output_path); //se construieste mesajul de eroare

        //citesc manual din fisierul de output si pun in buf
        int size = RESULT_SIZE;
        int fd = open(output_path, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
        read(fd, msg, size);
        daemon_message("Dupa read in Print\n");
        close(fd);
        //read_from_file(output_path, msg, buf); //se citeste din fisierul de output si se pune in buf
    } 
    else {
        sprintf(buf, "Print analysis report available only for those tasks that are 'done'");
    }
}


//functie care primeste instructiuni de la utilizator si apeleaza functiile corespunzatoare
void handling_instructions(char *instruction, char *buf){
    char *command = strtok(instruction, "\n");
    int task_id;
    switch (atoi(command)){
        case 0: // HELP command
            Help(buf);
            break;
        case 1: // ADD command
            ;
            char *path = malloc(PATH_MAX);
            path = strtok(NULL, "\n");
            char *priority = malloc(PATH_MAX);
            priority = strtok(NULL, "\n");
            daemon_message(priority);
            Add(path, priority, buf);
            break;
        case 2: // SUSPEND command
            ;
            task_id = atoi(strtok(NULL, "\n"));
            Suspend(task_id, buf);
            break;
        case 3: // RESUME commnad
            ;
            task_id = atoi(strtok(NULL, "\n"));
            Resume(task_id, buf);
            break;
        case 4: // REMOVE commnad
            ;
            task_id = atoi(strtok(NULL, "\n"));
            Remove(task_id, buf);
            break;
        case 5: // INFO commnad
            ;
            task_id = atoi(strtok(NULL, "\n"));
            Info(task_id, buf);
            break;
        case 6: // LIST commnad
            List(buf);
            break;
        case 7: // PRINT commnad
            ;
            task_id = atoi(strtok(NULL, "\n"));
            Print(task_id, buf);
            break;
        default:
            break;
    }
}

#endif // INSTRUCTIONS_H_INCLUDED