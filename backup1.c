#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <curl/curl.h>

void func(int connfd);
void searchInDir(char* dirname);
int searchFile(char* fileName,char* path);


int main(int argc, char** argv) {
    
    int sockfd, newsockfd, re, s;

    int n;
    char buffer[256];
    char* GET = "GET";


    struct addrinfo hints, *res;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }

    // Create address we're going to listen on (with given port number)
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
    // node (NULL means any interface), service (port), hints, res
    s = getaddrinfo(NULL, argv[2], &hints, &res);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // Create socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        perror("Failed to connect socket");
        exit(EXIT_FAILURE);
    }
    else{
        printf("successfuly connected socket\n");
    }

    // Reuse port if possible
    re = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // check the protocol number is correct or not
    if (atoi(argv[1])!=4 && atoi(argv[1])!=6){
        perror("wrong protocol number");
        exit(1);
    }
    
    // check valid port numebr 
    in_port_t portNumber=atoi(argv[2]);

    // check path
    searchInDir(argv[3]);

    if (portNumber==0){
        perror("wrong port number");
        exit(1);
    }
    
     // Bind address to the socket
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        
        perror("Failed to bind socket");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);

    // Listen on socket - means we're ready to accept connections,
    // incoming connection requests will be queued, man 3 listen
    if (listen(sockfd, 5) < 0) {
        perror("Failed to start listening");
        exit(EXIT_FAILURE);
    }

    // Accept a connection - blocks until a connection is ready to be accepted
    // Get back a new file descriptor to communicate on
    client_addr_size = sizeof client_addr;
    newsockfd =
            accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
    if (newsockfd < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }



    n = read(newsockfd, buffer, 255); // n is number of characters read
	if (n < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	// Null-terminate string
	buffer[n] = '\0';

    
	// Write message back
	printf("Here is the message: %s\n", buffer);
    
    char delim[]=" ";
    char* ptr = strtok(buffer,delim);
    

    // if its GET request
    if (strcmp(GET,ptr)==0){
        ptr=strtok(NULL,delim);

        // chop off the '/'
        char* ptrChopped=ptr+1;
        printf("%s\n", ptrChopped);
        if (searchFile(ptrChopped,argv[3])==0){

            char *reply = 
            "HTTP/1.1 200 OK\n"
            "Date: Thu, 19 Feb 2009 12:27:04 GMT\n"
            "Server: Apache/2.2.3\n"
            "Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\n"
            "ETag: \"56d-9989200-1132c580\"\n"
            "Content-Type: text/html\n"
            "Content-Length: 15\n"
            "Accept-Ranges: bytes\n"
            "Connection: close\n"
            "\n"
            "I love Serena";

            printf("successfly found\n");
            send(newsockfd,reply,strlen(reply),0);
            
           
        }
        else{

            char *reply1 = 
            "HTTP/1.1 404 Not Found\n"
            "Date: Thu, 19 Feb 2009 12:27:04 GMT\n"
            "Server: Apache/2.2.3\n"
            "Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\n"
            "ETag: \"56d-9989200-1132c580\"\n"
            "Content-Type: text/html\n"
            "Content-Length: 15\n"
            "Accept-Ranges: bytes\n"
            "Connection: close\n"
            "\n"
            "404 not found";
            printf("unsuccessfly found\n");
            send(newsockfd,reply1,strlen(reply1),0);

        }
    }
   

    

	n = write(newsockfd, "I got your message", 18);
	if (n < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}

    //persistent connection
    //func(newsockfd);



    close(sockfd);
    close(newsockfd);
    return 0;
}

//search if dir exist in the root 
void searchInDir(char* dirname){
    DIR *d;
    struct  dirent *dir;
    d = opendir(dirname);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            //printf("%s\n", dir->d_name);
        }
        
    }
    else{
        printf("path not found\n");
        exit(errno);
    }

    closedir(d);
}


//persistent connection unless exit is sent from client
void func(int newsockfd){
    char buffer[256];
    int n;
    char *reply = 
    "HTTP/1.1 200 OK\n"
    "Date: Thu, 19 Feb 2009 12:27:04 GMT\n"
    "Server: Apache/2.2.3\n"
    "Last-Modified: Wed, 18 Jun 2003 16:05:58 GMT\n"
    "ETag: \"56d-9989200-1132c580\"\n"
    "Content-Type: text/html\n"
    "Content-Length: 15\n"
    "Accept-Ranges: bytes\n"
    "Connection: close\n"
    "\n"
    "I Love Serena";

    while (1){
       
        // Read characters from the connection, then process
        n = read(newsockfd, buffer, 255); // n is number of characters read

        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        // Null-terminate string
        buffer[n] = '\0';

        send(newsockfd,reply,strlen(reply),0);
        n = write(newsockfd, "I got your message\n", 18);

        
        if (n < 0) {
            perror("write");
            exit(EXIT_FAILURE);
        }

        if (strncmp("exit", buffer, 4)==0){
            printf("Server Exit...\n");
            break;
        }

    }
}

//search file
int searchFile(char* fileName,char* path){

    //printf("fileName is %s, path is %s\n",fileName,path);

    DIR *d;
    struct  dirent *dir1;
    d = opendir(path);

    //searchInDir(path);

    if (d){
        while ((dir1 = readdir(d)) != NULL)
        {      
            
            if (strcmp(dir1->d_name,fileName)==0){
                //printf("filename is %s, current is %s\n",fileName,dir1->d_name);
                //printf("successfuly found\n");
                return 0;
            }
            
        }
         closedir(d);
    }
    
    return 1;

    
}

