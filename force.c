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
/* force.c */

/************************DESCRIPTION***********************************
  Routines for modifing the forcePV using a popup dialog window
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/PanedW.h>
#include <Xm/Protocols.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>

#include "axArea.h"
#include "alLib.h"
#include "ax.h"

/* global variables */
extern Pixel bg_pixel[ALH_ALARM_NSEV];

struct forcePVWindow {
	void *area;
	Widget menuButton;
	Widget forcePVDialog;
	Widget nameLabelW;
	Widget nameTextW;
	Widget forcePVnameTextW;
	Widget forcePVDisabledToggleButton;
	Widget forcePVmaskStringLabelW;
	Widget forceMaskToggleButtonW[ALARM_NMASK];
	Widget forcePVforceValueTextW;
	Widget forcePVresetValueTextW;
};

/* forward declarations */
static void forcePVApplyCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forcePVCancelCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forcePVDismissCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forcePVHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forcePVCreateDialog(ALINK*area);
static void forcePVUpdateDialogWidgets(struct forcePVWindow *forcePVWindow);
static void forcePVMaskChangeCallback( Widget widget, XtPointer calldata, XtPointer cbs);


/******************************************************
  forcePVUpdateDialog
******************************************************/
void forcePVUpdateDialog(ALINK *area)
{
	struct forcePVWindow *forcePVWindow;

	forcePVWindow = (struct forcePVWindow *)area->forcePVWindow;

	if (!forcePVWindow)  return;

	if (!forcePVWindow->forcePVDialog ||
	    !XtIsManaged(forcePVWindow->forcePVDialog)) return;

	forcePVUpdateDialogWidgets(forcePVWindow);
}

/******************************************************
  forcePVShowDialog
******************************************************/
void forcePVShowDialog(ALINK *area,Widget menuButton)
{
	struct forcePVWindow *forcePVWindow;

	forcePVWindow = (struct forcePVWindow *)area->forcePVWindow;

	/* dismiss Dialog */
	if (forcePVWindow && forcePVWindow->forcePVDialog && 
	    XtIsManaged(forcePVWindow->forcePVDialog)) {
		forcePVDismissCallback(NULL, (XtPointer)forcePVWindow, NULL);
		return;
	}

	/* create forcePVWindow and Dialog Widgets if necessary */
	if (!forcePVWindow)  forcePVCreateDialog(area);

	/* update forcePVWindow link info */
	forcePVWindow = (struct forcePVWindow *)area->forcePVWindow;
	forcePVWindow->menuButton = menuButton;

	/* update Dialog Widgets */
	forcePVUpdateDialogWidgets(forcePVWindow);

	/* show Dialog */
	if (!forcePVWindow->forcePVDialog) return;
	if (!XtIsManaged(forcePVWindow->forcePVDialog)) {
		XtManageChild(forcePVWindow->forcePVDialog);
	}
	XMapWindow(XtDisplay(forcePVWindow->forcePVDialog),
	    XtWindow(XtParent(forcePVWindow->forcePVDialog)));
	if (menuButton) XtVaSetValues(menuButton, XmNset, TRUE, NULL);
}

