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
  Routines for modifying noAck mask for a specified number of minutes.
**********************************************************************/

static char *sccsId = "@(#) $Id$";

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

static char buff[100];

/* forward declarations */
static void noAckDismissCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void noAckHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void noAckActivateCallback( Widget widget,XtPointer calldata,XtPointer cbs);
static void noAckCreateDialog(ALINK*area);
static void noAckUpdateDialogWidgets(struct noAckWindow *noAckWindow);
static void noAckOneHourTimerGroupCallback(XtPointer data, XtIntervalId *id);
static void noAckOneHourTimerChanCallback(XtPointer data, XtIntervalId *id);
static void alForceNoAckChan(CLINK *clink, int setNewNoAckTimer);
static void alForceNoAckGroup(GLINK *glink, int setNewNoAckTimer);

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
	    NULL);

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
	"This dialog window allows an operator to set ack/noAck masks\n"
	"to noAck and create a one hour timer.  After the one hour timer\n"
    "expires, the ack/noAck masks will be set to Ack.\n"
	"  \n"
	"Toggling the 'noAck for One Hour' button means toggling the ack/noAck\n"
	"mask for the selected channel or for all channels in the selected group\n"
	"and creating or removing the one hour timer.\n"
	"  \n"
	"Press the Dismiss button to close the dialog window.\n"
	"Press the Help    button to get this help description window.\n"
	;
	char * message2 = "  ";

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
	alForceNoAckChan(clink,0);
	sprintf(buff,"Set Ack after expiration of NoAck one hour timer  --- %s\n",
		clink->pchanData->name);
	alLogOpMod(buff);
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
	alForceNoAckGroup(glink,0);
	sprintf(buff,"Set Ack after expiration of NoAck one hour timer  --- %s\n",
		glink->pgroupData->name);
	alLogOpMod(buff);
	glink->pmainGroup->modified = TRUE;
	axUpdateDialogs(glink->pmainGroup->area);
}

/***************************************************
  noAckActivateCallback
****************************************************/
static void noAckActivateCallback(Widget widget,XtPointer link,
XtPointer call_data)
{
    /*int seconds = 3600;*/ /* 1 hour */
    int seconds = 30;
	ALINK *area;
    GCLINK *gclink;
    struct gcData *gcdata;
	int linkType;
	struct timerData *timerData;
    XtTimerCallbackProc proc;
	int setNewNoAckTimer;

	XtVaGetValues(widget, XmNuserData, &area, NULL);
	gclink =(GCLINK *)getSelectionLinkArea(area);
    gcdata = gclink->pgcData;
	linkType =getSelectionLinkTypeArea(area);
	if (linkType == GROUP) proc = noAckOneHourTimerGroupCallback;
	else proc = noAckOneHourTimerChanCallback;

    if (XmToggleButtonGadgetGetState(widget)) {
        gcdata->noAckTimerId = XtAppAddTimeOut(appContext,
            (unsigned long)(1000*seconds),
            (XtTimerCallbackProc)proc,
            (XtPointer)gclink);
		setNewNoAckTimer = 1;
		sprintf(buff,"Set NoAck and start NoAck one hour timer --- %s\n",gcdata->name);
		alLogOpMod(buff);
    } else {
        if (gcdata->noAckTimerId) {
            XtRemoveTimeOut(gcdata->noAckTimerId);
            gcdata->noAckTimerId = 0;
        }
		setNewNoAckTimer = 0;
		sprintf(buff,"Set Ack and cancel NoAck one hour timer  --- %s\n",gcdata->name);
		alLogOpMod(buff);
    }
	if (linkType==GROUP) {
	    alForceNoAckGroup((GLINK *)gclink,setNewNoAckTimer);
  	} else {
		alForceNoAckChan((CLINK *)gclink,setNewNoAckTimer);
	}
	gclink->modified = 1;
	gclink->pmainGroup->modified = 1;
    axUpdateDialogs(area);
}


/************************************************************************* 
	alForceNoAckChan
 **********************************************************************/
static void alForceNoAckChan(CLINK *clink, int setNewNoAckTimer)
{
	MASK mask;

	mask = clink->pchanData->curMask;

	if (setNewNoAckTimer) {
		if (clink->pchanData->noAckTimerId){
			XtRemoveTimeOut(clink->pchanData->noAckTimerId);
			clink->pchanData->noAckTimerId = 0;
		}
		mask.Ack = 1; /* NOACK */
	} else { /* timer callback */
		if (clink->pchanData->noAckTimerId) return; 
		mask.Ack = 0; /* ACK */
	}

	alChangeChanMask(clink,mask);

	if(_DB_call_flag)  alLog2DBMask(clink->pchanData->name);
}

/************************************************************************* 
	alForceNoAckGroup
 **********************************************************************/
static void alForceNoAckGroup(GLINK *glink, int setNewNoAckTimer)
{
	GLINK *group;
	CLINK *clink;
	SNODE *pt;
	/*
	 * for all channels in this group
	 */
	pt = sllFirst(&(glink->chanList));
	while (pt) {
		clink = (CLINK *)pt;
		alForceNoAckChan(clink,setNewNoAckTimer);
		pt = sllNext(pt);
	}
	/*
	 * for all subgroups
	 */
	pt = sllFirst(&(glink->subGroupList));
	while (pt) {
		group = (GLINK *)pt;
		if (setNewNoAckTimer) {
			if (group->pgroupData->noAckTimerId){
				XtRemoveTimeOut(group->pgroupData->noAckTimerId);
				group->pgroupData->noAckTimerId = 0;
			}
			alForceNoAckGroup(group,setNewNoAckTimer);
		} else {
			if (!group->pgroupData->noAckTimerId)
				alForceNoAckGroup(group,setNewNoAckTimer);
		}
		pt = sllNext(pt);
	}
}

