/*
 $Log$
 Revision 1.3  1995/05/30 16:04:36  jba
 Renamed guidance_spawn_callback to processSpawn_callback

 * Revision 1.2  1994/06/22  21:17:47  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)process.c	1.4\t8/12/93";

/* process.c
 *
 *      Author: Ben-chin Cha
 *      Date:   12-20-90
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01  10-04-91        bkc     Redesign the setup window,
 *                              resolve problems with new config,
 *				separate force variables / process
 *				reposition the group force dialog box
 * .02  02-16-93        jba     Reorganized files for new user interface
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

#include <stdio.h>
#include <Xm/Xm.h>
#include <alLib.h>
#include <ax.h>

/****************************************************
*      process.c 
*
*This file contains routines for spawning a related process
*
--------------
|   PUBLIC   |
--------------
void processSpawn_callback(w,command,call_data)      Spawn a related process
     Widget w;
     char *command;
     void *call_data;
*
void relatedProcess_callback(widget,link,cbs) Related process callback
     void *widget;
     GCLINK *link;
     void *cbs;

     
***************************************************************/

/******************************************************
  spawn a new related prcess if command is not null
******************************************************/
void processSpawn_callback(w,command,call_data)
Widget w;
char *command;
void * call_data;
{
char buff[120];
int l;



	if (command) {
	l = strlen(command);
	if (*(command+l-1) == '\n') *(command+l-1) = ' ';
	    	sprintf(buff,"%s &",command);
	    	system(buff);
		}


}	

/***************************************************
  relatedProcess_callback
****************************************************/

void relatedProcess_callback(widget,link,cbs)
     void *widget;
     GCLINK *link;
     void *cbs;
{
     void *area;

     XtVaGetValues(widget, XmNuserData, &area, NULL);

     if (link && alProcessExists(link)){
          processSpawn_callback(widget,link->pgcData->command,cbs);
     }
}

