/*
 $Log$
 Revision 1.5  1998/08/05 18:16:14  jba
 Removed some comment statements.

 Revision 1.4  1995/10/20 16:51:03  jba
 Modified Action menus and Action windows
 Renamed ALARMCOMMAND to SEVRCOMMAND
 Added STATCOMMAND facility
 Added ALIAS facility
 Added ALARMCOUNTFILTER facility
 Make a few bug fixes.

 * Revision 1.3  1995/02/28  16:43:56  jba
 * ansi c changes
 *
 * Revision 1.2  1994/06/22  21:18:05  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)testalarm.c	1.13\t10/1/93";

/* testalarm.c 
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
 * .01  06-12-91        bkc     Change valid severity to < 4
 * .02  10-04-91        bkc     Testalarm & addressInfo are debug options
 * .03  mm-dd-yy        iii     Comment
 *      ...
 */
/*******************************************************
 * testalarm.c: a popup dialog widget 
*
*This file contains the routines for generation test
*alarm dialog.
*
*
Routines defined in testalarm.c:

-------------
|  PUBLIC   |
-------------
void done_dialog(w, dialog, call_data)                          Destroy a dialog widget 
void hide_dialog(w, dialog, call_data)                          Unmanage a dialog box
Widget  createGetTestAlarm_dialog(parent,userData)             	Create the testalarm dialog

-------------
|  PRIVATE  |
-------------
static void  okGetTestAlarmData_callback(w,edit , call_value)   	Ok testalarm getdata callback
static void  okTestAlarmUpdate_callback(w,ind,call_value)       	Ok testalarm update callback
void show_dialog(w,dialog,call_data)
******************************************************/

#include <stdlib.h>
#include <time.h>
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
#include <Xm/CutPaste.h>

#include "sllLib.h"
#include "alLib.h"
#include <axArea.h>
#include <ax.h>

#ifdef __STDC__

static void  okGetTestAlarmData_callback( Widget  w,Widget edit[],
                                XmAnyCallbackStruct *call_value);
static void  okTestAlarmUpdate_callback( Widget  w, int ind,
                                XmAnyCallbackStruct *call_value);

#else

static void  okGetTestAlarmData_callback();
static void  okTestAlarmUpdate_callback();

#endif /*__STDC__*/


extern Widget bb;			/* setup window bb */
extern struct setup psetup;		/* default setup data structure */

extern XmStringCharSet charset;
extern Dimension char_width;
extern Display *display;

Widget label_testalarm[3],edit_testalarm[3];
char  *labels[] = { "Channel Name:", "Status:", "Severity: < 4" };
char  *editors[] =  { "CHANNEL", " ", " " };

char *help_str_generateTestAlarm[] = {
        "This dialog window allows an operator to generate test alarms.",
        " ",
        "The first field should contain an I/O channel name defined",
        "in the alarm configuration file.  The second field should contain",
	"the alarm status and the third field should contain the alarm",
	"severity which should not exceed 4.",
        " ",
	"To copy a channel name from any open window can be obtained by first",
	"positioning the mouse to the desired channel name then clicking the ",
	"middle button on the mouse, then followed by positioning the mouse",
	"to the input channel name field of testalarm window then clicking ",
	"the middle button again.",
	" ",
        "Edit any field and then press the Ok button ",
        "to accept all the information entered.",
        " ",
        "Press the Cancel button to abort test alarm generation.",
        "","" };


/***********************************************************************
	destroy a dialog widget 
***********************************************************************/
void done_dialog(w, dialog, call_data)
Widget  w;
Widget  dialog;
XmAnyCallbackStruct *call_data;
{
        XtUnmanageChild(dialog);
        XtDestroyWidget(dialog); 
	XFlush(display);
}


/***********************************************************************
	unmanage a dialog widget 
***********************************************************************/
void hide_dialog(w, dialog, call_data)
Widget  w;
Widget  dialog;
XmAnyCallbackStruct *call_data;
{
        XtUnmanageChild(dialog);
/*      XtDestroyWidget(dialog); */
}


/***********************************************************************
	create a dialog widget for generating test alarm
***********************************************************************/
Widget createGetTestAlarm_dialog(parent,userData)
Widget parent;
XtPointer userData;
{
Widget bb, ok_button, done_button, help_button;
Arg     wargs[10];
int     i, n=0;
size_t  len=0;
Dimension xloc;
        
        XtSetArg(wargs[n], XmNautoUnmanage, FALSE); n++;
        XtSetArg(wargs[n], XmNallowShellResize, FALSE); n++;
        bb = XmCreateBulletinBoardDialog(parent,"GenerateTestAlarm",wargs,n);


        /* create three single line XmEdit widgets
         * assign a button to each text widget
         * assign a callback to each button
         */

        for (i=0; i<XtNumber(labels); i++) {
        n = 0;
	if (len < strlen(labels[i])) len = strlen(labels[i]);
        XtSetArg(wargs[n], XmNlabelString, 
                XmStringCreateLtoR(labels[i],charset)); n++;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i); n++;

         label_testalarm[i] = XtCreateManagedWidget(labels[i],
                 xmLabelGadgetClass, bb,
                 wargs, n);
         }
