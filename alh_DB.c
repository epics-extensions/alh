/************************DESCRIPTION***********************************
  DataBase call routine. Work like independent process. Albert 
**********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/msg.h> 
#include <errno.h>
#include <alarm.h>
#define DEBUG 0

char msg[250];

int TCPInit(char *hostname,int port);
int put2TCP(char *hostname,int port,char *string,int len);

int main(argc,argv)
int argc;
char *argv[];
{
struct msqid_ds infoBuf;  
int DBMsgQId;
int DBMsgQKey;
int port;
int bytes=250;
char *blank;
int sev;
int socket;

	if (argc != 4) {
	  fprintf(stderr,"usage:%s TCPName TCPport Key\n",argv[0]);
	  exit(1);
	}
	if ( !(DBMsgQKey=atoi(argv[3])) || !(port=atoi(argv[2])) )  {
	  fprintf(stderr,"usage:%s TCPName TCPport Key\n",argv[0]);
	  exit(1);
	}

	DBMsgQId = msgget (DBMsgQKey, 0600|IPC_CREAT);
	if(DBMsgQId == -1) {perror("msgQ_create"); exit(1);}
	else 
	  {
	  fprintf(stderr,"msgQ with key=%d is OK\n",DBMsgQKey);
	  if (msgctl(DBMsgQId,IPC_STAT,&infoBuf) != 1)
	    {
	      fprintf(stderr,"owner = %d.%d, perms = %04o, max bytes = %d\n",
		      infoBuf.msg_perm.uid,infoBuf.msg_perm.gid,
		      infoBuf.msg_perm.mode,infoBuf.msg_qbytes);
	      fprintf(stderr,"%d msgs = %d bytes on queue\n",
		      infoBuf.msg_qnum, infoBuf.msg_cbytes);
	    }
	  else {perror("msgctl()");  exit(1);}
	  }

	fprintf(stderr,"Please wait ...\n");

	if( (socket=TCPInit(argv[1],port)) <= 0 ) { 
	  perror("can't connect to TCP task"); 
	  exit(1);
	}
	else fprintf(stderr,"TCPInit for %s port=%d is OK\n",argv[1],port);
        close(socket);
        sleep(2);

	while(1)
	{
	if( msgrcv(DBMsgQId,msg,bytes,0,0) >= 0)  /* got a new message */
	{
        if(DEBUG)  fprintf(stderr,"msg=%s\n",msg);
	  if ( (blank = strchr( (const char *) &msg[0], ' ')) != NULL) 
	    {
	      *blank=0;
	      if ( (sev=atoi(msg)) == 0)
		{
		  fprintf(stderr,"msgrcv mast have number first,but msg=%s\n",msg);
		  continue;
		} 
	      sev--;
	      blank++;
	    }
	  else 
	    {
	      perror("msgrcv has bad format");
	      break; 
	    }

	  put2TCP(argv[1],port,blank,strlen(blank));
	}
	else usleep(1000); /* # in microsec */ 
	}

exit(1);
}

int TCPInit(char *hostname,int port)
{
int DBSocket;
struct  sockaddr_in s_name; 
struct  hostent *h_ptr;
int true=1;

if ((DBSocket=socket(AF_INET, SOCK_STREAM, 0)) == -1){
  perror( "socket"); 
  return(-1);
}

if ((h_ptr = gethostbyname(hostname)) ==  NULL ) 
  {
    perror("gethostbyname"); 
    exit(1);
  }

memset(&s_name, 0, sizeof(s_name));
s_name.sin_family = AF_INET;
s_name.sin_port = htons(port); 
memcpy(&s_name.sin_addr, h_ptr->h_addr, sizeof (s_name.sin_addr));

true=1;
if(setsockopt(DBSocket,SOL_SOCKET,SO_KEEPALIVE,(char *)&true,sizeof(true)))  {
  perror("Keepalive option set failed");
  return(-1);  
  }
true=1;

if(setsockopt(DBSocket,IPPROTO_TCP,TCP_NODELAY,(char *)&true,sizeof(true))){
  perror("nodelay option set failed");
  return(-1);  
  }
true=200;
if(setsockopt(DBSocket,SOL_SOCKET,SO_SNDBUF,(char *)&true,sizeof(true))){
  perror("sendbuf option set failed");
  return(-1);  
  }

if (connect(DBSocket, (struct sockaddr *) &s_name, sizeof (s_name)) ) {
  perror("connect");
  close(DBSocket);
  return(-1);
}

return(DBSocket);
}


int put2TCP(char *hostname,int port,char *string,int len) 
{
int DBSock;
int ret;

while(1)
  {
	if( (DBSock=TCPInit(hostname,port)) <= 0) { 
	  sleep(1); 
          continue; 
	}
	ret=send(DBSock,string,len,0);
	if (ret != strlen(string)) {
	  fprintf(stderr,"DB_call:Send_byte=%d need%d\n",ret,len);
	}
        close(DBSock);
        usleep(1000);
	return(0);

  }
}