/******************************************************
  forcePVUpdateDialogWidgets
******************************************************/
static void forcePVUpdateDialogWidgets(struct forcePVWindow *forcePVWindow)
{
	struct gcData *pgcData;
	struct chanData *pcData;
	GCLINK *link;
	int linkType;
	XmString string;
	char buff[MAX_STRING_LENGTH];
	MASK mask;

	if (! forcePVWindow || !forcePVWindow->forcePVDialog ) return;

	link =getSelectionLinkArea(forcePVWindow->area);

	if (!link) {
		string = XmStringCreateSimple("");
		XtVaSetValues(forcePVWindow->nameTextW,XmNlabelString, string, NULL);
		XmStringFree(string);
		return;
	}

	pgcData = link->pgcData;
	linkType =getSelectionLinkTypeArea(forcePVWindow->area);

	/* ---------------------------------
	     Group/Channel Name 
	     --------------------------------- */
	if (linkType == GROUP) string = XmStringCreateSimple("Group Name:");
	else string = XmStringCreateSimple("Channel Name:");
	XtVaSetValues(forcePVWindow->nameLabelW, XmNlabelString, string, NULL);
	XmStringFree(string);

	if (pgcData->alias){
		string = XmStringCreateSimple(pgcData->alias);
	} else {
		string = XmStringCreateSimple(pgcData->name);
	}
	XtVaSetValues(forcePVWindow->nameTextW, XmNlabelString, string, NULL);
	XmStringFree(string);

	if (!link) {
		XmTextFieldSetString(forcePVWindow->forcePVnameTextW,"");
		string = XmStringCreateSimple("-----");
		XtVaSetValues(forcePVWindow->forcePVmaskStringLabelW,
		    XmNlabelString,string,NULL);
		XmStringFree(string);
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[0],
		    FALSE,TRUE);
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[1],
		    FALSE,TRUE);
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[2],
		    FALSE,TRUE);
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[3],
		    FALSE,TRUE);
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[4],
		    FALSE,TRUE);

		XmTextFieldSetString(forcePVWindow->forcePVforceValueTextW,"");
		XmTextFieldSetString(forcePVWindow->forcePVresetValueTextW,"");
		return;
	}

	pgcData = link->pgcData;
	linkType =getSelectionLinkTypeArea(forcePVWindow->area);
	if (linkType == CHANNEL) pcData = (struct chanData *)pgcData;

	/* ---------------------------------
	     Force Process Variable
	     --------------------------------- */
	if(strcmp(pgcData->forcePVName,"-") != 0)
		XmTextFieldSetString(forcePVWindow->forcePVnameTextW,pgcData->forcePVName);
	else XmTextFieldSetString(forcePVWindow->forcePVnameTextW,"");

	XmToggleButtonGadgetSetState(forcePVWindow->forcePVDisabledToggleButton,pgcData->forcePVDisabled,FALSE);

	alGetMaskString(pgcData->forcePVMask,buff);
	string = XmStringCreateSimple(buff);
	XtVaSetValues(forcePVWindow->forcePVmaskStringLabelW, XmNlabelString, string, NULL);
	XmStringFree(string);

	mask = pgcData->forcePVMask;
	if (mask.Cancel == 1 )
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[0],
		    TRUE,TRUE);
	else XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[0],
	    FALSE,TRUE);
	if (mask.Disable == 1 )
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[1],
		    TRUE,TRUE);
	else XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[1],                    FALSE,TRUE);
	if (mask.Ack == 1 )
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[2],
		    TRUE,TRUE);
	else XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[2],
	    FALSE,TRUE);
	if (mask.AckT == 1 )
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[3],
		    TRUE,TRUE);
	else XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[3],
	    FALSE,TRUE);
	if (mask.Log == 1 )
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[4],
		    TRUE,TRUE);
	else XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[4],
	    FALSE,TRUE);

	sprintf(buff,"%d",pgcData->forcePVValue);
	XmTextFieldSetString(forcePVWindow->forcePVforceValueTextW,buff);

	sprintf(buff,"%d",pgcData->resetPVValue);
	XmTextFieldSetString(forcePVWindow->forcePVresetValueTextW,buff);
}

