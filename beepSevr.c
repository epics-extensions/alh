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
/* beepSevr.c */

/************************DESCRIPTION***********************************
  beepSevr.c: a popup dialog  window
  This file contains routines for creating beep severity dialog.
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/ToggleB.h>
#include <Xm/PanedW.h>
#include <Xm/Protocols.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>

#include "axArea.h"
#include "alLib.h"
#include "alh.h"
#include "ax.h"
#include "alarm.h"

static struct beepSevrWindow {
	void   *area;
	Widget menuButton;
	Widget beepSevrDialog;
	Widget nameLabelW;
	Widget nameTextW;
	Widget beepSevrFrameW;
	Widget beepSevrToggleButtonW[ALH_ALARM_NSEV-1];
	int beepSevr;
};

extern const char * alhAlarmSeverityString[];

/* forward declarations */
static void beepSevrDismissCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void beepSevrHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void beepSevrCreateDialog(ALINK*area);
static void beepSevrUpdateDialogWidgets(struct beepSevrWindow *beepSevrWindow);
static void beepSevrChangeCallback( Widget widget,XtPointer calldata,XtPointer cbs);


/******************************************************
  beepSevrUpdateDialog
******************************************************/
void beepSevrUpdateDialog(area)
ALINK  *area;
{
	struct beepSevrWindow *beepSevrWindow;

	if (!area->beepSevrWindow)  return;

	beepSevrWindow = (struct beepSevrWindow *)area->beepSevrWindow;

	if (!beepSevrWindow->beepSevrDialog || !XtIsManaged(beepSevrWindow->beepSevrDialog)) return;

	beepSevrUpdateDialogWidgets(beepSevrWindow);
}

/******************************************************
  beepSevrShowDialog
******************************************************/
void beepSevrShowDialog(area, menuButton)
ALINK    *area;
Widget   menuButton;
{
	struct beepSevrWindow *beepSevrWindow;

	beepSevrWindow = (struct beepSevrWindow *)area->beepSevrWindow;

	/* Dismiss Dialog */
	if (beepSevrWindow && beepSevrWindow->beepSevrDialog && 
	    XtIsManaged(beepSevrWindow->beepSevrDialog)) {
		beepSevrDismissCallback(NULL, (XtPointer)beepSevrWindow, NULL);
		return;
	}

	/* create beepSevrWindow and Dialog Widgets if necessary */
	if (!beepSevrWindow)  beepSevrCreateDialog(area);

	/* update beepSevrWindow */
	beepSevrWindow = (struct beepSevrWindow *)area->beepSevrWindow;
	beepSevrWindow->menuButton = menuButton;

	/* update Dialog Widgets */
	beepSevrUpdateDialogWidgets(beepSevrWindow);

	/* show Dialog */
	if (!beepSevrWindow->beepSevrDialog) return;
	if (!XtIsManaged(beepSevrWindow->beepSevrDialog)) {
		XtManageChild(beepSevrWindow->beepSevrDialog);
	}
	XMapWindow(XtDisplay(beepSevrWindow->beepSevrDialog),
	    XtWindow(XtParent(beepSevrWindow->beepSevrDialog)));
	if (menuButton) XtVaSetValues(menuButton, XmNset, TRUE, NULL);

}

/******************************************************
  beepSevrUpdateDialogWidgets
******************************************************/

static void beepSevrUpdateDialogWidgets(beepSevrWindow)
struct beepSevrWindow *beepSevrWindow;
{
	struct gcData *pgcData;
	GCLINK *link;
	int i,linkType;
	int beepSevr;
	XmString string;
	char buff[MAX_STRING_LENGTH];

	if (! beepSevrWindow || !beepSevrWindow->beepSevrDialog ) return;

	link = (GCLINK *)getSelectionLinkArea(beepSevrWindow->area);

	if (!link) {

		string = XmStringCreateSimple("");
		XtVaSetValues(beepSevrWindow->nameTextW,XmNlabelString, string, NULL);
		XmStringFree(string);
		return;
	}

	linkType = getSelectionLinkTypeArea(beepSevrWindow->area);

	pgcData = link->pgcData;

	/* ---------------------------------
	     Group/Channel Name 
	     --------------------------------- */
	if (linkType == GROUP) string = XmStringCreateSimple("Group Name:");
	else string = XmStringCreateSimple("Channel Name:");
	XtVaSetValues(beepSevrWindow->nameLabelW, XmNlabelString, string, NULL);
	XmStringFree(string);

