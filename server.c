#define _POSIX_C_SOURCE 200112L
#define IMPLEMENTS_IPV6
#define MULTITHREADED
#define BUFSIZE 500
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include<pthread.h>




static volatile int keepRunning = 1;
char* path;

void intHandler(int dummy) {
    printf("\nctrl^c detected server close\n");
    keepRunning = 0;
}

void* handle_connection(void* p_client_socket);


void func(int connfd, char* path);
void searchInDir(char* dirname);
int searchFile(char* fileName,char* path);





int main(int argc, char** argv) {
    
    int sockfd, newsockfd, re, s;


    path=argv[3];
    struct addrinfo hints, *res;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;

    if (argc < 3) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }

     // check the protocol number is correct or not
    if (atoi(argv[1])!=4 && atoi(argv[1])!=6){
        perror("wrong protocol number");
        exit(1);
    }

    // Create address we're going to listen on (with given port number)
    if (atoi(argv[1])==4){
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;       // IPv4
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
        // node (NULL means any interface), service (port), hints, res
    }
    else if (atoi(argv[1])==6){
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET6;       // IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP
        hints.ai_flags = AI_PASSIVE;     // for bind, listen, accept
        // node (NULL means any interface), service (port), hints, res
    }
   
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

   
    
    // check valid port numebr 
    in_port_t portNumber=atoi(argv[2]);

    // check path
    searchInDir(argv[3]);

    if (portNumber==0){
        perror("wrong port number");
        exit(1);
    }

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt");
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
    
   

    //persistent connection
    //func(newsockfd,argv[3]);

    signal(SIGINT, intHandler);
    
    while (keepRunning){
        client_addr_size = sizeof client_addr;
        newsockfd =
            accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
        if (newsockfd < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        
        pthread_t t;
        int *pclient=malloc(sizeof(int));
        *pclient=newsockfd;
        pthread_create(&t, NULL,handle_connection,pclient);
 
    }

    
    close(sockfd);
   
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
void func(int newsockfd, char* path){

    char buffer[256];
    int n;
    int gotSend=1;
   

    

    while(1){
       
        // Read characters from the connection, then process
        n = read(newsockfd, buffer, 255); // n is number of characters read

        // exit the loop if n is 0, which happens when client disconnects the server
        

        if (n < 0) {
            perror("read");
            exit(EXIT_FAILURE);
            break;
        }
        // Null-terminate string
        buffer[n] = '\0';

      

        printf("buffer is:\n%s",buffer);
        
        if (gotSend==1 && n>0){

            if (strncmp(buffer,"GET",3)==0){
                
            }
           
            
            snprintf(buffer,sizeof(buffer),"HTTP/1.1 200 OK\r\n"
            "Content-Type: index/html\n");
            printf("send response \n%s\n",buffer);
            n=write(newsockfd,buffer,strlen(buffer));

            if (n < 0) {
                perror("write");
                exit(EXIT_FAILURE);
            }
            
            gotSend=0;
           
        }
       

    }
}

//search file
int searchFile(char* fileName,char* path){

    printf("fileName is %s, path is %s\n",fileName,path);

    char temp[500];
    strcpy(temp,path);
    strcat(temp,fileName);
    printf("whole path is %s\n",temp);
    if (strstr(temp,"../")){
        return 1;
    }
    
    if (access(temp,F_OK)!=-1){
        printf("found in searchfile\n");
        return 0;
    }
    return 1;

    
}

void* handle_connection(void* p_client_socket){
    char delim[]=" ";
    char buffer[BUFSIZE];
    char temp[BUFSIZE];
    int newsockfd=*((int*)p_client_socket);
    free(p_client_socket);
    int n;
    
    while(strstr(buffer,"HTTP/1.0")==0){
        n = read(newsockfd, buffer, sizeof(buffer));
        strcat(temp,buffer);
        printf("temp now is %s\n",temp);
        printf("buffer now is %s\n",buffer);
    }
   
    
   
    buffer[n] = '\0';
    
    if (strlen(temp)>strlen(buffer)){
        strcpy(buffer,temp);
    }
    
    if (n>0){
        printf("root is %s\n",path);
        char* ptr = strtok(buffer,delim);


        if (strncmp(buffer,"GET",3)==0){
                ptr=strtok(NULL,delim);

                //gets file name
                char* ptrChopped=ptr;
                printf("%s\n", ptrChopped);

                //if can be found in directory
                if (searchFile(ptrChopped,path)==0){
                    char* reply="HTTP/1.0 200 OK\r\n";
                    printf("successfly found\n");
                    send(newsockfd,reply,strlen(reply),0);
                    
                    char temp[500];
                    strcpy(temp,path);
                    strcat(temp,ptrChopped);

                    char buf[100];

                    //if its a html file
                    if (strstr(ptrChopped,".html")){
                        
                        send(newsockfd,"Content-Type: text/html\r\n",strlen("Content-Type: text/html\r\n"),0);
                        send(newsockfd,"\r\n",strlen("\r\n"),0);

                        
                        FILE* file=fopen(temp,"r");
                        printf("open successfully\n");
                        
                        while(fgets(buf,sizeof(buf),file)){
                            printf("%s",buf);
                            send(newsockfd,buf,strlen(buf),0);
                        }
                        fclose(file);
                       
                    }

                    //if its a jpg file 
                    else if (strstr(ptrChopped,".jpg")){

                        send(newsockfd,"Content-Type: image/jpeg\r\n",strlen("Content-Type: image/jpeg\r\n"),0);
                        send(newsockfd,"\r\n",strlen("\r\n"),0);
                        

                        printf("image file\n");
                       
                        size_t bytes_read=1;
                        //open the file in binary mode
                        FILE* jpg = fopen(temp,"rb");
                        printf("open successfully\n");
                        
                        while(bytes_read != 0){
                            bytes_read=fread(buf,sizeof(char),100,jpg);
                            send(newsockfd,buf,bytes_read,0);
                        }
                       
                       
                        fclose(jpg);
                       

                    }

                    //if its a css file
                    else if (strstr(ptrChopped,".css")){
                        send(newsockfd,"Content-Type: CSS\r\n",strlen("Content-Type: CSS\r\n"),0);
                        send(newsockfd,"\r\n",strlen("\r\n"),0);
                        printf("css file\n");

                        FILE* file=fopen(temp,"r");
                        printf("open successfully\n");
                        
                        while(fgets(buf,sizeof(buf),file)){
                            printf("%s",buf);
                            send(newsockfd,buf,strlen(buf),0);
                        }
                        fclose(file);
                       
                    }

                    //if its a js file
                    else if(strstr(ptrChopped,".js")){
                        send(newsockfd,"Content-Type: JavaScript\r\n",strlen("Content-Type: JavaScript\r\n"),0);
                        send(newsockfd,"\r\n",strlen("\r\n"),0);
                        printf("js file\n");

                        FILE* file=fopen(temp,"r");
                        printf("open successfully\n");
                        
                        while(fgets(buf,sizeof(buf),file)){
                            printf("%s",buf);
                            send(newsockfd,buf,strlen(buf),0);
                        }
                        fclose(file);
                       
                    }

                    //if its else 
                    else{

                        send(newsockfd,"Content-Type: application/octet-stream\r\n",strlen("Content-Type: application/octet-stream\r\n"),0);
                        //send(newsockfd,"\r\n",strlen("\r\n"),0);
                        printf("unknown file\n");
                        
                    }

                   
                  
                    printf("connection close\n");
                    close(newsockfd);
                }

                

                else{
                    char* reply1="HTTP/1.0 404 Not Found\r\n";
                    printf("unsuccessfly found\n");
                    send(newsockfd,reply1,strlen(reply1),0);
                    //send(newsockfd,"Content-Type: application/octet-stream\r\n",strlen("Content-Type: application/octet-stream\r\n"),0);
                  
                    close(newsockfd);
                }
                
            }
    }


    return NULL;
}



