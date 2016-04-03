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
#include <arpa/inet.h>

using namespace std;

void error(const char *msg)//prints error messages
{
    perror(msg);
    exit(0);
}

bool crack(string hash,string a)//checks the hash with a password
{
    string ssalt="";
    ssalt+=hash[0];
    ssalt+=hash[1];
    const char *salt=ssalt.c_str();
    const char *passwd=a.c_str();
    char *stemp=crypt(passwd,salt);
    string t1(stemp);
    if(t1==hash){
        return true;
    }
    else {return false;}

}

//backtracks the bruteforce search
string backtrack(string a,int pos,string flags){
    if(pos<0){
        for (int i = 0; i < a.length(); ++i)
        {
            if(flags[0]=='1')a[i]='a';
            else if(flags[1]=='1')a[i]='A';
            else if(flags[2]=='1')a[i]='0';
        }
        return a;
    }
    if(97<=a[pos] && a[pos]<=122)
    {
        if(a[pos]==122){
            if(flags[1]=='1'){
                a[pos]='A';
                return a;
            }
            else if(flags[2]=='1'){
                a[pos]='0';
                return a;
            }
            else
            {
                if(flags[0]=='1')a[pos]='a';
                else if(flags[1]=='1')a[pos]='A';
                else if(flags[2]=='1')a[pos]='0';
                return backtrack(a,pos-1,flags);
            }

        }
       else{
        a[pos]+=1;
        return a;
        }
    }
    else if(65<=a[pos] && a[pos]<=90)
    {
        if(a[pos]==90){
            if(flags[2]=='1'){
                a[pos]='0';
                return a;
            }
            else{
                if(flags[0]=='1')a[pos]='a';
                else if(flags[1]=='1')a[pos]='A';
                else if(flags[2]=='1')a[pos]='0';
                return backtrack(a,pos-1,flags);
            }
        }
        else{
            a[pos]+=1;
            return a;
        }
    }
    else if(48<=a[pos] && a[pos]<=57)
    {
        if(a[pos]==57){
                if(flags[0]=='1')a[pos]='a';
                else if(flags[1]=='1')a[pos]='A';
                else if(flags[2]=='1')a[pos]='0';
                return backtrack(a,pos-1,flags);
        }
        else{
            a[pos]+=1;
            return a;
        }
    }
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

    //contacting server for first time
    n = write(sockfd,"1",1);
    if (n < 0){
     error("Unable to write to socket");
    }

    //forever loop 
    for(;;){

    //reading from the socket
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0){
     error("Unable to read from socket");
    }
	
    //breaking the data into required parts
    string buf=buffer;
    cout<<buf<<endl;
    string start="",hash="",flags="";
    int i=0;

    cout<<start<<" "<<hash<<" "<<flags<<" "<<endl;

    //start contains the starting point of iteration
    for(;buf[i]!=' ';i++)
    {
        start+=buf[i];
    }
    i++;

    //hash contains the hash sent by the server
    for(;buf[i]!=' ';i++)
    {
        hash+=buf[i];
    }
    i++;

    for (; i < buf.length(); ++i)
    {
        flags+=buf[i];
    }
    bool success=0; // flag to know whether password is cracked or not

    string a="";string passwd="";
    for (int i = 0; i < start.length()-1; ++i)
    {
        a+=start[i+1];
    }

    string fin0a="",fin1a="",fin2a="";
    for (int i = 0; i < a.length(); ++i)
    {
        fin0a+="a";fin1a+="A";fin2a+="0";
    }
    passwd+=start[0];

    //loop for iterating over all possibilities
    do{
        if(97<=a[a.length()-1] && a[a.length()-1]<=122)
    {
        if(a[a.length()-1]==122){
            if(flags[1]=='1'){
                a[a.length()-1]='A';
                if(crack(hash,start[0]+a)){
                    for (int i = 0; i < a.length(); ++i)
                    {
                        passwd+=a[i];
                    }
                    success=1;break;
                }
                continue;
            }
            else if(flags[2]=='1'){
                a[a.length()-1]='0';
                if(crack(hash,start[0]+a)){
                    for (int i = 0; i < a.length(); ++i)
                    {
                        passwd+=a[i];
                    }
                    success=1;break;
                }
                continue;
            }
            else
            {
                if(flags[0]=='1')a[a.length()-1]='a';
                else if(flags[1]=='1')a[a.length()-1]='A';
                else if(flags[2]=='1')a[a.length()-1]='0';
                a=backtrack(a,a.length()-2,flags);
                if(crack(hash,start[0]+a)){
                    for (int i = 0; i < a.length(); ++i)
                    {
                        passwd+=a[i];
                    }
                    success=1;break;
                }
                continue;
            }

        }
       else{
        a[a.length()-1]+=1;
        if(crack(hash,start[0]+a)){
            for (int i = 0; i < a.length(); ++i)
            {
                passwd+=a[i];
            }
            success=1;break;
        }
        continue;
        }
    }
    else if(65<=a[a.length()-1] && a[a.length()-1]<=90)
    {
        if(a[a.length()-1]==90){
            if(flags[2]=='1'){
                a[a.length()-1]='0';
                if(crack(hash,start[0]+a)){
                    for (int i = 0; i < a.length(); ++i)
                    {
                        passwd+=a[i];
                    }
                    success=1;break;
                }
                continue;
            }
            else{
                if(flags[0]=='1')a[a.length()-1]='a';
                else if(flags[1]=='1')a[a.length()-1]='A';
                else if(flags[2]=='1')a[a.length()-1]='0';
                a=backtrack(a,a.length()-2,flags);
                if(crack(hash,start[0]+a)){
                    for (int i = 0; i < a.length(); ++i)
                    {
                        passwd+=a[i];
                    }
                    success=1;break;
                }
                continue;
            }
        }
        else{
            a[a.length()-1]+=1;
            if(crack(hash,start[0]+a)){
                for (int i = 0; i < a.length(); ++i)
                {
                    passwd+=a[i];
                }
                success=1;break;
            }
            continue;
        }
    }
    else if(48<=a[a.length()-1] && a[a.length()-1]<=57)
    {
        if(a[a.length()-1]==57){
                if(flags[0]=='1')a[a.length()-1]='a';
                else if(flags[1]=='1')a[a.length()-1]='A';
                else if(flags[2]=='1')a[a.length()-1]='0';
                a=backtrack(a,a.length()-2,flags);
                if(crack(hash,start[0]+a)){
                    for (int i = 0; i < a.length(); ++i)
                    {
                        passwd+=a[i];
                    }
                    success=1;break;
                }
                continue;
        }
        else{
            a[a.length()-1]+=1;
            if(crack(hash,start[0]+a)){
                    for (int i = 0; i < a.length(); ++i)
                    {
                        passwd+=a[i];
                    }
                    success=1;break;
                }
            continue;
        }
    }
    }while((flags[0]=='1' && a!=fin0a) || (flags[1]=='1' && flags[0]=='0' && a!=fin1a) || (flags[2]=='1' && flags[1]=='0' && flags[0]=='0' && a!=fin2a));

    //writing failure or password on cracking password
    if(!success){
        n = write(sockfd,"1 0",3);
        if (n < 0) error("ERROR writing to socket");
    }
    else{
        passwd="1 1 "+passwd;
        cout<<passwd<<endl;
        const char *cracked=passwd.c_str();
        n = write(sockfd,cracked,strlen(cracked));
        if (n < 0) error("ERROR writing to socket");
    }

}
    close(sockfd);
    return 0;
}
