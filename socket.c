#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<dirent.h>
#include<sys/stat.h>

/* Socket API headers */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>

/* Definations */
#define DEFAULT_BUFLEN 512
void write_to_file(const char* file_path, const char* content,int client_fd) {
    FILE* file = fopen(file_path, "a");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    fputs(content, file);
    fclose(file);
     
    char message[] = "200 OK: File written successfully\n";
    send(client_fd, message, strlen(message), 0);
}
void list_files(int client_fd, const char* path) {
    DIR* dir;
    struct dirent* entry;
    struct stat file_stat;
    char buffer[DEFAULT_BUFLEN];

    dir = opendir(path);
    if (dir == NULL) {
        perror("Failed to open directory");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        char file_path[DEFAULT_BUFLEN];
        snprintf(file_path,DEFAULT_BUFLEN,"%s/%s", path, entry->d_name);

        if (stat(file_path, &file_stat) < 0)
            continue;

        if (S_ISDIR(file_stat.st_mode))
            continue;

        snprintf(buffer, DEFAULT_BUFLEN, "File: %s, Size: %ld bytes\n", entry->d_name, file_stat.st_size);
        send(client_fd, buffer, strlen(buffer), 0);
    }

    closedir(dir);
}

void do_job(int fd,const char* path) {
int length,rcnt;
char recvbuf[DEFAULT_BUFLEN];

    char a[]="Welcome to Lailat's server\n";
    send(fd,a,strlen(a),0);

    // Receive until the peer shuts down the connection
    do {
        rcnt = recv(fd, recvbuf, sizeof(recvbuf), 0);
        if (rcnt > 0) {
            printf("Bytes received: %d\n", rcnt);
            if (strncmp(recvbuf,"LIST",4) == 0) {
                list_files(fd, path);
            }
            else if (strncmp(recvbuf, "GET", 3) == 0) {
   
    char filename[DEFAULT_BUFLEN];
    sscanf(recvbuf, "GET %s", filename);
                 char file_path[DEFAULT_BUFLEN];
                snprintf(file_path, DEFAULT_BUFLEN, "%s/%s", path, filename);
    FILE* file = fopen(file_path, "rb");
    if (file == NULL) {
        char error_message[DEFAULT_BUFLEN];
        snprintf(error_message, DEFAULT_BUFLEN, "File not found: %s\n", filename);
        send(fd, error_message, strlen(error_message), 0);
    } else {
       
        char buffer[DEFAULT_BUFLEN];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, DEFAULT_BUFLEN, file)) > 0) {
            send(fd, buffer, bytes_read, 0);
        }

        fclose(file);
    }
}
            else if (strncmp(recvbuf, "PUT", 3) == 0) {
                char filename[DEFAULT_BUFLEN];
                sscanf(recvbuf, "PUT %s", filename);
                // Read lines from the client until a "." is encountered
                char content[DEFAULT_BUFLEN];
                memset(content, 0, sizeof(content));
                while (1) {
                    rcnt = recv(fd, recvbuf, sizeof(recvbuf), 0);
                    if (rcnt > 0) {
                        recvbuf[rcnt] = '\0';
                        if (strcmp(recvbuf, ".\n") == 0) {
                            break;
                        }
                        strcat(content, recvbuf);
                    } else {
                        break;
                    }
                }
                char file_path[DEFAULT_BUFLEN];
                snprintf(file_path, DEFAULT_BUFLEN, "%s/%s", path, filename);
                write_to_file(file_path, content,fd);
            }
            else if(strncmp(recvbuf,"DEL",3)==0){
            char suna[DEFAULT_BUFLEN];
             sscanf(recvbuf,"DEL %s",suna);
             char hanya[DEFAULT_BUFLEN];
                snprintf(hanya,DEFAULT_BUFLEN,"%s/%s",path,suna);
                if(remove(hanya)==0){
                    char message[DEFAULT_BUFLEN];
        snprintf(message, DEFAULT_BUFLEN, "%s deleted\n", suna);
        send(fd, message, strlen(message), 0);
                }
                 else{
                     char message1[DEFAULT_BUFLEN];
        snprintf(message1, DEFAULT_BUFLEN, "%s could not be deleted\n", suna);
        send(fd, message1, strlen(message1), 0);
                 }
            }

            else if(strncmp(recvbuf,"QUIT",4)==0)
            {
                char message[]="You closed the server\n";
                send(fd,message,strlen(message),0);
                char reply[]="....Goodbye....\n";
                  send(fd,reply,strlen(reply),0);
                close(fd);
            }else {
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
    char *users=NULL;
    while((character=getopt(argc,argv,"d:p:u:"))!=-1){
      switch(character){
          case 'd':
              path=optarg;
              break;
          case 'p':
              PORT=atoi(optarg);
              break;
          case 'u':
              users=optarg;
              break;
          default:
              fprintf(stderr,"Please Enter port number using -p",argv[0]);
              return 1;
      }
        
        
       }
          if(path==NULL||users==NULL||PORT==0){
             printf("Please Enter arguements using -d,-p and -u\n");
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
        do_job(fd,path);
        printf("Disconnected!\n");
        close(fd);
        exit(0);
    }

}

// Final Cleanup
close(server);

}
