/*
 $Log$
 Revision 1.3  1995/06/22 19:39:39  jba
 Started cleanup of file

 * Revision 1.2  1994/06/22  21:17:33  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)guidance.c	1.5\t9/9/93";

/* guidance.c
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

#include <Xm/Xm.h>

#include <alLib.h>
#include <ax.h>

/****************************************************
*      guidance.c 
*
*This file contains routine for displaying group or channel guidance.
*
--------------
|   PUBLIC   |
--------------
*
void guidance_callback(widget,gclink,cbs)    guidance callback
     Widget widget;
     CLINK *gclink;
     XmAnyCallbackStruct *cbs;
*
***************************************************************/

void guidance_callback(w,gclink,cbs)
Widget w;
GCLINK *gclink;
XmAnyCallbackStruct *cbs;
{

     SNODE *pt;
     struct guideLink *guidelist;
     char *guidance_str[200];
     int i=0;

     pt = sllFirst(&(gclink->GuideList));
     i=0;
     while (pt) {
          guidelist = (struct guideLink *)pt;
          guidance_str[i] = guidelist->list;
          pt = sllNext(pt);
          i++;
     }
     guidance_str[i] = "";
     guidance_str[i+1] = "";

     xs_help_callback(w,guidance_str,cbs);

}

