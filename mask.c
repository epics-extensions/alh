static char *sccsId = "@(#)mask.c	1.4\t9/9/93";

/* mask.c 
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
 * .01  02-16-93        jba     Reorganized files for new user interface
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>

#include <alh.h>
#include <axArea.h>
#include <ax.h>

extern struct setup psetup;

/* external routines
extern void xs_ok_callback();
extern XmString xs_str_array_to_xmstr();
extern void xs_help_callback();
extern Widget createShowChanMasksDialog();
extern Widget createForcePVChanDialog();
extern Widget createForceCMaskDialog();
*/



extern int DEBUG;

/*
******************************************************************
	routines defined in mask.c
*******************************************************************
*
*	This file contains alh routines for modifying masks
*
******************************************************************
-------------
|  PUBLIC  |
-------------
*
void changeMasks_callback(area,index)   Modify alarm settings callback
     ALINK *area;
     int index;

*
void 
chanChangeMasks_callback(link,index)   Channel modify alarm settings
     void *link;
     int index;

*
void 
groupChangeMasks_callback(link,index)  Group modify alarm settings
     void *link;
     int index;

******************************************************************
-------------
|  PRIVATE  |
-------------
*

******************************************************************
*/


/***************************************************
  changeMasks_callback
****************************************************/

void changeMasks_callback(area,index)
     ALINK *area;
     int index;
{
     void *link;

     /* Change mask */
     link = area->selectionLink;
     if (link){
          if (area->selectionType == GROUP){
               groupChangeMasks_callback(link,index);
          } else {
               chanChangeMasks_callback(link,index);
          }
     } else {
          createDialog(area->form_main,XmDIALOG_WARNING,
               "Please select an alarm group or channel first."," ");
     }
}

/***************************************************
  chanChangeMasks_callback
****************************************************/

void chanChangeMasks_callback(link,index)
     void *link;
     int index;
{
     int maskid,maskno;


     maskid = index/ 10;
     maskno = index % 10;

     alForceChanMask(link,maskid,maskno);

     /* record on opmod file and text widget */
     alLogChanChangeMasks(link,maskno,maskid);

     /* set beep option on again  */
     resetBeep();

     ((GCLINK *)link)->pmainGroup->modified = 1;

}

/***************************************************
  groupChangeMasks_callback
****************************************************/

void groupChangeMasks_callback(link,index)
     void *link;
     int index;
{
     int maskid,maskno;


     maskid = index/ 10;
     maskno = index % 10;

     alForceGroupMask(link,maskid,maskno);

     /* record on opmod file and text widget */
     alLogChangeGroupMasks(link,maskno,maskid);

     /* set beep option on again  */
     resetBeep();

     ((GCLINK *)link)->pmainGroup->modified = 1;

}
