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
/* noAck.c */

/************************DESCRIPTION***********************************
  Routines for modifying noAck mask for 1 hour.
**********************************************************************/

#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/PanedW.h>
#include <Xm/Protocols.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>

#include "axArea.h"
#include "alLib.h"
#include "alh.h"
#include "ax.h"

extern int _DB_call_flag;

typedef struct {
	char *label;
	int index;
} noAckChoiceItem;

struct noAckWindow {
	void *area;
	Widget menuButton;
	Widget noAckDialog;
	Widget nameLabelW;
	Widget nameTextW;
	Widget noAckOneHourToggleButton;
};

/* forward declarations */
static void noAckDismissCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void noAckHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void noAckActivateCallback( Widget widget,XtPointer calldata,XtPointer cbs);
static void noAckCreateDialog(ALINK*area);
static void noAckUpdateDialogWidgets(struct noAckWindow *noAckWindow);
static void noAckOneHourTimerGroupCallback(XtPointer data, XtIntervalId *id);
static void noAckOneHourTimerChanCallback(XtPointer data, XtIntervalId *id);
static void alResetNoAckChan(CLINK *clink);
static void alResetNoAckGroup(GLINK *glink);

/******************************************************
  noAckUpdateDialog
******************************************************/
void noAckUpdateDialog(ALINK *area)
{
	struct noAckWindow *noAckWindow;

	noAckWindow = (struct noAckWindow *)area->noAckWindow;

	if (!noAckWindow)  return;

	if (!noAckWindow->noAckDialog ||
	    !XtIsManaged(noAckWindow->noAckDialog)) return;

	noAckUpdateDialogWidgets(noAckWindow);
}

/******************************************************
  noAckShowDialog
******************************************************/
void noAckShowDialog(ALINK *area,Widget menuButton)
{
	struct noAckWindow *noAckWindow;

	noAckWindow = (struct noAckWindow *)area->noAckWindow;

	/* dismiss Dialog */
	if (noAckWindow && noAckWindow->noAckDialog && 
	    XtIsManaged(noAckWindow->noAckDialog)) {
		noAckDismissCallback(NULL, (XtPointer)noAckWindow, NULL);
		return;
	}

	/* create noAckWindow and Dialog Widgets if necessary */
	if (!noAckWindow)  noAckCreateDialog(area);

	/* update noAckWindow */
	noAckWindow = (struct noAckWindow *)area->noAckWindow;
	noAckWindow->menuButton = menuButton;

	/* update Dialog Widgets */
	noAckUpdateDialogWidgets(noAckWindow);

	/* show Dialog */
	if (!noAckWindow->noAckDialog) return;
	if (!XtIsManaged(noAckWindow->noAckDialog)) {
		XtManageChild(noAckWindow->noAckDialog);
	}
	XMapWindow(XtDisplay(noAckWindow->noAckDialog),
	    XtWindow(XtParent(noAckWindow->noAckDialog)));
	if (menuButton) XtVaSetValues(menuButton, XmNset, TRUE, NULL);
}

/******************************************************
  noAckUpdateDialogWidgets
******************************************************/
static void noAckUpdateDialogWidgets(struct noAckWindow *noAckWindow)
{
	struct gcData *pgcData;
	GCLINK *link;
	int linkType;
	XmString string;

	if (! noAckWindow || !noAckWindow->noAckDialog ) return;

	link =getSelectionLinkArea(noAckWindow->area);

	/* ---------------------------------
	     Group/Channel name
	     --------------------------------- */
	if (!link) {
		string = XmStringCreateSimple("");
		if (noAckWindow->nameTextW )
			XtVaSetValues(noAckWindow->nameTextW,XmNlabelString, string, NULL);
		XmStringFree(string);
		return;
	}

	pgcData = link->pgcData;
	linkType =getSelectionLinkTypeArea(noAckWindow->area);

	/* ---------------------------------
	     Group/Channel label
	     --------------------------------- */
	if (linkType == GROUP) string = XmStringCreateSimple("Group: ");
	else string = XmStringCreateSimple("Channel: ");
	XtVaSetValues(noAckWindow->nameLabelW, XmNlabelString, string, NULL);
	XmStringFree(string);

	if (pgcData->alias){
		string = XmStringCreateSimple(pgcData->alias);
	} else {
		string = XmStringCreateSimple(pgcData->name);
	}
	XtVaSetValues(noAckWindow->nameTextW, XmNlabelString, string, NULL);
	XmStringFree(string);

	/* ---------------------------------
	     noAck for one hour toggle button
	     --------------------------------- */
	if (pgcData->noAckTimerId)
		XmToggleButtonGadgetSetState(noAckWindow->noAckOneHourToggleButton,TRUE,FALSE);
	else XmToggleButtonGadgetSetState(noAckWindow->noAckOneHourToggleButton,FALSE,FALSE);
}

