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
#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <Xm/PushBG.h>


#include <alLib.h>
#include <sllLib.h>
#include <line.h>
#include <ax.h>


/*   external routines

extern XmString xs_str_array_to_xmstr();
extern void xs_help_callback();
extern void xs_ok_callback();

*/

/****************************************************
*      guidance.c 
*
*This file contains routines for displaying group or channel guidance.
*
--------------
|   PUBLIC   |
--------------

*
void guidance_callback(widget,line,cbs) Line guidance callback
     Widget widget;
     struct anyLine  *line;
     XmAnyCallbackStruct *cbs;
*
void
GroupGuidance_callback(w,glink,call_data)    	Group guidance callback
     Widget w;
     GLINK *glink;
     XtPointer call_data;
*
void 					Channel guidance callback
ChannelGuidance_callback(w,clink,call_data)
	Widget w;
	CLINK *clink;
	XtPointer call_data;
*

***************************************************************/



/**************************************************
 	group guidance
**************************************************/
void GroupGuidance_callback(w,glink,cbs)
  Widget w;
  GLINK *glink;
  XmAnyCallbackStruct *cbs;
{

  SNODE *pt;
  struct guideLink *guidelist;
  char *group_guidance_str[20];
  int     n;
  Widget  dialog;
  Widget  label;
  XmString xmstr;
  Arg     wargs[5];
  int i=0;
  char buff[120];

  pt = sllFirst(&(glink->GuideList));
  i=0;
  while (pt) {
		guidelist = (struct guideLink *)pt;
		group_guidance_str[i] = guidelist->list;
/*
printf("guidance:%s",guidelist->list);
printf("group_guidance_str[%d] = %s",i,group_guidance_str[i]);
*/
		pt = sllNext(pt);
		 i++;
  }
  group_guidance_str[i] = "";
  group_guidance_str[i+1] = "";

/* 
 * Create the message dialog to display the help.
 */
  n=0;
  strcpy(buff,glink->pgroupData->name);
  strcat(buff,"  Guidance");
  XtSetArg(wargs[n], XmNtitle, buff); n++;
  XtSetArg(wargs[n], XmNautoUnmanage, FALSE); n++;
  XtSetArg(wargs[n], XmNallowShellResize, FALSE); n++;
  dialog = XmCreateMessageDialog(w, "Guidance", wargs, n);
/*
 * We won't use the cancel widget. Unmanage it.
 */
  XtUnmanageChild(XmMessageBoxGetChild(dialog,
                        XmDIALOG_CANCEL_BUTTON));
/*
 * Retrieve the label widget and make the 
 * text left justified
 */

  label = XmMessageBoxGetChild(dialog, 
                        XmDIALOG_MESSAGE_LABEL);

  n = 0;
  XtSetArg(wargs[n],XmNalignment,XmALIGNMENT_BEGINNING); n++;
  XtSetValues(label, wargs, n);
/*
 * Add an OK callback to pop down the dialog.
 */
  XtAddCallback(dialog, XmNokCallback,(XtCallbackProc) xs_ok_callback, NULL);

/*
 * count the test up to the first NUll string.
 */
  for (i=0;group_guidance_str[i][0] != '\0';i++)
                ;
/*
 * convert the string array to an XmString array and set it as the label text.
*/
  xmstr = xs_str_array_to_xmstr(group_guidance_str,i);
  n = 0;
  XtSetArg(wargs[n], XmNmessageString, xmstr); n++;
  XtSetValues(dialog, wargs, n);

/*
 * if the next entryu in the help string array is also NULL,
 * then this is the last message. Unmanage the help button.
 */

  if (group_guidance_str[++i][0] == '\0')
      XtUnmanageChild( XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

/*
 * otherwise add a help callback function with the address of
 * the next entryu in the help string as client_data.
 */

  else {
       XtAddCallback(dialog, XmNhelpCallback,(XtCallbackProc) xs_help_callback, 
		&group_guidance_str[i]);
  }

/*
 * display the dialog.
 */
  XtManageChild(dialog);


}

/*****************************************************
     channel guidance  (limit 20 lines)
*****************************************************/

void ChannelGuidance_callback(w,clink,cbs)
Widget w;
CLINK *clink;
XmAnyCallbackStruct *cbs;
{

SNODE *pt;
struct guideLink *guidelist;
char *chan_guidance_str[20];
int i=0;

        int     n;
        Widget  dialog;
        Widget  label;
        XmString xmstr;
        Arg     wargs[5];


        pt = sllFirst(&(clink->GuideList));
        i=0;
        while (pt) {
                guidelist = (struct guideLink *)pt;
                chan_guidance_str[i] = guidelist->list;
/*
printf("guidance:%s",guidelist->list);
printf("chan_guidance_str[%d] = %s",i,chan_guidance_str[i]);
*/
                pt = sllNext(pt);
                 i++;
                }
                chan_guidance_str[i] = "";
                chan_guidance_str[i+1] = "";

/* 
 * Create the message dialog to display the help.
 */
        n=0;
        XtSetArg(wargs[n], XmNautoUnmanage, FALSE); n++;
        XtSetArg(wargs[n], XmNallowShellResize, FALSE); n++;
        dialog = XmCreateMessageDialog(w, "Help", wargs, n);
/*
 * We won't use the cancel widget. Unmanage it.
 */
        XtUnmanageChild(XmMessageBoxGetChild(dialog,
                        XmDIALOG_CANCEL_BUTTON));
/*
 * Retrieve the label widget and make the 
 * text left justified
 */

        label = XmMessageBoxGetChild(dialog, 
                        XmDIALOG_MESSAGE_LABEL);

        n = 0;
        XtSetArg(wargs[n],XmNalignment,XmALIGNMENT_BEGINNING); n++;
        XtSetValues(label, wargs, n);
/*
 * Add an OK callback to pop down the dialog.
 */
        XtAddCallback(dialog, XmNokCallback,(XtCallbackProc) xs_ok_callback, NULL);

/*
 * count the test up to the first NUll string.
 */
        for (i=0;chan_guidance_str[i][0] != '\0';i++)
                ;
/*
 * convert the string array to an XmString array and set it as the label text.
*/


        xmstr = xs_str_array_to_xmstr(chan_guidance_str,i);

        n = 0;
        XtSetArg(wargs[n], XmNmessageString, xmstr); n++;
        XtSetValues(dialog, wargs, n);

/*
 * if the next entryu in the help string array is also NULL,
 * then this is the last message. Unmanage the help button.
 */

        if (chan_guidance_str[++i][0] == '\0')
                XtUnmanageChild( XmMessageBoxGetChild(dialog,
                        XmDIALOG_HELP_BUTTON));

/*
 * otherwise add a help callback function with the address of
 * the next entryu in the help string as client_data.
 */

        else {
                XtAddCallback(dialog, XmNhelpCallback,
                        (XtCallbackProc)xs_help_callback, &chan_guidance_str[i]);

        }

/*
 * display the dialog.
 */
        XtManageChild(dialog);



}

/***************************************************
  guidance_callback
****************************************************/

void guidance_callback(widget,line,cbs)
     Widget widget;
     struct anyLine  *line;
     XmAnyCallbackStruct *cbs;
{
     if (line->linkType == GROUP){
          GroupGuidance_callback(widget,(GLINK *)line->link,cbs);
     }
     else {
          if (line->linkType == CHANNEL)
               ChannelGuidance_callback(widget,(CLINK *)line->link,cbs);
     }


}