/*
 * get string width in pixel
 */

xloc = 10 + len * char_width; 

        for (i=0; i<XtNumber(editors); i++) {
          n = 0;
          XtSetArg(wargs[n], XmNx, 100); n++;
          XtSetArg(wargs[n], XmNy, 10+30*i); n++;
          XtSetArg(wargs[n], XmNwidth, 30 * char_width); n++;

          edit_testalarm[i] = XtCreateManagedWidget(editors[i], 
		xmTextWidgetClass, bb, wargs, n);
          XmTextSetString(edit_testalarm[i],editors[i]);
/*
 *  get channel name text from clipboard 
 */
          if (i == 0)
		XtAddEventHandler(edit_testalarm[i],ButtonPressMask,FALSE,
                	(XtEventHandler)clipboardGet,bb);

        }



        /*
         * add a OK button to accept text and to pop down the widget.
         */

        n = 0;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10+30*3); n++;
        XtSetArg(wargs[n], XmNuserData, userData); n++;

        ok_button = XtCreateManagedWidget(" Ok ", 
                                xmPushButtonGadgetClass,
                                bb, wargs, n);

        XtAddCallback(ok_button, XmNactivateCallback, 
                                (XtCallbackProc)okGetTestAlarmData_callback,
                                 edit_testalarm);


        XtAddCallback(ok_button, XmNactivateCallback, 
                                (XtCallbackProc)hide_dialog, bb);

        XtAddCallback(ok_button, XmNactivateCallback,
                                (XtCallbackProc)okTestAlarmUpdate_callback, 0);
	
        /*
         * add a button to pop down the widget.
         */

        n = 0;
        XtSetArg(wargs[n], XmNx, 100); n++;
        XtSetArg(wargs[n], XmNy, 10+30*3); n++;

        done_button = XtCreateManagedWidget("Cancel", 
                                xmPushButtonGadgetClass,
                                bb, wargs, n);

        XtAddCallback(done_button, XmNactivateCallback, 
                                (XtCallbackProc)hide_dialog, bb);

        /*
         * add a button for asking for help.
         */


        n = 0;
        XtSetArg(wargs[n], XmNx, 200); n++;
        XtSetArg(wargs[n], XmNy, 10+30*3); n++;

        help_button = XtCreateManagedWidget("Help", 
                                xmPushButtonWidgetClass,
                                bb, wargs, n);

        XtAddCallback(help_button, XmNactivateCallback, 
                                (XtCallbackProc)xs_help_callback,
                                help_str_generateTestAlarm);


        return bb;

}




/**********************************************************************
	get data from the the test alarm dialog window
**********************************************************************/
static void okGetTestAlarmData_callback(w,edit , call_value)
Widget  w;
Widget edit[];
XmAnyCallbackStruct *call_value;
{
char * str;
int status, sevr, i;
ALINK *area;
CLINK *clink;
GLINK *proot;
char name[40];

        str = ( char *)XmTextGetString(edit[0]);

	for (i=strlen(str)-1;i>0;i--) {
		if (*(str+i) == ' ') continue;
			*(str+i+1) = '\0';
			 break; 
			}
		
        strcpy(name,str);

        str = ( char *)XmTextGetString(edit[1]);
        status = atoi(str);

        str =( char *)XmTextGetString(edit[2]);
        sevr = atoi(str);
        XtFree(str);


        XtVaGetValues(w, XmNuserData, &area, NULL);

proot = area->pmainGroup->p1stgroup;
clink = alFindChannel((SLIST *)&proot,name);
if (clink) {
if (clink->pchanData->curSevr == sevr &&
	clink->pchanData->curStat == status) {
        printf("---same severity %d & status %d entered.\n",sevr, status);
        }
	else {

	alNewAlarm(status,sevr,clink->pchanData->value,clink);

	}
}
else {
        printf("---Invalid channel name '%s' entered.\n",name);
       }
}



/**********************************************************************
	update the window display if new test alarm was generated
**********************************************************************/
static void okTestAlarmUpdate_callback(w,ind,call_value)
Widget  w;
int ind;
XmAnyCallbackStruct *call_value;
{
ALINK *area;

        XtVaGetValues(w, XmNuserData, &area, NULL);

  /*      awInvokeCallback(); */
  if (area && area->pmainGroup) area->pmainGroup->modified = 1;
 
}


void alCoreInfo_callback(w,ind,call_value)
Widget  w;
int ind;
XmAnyCallbackStruct *call_value;
{
time_t timeofday;
ALINK *area;

	printf("************************************************\n");
        timeofday = time(0L);
        printf("%s\n",ctime(&timeofday));

    XtVaGetValues(w, XmNuserData, &area, NULL);

	if (area) alCaAddressInfo((SLIST *)&(area->pmainGroup->p1stgroup));

} 
/***************************************************
 show dialog 
***************************************************/
void show_dialog(w,dialog,call_data)
Widget          w;
Widget          dialog;
XmAnyCallbackStruct *call_data;
{
        XtManageChild(dialog);
}


