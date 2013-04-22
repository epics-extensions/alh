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
/* browser.c */

/************************DESCRIPTION***********************************
  Invokes default browser
  Original Author : Kenneth Evans, Jr.
**********************************************************************/

/* Note that there are separate WIN32 and UNIX versions */

#define DEBUG 0

#ifndef WIN32
/*************************************************************************/
/*************************************************************************/
/* Netscape UNIX Version                                                        */
/*************************************************************************/
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>

#include <X11/Xlib.h>

/* Function prototypes */

extern int kill(pid_t, int);     /* May not be defined for strict ANSI */

int callBrowser(char *url);
static int execute(char *s);

/* Global variables */
extern Display *display;

/**************************** callBrowser ********************************/
int callBrowser(char *url)
/* Returns non-zero on success, 0 on failure      */
/* url is the URL that the browser is to display  */
/*   or "quit" to terminate the browser           */
{
	static pid_t pid=0;
	int status;
	char command[BUFSIZ];

	/* Handle quit */
	if(!strcmp(url,"quit")) {
		if (pid) {
			kill(pid,SIGTERM);
			pid=0;
		}
		return 3;
	}
#if defined __linux__    /* defined by gnu compiler preprocessor */
	sprintf(command,"xdg-open \"%s\" &",url);
#elif defined SOLARIS    /* defined in EPICS base configure files */
	sprintf(command,"sdtwebclient \"%s\" &",url);
#elif defined darwin    /* defined in EPICS base configure files */
	sprintf(command,"open \"%s\" &",url);
#elif defined __CYGWIN32__  /* defined by gnu compiler preprocessor */
	sprintf(command,"cygstart \"%s\" &",url);
#elif defined USE_HTMLVIEW
 	sprintf(command,"htmlview \"%s\" &",url);
#else
	return 1;
#endif 
#if DEBUG
	printf("execute(before): cmd=%s\n",command);
#endif    
	status=execute(command);
#if DEBUG
	printf("execute(after): cmd=%s status=%d\n",command,status);
#endif    
	return 2;
}

/**************************** execute ************************************/
static int execute(char *s)
/* From O'Reilly, Vol. 1, p. 438 */
{
	int status,pid,w;
	register void (*istat)(),(*qstat)();

	if((pid=fork()) == 0) {
		signal(SIGINT,SIG_DFL);
		signal(SIGQUIT,SIG_DFL);
		signal(SIGHUP,SIG_DFL);
		execl("/bin/sh","sh","-c",s,(char *)0);
		_exit(127);
	}
	istat=signal(SIGINT,SIG_IGN);
	qstat=signal(SIGQUIT,SIG_IGN);
	while((w=wait(&status)) != pid && w != -1) ;
	if(w == -1) status=-1;
	signal(SIGINT,istat);
	signal(SIGQUIT,qstat);
	return(status);
}

#else 
/*************************************************************************/
/*************************************************************************/
/* WIN32 Version                                                        */
/*************************************************************************/
/*************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <errno.h>

/* void medmPrintf(int priority, char *format, ...); */

int callBrowser(char *url);

/**************************** callBrowser (WIN32) ************************/
int callBrowser(char *url)
/* Returns non-zero on success, 0 on failure */
/* Should use the default browser            */
/* Does nothing with "quit"                  */
{
	static int first=1;
	static char *ComSpec;
	char command[BUFSIZ];
	intptr_t status;

	/* Handle quit */
	if(!strcmp(url,"quit")) {
		/* For compatibility, but do nothing */
		return(3);
	}

	/* Get ComSpec for the command shell (should be defined) */
	if (first) {
		first=0;
		ComSpec = getenv("ComSpec");
	}
	if (!ComSpec) return(0);     /* Abort with no message like the UNIX version*/

	/* Spawn the process that handles a url */
	/* This seems to work on 95 and NT, with a command box on 95
	   *   It may have trouble if the URL has spaces */
	sprintf(command,"start %s",url);
	status = _spawnl(_P_DETACH, ComSpec, ComSpec, "/C", command, NULL);

	if(status == -1) {
		char *errstring=strerror(errno);

		printf("\ncallBrowser: Cannot start browser:\n"
		    "%s %s\n"
		    "  %s\n",ComSpec,command,errstring);
		/* 	perror("callBrowser:"); */
		return(0);
	}
	return(1);
}
#endif



