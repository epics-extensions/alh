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
/* showmask.c */

/************************DESCRIPTION***********************************
  showmask.c: a popup dialog  window
  This file contains routines for creating force mask dialog.
**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
/* If not MS Visual C++ or MS Visual C++ is 2010 or later  */
#if !defined(_MSC_VER) || _MSC_VER >= 1600
#include <stdint.h>
#endif

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

extern int _passive_flag;

struct forceMaskWindow {
	void   *area;
	Widget menuButton;
	Widget maskDialog;
	Widget nameLabelW;
	Widget nameTextW;
	Widget currentMaskLabel;
	Widget currentMaskStringLabelW;
	Widget resetMaskStringLabelW;
	Widget alarmMaskStringLabelW;
	Widget alarmMaskToggleButtonW[ALARM_NMASK];
	Widget maskFrameW;
};

/* forward declarations */
static void forceMaskApplyCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forceMaskResetCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forceMaskDismissCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forceMaskHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forceMaskCreateDialog(ALINK*area);
static void forceMaskUpdateDialogWidgets(struct forceMaskWindow *forceMaskWindow);
static void forceMaskChangeCallback( Widget widget,XtPointer calldata,XtPointer cbs);

/******************************************************
  forceMaskUpdateDialog
******************************************************/
void forceMaskUpdateDialog(area)
ALINK  *area;
{
	struct forceMaskWindow *forceMaskWindow;

	if (!area->forceMaskWindow)  return;

	forceMaskWindow = (struct forceMaskWindow *)area->forceMaskWindow;

	if (!forceMaskWindow->maskDialog || !XtIsManaged(forceMaskWindow->maskDialog)) return;

	forceMaskUpdateDialogWidgets(forceMaskWindow);
}

/******************************************************
  forceMaskShowDialog
******************************************************/
void forceMaskShowDialog(area, menuButton)
ALINK    *area;
Widget   menuButton;
{
	struct forceMaskWindow *forceMaskWindow;

	forceMaskWindow = (struct forceMaskWindow *)area->forceMaskWindow;

	/* dismiss Dialog */
	if (forceMaskWindow && forceMaskWindow->maskDialog && 
	    XtIsManaged(forceMaskWindow->maskDialog)) {
		forceMaskDismissCallback(NULL, (XtPointer)forceMaskWindow, NULL);
		return;
	}

	/* create forceMaskWindow and Dialog Widgets if necessary */
	if (!forceMaskWindow)  forceMaskCreateDialog(area);

	/* update forceMaskWindow */
	forceMaskWindow = (struct forceMaskWindow *)area->forceMaskWindow;
	forceMaskWindow->menuButton = menuButton;

	/* update Dialog Widgets */
	forceMaskUpdateDialogWidgets(forceMaskWindow);

	/* show Dialog */
	if (!forceMaskWindow->maskDialog) return;
	if (!XtIsManaged(forceMaskWindow->maskDialog)) {
		XtManageChild(forceMaskWindow->maskDialog);
	}
	XMapWindow(XtDisplay(forceMaskWindow->maskDialog),
	    XtWindow(XtParent(forceMaskWindow->maskDialog)));
	if (menuButton) XtVaSetValues(menuButton, XmNset, TRUE, NULL);

}

/******************************************************
  forceMaskUpdateDialogWidgets
******************************************************/

static void forceMaskUpdateDialogWidgets(forceMaskWindow)
struct forceMaskWindow *forceMaskWindow;
{
	struct gcData *pgcData;
	GCLINK *link;
	int linkType;
	XmString string;
	char buff[6];

	if (! forceMaskWindow || !forceMaskWindow->maskDialog  ) return;

	link = (GCLINK *)getSelectionLinkArea(forceMaskWindow->area);

