/* printer.c */

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
#include <netdb.h>
#include <arpa/inet.h>
#include <alarm.h> 
#include "printer.h"
char *colorStart[ALARM_NSEV];
int colorStartLen[ALARM_NSEV];
char *colorEnd;
int colorEndLen;
char buff[300];

int main(argc,argv)
int argc;
char *argv[];
{
	struct  sockaddr_in s_name;
	struct  hostent *h_ptr;
	int s;
	int ret;
	if (argc < 7) {
		fprintf(stderr,"usage:%s Name port ColorModel len_mes sev mes\n",argv[0]);
		return(0);
	}

	if (strcmp(argv[3],"hp_color") == 0) {
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
	sprintf(buff,"%s%s%s\n",colorStart[atoi(argv[5])],
	    argv[6],colorEnd);
	while(1)
	{
		if ((s=socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror( "socket"); 
			return(-1);
		}
		if ((h_ptr = gethostbyname (argv[1])) == NULL) {
			perror("gethostbyname"); 
			return(-1);
		}
		s_name.sin_family = AF_INET;
		s_name.sin_port = htons(atoi(argv[2]));
		s_name.sin_addr = * ((struct in_addr *) h_ptr->h_addr);

		if (connect(s, (struct sockaddr *) &s_name, sizeof (s_name)) ) {
			close(s);
			sleep(1);
			continue;
		}
		break;
	}

	ret = send(s,buff,strlen(buff),0);
	if (ret < 0) {
		perror ("send"); 
		close(s); 
		return(-1);
	}
	if (ret != strlen(buff) ) {
		perror ("Send: Wrong length.");
	}
	close(s);
	return(0);
}



