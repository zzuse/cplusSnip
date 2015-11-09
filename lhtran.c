/*
************************************************************************************
*
* Lhtran.c - Packet Transmit Tool For Unix/Linux.
*
* Copyright (C) 2006 XSEC All Rights Reserved.
*
* Author   : lion  Date   : 2003-10-20
*
* Rewritten by W.Z.T  Date   :2006-3-22
*
* Complie : gcc -o lhtran lhtran.c -lpthread
*
* Usage ./lhtran
*       : ======================== Packet Transmit Tool V1.01 For Unix/Linux ====================
*       : =========== Code by lion & bkbll, Rewritten by W.Z.T, Welcome to http://www.xsec.org ==
*       :
*       : [Usage of Packet Transmit:]
*       :   HTran -<listen|tran|slave> <option> [-log logfile]
*       :
*       : [option:]
*       :   -listen <ConnectPort> <TransmitPort>
*       :   -tran   <ConnectPort> <TransmitHost> <TransmitPort>
*       :   -slave <ConnectHost> <ConnectPort> <TransmitHost> <TransmitPort>
*
**************************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
#define VERSION                        "1.01"
#define MAXSIZE                        20480
#define HOSTLEN                        40
#define CONNECTNUM                     5
struct transocket
{
    int fd1;
    int fd2;
};
void ver();
void usage(char *prog);
void *thread(void *m);
void closeallfd();
void makelog(char *buffer, int length);
void daemonize(void);
void proxy(int port);
void bind2bind(int port1, int port2);
void bind2conn(int port1, char *host, int port2);
void conn2conn(char *host1, int port1, char *host2, int port2);
int testifisvalue(char *str);
void setup(pthread_attr_t *attr);
int max(int i,int j);
int create_socket();
int create_server(int sockfd, int port);
int client_connect(int sockfd, char* server, int port);

extern int errno;
FILE *fp;
int method=0;
int flag=0;
//************************************************************************************
//
// function main
//
//************************************************************************************
int main(int argc, char* argv[])
{
    char **p;
    char sConnectHost[HOSTLEN], sTransmitHost[HOSTLEN];
    int iConnectPort=0, iTransmitPort=0;
    char *logfile=NULL;
//    ver();
    memset(sConnectHost, 0, HOSTLEN);
    memset(sTransmitHost, 0, HOSTLEN);
    p=argv;
    while(*p)
    {
          if(strstr(*p, "-log") !=NULL)
          {
                if(testifisvalue(*(p+1)))
                {
                      logfile = *(++p);
                }
                else
                {
                      printf("[-] ERROR: Must supply logfile name.\r\n");
                      return;
                }
                p++;
                continue;
          }

          p++;
    }
    if(logfile !=NULL)
    {
          fp=fopen(logfile,"a");
          if(fp == NULL )
          {
                printf("[-] ERROR: open logfile");
                return;
          }
          makelog("====== Start ======\r\n", 22);
    }

    if(argc > 2)
    {
   if(!strcmp(argv[1], "-listen") && argc >=4)
          {
                iConnectPort = atoi(argv[2]);
                iTransmitPort = atoi(argv[3]);
                method = 1;
          }
          else

          if(!strcmp(argv[1], "-tran") && argc >= 5)
          {
                iConnectPort = atoi(argv[2]);
                strncpy(sTransmitHost, argv[3], HOSTLEN);
                iTransmitPort = atoi(argv[4]);
                method = 2;
          }
          else
          if(!strcmp(argv[1], "-slave") && argc >= 6)
          {
                strncpy(sConnectHost, argv[2], HOSTLEN);
                iConnectPort = atoi(argv[3]);
                strncpy(sTransmitHost, argv[4], HOSTLEN);
                iTransmitPort = atoi(argv[5]);
                method = 3;
          }
    }
    switch(method)
    {
    case 1:
          bind2bind(iConnectPort, iTransmitPort);
          break;
    case 2:
          bind2conn(iConnectPort, sTransmitHost, iTransmitPort);
          break;
    case 3:
          conn2conn(sConnectHost, iConnectPort, sTransmitHost, iTransmitPort);
          break;
    default:
          usage(argv[0]);
          break;
    }

    if(method)
    {
          closeallfd();
    }
    return 0;
}

//************************************************************************************
//
// print version message
//
//************************************************************************************
void ver()
{
    printf("\n======================== Packet Transmit Tool V .%s ====================================\r\n", VERSION);
    printf("=========== Code by lion & bkbll, Rewritten by W.Z.T Welcome to http://www.xsec.org=======\r\n\n");
}
//************************************************************************************
//
// print usage message
//
//************************************************************************************
void usage(char* prog)
{
    printf("[Usage of Packet Transmit:]\r\n");
    printf(" %s -<listen|tran|slave> <option> [-log logfile]\n\n", prog);
    printf("[option:]\n");
    printf(" -listen <ConnectPort> <TransmitPort>\n");
    printf(" -tran   <ConnectPort> <TransmitHost> <TransmitPort>\n");
    printf(" -slave <ConnectHost> <ConnectPort> <TransmitHost> <TransmitPort>\n\n");
}
//************************************************************************************
//
// test if is value
//
//************************************************************************************
int testifisvalue(char *str)
{
    if(str == NULL ) return(0);
    if(str[0]=='-') return(0);
    return(1);
}
//************************************************************************************
//
// LocalHost:ConnectPort transmit to LocalHost:TransmitPort
//
//************************************************************************************
void bind2bind(int port1, int port2)
{
    int fd1,fd2, sockfd1, sockfd2;
    struct sockaddr_in client1,client2;
    pthread_t t;
    pthread_attr_t attr;
    int size1,size2;
    struct transocket sock;

    setup(&attr);
    daemonize();
    if((fd1=create_socket())==0) return;
    if((fd2=create_socket())==0) return;
//    printf("[+] Listening port %d ......\r\n",port1);
    fflush(stdout);
    if(create_server(fd1, port1)==0)
    {
          close(fd1);
          return;
    }
  //  printf("[+] Listen OK!\r\n");
  //  printf("[+] Listening port %d ......\r\n",port2);
    fflush(stdout);
    if(create_server(fd2, port2)==0)
    {
          close(fd2);
          return;
    }
 //     printf("[+] Listen OK!\r\n");
    size1=size2=sizeof(struct sockaddr);
    while(1)
    {
        //         printf("[+] Waiting for Client on port:%d ......\r\n",port1);
        if((sockfd1 = accept(fd1,(struct sockaddr *)&client1,&size1))<0)
        {
            printf("[-] Accept1 error.\r\n");
            continue;
        }

        //         printf("[+] Accept a Client on port %d from %s ......\r\n", port1, inet_ntoa(client1.sin_addr));
        //         printf("[+] Waiting another Client on port:%d....\r\n", port2);
        if((sockfd2 = accept(fd2, (struct sockaddr *)&client2, &size2))<0)
        {
            //               printf("[-] Accept2 error.\r\n");
            close(sockfd1);
            continue;
        }
        //         printf("[+] Accept a Client on port %d from %s\r\n",port2, inet_ntoa(client2.sin_addr));
        //         printf("[+] Accept Connect OK!\r\n");
        sock.fd1 = sockfd1;
        sock.fd2 = sockfd2;
        if(sock.fd1<0||sockfd2<0){
            //  printf("sock error,wait new client come\r\n\n");
            continue;
        }
        if(pthread_create(&t,&attr,thread,(void *)&sock)!=0){
            //  printf("CreateThread Failed!\r\n\n");
            continue;
        }
        //          printf("[+] CreateThread OK!\r\n\n");
    }
}
//************************************************************************************
//
// LocalHost:ConnectPort transmit to TransmitHost:TransmitPort
//
//************************************************************************************
void bind2conn(int port1, char *host, int port2)
{
    int sockfd,sockfd1,sockfd2;
    pthread_t t;
    pthread_attr_t attr;
    struct sockaddr_in remote;
    int size;
    char buffer[1024];
    struct transocket sock;
    if (port1 > 65535 || port1 < 1)
    {
          printf("[-] ConnectPort invalid.\r\n");
          return;
    }
    if (port2 > 65535 || port2 < 1)
    {
          printf("[-] TransmitPort invalid.\r\n");
          return;
    }

    daemonize();
    memset(buffer,0,1024);
    setup(&attr);
    if((sockfd=create_socket()) <0) return;
    if(create_server(sockfd, port1) == 0)
    {
          close(sockfd);
          return;
    }

    size=sizeof(struct sockaddr);
    while(1)
    {
  //        printf("[+] Waiting for Client ......\r\n");
          if((sockfd1=accept(sockfd,(struct sockaddr *)&remote,&size))<0)
          {
  //              printf("[-] Accept error.\r\n");
                continue;
          }
 //         printf("[+] Accept a Client from %s:%d ......\r\n",inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));
            if((sockfd2=create_socket())==0)
            {
                  close(sockfd1);
                  continue;
            }
 //           printf("[+] Make a Connection to %s:%d ......\r\n",host,port2);
            fflush(stdout);
          if(client_connect(sockfd2,host,port2)==0)
          {
                close(sockfd2);
 //               sprintf(buffer,"[SERVER]connection to %s:%d error\r\n", host, port2);
                send(sockfd1,buffer,strlen(buffer),0);
                memset(buffer, 0, 1024);
                close(sockfd1);
                continue;
          }

 //         printf("[+] Connect OK!\r\n");
          sock.fd1 = sockfd1;
          sock.fd2 = sockfd2;
   if(sock.fd1<0||sock.fd2<0){
 printf("sock error\r\n\n");
 continue;
          }
   if(pthread_create(&t,&attr,thread,(void *)&sock)!=0){
 //  printf("[-] CreateThread Failed\r\n\n");
 continue;
          }
 //         printf("[+] CreateThread OK!\r\n\n");
    }
}
//************************************************************************************
//
// ConnectHost:ConnectPort transmit to TransmitHost:TransmitPort
//
//************************************************************************************
void conn2conn(char *host1,int port1,char *host2,int port2)
{
    int sockfd1,sockfd2;
    pthread_t t;
    pthread_attr_t attr;
    struct transocket sock;
    fd_set fds;
    int l;
    char buffer[MAXSIZE];
    setup(&attr);
    daemonize();
    while(1)
    {
          if((sockfd1=create_socket())==0) return;
          if((sockfd2=create_socket())==0) return;
   //       printf("[+] Make a Connection to %s:%d....\r\n",host1,port1);
          fflush(stdout);
          if(client_connect(sockfd1,host1,port1)==0)
          {
                close(sockfd1);
                close(sockfd2);
                continue;
          }

          l=0;
          memset(buffer,0,MAXSIZE);
          while(1)
          {
                FD_ZERO(&fds);
                FD_SET(sockfd1, &fds);

                if (select(sockfd1+1, &fds, NULL, NULL, NULL) < 0)
                {
                      break;
                }
                if (FD_ISSET(sockfd1, &fds))
                {
                      l=recv(sockfd1, buffer, MAXSIZE, 0);
                      break;
                }
          }
          if(l<=0)
          {
    //            printf("[-] There is a error...Create a new connection.\r\n");
                continue;
          }
          while(1)
          {
    //            printf("[+] Connect OK!\r\n");
    //            printf("[+] Make a Connection to %s:%d....\r\n", host2,port2);
                fflush(stdout);
                if(client_connect(sockfd2,host2,port2)==0)
                {
                      close(sockfd1);
                      close(sockfd2);
                      continue;
                }
                if(send(sockfd2,buffer,l,0)<0)
                {
   //                   printf("[-] Send failed.\r\n");
                      continue;
                }
                l=0;
                memset(buffer,0,MAXSIZE);
                break;
          }

   //       printf("[+] All Connect OK!\r\n");
          sock.fd1 = sockfd1;
          sock.fd2 = sockfd2;
    if(sock.fd1<0||sock.fd2<0){
   //  printf("sock error,wait another client.\r\n\n");
 continue;
          }
  if( pthread_create(&t,&attr,thread,(void *)&sock)!=0){
   //  printf("[-] CreateThread Failed!\r\n\n");
 continue;
          }
          }
  //        printf("[+] CreateThread OK!\r\n\n");


}
//************************************************************************************
//
// Socket Transmit to Socket
//
//************************************************************************************
void *thread(void *m)
{
    int fd1, fd2;
    struct transocket *sock=m;
    fd_set readfd,writefd;
    int result,i=0;
    char read_in1[MAXSIZE],send_out1[MAXSIZE];
    char read_in2[MAXSIZE],send_out2[MAXSIZE];
    int read1=0,totalread1=0,send1=0;
    int read2=0,totalread2=0,send2=0;
    int sendcount1,sendcount2;
    int maxfd;
    struct sockaddr_in client1,client2;
    int structsize1,structsize2;
    char host1[20],host2[20];
    int port1=0,port2=0;
    char tmpbuf[100];
    fd1 = sock->fd1;
    fd2 = sock->fd2;
    memset(host1,0,20);
    memset(host2,0,20);
    memset(tmpbuf,0,100);
    structsize1=sizeof(struct sockaddr);
    structsize2=sizeof(struct sockaddr);

    if(getpeername(fd1,(struct sockaddr *)&client1,&structsize1)<0)
    {
          strcpy(host1, "fd1");
    }
    else
    {
    //       printf("[+]got, ip:%s, port:%d\r\n",inet_ntoa(client1.sin_addr),ntohs(client1.sin_port));
          strcpy(host1, (char *)inet_ntoa(client1.sin_addr));
          port1=ntohs(client1.sin_port);
    }
    if(getpeername(fd2,(struct sockaddr *)&client2,&structsize2)<0)
    {
          strcpy(host2,"fd2");
    }
    else
    {
      //     printf("[+]got, ip:%s, port:%d\r\n",inet_ntoa(client2.sin_addr),ntohs(client2.sin_port));
          strcpy(host2, (char *)inet_ntoa(client2.sin_addr));
          port2=ntohs(client2.sin_port);
    }
//    printf("[+] Start Transmit (%s:%d <-> %s:%d) ......\r\n\n", host1, port1, host2, port2);

    maxfd=max(fd1,fd2)+1;
    memset(read_in1,0,MAXSIZE);
    memset(read_in2,0,MAXSIZE);
    memset(send_out1,0,MAXSIZE);
    memset(send_out2,0,MAXSIZE);
    while(1)
    {
          FD_ZERO(&readfd);
          FD_ZERO(&writefd);

          FD_SET(fd1, &readfd);
          FD_SET(fd1, &writefd);
          FD_SET(fd2, &writefd);
          FD_SET(fd2, &readfd);

          result=select(maxfd,&readfd,&writefd,NULL,NULL);
          if(result<0)
          {
  //              printf("[-] Select error.\r\n");
                break;
          }
          else if(result==0)
          {
 //               printf("[-] Socket time out.\r\n");
                break;
          }

          if(FD_ISSET(fd1, &readfd))
      {
                /* must < MAXSIZE-totalread1, otherwise send_out1 will flow */
                if(totalread1<MAXSIZE)
              {
                      read1=recv(fd1, read_in1, MAXSIZE-totalread1, 0);
                      if(read1==0)
                      {
    //                        printf("[-] Read fd1 data error,maybe close?\r\n");
                            break;
                      }

                      memcpy(send_out1+totalread1,read_in1,read1);
  //                    sprintf(tmpbuf,"\r\nRecv %5d bytes from %s:%d\r\n", read1, host1, port1);
  //                    printf(" Recv %5d bytes %16s:%d\r\n", read1, host1, port1);
                      makelog(tmpbuf,strlen(tmpbuf));
                      makelog(read_in1,read1);
                      totalread1+=read1;
                      memset(read_in1,0,MAXSIZE);
                }
          }
          if(FD_ISSET(fd2, &writefd))
          {
                int err=0;
                sendcount1=0;
                while(totalread1>0)
                {
                      send1=send(fd2, send_out1+sendcount1, totalread1, 0);
                      if(send1==0)break;
                      if(send1<0)
                      {
    //                        printf("[-] Send to fd2 unknow error.\r\n");
                            err=1;
                            break;
                      }

                      sendcount1+=send1;
                      totalread1-=send1;
      //                printf(" Send %5d bytes %16s:%d\r\n", send1, host2, port2);
                }

                if(err==1) break;
                if((totalread1>0) && (sendcount1>0))
                {
                      /* move not sended data to start addr */
                      memcpy(send_out1,send_out1+sendcount1,totalread1);
                      memset(send_out1+totalread1,0,MAXSIZE-totalread1);
                }
                else
                memset(send_out1,0,MAXSIZE);
          }

          if(FD_ISSET(fd2, &readfd))
          {
                if(totalread2<MAXSIZE)
                {
                      read2=recv(fd2,read_in2,MAXSIZE-totalread2, 0);
                      if(read2==0)break;
                      if(read2<0)
                      {
      //                      printf("[-] Read fd2 data error,maybe close?\r\n\r\n");
                            break;
                      }
                      memcpy(send_out2+totalread2,read_in2,read2);
     //                 sprintf(tmpbuf, "\r\nRecv %5d bytes from %s:%d\r\n", read2, host2, port2);
     //                 printf(" Recv %5d bytes %16s:%d\r\n", read2, host2, port2);
                      makelog(tmpbuf,strlen(tmpbuf));
                makelog(read_in2,read2);
                totalread2+=read2;
                memset(read_in2,0,MAXSIZE);
                }
   }
          if(FD_ISSET(fd1, &writefd))
      {
                int err2=0;
              sendcount2=0;
              while(totalread2>0)
              {
                    send2=send(fd1, send_out2+sendcount2, totalread2, 0);
                    if(send2==0)break;
                    if(send2<0)
                    {
       //                   printf("[-] Send to fd1 unknow error.\r\n");
                            err2=1;
                          break;
                    }
                    sendcount2+=send2;
                    totalread2-=send2;

       //             printf(" Send %5d bytes %16s:%d\r\n", send2, host1, port1);
              }
                if(err2==1) break;
            if((totalread2>0) && (sendcount2 > 0))
                {
                      /* move not sended data to start addr */
                      memcpy(send_out2, send_out2+sendcount2, totalread2);
                      memset(send_out2+totalread2, 0, MAXSIZE-totalread2);
                }
                else
                      memset(send_out2,0,MAXSIZE);
          }
   //       Sleep(5);
    }

    close(fd1);
    close(fd2);