	if (!link) {

		string = XmStringCreateSimple("");
		XtVaSetValues(forceMaskWindow->nameTextW,XmNlabelString, string, NULL);
		XmStringFree(string);
		string = XmStringCreateSimple("-----");
		XtVaSetValues(forceMaskWindow->currentMaskStringLabelW, XmNlabelString, string, NULL);
		XtVaSetValues(forceMaskWindow->resetMaskStringLabelW, XmNlabelString, string, NULL);
		XtVaSetValues(forceMaskWindow->alarmMaskStringLabelW, XmNlabelString, string, NULL);
		XmStringFree(string);
		XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[0],FALSE,TRUE);
		XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[1],FALSE,TRUE);
		XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[2],FALSE,TRUE);
		XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[3],FALSE,TRUE);
		XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[4],FALSE,TRUE);
		return;
	}

	linkType = getSelectionLinkTypeArea(forceMaskWindow->area);

	pgcData = link->pgcData;

	/* ---------------------------------
	     Group/Channel Name 
	     --------------------------------- */
	if (linkType == GROUP) string = XmStringCreateSimple("Group Name:");
	else string = XmStringCreateSimple("Channel Name:");
	XtVaSetValues(forceMaskWindow->nameLabelW, XmNlabelString, string, NULL);
	XmStringFree(string);

	if (pgcData->alias){
		string = XmStringCreateSimple(pgcData->alias);
	} else {
		string = XmStringCreateSimple(pgcData->name);
	}
	XtVaSetValues(forceMaskWindow->nameTextW, XmNlabelString, string, NULL);
	XmStringFree(string);

	/* ---------------------------------
	     Current Mask 
	     --------------------------------- */
	if (linkType == GROUP) string = XmStringCreateSimple("Current Mask Summary:");
	else string = XmStringCreateSimple("Current Mask:");
	XtVaSetValues(forceMaskWindow->currentMaskLabel, XmNlabelString, string, NULL);
	XmStringFree(string);

	if (linkType == GROUP) awGetMaskString(((struct groupData *)pgcData)->mask,buff);
	else alGetMaskString(((struct chanData *)pgcData)->curMask,buff);
	string = XmStringCreateSimple(buff);
	XtVaSetValues(forceMaskWindow->currentMaskStringLabelW, XmNlabelString, string, NULL);

	/*
	     XtVaSetValues(forceMaskWindow->alarmMaskStringLabelW, XmNlabelString, string, NULL);
	     alSetMask(buff,&mask);
	     if (mask.Cancel == 1 )
	          XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[0],TRUE,TRUE);
	     else XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[0],FALSE,TRUE);
	     if (mask.Disable == 1 )
	          XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[1],TRUE,TRUE);
	     else XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[1],FALSE,TRUE);
	     if (mask.Ack == 1 )
	          XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[2],TRUE,TRUE);
	     else XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[2],FALSE,TRUE);
	     if (mask.AckT == 1 )
	          XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[3],TRUE,TRUE);
	     else XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[3],FALSE,TRUE);
	     if (mask.Log == 1 )
	          XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[4],TRUE,TRUE);
	     else XmToggleButtonSetState(forceMaskWindow->alarmMaskToggleButtonW[4],FALSE,TRUE);
	*/

	XmStringFree(string);

	/* ---------------------------------
	     Reset Mask 
	     --------------------------------- */
	if (linkType == GROUP) string = XmStringCreateSimple("   ");
	else {
		alGetMaskString(((struct chanData *)pgcData)->defaultMask,buff);
		string = XmStringCreateSimple(buff);
	}
	XtVaSetValues(forceMaskWindow->resetMaskStringLabelW, XmNlabelString, string, NULL);
	XmStringFree(string);

}