/******************************************************
  noAckCreateDialog
******************************************************/
static void noAckCreateDialog(ALINK *area)
{
	struct noAckWindow *noAckWindow;

	Widget noAckDialogShell, noAckDialog;
	Widget form;
	Widget nameLabelW, nameTextW;
	Widget noAckOneHourToggleButton;
	static ActionAreaItem buttonItems[] = {
		         { "Dismiss", noAckDismissCallback, NULL    },
		         { "Help",    noAckHelpCallback,    NULL    },
		     	};

	if (!area) return;

	noAckWindow = (struct noAckWindow *)area->noAckWindow;

	if (noAckWindow && noAckWindow->noAckDialog){
		if (XtIsManaged(noAckWindow->noAckDialog)) return;
		else XtManageChild(noAckWindow->noAckDialog);
	}

	noAckWindow = (struct noAckWindow *)calloc(1,sizeof(struct noAckWindow));
	area->noAckWindow = (void *)noAckWindow;
	noAckWindow->area = (void *)area;

	noAckDialogShell = XtVaCreatePopupShell("Modify NoAck Mask",
	    transientShellWidgetClass, area->toplevel, 
	    XmNallowShellResize, TRUE,
	    NULL);

	/* Modify the window manager menu "close" callback */
	{
		Atom         WM_DELETE_WINDOW;
		XtVaSetValues(noAckDialogShell,
		    XmNdeleteResponse, XmDO_NOTHING, NULL);
		WM_DELETE_WINDOW = XmInternAtom(XtDisplay(noAckDialogShell),
		    "WM_DELETE_WINDOW", False);
		XmAddWMProtocolCallback(noAckDialogShell,WM_DELETE_WINDOW,
		    (XtCallbackProc)noAckDismissCallback, (XtPointer)noAckWindow);
	}

	noAckDialog = XtVaCreateWidget("noAckDialog",
	    xmPanedWindowWidgetClass, noAckDialogShell,
	    XmNallowResize,      TRUE,
	    XmNsashWidth,        1,
	    XmNsashHeight,       1,
	    XmNuserData,         area,
	    (XtPointer)NULL);

	form = XtVaCreateWidget("control_area",
	    xmFormWidgetClass, noAckDialog,
	    XmNallowResize,     TRUE,
	    NULL);

	/* ---------------------------------
	     Group/Channel Name 
	     --------------------------------- */
	nameLabelW = XtVaCreateManagedWidget("nameLabelW",
	    xmLabelGadgetClass, form,
	    XmNalignment,       XmALIGNMENT_END,
	    XmNtopAttachment,   XmATTACH_FORM,
	    XmNrecomputeSize,   True,
	    NULL);

	nameTextW = XtVaCreateManagedWidget("nameTextW",
	    xmLabelGadgetClass, form,
	    XmNalignment,       XmALIGNMENT_BEGINNING,
	    XmNleftAttachment,  XmATTACH_WIDGET,
	    XmNleftWidget,      nameLabelW,
	    XmNrightAttachment, XmATTACH_NONE,
	    XmNrecomputeSize,   True,
	    NULL);

	/* ---------------------------------
	     NoAck One Hour toggle button
	     --------------------------------- */
    noAckOneHourToggleButton = XtVaCreateManagedWidget("NoAck For One Hour",
        xmToggleButtonGadgetClass, form,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              nameLabelW,
		XmNtopOffset,              10,
        XmNuserData,               (XtPointer)area,
        NULL);

    XtAddCallback(noAckOneHourToggleButton, XmNvalueChangedCallback,
        (XtCallbackProc)noAckActivateCallback,area);

	XtManageChild(form);

	/* Set the client data "Dismiss" and "Help" button's callbacks. */
	buttonItems[0].data = (XtPointer)noAckWindow;
	buttonItems[1].data = (XtPointer)noAckWindow;

	(void)createActionButtons(noAckDialog, buttonItems, XtNumber(buttonItems));

	noAckWindow->noAckDialog = noAckDialog;
	noAckWindow->nameLabelW = nameLabelW;
	noAckWindow->nameTextW = nameTextW;
	noAckWindow->noAckOneHourToggleButton = noAckOneHourToggleButton;

	XtManageChild(noAckDialog);

	XtRealizeWidget(noAckDialogShell);
}

/******************************************************
  noAckHelpCallback
******************************************************/
static void noAckHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	char *message1 = 
	"Set group or channel ack/noAck mask to noAck for one hour and then\n"
	"reset it to initial (config file) value after the hour is over.\n\n"
	"Setting the 'noAck for One Hour' button to ON means setting the ack/noAck\n"
	"mask to noAck for the selected channel or for all channels in the\n"
	"selected group and creating a one hour timer. When the timer expires\n"
	"the ack/noAck masks will be set to the initial value from the config file.\n\n"
	;
	char * message2 =
	"Setting the 'noAck for One Hour' button to OFF means setting the ack/noAck\n"
	"mask to the initial value from the config file for the celected channel or\n"
	"for all channels in the selected group and removing the one hour timer.\n\n"
	"Press the Dismiss button to close the dialog window.\n"
	"Press the Help    button to get this help description window.\n"
	;

	createDialog(widget,XmDIALOG_INFORMATION, message1,message2);

}


