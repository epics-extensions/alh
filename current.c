/* current.c */

/************************DESCRIPTION***********************************
  Routines for viewing the current alarm history window
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/DialogS.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/CascadeB.h>
#include <Xm/LabelG.h>
#include <Xm/Protocols.h>
#include <Xm/Text.h>

#include "alh.h"
#include "axArea.h"
#include "cadef.h"
#include "ax.h"

struct currentData {
	time_t timeofday;
	char *name;
	int   sevr;
	int   stat;
	char  value[MAX_STRING_SIZE];
};

/* global variables */
extern char * alarmSeverityString[];
extern char * alarmStatusString[];

struct currentData currentAlarm[10];
int currentAlarmIndex;

/* forward declarations */
void closeCurrent_callback( Widget w, Widget currentForm, caddr_t call_data);


/**************************************************************************
	create scroll window for file view
**************************************************************************/
void currentAlarmHistoryWindow(ALINK *area,Widget menuButton)
{
	static Widget popup_shell,title,button;
	Widget previous;
	Atom         WM_DELETE_WINDOW;

	int i;

	if (!popup_shell) {

		popup_shell = XtAppCreateShell(
		    "Alarm Handler: Current Alarm History",
		    programName,
		    applicationShellWidgetClass, display, NULL, 0);

		/*  create current alarm view window */
		/*
		          popup_shell = XtVaCreatePopupShell(
		               "Alarm Handler: Current Alarm History",
		               xmDialogShellWidgetClass,   area->runtimeForm,
		               XmNautoUnmanage,            FALSE,
		               NULL);
		*/

		/*  create current alarm form window */
		area->currentAlarmForm = XtVaCreateManagedWidget("CurrentAlarm",
		    xmFormWidgetClass,         popup_shell,
		    XmNallowOverlap,           FALSE,
		    XmNuserData,               menuButton,
		    XmNnoResize,               TRUE,
		    NULL);

		/* Modify the window manager menu "close" callback */
		XtVaSetValues(popup_shell,
		    XmNdeleteResponse,       XmDO_NOTHING,
		    NULL);
		WM_DELETE_WINDOW = XmInternAtom(XtDisplay(popup_shell),
		    "WM_DELETE_WINDOW", False);
		XmAddWMProtocolCallback(popup_shell,WM_DELETE_WINDOW,
		    (XtCallbackProc)closeCurrent_callback,
		    (XtPointer)area->currentAlarmForm);

		/* add close button */
		button = XtVaCreateManagedWidget("Close",
		    xmPushButtonWidgetClass, area->currentAlarmForm,
		    XmNtopAttachment,          XmATTACH_FORM,
		    XmNtopOffset,              5,
		    XmNleftAttachment,         XmATTACH_FORM,
		    XmNleftOffset,             5,
		    XmNuserData,               menuButton,
		    NULL);
		XtAddCallback(button, XmNactivateCallback,
		    (XtCallbackProc)closeCurrent_callback, area->currentAlarmForm);

		previous = button;

		/* add title line */
		title = XtVaCreateManagedWidget(
		    "    TIME_STAMP        PROCESS_VARIABLE_NAME        STATUS     SEVERITY   VALUE       ",
		    xmLabelGadgetClass,        area->currentAlarmForm,
		    XmNtopAttachment,          XmATTACH_FORM,
		    XmNtopOffset,              10,
		    XmNleftAttachment,         XmATTACH_WIDGET,
		    XmNleftWidget,             button,
		    XmNrightAttachment,        XmATTACH_FORM,
		    NULL);

		/* create 10 label widgets  */
		for ( i=0;i<10;i++){
			area->currentAlarm[i] = XtVaCreateManagedWidget( "-----",
			    xmLabelGadgetClass,        area->currentAlarmForm,
			    XmNmarginHeight,           1,
			    XmNalignment,              XmALIGNMENT_BEGINNING,
			    XmNtopAttachment,          XmATTACH_WIDGET,
			    XmNtopWidget,              previous,
			    XmNleftAttachment,         XmATTACH_FORM,
			    XmNrightAttachment,        XmATTACH_FORM,
			    NULL);
			previous = area->currentAlarm[i];
		}

		/* create a blank line, because Form marginHeight wont work  */
		(void)XtVaCreateManagedWidget( " ",
		    xmLabelGadgetClass,        area->currentAlarmForm,
		    XmNalignment,              XmALIGNMENT_BEGINNING,
		    XmNtopAttachment,          XmATTACH_WIDGET,
		    XmNtopWidget,              previous,
		    XmNleftAttachment,         XmATTACH_FORM,
		    XmNrightAttachment,        XmATTACH_FORM,
		    NULL);

		XtRealizeWidget(popup_shell);
		updateCurrentAlarmWindow(area);


	} else {

		if (XtIsManaged(area->currentAlarmForm)){
			XtVaSetValues(menuButton, XmNset, FALSE, NULL);
			XtUnmanageChild(area->currentAlarmForm);
			XUnmapWindow(XtDisplay(area->currentAlarmForm),
			    XtWindow(XtParent(area->currentAlarmForm)));
		} else {
			XtVaSetValues(menuButton, XmNset, TRUE, NULL);
			XtManageChild(area->currentAlarmForm);
			XMapWindow(XtDisplay(area->currentAlarmForm),
			    XtWindow(XtParent(area->currentAlarmForm)));
			updateCurrentAlarmWindow(area);
		}
	}
}