/******************************************************
  forceMaskCreateDialog
******************************************************/
static void forceMaskCreateDialog(area)
ALINK    *area;
{
	struct forceMaskWindow *forceMaskWindow;

	Widget maskDialogShell, maskDialog;
	Widget rowcol, form, maskFrameW;
	Widget nameLabelW, nameTextW;
	Widget alarmMaskToggleButtonW[ALARM_NMASK];
	Widget alarmMaskLabel, alarmMaskStringLabelW;
	Widget currentMaskLabel, currentMaskStringLabelW;
	Widget resetMaskLabel, resetMaskStringLabelW;
	int i;
        intptr_t iptr;
	XmString string;
	static ActionAreaItem mask_items[] = {
		         { "Apply",   forceMaskApplyCallback,   NULL    },
		         { "Reset",   forceMaskResetCallback,   NULL    },
		         { "Dismiss", forceMaskDismissCallback, NULL    },
		         { "Help",    forceMaskHelpCallback,    NULL    },
		     	};
	static String maskFields[] = {
		         "Cancel Alarm", 
		         "Disable Alarm",
		         "NoAck Alarm",
		         "NoAck Transient Alarm",
		         "NoLog Alarm"
		     	};

	if (!area) return;

	forceMaskWindow = (struct forceMaskWindow *)area->forceMaskWindow;

	if (forceMaskWindow && forceMaskWindow->maskDialog){
		if (XtIsManaged(forceMaskWindow->maskDialog)) return;
		else XtManageChild(forceMaskWindow->maskDialog);
	}


	forceMaskWindow = (struct forceMaskWindow *)calloc(1,sizeof(struct forceMaskWindow));
	area->forceMaskWindow = (void *)forceMaskWindow;
	forceMaskWindow->area = (void *)area;

	maskDialogShell = XtVaCreatePopupShell("Force Mask",
	    transientShellWidgetClass, area->toplevel, 
	    XmNallowShellResize, TRUE,
	    NULL);

	/* Modify the window manager menu "close" callback */
	{
		Atom         WM_DELETE_WINDOW;
		XtVaSetValues(maskDialogShell,
		    XmNdeleteResponse, XmDO_NOTHING, NULL);
		WM_DELETE_WINDOW = XmInternAtom(XtDisplay(maskDialogShell),
		    "WM_DELETE_WINDOW", False);
		XmAddWMProtocolCallback(maskDialogShell,WM_DELETE_WINDOW,
		    (XtCallbackProc)forceMaskDismissCallback, (XtPointer)forceMaskWindow);
	}

	maskDialog = XtVaCreateWidget("maskDialog",
	    xmPanedWindowWidgetClass, maskDialogShell,
	    XmNallowResize, TRUE,
	    XmNsashWidth,  1,
	    XmNsashHeight, 1,
	    XmNuserData,   area,
	    NULL);

	form = XtVaCreateWidget("control_area",
	    xmFormWidgetClass, maskDialog,
	    XmNallowResize,    TRUE,
	    NULL);

	/* ---------------------------------
	     Group/Channel Name 
	     --------------------------------- */
	nameLabelW = XtVaCreateManagedWidget("nameLabelW",
	    xmLabelGadgetClass, form,
	    XmNalignment,       XmALIGNMENT_END,
	    XmNtopAttachment,   XmATTACH_FORM,
	    XmNleftAttachment,  XmATTACH_FORM,
	    XmNrecomputeSize,   True,
	    NULL);

	nameTextW = XtVaCreateManagedWidget("nameTextW",
	    xmLabelGadgetClass, form,
	    XmNalignment,       XmALIGNMENT_BEGINNING,
	    XmNtopAttachment,   XmATTACH_FORM,
	    XmNleftAttachment,  XmATTACH_WIDGET,
	    XmNleftWidget,      nameLabelW,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNrecomputeSize,   True,
	    NULL);

	/* ---------------------------------
	     Alarm Mask 
	     --------------------------------- */
	string = XmStringCreateSimple("Current Mask:");
	currentMaskLabel = XtVaCreateManagedWidget("currentMaskLabel",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    XmNalignment,       XmALIGNMENT_END,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       nameTextW,
	    XmNtopOffset,       5,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition,   50,
	    NULL);
	XmStringFree(string);

	string = XmStringCreateSimple("-----");
	currentMaskStringLabelW = XtVaCreateManagedWidget("currentMaskStringLabelW",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    /*XmNbackground,      0,*/
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       nameTextW,
	    XmNtopOffset,       5,
	    XmNleftAttachment,  XmATTACH_WIDGET,
	    XmNleftWidget,      currentMaskLabel,
	    NULL);
	XmStringFree(string);

	string = XmStringCreateSimple("Reset Mask:");
	resetMaskLabel = XtVaCreateManagedWidget("resetMaskLabel",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    XmNalignment,       XmALIGNMENT_END,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       currentMaskLabel,
	    XmNtopOffset,       5,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition,   50,
	    NULL);
	XmStringFree(string);

	string = XmStringCreateSimple("-----");
	resetMaskStringLabelW = XtVaCreateManagedWidget("resetMaskStringLabelW",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       currentMaskStringLabelW,
	    XmNtopOffset,       5,
	    XmNleftAttachment,  XmATTACH_WIDGET,
	    XmNleftWidget,      resetMaskLabel,
	    NULL);
	XmStringFree(string);

	string = XmStringCreateSimple("Mask:");
	alarmMaskLabel = XtVaCreateManagedWidget("alarmMaskLabel",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    XmNalignment,       XmALIGNMENT_END,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       resetMaskLabel,
	    XmNtopOffset,       5,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition,   50,
	    NULL);
	XmStringFree(string);

	string = XmStringCreateSimple("-----");
	alarmMaskStringLabelW = XtVaCreateManagedWidget("alarmMaskStringLabelW",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       resetMaskStringLabelW,
	    XmNtopOffset,       5,
	    XmNleftAttachment,  XmATTACH_WIDGET,
	    XmNleftWidget,      alarmMaskLabel,
	    NULL);
	XmStringFree(string);

	maskFrameW = XtVaCreateManagedWidget("maskFrameW",
	    xmFrameWidgetClass, form,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       alarmMaskStringLabelW,
	    XmNtopOffset,       5,
	    XmNleftAttachment, XmATTACH_POSITION,
	    XmNleftPosition,   25,
	    NULL);

	rowcol = XtVaCreateWidget("rowcol",
	    xmRowColumnWidgetClass, maskFrameW,
	    XmNspacing,          0,
	    XmNmarginHeight,     0,
	    NULL);

	for (i = 0; i < ALARM_NMASK; i++){
		alarmMaskToggleButtonW[i] = XtVaCreateManagedWidget(maskFields[i],
		    xmToggleButtonGadgetClass, rowcol,
		    XmNmarginHeight,     0,
		    XmNuserData,         (XtPointer)alarmMaskStringLabelW,
		    NULL);
		if (_passive_flag && i == ALARMACKT ) {
			XtVaSetValues(alarmMaskToggleButtonW[i], XmNsensitive, FALSE, NULL);
		}
                iptr=i;
		XtAddCallback(alarmMaskToggleButtonW[i], XmNvalueChangedCallback,
		    (XtCallbackProc)forceMaskChangeCallback, (XtPointer)iptr);
	}

	XtManageChild(rowcol);

	XtManageChild(form);

	/* Set the client data "Apply", "Reset", "Dismiss", and "Help" button's callbacks. */
	mask_items[0].data = (XtPointer)forceMaskWindow;
	mask_items[1].data = (XtPointer)forceMaskWindow;
	mask_items[2].data = (XtPointer)forceMaskWindow;
	mask_items[3].data = (XtPointer)forceMaskWindow;

	(void)createActionButtons(maskDialog, mask_items, XtNumber(mask_items));

	XtManageChild(maskDialog);

	forceMaskWindow->maskDialog = maskDialog;
	forceMaskWindow->nameLabelW = nameLabelW;
	forceMaskWindow->nameTextW = nameTextW;
	forceMaskWindow->currentMaskLabel = currentMaskLabel;
	forceMaskWindow->currentMaskStringLabelW = currentMaskStringLabelW;
	forceMaskWindow->resetMaskStringLabelW = resetMaskStringLabelW;
	forceMaskWindow->alarmMaskStringLabelW = alarmMaskStringLabelW;
	forceMaskWindow->maskFrameW = maskFrameW;
	for (i = 0; i < ALARM_NMASK; i++){
		forceMaskWindow->alarmMaskToggleButtonW[i] = alarmMaskToggleButtonW[i];
	}

	XtRealizeWidget(maskDialogShell);

}

