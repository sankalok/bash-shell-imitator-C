#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

/**
 * @author Sankalok Sen
 * @version 1.0.0s
 * Done: 
 * 1. Process creation and execution – foreground
 * 2. Process creation and execution – use of ‘|’
 * 3. Use of signals - SIGINT and SIGUSR1
 * 4. Built-in command: timeX
 * 5. Built-in command: exit
 * Not Done:
 * NO BONUS PARTS DONE
*/

// Process - for SIGINT Handling
int process = 0;
// Process - for SIGKILL Handling
int kills = 0;

void SIGUSR1Handler(int sigusr1) {
    /**
     * SIGUSR Handler
    */
    sleep(0.5);
}

void SIGINTHandler(int sigint) {
    /**
     * SIGINT Handler
    */
    signal(SIGINT, SIGINTHandler);
    if(process == 1) {
        printf("Terminated\n");
    }
    else if(process == 0) {
        printf("\n$$ 3230shell ##  ");  
    }
    fflush(stdout);
}

void SIGKILLHandler(int sigkill) {
    /**
     * SIGKILL Handler
    */
    signal(SIGKILL, SIGKILLHandler);
    if(kills == 1) {
        printf("Killed\n");
    }
    else if(kills == 0) {
        printf("\n$$ 3230shell ##  ");  
    }
    fflush(stdout);
}

