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
/* process.c */

/************************DESCRIPTION***********************************
  This file contains routines for spawning a related process
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef WIN32
#include <process.h>     /* For spawn */
#endif
#include <Xm/Xm.h>

#include "alLib.h"
#include "ax.h"

/******************************************************
  spawn a new related prcess if command is not null
******************************************************/
void processSpawn_callback(Widget w,char *command,void * call_data)
{
	char buff[MAX_STRING_LENGTH +2];
	int l;
	int status;
#ifdef WIN32
	static int first=1;
	static char *ComSpec;
#endif    

	if (command) {
		/* Strip any LF from the end */
		l = strlen(command);
		if (*(command+l-1) == '\n') *(command+l-1) = ' ';
#ifdef WIN32
		sprintf(buff,"%s",command);
		/* Get ComSpec for the command shell (should be defined) */
		if (first) {
			first=0;
			ComSpec = getenv("ComSpec");
		}
		if (!ComSpec) {
			errMsg("processSpawn_callback: Cannot find command processor\n");
			return;
		}
		status = _spawnl(_P_DETACH, ComSpec, ComSpec, "/C", buff, NULL);
#else
		sprintf(buff,"%s &",command);
		status=system(buff);
#endif	
		if(status == -1) {
			/* System call failed */
			char *errstring=strerror(errno);

			errMsg("processSpawn_callback: Cannot process command:\n"
			    "%s\n  %s",
			    buff,errstring);
		} else if (status > 0) {
			/* Assume program returned an error */
			errMsg("processSpawn_callback: Command returned %d:\n"
			    "%s\n",
			    status,buff);
		}
	}
}

/***************************************************
  relatedProcess_callback
****************************************************/
void relatedProcess_callback(void *widget,GCLINK *link,void *cbs)
{
	void *area;

	XtVaGetValues(widget, XmNuserData, &area, NULL);

	if (link && alProcessExists(link)){
		processSpawn_callback(widget,link->pgcData->command,cbs);
	}
}

