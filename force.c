/*
 $Log$
 Revision 1.2  1994/06/22 21:17:31  jba
 Added cvs Log keyword

 */

static char *sccsId = "@(#)force.c	1.7\t10/1/93";

/* force.c */
/* 
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
 * .01  07-18-91        bkc     Change forcePVValue & resetPVValue to short 
 *                               
 * .02  mm-dd-yy        iii     Comment
 *      ...
 */
/*******************************************************
 * force.c: create a popup dialog widget 
*
*Routines defined in this file provide the functions of reset 
*or force group/channel masks.  Each dialog box consists a set 
*of label and text widgets, and four push buttons: 'Force', 
*'Reset', 'Cancel', and 'Help'.  All the callbacks for 'Force' 
*and 'Reset' buttons are provided here , except the callbacks 
*for 'Cancel' and 'Help' buttons are defined in the file 'help.c'.
*
*It consists two sets of routines, one corresponding to group
*PV mask change and one corresponding to channel PV mask change.
*
*
*
*
*	LIST OF ROUTINES
*
*
-------------
|   PUBLIC  |
-------------
Widget 					Creat force group mask dialog
createForceGMaskDialog(parent,glink)
	Widget parent;
	GLINK *glink;
*
Widget 					Create group PV dialog
createForcePVGroupDialog(parent,glink)
	Widget parent;
	GLINK *glink;	
	return:  a bulletin board dialog widget

*
Widget 					Create channel PV dialog
createForcePVChanDialog(parent,clink)
	Widget parent;
	CLINK *clink;
	return:  a bulletin board dialog widget
*
Widget 					Creat force chann mask dialog
createForceCMaskDialog(parent,glink)
	Widget parent;
	GLINK *glink;
*
-------------
|  PRIVATE  |
-------------
*
void 					Force group mask callback
acceptForcePVGroupData_callback(w,glink, call_value)
	Widget  w;
	GLINK *glink;
	XmAnyCallbackStruct *call_value;
*
void 					Force group PV callback
okForcePVGroupData_callback(w,glink, call_value)
	Widget  w;
	GLINK *glink;
	XmAnyCallbackStruct *call_value;

*
void 					Display update callback(Force/Reset)
okForcePVUpdate_callback(w,ind,call_value)		     
	Widget  w;
	GCLINK *clink;
	XmAnyCallbackStruct *call_value;

*
void 					Reset group PV callback
okResetPVGroupData_callback(w,glink, call_value)
	Widget  w;
	GLINK *glink;
	XmAnyCallbackStruct *call_value;

*
void 					Force channel PV callback
acceptForcePVChanData_callback(w,clink, call_value)
	Widget  w;
	CLINK *clink;
	XEvent *call_value;

*
void 					Reset channel PV callback
okResetPVChanData_callback(w,clink, call_value)
	Widget  w;
	CLINK *clink;
	XEvent *call_value;
*
void
alOperatorForcePVChanEvent(clink,pvMask)
	CLINK *clink;
	MASK pvMask;
*
void
alOperatorForcePVGroupEvent(glink,mask)  
	GLINK *glink;
	MASK mask;
*
void
alOperatorResetPVGroupEvent(glink)  
	GLINK *glink;
*
void
alOperatorResetPVGroupEvent(glink)  
	GLINK *glink;

*************************************************************
*/

#include <stdio.h>

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Text.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/BulletinB.h>

#include <sllLib.h>
#include <alLib.h>
#include <ax.h>

#define OPERATOR 	1

extern struct setup psetup;
extern Dimension char_width;

extern XmStringCharSet charset;

#ifdef __STDC__

static void acceptForcePVGroupData_callback( Widget w, GLINK *glink,
      XmAnyCallbackStruct *call_value);
static void okForcePVGroupData_callback( Widget w, GLINK *glink, XmAnyCallbackStruct *call_value);
static void okForcePVUpdate_callback( Widget w, GCLINK *gclink, XmAnyCallbackStruct *call_value);
static void okResetPVGroupData_callback( Widget w, GLINK *glink, XmAnyCallbackStruct *call_value);
static void acceptForcePVChanData_callback( Widget w, CLINK *clink, XEvent *call_value);
static void okForcePVChanData_callback( Widget w, CLINK *clink, XEvent *call_value);
static void okResetPVChanData_callback( Widget w, CLINK *clink, XEvent *call_value);