/******************************************************
  forceMaskChangeCallback
******************************************************/
static void forceMaskChangeCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	long index=(long)calldata;
	char *mask;
	Widget maskWidget;
	XmString string;

	XtVaGetValues(widget, XmNuserData, &maskWidget, NULL);

	XtVaGetValues(maskWidget, XmNlabelString, &string, NULL);
	XmStringGetLtoR(string, XmFONTLIST_DEFAULT_TAG, &mask);
	XmStringFree(string);

	if (!XmToggleButtonGadgetGetState(widget)) {
		mask[index] = '-';
	}
	else {
		switch (index) {
		case ALARMCANCEL:
			mask[index] = 'C';
			break;
		case ALARMDISABLE:
			mask[index] = 'D';
			break;
		case ALARMACK:
			mask[index] = 'A';
			break;
		case ALARMACKT:
			mask[index] = 'T';
			break;
		case ALARMLOG:
			mask[index] = 'L';
			break;
		}
	}

	string = XmStringCreateSimple(mask);
	XtVaSetValues(maskWidget, XmNlabelString, string, NULL);
	XmStringFree(string);
	XtFree(mask);
}

/******************************************************
  forceMaskHelpCallback
******************************************************/
static void forceMaskHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	char *message1 = 
	"Set the current mask for a group or channel.\n"
	"Setting the mask for a group means setting the mask for all channels in the group.\n\n"
    "Changing NoAck Transient Alarm is not allowed when executing in passive state.\n \n"
	"Press the Apply   button to force the mask on the selected channel or on\n"
	"                  all channels in the selected group.\n"
	;
	char *message2 =
	"Press the Reset   button to reset channel mask(s) to their initial values.\n"
	"Press the Dismiss button to close the Force Mask dialog window.\n"
	"Press the Help    button to get this help description window.\n";
	;

	createDialog(widget,XmDIALOG_INFORMATION, message1,message2);

}

