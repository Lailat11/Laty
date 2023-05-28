#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Socket API headers */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>

/* Definations */
#define DEFAULT_BUFLEN 512

void list_files(int client_fd, const char* directory) {
    DIR* dir;
    struct dirent* entry;
    struct stat file_stat;
    char buffer[DEFAULT_BUFLEN];

    dir = opendir(directory);
    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char file_path[DEFAULT_BUFLEN];
        sprintf(file_path, "%s/%s", directory, entry->d_name);

        if (stat(file_path, &file_stat) < 0)
            continue;

        if (S_ISDIR(file_stat.st_mode))
            continue;

        snprintf(buffer, DEFAULT_BUFLEN, "File: %s, Size: %ld bytes\n", entry->d_name, file_stat.st_size);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    closedir(dir);
}


void do_job(int fd) {
int length,rcnt;
char recvbuf[DEFAULT_BUFLEN],bmsg[DEFAULT_BUFLEN];
int  recvbuflen = DEFAULT_BUFLEN;
    char a[]="Welcome to Lailat's server";
    send(fd,a,strlen(a),0);

    // Receive until the peer shuts down the connection
    do {
        rcnt = recv(fd, recvbuf, recvbuflen, 0);
        if (rcnt > 0) {
            printf("Bytes received: %d\n", rcnt);
            if (strcmp(recvbuf, "LIST") == 0) {
                list_files(fd, path);
            } else {
                printf("Invalid command: %s\n", recvbuf);
            }

      

        }
        else if (rcnt == 0)
            printf("Connection closing...\n");
        else  {
            printf("Receive failed:\n");
            close(fd);
            break;
        }
    } while (rcnt > 0);
}



int main(int argc,char *argv[])
{
int server, client;
struct sockaddr_in local_addr;
struct sockaddr_in remote_addr;
int length,fd,rcnt,optval;
pid_t pid;
    int character;
    int PORT=0;
    char *path=NULL;
    while((character=getopt(argc,argv,"d:p:"))!=-1){
      switch(character){
          case 'd':
              path=optarg;
              break;
          case 'p':
              PORT=atoi(optarg);
              break;
          default:
              fprintf(stderr,"Please Enter port number using -p",argv[0]);
              return 1;
      }
        
        
       }
          if(path==NULL||PORT==0){
              printf("Please Enter arguements using -d and -p \n");
          return 1;
              
          }
          

/* Open socket descriptor */
if ((server = socket( AF_INET, SOCK_STREAM, 0)) < 0 ) { 
    perror("Can't create socket!");
    return(1);
}


/* Fill local and remote address structure with zero */
memset( &local_addr, 0, sizeof(local_addr) );
memset( &remote_addr, 0, sizeof(remote_addr) );

/* Set values to local_addr structure */
local_addr.sin_family = AF_INET;
local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
local_addr.sin_port = htons(PORT);

// set SO_REUSEADDR on a socket to true (1):
optval = 1;
setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

if ( bind( server, (struct sockaddr *)&local_addr, sizeof(local_addr) ) < 0 )
{
    /* could not start server */
    perror("Bind error");
    return(1);
}

if ( listen( server, SOMAXCONN ) < 0 ) {
        perror("listen");
        exit(1);
}

printf("File server now listening on port %d\n",PORT);


while(1) {  // main accept() loop
    length = sizeof remote_addr;
    if ((fd = accept(server, (struct sockaddr *)&remote_addr, \
          &length)) == -1) {
          perror("Accept Problem!");
          continue;
    }

    printf("Server: got connection from %s\n", \
            inet_ntoa(remote_addr.sin_addr));

    /* If fork create Child, take control over child and close on server side */
    if ((pid=fork()) == 0) {
        close(server);
        do_job(fd);
        printf("Child finished their job!\n");
        close(fd);
        exit(0);
    }

}

// Final Cleanup
close(server);

}