/**************************************************************************
	close Current Alarm History window 
**************************************************************************/
void closeCurrent_callback(Widget w,Widget currentForm,caddr_t call_data)
{
	Widget menuButton;

	XtVaGetValues(currentForm, XmNuserData, &menuButton, NULL);

	XtVaSetValues(menuButton, XmNset, FALSE, NULL);
	XtUnmanageChild(currentForm);
	XUnmapWindow(XtDisplay(currentForm),
	    XtWindow(XtParent(currentForm)));
}

/******************************************************************
      updateCurrent Alarm History strings
*****************************************************************/
void updateCurrentAlarmString(time_t *ptimeofday,char *name,char value[],
int stat,int sev)
{
	int n;

	n = currentAlarmIndex;
	currentAlarm[n].timeofday=*ptimeofday;
	currentAlarm[n].name=name;
	currentAlarm[n].stat=stat;
	currentAlarm[n].sevr=sev;
	strncpy(currentAlarm[n].value,value,MAX_STRING_SIZE);
	n = (n+1)%10;
	currentAlarmIndex = n;
}

/******************************************************************
      updateCurrent Alarm History Window
*****************************************************************/
void updateCurrentAlarmWindow(ALINK *area)
{
	int i,j=0;
	XmString xstr;
	char *str;
	char buff[100];

	if ( area->currentAlarmForm && XtIsManaged(area->currentAlarmForm) ) {

		j = currentAlarmIndex;
		for (i=0;i<10;i++){
			if (currentAlarm[j].name){
				str = ctime(&(currentAlarm[j].timeofday));
				*(str + strlen(str)-1) = '\0';

				sprintf(buff,
				    "  %-24s :  %-28s %-10s %-10s %s",
				    str,
				    currentAlarm[j].name,
				    alarmStatusString[currentAlarm[j].stat],
				    alarmSeverityString[currentAlarm[j].sevr],
				    currentAlarm[j].value);
			} else {
				strcpy(buff,"  ");
			}
			xstr = XmStringCreateSimple(buff);
			XtVaSetValues(area->currentAlarm[i],
			    XmNlabelString, xstr,
			    NULL);
			XmStringFree(xstr);
			j = (j+1)%10;
		}
	}
}

/******************************************************************
      reset Current Alarm History Window
*****************************************************************/
void resetCurrentAlarmWindow()
{
	int i,j=0;

	for (i=0;i<10;i++){
		currentAlarm[j].name = NULL;
	}
}

