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
#include <fcntl.h>
#include <sys/stat.h>

#include "token.h"

#define PORT 40714
#define BUFSIZE (1024*256)

// Global that holds current directory client
char g_path[255];

//Message structure
typedef struct {
    char opCode;
    int messageSize;
    char message[512];
} msgStruct;

typedef struct {
    char opCode;
    int messageSize;
    int ackCode;
    char message[512];
} msgStructServer;



/**
 * init client and recv commands
 * 
 * 
 */ 
int initClient()
{
    int socketfd;
    struct sockaddr_in serverAddress;
    char buffer[100], buffer2[100];

    //create a socket
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Client: Failed while creating a Socket\n");
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

    printf("Connected to the server.\n"); 

    return socketfd;
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
        
        // printf("child\n\n");
        // printf("Command %s, %i\n", token[0], commandNumber);

        if (commandNumber == 1){ //pwd
            
            printf("%s\n", g_path);//getcwd(s, 100));
            // args[0] = "pwd";
            // token[totalTokens] = NULL;
            // execvp(token[0], token);
        }
        else if (commandNumber == 3) { //dir/ls
        
            // printf("%s\n", g_path);
            token[0] = "ls";
            token[totalTokens] =  g_path; 
            token[totalTokens+1] = NULL;
            execvp(token[0], token);
        }
       

        exit(0);
         /* parent to wait for next client */
    }
    
    
    if (commandNumber == 5) { //change parent dir
        // printf("Command lcd");
        if (chdir(token[1]) >= 0){
            getcwd(g_path, 100);
        } 
        else {
            perror("lcd");
        }
    }

    // printf("PID Process: %d Raised child: %d\n", getpid(), pid);
    wpid = wait(&status);
    // printf("%i", (int)wpid);
    // printf("s - %s,g_p %s\n", s, g_path);
    
    return 0;
}

/**
 * 
 * Logger, writes to log.txt
 */

void logger(char * log) {
	FILE *file;
	file = fopen("log.txt","a");
	if(file != NULL) {
		fprintf(file,"%s\n",log);
		fclose(file);
	}
}


    