	if (pgcData->alias){
		string = XmStringCreateSimple(pgcData->alias);
	} else {
		string = XmStringCreateSimple(pgcData->name);
	}
	XtVaSetValues(beepSevrWindow->nameTextW, XmNlabelString, string, NULL);
	XmStringFree(string);

	/* ---------------------------------
	     Beep Severity 
	     --------------------------------- */
	beepSevr = pgcData->beepSevr;
	if (beepSevr == 0) beepSevr=1;
	for (i = 0; i < ALH_ALARM_NSEV-1; i++){
		XmToggleButtonGadgetSetState(beepSevrWindow->beepSevrToggleButtonW[i], False, False);
	}
	XmToggleButtonGadgetSetState(beepSevrWindow->beepSevrToggleButtonW[beepSevr-1], True, False);

}

/******************************************************
  beepSevrCreateDialog
******************************************************/
static void beepSevrCreateDialog(area)
ALINK    *area;
{
	struct beepSevrWindow *beepSevrWindow;

	Widget beepSevrDialogShell, beepSevrDialog;
	Widget radiobox, form, beepSevrFrameW;
	Widget nameLabelW, nameTextW;
	Widget beepSevrToggleButtonW[ALH_ALARM_NSEV-1];
	int i;
	char *pstring;
	XmString string;
	static ActionAreaItem beepSevr_items[] = {
		         { "Dismiss",  beepSevrDismissCallback,  NULL    },
		         { "Help",    beepSevrHelpCallback,    NULL    },
		     	};

	if (!area) return;

	beepSevrWindow = (struct beepSevrWindow *)area->beepSevrWindow;

	if (beepSevrWindow && beepSevrWindow->beepSevrDialog){
		if (XtIsManaged(beepSevrWindow->beepSevrDialog)) return;
		else XtManageChild(beepSevrWindow->beepSevrDialog);
	}


	beepSevrWindow = (struct beepSevrWindow *)calloc(1,sizeof(struct beepSevrWindow));
	area->beepSevrWindow = (void *)beepSevrWindow;
	beepSevrWindow->area = (void *)area;

	beepSevrDialogShell = XtVaCreatePopupShell("Set Beep Severity",
	    transientShellWidgetClass, area->toplevel, 
	    XmNallowShellResize, TRUE,
	    NULL);

	/* Modify the window manager menu "close" callback */
	{
		Atom         WM_DELETE_WINDOW;
		XtVaSetValues(beepSevrDialogShell,
		    XmNdeleteResponse, XmDO_NOTHING, NULL);
		WM_DELETE_WINDOW = XmInternAtom(XtDisplay(beepSevrDialogShell),
		    "WM_DELETE_WINDOW", False);
		XmAddWMProtocolCallback(beepSevrDialogShell,WM_DELETE_WINDOW,
		    (XtCallbackProc)beepSevrDismissCallback, (XtPointer)beepSevrWindow);
	}

	beepSevrDialog = XtVaCreateWidget("beepSevrDialog",
	    xmPanedWindowWidgetClass, beepSevrDialogShell,
	    XmNallowResize, TRUE,
	    XmNsashWidth,  1,
	    XmNsashHeight, 1,
	    XmNuserData,   area,
	    NULL);


	form = XtVaCreateWidget("control_area",
	    xmFormWidgetClass, beepSevrDialog,
	    XmNallowResize,    TRUE,
	    NULL);

	/* ---------------------------------
	     Group/Channel Name 
	     --------------------------------- */
	nameLabelW = XtVaCreateManagedWidget("nameLabelW",
	    xmLabelGadgetClass, form,
	    XmNalignment,       XmALIGNMENT_END,
	    XmNtopAttachment,   XmATTACH_FORM,
/*
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition,   50,
*/
	    XmNrecomputeSize,   True,
	    NULL);

	nameTextW = XtVaCreateManagedWidget("nameTextW",
	    xmLabelGadgetClass, form,
	    XmNalignment,       XmALIGNMENT_BEGINNING,
	    XmNleftAttachment,  XmATTACH_WIDGET,
	    XmNleftWidget,      nameLabelW,
	    XmNrightAttachment,  XmATTACH_NONE,
	    XmNrecomputeSize,   True,
	    NULL);

	/* ---------------------------------
	     Beep Severity 
	     --------------------------------- */

	beepSevrFrameW = XtVaCreateManagedWidget("beepSevrFrameW",
		xmFrameWidgetClass, form,
	    XmNtopAttachment,  XmATTACH_WIDGET,
	    XmNtopWidget,      nameLabelW,
		XmNtopOffset,       5,
		XmNleftAttachment, XmATTACH_POSITION,
		XmNleftPosition,   25,
		NULL);

    radiobox = XmCreateRadioBox (beepSevrFrameW, "radiobox", NULL, 0);

	for (i = 0; i < ALH_ALARM_NSEV-1; i++){
	    beepSevrToggleButtonW[i] = XmCreateToggleButtonGadget (radiobox,
			 (char *)alhAlarmSeverityString[i+1], NULL, 0);
		XtVaSetValues(beepSevrToggleButtonW[i], XmNuserData, area, NULL);
		XtAddCallback(beepSevrToggleButtonW[i], XmNvalueChangedCallback,
		    beepSevrChangeCallback, (XtPointer)(i+1));
		XtManageChild(beepSevrToggleButtonW[i]);

	}

	XtManageChild(radiobox);

	XtManageChild(form);

	/* Set the client data "Apply", "Dismiss", "OK", and "Help" button's callbacks. */
	beepSevr_items[0].data = (XtPointer)beepSevrWindow;
	beepSevr_items[1].data = (XtPointer)beepSevrWindow;

	(void)createActionButtons(beepSevrDialog, beepSevr_items, XtNumber(beepSevr_items));

	beepSevrWindow->beepSevrDialog = beepSevrDialog;
	beepSevrWindow->nameLabelW = nameLabelW;
	beepSevrWindow->nameTextW = nameTextW;
	beepSevrWindow->beepSevrFrameW = beepSevrFrameW;
	for (i = 0; i < ALH_ALARM_NSEV-1; i++){
		beepSevrWindow->beepSevrToggleButtonW[i] = beepSevrToggleButtonW[i];
	}

	XtManageChild(beepSevrDialog);

	XtRealizeWidget(beepSevrDialogShell);

}

