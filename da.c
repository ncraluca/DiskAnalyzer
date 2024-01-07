#include<stdio.h> //library used for printf/scanf functions and also for read and write actions between the app and command line
#include<stdlib.h> //library used for memory allocation
#include<signal.h> //used for sending signals between shared files 
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include "utils.h"
#include "constants.h"

int daemon_pid;

int read_result_from_daemon(int signo){
    char final_result = malloc(RESULT_SIZE);
    read_from_file(output_from_daemon, "Error at opening output file from deamon\n", final_result);
    printf("%s\n",final_result);
}

void write_da_pid(){
    create_dir_if_needed(da_pid_path);
    char *pid_da_string = malloc(10);
    int pid = getpid();
    sprintf(pid_da_string, "%d", pid); //converted int to string so it could be written in the file
    write_to_file(da_pid_path, "Error at writing the corresponding pid in the da_pid_path file \n", pid_da_string);
}

void read_daemon_pid(){
    int fd = open(daemon_pid_path, O_RDONLY);
    if(fd<0){
        perror("Daemon is not running!");
        return errno;
    }
    char *buf = (char *)malloc(10);
    read(fd,buf,10);
    daemon_pid = buf; 
    close(fd);
}

int is_valid_option(const char *option, const char *format1, const char *format2) {
    return strcmp(option, format1) == 0 || strcmp(option, format2) == 0;
}

void invalid_arguments_message(){
    printf("Invalid number of arguments. \n");
    printf("Use --help command for more information. \n");
}

void send_input_from_user_to_daemon(const char* instruction){
    create_dir_if_needed(input_from_user);
    int fd = open(input_from_user, O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO);
    write(fd, instruction, strlen(instruction));
    close(fd);

    if(daemon_pid>0){
        //If daemon pid was correct send a signal to the daemon program
        kill(daemon_pid, SIGUSR1);

        sleep(2); //let da wait for 2 seconds for the daemon to process the instructions and give the signal back

    }
    else{
        printf("Daemon is not running!");
        exit(-1);
    }
}

int main(int argc, char *argv[]){

    //da.c and daemon.c communicate via signals
    //when da.c receives SIGUSR2 it means that the daemon finished a task and da.c can print it to the final user
    //when daemon.c receives SIGUSR1 it means that da.c sent as a set of instructions that the daemon has to resolve
    signal(SIGUSR2, read_result_from_daemon);
    write_da_pid(); //put da pid in the file so the daemon could send back the SIGUSR2 when it finishes
    read_daemon_pid(); //read daemon pid so that the main program knows where to send the signal
    char* instruction = malloc(INSTR_LENGTH);
    if(argc ==1 ){
        invalid_arguments_message();
        return 0;
    }

    //check the ADD command
    if(is_valid_option(argv[1], "a","--add")){
        if (argc < 3) {
            invalid_arguments_message();
            return -1;
        }

        //check priority
        int priority = 1; //default priority
        if(argc == 5 && is_valid_option(argv[3], "-p", "--priority")){
            priority = atoi(argv[4]);
            if(priority < 1 || priority > 3){
                printf("Invalid value for priority.\n");
                printf("Valid values for priority are: \n");
                printf("    1 - low \n");
                printf("    2 - normal \n");
                printf("    3 - high\n");
                return -1;
            }
        }

        sprintf(instruction, "%s\n%s\n%s\n", ADD, argv[2], priority);
    }

    //Check the suspend command
    if(is_valid_option(argv[1], "-S", "--suspend")){
        if(argc < 3){
            invalid_arguments_message();
            return -1;
        }
        if(!atoi(argv[2])){
            printf("Invalid task ID \n");     
            return -1;  
        }
        sprintf(instruction, "%s\n%s\n", SUSPEND, atoi(argv[2]));
    }

    //Check the resume command
    if(is_valid_option(argv[1], "-R", "--resume")){
        if(argc < 3){
            invalid_arguments_message();
            return -1;
        }
        if(!atoi(argv[2])){
            printf("Invalid task ID \n");     
            return -1;  
        }
        sprintf(instruction, "%s\n%s\n", RESUME, atoi(argv[2]));
    }

    //Check the remove command
    if(is_valid_option(argv[1], "-r", "--remove")){
        if(argc < 3){
            invalid_arguments_message();
            return -1;
        }
        if(!atoi(argv[2])){
            printf("Invalid task ID \n");     
            return -1;  
        }
        sprintf(instruction, "%s\n%s\n", REMOVE, atoi(argv[2]));
    }

    //check info command
    if(is_valid_option(argv[1], "-i", "--info")){
        if(argc < 3){
            invalid_arguments_message();
            return -1;
        }
        if(!atoi(argv[2])){
            printf("Invalid task ID \n");     
            return -1;  
        }
        sprintf(instruction, "%s\n%s\n", INFO, atoi(argv[2]));
    }

    //check print command
    if(is_valid_option(argv[1], "-p", "--print")){
        if(argc < 3){
            invalid_arguments_message();
            return -1;
        }
        if(!atoi(argv[2])){
            printf("Invalid task ID \n");     
            return -1;  
        }
        sprintf(instruction, "%s\n%s\n", PRINT, atoi(argv[2]));
    }

    //check list command
    if(is_valid_option(argv[1], "-l", "--list")){
        if(argc > 2){
            invalid_arguments_message();
            return -1;
        }
        sprintf(instruction, "%s\n", LIST);
    }

    //check help command
    if(is_valid_option(argv[1], "-h", "--help")){
       if(argc > 2){
            invalid_arguments_message();
            return -1;
        }
        sprintf(instruction, "%s\n", HELP);
    }

    send_input_from_user_to_daemon(instruction);

}