int main(int argc, char* argv[]) {

    int socketfd, n, nr, nw, count = 0;
    long fileLength = 0;
    struct sockaddr_in serverAddress;
    char buffer[BUFSIZE], command[BUFSIZE], buffer2[BUFSIZE], log[255];
    int totTokens;
    char *token[3];
    char *commands[] = {"pwd", "lpwd", "dir", "ldir", "cd", "lcd", "get", "put", "quit"};
    bool isValidCommand = false;
    msgStruct clientMsg;
    // FILE *receivedFile;
    struct stat statObj;
    // msgStructServer serverMessage;

    logger("\nStarting Client\n");

    if ((socketfd = initClient()) >= 0){
        logger("Successfully connected to the server\n");
        printf("Enter a command: ");

        while (++count) {
            getcwd(g_path, 255); //set cwd to global path
            printf("\n> ");

            msgStructServer serverMessage;
            fgets(buffer, BUFSIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            
            totTokens = tokenise(buffer, token);
            
            //check if the command is valid using the loop
            for (int i = 0; i < 9; i++){
                if(strcmp(token[0], commands[i]) == 0){
                    isValidCommand = true;

                    if (strcmp("quit", commands[i]) == 0){
                        printf("Terminating the client!\n");
                        sprintf(log, "client[%d]: Received quit command %s", count, commands[i]);
                        logger(log); 
                        close(socketfd);
                        return 0;
                    }

                    //the client commands
                    else if (i == 1 || i == 3 || i == 5){
                        sprintf(log, "client[%d]: Received client command %s", count, commands[i]);
                        logger(log);
                        token[0] = commands[i-1];
                        // printf("%s\n", token[0]);
                        execClientCommands(i, token, totTokens); // call command handler func
                    }
                    // server commands
                    else if (i == 0 || i == 2){
                        sprintf(log, "client[%d]: Received server command %s", count, commands[i]);
                        logger(log);

                        clientMsg.opCode = i +'0';
                        // clientMsg.messageSize = sizeof(token[1]);
                        // strcpy(clientMsg.message, token[1]);
                        nw = write(socketfd, &clientMsg, sizeof(clientMsg));

                        char  buffer[100];
                        int recvFileSize;
                        read(socketfd, &buffer, 100);
                        recvFileSize = atoi(buffer);
                        char msgServer[BUFSIZE];

                         while (recvFileSize > 0){
                            nr = read(socketfd, &msgServer, sizeof(msgServer));
                            recvFileSize -=nr;
                            sprintf(log, "client[%d]: %d bytes received", count, nr);
                            logger(log);
                            printf("%s\n", msgServer);
                        }
                        sprintf(log, "client[%d]: Completed server command %s, ack %i", count, commands[i], serverMessage.ackCode);
                        logger(log); 
                    }
                    else if (i == 4 ){
                        sprintf(log, "client[%d]: Received server command %s", count, commands[i]);
                        logger(log);
                        if (totTokens < 2) {
                            sprintf(log, "client[%d]: Path not provided", count); 
                            logger(log);
                            perror("No path provided");
                            continue;
                        }

                        
                        clientMsg.opCode = i +'0';
                        clientMsg.messageSize = sizeof(token[1]);
                        strcpy(clientMsg.message, token[1]);

                        write(socketfd, &clientMsg, sizeof(clientMsg));

                        // char  buffer[100];
                        // int recvFileSize;
                        // read(socketfd, &buffer, 100);
                        // printf("filelen: %s", buffer);
                        // recvFileSize = atoi(buffer);
                        // char msgServer[BUFSIZE];

                        // while (recvFileSize > 0){
                        //     nr = read(socketfd, &msgServer, sizeof(msgServer));
                        //     // fwrite(msgServer, sizeof(char), nr, receivedFile);
                        //     recvFileSize -=nr;
                        //     printf("Message From Server: %i, %s\n", nr, msgServer);
                        // }  
                            
                        nr = read(socketfd, &serverMessage, sizeof(serverMessage));
                       
                        sprintf(log, "client[%d]: Completed server command %s, ack %i", count, commands[i], serverMessage.ackCode);
                        logger(log); 
                    }
                    else if (i == 6 ){
                        sprintf(log, "client[%d]: Received server command %s", count, commands[i]);
                        logger(log);
                        if (totTokens < 2) {
                            sprintf(log, "client[%d]: Path not provided", count);
                            logger(log); 
                            perror("No path provided");
                            continue;
                        }

                        clientMsg.opCode = i +'0';
                        clientMsg.messageSize = sizeof(token[1]);
                        strcpy(clientMsg.message, token[1]);

                        write(socketfd, &clientMsg, sizeof(clientMsg));

                        char  buffer[100];
                        int recvFileSize;
                        read(socketfd, &buffer, 100);
                        recvFileSize = atoi(buffer);
                        // printf("Size %s", buffer);

                        char locate[100];
                        char msgServer[BUFSIZE];
                        strcpy(locate, g_path);
                        locate[strcspn(locate, "\n")] = 0;
                        strcat(locate, "/");
                        strcat(locate, token[1]);
                        //remove null byte if exsiting
                        // locate[strlen(locate)-1] = '\0';
                        


                        //Gets the file name and saves to download directory
                        int fd = open(locate, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd < 0) 
                        { 
                            printf("%s", locate);
                            perror("download"); 
                        } 

                        // receivedFile = fopen(locate, "w");

                        // printf("Message From Server: %i, %s\n", nr, serverMessage.message);
                        while (recvFileSize > 0){
                            nr = read(socketfd, &msgServer, sizeof(msgServer));
                            int sz = write(fd, msgServer, nr); 
                            sprintf(log, "client[%d]: %d bytes received", count, nr);
                            logger(log);
                            recvFileSize -=nr;
                        }    
                        printf("Download Completed\n");
                        close(fd); 
                        sprintf(log, "client[%d]: Completed server command %s, ack %i", count, commands[i], serverMessage.ackCode);
                        logger(log);
                    }
                    else if (i == 7){
                       sprintf(log, "client[%d]: Received server command %s", count, commands[i]);
                       logger(log);
                        if (totTokens < 2) {
                            sprintf(log, "client[%d]: Path not provided", count);
                            logger(log); 
                            perror("No path provided");
                            continue;
                        }

                        char locate[100];
                        char msgServer[BUFSIZE];
                        strcpy(locate, g_path);
                        locate[strcspn(locate, "\n")] = 0;
                        strcat(locate, "/");
                        strcat(locate, token[1]);
                        
                        // printf("%s", locate);
                        int fd = open(locate, O_RDONLY);			// Open the file
                        if (fd < 0) 
                        { 
                            printf("%s", locate);
                            perror("upload"); 
                        } 

                        clientMsg.opCode = i +'0';
                        clientMsg.messageSize = sizeof(token[1]);
                        strcpy(clientMsg.message, token[1]);

                        write(socketfd, &clientMsg, sizeof(clientMsg));

                        nr = read(socketfd, &serverMessage, sizeof(serverMessage));
                        printf("server: %i\n", serverMessage.ackCode);

                        if (serverMessage.ackCode == 0){
                            stat(locate, &statObj);
                            fileLength = statObj.st_size;
                            char fileBuffer[fileLength];	
                            
                            read(fd, &fileBuffer, fileLength);
                            
                            int fl = fileLength;
                            char c[100];
                            sprintf(c, "%li", fileLength);
                            // printf("c --- %s", c);

                        
                            write(socketfd, c, sizeof(c));
                            // printf("c- %s", c);

                            while (fl > 0){
                                int bytesWritten = write(socketfd, fileBuffer, fileLength);
                                fl -=bytesWritten;
                                sprintf(log, "client[%d]: %d bytes sent", count, bytesWritten);
                                logger(log); 
                                // printf("%i\n", bytes_written);
                            }
                            printf("Upload Completed\n");
                            close(fd);
                            sprintf(log, "client[%d]: Completed server command %s, ack %i", count, commands[i], serverMessage.ackCode);
                            logger(log); 
                        }
                        else {
                            printf("Error from server");
                        }

                    }
                }
               
            }

            if (!isValidCommand){
                sprintf(log, "client[%d]: Invalid command ", count);
                logger(log); 
                printf("Invalid Command\n");
            }
        }
        
    }
    sprintf(log, "client: Terminating ");
    logger(log);
    close(socketfd); 
    
    return 0;
}

