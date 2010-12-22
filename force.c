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

#include <stdlib.h>
#include <ctype.h>

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

#include "postfix.h"
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
	Widget forcePVdisabledToggleButton;
	Widget forcePVmaskStringLabelW;
	Widget forceMaskToggleButtonW[ALARM_NMASK];
	Widget forcePVforceValueTextW;
	Widget forcePVresetValueTextW;
    Widget forcePVCalcExpressionTextW;
    Widget forcePVCalcPVTextW[NO_OF_CALC_PVS];
};

/* forward declarations */
static void forcePVApplyCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forcePVCancelCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forcePVDismissCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forcePVHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void forcePVCreateDialog(ALINK*area);
static void forcePVUpdateDialogWidgets(struct forcePVWindow *forcePVWindow);
static void forcePVMaskChangeCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void forcePVUpdateFields(GCLINK* gclink,FORCEPV* pfPV,int context);
static void forcePVCalcPerform(GCLINK* gclink,int linktype,int forcePVChanged);
static void forcePVNewValueEvent(GCLINK* gclink,short linktype,double value);


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
	GCLINK *link;
	int i,linkType;
	XmString string,oldString;
	char buff[MAX_STRING_LENGTH];
	char buff1[MAX_STRING_LENGTH];
	MASK mask;
	Boolean sameName;

	if (! forcePVWindow || !forcePVWindow->forcePVDialog ) return;

	link =getSelectionLinkArea(forcePVWindow->area);

	if (!link) {
		string = XmStringCreateSimple("");
		XtVaSetValues(forcePVWindow->nameTextW,XmNlabelString, string, NULL);
		XmStringFree(string);
		return;
	} else {
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
			if (pgcData->name) string = XmStringCreateSimple(pgcData->name);
		}
	}
	XtVaGetValues(forcePVWindow->nameTextW, XmNlabelString, &oldString, NULL);
	XtVaSetValues(forcePVWindow->nameTextW, XmNlabelString, string, NULL);
	sameName = XmStringCompare(string, oldString);
	XmStringFree(string);
	XmStringFree(oldString);

	if (!link || !pgcData->pforcePV || !sameName ) {
		XmToggleButtonGadgetSetState(forcePVWindow->forcePVdisabledToggleButton,
			FALSE,FALSE);
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
        XmTextFieldSetString(forcePVWindow->forcePVCalcExpressionTextW,"");
        for (i=0;i<NO_OF_CALC_PVS;i++){
            XmTextFieldSetString(forcePVWindow->forcePVCalcPVTextW[i],"");
        }
		if (!link || !pgcData->pforcePV) return;
	}

	/* ---------------------------------
	     Force Process Variable
	     --------------------------------- */
	if(pgcData->pforcePV->name)
		XmTextFieldSetString(forcePVWindow->forcePVnameTextW,pgcData->pforcePV->name);
	else XmTextFieldSetString(forcePVWindow->forcePVnameTextW,"");

	XmToggleButtonGadgetSetState(forcePVWindow->forcePVdisabledToggleButton,
		(Boolean)pgcData->pforcePV->disabled,FALSE);

	alGetMaskString(pgcData->pforcePV->forceMask,buff);
	string = XmStringCreateSimple(buff);
	XtVaSetValues(forcePVWindow->forcePVmaskStringLabelW, XmNlabelString, string, NULL);
	XmStringFree(string);

	mask = pgcData->pforcePV->forceMask;
	if (mask.Cancel == 1 )
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[0],
		    TRUE,TRUE);
	else XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[0],
	    FALSE,TRUE);
	if (mask.Disable == 1 )
		XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[1],
		    TRUE,TRUE);
	else XmToggleButtonSetState(forcePVWindow->forceMaskToggleButtonW[1],
			FALSE,TRUE);
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

	sprintf(buff,"%g",pgcData->pforcePV->forceValue);
	XmTextFieldSetString(forcePVWindow->forcePVforceValueTextW,buff);

	sprintf(buff1,"%g",pgcData->pforcePV->resetValue);
	if (!strcmp(buff,buff1)) sprintf(buff,"%s","NE");
	else sprintf(buff,"%g",pgcData->pforcePV->resetValue);
	XmTextFieldSetString(forcePVWindow->forcePVresetValueTextW,buff);

	if(pgcData->pforcePV->pcalc){
		if (pgcData->pforcePV->pcalc->expression)
			XmTextFieldSetString(forcePVWindow->forcePVCalcExpressionTextW,
				pgcData->pforcePV->pcalc->expression);
		else XmTextFieldSetString(forcePVWindow->forcePVCalcExpressionTextW,"");
		for (i=0;i<NO_OF_CALC_PVS;i++){
			if (pgcData->pforcePV->pcalc->name[i])
				XmTextFieldSetString(forcePVWindow->forcePVCalcPVTextW[i],
					pgcData->pforcePV->pcalc->name[i]);
			else XmTextFieldSetString(forcePVWindow->forcePVCalcPVTextW[i],"");
		}
	} else {
        XmTextFieldSetString(forcePVWindow->forcePVCalcExpressionTextW,"");
        for (i=0;i<NO_OF_CALC_PVS;i++){
            XmTextFieldSetString(forcePVWindow->forcePVCalcPVTextW[i],"");
        }
	}
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
	Widget forcePVdisabledToggleButton;
	Widget forceMaskToggleButtonW[ALARM_NMASK];
	Widget forcePVforceValueLabel,forcePVnameTextW, forcePVforceValueTextW,
	    forcePVresetValueTextW, forcePVresetValueLabel;
	Widget forcePVmaskStringLabelW, frame2, form2, frame3,
	    rowcol3;
	Widget forceMaskLabel, forcePVnameLabel;
	Widget prev;
	Widget frame4, form3, forcePVCalcLabel,forcePVCalcExpressionLabel;
	Widget forcePVCalcExpressionTextW;
	Widget forcePVCalcPVLabel[NO_OF_CALC_PVS], forcePVCalcPVTextW[NO_OF_CALC_PVS];
	int i;
	Pixel textBackground;
	XmString string;
    char letter[]={"ABCDEF"};
	char pvid[]="A ";
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
	    transientShellWidgetClass, area->toplevel, NULL);

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
	    (XtPointer)NULL);

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

	form2 = XtVaCreateWidget("form2",
	    xmFormWidgetClass, frame2,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNspacing,          0,
	    XmNmarginHeight,     0,
	    NULL);

    forcePVdisabledToggleButton = XtVaCreateManagedWidget(
		"ForcePV Disabled",
        xmToggleButtonGadgetClass, form2,
        NULL);

	string = XmStringCreateSimple("Force Process Variable Name (or CALC):");
	forcePVnameLabel = XtVaCreateManagedWidget("forcePVnameLabel",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              forcePVdisabledToggleButton,
		XmNtopOffset,              10,
	    NULL);
	XmStringFree(string);

	forcePVnameTextW = XtVaCreateManagedWidget("forcePVnameTextW",
	    xmTextFieldWidgetClass, form2,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                30,
	    XmNmaxLength,              PVNAME_SIZE,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    (XtPointer)NULL);

	XtAddCallback(forcePVnameTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	string = XmStringCreateSimple("Force Mask ");
	forceMaskLabel = XtVaCreateManagedWidget("forceMaskLabel",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameTextW,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);
	prev = forceMaskLabel;

	string = XmStringCreateSimple("-----");
	forcePVmaskStringLabelW = XtVaCreateManagedWidget("forcePVmaskStringLabelW",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameTextW,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forceMaskLabel,
	    NULL);
	XmStringFree(string);

	frame3 = XtVaCreateManagedWidget("frame3",
	    xmFrameWidgetClass, form2,
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
		long ilong=i;
		forceMaskToggleButtonW[i] = XtVaCreateManagedWidget(maskFields[i],
		    xmToggleButtonGadgetClass, rowcol3,
		    XmNmarginHeight,     0,
		    XmNuserData,         (XtPointer)forcePVmaskStringLabelW,
		    NULL);
		XtAddCallback(forceMaskToggleButtonW[i], XmNvalueChangedCallback,
		    (XtCallbackProc)forcePVMaskChangeCallback, (XtPointer)ilong);
	}

	XtManageChild(rowcol3);

	string = XmStringCreateSimple("Force Value ");
	forcePVforceValueLabel = XtVaCreateManagedWidget("forcePVvalue",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              prev,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	forcePVforceValueTextW = XtVaCreateManagedWidget("forcePVforceValueTextW",
	    xmTextFieldWidgetClass, form2,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                8,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              prev,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVforceValueLabel,
	    NULL);

	XtAddCallback(forcePVforceValueTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	string = XmStringCreateSimple("Reset Value ");
	forcePVresetValueLabel = XtVaCreateManagedWidget("forcePVresetValueLabel",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVforceValueLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	forcePVresetValueTextW = XtVaCreateManagedWidget("forcePVresetValueTextW",
	    xmTextFieldWidgetClass, form2,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                8,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVforceValueTextW,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVresetValueLabel,
	    NULL);

	XtAddCallback(forcePVresetValueTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	frame4 = XtVaCreateManagedWidget("frame4",
	    xmFrameWidgetClass, form2,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       forcePVresetValueTextW,
	    XmNleftAttachment,  XmATTACH_FORM,
	    XmNrightAttachment,  XmATTACH_FORM,
	    NULL);

	form3 = XtVaCreateWidget("form3",
	    xmFormWidgetClass, frame4,
	    XmNspacing,          0,
	    XmNmarginHeight,     0,
	    NULL);

	string = XmStringCreateSimple("Force CALC");
	forcePVCalcLabel = XtVaCreateManagedWidget("forcePVCalcLabel",
	    xmLabelGadgetClass, form3,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVresetValueLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	string = XmStringCreateSimple("Expression: ");
	forcePVCalcExpressionLabel = XtVaCreateManagedWidget("forcePVCalcExpression",
	    xmLabelGadgetClass, form3,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVCalcLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	forcePVCalcExpressionTextW = XtVaCreateManagedWidget("forcePVCalcExpressionTextW",
	    xmTextFieldWidgetClass, form3,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
/*
	    XmNcolumns,                40,
	    XmNmaxLength,              100,
*/
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVCalcExpressionLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);

	XtAddCallback(forcePVforceValueTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	prev = forcePVCalcExpressionTextW;

	for (i=0;i<NO_OF_CALC_PVS;i++) {

		pvid[0]=letter[i];
		string = XmStringCreateSimple(pvid);

		forcePVCalcPVLabel[i] = XtVaCreateManagedWidget("forcePVCalcPVLabel",
		    xmLabelGadgetClass, form3,
		    XmNlabelString,            string,
		    XmNtopAttachment,          XmATTACH_WIDGET,
		    XmNtopWidget,              prev,
		    XmNleftAttachment,         XmATTACH_FORM,
		    NULL);
		XmStringFree(string);
	
		forcePVCalcPVTextW[i] = XtVaCreateManagedWidget("forcePVCalcPVTextW",
		    xmTextFieldWidgetClass, form3,
		    XmNspacing,                0,
		    XmNmarginHeight,           0,
		    XmNcolumns,                30,
		    XmNmaxLength,              PVNAME_SIZE,
		    XmNbackground,             textBackground,
		    XmNtopAttachment,          XmATTACH_WIDGET,
		    XmNtopWidget,              prev,
		    XmNleftAttachment,         XmATTACH_WIDGET,
		    XmNleftWidget,             forcePVCalcPVLabel[i],
#if 0
			XmNrightAttachment,         XmATTACH_POSITION,
			XmNrightPosition,           50,
#endif
		    (XtPointer)NULL);
	
		prev = forcePVCalcPVTextW[i];
	
		XtAddCallback(forcePVCalcPVTextW[i], XmNactivateCallback,
		    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);
	}

	/* Set the client data "Apply", "Cancel", "Dismiss" and "Help" button's callbacks. */
	forcePV_items[0].data = (XtPointer)forcePVWindow;
	forcePV_items[1].data = (XtPointer)forcePVWindow;
	forcePV_items[2].data = (XtPointer)forcePVWindow;
	forcePV_items[3].data = (XtPointer)forcePVWindow;

	(void)createActionButtons(forcePVDialog, forcePV_items, XtNumber(forcePV_items));

	forcePVWindow->forcePVDialog = forcePVDialog;
	forcePVWindow->nameLabelW = nameLabelW;
	forcePVWindow->nameTextW = nameTextW;
	forcePVWindow->forcePVdisabledToggleButton = forcePVdisabledToggleButton;
	forcePVWindow->forcePVnameTextW = forcePVnameTextW;
	forcePVWindow->forcePVmaskStringLabelW = forcePVmaskStringLabelW;
	forcePVWindow->forcePVforceValueTextW = forcePVforceValueTextW;
	forcePVWindow->forcePVresetValueTextW = forcePVresetValueTextW;
	forcePVWindow->forcePVCalcExpressionTextW = forcePVCalcExpressionTextW;

	for (i=0;i<NO_OF_CALC_PVS;i++) {
		forcePVWindow->forcePVCalcPVTextW[i] = forcePVCalcPVTextW[i];
	}

	for (i = 0; i < ALARM_NMASK; i++){
		forcePVWindow->forceMaskToggleButtonW[i] = forceMaskToggleButtonW[i];
	}

	/* update forcePVWindow link info */
	forcePVUpdateDialogWidgets(forcePVWindow);

	/* RowColumn is full -- now manage */
	XtManageChild(form3);

	/* RowColumn is full -- now manage */
	XtManageChild(form2);

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
	int index=(long)calldata;
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
  forcePVApplyCallback
******************************************************/
static void forcePVApplyCallback(Widget widget, XtPointer calldata,
XtPointer cbs)
{
	struct forcePVWindow *forcePVWindow= (struct forcePVWindow *)calldata;
	double dbl;
	int rtn,i;
	XmString string;
	char *buff;
	GCLINK *link;
	int linkType;
	FORCEPV forcePV;
    FORCEPV_CALC calc;
	FORCEPV* pforcePV;
    FORCEPV_CALC* pcalc;
	char buff1[6];

	link =getSelectionLinkArea(forcePVWindow->area);
	if (!link) return;
	linkType =getSelectionLinkTypeArea(forcePVWindow->area);

    pforcePV=&forcePV;
    pforcePV->pcalc=&calc;
	pcalc=pforcePV->pcalc;

	/* initialize */
	pforcePV->name=0;
	pcalc->expression=0;
	for (i=0;i<NO_OF_CALC_PVS;i++) pcalc->name[i]=0;

	/* ---------------------------------
	     Force Process Variable
	     --------------------------------- */
	/*  update link field  - forcePVMask */
	XtVaGetValues(forcePVWindow->forcePVmaskStringLabelW, XmNlabelString, &string, NULL);
	XmStringGetLtoR(string,XmFONTLIST_DEFAULT_TAG,&buff);
	XmStringFree(string);
	alSetMask(buff,&(pforcePV->forceMask));
	XtFree(buff);

	/*  update link field  - pforcePV->forceValue */
	buff = XmTextFieldGetString(forcePVWindow->forcePVforceValueTextW);
	dbl=0;
	rtn = sscanf(buff,"%lf",&dbl);
	if (rtn == 1) pforcePV->forceValue = dbl;
	else pforcePV->forceValue = 1;
	XtFree(buff);

	/*  update link field  - pforcePV->resetValue */
	buff = XmTextFieldGetString(forcePVWindow->forcePVresetValueTextW);
	if (strncmp(buff,"NE",2)==0 || strncmp(buff,"ne",2)==0) {
		pforcePV->resetValue = pforcePV->forceValue;
	} else {
		dbl=0;
		rtn = sscanf(buff,"%lf",&dbl);
		if (rtn == 1) pforcePV->resetValue = dbl;
		else pforcePV->resetValue = 0;
	}
	XtFree(buff);

	/*  update link field  - pforcePV->name */
	buff = XmTextFieldGetString(forcePVWindow->forcePVnameTextW);
	if (strlen(buff)) pforcePV->name=buff;
	else {
		pforcePV->name=0;
		XtFree(buff);
	}

	/*  update disabled field  - pforcePV->disabled */
    pforcePV->disabled=XmToggleButtonGadgetGetState(forcePVWindow->forcePVdisabledToggleButton);

	/* CALC expression */
	buff = XmTextFieldGetString(forcePVWindow->forcePVCalcExpressionTextW);
	if (strlen(buff)) pcalc->expression = buff;
	else {
		pcalc->expression=0;
		XtFree(buff);
	}

	for (i=0;i<NO_OF_CALC_PVS;i++) {
		buff = XmTextFieldGetString(forcePVWindow->forcePVCalcPVTextW[i]);
		if (strlen(buff)) pcalc->name[i]=buff;
		else {
			pcalc->name[i]=0;
			XtFree(buff);
		}
	}

	forcePVUpdateFields(link,pforcePV,linkType);

	if (linkType == GROUP){
		awGetMaskString(((GLINK*)link)->pgroupData->mask,buff1);
		alLogOpModMessage(FORCE_MASK_GROUP,link,
			" Group forcePV modified <%s> [%g] [%s]",
              buff1,
              link->pgcData->pforcePV->forceValue,
              link->pgcData->pforcePV->name ? link->pgcData->pforcePV->name : " ");

	}else {
		alGetMaskString(((CLINK*)link)->pchanData->curMask,buff1);
		alLogOpModMessage(FORCE_MASK,(GCLINK*)link,
			"Channel forcePV modified <%s> [%g] [%s]",
              buff1,
              link->pgcData->pforcePV->forceValue,
              link->pgcData->pforcePV->name ? link->pgcData->pforcePV->name : " ");
	}

	/* ---------------------------------
	     Update dialog windows
	     --------------------------------- */
	axUpdateDialogs(forcePVWindow->area);

	/* free memory */
	if (pforcePV->name) XtFree(pforcePV->name);
	if (pcalc->expression) XtFree(pcalc->expression);
	for (i=0;i<NO_OF_CALC_PVS;i++) {
		if (pcalc->name[i]) XtFree(pcalc->name[i]);
	}
}

/******************************************************
  forcePVHelpCallback
******************************************************/
static void forcePVHelpCallback(Widget widget, XtPointer calldata,
XtPointer cbs)
{
	char *message1 =
	"Create or modify forcePV values for a selected group or channel.\n"
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


/**************************************************************
  Free forcePV calc memory
*************************************************************/
static void forcePVCalcDelete(FORCEPV_CALC** ppcalc)
{
	FORCEPV_CALC* pcalc=*ppcalc;
	int i;

	if (!ppcalc || !pcalc) return;
    if (pcalc->expression) free(pcalc->expression);
    if (pcalc->rpbuf) free(pcalc->rpbuf);
	for (i=0;i<NO_OF_CALC_PVS;i++) {
    	if (pcalc->puser[i]) free(pcalc->puser[i]);
    	if (pcalc->evid[i]) alCaClearEvent(&pcalc->evid[i]);
    	if (pcalc->chid[i]) alCaClearChannel(&pcalc->chid[i]);
    	if (pcalc->name[i]) free(pcalc->name[i]);
	}
	free(pcalc);
	ppcalc=0;
}

/**************************************************************
  Delete forcePV
*************************************************************/
void alForcePVDelete(FORCEPV** ppforcePV)
{
	FORCEPV* pforcePV=*ppforcePV;

	if (!ppforcePV || !pforcePV) return;
    alCaClearEvent(&pforcePV->evid);
    if (pforcePV->puser) free(pforcePV->puser);
    alCaClearChannel(&pforcePV->chid);
    alCaClearChannel(&pforcePV->chid);
    if (pforcePV->name) free(pforcePV->name);
	forcePVCalcDelete(&pforcePV->pcalc);
	free(pforcePV);
	pforcePV=0;
}

/**************************************************************
  Copy forcePV - DO NOT COPY CHANNEL ACCESS FIELDS
*************************************************************/
FORCEPV* alForcePVCopy(FORCEPV* pforcePV)
{
	FORCEPV* pnew;
	int i;

	if (!pforcePV) return 0;
	pnew=(FORCEPV*)calloc(1,sizeof(FORCEPV));
	if (pforcePV->name){
		pnew->name = (char *)calloc(strlen(pforcePV->name)+1,sizeof(char));
		strcpy(pforcePV->name,pnew->name);
	}
	pnew->forceValue=pforcePV->forceValue;
	pnew->resetValue=pforcePV->resetValue;
	pnew->disabled=pforcePV->disabled;
	pnew->forceMask=pforcePV->forceMask;

	if (!pforcePV->pcalc) return pnew;
	pnew->pcalc=(FORCEPV_CALC*)calloc(1,sizeof(FORCEPV_CALC));
	if (pforcePV->pcalc->expression){
		pnew->pcalc->expression=(char *)calloc(strlen(pforcePV->pcalc->expression)+1,sizeof(char));
		strcpy(pforcePV->pcalc->expression,pnew->pcalc->expression);
	}
	if (pforcePV->pcalc->rpbuf){
		pnew->pcalc->rpbuf=(char*)calloc(strlen(pforcePV->pcalc->rpbuf)+1,sizeof(char));
		strcpy(pforcePV->pcalc->rpbuf,pnew->pcalc->rpbuf);
	}
	for (i=0;i<NO_OF_CALC_PVS;i++) {
		if (pforcePV->pcalc->name[i]){
			pnew->pcalc->name[i]=(char*)calloc(strlen(pforcePV->pcalc->name[i])+1,sizeof(char));
			strcpy(pforcePV->pcalc->name[i],pnew->pcalc->name[i]);
		}
	}
	return pnew;
}

/**************************************************************
  alForcePVClearCA
*************************************************************/
void alForcePVClearCA(FORCEPV* pforcePV)
{
	FORCEPV_CALC* pcalc;
	int i;

	if (!pforcePV) return;
    if (pforcePV->puser) free(pforcePV->puser);
	pforcePV->puser=0;
    if (pforcePV->evid) alCaClearEvent(&pforcePV->evid);
    if (pforcePV->chid) alCaClearChannel(&pforcePV->chid);
	if (!pforcePV->pcalc) return;
	pcalc=pforcePV->pcalc;
	for (i=0;i<NO_OF_CALC_PVS;i++) {
    	if (pcalc->puser[i]) free(pcalc->puser[i]);
		pcalc->puser[i]=0;
    	if (pcalc->evid[i]) alCaClearEvent(&pcalc->evid[i]);
    	if (pcalc->chid[i]) alCaClearChannel(&pcalc->chid[i]);
	}
}


/**************************************************************
  alForcePVSetNotConnected
*************************************************************/
void alForcePVSetNotConnected(FORCEPV* pforcePV,char* name)
{
    FORCEPV_CALC* pcalc;
	int i;

	if (!pforcePV) return;
	if ( pforcePV->chid && !alCaIsConnected(pforcePV->chid) ) {
			errMsg("Force PV %s for %s Not Connected\n",
				pforcePV->name, name);
		}
    	pcalc=pforcePV->pcalc;
		if (pforcePV->pcalc) {
			for (i=0;i<NO_OF_CALC_PVS;i++) {
				if ( pforcePV->pcalc->chid[i] &&
					 	!alCaIsConnected(pforcePV->pcalc->chid[i]) ) {
					errMsg("Force CALC PV %s for %s Not Connected\n",
						pforcePV->pcalc->name[i], name);
				}
			}
		}
}

/**************************************************************
  forcePVUpdateFields
*************************************************************/
void forcePVUpdateFields(GCLINK* gclink,FORCEPV* pfPV,int context)
{
	FORCEPV* pforcePV;
	int i=0;
	long status=0;
	short err=0;
	short disabledHold;
	FORCEPV_CALC* pcalc;
	FORCEPV_CALC* pcalcNew;
	FORCEPVCADATA* puser;
	char buf[MAX_STRING_LENGTH];
	double holdValue;
	ALINK *area = gclink->pmainGroup->area;

	if (!gclink->pgcData->pforcePV)
			gclink->pgcData->pforcePV=(FORCEPV*)calloc(1,sizeof(FORCEPV));
	pforcePV=gclink->pgcData->pforcePV;

	/* temporarily disable forcePV */
	disabledHold=pforcePV->disabled;
	pforcePV->disabled=1;

	if (pforcePV->forceValue != pfPV->forceValue) pforcePV->forceValue=pfPV->forceValue;
	if (pforcePV->resetValue != pfPV->resetValue) pforcePV->resetValue=pfPV->resetValue;
	pforcePV->forceMask=pfPV->forceMask;

	if (pfPV->name){
		if (!pforcePV->name || strcmp(pforcePV->name,pfPV->name)){
			if (pforcePV->name){
				free(pforcePV->name);
				pforcePV->name=0;
				/* cancel channel access */
				if (pforcePV->puser) free(pforcePV->puser);
				pforcePV->puser=0;
				if (pforcePV->evid) alCaClearEvent(&pforcePV->evid);
				if (pforcePV->chid) alCaClearChannel(&pforcePV->chid);
			}
			pforcePV->name = (char *)calloc(strlen(pfPV->name)+1,sizeof(char));
			strcpy(pforcePV->name,pfPV->name);
			if (strlen(pforcePV->name) && strcmp(pforcePV->name,"CALC")) {
				/* start channel access */
				pforcePV->currentValue = -999;
				alCaConnectForcePV(pforcePV->name,&pforcePV->chid,gclink->pgcData->name);
				puser=(FORCEPVCADATA *)calloc(1,sizeof(FORCEPVCADATA));
				puser->index=-1;
				puser->link=gclink;
				puser->linktype=context;
				if (pforcePV->puser) free(pforcePV->puser);
				pforcePV->puser = puser;
				alCaAddForcePVEvent (pforcePV->chid,puser,&pforcePV->evid);
			}
		}
	} else {
		if (pforcePV->name){
			free(pforcePV->name);
			pforcePV->name=0;
			/* cancel channel access */
			if (pforcePV->puser) free(pforcePV->puser);
			pforcePV->puser=0;
			if (pforcePV->evid) alCaClearEvent(&pforcePV->evid);
			if (pforcePV->chid) alCaClearChannel(&pforcePV->chid);
		}
	}

	/* update expression */
	if (pfPV->pcalc->expression){
		if (!pforcePV->pcalc) pforcePV->pcalc=(FORCEPV_CALC*)calloc(1,sizeof(FORCEPV_CALC));
		if (!pforcePV->pcalc->expression || 
				strcmp(pforcePV->pcalc->expression,pfPV->pcalc->expression)){
			if (pforcePV->pcalc->expression){
				free(pforcePV->pcalc->expression);
				if (pforcePV->pcalc->rpbuf) free(pforcePV->pcalc->rpbuf);
			}
			pforcePV->pcalc->expression = 
				(char *)calloc(strlen(pfPV->pcalc->expression)+1,sizeof(char));
			strcpy(pforcePV->pcalc->expression,pfPV->pcalc->expression);
			/* convert an algebraic expression to symbolic postfix */
			status=postfix(pforcePV->pcalc->expression,buf,&err);
			if(status || err) {
				errMsg("Error converting '%s' to symbolic postfix: status=%ld err=%d\n",
					pforcePV->pcalc->expression,status,err);
			} else {
				pforcePV->pcalc->rpbuf=(char*)calloc(strlen(buf)+1,sizeof(char));
				strcpy(pforcePV->pcalc->rpbuf,buf);
			}
		}
	} else {
		if (pforcePV->pcalc){ 
			if (pforcePV->pcalc->expression) free(pforcePV->pcalc->expression);
			pforcePV->pcalc->expression=0;
			if (pforcePV->pcalc->rpbuf) free(pforcePV->pcalc->rpbuf);
		 	pforcePV->pcalc->rpbuf=0;
		}
	}

	pcalc=pforcePV->pcalc;
	pcalcNew=pfPV->pcalc;
	/* update CALC pv names */
	for (i=0;i<NO_OF_CALC_PVS;i++) {
		if (pcalcNew->name[i]){
			if (!pcalc) pcalc=(FORCEPV_CALC*)calloc(1,sizeof(FORCEPV_CALC));
			if (!pcalc->name[i] ||
					strcmp(pcalc->name[i],pcalcNew->name[i])){
				if (pcalc->name[i]){
					free(pcalc->name[i]);
					pcalc->name[i]=0;
					pcalc->value[i] = -999;
					/* cancel channel access */
					if (pcalc->puser[i]) free(pcalc->puser[i]);
					pcalc->puser[i]=0;
					if (pcalc->evid[i]) alCaClearEvent(&pcalc->evid[i]);
					if (pcalc->chid[i]) alCaClearChannel(&pcalc->chid[i]);
				}
				pcalc->name[i] = 
					(char *)calloc(strlen(pcalcNew->name[i])+1,sizeof(char));
				strcpy(pcalc->name[i],pcalcNew->name[i]);
				if (strlen(pcalc->name[i]) && strcmp(pcalc->name[i],"CALC")) {
	            	if (!isalpha(*pcalcNew->name[i])) {
						pcalc->value[i] = atof(pcalcNew->name[i]);
					}else {
						/* start channel access */
						pcalc->value[i] = -999;
						alCaConnectForcePV(pcalc->name[i],&pcalc->chid[i],
							gclink->pgcData->name);
						puser=(FORCEPVCADATA *)calloc(1,sizeof(FORCEPVCADATA));
						puser->index=i;
						puser->link=gclink;
						puser->linktype=context;
						if (pcalc->puser[i]) free(pcalc->puser[i]);
						pcalc->puser[i] = puser;
						alCaAddForcePVEvent(pcalc->chid[i],puser,&pcalc->evid[i]);
					}
				}
			}
		} else {
			if (pcalc){ 
				if (pcalc->name[i]==0) continue;
				free(pcalc->name[i]);
				pcalc->name[i]=0;
				pcalc->value[i] = -999;
				/* cancel channel access */
				if (pcalc->puser[i]) free(pcalc->puser[i]);
				pcalc->puser[i]=0;
				if (pcalc->evid[i]) alCaClearEvent(&pcalc->evid[i]);
				if (pcalc->chid[i]) alCaClearChannel(&pcalc->chid[i]);
			}
		}
	}
	pforcePV->pcalc=pcalc;
	pforcePV->disabled=pfPV->disabled;
        updateDisabledForcePVCount(area,(int)(pforcePV->disabled-disabledHold));

	if (pforcePV->name && strcmp(pforcePV->name,"CALC")==0) {
                pforcePV->currentValue = -999;
		forcePVCalcPerform(gclink,context,1);
	} else {
		holdValue=pforcePV->currentValue;
		pforcePV->currentValue = -999;
		forcePVNewValueEvent(gclink,(short)context,holdValue);
	}
}

/*******************************************************************
    ForcePV New Value Event
*******************************************************************/
static void forcePVNewValueEvent(GCLINK* gclink,short linktype,double value)
{
	struct gcData *pgcData;
	FORCEPV* pforcePV;
	char buff1[6];
	char buff2[20];

	pgcData=gclink->pgcData;
	pforcePV=pgcData->pforcePV;
	if (!pforcePV) return;
	if (!alCaIsConnected(pforcePV->chid)) return;

	if (!pforcePV->disabled) {
		if (pforcePV->currentValue==value) return;
		if (value==pforcePV->forceValue) {
			if (linktype==GROUP){
				alChangeGroupMask((GLINK*)gclink,pforcePV->forceMask);
				awGetMaskString(((GLINK*)gclink)->pgroupData->mask,buff1);
				alLogOpModMessage(FORCE_MASK_GROUP,gclink,
					"Group forcePV FORCE <%s> [%g] [%s]",
              		buff1,
              		pforcePV->forceValue,
              		pforcePV->name);
			} else {
				alChangeChanMask((CLINK*)gclink,pforcePV->forceMask);
				alGetMaskString(((CLINK*)gclink)->pchanData->curMask,buff1);
				alLogOpModMessage(FORCE_MASK,gclink,
					"Channel forcePV FORCE <%s> [%g] [%s]",
              		buff1,
              		pforcePV->forceValue,
              		pforcePV->name);
			}
			alCaFlushIo();
		}
    	else if ( ( (pforcePV->currentValue == -999 ||
                 	pforcePV->currentValue == pforcePV->forceValue) &&
                	pforcePV->forceValue == pforcePV->resetValue ) ||
              	( value == pforcePV->resetValue  &&
                	pforcePV->forceValue != pforcePV->resetValue  ) ) {
			if (linktype==GROUP){
				alResetGroupMask((GLINK*)gclink);
				awGetMaskString(((GLINK*)gclink)->pgroupData->mask,buff1);
				if (pforcePV->resetValue == pforcePV->forceValue )
					 sprintf(buff2,"%s %g","NE",pforcePV->forceValue);
				else sprintf(buff2,"%g",pforcePV->resetValue);
				alLogOpModMessage(CHANGE_MASK_GROUP,gclink,
					"Group forcePV RESET <%s> [%s] [%s]",
              		buff1,
              		buff2,
              		pforcePV->name);

			} else {
				CLINK *clink=(CLINK*)gclink;;
				alChangeChanMask(clink,clink->pchanData->defaultMask);
				alGetMaskString(clink->pchanData->curMask,buff1);
				if (pforcePV->resetValue == pforcePV->forceValue )
					 sprintf(buff2,"%s %g","NE",pforcePV->forceValue);
				else sprintf(buff2,"%g",pforcePV->resetValue);
				alLogOpModMessage(CHANGE_MASK,gclink,
					"Channel forcePV RESET <%s> [%s] [%s]",
              		buff1,
              		buff2,
              		pforcePV->name);
			}
			alCaFlushIo();
		}
	}
	pforcePV->currentValue=value;
}


/*******************************************************************
    ForcePVCalc New Value Event
*******************************************************************/
void alForcePVCalcNewValueEvent(GCLINK* gclink,short linktype,short index,double value)
{
	FORCEPV* pforcePV;
	FORCEPV_CALC* pcalc;

	pforcePV=gclink->pgcData->pforcePV;
	if (!pforcePV) return;
	if (!pforcePV->pcalc) return;
	pcalc=pforcePV->pcalc;
	if (pcalc->value[index]==value) return;
	pcalc->value[index]=value;

	forcePVCalcPerform(gclink,linktype,0);
}

/*******************************************************************
    ForcePVCalc Calculate expression value
*******************************************************************/
static void forcePVCalcPerform(GCLINK* gclink,int linktype,int forcePVChanged)
{
	FORCEPV* pforcePV=gclink->pgcData->pforcePV;
	FORCEPV_CALC* pcalc=gclink->pgcData->pforcePV->pcalc;
	int i;
	double calcValue=0;
	long status =0;
	char buff1[6];
	char buff2[20];

	if (!pcalc) return;

	for (i=0;i<NO_OF_CALC_PVS;i++){
		if (pcalc->chid[i] && !alCaIsConnected(pcalc->chid[i])) return;
		if (pcalc->value[i]==-999.) return;
	}
	if (!pcalc->rpbuf) return;
	status=calcPerform(pcalc->value,&calcValue,pcalc->rpbuf);
	if (status) errMsg("ForcePV calcPerform failed: status=%ld\n",status);

	if (!forcePVChanged && calcValue == pforcePV->currentValue) return;

	if (pforcePV->name && strcmp(pforcePV->name,"CALC")==0 && !pforcePV->disabled) {
		if ((float)calcValue == (float)pforcePV->forceValue) {
			if (linktype==GROUP){
				alChangeGroupMask((GLINK*)gclink,pforcePV->forceMask);
				awGetMaskString(((GLINK*)gclink)->pgroupData->mask,buff1);
				alLogOpModMessage(FORCE_MASK_GROUP,gclink,
					"Group forcePV FORCE <%s> [%g] [%s]",
              		buff1,
              		pforcePV->forceValue,
              		pforcePV->name);
			} else {
				alChangeChanMask((CLINK*)gclink,pforcePV->forceMask);
				alGetMaskString(((CLINK*)gclink)->pchanData->curMask,buff1);
				alLogOpModMessage(FORCE_MASK,gclink,
					"Channel forcePV FORCE <%s> [%g] [%s]",
              		buff1,
              		pforcePV->forceValue,
              		pforcePV->name);
			}
			alCaFlushIo();
		} else if (((pforcePV->currentValue == -999 ||
				pforcePV->currentValue == pforcePV->forceValue) &&
		 		pforcePV->forceValue == pforcePV->resetValue) ||
              	( (float)calcValue == (float)pforcePV->resetValue  &&
                	pforcePV->forceValue != pforcePV->resetValue) ){
			if (linktype==GROUP){
				alResetGroupMask((GLINK*)gclink);
				awGetMaskString(((GLINK*)gclink)->pgroupData->mask,buff1);
				if (pforcePV->resetValue == pforcePV->forceValue )
					 sprintf(buff2,"%s %g","NE",pforcePV->forceValue);
				else sprintf(buff2,"%g",pforcePV->resetValue);
				alLogOpModMessage(CHANGE_MASK_GROUP,gclink,
					"Group forcePV RESET <%s> [%s] [%s]",
              		buff1,
              		buff2,
              		pforcePV->name);
			} else {
				CLINK *clink=(CLINK*)gclink;;
				alChangeChanMask(clink,clink->pchanData->defaultMask);
				alGetMaskString(clink->pchanData->curMask,buff1);
				if (pforcePV->resetValue == pforcePV->forceValue )
					 sprintf(buff2,"%s %g","NE",pforcePV->forceValue);
				else sprintf(buff2,"%g",pforcePV->resetValue);
				alLogOpModMessage(CHANGE_MASK,gclink,
					"Channel forcePV RESET <%s> [%s] [%s]",
              		buff1,
              		buff2,
              		pforcePV->name);
				}
			alCaFlushIo();
		}
	}
	pforcePV->currentValue=calcValue;
}

/*******************************************************************
    ForcePV Value Event from CA
*******************************************************************/
void alForcePVValueEvent ( void* userdata,double value) {
	FORCEPVCADATA* puser = (FORCEPVCADATA*)userdata;
	GCLINK* gclink;
	short index;
	short linktype;

	if (!puser) return;
	gclink=(GCLINK*)puser->link;
	index=puser->index;
	linktype=puser->linktype;

	if (puser->index==-1) forcePVNewValueEvent(gclink,linktype,value);
	else alForcePVCalcNewValueEvent(gclink,linktype,index,value);
}