/******************************************************
  beepSevrHelpCallback
******************************************************/
static void beepSevrHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	char *message1 = 
	"Set the beep severity level for a group or channel.\n"
	"\n"
    "Beep severity is the minimum severity level required for beeping.\n"
    "Beeping for this group or channel will not occur when the highest\n"
	"unacknowledged severity is less than the specified beep severity.\n"
	"\n"
	"Press the Dismiss button to close the dialog window.\n"
	"Press the Help    button to get this help description window.\n"
    "  \n"
	;
	char * message2 = "  ";

	createDialog(widget,XmDIALOG_INFORMATION, message1,message2);

}

/******************************************************
  beepSevrChangeCallback
******************************************************/
static void beepSevrChangeCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	int beepSevr=(int)calldata;
    ALINK *area;
	struct chanData *cdata;
	struct gcData *pgcData;
	GCLINK *link;
	int linkType;

	if (!XmToggleButtonGadgetGetState(widget)) return;

    XtVaGetValues(widget, XmNuserData, &area, NULL);

	link =getSelectionLinkArea(area);
	if (!link) return;
	linkType =getSelectionLinkTypeArea(area);
	if (linkType == CHANNEL) alSetBeepSevrChan((CLINK *)link,beepSevr);
	if (linkType == GROUP) alSetBeepSevrGroup((GLINK *)link,beepSevr);

	pgcData = link->pgcData;
	alLogSetBeepSevr(pgcData->name,alhAlarmSeverityString[pgcData->beepSevr]);
	link->pmainGroup->modified = 1;

	/* ---------------------------------
	     Update all dialog Windows
	     --------------------------------- */
	axUpdateDialogs(area);

}

/******************************************************
  beepSevrDismissCallback
******************************************************/
static void beepSevrDismissCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	struct beepSevrWindow *beepSevrWindow=(struct beepSevrWindow *)calldata;
	Widget beepSevrDialog;

	beepSevrDialog = beepSevrWindow->beepSevrDialog;
	XtUnmanageChild(beepSevrDialog);
	XUnmapWindow(XtDisplay(beepSevrDialog), XtWindow(XtParent(beepSevrDialog)));
	if (beepSevrWindow->menuButton)
		XtVaSetValues(beepSevrWindow->menuButton, XmNset, FALSE, NULL);

}


