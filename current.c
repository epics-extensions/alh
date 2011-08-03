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
/* current.c */

/************************DESCRIPTION***********************************
  Routines for viewing the current alarm history window
**********************************************************************/

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

/* global variables */
extern char * alhAlarmSeverityString[];
extern char * alhAlarmStatusString[];

/* forward declarations */
void closeCurrentCallback( Widget w, Widget currentForm, caddr_t call_data);


/**************************************************************************
	create scroll window for file view
**************************************************************************/
void currentAlarmHistoryWindow(ALINK *area,Widget menuButton)
{
	Widget popup_shell,title,button;
	Widget previous;
	Atom   WM_DELETE_WINDOW;
	int    i;
	char   *app_name;
	XmString xstr;

	if (!area->currentAlarmForm) {
		app_name = (char*) calloc(1,strlen(programName)+6);
		strcpy(app_name, programName);
		strcat(app_name, "-hist");

		popup_shell = XtAppCreateShell( app_name, programName,
		    applicationShellWidgetClass, display, NULL, 0);

		free(app_name);

		XtVaSetValues(popup_shell,
		    XmNtitle, "Alarm Handler: Current Alarm History", NULL);

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
		    (XtCallbackProc)closeCurrentCallback,
		    (XtPointer)area->currentAlarmForm);

		/* add close button */
		button = XtVaCreateManagedWidget("Close",
		    xmPushButtonWidgetClass, area->currentAlarmForm,
		    XmNtopAttachment,          XmATTACH_FORM,
		    XmNtopOffset,              5,
		    XmNleftAttachment,         XmATTACH_FORM,
		    XmNleftOffset,             5,
		    XmNuserData,               menuButton,
		    (XtPointer)NULL);
		XtAddCallback(button, XmNactivateCallback,
		    (XtCallbackProc)closeCurrentCallback,
		    area->currentAlarmForm);

		previous = button;

		/* add title line */
		xstr = XmStringCreateSimple(
		    "    TIME_STAMP       PROCESS_VARIABLE_NAME          "
		    "STATUS     SEVERITY   VALUE       ");
		title = XtVaCreateManagedWidget("CurrentTitle",
		    xmLabelGadgetClass,        area->currentAlarmForm,
		    XmNlabelString,            xstr,
		    XmNtopAttachment,          XmATTACH_FORM,
		    XmNtopOffset,              10,
		    XmNleftAttachment,         XmATTACH_WIDGET,
		    XmNleftWidget,             button,
		    XmNrightAttachment,        XmATTACH_FORM,
		    NULL);
		XmStringFree(xstr);

		/* create 10 label widgets  */
		for ( i=0;i<10;i++){

			area->currentAlarm[i] = XtVaCreateManagedWidget( "CurrentAlarm",
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

		/* create a blank line */
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
void closeCurrentCallback(Widget w,Widget currentForm,caddr_t call_data)
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
void updateCurrentAlarmString(ALINK *area, time_t *ptimeofday,char *name,
char value[],int stat,int sevr)
{
	int n;

  if ( !area ) return;
	n = area->currentAlarmIndex;
	sprintf(area->currentAlarmString[n],
		"  %-24s %-31.31s %-10.10s %-10.10s %s",
		ctime(ptimeofday),
		name,
		alhAlarmStatusString[stat],
		alhAlarmSeverityString[sevr],
		value);
	n = (n+1)%10;
	area->currentAlarmIndex = n;

}

/******************************************************************
      updateCurrent Alarm History Window
*****************************************************************/
void updateCurrentAlarmWindow(ALINK *area)
{
	int i,j;
	XmString xstr;

	if ( area->currentAlarmForm && XtIsManaged(area->currentAlarmForm) ) {

		j = area->currentAlarmIndex;
		for (i=0;i<10;i++){
			xstr = XmStringCreateSimple(area->currentAlarmString[j]);
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
void resetCurrentAlarmWindow(ALINK *area)
{
	int i;

	for (i=0;i<10;i++){
		strcpy(area->currentAlarmString[i],"   ");
	}
	area->currentAlarmIndex = 0;
	updateCurrentAlarmWindow(area);
}

