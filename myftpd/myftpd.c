//Kavi Chapagain
//

#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define KAVI 40714
#define PORT 40714
#define BUFSIZE (1024*256)



// Global that holds current directory
char g_path[255];

typedef struct {
    char opCode;
    int messageSize;
    char message[512];
} msgStruct;

typedef struct {
    char opCode;
    int messageSize;
    int ackCode;
    char * message;
} msgStructServer;


/*
 * Initilise the server
 * params:
 */
int initServer(){
    struct  sockaddr_in serverAddress, clientaddr;
    int socketfd;

    //create a socket
    //using AF_INET - IPv4 
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Error: Socket Create Failed\n");
        close(socketfd);
        return -1;
    }
    printf("Created a socket successfully\n");
    bzero(&serverAddress, sizeof(serverAddress)); //memset
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_addr.s_addr =  htonl(INADDR_ANY); //order of bytes
    serverAddress.sin_port = htons(PORT);

    //bind a socket
    if (bind(socketfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        printf("Error: Socket Bind Failed\n");
        close(socketfd);
        return  -1;
    }
    printf("Binded socket successfully\n");

    //start listening, max 5 pending connections
    if(listen(socketfd, 5) < 0){
        printf("Error: Listen Failed \n");
        return -1;
    }
    printf("Server Listening\n");

    return (socketfd);
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

/**
 * Claim Zombies processes
 */ 
void claimZombies() {
	pid_t pid;

	while(pid > 0) {
		pid = waitpid(0, (int*)0, WNOHANG); //claim
	}
}

/**
 * Run the server as daemon 
 */

void initDaemon(void) {
	pid_t pid;
	struct sigaction act;

	if ((pid = fork()) < 0) {
		exit(1);
	} else if (pid != 0) {
		exit(0);
	}

	setsid(); //become session leader
	umask(0); //clear file mode 
	
	act.sa_handler = claimZombies;
	sigemptyset(&(act.sa_mask));
	act.sa_flags = SA_NOCLDSTOP;
	
	if((sigaction(SIGCHLD, &act, (struct sigaction*)0)) != 0) {
		exit(1);
	}
}


void serveClient(int sd)
{   int nr, nw, i=0, filefd;
    char buf[BUFSIZE], buf1[BUFSIZE];
    char sendBuffer[1024], fileLenStr[100], log[256];
    char * token [2];
    long filelength = 0;
    char *toClient = (char *) calloc(1024, sizeof(char)); 
    int toClientSize = 0;
    msgStruct clientMessage;
    msgStructServer serverMessage;
    struct stat obj;
    char * fullFilepath;
    char msgServer[BUFSIZE];

    while (++i){
        getcwd(g_path, 255); //set cwd to global path

        // read the command from client
        if ((nr = read(sd, &clientMessage, 1024)) <= 0) 
            exit(0);   /* connection broken down */


        if (clientMessage.opCode == '0') {
            // logger("Command: pwd\n");
            sprintf(log, "server[%d]: received command pwd\n", i); 
            logger(log);

            sprintf(fileLenStr, "%li", sizeof(g_path));
            write(sd, fileLenStr, sizeof(fileLenStr));
            
            nw = write(sd, g_path, sizeof(g_path));
            
            sprintf(log, "server[%d]: completed. %d bytes sent out\n", i, nw); 
            logger(log);
        }
        else if (clientMessage.opCode ==  '2'){
            sprintf(log, "server[%d]: received command dir\n", i); 
            logger(log);
            system("ls>temp.txt");		// Run the command line ls
			stat("temp.txt", &obj);		// Read the properties of the file into the obj struct
			filelength = obj.st_size;
            char fileBuffer[filelength];	
			filefd = open("temp.txt", O_RDONLY);

            if (filefd < 0) { 
                perror("reading dir"); 
                serverMessage.opCode = '2';
                serverMessage.ackCode = 1;
            }
            else{
                toClientSize = read(filefd, fileBuffer, filelength);

                sprintf(fileLenStr, "%li", filelength);
                write(sd, fileLenStr, sizeof(fileLenStr));
                
                nw = write(sd, fileBuffer, sizeof(fileBuffer));
                printf("server[%d]: %d bytes sent out, %s\n", i, nw, fileBuffer); 

            } 
            sprintf(log, "server[%d]: completed. %d bytes sent out\n", i, nw); 
            logger(log);

            // nw = write(sd, &serverMessage, sizeof(serverMessage));
            // printf("server[%d]: %d bytes sent out, %s\n", i, nw, serverMessage.message); 
        }
        else if (clientMessage.opCode == '4') {
            sprintf(log, "server[%d]: received command cd \n", i); 
            logger(log);
            // printf("Command: cd\n");
            if (chdir(clientMessage.message) >= 0){
                printf("%s\n",getcwd(g_path, 100));
                strcpy(sendBuffer, g_path);
               
                serverMessage.opCode = '4';
                serverMessage.ackCode = 0;
                serverMessage.messageSize = 0;
                // strcpy(serverMessage.message, ""); //dyld: lazy symbol binding failed: symbol is not in trie
                
            }
            else {
                perror("dir error");
                serverMessage.opCode = '4';
                serverMessage.ackCode = 1;
                // strcpy(sendBuffer, "1"); ;
            }

            nw = write(sd, &serverMessage, sizeof(serverMessage));
            sprintf(log, "server[%d]: completed. %d bytes sent out\n", i, nw); 
            logger(log);
            // printf("server[%d]: %d bytes sent out, %i\n", i, nw, serverMessage.ackCode); 
            
        }
        else if(clientMessage.opCode == '6'){
            sprintf(log, "server[%d]: received command get \n", i); 
            logger(log);
            fullFilepath = strcat(g_path, "/");
            fullFilepath = strcat(fullFilepath, clientMessage.message);

            if(stat(fullFilepath, &obj) == 0){	
                filefd = open(fullFilepath, O_RDONLY);			// Open the file
                
                if (filefd < 0) { 
                    perror("get"); 
                    serverMessage.opCode = '6';
                    serverMessage.ackCode = 1;

                    char c[100];
                    sprintf(c, "%i", 1);
                    write(sd, c, sizeof(c));
                    sprintf(log, "server[%d]: cannot open the file", i);
                    logger(log);
                    printf("%s", log);

                    // strcpy(serverMessage.message, "Error");
                    // serverMessage.me
                    // write(sd, &serverMessage, sizeof(serverMessage));
                }
                else{
                    filelength = obj.st_size;
                    char fileBuffer[filelength];	
                    
                    int r =read(filefd, &fileBuffer, filelength);

                    serverMessage.opCode = '6';
                    serverMessage.ackCode = 0;
                    serverMessage.messageSize = sizeof(filelength);
                    // strcpy(serverMessage.message, fileBuffer);

                    long fl = filelength;

                    char c[100];
                    sprintf(c, "%lli", filelength);
                    write(sd, c, sizeof(c));
                    while (fl > 0){
                        nw = write(sd, fileBuffer, filelength);
                        fl -=nw;
                        sprintf(log, "server[%d]: %d bytes sent", i, nw);
                        logger(log);
                    }
                }
                printf("get finished\n");
            }
            else {
                perror("get"); 
                serverMessage.opCode = '6';
                serverMessage.ackCode = 2;

                char c[100];
                sprintf(c, "%i", 2);
                write(sd, c, sizeof(c));
            }
            sprintf(log, "server[%d]: get completed\n", i); 
            logger(log);
        }
        else if (clientMessage.opCode == '7'){
            sprintf(log, "server[%d]: received command put \n", i); 
            logger(log);

            fullFilepath = strcat(g_path, "/");
            fullFilepath = strcat(fullFilepath, clientMessage.message);
            //only if the file does not exist
            if (stat(fullFilepath, &obj) != 0)	{
                serverMessage.opCode = '7';
                serverMessage.ackCode = 0;
                write(sd, &serverMessage, sizeof(serverMessage));

                int fd = open(fullFilepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) 
                {
                   
                    serverMessage.opCode = '7';
                    serverMessage.ackCode = 2;
                    write(sd, &serverMessage, sizeof(serverMessage));
            
                    sprintf(log, "server[%d]: cannot create a file \n", i); 
                    logger(log);
                    perror("server get"); 
                }
                else {
                    char  buffer[100];
                    int recvFileSize;
                    read(sd, &buffer, 100);
                    recvFileSize = atoi(buffer);

                    while (recvFileSize > 0){
                        nr = read(sd, &msgServer, sizeof(msgServer));
                        int sz = write(fd, msgServer, nr); 
                        recvFileSize -=nr;
                        sprintf(log, "server[%d]: %d bytes received", i, nr);
                        logger(log);
                    }
                } 

                printf("put finished\n");
                close(fd);
                sprintf(log, "server[%d]: put completed\n", i); 
                logger(log); 
            }
            else {
                serverMessage.opCode = '7';
                serverMessage.ackCode = 1;
                write(sd, &serverMessage, sizeof(serverMessage));
            }
        }
        else{
            // printf("else .. %c", clientMessage.opCode);
        }
    } 
    close(sd);
}


int main(){
    int socketfd, connectionfd, cli_addrlen;
    pid_t pid;
    int runFlag = 1;
    struct  sockaddr_in clientaddr;
    char *command_buffer;
    char buffer[100];
    int addrlen = sizeof(struct sockaddr_in);

    logger("\nStarting Daemon Server\n");
    printf("Strating myftpd\n");
    // initDaemon();
    // initilise the server
    socketfd = initServer();

    while (1) {

        //wait to accept a client 
        cli_addrlen = sizeof(clientaddr);
        connectionfd = accept(socketfd, (struct sockaddr *) &clientaddr, (socklen_t *)&cli_addrlen);

        // create a child process to 
        if ((pid=fork()) <0) {
            perror("fork"); exit(1);
        } else if (pid > 0) { 
            close(connectionfd);
            continue; //parent, for next client accept
        }

        // serve the current client 
        close(socketfd); 
        logger("server: Serving a new client\n");
        serveClient(connectionfd);
    }

    logger("\nTerminating the Server\n");

    return 0;
}
