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
/* property.c */

/************************DESCRIPTION***********************************
  routines for display of group and channel properties
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#include <stdlib.h>
#include <stdio.h>

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

#include "alarm.h"
#include "axArea.h"
#include "alLib.h"
#include "ax.h"

extern Pixel bg_pixel[ALH_ALARM_NSEV];

struct propWindow {
	void *area;
	Widget menuButton;
	Widget propDialog;
	Widget nameLabelW;
	Widget nameTextW;
	Widget alarmMaskStringLabelW;
	Widget alarmMaskToggleButtonW[ALARM_NMASK];
	Widget resetMaskStringLabelW;
	Widget maskFrameW;
	Widget beepSeverityValueTextW;
	Widget severityPVnameTextW;
	Widget countFilterFrame;
	Widget countFilterCountTextW;
	Widget countFilterSecondsTextW;
	Widget forcePVnameTextW;
	Widget forcePVForceMaskStringLabelW;
	Widget forceMaskToggleButtonW[ALARM_NMASK];
	Widget forcePVcurrentValueTextW;
	Widget forcePVforceValueTextW;
	Widget forcePVresetValueTextW;
	Widget forcePVCalcExpressionTextW;
	Widget forcePVCalcPVTextW[NO_OF_CALC_PVS];
	Widget aliasTextW;
	Widget processTextW;
	Widget statProcessTextW;
	Widget sevrProcessTextW;
	Widget guidanceTextW;
	Widget guidanceUrlW;

};

extern char * alhAlarmSeverityString[];

/* forward declarations */
static void propApplyCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void propCancelCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void propDismissCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void propHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void propCreateDialog(ALINK*area);
static void propUpdateDialogWidgets(struct propWindow *propWindow);
static void propMaskChangeCallback( Widget widget,XtPointer calldata,XtPointer cbs);
static void propEditableDialogWidgets(ALINK *area);
static GCLINK *propCreateClone(GCLINK *link,int linkType);
static void propDeleteClone(GCLINK *link,int linkType);

/******************************************************
  propUpdateDialog
******************************************************/
void propUpdateDialog(ALINK *area)
{
	struct propWindow *propWindow;

	propWindow = (struct propWindow *)area->propWindow;

	if (!propWindow)  return;

	if (!propWindow->propDialog || !XtIsManaged(propWindow->propDialog)) return;

	propUpdateDialogWidgets(propWindow);

}

/******************************************************
  propShowDialog
******************************************************/
void propShowDialog(ALINK *area,Widget menuButton)
{
	struct propWindow *propWindow;

	propWindow = (struct propWindow *)area->propWindow;

	/* dismiss Dialog */
	if (propWindow && propWindow->propDialog && 
	    XtIsManaged(propWindow->propDialog)) {
		propDismissCallback(NULL, (XtPointer)propWindow, NULL);
		return;
	}

	/* create propWindow and Dialog Widgets if necessary */
	if (!propWindow)  propCreateDialog(area);

	/* update propWindow link info */
	propWindow = (struct propWindow *)area->propWindow;
	propWindow->menuButton = menuButton;

	/* update Dialog Widgets */
	propUpdateDialogWidgets(propWindow);

	/* show Dialog */
	if (!propWindow->propDialog) return;
	if (!XtIsManaged(propWindow->propDialog)) {
		XtManageChild(propWindow->propDialog);
	}
	XMapWindow(XtDisplay(propWindow->propDialog),
	    XtWindow(XtParent(propWindow->propDialog)));
	if (menuButton) XtVaSetValues(menuButton, XmNset, TRUE, NULL);

}

/******************************************************
  propUpdateDialogWidgets
******************************************************/
static void propUpdateDialogWidgets(struct propWindow *propWindow)
{
	struct gcData *pgcData;
	struct chanData *pcData = 0;
	GCLINK *link;
	int linkType;
	XmString string;
	char buff[MAX_STRING_LENGTH];
	char buff1[MAX_STRING_LENGTH];
	char *str;
	SNODE *pt;
	int i=0;
	struct guideLink *guideLink;
	MASK mask;
	Pixel textBackground;
	Pixel textBackgroundNS;

	if (programId != ALH) textBackground = bg_pixel[3];
	else textBackground = bg_pixel[0];
	textBackgroundNS = bg_pixel[0];

	link =getSelectionLinkArea(propWindow->area);

	if (! propWindow || !propWindow->propDialog) return;

	if (!link) {

		XmTextFieldSetString(propWindow->nameTextW, "");
		string = XmStringCreateSimple("-----");
		XtVaSetValues(propWindow->alarmMaskStringLabelW, XmNlabelString, string, NULL);
		XmStringFree(string);
		if (programId == ALH) {
			string = XmStringCreateSimple("-");
			XtVaSetValues(propWindow->resetMaskStringLabelW, XmNlabelString, string, NULL);
			XmStringFree(string);
		}
		if (programId != ALH) {
			XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[0],FALSE,TRUE);
			XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[1],FALSE,TRUE);
			XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[2],FALSE,TRUE);
			XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[3],FALSE,TRUE);
			XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[4],FALSE,TRUE);
		}
		XmTextFieldSetString(propWindow->beepSeverityValueTextW,"");
		XmTextFieldSetString(propWindow->severityPVnameTextW,"");
		XmTextFieldSetString(propWindow->countFilterCountTextW,"");
		XmTextFieldSetString(propWindow->countFilterSecondsTextW,"");



		/* ForcePV data */
		XmTextFieldSetString(propWindow->forcePVnameTextW,"");
		string = XmStringCreateSimple("-----");
		XtVaSetValues(propWindow->forcePVForceMaskStringLabelW, XmNlabelString, string, NULL);
		XmStringFree(string);