/******************************************************
  forceMaskApplyCallback
******************************************************/
static void forceMaskApplyCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	struct forceMaskWindow *forceMaskWindow=(struct forceMaskWindow *)calldata;
	struct chanData *cdata;
	XmString string;
	char *buff;
	struct gcData *pgcData;
	GCLINK *link;
	int linkType;
	MASK mask;

	link =getSelectionLinkArea(forceMaskWindow->area);
	if (!link) return;
	pgcData = link->pgcData;
	linkType =getSelectionLinkTypeArea(forceMaskWindow->area);

	/* ---------------------------------
	     Update alarm Mask 
	     --------------------------------- */
	XtVaGetValues(forceMaskWindow->alarmMaskStringLabelW, XmNlabelString, &string, NULL);
	XmStringGetLtoR(string,XmFONTLIST_DEFAULT_TAG,&buff);
	XmStringFree(string);
	alSetMask(buff,&mask);
	if (linkType == CHANNEL) {
		alRemoveNoAck1HrTimerChan((CLINK *)link);
		alChangeChanMask((CLINK *)link,mask);
		alCaFlushIo();

		alLogOpModMessage(CHANGE_MASK,(GCLINK*)link,
			"OPER Channel Set Mask <%s> [%s]",
			link->pgcData->name,
			buff);

		cdata = (struct chanData *)pgcData;
		if (programId != ALH) cdata->defaultMask = cdata->curMask;
	}
	if (linkType == GROUP) {
		alRemoveNoAck1HrTimerGroup((GLINK *)link);
		alChangeGroupMask((GLINK *)link,mask);
		alCaFlushIo();

		alLogOpModMessage(CHANGE_MASK_GROUP,link,
			"OPER Group Set Mask <%s> [%s]",
			link->pgcData->name,
			buff);
	}
	XtFree(buff);

	silenceCurrentReset(forceMaskWindow->area);
	link->pmainGroup->modified = 1;

	/* ---------------------------------
	     Update all dialog Windows
	     --------------------------------- */
	axUpdateDialogs(forceMaskWindow->area);
}

/******************************************************
  forceMaskResetCallback
******************************************************/
static void forceMaskResetCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	struct forceMaskWindow *forceMaskWindow=(struct forceMaskWindow *)calldata;
	struct chanData *cdata;
	GCLINK *link;
	int linkType;
	char buff1[6];

	link =getSelectionLinkArea(forceMaskWindow->area);
	if (!link) return;
	linkType =getSelectionLinkTypeArea(forceMaskWindow->area);

	if (linkType == CHANNEL) {
		cdata = (struct chanData *)link->pgcData;
		alRemoveNoAck1HrTimerChan((CLINK *)link);
		alChangeChanMask((CLINK *)link,cdata->defaultMask);
		alCaFlushIo();

		alGetMaskString(cdata->curMask,buff1);
		alLogOpModMessage(CHANGE_MASK,(GCLINK*)link,
			"OPER Channel Reset Mask <%s> [%s]",
			cdata->name,
			buff1);

		if (programId != ALH) cdata->defaultMask = cdata->curMask;
	}
	if (linkType == GROUP) {
		alRemoveNoAck1HrTimerGroup((GLINK *)link);
		alResetGroupMask((GLINK *)link);
		alCaFlushIo();

		alLogOpModMessage(CHANGE_MASK_GROUP,link,
			"OPER Group Reset Masks <%s>",
			link->pgcData->name);
	}

	silenceCurrentReset(forceMaskWindow->area);
	link->pmainGroup->modified = 1;

	/* ---------------------------------
	     Update all dialog Windows
	     --------------------------------- */
	axUpdateDialogs(forceMaskWindow->area);

}

/******************************************************
  forceMaskDismissCallback
******************************************************/
static void forceMaskDismissCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	struct forceMaskWindow *forceMaskWindow=(struct forceMaskWindow *)calldata;
	Widget maskDialog;

	maskDialog = forceMaskWindow->maskDialog;
	XtUnmanageChild(maskDialog);
	XUnmapWindow(XtDisplay(maskDialog), XtWindow(XtParent(maskDialog)));
	if (forceMaskWindow->menuButton)
		XtVaSetValues(forceMaskWindow->menuButton, XmNset, FALSE, NULL);
}

