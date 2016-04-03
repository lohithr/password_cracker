#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <queue>

using namespace std;
struct userinfo
{
public:
    string hash;
    int plen;
    string pflags;
    string msgtoworker;
};

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr,"usage %s <port>", argv[0]);
        exit(0);
    }

    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    portno = atoi(argv[1]);
    fd_set master;
    fd_set read_fds;
    fd_set write_fds;
    fd_set worker;
    map<int,struct userinfo> usermap;
    map<int,int> workertouser;
    set<int> freeworkers;
    queue<int> userqueue;
    //set<int> redundant;
    int yes = 1;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd,10);
    clilen = sizeof(cli_addr);
    FD_SET(sockfd, &master);
    int fdmax= sockfd;

    for(;;)
    {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
        {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(int i = 0; i <= fdmax; i++)
        {
            if (FD_ISSET(i, &read_fds))   // we got one!!
            {
                if (i == sockfd)
                {
                    // handle new connections
                    clilen = sizeof cli_addr;
                    newsockfd = accept(sockfd,
                                       (struct sockaddr *)&cli_addr,
                                       &clilen);

                    if (newsockfd == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        FD_SET(newsockfd, &master); // add to master set
                        if (newsockfd > fdmax)      // keep track of the max
                        {
                            fdmax = newsockfd;
                        }
                        printf("new connection on socket %d\n",newsockfd);
                    }
                }
                else
                {
                    // handle data from a client
                    int nbytes;
                    bzero(buffer,sizeof(buffer));
                    if ((nbytes = recv(i, buffer, sizeof buffer, 0)) <= 0)
                    {
                        // got error or connection closed by client
                        if (nbytes == 0)
                        {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        }
                        else
                        {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    }
                    else
                    {
                        // we got some data from a client
                        /*for(int j = 0; j <= fdmax; j++) {
                            // send to everyone!
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != sockfd && j != i) {
                                    if (send(j, buffer, nbytes, 0) == -1) {
                                        perror("send");
                                    }
                                }
                            }*/
                        if(buffer[0]=='0')
                        {
                            struct userinfo* newstruct = new struct userinfo;
                            string usermsg(buffer);
                            cout<<usermsg<<endl;
                            string hash = "";
                            int plen = 0;
                            string pflags = "";
                            int j;
                            for(j=2; usermsg[j]!=' '; j++)
                            {
                                hash += usermsg[j];
                            }
                            j++;
                            plen = usermsg[j]-48;

                            j++;
                            j++;
                            for(; j<usermsg.length(); j++)
                            {
                                pflags+=usermsg[j];
                            }
                            newstruct->hash=hash;
                            newstruct->pflags=pflags;
                            newstruct->plen=plen;
                            string msg= "";
                            for(int k=0; k<plen; k++)
                            {
                                if(pflags[0]=='1')
                                {
                                    msg+="a";
                                }
                                else if (pflags[1]=='1')
                                {
                                    msg+="A";
                                }
                                else if (pflags[2]=='1')
                                {
                                    msg+="0";
                                }
                            }
                            msg+=" ";
                            msg+=hash;
                            msg+=" ";
                            msg+=pflags;
                            newstruct->msgtoworker=msg;
                            usermap[i]=*newstruct;
                            userqueue.push(i);
                            cout<<"User "<<i<<" registered"<<endl;
                            while(!freeworkers.empty())
                            {
                                set<int>::iterator it=freeworkers.begin();
                                const char* msgsend=usermap[userqueue.front()].msgtoworker.c_str();
                                    if(send(*it,msgsend,usermap[userqueue.front()].msgtoworker.length(),0)<0)
                                    {

                                        error("send failed");
                                    }
                                    else {
                                    freeworkers.erase(*it);
                                    cout<<"Message sent to worker"<<usermap[userqueue.front()].msgtoworker<<endl;
                                        workertouser[*it]=userqueue.front();
                                        if(usermap[userqueue.front()].pflags=="001")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]!='9')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }

                                        }
                                        else if(usermap[userqueue.front()].pflags=="010")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]!='Z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                        else if(usermap[userqueue.front()].pflags=="100")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]!='z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                        else if(usermap[userqueue.front()].pflags=="011")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]=='Z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='1';
                                            }
                                            else if(usermap[userqueue.front()].msgtoworker[0]!='9')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                        else if(usermap[userqueue.front()].pflags=="101")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]=='z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='1';
                                            }
                                            else if(usermap[userqueue.front()].msgtoworker[0]!='9')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                        else if(usermap[userqueue.front()].pflags=="110")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]=='z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='A';
                                            }
                                            else if(usermap[userqueue.front()].msgtoworker[0]!='Z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                        else if(usermap[userqueue.front()].pflags=="111")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]=='z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='A';
                                            }
                                            else if(usermap[userqueue.front()].msgtoworker[0]=='Z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='1';
                                            }
                                            else if(usermap[userqueue.front()].msgtoworker[0]!='9')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                    }

                            }

                        }
                        else if(buffer[0]=='1')
                        {

                            if(strlen(buffer)==1)
                            {
                                freeworkers.insert(i);
                                cout<<"Worker "<<i<<" registered"<<endl;
                            }
                            //set<int> iterator it;
                            //it = redundant.find(i);
                            /*if(it!=redundant.end())
                            {

                            }*/
                            else if(buffer[2]=='1')
                            {
                                string buf(buffer);
                                string pwd=buf.substr(4);
                                cout<<"BUFFER IS "<<pwd<<endl;
                                const char* pwdf = pwd.c_str();
                                send(userqueue.front(),pwdf,pwd.length(),0);
                                usermap.erase(userqueue.front());
                                /*for(map<int,int>::iterator it=workertouser.begin(); it!=workertouser.end(); ++it)
                                {
                                    if(it->second==userqueue.front()&&it->first!=i)
                                    {
                                        redundant.add(it->first);
                                    }
                                }*/
                                userqueue.pop();

                                workertouser.erase(i);
                                freeworkers.insert(i);
                                cout<<"Password cracked!"<<endl;
                                }

                                else if(buffer[2]=='0')
                                {
                                freeworkers.insert(i);
                                }
                                if(!userqueue.empty())

                                {
                                    if(usermap[userqueue.front()].msgtoworker[0]=='$')
                                    {
                                        send(userqueue.front(),"Sorry.Password cannot be cracked",32,0);
                                        usermap.erase(userqueue.front());
                           /* for(map<int,int>::iterator it=workertouser.begin(); it!=workertouser.end(); ++it)
                                {
                                    if(it->second==userqueue.front()&&it->first!=i)
                                    {
                                        redundant.add(it->first);
                                    }
                                }*/

                                        userqueue.pop();

                                    }
                                    const char* msgsend=usermap[userqueue.front()].msgtoworker.c_str();
                                    if(send(i,msgsend,usermap[userqueue.front()].msgtoworker.length(),0)<0)
                                    {

                                        error("send failed");
                                    }
                                    else {
                                    freeworkers.erase(i);
                                    cout<<"Message sent to worker"<<usermap[userqueue.front()].msgtoworker<<endl;
                                        workertouser[i]=userqueue.front();
                                        if(usermap[userqueue.front()].pflags=="001")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]!='9')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }

                                        }
                                        else if(usermap[userqueue.front()].pflags=="010")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]!='Z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                        else if(usermap[userqueue.front()].pflags=="100")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]!='z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                        else if(usermap[userqueue.front()].pflags=="011")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]=='Z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='1';
                                            }
                                            else if(usermap[userqueue.front()].msgtoworker[0]!='9')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                        else if(usermap[userqueue.front()].pflags=="101")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]=='z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='1';
                                            }
                                            else if(usermap[userqueue.front()].msgtoworker[0]!='9')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                        else if(usermap[userqueue.front()].pflags=="110")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]=='z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='A';
                                            }
                                            else if(usermap[userqueue.front()].msgtoworker[0]!='Z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }
                                        else if(usermap[userqueue.front()].pflags=="111")
                                        {
                                            if(usermap[userqueue.front()].msgtoworker[0]=='z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='A';
                                            }
                                            else if(usermap[userqueue.front()].msgtoworker[0]=='Z')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='1';
                                            }
                                            else if(usermap[userqueue.front()].msgtoworker[0]!='9')
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]++;
                                            }
                                            else
                                            {
                                                usermap[userqueue.front()].msgtoworker[0]='$';
                                            }
                                        }

                                    }

                            }
                            }
                        }

                    } // END handle data from client
                } // END got new incoming connection
            } // END looping through file descriptors // END for(;;)--and you thought it would never end!
        }
    }

