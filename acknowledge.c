/*
 $Log$
 Revision 1.2  1994/06/22 21:16:20  jba
 Added cvs Log keyword

 */

 /*  @(#)acknowledge.c  */
static char *sccsId = "@(#)acknowledge.c	1.4\t9/9/93";

 /* share/src/alh        @(#)acknowledge.c	1.4     9/9/93 
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
 * .01  02-16-93        jba     Reorganized files for new user interface
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

#include <stdio.h>

#include <Xm/Xm.h>

#include <alh.h>
#include <alLib.h>
#include <line.h>
#include <ax.h>


/*
******************************************************************
	routines defined in acknowledge.c
*******************************************************************
	acknowledge.c 
*
*	This file contains routines for alarm acknowledgement
*
******************************************************************
-------------
|  PUBLIC   |
-------------
*
void ack_callback(widget,line,cbs)      Line acknowledge callback
     Widget widget;
     struct anyLine  *line;
     XmAnyCallbackStruct *cbs;
-------------
|  PRIVATE  |
-------------
*
static void 
ackChan(cline)				Channel acknowledge callback
 	struct chanLine *cline;
*
static void 
ackGroup(gline)				Group acknowledge callback
  	struct groupLine *gline;
*
******************************************************************
*/

/***************************************************
 channel acknowledge button callback 
****************************************************/

static void ackChan(cline)
  struct chanLine *cline;
{
  CLINK *clink;
  struct chanData *pdata;


  clink = (CLINK *)cline->clink;
  pdata = clink->pchanData;
  if ( pdata->unackSevr == 0) return;
  alLogAckChan(cline);
  alAckChan(clink);
/*  awInvokeCallback(); */
  clink->pmainGroup->modified = 1;
 
}

/***************************************************
 group acknowledge button callback 
****************************************************/

static void ackGroup(gline)
  struct groupLine *gline;
{
  GLINK *glink;


  glink = (GLINK *)gline->glink;
  if ( alHighestSeverity(glink->pgroupData->unackSev) == 0) return;
  alLogAckGroup(gline);
  alAckGroup(glink);
/*  awInvokeCallback(); */ 
  glink->pmainGroup->modified = 1;
 
}

/***************************************************
  ack_callback
****************************************************/
 
void ack_callback(widget,line,cbs)
     Widget widget;
     struct anyLine  *line;
     XmAnyCallbackStruct *cbs;
{
     if (line->linkType == GROUP) ackGroup((struct groupLine *)line); 
     else if (line->linkType == CHANNEL) ackChan((struct chanLine *)line); 
}