int main(int argc, char *argv[]) {

    // Signal calls for SIGINT & SIGKILL
    signal(SIGINT, SIGINTHandler);
    signal(SIGKILL, SIGKILLHandler);

    while(1) {
        process = 0;
        kills = 0;
        // Command Line Input
        char line[1025];
        // Maximum Number of Commands
        char *commands[30];
        // Argument Count
        int count_args = 0;
        // Pipe Count
        int count_pipe = 0;
        int wrong_pipe = -1;
        
        //Shell Line
        printf("$$ 3230shell ##  ");
        //fgets() shell
        fgets(line, sizeof line, stdin);
        if(line[0] == '\n') {
            continue;
        }

        //Clean Pipes
        char result[1025] = "";
        for(int i = 0; i < strlen(line)-1; i++) {
            if(line[i] != '|') {
                strncat(result, &line[i], 1);
            }
            else {
                char space = ' ';
                char pipe = '|';
                strncat(result, &space, 1);
                strncat(result, &pipe, 1);
                strncat(result, &space, 1);
            }
        }

        //Extract commands
        char *token = strtok(result, " ");
        while(token != NULL) {
            commands[count_args++] = token;
            token = strtok(NULL, " ");
        }

        //Handle Absolute path
        for(int i = 0; i < count_args; i++) {
            if(commands[i][0] == '/') {
                memmove(commands[i], commands[i]+1, strlen(commands[i]));
            }
        }

        //Exit Handler (main)
        if(count_args == 1 && strcmp(commands[0], "exit") == 0) {
            printf("3230shell: Terminated\n");
            exit(1);            
        }
        //Exit Handler (other cases)
        if(count_args > 1 && strcmp(commands[0], "exit") == 0) {
            printf("3230shell: \"exit\" with other arguments!!!\n");
            continue;
        }
        else if(count_args > 1 && strcmp(commands[0], "exit") != 0){
            int check = -1;
            for (int i = 1; i < count_args; i++) {
                if(strcmp(commands[i], "exit") == 0) {
                    check = 1;
                }
            }
            if(check == 1) {
                printf("3230shell: \'%s\': No such file or directory\n", commands[0]);
                continue;
            }
        }

        //Pipe Counts
        for(int i = 1; i < count_args; i++){
            if(strcmp(commands[i], "|") == 0 && strcmp(commands[i-1], "|") != 0) {
                count_pipe++;
            }
            else if(strcmp(commands[i], "|") == 0 && strcmp(commands[i-1], "|") == 0) {
                wrong_pipe = 1;
                break;
            }
        }
        
        //Pipe extreme cases handled
        if(strcmp(commands[0], "|") == 0) {
            printf("3230shell: | cannot be the first command\n");
            continue;
        }

        if(wrong_pipe == 1) {
            printf("3230shell: should not have two consecutive | without in-between command\n");
            continue;
        }

        if(count_pipe > 0 && strcmp(commands[count_args-1], "|") == 0) {
            printf("3230shell: | cannot be the last command\n");
            continue;
        }
        
        //Handled 1 count arguments
        if(count_args == 1 && count_pipe == 0) {
            //ls command
            if(strcmp(commands[0], "ls") == 0) {
                signal(SIGUSR1, SIGUSR1Handler);
                pid_t pid = fork();
                process = 1;
                kills = 1; 

                if(pid < 0) {
                    printf("3230shell: \'ls\': Command failed\n");
                }
                else if(pid == 0) {
                    char* args[2];
                    args[0] = strdup(commands[0]);
                    args[1] = NULL;
                    int exec_return = execvp(args[0], args);
                    if(exec_return < 0) {
                        printf("3230shell: \'ls\': Command failed\n");
                        exit(1);
                    }
                }
                else {
                    kill(pid, SIGUSR1);
                    int wait_value = wait(NULL);
                }
            }
            else if(strcmp(commands[0], "cat") == 0 || strcmp(commands[0], "grep") == 0 || strcmp(commands[0], "wc") == 0) {
                //cat, wc, grep errors
                printf("3230shell: \'%s\': Missing filename or arguments\n", commands[0]);
                continue;
            }
            else if(strcmp(commands[0], "timeX") == 0) {
                //timeX errors
                printf("3230shell: \"%s\": cannot be a standalone command\n", commands[0]);
                continue;
            }
            else {
                //Singular arguments
                signal(SIGUSR1, SIGUSR1Handler);
                pid_t pid = fork();
                process = 1;
                kills = 1;

                if(pid < 0) {
                    printf("3230shell: \'%s\': Process failed\n", commands[0]);
                }
                else if(pid == 0) {
                    char* args[2];
                    args[0] = strdup(commands[0]);
                    args[1] = NULL;;
                    int exec_return = execvp(args[0], args);
                    if(exec_return < 0) {
                        printf("3230shell: \'%s\': No such file or directory\n", commands[0]);
                        exit(1);
                    }
                }
                else {
                    kill(pid, SIGUSR1);
                    int wait_value = wait(NULL);
                }                
            }
        }
        //Handled multi-count arguments without pipe
        else if(count_args > 1 && count_pipe == 0) {
            //Multiple ls
            if(strcmp(commands[0], "ls") == 0) {
                signal(SIGUSR1, SIGUSR1Handler);
                pid_t pid = fork();
                process = 1;
                kills = 1;

                if(pid < 0) {
                    printf("3230shell: \'ls\': Command failed\n");
                }
                else if(pid == 0) {
                    if(commands[1][0] != '-') {
                        printf("3230shell: \'ls\': Command failed\n");
                        exit(1);                      
                    }
                    char* args[3];
                    args[0] = strdup(commands[0]);
                    args[1] = strdup(commands[1]);
                    args[2] = NULL;
                    int exec_return = execvp(args[0], args);
                    if(exec_return < 0) {
                        printf("3230shell: \'ls\': Command failed\n");
                        exit(1);
                    }
                }
                else {
                    kill(pid, SIGUSR1);
                    int wait_value = wait(NULL);
                }
            }  
            //Multiple timeX
            else if(strcmp(commands[0], "timeX") == 0) {
                signal(SIGUSR1, SIGUSR1Handler);
                pid_t pid = fork();
                process = 1;
                kills = 1;

                if(pid < 0) {
                    printf("3230shell: \'%s\': Process failed\n", commands[1]);
                }
                else if(pid == 0) {
                    int update_args = count_args-1;
                    char* args[update_args];

                    for(int i = 0; i < update_args; i++) {
                        args[i] = commands[i+1];
                    }
                    args[update_args] = NULL;

                    int exec_return = execvp(args[0], args);
                    if(exec_return < 0) {
                        printf("3230shell: \'%s\': Process failed\n", commands[1]);
                        exit(1);
                    }
                }
                else {
                    kill(pid, SIGUSR1);
                    int status;
                    struct rusage rusage;
                    int wait_value = wait4(pid, &status, 0, &rusage);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value, 
                    commands[1], rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec / 1000000.0, rusage.ru_stime.tv_sec + rusage.ru_stime.tv_usec / 1000000.0);
                }
            }
            //Multiple ps
            else if(strcmp(commands[0], "ps") == 0) {
                signal(SIGUSR1, SIGUSR1Handler);
                pid_t pid = fork();
                process = 1;
                kills = 1;

                if(pid < 0) {
                    printf("3230shell: \'ps\': Command failed\n");
                }
                else if(pid == 0) {
                    char* args[3];
                    args[0] = strdup(commands[0]);
                    args[1] = strdup(commands[1]);
                    args[2] = NULL;
                    int exec_return = execvp(args[0], args);
                    if(exec_return < 0) {
                        printf("3230shell: \'ps\': Command failed\n");
                        exit(1);
                    }
                }
                else {
                    kill(pid, SIGUSR1);
                    int wait_value = wait(NULL);
                }
            }
            else {
                //Multiple Arguments without Pipes 
                signal(SIGUSR1, SIGUSR1Handler);
                pid_t pid = fork();
                process = 1;
                kills = 1;

                if(pid < 0) {
                    printf("3230shell: \'%s\': Process failed\n", commands[0]);
                }
                else if(pid == 0) {
                    char* args[count_args];
                    
                    for(int i = 0; i < count_args; i++) {
                        args[i] = strdup(commands[i]);
                    }
                    args[count_args] = NULL;
                    
                    int exec_return = execvp(args[0], args);
                    if(exec_return < 0) {
                        printf("3230shell: \'%s\': No such file or directory\n", commands[0]);
                        exit(1);
                    }
                }
                else {
                    kill(pid, SIGUSR1);
                    int wait_value = wait(NULL);
                }                   
            }
        }
        //Handled multi count argumets and pipes
        else if(count_args > 1 && count_pipe > 0) {
            //Pipe Count = 1
            if(count_pipe == 1) {
                int pipe_position = -1;
                for(int i = 0; i < count_args; i++) {
                    if(strcmp(commands[i], "|") == 0) {
                        pipe_position = i;
                        break;
                    }
                }

                char* args_1[pipe_position+1];
                int len_args_1 = pipe_position+1;
                for(int i = 0; i < len_args_1-1; i++) {
                    args_1[i] = strdup(commands[i]);
                }
                args_1[len_args_1-1] = NULL;
                
                char* args_2[count_args-pipe_position];
                int len_args_2 = count_args-pipe_position;
                int j = 0;
                for(int i = len_args_1; i < len_args_1+len_args_2-1; i++) {
                    args_2[j++] = strdup(commands[i]);
                }
                args_2[len_args_2-1] = NULL;

                int fd[2];
                pipe(fd);

                //signal(SIGUSR1, SIGUSR1Handler);
                pid_t pid_1 = fork();
                pid_t pid_2;
                process = 1;
                kills = 1;

                if(pid_1 < 0) {
                    printf("3230shell: \'%s\': Process failed\n", args_1[0]);
                }
                else if(pid_1 == 0) {
                    dup2(fd[1], 1);
                    close(fd[0]);
                    close(fd[1]);
                    int exec_return;
                    if(strcmp(args_1[0], "timeX") == 0) {
                        char* args_1_timeX[len_args_1-1];
                        for(int i = 1; i < len_args_1; i++) {
                            args_1_timeX[i-1] = args_1[i];
                        }
                        exec_return = execvp(args_1_timeX[0], args_1_timeX);
                    }
                    else {
                        exec_return = execvp(args_1[0], args_1);
                    }
                    if(exec_return < 0) {
                        printf("3230shell: \'%s\': No such file, command or directory\n", args_1[0]);
                        exit(1);
                    }
                }
                else {
                    //signal(SIGUSR1, SIGUSR1Handler);
                    pid_2 = fork();
                    if(pid_2 < 0) {
                        printf("3230shell: \'%s\': Process failed\n", args_2[0]);
                    }
                    else if(pid_2 == 0) {
                        dup2(fd[0], 0);
                        close(fd[0]);
                        close(fd[1]);
                        int exec_return = execvp(args_2[0], args_2);
                        if(exec_return < 0) {
                            printf("3230shell: \'%s\': No such file, command or directory\n", args_2[0]);
                            exit(1);
                        }
                    }
                }

                close(fd[0]);
                close(fd[1]);

                //kill(pid_1, SIGUSR1);
                int status;
                struct rusage rusage_1;
                struct rusage rusage_2;
                int wait_value_1 = wait4(pid_1, &status, 0, &rusage_1);
                int wait_value_2 = wait4(pid_2, &status, 0, &rusage_2);
                if(strcmp(args_1[0], "timeX") == 0) {
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_1, 
                            args_1[1], rusage_1.ru_utime.tv_sec + rusage_1.ru_utime.tv_usec / 1000000.0, rusage_1.ru_stime.tv_sec + rusage_1.ru_stime.tv_usec / 1000000.0);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_2, 
                            args_2[0], rusage_2.ru_utime.tv_sec + rusage_2.ru_utime.tv_usec / 1000000.0, rusage_2.ru_stime.tv_sec + rusage_2.ru_stime.tv_usec / 1000000.0);
                }
                //kill(pid_2, SIGUSR1);
            }
            //Pipe Count = 2
            else if(count_pipe == 2) {
                int pipe_positions[2];
                int pos = 0;
                for(int i = 0; i < count_args; i++) {
                    if(strcmp(commands[i], "|") == 0) {
                        pipe_positions[pos++] = i;
                    }
                }

                char* args_1[pipe_positions[0]+1];
                int len_args_1 = pipe_positions[0]+1;
                for(int i = 0; i < len_args_1-1; i++) {
                    args_1[i] = strdup(commands[i]);
                }
                args_1[len_args_1-1] = NULL;

                char* args_2[pipe_positions[1]-pipe_positions[0]];
                int len_args_2 = pipe_positions[1]-pipe_positions[0];
                int j = 0;
                for(int i = len_args_1; i < len_args_1+len_args_2; i++) {
                    args_2[j++] = strdup(commands[i]);
                }
                args_2[len_args_2-1] = NULL;

                char* args_3[count_args-pipe_positions[1]];
                int len_args_3 = count_args-pipe_positions[1];
                j = 0;
                for(int i = pipe_positions[1]+1; i < count_args; i++) {
                    args_3[j++] = strdup(commands[i]);
                }
                args_3[len_args_3-1] = NULL;

                int fd[4];
                pipe(fd);
                pipe(fd+2);

                process = 1;
                kills = 1;

                //signal(SIGUSR1, SIGUSR1Handler);
                pid_t pid_1 = fork();
                pid_t pid_2;
                pid_t pid_3;

                if(pid_1 < 0) {
                    printf("3230shell: \'%s\': Process failed\n", args_1[0]);
                }
                else if(pid_1 == 0) {
                    dup2(fd[1], 1);
                    close(fd[0]);
                    close(fd[1]);
                    close(fd[2]);
                    close(fd[3]);
                    int exec_return;
                    if(strcmp(args_1[0], "timeX") == 0) {
                        char* args_1_timeX[len_args_1-1];
                        for(int i = 1; i < len_args_1; i++) {
                            args_1_timeX[i-1] = args_1[i];
                        }
                        exec_return = execvp(args_1_timeX[0], args_1_timeX);
                    }
                    else {
                        exec_return = execvp(args_1[0], args_1);
                    }
                    if(exec_return < 0) {
                        printf("3230shell: \'%s\': No such file, command or directory\n", args_1[0]);
                        exit(1);
                    }                    
                }
                else {
                    //signal(SIGUSR1, SIGUSR1Handler);
                    pid_2 = fork();
                    if(pid_2 < 0) {
                        printf("3230shell: \'%s\': Process failed\n", args_2[0]);
                    }
                    else if(pid_2 == 0) {
                        dup2(fd[0], 0);
                        dup2(fd[3], 1);
                        close(fd[0]);
                        close(fd[1]);
                        close(fd[2]);
                        close(fd[3]);
                        int exec_return = execvp(args_2[0], args_2);
                        if(exec_return < 0) {
                            printf("3230shell: \'%s\': No such file, command or directory\n", args_2[0]);
                            exit(1);
                        }   
                    }
                    else {
                        //signal(SIGUSR1, SIGUSR1Handler);
                        pid_3 = fork();
                        if(pid_3 < 0) {
                            printf("3230shell: \'%s\': Process failed\n", args_3[0]);
                        }
                        else if(pid_3 == 0) {
                            dup2(fd[2], 0);
                            close(fd[0]);
                            close(fd[1]);
                            close(fd[2]);
                            close(fd[3]);
                            int exec_return = execvp(args_3[0], args_3);
                            if(exec_return < 0) {
                                printf("3230shell: \'%s\': No such file, command or directory\n", args_3[0]);
                                exit(1);
                            }   
                        }
                    }
                }

                close(fd[0]);
                close(fd[1]);
                close(fd[2]);
                close(fd[3]);

                //kill(pid_3, SIGUSR1);
                //kill(pid_2, SIGUSR1);
                //kill(pid_1, SIGUSR1);
                int status;
                struct rusage rusage_1;
                struct rusage rusage_2;
                struct rusage rusage_3;
                int wait_value_1 = wait4(pid_1, &status, 0, &rusage_1);
                int wait_value_2 = wait4(pid_2, &status, 0, &rusage_2);
                int wait_value_3 = wait4(pid_3, &status, 0, &rusage_3);
                if(strcmp(args_1[0], "timeX") == 0) {
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_1, 
                            args_1[1], rusage_1.ru_utime.tv_sec + rusage_1.ru_utime.tv_usec / 1000000.0, rusage_1.ru_stime.tv_sec + rusage_1.ru_stime.tv_usec / 1000000.0);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_2, 
                            args_2[0], rusage_2.ru_utime.tv_sec + rusage_2.ru_utime.tv_usec / 1000000.0, rusage_2.ru_stime.tv_sec + rusage_2.ru_stime.tv_usec / 1000000.0);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_3, 
                            args_3[0], rusage_3.ru_utime.tv_sec + rusage_3.ru_utime.tv_usec / 1000000.0, rusage_3.ru_stime.tv_sec + rusage_3.ru_stime.tv_usec / 1000000.0);
                }
            }
            //Pipe Count = 3
            else if(count_pipe == 3) {
                int pipe_positions[3];
                int pos = 0;
                for(int i = 0; i < count_args; i++) {
                    if(strcmp(commands[i], "|") == 0) {
                        pipe_positions[pos++] = i;
                    }
                }

                char* args_1[pipe_positions[0]+1];
                int len_args_1 = pipe_positions[0]+1;
                for(int i = 0; i < pipe_positions[0]; i++) {
                    args_1[i] = strdup(commands[i]);
                }
                args_1[len_args_1-1] = NULL;

                char* args_2[pipe_positions[1]-pipe_positions[0]];
                int len_args_2 = pipe_positions[1]-pipe_positions[0];
                int j = 0;
                for(int i = pipe_positions[0]+1; i < pipe_positions[1]; i++) {
                    args_2[j++] = strdup(commands[i]);
                }
                args_2[len_args_2-1] = NULL;

                char* args_3[pipe_positions[2]-pipe_positions[1]];
                int len_args_3 = pipe_positions[2]-pipe_positions[1];
                j = 0;
                for(int i = pipe_positions[1]+1; i < pipe_positions[2]; i++) {
                    args_3[j++] = strdup(commands[i]);
                }
                args_3[len_args_3-1] = NULL;

                char* args_4[count_args-pipe_positions[2]];
                int len_args_4 = count_args-pipe_positions[2];
                j = 0;
                for(int i = pipe_positions[2]+1; i < count_args; i++) {
                    args_4[j++] = strdup(commands[i]);
                }
                args_4[len_args_4-1] = NULL;

                int fd[6];
                pipe(fd);
                pipe(fd+2);
                pipe(fd+4);

                process = 1;
                kills = 1;

                //signal(SIGUSR1, SIGUSR1Handler);
                pid_t pid_1 = fork();
                pid_t pid_2;
                pid_t pid_3;
                pid_t pid_4;

                if(pid_1 < 0) {
                    printf("3230shell: \'%s\': Process failed\n", args_1[0]);
                }
                else if(pid_1 == 0) {
                    dup2(fd[1], 1);
                    close(fd[0]);
                    close(fd[1]);
                    close(fd[2]);
                    close(fd[3]);
                    close(fd[4]);
                    close(fd[5]);

                    int exec_return;
                    if(strcmp(args_1[0], "timeX") == 0) {
                        char* args_1_timeX[len_args_1-1];
                        for(int i = 1; i < len_args_1; i++) {
                            args_1_timeX[i-1] = args_1[i];
                        }
                        exec_return = execvp(args_1_timeX[0], args_1_timeX);
                    }
                    else {
                        exec_return = execvp(args_1[0], args_1);
                    }
                    if(exec_return < 0) {
                        printf("3230shell: \'%s\': No such file, command or directory\n", args_1[0]);
                        exit(1);
                    }                    
                }
                else {
                    //signal(SIGUSR1, SIGUSR1Handler);
                    pid_2 = fork();
                    if(pid_2 < 0) {
                        printf("3230shell: \'%s\': Process failed\n", args_2[0]);
                    }
                    else if(pid_2 == 0) {
                        dup2(fd[0], 0);
                        dup2(fd[3], 1);
                        close(fd[0]);
                        close(fd[1]);
                        close(fd[2]);
                        close(fd[3]);
                        close(fd[4]);
                        close(fd[5]);
                        int exec_return = execvp(args_2[0], args_2);
                        if(exec_return < 0) {
                            printf("3230shell: \'%s\': No such file, command or directory\n", args_2[0]);
                            exit(1);
                        }   
                    }
                    else {
                        //signal(SIGUSR1, SIGUSR1Handler);
                        pid_3 = fork();
                        if(pid_3 < 0) {
                            printf("3230shell: \'%s\': Process failed\n", args_3[0]);
                        }
                        else if(pid_3 == 0) {
                            dup2(fd[2], 0);
                            dup2(fd[5], 1);
                            close(fd[0]);
                            close(fd[1]);
                            close(fd[2]);
                            close(fd[3]);
                            close(fd[4]);
                            close(fd[5]);
                            int exec_return = execvp(args_3[0], args_3);
                            if(exec_return < 0) {
                                printf("3230shell: \'%s\': No such file, command or directory\n", args_3[0]);
                                exit(1);
                            }   
                        }
                        else {
                            //signal(SIGUSR1, SIGUSR1Handler);
                            pid_4 = fork();
                            if(pid_4 < 0) {
                                printf("3230shell: \'%s\': Process failed\n", args_4[0]);
                            }
                            else if(pid_4 == 0) {
                                dup2(fd[4], 0);
                                close(fd[0]);
                                close(fd[1]);
                                close(fd[2]);
                                close(fd[3]);
                                close(fd[4]);
                                close(fd[5]);
                                int exec_return = execvp(args_4[0], args_4);
                                if(exec_return < 0) {
                                    printf("3230shell: \'%s\': No such file, command or directory\n", args_4[0]);
                                    exit(1);
                                }   
                            }
                        }
                    }
                }

                close(fd[0]);
                close(fd[1]);
                close(fd[2]);
                close(fd[3]);
                close(fd[4]);
                close(fd[5]);

                //kill(pid_1, SIGUSR1);
                //kill(pid_2, SIGUSR1);
                //kill(pid_3, SIGUSR1);      
                //kill(pid_4, SIGUSR1);      
                int status;
                struct rusage rusage_1;
                struct rusage rusage_2;
                struct rusage rusage_3;
                struct rusage rusage_4;
                int wait_value_1 = wait4(pid_1, &status, 0, &rusage_1);
                int wait_value_2 = wait4(pid_2, &status, 0, &rusage_2);
                int wait_value_3 = wait4(pid_3, &status, 0, &rusage_3);
                int wait_value_4 = wait4(pid_4, &status, 0, &rusage_4);
                if(strcmp(args_1[0], "timeX") == 0) {
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_1, 
                            args_1[1], rusage_1.ru_utime.tv_sec + rusage_1.ru_utime.tv_usec / 1000000.0, rusage_1.ru_stime.tv_sec + rusage_1.ru_stime.tv_usec / 1000000.0);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_2, 
                            args_2[0], rusage_2.ru_utime.tv_sec + rusage_2.ru_utime.tv_usec / 1000000.0, rusage_2.ru_stime.tv_sec + rusage_2.ru_stime.tv_usec / 1000000.0);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_3, 
                            args_3[0], rusage_3.ru_utime.tv_sec + rusage_3.ru_utime.tv_usec / 1000000.0, rusage_3.ru_stime.tv_sec + rusage_3.ru_stime.tv_usec / 1000000.0);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_4, 
                            args_4[0], rusage_4.ru_utime.tv_sec + rusage_4.ru_utime.tv_usec / 1000000.0, rusage_4.ru_stime.tv_sec + rusage_4.ru_stime.tv_usec / 1000000.0);
                }
            }
            //Pipe count = 4
            else if(count_pipe == 4) {
                int pipe_positions[4];
                int pos = 0;
                for(int i = 0; i < count_args; i++) {
                    if(strcmp(commands[i], "|") == 0) {
                        pipe_positions[pos++] = i;
                    }
                }

                char* args_1[pipe_positions[0]+1];
                int len_args_1 = pipe_positions[0]+1;
                for(int i = 0; i < pipe_positions[0]; i++) {
                    args_1[i] = strdup(commands[i]);
                }
                args_1[len_args_1-1] = NULL;

                char* args_2[pipe_positions[1]-pipe_positions[0]];
                int len_args_2 = pipe_positions[1]-pipe_positions[0];
                int j = 0;
                for(int i = pipe_positions[0]+1; i < pipe_positions[1]; i++) {
                    args_2[j++] = strdup(commands[i]);
                }
                args_2[len_args_2-1] = NULL;

                char* args_3[pipe_positions[2]-pipe_positions[1]];
                int len_args_3 = pipe_positions[2]-pipe_positions[1];
                j = 0;
                for(int i = pipe_positions[1]+1; i < pipe_positions[2]; i++) {
                    args_3[j++] = strdup(commands[i]);
                }
                args_3[len_args_3-1] = NULL;

                char* args_4[pipe_positions[3]-pipe_positions[2]];
                int len_args_4 = pipe_positions[3]-pipe_positions[2];
                j = 0;
                for(int i = pipe_positions[2]+1; i < pipe_positions[3]; i++) {
                    args_4[j++] = strdup(commands[i]);
                }
                args_4[len_args_4-1] = NULL;

                char* args_5[count_args-pipe_positions[3]];
                int len_args_5 = count_args-pipe_positions[3];
                j = 0;
                for(int i = pipe_positions[3]+1; i < count_args; i++) {
                    args_5[j++] = strdup(commands[i]);
                }
                args_5[len_args_5-1] = NULL;

                int fd[6];
                pipe(fd);
                pipe(fd+2);
                pipe(fd+4);
                pipe(fd+6);

                process = 1;
                kills = 1;

                //signal(SIGUSR1, SIGUSR1Handler);
                pid_t pid_1 = fork();
                pid_t pid_2;
                pid_t pid_3;
                pid_t pid_4;
                pid_t pid_5;

                if(pid_1 < 0) {
                    printf("3230shell: \'%s\': Process failed\n", args_1[0]);
                }
                else if(pid_1 == 0) {
                    dup2(fd[1], 1);
                    close(fd[0]);
                    close(fd[1]);
                    close(fd[2]);
                    close(fd[3]);
                    close(fd[4]);
                    close(fd[5]);
                    close(fd[6]);
                    close(fd[7]);

                    int exec_return;
                    if(strcmp(args_1[0], "timeX") == 0) {
                        char* args_1_timeX[len_args_1-1];
                        for(int i = 1; i < len_args_1; i++) {
                            args_1_timeX[i-1] = args_1[i];
                        }
                        exec_return = execvp(args_1_timeX[0], args_1_timeX);
                    }
                    else {
                        exec_return = execvp(args_1[0], args_1);
                    }
                    if(exec_return < 0) {
                        printf("3230shell: \'%s\': No such file, command or directory\n", args_1[0]);
                        exit(1);
                    }                    
                }
                else {
                    //signal(SIGUSR1, SIGUSR1Handler);
                    pid_2 = fork();
                    if(pid_2 < 0) {
                        printf("3230shell: \'%s\': Process failed\n", args_2[0]);
                    }
                    else if(pid_2 == 0) {
                        dup2(fd[0], 0);
                        dup2(fd[3], 1);
                        close(fd[0]);
                        close(fd[1]);
                        close(fd[2]);
                        close(fd[3]);
                        close(fd[4]);
                        close(fd[5]);
                        int exec_return = execvp(args_2[0], args_2);
                        if(exec_return < 0) {
                            printf("3230shell: \'%s\': No such file, command or directory\n", args_2[0]);
                            exit(1);
                        }   
                    }
                    else {
                        //signal(SIGUSR1, SIGUSR1Handler);
                        pid_3 = fork();
                        if(pid_3 < 0) {
                            printf("3230shell: \'%s\': Process failed\n", args_3[0]);
                        }
                        else if(pid_3 == 0) {
                            dup2(fd[2], 0);
                            dup2(fd[5], 1);
                            close(fd[0]);
                            close(fd[1]);
                            close(fd[2]);
                            close(fd[3]);
                            close(fd[4]);
                            close(fd[5]);
                            close(fd[6]);
                            close(fd[7]);
                            int exec_return = execvp(args_3[0], args_3);
                            if(exec_return < 0) {
                                printf("3230shell: \'%s\': No such file, command or directory\n", args_3[0]);
                                exit(1);
                            }   
                        }
                        else {
                            //signal(SIGUSR1, SIGUSR1Handler);
                            pid_4 = fork();
                            if(pid_4 < 0) {
                                printf("3230shell: \'%s\': Process failed\n", args_4[0]);
                            }
                            else if(pid_4 == 0) {
                                dup2(fd[4], 0);
                                dup2(fd[7], 1);
                                close(fd[0]);
                                close(fd[1]);
                                close(fd[2]);
                                close(fd[3]);
                                close(fd[4]);
                                close(fd[5]);
                                close(fd[6]);
                                close(fd[7]);
                                int exec_return = execvp(args_4[0], args_4);
                                if(exec_return < 0) {
                                    printf("3230shell: \'%s\': No such file, command or directory\n", args_4[0]);
                                    exit(1);
                                }   
                            }
                            else {
                                //signal(SIGUSR1, SIGUSR1Handler);
                                pid_5 = fork();
                                if(pid_5 < 0) {
                                    printf("3230shell: \'%s\': Process failed\n", args_5[0]);
                                }
                                else if(pid_5 == 0) {
                                    dup2(fd[6], 0);
                                    close(fd[0]);
                                    close(fd[1]);
                                    close(fd[2]);
                                    close(fd[3]);
                                    close(fd[4]);
                                    close(fd[5]);
                                    close(fd[6]);
                                    close(fd[7]);
                                    int exec_return = execvp(args_5[0], args_5);
                                    if(exec_return < 0) {
                                        printf("3230shell: \'%s\': No such file, command or directory\n", args_5[0]);
                                        exit(1);  
                                    }                                  
                                }
                            }
                        }
                    }                    
                }

                close(fd[0]);
                close(fd[1]);
                close(fd[2]);
                close(fd[3]);
                close(fd[4]);
                close(fd[5]);
                close(fd[6]);
                close(fd[7]);

                //kill(pid_1, SIGUSR1);
                //kill(pid_2, SIGUSR1);
                //kill(pid_3, SIGUSR1);      
                //kill(pid_4, SIGUSR1); 
                //kill(pid_5, SIGUSR1);
                int status;
                struct rusage rusage_1;
                struct rusage rusage_2;
                struct rusage rusage_3;
                struct rusage rusage_4;
                struct rusage rusage_5;
                int wait_value_1 = wait4(pid_1, &status, 0, &rusage_1);
                int wait_value_2 = wait4(pid_2, &status, 0, &rusage_2);
                int wait_value_3 = wait4(pid_3, &status, 0, &rusage_3);
                int wait_value_4 = wait4(pid_4, &status, 0, &rusage_4);
                int wait_value_5 = wait4(pid_5, &status, 0, &rusage_5);
                if(strcmp(args_1[0], "timeX") == 0) {
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_1, 
                            args_1[1], rusage_1.ru_utime.tv_sec + rusage_1.ru_utime.tv_usec / 1000000.0, rusage_1.ru_stime.tv_sec + rusage_1.ru_stime.tv_usec / 1000000.0);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_2, 
                            args_2[0], rusage_2.ru_utime.tv_sec + rusage_2.ru_utime.tv_usec / 1000000.0, rusage_2.ru_stime.tv_sec + rusage_2.ru_stime.tv_usec / 1000000.0);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_3, 
                            args_3[0], rusage_3.ru_utime.tv_sec + rusage_3.ru_utime.tv_usec / 1000000.0, rusage_3.ru_stime.tv_sec + rusage_3.ru_stime.tv_usec / 1000000.0);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_4, 
                            args_4[0], rusage_4.ru_utime.tv_sec + rusage_4.ru_utime.tv_usec / 1000000.0, rusage_4.ru_stime.tv_sec + rusage_4.ru_stime.tv_usec / 1000000.0);
                    printf("(PID)%d   (CMD)%s    (user)%.3f s  (sys)%.3f\n", wait_value_5, 
                            args_5[0], rusage_5.ru_utime.tv_sec + rusage_5.ru_utime.tv_usec / 1000000.0, rusage_5.ru_stime.tv_sec + rusage_5.ru_stime.tv_usec / 1000000.0);
                }
            }
            //Pipe count > 4
            else if(count_pipe > 4) {
                printf("3230shell: \'|\': Cannot handle more than 5 commands\n");
                continue;
            }
        }
    }

    return(0);
}

    
