#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <ctime>
#include <time.h>
#include <arpa/inet.h>
using namespace std;

void error(const char *msg)//function prints the error and exits the program
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    //initializing socket descriptor, port etc..
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //usage of executable from command line
    if (argc != 6) {
       fprintf(stderr,"usage %s hostname server-port hash passwd-length binary-string\n", argv[0]);
       exit(0);
    }

    //creating the socket and connecting
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    /*server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }*/
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    /*bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);*/
    serv_addr.sin_port = htons(portno);
    inet_aton(argv[1],&(serv_addr.sin_addr));
    memset(&(serv_addr.sin_zero),'\0',8);
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    //forming the message(data packet) to be sent as a string
    string hash(argv[3]);
    string pswdlwn(argv[4]);
    string flags(argv[5]);
    string buf="0 ";
    buf=buf+hash+" "+pswdlwn+" "+flags;
    const char *buffer=buf.c_str();      //buffer is the final data string 

    clock_t t1,t2;
    t1=clock();   //noting the time before sending the packet

    //writing data to the socket
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    char readbuff[256];
    bzero(readbuff,256);
    //reading data from socket
    n = read(sockfd,readbuff,255);
    if (n < 0) 
         error("ERROR reading from socket");
    printf("%s\n",readbuff);
    cout<<readbuff;

    t2=clock();                     //ending the clock 
    float diff((float)t2-(float)t1);//calculating the time taken for cracking the password
    cout<<"Time taken: "<<diff/CLOCKS_PER_SEC<<endl;
    close(sockfd);
    return 0;
}
