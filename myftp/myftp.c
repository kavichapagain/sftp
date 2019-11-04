//
// 

#include <sys/wait.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "token.h"

#define PORT 40714
#define BUFSIZE 256


int cd(char *pth){
    char path[BUFSIZE];
    strcpy(path,pth);

    char cwd[BUFSIZE];
    if(pth[0] != '/')
    {// true for the dir in cwd
        getcwd(cwd,sizeof(cwd));
        strcat(cwd,"/");
        strcat(cwd,path);
        chdir(cwd);
    }else{//true for dir w.r.t. /
        chdir(pth);
    }

    return 0;
}

/**
 * Run client commands
 * 
 */
int execClientCommands(int commandNumber, char * token [], int totalTokens){
    int execState;
    int pid, status, wpid;
    // char * args [3];
    char * args[2];
    char s[100];

    if ((pid=fork()) <0) {
        perror("fork"); 
        exit(1);
    } 
    else if (pid == 0) { 
        
        printf("child\n\n");
        printf("Command %s\n", token[0]);

        if (commandNumber == 1){ //pwd
            printf("%s\n", getcwd(s, 100));
            // args[0] = "pwd";
            // token[totalTokens] = NULL;
            // execvp(token[0], token);
        }
        else if (commandNumber == 3) { //dir/ls
            token[0] = "ls";
            token[totalTokens] = getcwd(s, 100);
            token[totalTokens+1] = NULL;
            execvp(token[0], token);
        }
        else if (commandNumber == 5) { //change dir
            
            // the second token is the path
            chdir(token[1]); 

            // printing current working directory 
            printf("cwd - %s\n", getcwd(s, 100));
            
        }

        exit(0);
         /* parent to wait for next client */
    }
    
    printf("parent");

    printf("PID Process: %d Raised child: %d\n", getpid(), pid);
    wpid = wait(&status);
    printf("%i", (int)wpid);
    
    return 0;
}


/**
 * init client and recv commands
 * 
 * 
 */ 
int initClient()
{
    int socketfd;
    struct sockaddr_in serverAddress;
    char buffer[100];
    

    //create a socket
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("CLIENT: Failed while creating a Socket\n");
        close(socketfd);
        return -1;
    }
    printf("Client: Created a socket successfully, socketfd: %i\n", socketfd);

    bzero(&serverAddress, sizeof(serverAddress)); //mem

    //connect to the server
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(PORT);

    // printf("%i", connect(socketfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)));

    if (connect(socketfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
       printf("\nClient: Connection Failed \n");
       return -1;
    }

    //TODO: Don't forget to close the conn in the end
    printf("Connected to the server.\n"); 

    return socketfd;
}
    

int main(int argc, char* argv[]) {

    int socketfd, n;
    struct sockaddr_in serverAddress;
    char buffer[BUFSIZE], command[BUFSIZE];
    int totTokens;
    char *token[MAX_NUM_TOKENS];
    char *commands[] = {"pwd", "lpwd", "dir", "ldir", "cd", "lcd", "get", "put", "quit"};
    bool isValidCommand = false;

    socketfd = initClient();
    printf("Enter the command: ");
    while (1) {
        printf("\n> ");

        fgets(buffer, BUFSIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0)
            break;
        
        totTokens = tokenise(buffer, token);
      
        printf("added new line, %s\n", token[0]);
        //check if the command is valid using the loop
        for (int i = 0; i < 9; i++){
            if(strcmp(token[0], commands[i]) == 0){
                isValidCommand = true;

                //quit
                if (strcmp("quit", commands[i]) == 0){
                    printf("Quitting the program!");
                    return 0;
                }

                //the client command
                else if ('l' == commands[i][0]){
                    printf("Received client command, %s\n", commands[i]);
                    token[0] = commands[i-1];
                    // for (size_t j=1; commands[i][j]; j++) {
                    //      command[i] = commands[i-1];
                    //      printf("%c  %c\n", command[j-1], commands[i][j]);
                    // }
                    printf("%s\n", token[0]);
                    execClientCommands(i, token, totTokens); // call command handler func
                }

                //the server command
                else{
                    printf("Received server command\n");
                    // send & recv ;
                    strcpy(buffer, commands[i]);
                    send(socketfd, buffer, 100, 0);
                    recv(socketfd, buffer, 100, 0);
                    printf("%s \n", buffer);
                    exit(0);
                }
            }
            //  else{
            //      printf("myftp:  Command\n");
            //  }
        }

        if (!isValidCommand){
            printf("Invalid Command");
        
        }
    }
    

    return 0;
}