#else

static void acceptForcePVGroupData_callback();
static void okForcePVGroupData_callback();
static void okForcePVUpdate_callback();
static void okResetPVGroupData_callback();
static void acceptForcePVChanData_callback();
static void okForcePVChanData_callback();
static void okResetPVChanData_callback();

#endif /*__STDC__*/




char  editors_forcePVG[5][36];
Widget label_forcePVG[5],edit_forcePVG[5];
char  *labels_forcePVG[] = { 
	"Group Name",
	"Force Process Variable : name",
	"Force Process Variable : force value",
	"Force Process Variable : reset value",
	"Force Process Variable : force mask"
	};
	
char  editors_forceGMask[2][36];
Widget label_forceGMask[2],edit_forceGMask[2];
char *labels_forceGMask[] = {
	"Group Name ",
	"Mask <CDATL>" };




char *help_str_forcePVG[] = {
        "This dialog window allows an operator to specify force and reset",
	"mask variables for a given group.",
        " ",
	"The PV force value must be set to a value different from",
	"the PV reset value.",
        " ",
	"Press the Accept button to accept the change.",
	"Press the Cancel button to abort current change.",
	"Press the Help   button to get this help description.",
	"","" };

char *help_str_forceGMask[] = {
        "This dialog window allows an operator to set the current",
	"mask for the whole group.",
	" ",
	"Press the Force  button to force the group channel masks.",
	"Press the Reset  button to reset group channel masks to default.",
        "Press the Cancel button to abort this dialog .",
	"Press the Help   button to get this help description.",
        "","" };

char  editors_forcePVC[6][36];
Widget label_forcePVC[6],edit_forcePVC[6];
char  *labels_forcePVC[] = { 
	"Channel Name",
	"Force Process Variable : name",
	"Force Process Variable : force value",
	"Force Process Variable : reset value",
	"Force Process Variable : force mask",
	"Force Process Variable : reset mask"
	};
	
char  editors_forceCMask[3][36];
Widget label_forceCMask[3],edit_forceCMask[3];
char *labels_forceCMask[] = {
	"Channel Name ",
	"Current Mask ",
	"Mask <CDATL>" };




char *help_str_forcePVC[] = {
        "This dialog window allows an operator to specify force and reset",
	"mask variables for a given channel.",
        " ",
	"The PV force value must be set to a value different from",
	"the PV reset value.",
        " ",
	"Press the Accept button to accept the change.",
	"Press the Cancel button to abort current change.",
	"Press the Help   button to get this help description.",
	"","" };

char *help_str_forceCMask[] = {
        "This dialog window allows an operator to set the current",
	"mask for the considered channel.",
	" ",
	"Press the Force  button to force the channel masks.",
	"Press the Reset  button to reset channel masks to default.",
        "Press the Cancel button to abort this dialog .",
	"Press the Help   button to get this help description.",
        "","" };


