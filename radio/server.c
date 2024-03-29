#include <stdio.h> /* These are the usual header files */
#include <string.h>
#include <unistd.h> /* for close() */
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 1234 /* Port that will be opened */
#define MAXDATASIZE 100 /* Max number of bytes of data */

main()
{
    int sockfd; /* socket descriptors */
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    socklen_t sin_size;
    int num;
    char recvmsg[MAXDATASIZE]; /* buffer for message */
    
    char condition[] = "quit";
    /* Creating UDP socket */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        /* handle exception */
        perror("Creating socket failed.");
        exit(1);
    }
    
    bzero(&server,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(PORT);
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        /* handle exception */
        perror("Bind error.");
        exit(1);
    }
    
    sin_size=sizeof(struct sockaddr_in);
    while (1) {
        num = recvfrom(sockfd,recvmsg,MAXDATASIZE,0,(struct sockaddr *)&client,&sin_size);
        if (num < 0){
            perror("recvfrom errorn");
            exit(1);
        }
        
        recvmsg[num-1] = '\0';
        recvmsg[num] = '\0';
        printf("Message (%s) from %s\n",recvmsg,inet_ntoa(client.sin_addr) ); /* prints client's IP */
        if(strcmp(recvmsg,condition)==0) break;
        
        char sendmsg [22]= {'R','e','s','p','o','n','s','e',' ','t','o',' ','c','l','i','e','n','t','!','\0'};
        sendto(sockfd,sendmsg,strlen(sendmsg),0,(struct sockaddr *)&client,sin_size);
        
    }
    
    close(sockfd); /* close listenfd */
}