//      if(method == 3)
//            connectnum --;

  //  printf("\r\n[+] OK! I Closed The Two Socket.\r\n");
}
void closeallfd()
{
    int i;
   // printf("[+] Let me exit ......\r\n");
    fflush(stdout);
    for(i=3; i<256; i++)
    {
          close(i);
    }
    if(fp != NULL)
    {
          fprintf(fp,"\r\n====== Exit ======\r\n");
          fclose(fp);
    }
    printf("[+] All Right!\r\n");
}
int create_socket()
{
    int sockfd;
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
    {
          printf("[-] Create socket error.\r\n");
          return(0);
    }

    return(sockfd);
}
int create_server(int sockfd,int port)
{
    struct sockaddr_in srvaddr;
    int on=1;

    memset(&srvaddr, 0, sizeof(struct sockaddr));
    srvaddr.sin_port=htons(port);
    srvaddr.sin_family=AF_INET;
    srvaddr.sin_addr.s_addr=htonl(INADDR_ANY);

    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR, (char*)&on,sizeof(on)); //so I can rebind the port
    if(bind(sockfd,(struct sockaddr *)&srvaddr,sizeof(struct sockaddr))<0)
    {
          printf("[-] Socket bind error.\r\n");
          return(0);
    }
    if(listen(sockfd,CONNECTNUM)<0)
    {
          printf("[-] Socket Listen error.\r\n");
          return(0);
    }

    return(1);
}
int client_connect(int sockfd,char* server,int port)
{
 struct sockaddr_in cliaddr;
 struct hostent *host;
 if(!(host=gethostbyname(server)))
 {
       printf("[-] Gethostbyname(%s) error:%s\n",server,strerror(errno));
       return(0);
 }

 memset(&cliaddr, 0, sizeof(struct sockaddr));
 cliaddr.sin_family=AF_INET;
 cliaddr.sin_port=htons(port);
 cliaddr.sin_addr=*((struct in_addr *)host->h_addr);

 if(connect(sockfd,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr))<0)
 {
       printf("[-] Connect error.\r\n");
       return(0);
 }
 return(1);
}
void makelog(char *buffer,int length)
{
    if(fp !=NULL)
    {
          write(fileno(fp),buffer,length);
    }
}
void setup(pthread_attr_t *attr)
{
pthread_attr_init(attr);
pthread_attr_setdetachstate(attr,PTHREAD_CREATE_DETACHED);
}
int max(int i,int j)
{
return i>j?i:j;
}
void daemonize(void)
{
       int i;

       setuid(0);
       setgid(0);
       seteuid(0);
       setegid(0);

       signal(SIGCHLD,SIG_IGN);
       signal(SIGHUP,SIG_IGN);
       signal(SIGTERM,SIG_IGN);
       signal(SIGINT,SIG_IGN);
       signal(SIGKILL,SIG_IGN);

       if(fork())
               exit(0);

       setpgrp();

}
