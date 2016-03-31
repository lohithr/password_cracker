#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <unistd.h>
#include <crypt.h>

using namespace std;

void error(const char *msg)//prints error messages
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256]; //data to be sent and read from socket

    //describing how to run the executable
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    //creating a socket and connecting to server
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    //reading from the socket
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0){
     error("Unable to read from socket");
    }

    //breaking the data into required parts
    string buf=buffer;
    string start="",hash="";
    int i=0;

    //start contains the starting point of iteration
    for(i=4;buf[i]!=' ';i++)
    {
        start+=buf[i];
    }
    i++;

    //pos indicates the position to be iterated
    int pos;
    for(;buf[i]!=' ';i++)
    {
    pos=buf[i]-48;
    }
    i++;

    //hash contains the hash sent by the server
    for(;i<buf.length();i++)
    {
        hash+=buf[i];
    }

    bool success=0; // flag to know whether password is cracked or not

    //iterating on capital alphabets if corresponding flag is set
    if(buf[0]==49){
        string temp=start;
        temp[pos]='A';
        for(int j=0;j<25;j++){
            string ssalt="";
            ssalt+=hash[0];
            ssalt+=hash[1];
            const char *salt=ssalt.c_str();
            const char *passwd=temp.c_str();
            char *stemp=crypt(passwd,salt);
            string t1(stemp);
            if(t1==hash){
                 n = write(sockfd,passwd,sizeof(passwd));
                if (n < 0) error("ERROR writing to socket");
                break;
                success=1;
            }
            temp[pos]+=1;
        }
    }

    //iterating on small alphabets if corresponding flag is set
    if(buf[1]==49){
        string temp=start;
        temp[pos]='a';
        for(int j=0;j<25;j++){
            string ssalt="";
            ssalt+=hash[0];
            ssalt+=hash[1];
            const char *salt=ssalt.c_str();
            const char *passwd=temp.c_str();
            char *stemp=crypt(passwd,salt);
            string t1(stemp);
            if(t1==hash){
                 n = write(sockfd,passwd,sizeof(passwd));
                if (n < 0) error("ERROR writing to socket");
                break;
                success=1;
            }
            temp[pos]+=1;
        }
    }

    //iterating on numeric character if corresponding flag is set
    if(buf[2]==49){
        string temp=start;
        temp[pos]='0';
        for(int j=0;j<9;j++){
            string ssalt="";
            ssalt+=hash[0];
            ssalt+=hash[1];
            const char *salt=ssalt.c_str();
            const char *passwd=temp.c_str();
            char *stemp=crypt(passwd,salt);
            string t1(stemp);
            if(t1==hash){
                 n = write(sockfd,passwd,sizeof(passwd));
                if (n < 0) error("ERROR writing to socket");
                break;
                success=1;
            }
            temp[pos]+=1;
        }
    }

    //returning failure on being unable to crack the password
    if(!success){
        n = write(sockfd,"failed",6);
        if (n < 0) error("ERROR writing to socket");
    }


    close(sockfd);
    return 0;
}
