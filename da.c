#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "utils.h"
#define VALID_OPTIONS_COUNT 16
const char *VALID_OPTIONS[VALID_OPTIONS_COUNT] = {"--add", "-a","--list","-l", "--help", "-h","--suspend", "-S", "--resume","-R","--info","-i","--print","-p","--remove","-r"};
int daemon_pid;

bool check_valid_option(const char *option) {
    for (int i = 0; i < VALID_OPTIONS_COUNT; ++i) {
        if (strcmp(option, VALID_OPTIONS[i]) == 0) {
            return true;  // Valid option found
        }
    }
    return false;  // Option not found in the valid options
}
bool is_list_help_command(const char *command) {
    return (strcmp(command, "--list") == 0 || strcmp(command, "--help") == 0 || strcmp(command,"-l")==0 || strcmp(command,"-h")==0);
}


bool is_add_command(const char *command) {
    return (strcmp(command, "--add") == 0 || strcmp(command, "-a") == 0);
}


bool is_priority_command(const char *command){
    return (strcmp(command,"-p")==0 || strcmp(command,"--priority")==0);
}


bool is_list_command(const char *command) {
    return (strcmp(command, "--list") == 0 || strcmp(command, "-l")==0);
}

bool is_help_command(const char *command) {
    return (strcmp(command, "--help") == 0 || strcmp(command, "-h") ==0);
}

bool is_suspend_command(const char *command) {
    return (strcmp(command, "--suspend") == 0||strcmp(command, "-S") ==0);
}

bool is_remove_command(const char *command) {
    return (strcmp(command, "--remove") == 0 || strcmp(command, "-r") ==0);
}

bool is_info_command(const char *command) {
    return (strcmp(command, "--info") == 0 || strcmp(command, "-i") ==0);
}

bool is_print_command(const char *command) {
    return (strcmp(command, "--print") == 0 || strcmp(command, "-p") ==0);
}

bool is_resume_command(const char *command) {
    return (strcmp(command, "--resume") == 0 || strcmp(command, "-R") ==0);
}


void write_to_daemon(const char* instruction) {
    create_directory(input_from_user);

    int fd = open(input_from_user, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    int len = strlen(instruction);
    write(fd, instruction, len);
    close(fd);

    if (daemon_pid > 0) {
        kill(daemon_pid, SIGUSR1);
        sleep(2);
    } else {
        fprintf(stderr, "Couldn't send instruction to daemon because it is not running\n");
        exit(-1);
    }
}

void read_daemon_pid() {
    int fd = open(daemon_pid_path, O_RDONLY);
    if (fd < 0) {
        perror("Daemon hasn't started");
        exit(EXIT_FAILURE);
    }
    char *buf = (char*) malloc(MAX_PID_LENGHT); 
    read(fd, buf, MAX_PID_LENGHT);
    close(fd);
    daemon_pid=atoi(buf);
}

void read_results_from_daemon(int signo) {
    char* res = malloc(1000000);
    int size = RESULT_SIZE;
    int fd = open(output_from_daemon, O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd<0){
        daemon_message("Eroare la deschidere fisier output\n");
    }
    read(fd, res, size);
    daemon_message("Dupa read\n");
    close(fd);
    printf("%s\n", res);
    //free(res);
}

void write_da_pid() {
    create_directory(da_pid_path);

    char* pidstring = malloc(MAX_PID_LENGHT);
    sprintf(pidstring, "%d", getpid());

    int fd = open(da_pid_path, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0) {
        perror("Could not create da pid file");
        //exit(EXIT_FAILURE);
    }

    write(fd, pidstring, strlen(pidstring));
    close(fd);
}
void error_message(){
    printf("No arguments found. Please introduce an analyze-option and a desired path.\n");
    printf("Use --help command for more information.\n");
}

int main(int argc, char** argv) {
    signal(SIGUSR2, read_results_from_daemon);
    write_da_pid();
    read_daemon_pid();
    
    char* instruction = malloc(INSTR_LENGTH);
    if (argc == 1) {
        error_message();
        return EXIT_FAILURE;
    } else if (check_valid_option(argv[1]) == false) 
    {
        error_message();
        return EXIT_FAILURE;
    } else if (argc == 2 && is_list_help_command(argv[1]) == false){
        error_message();
        return EXIT_FAILURE;
    } else if (argc >= 4 && (is_add_command(argv[1]) && (is_priority_command(argv[3]))==false)){
        error_message();
        return EXIT_FAILURE;
    } else {
        if (is_add_command(argv[1])){
            if (argc != 5 || (argc == 5 && !is_priority_command(argv[3]))){
                printf("Invalid arguments for --add command.\nUse --help command for more information.\n");
                return EXIT_FAILURE;
            } else { // ADD command
                char* priority = malloc(7);
                // TODO check is valid path 
                if (!strcmp(argv[4], "1"))
                    strcpy(priority, "1");
                else if (!strcmp(argv[4], "2"))
                    strcpy(priority, "2");
                else if (!strcmp(argv[4], "3"))
                    strcpy(priority, "3");
                else printf("Invalid arguments for --add command.\nUse --help command for more information.\n");
                sprintf(instruction, "%s\n%s\n%s\n", ADD, argv[2], priority);
            }
        } else if (is_list_command(argv[1])){ 
            if (argc > 2){
                printf("Invalid arguments for --list command.\nUse --help command for more information.\n");
                return 0;
            } else { // LIST command
                sprintf(instruction, "%s\n", LIST);
            }
        } else if (is_help_command(argv[1])){ 
            if (argc > 2){
                printf("Invalid arguments for --help command.\n");
                return 0;
            } else { // HELP command
                sprintf(instruction, "%s\n", HELP);
            }
        } else if (argc != 3){
            printf("Invalid number of arguments.\nUse --help command for more information.\n");
            return 0;
        } else if (is_suspend_command(argv[1])){
            if(!atoi(argv[2]))
                printf ("Invalid task ID.");
            // SUSPEND command
            sprintf(instruction, "%s\n%s\n", SUSPEND, argv[2]);
        } else if (is_resume_command(argv[1])){
            if(!atoi(argv[2]))
                printf ("Invalid task ID.");
            // RESUME command
            sprintf(instruction, "%s\n%s\n", RESUME, argv[2]);
        } else if (is_remove_command(argv[1])){
            if(!atoi(argv[2]))
                printf ("Invalid task ID.");
            // REMOVE command
            sprintf(instruction, "%s\n%s\n", REMOVE, argv[2]);
        } else if (is_info_command(argv[1])){
            if(!atoi(argv[2]))
                printf ("Invalid task ID.");
            // INFO command
            sprintf(instruction, "%s\n%s\n", INFO, argv[2]);
        } else if (is_print_command(argv[1])){
            if(!atoi(argv[2]))
                printf ("Invalid task ID.");
            // PRINT command
            sprintf(instruction, "%s\n%s\n", PRINT, argv[2]);
        }
    }
    write_to_daemon(instruction);

    return 0;
}