/******************************************************
  forcePVCreateDialog
******************************************************/
static void forcePVCreateDialog(ALINK *area)
{
	struct forcePVWindow *forcePVWindow;

	Widget forcePVDialogShell, forcePVDialog;
	Widget form;
	Widget nameLabelW, nameTextW;
	Widget forcePVDisabledToggleButton;
	Widget forceMaskToggleButtonW[ALARM_NMASK];
	Widget forcePVforceValueLabel,forcePVnameTextW, forcePVforceValueTextW,
	    forcePVresetValueTextW, forcePVresetValueLabel;
	Widget forcePVmaskStringLabelW, frame2, rowcol2, frame3,
	    rowcol3;
	Widget forceMaskLabel, forcePVnameLabel;
	Widget prev;
	int i;
	Pixel textBackground;
	XmString string;
	static ActionAreaItem forcePV_items[] = {
		         { "Apply",   forcePVApplyCallback,   NULL    },
		         { "Cancel",  forcePVCancelCallback,  NULL    },
		         { "Dismiss", forcePVDismissCallback, NULL    },
		         { "Help",    forcePVHelpCallback,    NULL    },
		     	};
	static String maskFields[] = {
		         "Cancel Alarm", 
		         "Disable Alarm",
		         "NoAck Alarm",
		         "NoAck Transient Alarm",
		         "NoLog Alarm"
		     	};

	if (!area) return;

	forcePVWindow = (struct forcePVWindow *)area->forcePVWindow;

	if (forcePVWindow && forcePVWindow->forcePVDialog){
		if (XtIsManaged(forcePVWindow->forcePVDialog)) return;
		else XtManageChild(forcePVWindow->forcePVDialog);
	}

	textBackground = bg_pixel[3];

	forcePVWindow = (struct forcePVWindow *)calloc(1,sizeof(struct forcePVWindow));
	area->forcePVWindow = (void *)forcePVWindow;
	forcePVWindow->area = (void *)area;

	forcePVDialogShell = XtVaCreatePopupShell("Force Process Variable",
	    transientShellWidgetClass, area->toplevel, NULL, 0);

	/* Modify the window manager menu "close" callback */
	{
		Atom         WM_DELETE_WINDOW;
		XtVaSetValues(forcePVDialogShell,
		    XmNdeleteResponse, XmDO_NOTHING, NULL);
		WM_DELETE_WINDOW = XmInternAtom(XtDisplay(forcePVDialogShell),
		    "WM_DELETE_WINDOW", False);
		XmAddWMProtocolCallback(forcePVDialogShell,WM_DELETE_WINDOW,
		    (XtCallbackProc)forcePVDismissCallback, (XtPointer)forcePVWindow);
	}

	forcePVDialog = XtVaCreateWidget("forcePVDialog",
	    xmPanedWindowWidgetClass, forcePVDialogShell,
	    XmNsashWidth,  1,
	    XmNsashHeight, 1,
	    XmNuserData,   area,
	    NULL);

	form = XtVaCreateWidget("control_area", 
	    xmFormWidgetClass, forcePVDialog,
	    NULL);

	/* ---------------------------------
	     Group/Channel Name 
	     --------------------------------- */
	nameLabelW = XtVaCreateManagedWidget("nameLabelW",
	    xmLabelGadgetClass, form,
	    XmNalignment,       XmALIGNMENT_END,
	    XmNtopAttachment,   XmATTACH_FORM,
	    XmNrightAttachment, XmATTACH_POSITION,
	    XmNrightPosition,   50,
	    XmNrecomputeSize,   True,
	    NULL);

	nameTextW = XtVaCreateManagedWidget("nameTextW",
	    xmLabelGadgetClass, form,
	    XmNalignment,       XmALIGNMENT_BEGINNING,
	    XmNtopAttachment,   XmATTACH_FORM,
	    XmNleftAttachment,  XmATTACH_POSITION,
	    XmNleftPosition,    50,
	    XmNrecomputeSize,   True,
	    NULL);


	/* ---------------------------------
	     Force Process Variable
	     --------------------------------- */

	frame2 = XtVaCreateWidget("frame2",
	    xmFrameWidgetClass, form,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       nameLabelW,
	    XmNleftAttachment,  XmATTACH_FORM,
	    XmNrightAttachment,  XmATTACH_FORM,
	    XmNbottomAttachment,  XmATTACH_FORM,
	    NULL);

	rowcol2 = XtVaCreateWidget("rowcol2",
	    xmFormWidgetClass, frame2,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNspacing,          0,
	    XmNmarginHeight,     0,
	    NULL);

    forcePVDisabledToggleButton = XtVaCreateManagedWidget(
		"ForcePV Disabled",
        xmToggleButtonGadgetClass, rowcol2,
        NULL);

	string = XmStringCreateSimple("Force Process Variable Name    ");
	forcePVnameLabel = XtVaCreateManagedWidget("forcePVnameLabel",
	    xmLabelGadgetClass, rowcol2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              forcePVDisabledToggleButton,
		XmNtopOffset,              10,
	    NULL);
	XmStringFree(string);

	forcePVnameTextW = XtVaCreateManagedWidget("forcePVnameTextW",
	    xmTextFieldWidgetClass, rowcol2,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                30,
	    XmNmaxLength,              PVNAME_SIZE,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);

	XtAddCallback(forcePVnameTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	string = XmStringCreateSimple("Force Mask:  ");
	forceMaskLabel = XtVaCreateManagedWidget("forceMaskLabel",
	    xmLabelGadgetClass, rowcol2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameTextW,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);
	prev = forceMaskLabel;

	string = XmStringCreateSimple("-----");
	forcePVmaskStringLabelW = XtVaCreateManagedWidget("forcePVmaskStringLabelW",
	    xmLabelGadgetClass, rowcol2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameTextW,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forceMaskLabel,
	    NULL);
	XmStringFree(string);

	frame3 = XtVaCreateManagedWidget("frame3",
	    xmFrameWidgetClass, rowcol2,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              prev,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	prev = frame3;

	rowcol3 = XtVaCreateWidget("rowcol3",
	    xmRowColumnWidgetClass, frame3,
	    XmNspacing,          0,
	    XmNmarginHeight,     0,
	    NULL);

	for (i = 0; i < ALARM_NMASK; i++){
		forceMaskToggleButtonW[i] = XtVaCreateManagedWidget(maskFields[i],
		    xmToggleButtonGadgetClass, rowcol3,
		    XmNmarginHeight,     0,
		    XmNuserData,         (XtPointer)forcePVmaskStringLabelW,
		    NULL);
		XtAddCallback(forceMaskToggleButtonW[i], XmNvalueChangedCallback,
		    (XtCallbackProc)forcePVMaskChangeCallback, (XtPointer)i);
	}

	XtManageChild(rowcol3);

	string = XmStringCreateSimple("Force Value: ");
	forcePVforceValueLabel = XtVaCreateManagedWidget("forcePVvalue",
	    xmLabelGadgetClass, rowcol2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              prev,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	forcePVforceValueTextW = XtVaCreateManagedWidget("forcePVforceValueTextW",
	    xmTextFieldWidgetClass, rowcol2,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                5,
	    XmNmaxLength,              5,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              prev,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVforceValueLabel,
	    NULL);

	XtAddCallback(forcePVforceValueTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	string = XmStringCreateSimple("Reset Value: ");
	forcePVresetValueLabel = XtVaCreateManagedWidget("forcePVresetValueLabel",
	    xmLabelGadgetClass, rowcol2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVforceValueLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	forcePVresetValueTextW = XtVaCreateManagedWidget("forcePVresetValueTextW",
	    xmTextFieldWidgetClass, rowcol2,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                5,
	    XmNmaxLength,              5,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVforceValueTextW,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVresetValueLabel,
	    NULL);

	XtAddCallback(forcePVresetValueTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	/* Set the client data "Apply", "Cancel", "Dismiss" and "Help" button's callbacks. */
	forcePV_items[0].data = (XtPointer)forcePVWindow;
	forcePV_items[1].data = (XtPointer)forcePVWindow;
	forcePV_items[2].data = (XtPointer)forcePVWindow;
	forcePV_items[3].data = (XtPointer)forcePVWindow;

	(void)createActionButtons(forcePVDialog, forcePV_items, XtNumber(forcePV_items));

	forcePVWindow->forcePVDialog = forcePVDialog;
	forcePVWindow->nameLabelW = nameLabelW;
	forcePVWindow->nameTextW = nameTextW;
	forcePVWindow->forcePVDisabledToggleButton = forcePVDisabledToggleButton;
	forcePVWindow->forcePVnameTextW = forcePVnameTextW;
	forcePVWindow->forcePVmaskStringLabelW = forcePVmaskStringLabelW;
	forcePVWindow->forcePVforceValueTextW = forcePVforceValueTextW;
	forcePVWindow->forcePVresetValueTextW = forcePVresetValueTextW;
	for (i = 0; i < ALARM_NMASK; i++){
		forcePVWindow->forceMaskToggleButtonW[i] = forceMaskToggleButtonW[i];
	}

	/* update forcePVWindow link info */
	forcePVUpdateDialogWidgets(forcePVWindow);

	/* RowColumn is full -- now manage */
	XtManageChild(rowcol2);
	XtManageChild(frame2);
	XtManageChild(form);

	XtManageChild(forcePVDialog);

	XtRealizeWidget(forcePVDialogShell);

}

/******************************************************
  forcePVMaskChangeCallback
******************************************************/
static void forcePVMaskChangeCallback(Widget widget, XtPointer calldata,
XtPointer cbs)
{
	int index=(int)calldata;
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
	free(mask);
}

/******************************************************
  forcePVApplyCallback
******************************************************/
static void forcePVApplyCallback(Widget widget, XtPointer calldata,
XtPointer cbs)
{
	struct forcePVWindow *forcePVWindow= (struct forcePVWindow *)calldata;
	short f1;
	int rtn;
	XmString string;
	char *buff;
	struct gcData *pgcData;
	GCLINK *link;
	int linkType;

	link =getSelectionLinkArea(forcePVWindow->area);
	if (!link) return;
	linkType =getSelectionLinkTypeArea(forcePVWindow->area);
	pgcData = link->pgcData;

	/* ---------------------------------
	     Force Process Variable
	     --------------------------------- */
	/*  update link field  - forcePVMask */
	XtVaGetValues(forcePVWindow->forcePVmaskStringLabelW, XmNlabelString, &string, NULL);
	XmStringGetLtoR(string,XmFONTLIST_DEFAULT_TAG,&buff);
	XmStringFree(string);
	alSetMask(buff,&(pgcData->forcePVMask));
	XtFree(buff);

	/*  update link field  - forcePVValue */
	buff = XmTextFieldGetString(forcePVWindow->forcePVforceValueTextW);
	rtn = sscanf(buff,"%hd",&f1);
	if (rtn == 1) pgcData->forcePVValue = f1;
	else pgcData->forcePVValue = 1;
	XtFree(buff);

	/*  update link field  - resetPVValue */
	buff = XmTextFieldGetString(forcePVWindow->forcePVresetValueTextW);
	rtn = sscanf(buff,"%hd",&f1);
	if (rtn == 1) pgcData->resetPVValue = f1;
	else pgcData->resetPVValue = 0;
	XtFree(buff);

	/*  update disabled field  - forcePVDisabled */
    pgcData->forcePVDisabled = XmToggleButtonGadgetGetState(forcePVWindow->forcePVDisabledToggleButton);

	/*  update link field  - forcePVName */
	buff = XmTextFieldGetString(forcePVWindow->forcePVnameTextW);
	if (strlen(buff) > (size_t)1 && strcmp(buff,pgcData->forcePVName) != 0) {
		if (linkType == GROUP) alReplaceGroupForceEvent((GLINK *)link,buff);
		else alReplaceChanForceEvent((CLINK *)link,buff);
	}
	XtFree(buff);

	/*  log on operation file */
	if (linkType == GROUP) alLogForcePVGroup((GLINK *)link,OPERATOR);
	else alLogForcePVChan((CLINK *)link,OPERATOR);


	/* ---------------------------------
	     Update dialog windows
	     --------------------------------- */
	/*  update forcePVerties dialog window field */
	axUpdateDialogs(forcePVWindow->area);

}

/******************************************************
  forcePVHelpCallback
******************************************************/
static void forcePVHelpCallback(Widget widget, XtPointer calldata,
XtPointer cbs)
{
	char *message1 =
	"This dialog window allows an operator to specify"
	" or modify the forcePV\n"
	"values for a selected group or channel.\n"
	"  \n"
	"NOTE: The force value must be set to a value"
	" different from the reset value.\n"
	"  \n"
	"Press the Apply   button to change the forcePV"
	" values for the group or channel.\n"
	"Press the Cancel button to abort current change.\n"
	"Press the Dismiss button to close the Force PV dialog window.\n"
	"Press the Help    button to get this help description window.\n"
	;
	char *message2 = "  ";

	createDialog(widget,XmDIALOG_INFORMATION, message1,message2);
}

/******************************************************
  forcePVDismissCallback
******************************************************/
static void forcePVDismissCallback(Widget widget, XtPointer calldata,
XtPointer cbs)
{
	struct forcePVWindow *forcePVWindow= (struct forcePVWindow *)calldata;
	Widget forcePVDialog;

	forcePVDialog = forcePVWindow->forcePVDialog;
	XtUnmanageChild(forcePVDialog);
	XUnmapWindow(XtDisplay(forcePVDialog), XtWindow(XtParent(forcePVDialog)));
	if (forcePVWindow->menuButton)
		XtVaSetValues(forcePVWindow->menuButton, XmNset, FALSE, NULL);
}

/******************************************************
  forcePVCancelCallback
******************************************************/
static void forcePVCancelCallback(Widget widget, XtPointer calldata,
XtPointer cbs)
{
	struct forcePVWindow *forcePVWindow= (struct forcePVWindow *)calldata;
	forcePVUpdateDialog((ALINK *)(forcePVWindow->area));
}