/************************************************************************
	create force PV dialog box
************************************************************************/
Widget createForcePVGroupDialog(parent,glink)
Widget parent;
GLINK *glink;
{
Widget bb, done_button, ok_button,  help_button;
Arg     wargs[10];
int     i, n=0,len=0,xloc,yloc;
struct groupData *gdata;

	gdata = glink->pgroupData;
	
	strcpy(editors_forcePVG[0],gdata->name);
	strcpy(editors_forcePVG[1],gdata->forcePVName);
	sprintf(editors_forcePVG[2],"%d",gdata->forcePVValue);
	sprintf(editors_forcePVG[3],"%d",gdata->resetPVValue);
  	alGetMaskString(gdata->forcePVMask,editors_forcePVG[4]);
	n =0;
        XtSetArg(wargs[n], XmNautoUnmanage, FALSE); n++;
        XtSetArg(wargs[n], XmNdialogTitle,
		XmStringLtoRCreate("Force Process Vaiable",
		XmSTRING_DEFAULT_CHARSET)); n++;
        XtSetArg(wargs[n], XmNallowShellResize, FALSE); n++;
        bb = XmCreateBulletinBoardDialog(parent, 
			"PVG", wargs, n);


        for (i=0; i<XtNumber(labels_forcePVG); i++) {
        n = 0;
	if (len < strlen(labels_forcePVG[i]))
		len = strlen(labels_forcePVG[i]);
        XtSetArg(wargs[n], XmNlabelString, 
                XmStringCreateLtoR(labels_forcePVG[i],charset)); n++;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;

         label_forcePVG[i] = XtCreateManagedWidget(labels_forcePVG[i],
                 xmLabelGadgetClass, bb,
                 wargs, n);
         }

        i++;
        yloc = 10+30*i;


/*
 * get string width in pixel
 */
  xloc = 10 + len * char_width;

        for (i=0; i<XtNumber(editors_forcePVG); i++) {
        n = 0;
        XtSetArg(wargs[n], XmNx, xloc + 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
        XtSetArg(wargs[n], XmNwidth, char_width * 30); n++;
	if (i < 1) { 
                XtSetArg(wargs[n],XmNcursorPositionVisible,FALSE); n++;
		XtSetArg(wargs[n],XmNeditable,FALSE); n++;
		}

        edit_forcePVG[i] = XtCreateManagedWidget(editors_forcePVG[i], 
			xmTextWidgetClass, bb, wargs, n);
        XmTextSetString(edit_forcePVG[i],editors_forcePVG[i]);
        }

        /*
         * add a Accept  button to accept text and to pop down the widget.
 	 */
       
        n = 0;
        xloc = 10;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        ok_button = XtCreateManagedWidget("Accept", xmPushButtonGadgetClass,
                                bb, wargs, n);
        XtAddCallback(ok_button, XmNactivateCallback, 
                                (XtCallbackProc)acceptForcePVGroupData_callback, glink);
        XtAddCallback(ok_button, XmNactivateCallback, 
                                (XtCallbackProc)done_dialog, bb);
/*
        XtAddCallback(ok_button, XmNactivateCallback,
                                (XtCallbackProc)okForcePVUpdate_callback, glink);
 */


        /*
         * add a button to pop down the widget.
         */

        n = 0;
        xloc = xloc + 10 * char_width ;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        done_button = XtCreateManagedWidget("Cancel", 
                                xmPushButtonGadgetClass, bb, wargs, n);
        XtAddCallback(done_button, XmNactivateCallback,
                                (XtCallbackProc)done_dialog, bb);

        /*
         * add a button for asking for help.
         */


        n = 0;
        xloc = xloc + 10 * char_width ;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        help_button = XtCreateManagedWidget(" Help ", 
                                xmPushButtonWidgetClass, bb, wargs, n);

        XtAddCallback(help_button, XmNactivateCallback, 
                                (XtCallbackProc)xs_help_callback,
                                 help_str_forcePVG);


        return bb;

}

/************************************************************************
	create force PV dialog box
************************************************************************/
Widget createForceGMaskDialog(parent,glink)
Widget parent;
GLINK *glink;
{
Widget bb, done_button, ok_button, reset_button, help_button;
Widget label;
Arg     wargs[10];
int     i, n=0,len=0,xloc,yloc;
struct groupData *gdata;

	gdata = glink->pgroupData;
	
	strcpy(editors_forceGMask[0],gdata->name);
  	alGetMaskString(gdata->forcePVMask,editors_forceGMask[1]);
	n =0;
        XtSetArg(wargs[n], XmNautoUnmanage, FALSE); n++;
        XtSetArg(wargs[n], XmNdialogTitle,
		XmStringLtoRCreate("Force Mask",
		XmSTRING_DEFAULT_CHARSET)); n++;
        XtSetArg(wargs[n], XmNallowShellResize, FALSE); n++;
        bb = XmCreateBulletinBoardDialog(parent, 
			"FG", wargs, n);


        for (i=0; i<XtNumber(labels_forceGMask); i++) {
        n = 0;
        if (len < strlen(labels_forceGMask[i]))
                len = strlen(labels_forceGMask[i]);
        XtSetArg(wargs[n], XmNlabelString,
                XmStringCreateLtoR(labels_forceGMask[i],charset)); n++;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
        label_forceGMask[i] = XtCreateManagedWidget(labels_forceGMask[i],
                 xmLabelGadgetClass, bb,
                 wargs, n);
        }

	n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
         label = XtCreateManagedWidget("C (Cancel)",
                 xmLabelGadgetClass, bb,
                 wargs, n);
	i++;
	n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
         label = XtCreateManagedWidget("D (Disable)",
                 xmLabelGadgetClass, bb,
                 wargs, n);
	i++;
	n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
         label = XtCreateManagedWidget("A (NoAck)",
                 xmLabelGadgetClass, bb,
                 wargs, n);
	i++;
	n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
         label = XtCreateManagedWidget("T (NoAck Transient)",
                 xmLabelGadgetClass, bb,
                 wargs, n);
	i++;
	n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
         label = XtCreateManagedWidget("L (NoLog)",
                 xmLabelGadgetClass, bb,
                 wargs, n);

	i++;
        yloc = 10+30*i;

/*
 * get string width in pixel
 */
  xloc = 10 + len * char_width;

        for (i=0; i<XtNumber(editors_forceGMask); i++) {
        n = 0;
        XtSetArg(wargs[n], XmNx, xloc + 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
        XtSetArg(wargs[n], XmNwidth, char_width * 30); n++;
	if (i < 1) { 
                XtSetArg(wargs[n],XmNcursorPositionVisible,FALSE); n++;
		XtSetArg(wargs[n],XmNeditable,FALSE); n++;
		}

        edit_forceGMask[i] = XtCreateManagedWidget(editors_forceGMask[i], 
			xmTextWidgetClass, bb, wargs, n);
        XmTextSetString(edit_forceGMask[i],editors_forceGMask[i]);
        }

        /*
         * add a FORCE  button to accept text and to pop down the widget.
	 */
        
        n = 0;
        xloc = 10;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        ok_button = XtCreateManagedWidget("Force ", xmPushButtonGadgetClass,
                                bb, wargs, n);
        XtAddCallback(ok_button, XmNactivateCallback, 
                                (XtCallbackProc)okForcePVGroupData_callback, glink);
        XtAddCallback(ok_button, XmNactivateCallback, 
                                (XtCallbackProc)done_dialog, bb);
        XtAddCallback(ok_button, XmNactivateCallback,
                                (XtCallbackProc)okForcePVUpdate_callback, glink);


        /*
         * add a reset button to accept text and to pop down the widget.
         */

        n = 0;
        xloc = xloc + 10 * char_width;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        reset_button = XtCreateManagedWidget("Reset ", 
                                xmPushButtonGadgetClass, bb, wargs, n);
        XtAddCallback(reset_button, XmNactivateCallback, 
                                (XtCallbackProc)okResetPVGroupData_callback, glink);
        XtAddCallback(reset_button, XmNactivateCallback,
                                (XtCallbackProc)done_dialog, bb);
        XtAddCallback(reset_button, XmNactivateCallback, 
				(XtCallbackProc)okForcePVUpdate_callback, glink);
	


        /*
         * add a button to pop down the widget.
         */

        n = 0;
        xloc = xloc + 10 * char_width;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        done_button = XtCreateManagedWidget("Cancel", 
                                xmPushButtonGadgetClass, bb, wargs, n);
        XtAddCallback(done_button, XmNactivateCallback,
                                  (XtCallbackProc)done_dialog, bb);

        /*
         * add a button for asking for help.
         */


        n = 0;
        xloc = xloc + 10 * char_width;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        help_button = XtCreateManagedWidget(" Help  ", 
                                xmPushButtonWidgetClass, bb, wargs, n);

        XtAddCallback(help_button, XmNactivateCallback, 
                                (XtCallbackProc)xs_help_callback,
                                help_str_forceGMask);


        return bb;

}


/************************************************************************
	accept  data from  force PV dialog box
************************************************************************/
static void acceptForcePVGroupData_callback(w,glink, call_value)
Widget  w;
GLINK *glink;
XmAnyCallbackStruct *call_value;
{
short value;
char * str,buff1[6];
MASK mask;
struct groupData *gdata;

	gdata = glink->pgroupData;

        str = ( char *)XmTextGetString(edit_forcePVG[2]);
        value = atoi(str);
	gdata->forcePVValue = value;

        str = ( char *)XmTextGetString(edit_forcePVG[3]);
        value = atoi(str);
	gdata->resetPVValue = value;

        str =( char *)XmTextGetString(edit_forcePVG[4]);
	strcpy(buff1,str);
	alSetMask(buff1,&mask);
	gdata->forcePVMask = mask;

/* New forcePVName replace CA search and event */

        str = ( char *)XmTextGetString(edit_forcePVG[1]);
	if (strlen(str) > 1 && strcmp(str,gdata->forcePVName) != 0) {
		alReplaceGroupForceEvent(glink,str);
		}
        XtFree(str);


	alProcessCA();

/*  log on operation file */

	alLogForcePVGroup(glink,OPERATOR);

}





/************************************************************************
	Get data from  force PV dialog box
************************************************************************/
static void okForcePVGroupData_callback(w,glink, call_value)
Widget  w;
GLINK *glink;
XmAnyCallbackStruct *call_value;
{
char * str,buff1[6];
MASK mask;
struct groupData *gdata;


	gdata = glink->pgroupData;

        str =( char *)XmTextGetString(edit_forceGMask[1]);
	strcpy(buff1,str);
	alSetMask(buff1,&mask);

/*	gdata->forcePVMask = mask; */

        XtFree(str);

	alOperatorForcePVGroupEvent(glink,mask);

	alProcessCA();

/*  log on operation file */

	alLogForcePVGroup(glink,OPERATOR);

}



/************************************************************************
	Update display window after force PV dialog
************************************************************************/
static void okForcePVUpdate_callback(w,gclink,call_value)
Widget  w;
GCLINK *gclink;
XmAnyCallbackStruct *call_value;
{
/*
 * if alarm change states due to force  reset silenceCurrent button
if (psetup.nobeep == FALSE && psetup.beep == FALSE) {
	XmToggleButtonGadgetSetState(((ALINK *)gclink->pmainGroup->area)->silenceCurrent,FALSE,FALSE);
	psetup.beep = TRUE;
	}
 */
	resetBeep();

/*      awInvokeCallback();  */
	gclink->pmainGroup->modified = 1;

 }


/************************************************************************
	reset group mask callback for PV dialog
************************************************************************/
static void okResetPVGroupData_callback(w,glink, call_value)
Widget  w;
GLINK *glink;
XmAnyCallbackStruct *call_value;
{
char * str,buff1[6];
MASK mask;
struct groupData *gdata;


	gdata = glink->pgroupData;


        str =( char *)XmTextGetString(edit_forceGMask[1]);
	strcpy(buff1,str);
	alSetMask(buff1,&mask);

/*	gdata->resetPVMask = mask; */

        XtFree(str);

	alOperatorResetPVGroupEvent(glink);

	alProcessCA();

/*
log reset PV group masks on operation log file
*/
	alLogResetPVGroup(glink,OPERATOR);

}
 




/************************************************************************
	create force channel PV dialog
************************************************************************/
Widget createForcePVChanDialog(parent,clink)
Widget parent;
CLINK *clink;
{
Widget bb, done_button, ok_button,  help_button;
Arg     wargs[10];
int     i, n=0,len=0,xloc,yloc;
struct chanData *cdata;

	cdata = clink->pchanData;
	
	strcpy(editors_forcePVC[0],cdata->name);
	strcpy(editors_forcePVC[1],cdata->forcePVName);
	sprintf(editors_forcePVC[2],"%d",cdata->forcePVValue);
	sprintf(editors_forcePVC[3],"%d",cdata->resetPVValue);
 	alGetMaskString(cdata->forcePVMask,editors_forcePVC[4]);
 	alGetMaskString(cdata->defaultMask,editors_forcePVC[5]);


	n =0;
        XtSetArg(wargs[n], XmNautoUnmanage, FALSE); n++;
        XtSetArg(wargs[n], XmNdialogTitle,
		XmStringLtoRCreate("Force Process Variable",
		XmSTRING_DEFAULT_CHARSET)); n++;
        XtSetArg(wargs[n], XmNallowShellResize, FALSE); n++;
        bb = XmCreateBulletinBoardDialog(parent,"PVC", wargs, n);


        for (i=0; i<XtNumber(labels_forcePVC); i++) {
        n = 0;
	if (len < strlen(labels_forcePVC[i]))
		len = strlen(labels_forcePVC[i]);
        XtSetArg(wargs[n], XmNlabelString, 
                XmStringCreateLtoR(labels_forcePVC[i],charset)); n++;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;

         label_forcePVC[i] = XtCreateManagedWidget(labels_forcePVC[i],
                 xmLabelGadgetClass, bb,
                 wargs, n);
         }

        i++;
        yloc = 10+30*i;


/*
 * get string width in pixel
 */
  xloc = 10 + len * char_width;

        for (i=0; i<XtNumber(editors_forcePVC); i++) {
        n = 0;
        XtSetArg(wargs[n], XmNx, xloc + 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
        XtSetArg(wargs[n], XmNwidth, char_width * 30); n++;
	if ( i < 1) {
            XtSetArg(wargs[n],XmNcursorPositionVisible,FALSE); n++;
	    XtSetArg(wargs[n], XmNeditable, FALSE); n++;
	}

        edit_forcePVC[i] = XtCreateManagedWidget(editors_forcePVC[i], 
			xmTextWidgetClass, bb, wargs, n);
        XmTextSetString(edit_forcePVC[i],editors_forcePVC[i]);
        }

        /*
         * add a Accept  button to accept text and to pop down the widget.
         */

        n = 0;
        xloc = 10;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        ok_button = XtCreateManagedWidget("Accept", 
			xmPushButtonGadgetClass, bb, wargs, n);
        XtAddCallback(ok_button, XmNactivateCallback, 
			(XtCallbackProc)acceptForcePVChanData_callback, clink);
        XtAddCallback(ok_button, XmNactivateCallback,
                                (XtCallbackProc)done_dialog, bb);
/*
        XtAddCallback(ok_button, XmNactivateCallback, 
			okForcePVUpdate_callback, clink);
 */	


        /*
         * add a button to pop down the widget.
         */

        n = 0;
        xloc = xloc + 10 * char_width;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        done_button = XtCreateManagedWidget("Cancel", 
                                xmPushButtonGadgetClass, bb, wargs, n);
        XtAddCallback(done_button, XmNactivateCallback,
                                (XtCallbackProc)done_dialog, bb);

        /*
         * add a button for asking for help.
         */

        n = 0;
        xloc = xloc + 10 * char_width;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        help_button = XtCreateManagedWidget(" Help ", 
			xmPushButtonWidgetClass, bb, wargs, n);
        XtAddCallback(help_button, XmNactivateCallback, 
			(XtCallbackProc)xs_help_callback, help_str_forcePVC);


        return bb;



}



/************************************************************************
	create force PV dialog box
************************************************************************/
Widget createForceCMaskDialog(parent,clink)
Widget parent;
CLINK *clink;
{
Widget bb, done_button, ok_button, reset_button, help_button;
Widget label;
Arg     wargs[10];
int     i, n=0,len=0,xloc,yloc;
struct chanData *cdata;

	cdata = clink->pchanData;
	
	strcpy(editors_forceCMask[0],cdata->name);
  	alGetMaskString(cdata->curMask,editors_forceCMask[1]);
  	alGetMaskString(cdata->forcePVMask,editors_forceCMask[2]);
	n =0;
        XtSetArg(wargs[n], XmNautoUnmanage, FALSE); n++;
        XtSetArg(wargs[n], XmNdialogTitle,
		XmStringLtoRCreate("Force Mask",
		XmSTRING_DEFAULT_CHARSET)); n++;
        XtSetArg(wargs[n], XmNallowShellResize, FALSE); n++;
        bb = XmCreateBulletinBoardDialog(parent, 
			"FC", wargs, n);


        for (i=0; i<XtNumber(labels_forceCMask); i++) {
        n = 0;
	if (len < strlen(labels_forceCMask[i]))
		len = strlen(labels_forceCMask[i]);
        XtSetArg(wargs[n], XmNlabelString, 
                XmStringCreateLtoR(labels_forceCMask[i],charset)); n++;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;

         label_forceCMask[i] = XtCreateManagedWidget(labels_forceCMask[i],
                 xmLabelGadgetClass, bb,
                 wargs, n);
         }

	n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
         label = XtCreateManagedWidget("C (Cancel)",
                 xmLabelGadgetClass, bb,
                 wargs, n);
	i++;
	n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
         label = XtCreateManagedWidget("D (Disable)",
                 xmLabelGadgetClass, bb,
                 wargs, n);
	i++;
	n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
         label = XtCreateManagedWidget("A (NoAck)",
                 xmLabelGadgetClass, bb,
                 wargs, n);
	i++;
	n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
         label = XtCreateManagedWidget("T (NoAck Transient)",
                 xmLabelGadgetClass, bb,
                 wargs, n);
	i++;
	n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
         label = XtCreateManagedWidget("L (NoLog)",
                 xmLabelGadgetClass, bb,
                 wargs, n);

        i++;
        yloc = 10+30*i;

/*
 * get string width in pixel
 */
  xloc = 10 + len * char_width;

        for (i=0; i<XtNumber(editors_forceCMask); i++) {
        n = 0;
        XtSetArg(wargs[n], XmNx, xloc + 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;
        XtSetArg(wargs[n], XmNwidth, char_width * 30); n++;
	if (i < 2) { 
                XtSetArg(wargs[n],XmNcursorPositionVisible,FALSE); n++;
		XtSetArg(wargs[n],XmNeditable,FALSE); n++;
		}

        edit_forceCMask[i] = XtCreateManagedWidget(editors_forceCMask[i], 
			xmTextWidgetClass, bb, wargs, n);
        XmTextSetString(edit_forceCMask[i],editors_forceCMask[i]);
        }

        /*
         * add a FORCE  button to accept text and to pop down the widget.
	 */
        
        n = 0;
        xloc = 10;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        ok_button = XtCreateManagedWidget("Force ", xmPushButtonGadgetClass,
                                bb, wargs, n);
        XtAddCallback(ok_button, XmNactivateCallback, 
                                (XtCallbackProc)okForcePVChanData_callback, clink);
        XtAddCallback(ok_button, XmNactivateCallback, 
                                (XtCallbackProc)done_dialog, bb);
        XtAddCallback(ok_button, XmNactivateCallback,
                                (XtCallbackProc)okForcePVUpdate_callback, clink);


        /*
         * add a reset button to accept text and to pop down the widget.
         */

        n = 0;
        xloc = xloc + 10 * char_width;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        reset_button = XtCreateManagedWidget("Reset ", 
                                xmPushButtonGadgetClass, bb, wargs, n);
        XtAddCallback(reset_button, XmNactivateCallback, 
                                (XtCallbackProc)okResetPVChanData_callback, clink);
        XtAddCallback(reset_button, XmNactivateCallback, (XtCallbackProc)done_dialog, bb);
        XtAddCallback(reset_button, XmNactivateCallback, 
				(XtCallbackProc)okForcePVUpdate_callback, clink);
	


        /*
         * add a button to pop down the widget.
         */

        n = 0;
        xloc = xloc + 10 * char_width;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        done_button = XtCreateManagedWidget("Cancel", 
                                xmPushButtonGadgetClass, bb, wargs, n);
        XtAddCallback(done_button, XmNactivateCallback, 
                                (XtCallbackProc)done_dialog, bb);

        /*
         * add a button for asking for help.
         */


        n = 0;
        xloc = xloc + 10 * char_width;
        XtSetArg(wargs[n], XmNx, xloc); n++;
        XtSetArg(wargs[n], XmNy, yloc); n++;
        help_button = XtCreateManagedWidget(" Help ", 
                                xmPushButtonWidgetClass, bb, wargs, n);

        XtAddCallback(help_button, XmNactivateCallback, 
                                (XtCallbackProc)xs_help_callback,
                                help_str_forceCMask);


        return bb;

}




/************************************************************************
	getdata from the  force channel PV dialog
************************************************************************/
static void acceptForcePVChanData_callback(w,clink, call_value)
Widget  w;
CLINK *clink;
XEvent *call_value;
{
short value;
char * str,buff1[6];
MASK mask;
struct chanData *cdata;

	cdata = clink->pchanData;
 
        str = ( char *)XmTextGetString(edit_forcePVC[2]);
        value = atoi(str);
	cdata->forcePVValue = value;

        str = ( char *)XmTextGetString(edit_forcePVC[3]);
        value = atoi(str);
	cdata->resetPVValue = value;

        str =( char *)XmTextGetString(edit_forcePVC[4]);
	strcpy(buff1,str);
	alSetMask(buff1,&mask);
	cdata->forcePVMask = mask;

        str =( char *)XmTextGetString(edit_forcePVC[5]);
	strcpy(buff1,str);
	alSetMask(buff1,&mask);
	cdata->defaultMask = mask;

/* New forcePVName replace CA search and event */

        str = ( char *)XmTextGetString(edit_forcePVC[1]);
	if (strlen(str) > 1 && strcmp(str,cdata->forcePVName) != 0) {
		alReplaceChanForceEvent(clink,str);
		}

        XtFree(str);

/*	alOperatorForcePVChanEvent(clink,cdata->forcePVMask); */

	alProcessCA();

/*  log force PV chan on operation file */

	alLogForcePVChan(clink,OPERATOR);
}


/************************************************************************
	 force channel PV mask
************************************************************************/
static void okForcePVChanData_callback(w,clink, call_value)
Widget  w;
CLINK *clink;
XEvent *call_value;
{
char * str,buff1[6];
MASK mask;
struct chanData *cdata;

	cdata = clink->pchanData;

        str =( char *)XmTextGetString(edit_forceCMask[2]);
	strcpy(buff1,str);
	alSetMask(buff1,&mask);

/*	cdata->forcePVMask = mask; */
 
	XtFree(str);

	alOperatorForcePVChanEvent(clink,mask);

	alProcessCA();

/* log reset PV chan on operation file */

	alLogResetPVChan(clink,OPERATOR);

}
 




/************************************************************************
	get reset data from the  force channel PV dialog
************************************************************************/
static void okResetPVChanData_callback(w,clink, call_value)
Widget  w;
CLINK *clink;
XEvent *call_value;
{
struct chanData *cdata;

	cdata = clink->pchanData;

/*
        str =( char *)XmTextGetString(edit_forceCMask[2]);
	strcpy(buff1,str);
	alSetMask(buff1,&mask);
	cdata->forcePVMask = mask; 
	XtFree(str);
 */

	alOperatorForcePVChanEvent(clink,cdata->defaultMask);

	alProcessCA();

/* log reset PV chan on operation file */

	alLogResetPVChan(clink,OPERATOR);

}
 



/*********************************************************************** *
 * This function forces / resets channel mask and updates all 
 * the parent groups mask values.
 ***********************************************************************/
void alOperatorForcePVChanEvent(clink,pvMask)
CLINK *clink;
MASK pvMask;
{
struct chanData *cdata;
char s1[6],s2[6];
 
        cdata = clink->pchanData;

	alGetMaskString(cdata->curMask,s1);
	alGetMaskString(pvMask,s2);
/* 
 * if value = cdata->forcePVValue  force channel mask
 */

        if (strcmp(s1,s2) != 0)  {
                
                /*alOrMask(&cdata->curMask,&mask);
                */

                alChangeChanMask(clink,pvMask);

                }
}


/*********************************************************************** 
 * This function   forces the masks of  all the channels in
 * the glink.  This function will loop through all the channels belonging 
 * to this group and update all the group mask according to input mask.
 ***********************************************************************/
void alOperatorForcePVGroupEvent(glink,mask)  
GLINK *glink;
MASK mask;
{
struct groupData *gdata;
GLINK *subgroup;
SNODE *pt;

        if (glink == NULL) return;

        gdata = glink->pgroupData;

        pt = sllFirst(&glink->chanList);

/*
 * for each channel force to group mask
 */
 
              alChangeGroupMask(glink,mask);



/*
 * for each subgroup in this group
 */

        pt = sllFirst(&glink->subGroupList);

        while (pt) {                /* force all the subgroup mask */
                subgroup = (GLINK *)pt;
                alOperatorForcePVGroupEvent(subgroup,mask);
                pt = sllNext(pt);
                }


}


/*********************************************************************** 
 * This function   resets the masks of  all the channels in
 * the glink.  This function will loop through all the channels belonging 
 * to this group and update all the group mask according to input mask.
 ***********************************************************************/
void alOperatorResetPVGroupEvent(glink)  
GLINK *glink;
{
struct groupData *gdata;
GLINK *subgroup;
CLINK *clink;
SNODE *pt;
MASK mask;
 
        if (glink == NULL) return;

        gdata = glink->pgroupData;

        pt = sllFirst(&glink->chanList);

/*
 * for each channel reset to default mask
 */
	while (pt) {
		clink = (CLINK *)pt;
		mask = clink->pchanData->defaultMask;
		alChangeChanMask(clink,mask);
		pt = sllNext(pt);
		}
               


/*
 * for each subgroup in this group
 */

        pt = sllFirst(&glink->subGroupList);

        while (pt) {                /* force all the subgroup mask */
                subgroup = (GLINK *)pt;
                alOperatorResetPVGroupEvent(subgroup);
                pt = sllNext(pt);
                }


}
