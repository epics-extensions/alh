/************************DESCRIPTION***********************************
  Printer routine. Work like independent process. Albert 
**********************************************************************/

static char *sccsId = "@(#) $Id$";

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
#include "printer.h"
#define DEBUG 0

char *colorStart[ALARM_NSEV]; 
int colorStartLen[ALARM_NSEV];
char *colorEnd;
int colorEndLen;
char msg[250];
char buff[250];

int printerInit(char *,int);
int put2printer(char *,int,char *,int);
int compressMsg(char * msg, char *compress_buff);

int main(argc,argv)
int argc;
char *argv[];
{
int sev; /* define color for printer */
struct msqid_ds infoBuf;  
int printerMsgQId;
int printerMsgQKey;
int port;
int bytes=250;
int socket;

	if (argc != 5) {
		fprintf(stderr,"usage:%s TCPName TCPport Key ColorModel\nColor model={bw,bw_bold,oki_bold,hp_color}\n",argv[0]);
		exit(1);
	}
	if ( !(printerMsgQKey=atoi(argv[3])) || !(port=atoi(argv[2])) )  {
		fprintf(stderr,"usage:%s TCPName TCPport Key ColorModel\nColor model={bw,bw_bold,oki_bold,hp_color}\n",argv[0]);
		exit(1);
	}

	if (strcmp(argv[4],"hp_color") == 0) {
		colorStart[NO_ALARM]=&colorStartNoHpColor[0];
		colorStartLen[NO_ALARM]=colorStartNoHpColorLen;
		colorStart[MINOR_ALARM]=&colorStartMinorHpColor[0];
		colorStartLen[MINOR_ALARM]=colorStartMinorHpColorLen;
		colorStart[MAJOR_ALARM]=&colorStartMajorHpColor[0];
		colorStartLen[MAJOR_ALARM]=colorStartMajorHpColorLen;
		colorStart[INVALID_ALARM]=&colorStartMinorHpColor[0];
		colorStartLen[INVALID_ALARM]=colorStartInvalidHpColorLen;
		colorEnd=&colorEndHpColor[0];
		colorEndLen=colorEndHpColorLen;
	}

	else if (strcmp(argv[4],"bw_bold") == 0)  {
		colorStart[NO_ALARM]=&colorStartNoMonoBold[0];
		colorStartLen[NO_ALARM]=colorStartNoMonoBoldLen;
		colorStart[MINOR_ALARM]=&colorStartMinorMonoBold[0];
		colorStartLen[MINOR_ALARM]=colorStartMinorMonoBoldLen;
		colorStart[MAJOR_ALARM]=&colorStartMajorMonoBold[0];
		colorStartLen[MAJOR_ALARM]=colorStartMajorMonoBoldLen;
		colorStart[INVALID_ALARM]=&colorStartMinorMonoBold[0];
		colorStartLen[INVALID_ALARM]=colorStartInvalidMonoBoldLen;
		colorEnd=&colorEndMonoBold[0];
		colorEndLen=colorEndMonoBoldLen;
	}

	else if (strcmp(argv[4],"oki_bold") == 0)  {
		colorStart[NO_ALARM]=&colorStartNoOkiBold[0];
		colorStartLen[NO_ALARM]=colorStartNoOkiBoldLen;
		colorStart[MINOR_ALARM]=&colorStartMinorOkiBold[0];
		colorStartLen[MINOR_ALARM]=colorStartMinorOkiBoldLen;
		colorStart[MAJOR_ALARM]=&colorStartMajorOkiBold[0];
		colorStartLen[MAJOR_ALARM]=colorStartMajorOkiBoldLen;
		colorStart[INVALID_ALARM]=&colorStartMinorOkiBold[0];
		colorStartLen[INVALID_ALARM]=colorStartInvalidOkiBoldLen;
		colorEnd=&colorEndOkiBold[0];
		colorEndLen=colorEndOkiBoldLen;
	}

        else {
		colorStart[NO_ALARM]=&colorStartNoMono[0];
		colorStartLen[NO_ALARM]=colorStartNoMonoLen;
		colorStart[MINOR_ALARM]=&colorStartMinorMono[0];
		colorStartLen[MINOR_ALARM]=colorStartMinorMonoLen;
		colorStart[MAJOR_ALARM]=&colorStartMajorMono[0];
		colorStartLen[MAJOR_ALARM]=colorStartMajorMonoLen;
		colorStart[INVALID_ALARM]=&colorStartMinorMono[0];
		colorStartLen[INVALID_ALARM]=colorStartInvalidMonoLen;
		colorEnd=&colorEndMono[0];
		colorEndLen=colorEndMonoLen;
	} 
       
	printerMsgQId = msgget (printerMsgQKey, 0600|IPC_CREAT);
	if(printerMsgQId == -1) {perror("msgQ_create"); exit(1);}
	else 
	  {
	  fprintf(stderr,"msgQ with key=%d is OK\n",printerMsgQKey);
	  if (msgctl(printerMsgQId,IPC_STAT,&infoBuf) != 1)
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

	if( (socket=printerInit(argv[1],port)) <= 0 ) { 
	  perror("can't connect to TCPprinter"); 
	  exit(1);
	}
	else fprintf(stderr,"printerInit for %s port=%d is OK\n",argv[1],port);
        close(socket);
        sleep(2);

	while(1)
	{
	if( msgrcv(printerMsgQId,msg,bytes,0,0) >= 0)  /* got a new message */
	{
	  if ( (sev=compressMsg(msg+2,buff)) < 0 ) /* msg's big for 80char printer:cut it*/  
	    { perror("bad format"); continue;}                     
	  put2printer(argv[1],port,buff,strlen(buff));
	}
	else usleep(1000); /* # in microsec */ 
	}

}

int printerInit(char *hostname,int port)
{
int printerSocket;
struct  sockaddr_in s_name; 
struct  hostent *h_ptr;
int true=1;

if ((printerSocket=socket(AF_INET, SOCK_STREAM, 0)) == -1){
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
if(setsockopt(printerSocket,SOL_SOCKET,SO_KEEPALIVE,(char *)&true,sizeof(true)))  {
  perror("Keepalive option set failed");
  return(-1);  
  }
true=1;

if(setsockopt(printerSocket,IPPROTO_TCP,TCP_NODELAY,(char *)&true,sizeof(true))){
  perror("nodelay option set failed");
  return(-1);  
  }
true=128;
if(setsockopt(printerSocket,SOL_SOCKET,SO_SNDBUF,(char *)&true,sizeof(true))){
  perror("sendbuf option set failed");
  return(-1);  
  }

if (connect(printerSocket, (struct sockaddr *) &s_name, sizeof (s_name)) ) {
  perror("connect");
  close(printerSocket);
  return(-1);
}

return(printerSocket);
}


int put2printer(char *hostname,int port,char *string,int len) 
{
int printerSock;
int ret;

while(1)
  {
	if( (printerSock=printerInit(hostname,port)) <= 0) { 
	  sleep(1); 
          continue; 
	}
	ret=send(printerSock,string,len,0);
	if (ret != strlen(string)) {
	  fprintf(stderr,"Printer:Send_byte=%d need%d\n",ret,len);
	}
        close(printerSock);
        usleep(1000);
	return(0);

  }
}

int compressMsg(char * msg, char *compress_buff)
{
int sev,type;
char *blank;
  /* We assume that mess has next format: 
     "typeOfRecord+1 time_stamp  buff"
             in regular case buff =(name al al_type h_al h_al_type value)
   then we calculate sevirity: if typeOfRecord > 0 sev=MAJOR=2 -- service error;
   if typeOfRecord=0 we calculate 3-rd word in buff  
   if word = NO_ALARM sev=0
             MINOR sev=1;
	     MAJOR sev=2;
	     INVALID sev =3;
	     all other sev=2;
	     
   in regular case (typeOfRecord = 0) we cut  al al_type h_al h_al_type till 5 char
   in begin of compress_buff we add ColorStart-symbols and finish it ColorEnd-symbols,
   so printer will print it in color!!!
   */ 

if ( ( type = atoi(msg) ) == 0 ) {
  fprintf(stderr,"%s: first element must be number\n",msg);
  return(-1); 
} 

if (type > 1) {
  sprintf(compress_buff,"%s%s%s",colorStart[MAJOR_ALARM],msg+2,colorEnd);
  if(DEBUG) fprintf(stderr,"buff=%s;",compress_buff);
  return(MAJOR_ALARM);
}

/* EXAMPLE:0 01-Apr-1999 12:39:49 AHTST:out2_ao NO_ALARM NO_ALARM HIHI MAJOR 0.00  
              + 20               +1       +28  +1  +12  +1 +16   +1+12+1+16 +1           
           | |                    |             |        |        |    |     |         
           0 2                    23            52       65       82   95    112  
                                                              */

if      (strncmp(msg+52,"NO_ALARM",7) == 0) sev=0;
else if (strncmp(msg+52,"MINOR"   ,5) == 0) sev=1;
else if (strncmp(msg+52,"INVALID", 6) == 0) sev=3;
else sev=2;

strcpy (compress_buff,colorStart[sev]);              /* color symb */
strncat(compress_buff,msg+2,20);                     /* TS */

if ( (blank = strchr( (const char *) msg+23, ' ')) == NULL) {
  fprintf(stderr,"%s: bad 1 blank in msg \n",msg);
  return(-1); 
} 

strncat(compress_buff,msg+22,blank - msg-22 );        /* blank + name */
strncat(compress_buff,msg+51,6);                      /* blank + first 5 symbols of alarm */
strncat(compress_buff,msg+64,6);                      /* blank + first 5 symbols of sev */
strcat (compress_buff,msg+111);                       /* blank +value */
strcat (compress_buff,colorEnd);                      /* color symb */
strcat (compress_buff,"\n");                          /*  \n-it's important for printer */
 
if(DEBUG) fprintf(stderr,"uncompress buff=%s;\n  compress buff=%s;\n",msg,compress_buff);

return(sev);
}