/*
		if (programId == ALH) {
			string = XmStringCreateSimple("");
			XtVaSetValues(propWindow->forcePVcurrentValueTextW, XmNlabelString, string, NULL);
			XmStringFree(string);
		}
*/
		XmTextFieldSetString(propWindow->forcePVforceValueTextW,"");
		XmTextFieldSetString(propWindow->forcePVresetValueTextW,"");
		XmTextFieldSetString(propWindow->forcePVCalcExpressionTextW,"");
		for (i=0;i<NO_OF_CALC_PVS;i++){
			XmTextFieldSetString(propWindow->forcePVCalcPVTextW[i],"");
		}

		XmTextFieldSetString(propWindow->aliasTextW,"");
		XmTextFieldSetString(propWindow->processTextW,"");
		XmTextFieldSetString(propWindow->sevrProcessTextW,"");
		XmTextFieldSetString(propWindow->statProcessTextW,"");
		XmTextSetString(propWindow->guidanceTextW, "");
		XmTextSetString(propWindow->guidanceUrlW, "");

		return;
	}

	pgcData = link->pgcData;
	linkType =getSelectionLinkTypeArea(propWindow->area);
	if (linkType == CHANNEL) pcData = (struct chanData *)pgcData;

	/* ---------------------------------
	     Group/Channel Name 
	     --------------------------------- */
	if (linkType == GROUP) string = XmStringCreateSimple("Group  ");
	else string = XmStringCreateSimple("Channel");
	XtVaSetValues(propWindow->nameLabelW, XmNlabelString, string, NULL);
	XmStringFree(string);

	XmTextFieldSetString(propWindow->nameTextW, pgcData->name);
	/*
	     if (strncmp(pgcData->name,"Unnamed",7))
	          XmTextFieldSetString(propWindow->nameTextW, pgcData->name);
	     else 
	          XmTextFieldSetString(propWindow->nameTextW, "");
	*/

	/* ---------------------------------
	     Current Alarm Mask 
	     --------------------------------- */
	if (linkType == GROUP) awGetMaskString(((struct groupData *)pgcData)->mask,buff);
	else alGetMaskString(pcData->curMask,buff);
	string = XmStringCreateSimple(buff);
	XtVaSetValues(propWindow->alarmMaskStringLabelW, XmNlabelString, string, NULL);
	XmStringFree(string);

	if (programId != ALH) {
		if (linkType == GROUP) XtSetSensitive(propWindow->maskFrameW, FALSE);
		else XtSetSensitive(propWindow->maskFrameW, TRUE);

		alSetMask(buff,&mask);
		if (mask.Cancel == 1 )
			XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[0],TRUE,TRUE);
		else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[0],FALSE,TRUE);
		if (mask.Disable == 1 )
			XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[1],TRUE,TRUE);
		else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[1],FALSE,TRUE);
		if (mask.Ack == 1 )
			XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[2],TRUE,TRUE);
		else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[2],FALSE,TRUE);
		if (mask.AckT == 1 )
			XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[3],TRUE,TRUE);
		else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[3],FALSE,TRUE);
		if (mask.Log == 1 )
			XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[4],TRUE,TRUE);
		else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[4],FALSE,TRUE);
	}

	/* ---------------------------------
	     Reset Mask 
	     --------------------------------- */
	if (programId == ALH ) {
		if ( linkType == CHANNEL) {
			alGetMaskString(pcData->defaultMask,buff);
			string = XmStringCreateSimple(buff);
		} else {
			string = XmStringCreateSimple("");
		}
		XtVaSetValues(propWindow->resetMaskStringLabelW, XmNlabelString, string, NULL);
		XmStringFree(string);
	}

	/* ---------------------------------
	     Beep Severity
	     --------------------------------- */
	if(pgcData->beepSevr > 1)
		XmTextFieldSetString(propWindow->beepSeverityValueTextW,alhAlarmSeverityString[pgcData->beepSevr]);
	else XmTextFieldSetString(propWindow->beepSeverityValueTextW,alhAlarmSeverityString[1]);

	/* ---------------------------------
	     Severity Process Variable
	     --------------------------------- */
	if(strcmp(pgcData->sevrPVName,"-") != 0)
		XmTextFieldSetString(propWindow->severityPVnameTextW,pgcData->sevrPVName);
	else XmTextFieldSetString(propWindow->severityPVnameTextW,"");

	/* ---------------------------------
	     Alarm Count Filter 
	     --------------------------------- */
	if (linkType == GROUP) {
		XmTextFieldSetString(propWindow->countFilterCountTextW,"");
		XmTextFieldSetString(propWindow->countFilterSecondsTextW,"");
		XtVaSetValues(propWindow->countFilterCountTextW,XmNbackground,textBackgroundNS,NULL);
		XtVaSetValues(propWindow->countFilterSecondsTextW,XmNbackground,textBackgroundNS,NULL);
	} else {
		XtVaSetValues(propWindow->countFilterCountTextW,XmNbackground,textBackground,NULL);
		XtVaSetValues(propWindow->countFilterSecondsTextW,XmNbackground,textBackground,NULL);
		if(pcData->countFilter) {
			sprintf(buff,"%d",pcData->countFilter->inputCount);
			XmTextFieldSetString(propWindow->countFilterCountTextW,buff);
			sprintf(buff,"%d",pcData->countFilter->inputSeconds);
			XmTextFieldSetString(propWindow->countFilterSecondsTextW,buff);
		} else {
			XmTextFieldSetString(propWindow->countFilterCountTextW,"");
			XmTextFieldSetString(propWindow->countFilterSecondsTextW,"");
		}
	}

	/* ---------------------------------
	     Force Process Variable
	     --------------------------------- */
	if(pgcData->pforcePV && pgcData->pforcePV->name)
		XmTextFieldSetString(propWindow->forcePVnameTextW,pgcData->pforcePV->name);
	else XmTextFieldSetString(propWindow->forcePVnameTextW,"");

	if(pgcData->pforcePV){
		XmTextFieldSetString(propWindow->forcePVnameTextW,pgcData->pforcePV->name);
		alGetMaskString(pgcData->pforcePV->forceMask,buff);
		string = XmStringCreateSimple(buff);
	} else string = XmStringCreateSimple("-----");
	XtVaSetValues(propWindow->forcePVForceMaskStringLabelW, XmNlabelString, string, NULL);
	XmStringFree(string);
	if (programId != ALH) {
		if(pgcData->pforcePV) mask = pgcData->pforcePV->forceMask;
		if (mask.Cancel == 1 )
			XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[0],TRUE,TRUE);
		else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[0],FALSE,TRUE);
		if (mask.Disable == 1 )
			XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[1],TRUE,TRUE);
		else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[1],FALSE,TRUE);
		if (mask.Ack == 1 )
			XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[2],TRUE,TRUE);
		else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[2],FALSE,TRUE);
		if (mask.AckT == 1 )
			XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[3],TRUE,TRUE);
		else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[3],FALSE,TRUE);
		if (mask.Log == 1 )
			XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[4],TRUE,TRUE);
		else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[4],FALSE,TRUE);
	}

	if(pgcData->pforcePV){

		sprintf(buff,"%g",pgcData->pforcePV->forceValue);
		XmTextFieldSetString(propWindow->forcePVforceValueTextW,buff);

		sprintf(buff1,"%g",pgcData->pforcePV->resetValue);
		if (!strcmp(buff,buff1)) sprintf(buff,"%s","NE");
		else strcpy(buff,buff1);
		XmTextFieldSetString(propWindow->forcePVresetValueTextW,buff);

		if(pgcData->pforcePV->pcalc){
			if (pgcData->pforcePV->pcalc->expression)
				XmTextFieldSetString(propWindow->forcePVCalcExpressionTextW,
					pgcData->pforcePV->pcalc->expression);
			else XmTextFieldSetString(propWindow->forcePVCalcExpressionTextW,"");
			for (i=0;i<NO_OF_CALC_PVS;i++){
				if (pgcData->pforcePV->pcalc->name[i])
					XmTextFieldSetString(propWindow->forcePVCalcPVTextW[i],
						pgcData->pforcePV->pcalc->name[i]);
				else XmTextFieldSetString(propWindow->forcePVCalcPVTextW[i],"");
			}
		} else {
			XmTextFieldSetString(propWindow->forcePVCalcExpressionTextW,"");
			for (i=0;i<NO_OF_CALC_PVS;i++){
				XmTextFieldSetString(propWindow->forcePVCalcPVTextW[i],"");
			}
		}
	} else {
		/* ForcePV data */
		XmTextFieldSetString(propWindow->forcePVnameTextW,"");
		string = XmStringCreateSimple("-----");
		XtVaSetValues(propWindow->forcePVForceMaskStringLabelW, XmNlabelString, string, NULL);
		XmStringFree(string);
		XmTextFieldSetString(propWindow->forcePVforceValueTextW,"");
		XmTextFieldSetString(propWindow->forcePVresetValueTextW,"");
		XmTextFieldSetString(propWindow->forcePVCalcExpressionTextW,"");
		for (i=0;i<NO_OF_CALC_PVS;i++){
			XmTextFieldSetString(propWindow->forcePVCalcPVTextW[i],"");
		}
	}
	/* ---------------------------------
	     Alias
	     --------------------------------- */
	XmTextFieldSetString(propWindow->aliasTextW,pgcData->alias);

	/* ---------------------------------
	     Related Process Command
	     --------------------------------- */
	XmTextFieldSetString(propWindow->processTextW,pgcData->command);

	/* ---------------------------------
	     Sevr Command
	     --------------------------------- */
	getStringSevrCommandList(&pgcData->sevrCommandList,&str);
	XmTextSetString(propWindow->sevrProcessTextW,str);
	free(str);

	/* ---------------------------------
	     Stat Command
	     --------------------------------- */
	if (linkType == GROUP) {
		XtSetSensitive(propWindow->statProcessTextW, FALSE);
		XtVaSetValues(propWindow->statProcessTextW,XmNbackground,textBackgroundNS,NULL);
		XmTextSetString(propWindow->statProcessTextW,"");
	} else {
		XtSetSensitive(propWindow->statProcessTextW, TRUE);
		XtVaSetValues(propWindow->statProcessTextW,XmNbackground,textBackground,NULL);
		getStringStatCommandList(&pcData->statCommandList,&str);
		XmTextSetString(propWindow->statProcessTextW,str);
		free(str);
	}

	/* ---------------------------------
	     Guidance URL
	     --------------------------------- */
	XmTextFieldSetString(propWindow->guidanceUrlW,link->guidanceLocation);

	/* ---------------------------------
	     Guidance Text
	     --------------------------------- */
	pt = sllFirst(&(link->GuideList));
	i=0;
	while (pt) {
		guideLink = (struct guideLink *)pt;
		i += strlen(guideLink->list);
		i += 1;
		pt = sllNext(pt);
	}
	str = (char*)calloc(i+1,sizeof(char));
	pt = sllFirst(&(link->GuideList));
	while (pt) {
		guideLink = (struct guideLink *)pt;
		strcat(str,guideLink->list);
		pt = sllNext(pt);
		if (pt) strcat(str,"\n");
	}

	XmTextSetString(propWindow->guidanceTextW, str);
	free(str);

}

