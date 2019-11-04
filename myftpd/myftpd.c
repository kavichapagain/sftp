//
// server.c
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


#define KAVI 40714
#define PORT 40714
#define BUFSIZE 512

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

void reverse(char *s, int len)
{
    char c;   
    int i, j;

    for (i=0, j = len-1; i<j; i++, j--) {
        c = s[i]; s[i] = s[j]; s[j] = c;
    }
}

void serveClient(int sd)
{   int nr, nw, i=0;
    char buf[BUFSIZE];
    

    while (++i){
        printf("serving client");

         /* read data from client */
         if ((nr = read(sd, buf, sizeof(buf))) <= 0) 
             exit(0);   /* connection broken down */
         printf("server[%d]: %d bytes received\n", i, nr);

         /* process the data we have received */
         reverse(buf, nr);
         printf("server[%d]: %d bytes processed\n", i, nr); 

         /* send results to client */
         nw = write(sd, buf, nr);
         printf("server[%d]: %d bytes sent out\n", i, nw); 
    } 
}


int main(){

    // int sd, nsd, n, cli_addrlen;  pid_t pid;
    //  struct sockaddr_in ser_addr, cli_addr; 
     
     /* turn the program into a daemon */
    //  daemon_init();  

     /* set up listening socket sd */
    //  if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    //        perror("server:socket"); exit(1);
    //  } 

    //  /* build server Internet socket address */
    //  bzero((char *)&ser_addr, sizeof(ser_addr));
    //  ser_addr.sin_family = AF_INET;
    //  ser_addr.sin_port = htons(PORT);
    //  ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); 

    //  /* bind server address to socket sd */
    //  if (bind(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0){
    //        perror("server bind"); exit(1);
    //  }

    //  /* become a listening socket */
    //  listen(sd, 5);

//      while (1) {

//           /* wait to accept a client request for connection */
//           cli_addrlen = sizeof(cli_addr);
//           nsd = accept(sd, (struct sockaddr *) &cli_addr, &cli_addrlen);
//           if (nsd < 0) {
//                perror("server:accept"); exit(1);
//           }

//           /* create a child process to handle this client */
//           if ((pid=fork()) <0) {
//               perror("fork"); exit(1);
//           } else if (pid > 0) { 
//               close(nsd);
//               continue; /* parent to wait for next client */
//           }

//           /* now in child, serve the current client */
//           close(sd); serveClient(nsd);
//      }
// */

    int socketfd, connectionfd, cli_addrlen;
    pid_t pid;
    int runFlag = 1;
    struct  sockaddr_in clientaddr;
    char *command_buffer;
    char buffer[100];
    int addrlen = sizeof(struct sockaddr_in);

    // initilise the server
    socketfd = initServer();








    //accept the data pkt from client
    // printf("Socketfd: %i\n", socketfd);
    // if ((connectionfd = accept(socketfd, (struct sockaddr *)&clientaddr, (socklen_t *)&addrlen)) < 0) {
    //     printf("Error: Accept Failed\n");
    //     exit(0);
    // }
    // else
    //     printf("Success: acccepted the client...\n");

    while (1) {

          /* wait to accept a client request for connection */
           cli_addrlen = sizeof(clientaddr);
           connectionfd = accept(socketfd, (struct sockaddr *) &clientaddr, (socklen_t *)&cli_addrlen);

          /* create a child process to handle this client */
          if ((pid=fork()) <0) {
              perror("fork"); exit(1);
          } else if (pid > 0) { 
              close(connectionfd);
              continue; /* parent to wait for next client */
          }

          /* now in child, serve the current client */
          close(socketfd); 
          serveClient(connectionfd);
     }

    // while(runFlag == 1)
    // {
	//     printf("reading value...\n");
    //     command_buffer = malloc(100 * sizeof(char));
    //     if( (read(connectionfd, buffer, 100)) < 0)
    //     {
    //         perror("Read error!!!\n");
    //     }
    //     printf("buffer: %s \n", buffer);
    //     strcpy(command_buffer, buffer);
    //     printf("command buffer: %s \n", command_buffer);

	
    //     // if(strcmp(command_buffer, "pwd") == 0)
    //     // {
    //     //     runFlag = run_cmd(connectionfd);
    //     // }
    //     // else if(strcmp(command_buffer, "dir") == 0)
    //     // {
    //     //     runFlag = run_cmd(connectionfd);
    //     // }
    //     // else if(strcmp(command_buffer, "Quit") == 0)
    //     // {
    //     //     runFlag = 0;
    //     // }
    //     // free(command_buffer);
    // }

    // for (;;) { }

    //close the socket
    // close(socketfd);
    return 0;
}