/******************************************************
  noAckDismissCallback
******************************************************/
static void noAckDismissCallback(Widget widget,XtPointer calldata,
XtPointer cbs)
{
	struct noAckWindow *noAckWindow=(struct noAckWindow *)calldata;
	Widget noAckDialog;

	noAckDialog = noAckWindow->noAckDialog;
	XtUnmanageChild(noAckDialog);
	XUnmapWindow(XtDisplay(noAckDialog), XtWindow(XtParent(noAckDialog)));
	if (noAckWindow->menuButton)
		XtVaSetValues(noAckWindow->menuButton, XmNset, FALSE, NULL);
}


/***************************************************
  noAckOneHourTimerChanCallback
****************************************************/
static void noAckOneHourTimerChanCallback(XtPointer data, XtIntervalId *id)
{
	CLINK * clink = (CLINK *)data;

	clink->pchanData->noAckTimerId=0;;
	alResetNoAckChan(clink);
	alLogOpModMessage(0,(GCLINK*)clink,
		"Set Ack after expiration of NoAck one hour timer");
	clink->pmainGroup->modified = TRUE;
	axUpdateDialogs(clink->pmainGroup->area);
}

/***************************************************
  noAckOneHourTimerGroupCallback
****************************************************/
static void noAckOneHourTimerGroupCallback(XtPointer data, XtIntervalId *id)
{
	GLINK * glink = (GLINK *)data;

	glink->pgroupData->noAckTimerId=0;;
	alResetNoAckGroup(glink);
	alLogOpModMessage(0,(GCLINK*)glink,
		"Set Ack after expiration of NoAck one hour timer");
	glink->pmainGroup->modified = TRUE;
	axUpdateDialogs(glink->pmainGroup->area);
}

/***************************************************
  noAckActivateCallback
****************************************************/
static void noAckActivateCallback(Widget widget,XtPointer link,
XtPointer call_data)
{
    int seconds = 3600; /* 1 hour */
#if 0
    int seconds = 60; /* 1 minute */
#endif
	ALINK *area;
    GCLINK *gclink;
    struct gcData *gcdata;
	int linkType;
    XtTimerCallbackProc proc;

	XtVaGetValues(widget, XmNuserData, &area, NULL);
	gclink =(GCLINK *)getSelectionLinkArea(area);
    gcdata = gclink->pgcData;
	linkType =getSelectionLinkTypeArea(area);
	if (linkType == GROUP) proc = noAckOneHourTimerGroupCallback;
	else proc = noAckOneHourTimerChanCallback;

	if (gcdata->noAckTimerId) {
		XtRemoveTimeOut(gcdata->noAckTimerId);
		gcdata->noAckTimerId = 0;
	}
    if (XmToggleButtonGadgetGetState(widget)) {
		if (linkType==GROUP) {
			alRemoveNoAck1HrTimerGroup((GLINK*)gclink);
			alForceGroupMask((GLINK*)gclink,ALARMACK,1);
  		} else {
			alRemoveNoAck1HrTimerChan((CLINK*)gclink);
			alForceChanMask((CLINK*)gclink,ALARMACK,1);
		}
        gcdata->noAckTimerId = XtAppAddTimeOut(appContext,
            (unsigned long)(1000*seconds),
            (XtTimerCallbackProc)proc,
            (XtPointer)gclink);
		alLogOpModMessage(0,gclink,
			"Set NoAck and start NoAck one hour timer");
    } else {
		if (linkType==GROUP) {
			alResetNoAckGroup((GLINK *)gclink);
  		} else {
			alResetNoAckChan((CLINK *)gclink);
		}
		alLogOpModMessage(0,gclink,
			"Reset Ack mask and cancel NoAck one hour timer");
    }
	gclink->modified = 1;
	gclink->pmainGroup->modified = 1;
    axUpdateDialogs(area);
}


/************************************************************************* 
	alResetNoAckChan
 **********************************************************************/
static void alResetNoAckChan(CLINK *clink)
{
	MASK mask;

	if (clink->pchanData->noAckTimerId) return; 

	mask = clink->pchanData->curMask;
	mask.Ack = clink->pchanData->defaultMask.Ack;
	alChangeChanMask(clink,mask);
	if(_DB_call_flag)  alLog2DBMask(clink->pchanData->name);
}

/************************************************************************* 
	alResetNoAckGroup
 **********************************************************************/
static void alResetNoAckGroup(GLINK *glink)
{
	GLINK *group;
	CLINK *clink;
	SNODE *pt;

	if (glink->pgroupData->noAckTimerId) return; 

	/*
	 * for all channels in this group
	 */
	pt = sllFirst(&(glink->chanList));
	while (pt) {
		clink = (CLINK *)pt;
		alResetNoAckChan(clink);
		pt = sllNext(pt);
	}
	/*
	 * for all subgroups
	 */
	pt = sllFirst(&(glink->subGroupList));
	while (pt) {
		group = (GLINK *)pt;
		alResetNoAckGroup(group);
		pt = sllNext(pt);
	}
}