/******************************************************
  propCreateDialog
******************************************************/
static void propCreateDialog(ALINK *area)
{
	struct propWindow *propWindow;
	int n;
	Arg args[10];

	Widget propDialogShell, propDialog, severityPVnameTextW;
	Widget beepSeverityValueTextW;
	Widget rowcol, form, maskFrameW=0;
	Widget nameLabelW, nameTextW;
	Widget beepSeverityLabel, severityPVlabel;
	Widget alarmMaskToggleButtonW[ALARM_NMASK];
	Widget forceMaskToggleButtonW[ALARM_NMASK];
	Widget aliasLabel, aliasTextW;
	Widget processLabel, processTextW;
	Widget sevrProcessLabel, sevrProcessTextW;
	Widget statProcessLabel, statProcessTextW;
	Widget forcePVcurrentValueTextW=0;
	Widget forcePVLabel; 
	Widget forcePVnameLabel, forcePVnameTextW;
	Widget forcePVForceMaskLabel;
	Widget forcePVForceMaskStringLabelW;
	Widget forcePVforceValueLabel, forcePVforceValueTextW;
	Widget forcePVresetValueLabel, forcePVresetValueTextW;
	Widget forcePVCalcLabel; 
	Widget forcePVCalcExpressionLabel, forcePVCalcExpressionTextW;
	Widget forcePVCalcPVLabel[NO_OF_CALC_PVS],
           forcePVCalcPVTextW[NO_OF_CALC_PVS];
	Widget frame2, form2, frame3, form3;
	Widget  guidanceLabel, guidanceTextW,
	    guidanceUrlLabel, guidanceUrlW;
	Widget alarmMaskLabel, alarmMaskStringLabelW;
	Widget resetMaskLabel=0, resetMaskStringLabelW=0;
	Widget prev;
	int i;
	Widget countFilterFrame, form1, countFilterLabel,
	    countFilterCountLabel,countFilterCountTextW,
	    countFilterSecondsLabel, countFilterSecondsTextW;
	Pixel textBackground;
	XmString string;
    char letter[]={"ABCDEF"};
    char pvid[]="A ";
	static ActionAreaItem prop_items_act[] = {
		         { "Apply",   propApplyCallback,   NULL    },
		         { "Cancel",  propCancelCallback,  NULL    },
		         { "Dismiss", propDismissCallback, NULL    },
		         { "Help",    propHelpCallback,    NULL    },
		     	};
	static ActionAreaItem prop_items_alh[] = {
		         { "Dismiss", propDismissCallback, NULL    },
		         { "Help",    propHelpCallback,    NULL    },
		     	};
	static String maskFields[] = {
		         "Cancel Alarm", 
		         "Disable Alarm",
		         "NoAck Alarm",
		         "NoAck Transient Alarm",
		         "NoLog Alarm"
		     	};

	if (!area) return;

	propWindow = (struct propWindow *)area->propWindow;

	if (propWindow && propWindow->propDialog){
		if (XtIsManaged(propWindow->propDialog)) return;
		else XtManageChild(propWindow->propDialog);
	}

	if (programId != ALH) textBackground = bg_pixel[3];
	else textBackground = bg_pixel[0];

	propWindow = (struct propWindow *)calloc(1,sizeof(struct propWindow));
	area->propWindow = (void *)propWindow;
	propWindow->area = (void *)area;

	propDialogShell = XtVaCreatePopupShell("Alarm Handler Properties",
	    transientShellWidgetClass, area->toplevel, NULL, 0);

	/* Modify the window manager menu "close" callback */
	{
		Atom         WM_DELETE_WINDOW;
		XtVaSetValues(propDialogShell,
		    XmNdeleteResponse, XmDO_NOTHING, NULL);
		WM_DELETE_WINDOW = XmInternAtom(XtDisplay(propDialogShell),
		    "WM_DELETE_WINDOW", False);
		XmAddWMProtocolCallback(propDialogShell,WM_DELETE_WINDOW,
		    (XtCallbackProc)propDismissCallback, (XtPointer)propWindow);
	}

	propDialog = XtVaCreateWidget("propDialog",
	    xmPanedWindowWidgetClass, propDialogShell,
	    XmNsashWidth,  1,
	    XmNsashHeight, 1,
	    XmNuserData,   area,
	    NULL);

	(void)XtVaCreateWidget("control_area", xmRowColumnWidgetClass, propDialog, NULL);
	form = XtVaCreateWidget("control_area", xmFormWidgetClass, propDialog, NULL);

	/* ---------------------------------
	     Group/Channel Name 
	     --------------------------------- */
	nameLabelW = XtVaCreateManagedWidget("nameLabelW",
	    xmLabelGadgetClass, form,
	    XmNrecomputeSize,   True,
	    XmNtopAttachment,   XmATTACH_FORM,
	    XmNleftAttachment,  XmATTACH_FORM,
	    NULL);

	nameTextW = XtVaCreateManagedWidget("nameTextW",
	    xmTextFieldWidgetClass, form,
	    XmNspacing,          0,
	    XmNmarginHeight,     0,
/*
	    XmNcolumns,         30,
	    XmNrecomputeSize,   True,
*/
	    XmNmaxLength,       PVNAME_SIZE,
	    XmNbackground,      textBackground,
	    XmNtopAttachment,   XmATTACH_FORM,
	    XmNleftAttachment,  XmATTACH_WIDGET,
	    XmNleftWidget,      nameLabelW,
	    XmNrightAttachment,  XmATTACH_POSITION,
	    XmNrightPosition,    50,
	    NULL);

	XtAddCallback(nameTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	/* ---------------------------------
	     Current Alarm Mask 
	     --------------------------------- */
	if (programId != ALH) string = XmStringCreateSimple("Alarm Mask ");
	else string = XmStringCreateSimple("Current Mask ");
	alarmMaskLabel = XtVaCreateManagedWidget("alarmMaskLabel",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       nameTextW,
	    XmNleftAttachment,  XmATTACH_FORM,
	    NULL);
	XmStringFree(string);
	prev = alarmMaskLabel;

	string = XmStringCreateSimple("-----");
	alarmMaskStringLabelW = XtVaCreateManagedWidget("alarmMaskStringLabelW",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       nameTextW,
	    XmNleftAttachment,  XmATTACH_WIDGET,
	    XmNleftWidget,      alarmMaskLabel,
	    NULL);
	XmStringFree(string);

	/* ---------------------------------
	     Reset Mask 
	     --------------------------------- */
	if (programId == ALH ) {
		string = XmStringCreateSimple("Reset Mask ");
		resetMaskLabel = XtVaCreateManagedWidget("resetMaskLabel",
		    xmLabelGadgetClass, form,
		    XmNlabelString,     string,
	    	XmNleftAttachment,  XmATTACH_FORM,
		    XmNtopAttachment,   XmATTACH_WIDGET,
		    XmNtopWidget,       alarmMaskLabel,
		    NULL);
		XmStringFree(string);
		prev = resetMaskLabel;

		string = XmStringCreateSimple("     ");
		resetMaskStringLabelW = XtVaCreateManagedWidget("resetMaskStringLabelW",
		    xmLabelGadgetClass, form,
		    XmNlabelString,     string,
		    XmNtopAttachment,   XmATTACH_WIDGET,
		    XmNtopWidget,       alarmMaskLabel,
		    XmNleftAttachment,  XmATTACH_WIDGET,
		    XmNleftWidget,      resetMaskLabel,
		    NULL);
		XmStringFree(string);
	}

	if (programId != ALH) {
		maskFrameW = XtVaCreateManagedWidget("maskFrameW",
		    xmFrameWidgetClass, form,
		    XmNtopAttachment,   XmATTACH_WIDGET,
		    XmNtopWidget,       prev,
		    XmNleftAttachment,  XmATTACH_FORM,
		    NULL);
		prev=maskFrameW;

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
			XtAddCallback(alarmMaskToggleButtonW[i], XmNvalueChangedCallback,
			    (XtCallbackProc)propMaskChangeCallback, (XtPointer)i);
		}

		XtManageChild(rowcol);
	}

	/* ---------------------------------
	     Alarm Count Filter
	     --------------------------------- */
	countFilterFrame = XtVaCreateManagedWidget("countFilterFrame",
	    xmFrameWidgetClass, form,
	    XmNtopAttachment,   XmATTACH_FORM,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNleftAttachment,  XmATTACH_POSITION,
	    XmNleftPosition,    50,
	    NULL);

	form1 = XtVaCreateWidget("form1",
	    xmFormWidgetClass, countFilterFrame,
	    XmNspacing,          0,
	    XmNmarginHeight,     0,
	    NULL);

	string = XmStringCreateSimple("Alarm Count Filter             ");
	countFilterLabel = XtVaCreateManagedWidget("countFilterLabel",
	    xmLabelGadgetClass, form1,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	string = XmStringCreateSimple("Count ");
	countFilterCountLabel = XtVaCreateManagedWidget("countFilterCountLabel",
	    xmLabelGadgetClass, form1,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              countFilterLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	countFilterCountTextW = XtVaCreateManagedWidget("countFilterCountTextW",
	    xmTextFieldWidgetClass, form1,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                3,
	    XmNmaxLength,              3,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              countFilterLabel,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             countFilterCountLabel,
	    NULL);

	XtAddCallback(countFilterCountTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	string = XmStringCreateSimple("Seconds ");
	countFilterSecondsLabel = XtVaCreateManagedWidget("countFilterSecondsLabel",
	    xmLabelGadgetClass, form1,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              countFilterLabel,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             countFilterCountTextW,
	    NULL);
	XmStringFree(string);

	countFilterSecondsTextW = XtVaCreateManagedWidget("countFilterSecondsTextW",
	    xmTextFieldWidgetClass, form1,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                3,
	    XmNmaxLength,              3,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              countFilterLabel,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             countFilterSecondsLabel,
	    NULL);

	XtAddCallback(countFilterSecondsTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	/* Form is full -- now manage */
	XtManageChild(form1);

	/* ---------------------------------
	     Force Process Variable
	     --------------------------------- */

	frame2 = XtVaCreateManagedWidget("frame2",
	    xmFrameWidgetClass, form,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       prev,
	    XmNleftAttachment,  XmATTACH_FORM,
	    XmNrightAttachment,  XmATTACH_FORM,
	    NULL);

	form2 = XtVaCreateWidget("form2",
	    xmFormWidgetClass, frame2,
	    XmNspacing,          0,
	    XmNmarginHeight,     0,
	    NULL);

	string = XmStringCreateSimple("Force Process Variable");
	forcePVLabel = XtVaCreateManagedWidget("forcePVLabel",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	string = XmStringCreateSimple("PV Name ");
	forcePVnameLabel = XtVaCreateManagedWidget("forcePVnameLabel",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
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
	    XmNtopWidget,              forcePVLabel,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVnameLabel,
	    NULL);

	XtAddCallback(forcePVnameTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	string = XmStringCreateSimple("Force Mask ");
	forcePVForceMaskLabel = XtVaCreateManagedWidget("forcePVForceMaskLabel",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameTextW,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);
	prev = forcePVForceMaskLabel;

	string = XmStringCreateSimple("-----");
	forcePVForceMaskStringLabelW = XtVaCreateManagedWidget("forcePVmaskStringLabelW",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameTextW,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVForceMaskLabel,
	    NULL);
	XmStringFree(string);

	string = XmStringCreateSimple("    Force Value ");
	forcePVforceValueLabel = XtVaCreateManagedWidget("forcePVvalue",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameTextW,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVForceMaskStringLabelW,
	    NULL);
	XmStringFree(string);

	forcePVforceValueTextW = XtVaCreateManagedWidget("forcePVforceValueTextW",
	    xmTextFieldWidgetClass, form2,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                8,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameTextW,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVforceValueLabel,
	    NULL);

	XtAddCallback(forcePVforceValueTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	string = XmStringCreateSimple("     Reset Value ");
	forcePVresetValueLabel = XtVaCreateManagedWidget("forcePVresetValueLabel",
	    xmLabelGadgetClass, form2,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameTextW,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVforceValueTextW,
	    NULL);
	XmStringFree(string);

	forcePVresetValueTextW = XtVaCreateManagedWidget("forcePVresetValueTextW",
	    xmTextFieldWidgetClass, form2,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                8,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              forcePVnameTextW,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVresetValueLabel,
	    NULL);

	XtAddCallback(forcePVresetValueTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	frame3 = XtVaCreateManagedWidget("frame3",
	    xmFrameWidgetClass, form2,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       forcePVforceValueTextW,
	    XmNleftAttachment,  XmATTACH_FORM,
	    XmNrightAttachment,  XmATTACH_FORM,
	    NULL);

	form3 = XtVaCreateWidget("form2",
	    xmFormWidgetClass, frame3,
	    XmNspacing,          0,
	    XmNmarginHeight,     0,
	    NULL);

	string = XmStringCreateSimple("Force CALC");
	forcePVCalcLabel = XtVaCreateManagedWidget("forcePVCalcLabel",
	    xmLabelGadgetClass, form3,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	string = XmStringCreateSimple("       Expression ");
	forcePVCalcExpressionLabel = XtVaCreateManagedWidget("forcePVCalcExpression",
	    xmLabelGadgetClass, form3,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVCalcLabel,
	    NULL);
	XmStringFree(string);

	forcePVCalcExpressionTextW = XtVaCreateManagedWidget("forcePVCalcExpressionTextW",
	    xmTextFieldWidgetClass, form3,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                40,
	    XmNmaxLength,              100,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             forcePVCalcExpressionLabel,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);

	XtAddCallback(forcePVforceValueTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	prev = forcePVCalcExpressionTextW;

	for (i=0;i<NO_OF_CALC_PVS/2;i++) {

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
			XmNrightAttachment,        XmATTACH_POSITION,
			XmNrightPosition,          50,
		    NULL);
	
		prev = forcePVCalcPVTextW[i];
	
		XtAddCallback(forcePVCalcPVTextW[i], XmNactivateCallback,
		    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);
	}

	prev = forcePVCalcExpressionTextW;

	for (i=NO_OF_CALC_PVS/2;i<NO_OF_CALC_PVS;i++) {

		pvid[0]=letter[i];
        string = XmStringCreateSimple(pvid);
		forcePVCalcPVLabel[i] = XtVaCreateManagedWidget("forcePVCalcPVLabel",
		    xmLabelGadgetClass, form3,
		    XmNlabelString,            string,
		    XmNtopAttachment,          XmATTACH_WIDGET,
		    XmNtopWidget,              prev,
			XmNleftAttachment,         XmATTACH_POSITION,
			XmNleftPosition,           50,
/*
		    XmNleftAttachment,         XmATTACH_WIDGET,
		    XmNleftWidget,             forcePVCalcPVTextW[i-6],
*/
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
		    XmNrightAttachment,        XmATTACH_FORM,
		    NULL);
	
		prev = forcePVCalcPVTextW[i];
	
	
		XtAddCallback(forcePVCalcPVTextW[i], XmNactivateCallback,
		    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);
	}

	/* RowColumn is full -- now manage */
	XtManageChild(form3);

	/* RowColumn is full -- now manage */
	XtManageChild(form2);

	/* ----------------
	     Beep Severity
	     -------------- */
	string = XmStringCreateSimple("Beep Severity ");
	beepSeverityLabel = XtVaCreateManagedWidget("beepSeverityLabel",
	    xmLabelGadgetClass, form,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              frame2,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	beepSeverityValueTextW = XtVaCreateManagedWidget("beepSeverityValueTextW",
	    xmTextFieldWidgetClass, form,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNcolumns,                10,
	    XmNmaxLength,              10,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              frame2,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             beepSeverityLabel,
	    NULL);

	XtAddCallback(beepSeverityValueTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	/* ---------------------------------
	     Severity Process Variable
	     --------------------------------- */
	string = XmStringCreateSimple("Severity PV Name");
	severityPVlabel = XtVaCreateManagedWidget("severityPVlabel",
	    xmLabelGadgetClass, form,
	    XmNlabelString,    string,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       beepSeverityValueTextW,
	    XmNleftAttachment,  XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	severityPVnameTextW = XtVaCreateManagedWidget("severityPVnameTextW",
	    xmTextFieldWidgetClass, form,
	    XmNspacing,          0,
	    XmNmarginHeight,     0,
/*
	    XmNcolumns,          30,
	    XmNmaxLength,        PVNAME_SIZE,
*/
	    XmNbackground,       textBackground,
	    XmNtopAttachment,    XmATTACH_WIDGET,
	    XmNtopWidget,        beepSeverityValueTextW,
	    XmNleftAttachment,   XmATTACH_WIDGET,
	    XmNleftWidget,       severityPVlabel,
	    XmNrightAttachment,  XmATTACH_FORM,
	    NULL);

	XtAddCallback(severityPVnameTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	/* ---------------------------------
	     Alias
	     --------------------------------- */
	string = XmStringCreateSimple("Alias");
	aliasLabel = XtVaCreateManagedWidget("aliasLabel",
	    xmLabelGadgetClass,        form,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              severityPVnameTextW,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	/*XmNcolumns,                80,*/

	aliasTextW = XtVaCreateManagedWidget("aliasTextW",
	    xmTextFieldWidgetClass, form,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              severityPVnameTextW,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             aliasLabel,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);

	XtAddCallback(aliasTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	/* ---------------------------------
	     Related Process Command
	     --------------------------------- */
	string = XmStringCreateSimple("Related Process Command");
	processLabel = XtVaCreateManagedWidget("processLabel",
	    xmLabelGadgetClass,        form,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              aliasLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	processTextW = XtVaCreateManagedWidget("processTextW",
	    xmTextFieldWidgetClass, form,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              processLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);

	XtAddCallback(processTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	/* ---------------------------------
	     Sevr Command
	     --------------------------------- */
	string = XmStringCreateSimple("Alarm Severity Commands");
	sevrProcessLabel = XtVaCreateManagedWidget("sevrProcessLabel",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              processTextW,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	/* Create Scrolled Text  */
	n=0;
	XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); 
	n++;
	XtSetArg(args[n], XmNbackground, textBackground); 
	n++;
	XtSetArg(args[n], XmNrows, 2); 
	n++;
	sevrProcessTextW = XmCreateScrolledText(form,"sevrProcessTextW",args,n);

	XtVaSetValues(XtParent(sevrProcessTextW),
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              sevrProcessLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);

	XtManageChild(sevrProcessTextW);

	/* ---------------------------------
	     Stat Command
	     --------------------------------- */
	string = XmStringCreateSimple("Alarm Status Commands");
	statProcessLabel = XtVaCreateManagedWidget("statProcessLabel",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              XtParent(sevrProcessTextW),
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	/* Create Scrolled Text  */
	n=0;
	XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); 
	n++;
	XtSetArg(args[n], XmNbackground, textBackground); 
	n++;
	XtSetArg(args[n], XmNrows, 2); 
	n++;
	statProcessTextW = XmCreateScrolledText(form,"statProcessTextW",args,n);

	XtVaSetValues(XtParent(statProcessTextW),
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              statProcessLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);

	XtManageChild(statProcessTextW);

	/* ---------------------------------
	     Guidance URL Location
	     --------------------------------- */
	string = XmStringCreateSimple("Guidance URL");
	guidanceUrlLabel = XtVaCreateManagedWidget("guidanceUrlLabel",
	    xmLabelGadgetClass,        form,
	    XmNlabelString,            string,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              XtParent(statProcessTextW),
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	guidanceUrlW = XtVaCreateManagedWidget("guidanceUrlW",
	    xmTextFieldWidgetClass, form,
	    XmNspacing,                0,
	    XmNmarginHeight,           0,
	    XmNbackground,             textBackground,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              guidanceUrlLabel,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);

	XtAddCallback(processTextW, XmNactivateCallback,
	    (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

	/* ---------------------------------
	     Guidance Text
	     --------------------------------- */
	string = XmStringCreateSimple("Guidance Text              ");
	guidanceLabel = XtVaCreateManagedWidget("guidanceLabel",
	    xmLabelGadgetClass, form,
	    XmNlabelString,     string,
	    XmNtopAttachment,   XmATTACH_WIDGET,
	    XmNtopWidget,       guidanceUrlW,
	    XmNleftAttachment,  XmATTACH_FORM,
	    NULL);
	XmStringFree(string);

	/* Create Scrolled Text  */
	n=0;
	XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); 
	n++;
	XtSetArg(args[n], XmNbackground, textBackground); 
	n++;
	XtSetArg(args[n], XmNrows, 6); 
	n++;
	/*XtSetArg(args[n], XmNcursorPositionVisible, True); n++; */
	/*XtSetArg(args[n], XmNscrollingPolicy, XmAUTOMATIC); n++; */
	guidanceTextW = XmCreateScrolledText(form,"guidanceTextW",args,n);

	XtVaSetValues(XtParent(guidanceTextW),
	    XmNtopAttachment, XmATTACH_WIDGET,
	    XmNtopWidget, guidanceLabel,
	    XmNleftAttachment, XmATTACH_FORM,
	    XmNrightAttachment, XmATTACH_FORM,
	    XmNbottomAttachment, XmATTACH_FORM,
	    NULL);

	XtManageChild(guidanceTextW);

	if (programId != ALH) {
		/* Set the client data "Apply", "Cancel", "Dismiss" and "Help" button's callbacks. */
		prop_items_act[0].data = (XtPointer)propWindow;
		prop_items_act[1].data = (XtPointer)propWindow;
		prop_items_act[2].data = (XtPointer)propWindow;
		prop_items_act[3].data = (XtPointer)propWindow;

		(void)createActionButtons(propDialog, prop_items_act, XtNumber(prop_items_act));
	} else {
		prop_items_alh[0].data = (XtPointer)propWindow;
		prop_items_alh[1].data = (XtPointer)propWindow;

		(void)createActionButtons(propDialog, prop_items_alh, XtNumber(prop_items_alh));
	}

	XtManageChild(form);
	XtManageChild(propDialog);

	propWindow->propDialog = propDialog;
	propWindow->nameLabelW = nameLabelW;
	propWindow->nameTextW = nameTextW;
	propWindow->alarmMaskStringLabelW = alarmMaskStringLabelW;
	propWindow->resetMaskStringLabelW = resetMaskStringLabelW;
	propWindow->maskFrameW = maskFrameW;
	propWindow->beepSeverityValueTextW = beepSeverityValueTextW;
	propWindow->severityPVnameTextW = severityPVnameTextW;
	propWindow->countFilterFrame = countFilterFrame;
	propWindow->countFilterCountTextW = countFilterCountTextW;
	propWindow->countFilterSecondsTextW = countFilterSecondsTextW;
	propWindow->forcePVnameTextW = forcePVnameTextW;
	propWindow->forcePVForceMaskStringLabelW = forcePVForceMaskStringLabelW;
	propWindow->forcePVcurrentValueTextW = forcePVcurrentValueTextW;
	propWindow->forcePVforceValueTextW = forcePVforceValueTextW;
	propWindow->forcePVresetValueTextW = forcePVresetValueTextW;
	propWindow->forcePVCalcExpressionTextW = forcePVCalcExpressionTextW;
	for (i=0;i<NO_OF_CALC_PVS;i++) {
	propWindow->forcePVCalcPVTextW[i] = forcePVCalcPVTextW[i];
	}
	propWindow->aliasTextW = aliasTextW;
	propWindow->processTextW = processTextW;
	propWindow->sevrProcessTextW = sevrProcessTextW;
	propWindow->statProcessTextW = statProcessTextW;
	propWindow->guidanceTextW = guidanceTextW;
	propWindow->guidanceUrlW = guidanceUrlW;
	if (programId != ALH) {
		for (i = 0; i < ALARM_NMASK; i++){
			propWindow->alarmMaskToggleButtonW[i] = alarmMaskToggleButtonW[i];
			propWindow->forceMaskToggleButtonW[i] = forceMaskToggleButtonW[i];
		}
	}

	/* update propWindow link info */
	propEditableDialogWidgets(area);

	XtRealizeWidget(propDialogShell);

}

/******************************************************
  propMaskChangeCallback
******************************************************/
static void propMaskChangeCallback( Widget widget,XtPointer calldata,XtPointer cbs)
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
	XtFree(mask);
}

/******************************************************
  propApplyCallback
******************************************************/
static void propApplyCallback( Widget widget,XtPointer calldata,XtPointer cbs)
{
	struct propWindow *propWindow=(struct propWindow *)calldata;
	short f1, f2;
	double dbl;
	int i, rtn, rtn2;
	struct anyLine *line;
	struct chanData *cdata;
	XmString string;
	char *buff;
	struct gcData *pgcData;
	GCLINK *link;
	int linkType;
	MASK mask;
	struct guideLink *guideLink;
	FORCEPV_CALC* pcalc;

	/* PROPERTY UNDO WORKS BUT NOT IMPLEMENTED YET
	     GCLINK *undoLink=NULL;
	     int undoLinkType;
	
	     editUndoGet(&undoLink, &linkType, &link);
	*/

	link =getSelectionLinkArea(propWindow->area);
	if (!link) return;
	linkType =getSelectionLinkTypeArea(propWindow->area);
	pgcData = link->pgcData;

	/* PROPERTY UNDO WORKS BUT NOT IMPLEMENTED YET
	     propDeleteClone(undoLink,undoLinkType);
	     undoLink = propCreateClone(link,linkType);
	     undoLinkType = linkType;
	*/

	/* ---------------------------------
	     Group/Channel Name 
	     --------------------------------- */
	buff = XmTextFieldGetString(propWindow->nameTextW);
	pgcData->name = buff;

	/* ---------------------------------
	     Alarm Mask 
	     --------------------------------- */
	if (linkType == CHANNEL) {
		XtVaGetValues(propWindow->alarmMaskStringLabelW, XmNlabelString, &string, NULL);
		XmStringGetLtoR(string,XmFONTLIST_DEFAULT_TAG,&buff);
		XmStringFree(string);
		cdata = (struct chanData *)pgcData;
		alSetMask(buff,&mask);
		XtFree(buff);
		alChangeChanMask((CLINK *)link,mask);
		if (programId != ALH) cdata->defaultMask = cdata->curMask;
	}

	/* ---------------------------------
	     Beep Severity
	     --------------------------------- */
	buff = XmTextFieldGetString(propWindow->beepSeverityValueTextW);
	pgcData->beepSevr = 0;
	if (strlen(buff)) {
		for (i=1; i<ALH_ALARM_NSEV; i++) {
			if (strncmp(buff,alhAlarmSeverityString[i],
			strlen(alhAlarmSeverityString[i]))==0){
				pgcData->beepSevr = i;
			}
		}
	}


	/* ---------------------------------
	     Severity Process Variable
	     --------------------------------- */
	buff = XmTextFieldGetString(propWindow->severityPVnameTextW);
	if (strlen(buff)) pgcData->sevrPVName = buff;
	else pgcData->sevrPVName = "-";

	/* ---------------------------------
	     Alarm Count Filter
	     --------------------------------- */
	if (linkType == CHANNEL) {
		cdata = (struct chanData *)pgcData;
		if (cdata->countFilter){
		    if (cdata->countFilter->alarmTimeHistory){
                free(cdata->countFilter->alarmTimeHistory);
            }
    		free(cdata->countFilter);
    		cdata->countFilter = 0;
        }
		buff = XmTextFieldGetString(propWindow->countFilterCountTextW);
		rtn = sscanf(buff,"%hd",&f1);
		buff = XmTextFieldGetString(propWindow->countFilterCountTextW);
		rtn2 = sscanf(buff,"%hd",&f2);
		if (rtn == 1 || rtn2 ==1 ){
			cdata->countFilter = (COUNTFILTER *)calloc(1,sizeof(COUNTFILTER));
			cdata->countFilter->clink=link;
			if (rtn == 1 ) cdata->countFilter->inputCount = f1;
			else cdata->countFilter->inputCount = 1;
			if (rtn2 == 1 ) cdata->countFilter->inputSeconds = f2;
			else cdata->countFilter->inputSeconds = 1;
            if (cdata->countFilter->inputCount) cdata->countFilter->alarmTimeHistory =
                (time_t *)calloc(2*cdata->countFilter->inputCount,sizeof(time_t));
		}
	}

	/* ---------------------------------
	     Force Process Variable
	     --------------------------------- */
	if (!pgcData->pforcePV) pgcData->pforcePV=(FORCEPV*)calloc(1,sizeof(FORCEPV));
	/*  update link field  - pforcePV->name */
	buff = XmTextFieldGetString(propWindow->forcePVnameTextW);
	if (pgcData->pforcePV->name) free(pgcData->pforcePV->name);
	if (strlen(buff)) pgcData->pforcePV->name = buff;
	else pgcData->pforcePV->name = 0;

	/*  update link field  - forcePVMask */
	XtVaGetValues(propWindow->forcePVForceMaskStringLabelW, XmNlabelString, &string, NULL);
	XmStringGetLtoR(string,XmFONTLIST_DEFAULT_TAG,&buff);
	XmStringFree(string);
	alSetMask(buff,&(pgcData->pforcePV->forceMask));
	XtFree(buff);

	/*  update link field  - pforcePV->forceValue */
	buff = XmTextFieldGetString(propWindow->forcePVforceValueTextW);
	dbl=0.0;
	rtn = sscanf(buff,"%lf",&dbl);
	if (rtn == 1) pgcData->pforcePV->forceValue = dbl;
	else pgcData->pforcePV->forceValue = 1;

	/*  update link field  - pforcePV->resetValue */
	buff = XmTextFieldGetString(propWindow->forcePVresetValueTextW);
	if (strncmp(buff,"NE",2)==0 || strncmp(buff,"ne",2)==0){
		pgcData->pforcePV->resetValue = pgcData->pforcePV->forceValue;
	} else {
		dbl=0.0;
		rtn = sscanf(buff,"%lf",&dbl);
		if (rtn == 1) pgcData->pforcePV->resetValue = dbl;
		else pgcData->pforcePV->resetValue = 0;
	}

	if (!pgcData->pforcePV->pcalc)
		 pgcData->pforcePV->pcalc=(FORCEPV_CALC*)calloc(1,sizeof(FORCEPV_CALC));
	pcalc=pgcData->pforcePV->pcalc;

	buff = XmTextFieldGetString(propWindow->forcePVCalcExpressionTextW);
	if (pcalc->expression) free(pcalc->expression);
	if (strlen(buff)) pcalc->expression = buff;
	else pcalc->expression = 0;

	for (i=0;i<NO_OF_CALC_PVS;i++) {
		buff = XmTextFieldGetString(propWindow->forcePVCalcPVTextW[i]);
		if (pcalc->name[i]) free(pcalc->name[i]);
		if (strlen(buff)) pcalc->name[i] = buff;
		else pcalc->name[i] = 0;
	}

	/* ---------------------------------
	     Alias
	     --------------------------------- */
	buff = XmTextFieldGetString(propWindow->aliasTextW);
	if (strlen(buff)) pgcData->alias = buff;
	else pgcData->alias = 0;

	/* ---------------------------------
	     Related Process Command
	     --------------------------------- */
	buff = XmTextFieldGetString(propWindow->processTextW);
	if (strlen(buff)) pgcData->command = buff;
	else pgcData->command = 0;

	/* ---------------------------------
	     Sevr Commands
	     --------------------------------- */
	ellInit(&(pgcData->sevrCommandList));

	buff = XmTextGetString(propWindow->sevrProcessTextW);
	if (strlen(buff)){
		while (TRUE) {
			addNewSevrCommand(&pgcData->sevrCommandList,buff);
			buff=strchr(buff,'\n');
			if ( !buff ) break;
			*buff='\0';
			buff++;
		}
	}

	/* ---------------------------------
	     Stat Commands
	     --------------------------------- */
	if (linkType == CHANNEL) {
		cdata = (struct chanData *)pgcData;
		ellInit(&(cdata->statCommandList));

		buff = XmTextGetString(propWindow->statProcessTextW);
		if (strlen(buff)){
			while (TRUE) {
				addNewStatCommand(&cdata->statCommandList,buff);
				buff=strchr(buff,'\n');
				if ( !buff ) break;
				*buff='\0';
				buff++;
			}
		}
	}

	/* ---------------------------------
	     Guidance URL
	     --------------------------------- */
	buff = XmTextFieldGetString(propWindow->guidanceUrlW);
	if (strlen(buff)) link->guidanceLocation = buff;
	else link->guidanceLocation = 0;

	/* ---------------------------------
	     Guidance Text
	     --------------------------------- */
	sllInit(&(link->GuideList));

	buff = XmTextGetString(propWindow->guidanceTextW);
	if (strlen(buff)){
		while (TRUE) {
			guideLink = (struct guideLink *)calloc(1,sizeof(struct guideLink));
			guideLink->list=buff;
			sllAdd(&(link->GuideList),(SNODE *)guideLink);
			buff=strchr(buff,'\n');
			if ( !buff ) break;
			*buff='\0';
			buff++;
		}
	}

	/* ---------------------------------
	     Update dialog windows
	     --------------------------------- */
	/*  update properties dialog window field */
	axUpdateDialogs(propWindow->area);

	/*  update group line data and tree window */
	line = link->lineGroupW;
	if (line && line->link==link ){
		line->pname = ((GCLINK *)link)->pgcData->name;
		if (((GCLINK *)link)->pgcData->alias){
			line->alias = ((GCLINK *)link)->pgcData->alias;
		} else {
			line->alias = ((GCLINK *)link)->pgcData->name;
		}
		markSelectedWidget(((ALINK *)propWindow->area)->groupWindow, 0);
		awRowWidgets(line, propWindow->area);
		markSelectedWidget( ((ALINK *)propWindow->area)->groupWindow, ((WLINE *)line->wline)->name);
	}

	/*  update tree line data and group window */
	line = link->lineTreeW;
	if (line && line->link==link ){
		line->pname = ((GCLINK *)link)->pgcData->name;
		if (((GCLINK *)link)->pgcData->alias){
			line->alias = ((GCLINK *)link)->pgcData->alias;
		} else {
			line->alias = ((GCLINK *)link)->pgcData->name;
		}
		awRowWidgets(line, propWindow->area);
	}

	/* ---------------------------------
	     set undo data
	     --------------------------------- */
	/* PROPERTY UNDO WORKS BUT NOT IMPLEMENTED YET
	     editUndoSet(undoLink,linkType, link, MENU_EDIT_UNDO_PROPERTIES, FALSE);
	*/
}

/******************************************************
  propHelpCallback
******************************************************/
static void propHelpCallback( Widget widget,XtPointer calldata,XtPointer cbs)
{
	char *messageALH1 =
	"This dialog window allows an operator to view the alarm properties\n"
	"for a group or channel.\n"
	"  \n"
	"Press the Dismiss button to close the Properties dialog window.\n"
	"Press the Help    button to get this help description window.\n"
	;
	char * messageALH2 = "  ";

	char *messageACT1 =
	"This dialog window allows an operator to view and change alarm properties\n"
	"for a group or channel.\n"
	"  \n"
	"Press the Apply   button to change the properties for the selected group or channel.\n"
	"Press the Reset   button to reset the properties to their initial values.\n"
	"Press the Dismiss button to close the Properties dialog window.\n"
	"Press the Help    button to get this help description window.\n"
	;
	char * messageACT2 = "  ";

	if (programId == ALH) {
		createDialog(widget,XmDIALOG_INFORMATION, messageALH1,messageALH2);
	} else {
		createDialog(widget,XmDIALOG_INFORMATION, messageACT1,messageACT2);
	}

}

/******************************************************
  propDismissCallback
******************************************************/
static void propDismissCallback( Widget widget,XtPointer calldata,XtPointer cbs)
{
	struct propWindow *propWindow=(struct propWindow *)calldata;
	Widget propDialog;

	propDialog = propWindow->propDialog;
	XtUnmanageChild(propDialog);
	XUnmapWindow(XtDisplay(propDialog), XtWindow(XtParent(propDialog)));
	if (propWindow->menuButton)
		XtVaSetValues(propWindow->menuButton, XmNset, FALSE, NULL);
}

/******************************************************
  propCancelCallback
******************************************************/
static void propCancelCallback( Widget widget,XtPointer calldata,XtPointer cbs)
{
	struct propWindow *propWindow=(struct propWindow *)calldata;
	propUpdateDialog((ALINK *)(propWindow->area));
}

/******************************************************
  propUndo
******************************************************/
void propUndo(area)
void *area;
{
	GCLINK *link;
	int linkType;
	GCLINK *undoLink;
	struct anyLine *line;
	GCLINK *tempLink;
	GLINK *glink;
	CLINK *clink;
	struct chanData *pchanData;
	struct groupData *pgroupData;

	editUndoGet(&undoLink, &linkType, &link);

	if (!link) return;
	tempLink = propCreateClone(link,linkType);

	if (linkType == GROUP) {
		glink=(GLINK *)link;
		pgroupData = glink->pgroupData;
		*glink = *(GLINK*)undoLink;
		*pgroupData = *(struct groupData *)undoLink->pgcData;
		glink->pgroupData = pgroupData;
		free((struct groupData *)undoLink->pgcData);
		free((GLINK*)undoLink);
	}

	if (linkType == CHANNEL) {
		clink=(CLINK *)link;
		pchanData = clink->pchanData;
		*clink = *(CLINK *)undoLink;
		*pchanData = *(struct chanData *)undoLink->pgcData;
		clink->pchanData = pchanData;
		free((struct chanData *)undoLink->pgcData);
		free((CLINK*)undoLink);
	}
	undoLink = tempLink;

	/* ---------------------------------
	     Update dialog windows
	     --------------------------------- */
	axUpdateDialogs(area);

	/* ---------------------------------
	     Update tree line data and tree window 
	     --------------------------------- */
	line = link->lineGroupW;
	if (line && line->link==link ){
		line->pname = ((GCLINK *)link)->pgcData->name;
		if (((GCLINK *)link)->pgcData->alias){
			line->alias = ((GCLINK *)link)->pgcData->alias;
		} else {
			line->alias = ((GCLINK *)link)->pgcData->name;
		}
		awRowWidgets(line,area);
	}

	/* ---------------------------------
	     Update group line data and group window
	     --------------------------------- */
	line = link->lineTreeW;
	if (line && line->link==link ){
		line->pname = ((GCLINK *)link)->pgcData->name;
		if (((GCLINK *)link)->pgcData->alias){
			line->alias = ((GCLINK *)link)->pgcData->alias;
		} else {
			line->alias = ((GCLINK *)link)->pgcData->name;
		}
		awRowWidgets(line,area);
	}

	/* ---------------------------------
	     set undo data
	     --------------------------------- */
	editUndoSet(undoLink,linkType, link, MENU_EDIT_UNDO_PROPERTIES, FALSE);

}

/******************************************************
  propEditableDialogWidgets
******************************************************/
static void propEditableDialogWidgets(ALINK  *area)
{
	struct propWindow *propWindow;

	propWindow = (struct propWindow *)area->propWindow;

	if (programId == ALH) {
		XtVaSetValues(propWindow->nameTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->beepSeverityValueTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->severityPVnameTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->countFilterCountTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->countFilterSecondsTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->forcePVnameTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->forcePVforceValueTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->forcePVresetValueTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->aliasTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->processTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->sevrProcessTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->statProcessTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->guidanceTextW,XmNeditable, FALSE, NULL);
		XtVaSetValues(propWindow->guidanceUrlW,XmNeditable, FALSE, NULL);
	} else {
		XtVaSetValues(propWindow->nameTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->beepSeverityValueTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->severityPVnameTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->countFilterCountTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->countFilterSecondsTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->forcePVnameTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->forcePVforceValueTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->forcePVresetValueTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->aliasTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->processTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->sevrProcessTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->statProcessTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->guidanceTextW,XmNeditable, TRUE, NULL);
		XtVaSetValues(propWindow->guidanceUrlW,XmNeditable, TRUE, NULL);
	}
	return;
}

/******************************************************
  propDeleteClone
******************************************************/
static void propDeleteClone(GCLINK *link,int linkType)
{
	SNODE *pt,*next;
	struct guideLink *guidelist;
	struct gcData *pgcData;
	struct chanData *pcData;
	struct groupData *pgData;

	if (link == NULL) return;

	pgcData = link->pgcData;
	if (pgcData->name) free(pgcData->name);
	if (pgcData->pforcePV) alForcePVDelete(&pgcData->pforcePV);
	if (strcmp(pgcData->sevrPVName,"-") != 0) free(pgcData->sevrPVName);
	if (pgcData->command) free(pgcData->command);
	if (pgcData->alias) free(pgcData->alias);
	removeSevrCommandList(&pgcData->sevrCommandList);
	if (linkType == CHANNEL) {
		pcData = (struct chanData *)pgcData;
		if (pcData->countFilter){
		    if (pcData->countFilter->alarmTimeHistory){
                free(pcData->countFilter->alarmTimeHistory);
            }
    		free(pcData->countFilter);
    		pcData->countFilter = 0;
        }
		removeStatCommandList(&pcData->statCommandList);
	}
	if (linkType == GROUP) {
		pgData = (struct groupData *)pgcData;
		if (pgData->treeSym) free(pgData->treeSym);
	}

	if (link->guidanceLocation) free(link->guidanceLocation);

	pt = sllFirst(&link->GuideList);
	while (pt) {
		next = sllNext(pt);
		guidelist = (struct guideLink *)pt;
		free(guidelist->list);
		free(guidelist);
		pt = next;
	}
	free(pgcData);
	free(link);
}

/******************************************************
  propCreateClone
******************************************************/
static GCLINK *propCreateClone(GCLINK *link,int linkType)
{
	CLINK *newChan;
	struct chanData *pchanData;
	GLINK *newGroup;
	struct groupData *pgroupData;

        /* pointer contents not copied */
	if (linkType == GROUP) {
		newGroup = alCreateGroup();
		pgroupData = newGroup->pgroupData;
		*newGroup = *(GLINK *)link;
		*pgroupData = *(struct groupData *)link->pgcData;
		newGroup->pgroupData = pgroupData;
		return (GCLINK *)newGroup;
	}

	if (linkType == CHANNEL) {
		newChan = alCreateChannel();
		pchanData = newChan->pchanData;
		*newChan = *(CLINK *)link;
		*pchanData = *(struct chanData *)link->pgcData;
		newChan->pchanData = pchanData;
		return (GCLINK *)newChan;
	}
	return NULL;
}

