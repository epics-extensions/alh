/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 Deutches Elektronen-Synchrotron in der Helmholtz-
* Gemelnschaft (DESY).
* Copyright (c) 2002 Berliner Speicherring-Gesellschaft fuer Synchrotron-
* Strahlung mbH (BESSY).
* Copyright (c) 2002 Southeastern Universities Research Association, as
* Operator of Thomas Jefferson National Accelerator Facility.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/
/************************DESCRIPTION***********************************
  DataBase call routine. Work like independent process.
We have simple Perl call and more usefull C-RPC call. 
rpc_gen X-files for RPC is :
struct DBSend {string msg<>;};
program DB_PORT {
 version DB_VERS {
         void dbSend(DBSend) = 1;
         } =1;
}=port;
**********************************************************************/

#define C_CALL
#undef  PERL_CALL
/* #define PERL_CALL */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alh.h>
#ifdef WIN32
#include <process.h>
#else
#include <unistd.h>
#endif
#ifdef HAVE_SYSV_IPC
#include <sys/msg.h>
#endif
#include <errno.h>

#ifdef  PERL_CALL
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
int TCPInit(char *hostname,int port);
int put2TCP(char *hostname,int port,char *string,int len);
#endif

#ifdef  C_CALL
#include <rpc/rpc.h>
struct DBSend {
	char *msg;
};
typedef struct DBSend DBSend;
#define	DB_VERS ((unsigned long)(1))
#define	dbSend ((unsigned long)(1))
extern  void * dbsend_1();
extern bool_t xdr_DBSend();
static struct timeval TIMEOUT = { 10, 0 };
#endif

#define DEBUG 1

int put2RPC(char *host ,int port,char *msg,int len);

char msg[250];

int main(argc,argv)
int argc;
char *argv[];
{
struct msqid_ds infoBuf;  
int DBMsgQId;
int DBMsgQKey;
int port;
int bytes=250;

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
	      fprintf(stderr,"owner = %d.%d, perms = %04o, max bytes = %ld\n",
		      infoBuf.msg_perm.uid,infoBuf.msg_perm.gid,
		      infoBuf.msg_perm.mode,infoBuf.msg_qbytes);
	      fprintf(stderr,"%ld msgs = %ld bytes on queue\n",
		      infoBuf.msg_qnum, infoBuf.msg_cbytes);
	    }
	  else {perror("msgctl()");  exit(1);}
	  }
#ifdef PERL_CALL
	fprintf(stderr,"Please wait ...\n");
	if( (socket=TCPInit(argv[1],port)) <= 0 ) { 
	  perror("can't connect to TCP task"); 
	  exit(1);
	}
	else fprintf(stderr,"TCPInit for %s port=%d is OK\n",argv[1],port);
        close(socket);
        sleep(2);
#endif
		if(DEBUG)  fprintf(stderr,"star\n");

	while(1)
	  {
	    if( msgrcv(DBMsgQId,msg,bytes,0,0) >= 0)  /* got a new message */
	      {
		if(DEBUG)  fprintf(stderr,"msg=%s\n",msg);
#ifdef C_CALL
		put2RPC(argv[1],port,msg,strlen(msg));
#else
		put2TCP(argv[1],port,msg,strlen(msg));
#endif
	      }
	    else usleep(1000); /* # in microsec */ 
	  }
	
}

#ifdef C_CALL
int put2RPC(char *host ,int port,char *msg,int len)
{
	CLIENT *clnt;
	void  *result_1;
	DBSend  dbsend_1_arg;
	dbsend_1_arg.msg=msg;

	clnt = clnt_create(host, port, DB_VERS, "netpath");
	if (clnt == (CLIENT *) NULL) {
		clnt_pcreateerror(host);
		return(-1);
	}
	result_1 = dbsend_1(&dbsend_1_arg, clnt);
	if (result_1 == (void *) NULL) {
		clnt_perror(clnt, "call failed");
	}
	clnt_destroy(clnt);
return (0);
}
void *
dbsend_1(argp, clnt)
	DBSend *argp;
	CLIENT *clnt;
{
	static char clnt_res;

	memset((char *)&clnt_res, 0, sizeof (clnt_res));
	if (clnt_call(clnt, dbSend,
		(xdrproc_t) xdr_DBSend, (caddr_t) argp,
		(xdrproc_t) xdr_void, (caddr_t) &clnt_res,
		TIMEOUT) != RPC_SUCCESS) {
		return (NULL);
	}
	return ((void *)&clnt_res);
}

bool_t
xdr_DBSend(xdrs, objp)
	register XDR *xdrs;
	DBSend *objp;
{

	if (!xdr_string(xdrs, &objp->msg, ~0))
		return (FALSE);
	return (TRUE);
}
#endif

#ifdef PERL_CALL
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
#endif
