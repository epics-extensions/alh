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
/* axArea.c */

/************************DESCRIPTION***********************************
  Routines for main window widgets
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>

#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/CascadeBG.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Scale.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>

#include "cadef.h"
#include "alarm.h"
#include "ALH.bit"

#include "alh.h"
#include "line.h"
#include "axArea.h"
#include "sllLib.h"
#include "ax.h"

char *silenceString[] = {"Off","On"};
const char *executionModeString[] = {"Local","Global"};
const char *executionStateString[] = {"Active","Passive"};
char *disabledForcePVCountString[] = {" ","Disabled forcePVs:"};

/* global variables */
extern int _global_flag;
extern int _passive_flag;
extern int toBeConnectedCount;
extern int DEBUG;
extern SLIST *areaList;
extern struct setup psetup;
extern Display *display;
extern char *alhAlarmSeverityString[];
extern int _main_window_flag;
extern int (*default_display_filter)(GCLINK *);

ALINK *alhArea;

/* forward definitions */
static ALINK *setupArea( ALINK *areaOld);
static void scale_callback(Widget widget,ALINK *area,XmScaleCallbackStruct *cbs);


/******************************************************
  buildPulldownMenu - Generic menu creation
******************************************************/
Widget buildPulldownMenu( Widget parent, char *menu_title, char menu_mnemonic,
int tearOff, MenuItem *items, XtPointer user_data)
{
	Widget PullDown, cascade, widget;
	int i;
	XmString str;
    WidgetClass *xmclass[] = { &xmPushButtonGadgetClass, &xmSeparatorGadgetClass, &xmToggleButtonGadgetClass };

	/* Pulldown menus are built from cascade buttons, so this function
	      * also includes pullright menus.  Create the menu, the cascade button
	      * that owns the menu, and then the submenu items.
	      */
	PullDown = XmCreatePulldownMenu(parent, "_pulldown", NULL, 0);

	str = XmStringCreateSimple(menu_title);
	cascade = XtVaCreateManagedWidget(menu_title,
	    xmCascadeButtonGadgetClass, parent,
	    XmNsubMenuId,   PullDown,
	    XmNlabelString, str,
	    XmNmnemonic,    menu_mnemonic,
	    NULL);
	XmStringFree(str);

#if  XmVersion && XmVersion >= 1002
	if (tearOff){
		/* Enable pulldown menu tearoff functionality */
		XtVaSetValues(PullDown, XmNtearOffModel, XmTEAR_OFF_ENABLED, NULL);
	}
#endif

	/* Now add the menu items */
	for (i = 0; items[i].label != NULL; i++) {
		/* If subitems exist, create the pull-right menu by calling this
		         * function recursively.  Since the function returns a cascade
		         * button, the widget returned is used..
		         */
		if (items[i].subitems)
			widget = buildPulldownMenu(PullDown,
			    items[i].label, items[i].mnemonic, FALSE, items[i].subitems, user_data);
		else {
			widget = XtVaCreateManagedWidget(items[i].label,
			    *xmclass[items[i].class], PullDown,
			    XmNuserData,    user_data,
			    NULL);

			/* Make spacing of toggle button items the same as pushButtons */
			if (xmclass[items[i].class] == &xmToggleButtonWidgetClass ||
			    xmclass[items[i].class] == &xmToggleButtonGadgetClass)
				XtVaSetValues(widget, XmNmarginHeight, 1, NULL);

			/* Set initial state of of toggle button items */
			if (xmclass[items[i].class] == &xmToggleButtonWidgetClass ||
			    xmclass[items[i].class] == &xmToggleButtonGadgetClass)
				XmToggleButtonSetState(widget,(Boolean)items[i].initial_state,FALSE);
		}
		/* Whether the item is a real item or a cascade button with a
		         * menu, it can still have a mnemonic.  */
		if (items[i].mnemonic)
			XtVaSetValues(widget, XmNmnemonic, items[i].mnemonic, NULL);
		/* any item can have an accelerator, except cascade menus. But,
		         * we don't worry about that; we know better in our declarations.  */
		if (items[i].accelerator) {
			str = XmStringCreateSimple(items[i].accel_text);
			XtVaSetValues(widget,
			    XmNaccelerator, items[i].accelerator,
			    XmNacceleratorText, str,
			    NULL);
			XmStringFree(str);
		}
		/* again, anyone can have a callback -- however, this is an
		         * activate-callback.  This may not be appropriate for all items.
		         */
		if (items[i].callback)
			XtAddCallback(widget,
			    (xmclass[items[i].class] == &xmToggleButtonWidgetClass ||
			    xmclass[items[i].class] == &xmToggleButtonGadgetClass)?
			    XmNvalueChangedCallback : /* ToggleButton class */
			XmNactivateCallback,      /* PushButton class */
			items[i].callback, items[i].callback_data);
	}
	return cascade;
}

/***************************************************
  setupConfig -  Setup area with new config.file
****************************************************/
void setupConfig(char *filename,int program,ALINK *areaOld)
{
	ALINK     *area;
	struct mainGroup *pmainGroup = NULL;
	XmString    str;
	SNODE *proot;
	static int firstTime = TRUE;
	int beepSevrOld;
	int holdToBeConnectedCount;

	/* initialize channel access */
	if (program == ALH) {
		if (firstTime) {
			firstTime = FALSE;
			alCaInit();
		}
	}

	/* create main group */
	pmainGroup = alAllocMainGroup();
	if (!pmainGroup ) {
		if (areaOld)
			createDialog(areaOld->form_main,XmDIALOG_ERROR,
				"mainGroup allocation error: ",filename);
		return;
	}

	/* reinitialize beep severity */
	beepSevrOld = psetup.beepSevr;
	psetup.beepSevr = 1;

	/* Read the config file  or create a minimal config file  */
	if (filename[0] != '\0'){
		if ( program == ALH) {

			toBeConnectedCount =0;
			alGetConfig(pmainGroup,filename,CA_CONNECT_YES);

			/* now lets give the connection layer a little time
			                * to establish communications */
			holdToBeConnectedCount = toBeConnectedCount;
			while (TRUE) {
				alCaPend(1.0);
				if (toBeConnectedCount == holdToBeConnectedCount) break;
				holdToBeConnectedCount = toBeConnectedCount;
			}

			alSetNotConnected((SLIST *)pmainGroup);
			alPutGblAckT((SLIST *)pmainGroup);
		}
		else alGetConfig(pmainGroup,filename,CA_CONNECT_NO);

	} else{
		if (program == ACT) alCreateConfig(pmainGroup);
		else {
			free(pmainGroup);
			pmainGroup->p1stgroup = alCopyGroup(areaOld->pmainGroup->p1stgroup);
			alSetPmainGroup(pmainGroup->p1stgroup, pmainGroup);
			return;
		}
	}
	if ( sllFirst(pmainGroup)) {

		/* Log new configfile filename */
		alLogSetupConfigFile(filename);

		if (filename[0] != '\0') strcpy(psetup.configFile,filename);

		proot = sllFirst(pmainGroup);

		/*  initialize unused area  */
		area = setupArea(areaOld);

		/* initialize subWindow create/modify line routines  */
		setLineRoutine(area,area->treeWindow,program);
		setLineRoutine(area,area->groupWindow,program);

		pmainGroup->area = area;
		area->programId = program;
		area->pmainGroup = pmainGroup;
		area->changed = FALSE;

		/* activate runtime window for ALH */
		if (program == ALH){
			area->blinkString = alAlarmGroupName((GLINK *)proot);
			alHighestSystemSeverity((GLINK *)proot);
			if (_main_window_flag) showMainWindow(area);
			else createRuntimeWindow(area);
		}

		/* create/display main window for ACT */
		if (program == ACT )  showMainWindow(area);

		createConfigDisplay(area,EXPANDCOLLAPSE1);

		if (program == ALH ) alhArea = area;

		area->managed = TRUE;

		/* update filename string on main window */
		if (area->label_filename){
			str = XmStringCreateSimple(psetup.configFile);
			XtVaSetValues(area->label_filename,
			    XmNlabelString,            str,
			    NULL);
			XmStringFree(str);
		}

		/* update dialog windows */
		axUpdateDialogs(area);

		/* unmap dialog */
		createDialog(0,0," "," ");

	} else {

		psetup.beepSevr = beepSevrOld;

		if (areaOld) {
			areaOld->managed = TRUE;

			if (areaOld->form_main )
				createDialog(areaOld->form_main,XmDIALOG_WARNING,
					"Configuration file error: ",filename);
			else if (areaOld->runtimeForm)
				createDialog(areaOld->runtimeForm,XmDIALOG_WARNING,
					"Configuration file error: ",filename);
		} else {
			area = NULL;
			errMsg ("ALH Error: Invalid config file: %s\n",filename);
			exit_quit(NULL, area, NULL);
		}
	}
	return;
}

/******************************************************
  markActiveWidget - Mark active widget border
******************************************************/
void markActiveWidget(ALINK *area,Widget newWidget)
{

	/*   NOTE: this routine not implemented yet. Border looks YUK! 
	
	     if (area->selectionWidget == newWidget ) return;
	
	     if (area->selectionWidget) {
	          XtVaSetValues(area->selectionWidget,
	               XmNborderWidth,            0,
	               NULL);
	     }
	
	     if (newWidget) {
	          XtVaSetValues(newWidget, 
	               XmNborderWidth,            1,
	               NULL);
	     }
	
	     area->selectionWidget = newWidget;
	*/
}

/******************************************************
  setupArea - Initialize an area 
******************************************************/

static ALINK *setupArea(ALINK *areaOld)
{
	ALINK *area;
	SNODE *proot;

	if (!areaOld){
		/* create work area linked list */
		if (!areaList){
			areaList = (SLIST *)calloc(1,sizeof(SLIST));
			sllInit(areaList);
		}

		/* find an unused existing work area */
		area = (ALINK *)sllFirst(areaList);
		while (area){
			if (area->managed == FALSE || area->pmainGroup == NULL) break;
			area = (ALINK *)sllNext(area);
		}
	} else area = areaOld;

	/* reinitialize an unused existing work area */
	if (area){

		area->managed = FALSE;

		/*  unmark selected widgets */
		markSelectedWidget(area->treeWindow,NULL);
		markSelectedWidget(area->groupWindow,NULL);
		markActiveWidget(area,0);

		/* Delete the current config */
		if (area->pmainGroup){

			/* cancel channel access */
			alCaCancel((SLIST *)area->pmainGroup);

			proot = sllFirst(area->pmainGroup);
			if (proot) alDeleteGroup((GLINK *)proot);

			free((SLIST *)area->pmainGroup);
			area->pmainGroup = NULL;
		}

		/* or create a new work area */
	} else {
		area=(ALINK *)calloc(1,sizeof(ALINK));
		sllAdd(areaList,(SNODE *)area);

		/* Set alarm  filter to default value */
		area->viewFilter = default_display_filter;

		area->blinkString = NULL;

		area->runtimeToplevel = NULL;
		area->treeWindow = createSubWindow(area);
		area->groupWindow = createSubWindow(area);
	}

	/* Reset count of disabled forcePVs  */
	area->disabledForcePVCount = 0;

	/* Reset data for Current alarm window */
	resetCurrentAlarmWindow(area);

	/* Make NULL the selected group for Area */
	area->selectionLink = NULL;

	/* initialize the subWindows */
	initializeSubWindow(area->treeWindow);
	initializeSubWindow(area->groupWindow);

	return area;
}

/******************************************************
  createMainWindowWidgets - Create area Main Window widgets
******************************************************/
void createMainWindowWidgets(ALINK *area)
{
	char actTitle[] = "Alarm Configuration Tool";
	char alhTitle[] = "Alarm Handler";
	char execMode[50] = "Execution Status:  ";
	XmString    str;
	char   *app_name;
	char   *title_str;
	int len = 0;
	Widget label_executionMode;

	if (area->toplevel) return;

	/* create toplevel shell */
	app_name = (char*) calloc(1,strlen(programName)+6);
	strcpy(app_name, programName);
	strcat(app_name, "-main");

	area->toplevel = XtAppCreateShell(app_name, programName,
	    applicationShellWidgetClass, display, NULL, 0);

	free(app_name);

	if (area->blinkString) len = strlen(area->blinkString);
	if (area->programId == ACT){
	   title_str = (char*) calloc(1,strlen(actTitle)+len+3);
	   strcpy(title_str, actTitle);
	} else {
	   title_str = (char*) calloc(1,strlen(alhTitle)+len+3);
	   strcpy(title_str, alhTitle);
	}
	if (area->blinkString){
		strcat(title_str, ": ");
		strcat(title_str, area->blinkString);
	}

	XtVaSetValues(area->toplevel,
	    XmNtitle,     title_str,
	    XmNiconName,  area->blinkString,
	    NULL);

	free(title_str);

	/* Create form_main for toplevel */
	area->form_main = XtVaCreateManagedWidget("form_main",
	    xmFormWidgetClass,         area->toplevel,
	    XmNuserData,               (XtPointer)area,
	    NULL);

	pixelData(area->form_main);

	if (!ALH_pixmap) axMakePixmap(area->form_main);

#ifndef WIN32
	XtVaSetValues(area->toplevel, XmNiconPixmap, ALH_pixmap, NULL);
#endif

	/* Modify the window manager menu "close" callback */
	{
		Atom         WM_DELETE_WINDOW;
		XtVaSetValues(area->toplevel,
		    XmNdeleteResponse,       XmDO_NOTHING,
		    NULL);
		WM_DELETE_WINDOW = XmInternAtom(XtDisplay(area->form_main),
		    "WM_DELETE_WINDOW", False);
		if (_main_window_flag)
			XmAddWMProtocolCallback(area->toplevel,WM_DELETE_WINDOW,
		    	(XtCallbackProc) exit_quit, (XtPointer) area);
		else
			XmAddWMProtocolCallback(area->toplevel,WM_DELETE_WINDOW,
		    	(XtCallbackProc) unmapArea_callback, (XtPointer) area->form_main);
	}

	/* Create MenuBar */
	if (area->programId == ACT){
		area->menubar = actCreateMenu(area->form_main, (XtPointer)area);
	} else {
		area->menubar = alhCreateMenu(area->form_main, (XtPointer)area);
	}

	/* Create message Area Form  */
	area->messageArea = XtVaCreateWidget("message_area",
	    xmFormWidgetClass, area->form_main,
	    XmNallowOverlap,           FALSE,
	    XmNbottomAttachment,       XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);

	/* Create scale in the MessageArea to change display split  */
	area->scale = XtVaCreateManagedWidget("scale",
	    xmScaleWidgetClass, area->messageArea,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNrightAttachment,        XmATTACH_FORM,
	    XmNorientation,            XmHORIZONTAL,
	    XmNuserData,               (XtPointer)area,
	    NULL);

	/* Add scale valueChanged Callback */
	XtAddCallback(area->scale, XmNvalueChangedCallback, (XtCallbackProc)scale_callback, area);

	/* Add scale drag Callback */
	XtAddCallback(area->scale, XmNdragCallback, (XtCallbackProc)scale_callback, area);

	/* Create execution mode indicator label for the messageArea */
	strcat(execMode,executionModeString[_global_flag]);
	strcat(execMode," ");
	strcat(execMode,executionStateString[_passive_flag]);
	label_executionMode = XtVaCreateManagedWidget(
	    execMode,
	    xmLabelGadgetClass,        area->messageArea,
	    XmNmarginHeight,           3,
	    XmNalignment,              XmALIGNMENT_BEGINNING,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->scale,
	    XmNleftAttachment,         XmATTACH_POSITION,
	    XmNleftPosition,           1,
	    NULL);

	/* Create group alarm decoder label for the messageArea */
	area->label_mask = XtVaCreateManagedWidget(
	    "Mask <CDATL>:  <Cancel,Disable,noAck,noackT,noLog>",
	    xmLabelGadgetClass,        area->messageArea,
	    XmNmarginHeight,           3,
	    XmNalignment,              XmALIGNMENT_BEGINNING,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              label_executionMode,
	    XmNleftAttachment,         XmATTACH_POSITION,
	    XmNleftPosition,           1,
	    NULL);

	/* Create group alarm decoder label for the messageArea */
	area->label_groupAlarm = XtVaCreateManagedWidget(
	    "Group Alarm Counts:  (ERROR,INVALID,MAJOR,MINOR,NOALARM)",
	    xmLabelGadgetClass,        area->messageArea,
	    XmNmarginHeight,           3,
	    XmNalignment,              XmALIGNMENT_BEGINNING,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->label_mask,
	    XmNleftAttachment,         XmATTACH_POSITION,
	    XmNleftPosition,           1,
	    NULL);

	/* Create filenameTitle label for the messageArea */
	area->label_channelAlarm = XtVaCreateManagedWidget(
	    "Channel Alarm Data:  <Status,Severity>,<Unack Severity>",
	    xmLabelGadgetClass,        area->messageArea,
	    XmNmarginHeight,           3,
	    XmNalignment,              XmALIGNMENT_BEGINNING,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->label_groupAlarm,
	    XmNleftAttachment,         XmATTACH_POSITION,
	    XmNleftPosition,           1,
	    NULL);

	/* Create filenameTitle label for the messageArea */
	area->label_filenameTitle = XtVaCreateManagedWidget("Filename:  ",
	    xmLabelGadgetClass,        area->messageArea,
	    XmNmarginHeight,           3,
	    XmNalignment,              XmALIGNMENT_BEGINNING,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->label_channelAlarm,
	    XmNleftAttachment,         XmATTACH_POSITION,
	    XmNleftPosition,           1,
	    NULL);


	/* Create filename label for the messageArea */
	str = XmStringCreateSimple(psetup.configFile);
	area->label_filename = XtVaCreateManagedWidget("label_filename",
	    xmLabelGadgetClass,        area->messageArea,
	    XmNlabelString,            str,
	    XmNshadowThickness,        2,
	    XmNalignment,              XmALIGNMENT_BEGINNING,
	    /*
	          xmTextWidgetClass,         area->messageArea,
	          XmNeditable,               FALSE,
	          XmNmarginHeight,           0,
	          XmNresizeWidth,            FALSE,
	          XmNvalue,                  psetup.configFile,
	          XmNcursorPositionVisible,  FALSE,
	          XmNrightAttachment,        XmATTACH_POSITION,
	          XmNrightPosition,           50,
	*/
	XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->label_channelAlarm,
	    XmNleftAttachment,         XmATTACH_WIDGET,
	    XmNleftWidget,             area->label_filenameTitle,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);
	XmStringFree(str);


	/* Create a Silence One Hour Toggle Button in the messageArea */
	area->silenceOneHour = XtVaCreateManagedWidget("SilenceOneHour",
	    xmToggleButtonGadgetClass, area->messageArea,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->scale,
	    XmNrightAttachment,        XmATTACH_FORM,
	    XmNuserData,               (XtPointer)area,
	    NULL);

	/* Create a Silence Current Toggle Button in the messageArea */
	area->silenceCurrent = XtVaCreateManagedWidget("SilenceCurrent",
	    xmToggleButtonGadgetClass, area->messageArea,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->silenceOneHour,
	    XmNrightAttachment,        XmATTACH_FORM,
	    XmNuserData,               (XtPointer)area,
	    NULL);

	/* Create SilenceForever string for the messageArea */
	str = XmStringCreateSimple(silenceString[psetup.silenceForever]);
	area->silenceForever = XtVaCreateManagedWidget("silenceForever",
	    xmLabelGadgetClass,        area->messageArea,
	    XmNlabelString,            str,
	    XmNshadowThickness,        2,
	    XmNalignment,              XmALIGNMENT_BEGINNING,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->silenceCurrent,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);
	XmStringFree(str);

	/* Create SilenceForeverLabel string for the messageArea */
	str = XmStringCreateSimple("Silence Forever: ");
	area->silenceForeverLabel = XtVaCreateManagedWidget("silenceForeverLabel",
	    xmLabelGadgetClass,        area->messageArea,
	    XmNshadowThickness,        2,
	    XmNlabelString,            str,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->silenceCurrent,
	    XmNrightAttachment,        XmATTACH_WIDGET,
	    XmNrightWidget,            area->silenceForever,
	    NULL);
	XmStringFree(str);

	/* Create BeepSeverity string for the messageArea */
	str = XmStringCreateSimple(alhAlarmSeverityString[psetup.beepSevr]);
	area->beepSeverity = XtVaCreateManagedWidget("beepSeverity",
	    xmLabelGadgetClass,        area->messageArea,
	    XmNlabelString,            str,
	    XmNshadowThickness,        2,
	    XmNalignment,              XmALIGNMENT_BEGINNING,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->silenceForever,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);
	XmStringFree(str);

	/* Create BeepSeverityLabel string for the messageArea */
	str = XmStringCreateSimple("ALH Beep Severity:");
	area->beepSeverityLabel = XtVaCreateManagedWidget("beepSeverityLabel",
	    xmLabelGadgetClass,        area->messageArea,
	    XmNshadowThickness,        2,
	    XmNlabelString,            str,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->silenceForever,
	    XmNrightAttachment,        XmATTACH_WIDGET,
	    XmNrightWidget,            area->beepSeverity,
	    NULL);
	XmStringFree(str);

	XtAddCallback(area->silenceOneHour, XmNvalueChangedCallback,
	    (XtCallbackProc)silenceOneHour_callback,area);

	XtAddCallback(area->silenceCurrent, XmNvalueChangedCallback,
	    (XtCallbackProc)silenceCurrent_callback,0);

	/* Create Disabled ForcePV Count string for the messageArea */
	str = XmStringCreateSimple(disabledForcePVCountString[0]);
	area->disabledForcePVCountLabel = XtVaCreateManagedWidget("disabledForcePVCountLabel",
	    xmLabelGadgetClass,        area->messageArea,
	    XmNshadowThickness,        2,
	    XmNlabelString,            str,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->beepSeverity,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);
	XmStringFree(str);

	/* manage the message area */
	XtManageChild(area->messageArea);

	/* Create a Form for the treeWindow */
	area->treeWindowForm = XtVaCreateManagedWidget("treeForm",
	    xmFormWidgetClass,         area->form_main,
	    XmNborderWidth,            1,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->menubar,
	    XmNbottomAttachment,       XmATTACH_WIDGET,
	    XmNbottomWidget,           area->messageArea,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNrightAttachment,        XmATTACH_POSITION,
	    XmNrightPosition,          50,
	    NULL);

	XtAddEventHandler(area->form_main, StructureNotifyMask,
	    FALSE, (XtEventHandler)exposeResizeCallback, (XtPointer *)area->treeWindow);

	createSubWindowWidgets(area->treeWindow,area->treeWindowForm);

	/* Create a Form for the groupWindow */
	area->groupWindowForm = XtVaCreateManagedWidget("groupForm",
	    xmFormWidgetClass,         area->form_main,
	    XmNborderWidth,            1,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              area->menubar,
	    XmNbottomAttachment,       XmATTACH_WIDGET,
	    XmNbottomWidget,           area->messageArea,
	    XmNleftAttachment,         XmATTACH_POSITION,
	    XmNleftPosition,           50,
	    XmNrightAttachment,        XmATTACH_FORM,
	    NULL);

	XtAddEventHandler(area->form_main, StructureNotifyMask,
	    FALSE, (XtEventHandler)exposeResizeCallback, (XtPointer *)area->groupWindow);

	createSubWindowWidgets(area->groupWindow,area->groupWindowForm);

}

/******************************************************
  createMainWindow_callback
******************************************************/
void showMainWindow(ALINK *area)
{
	if (area->toplevel == 0){
		area->mapped = FALSE;
		createMainWindowWidgets(area);
		XtRealizeWidget(area->toplevel);
	}
	if (area->mapped == FALSE){
		XMapWindow(XtDisplay(area->toplevel),XtWindow(area->toplevel));
		area->mapped = TRUE;
		redraw(area->treeWindow,0);

		/* mark first line as treeWindow selection */
		defaultTreeSelection(area);
	}
	else {
		XRaiseWindow(XtDisplay(area->toplevel), XtWindow(area->toplevel));
		redraw(area->treeWindow,0);
	}
}

/***************************************************
 isTreeWindow - Returns TRUE if area is a treeWindow 
****************************************************/
int isTreeWindow(ALINK *area,void * subWindow)
{
	if (area->treeWindow == subWindow ) return(TRUE);
	else return(FALSE);
}

/******************************************************
  unmapArea_callback
******************************************************/
void unmapArea_callback(Widget main,Widget w,XmAnyCallbackStruct *cbs)
{
	ALINK *area;

	XtVaGetValues(w, XmNuserData, &area, NULL);

	XUnmapWindow(XtDisplay(main),XtWindow(main));

	area->mapped = FALSE;

}
/******************************************************
  scale_callback - Scale moved callback
******************************************************/
static void scale_callback(Widget widget,ALINK *area,XmScaleCallbackStruct *cbs)
{
	int value;

	value = Mmin(95,Mmax(5,cbs->value));

	XtVaSetValues(area->groupWindowForm,
	    XmNleftPosition, value,
	    NULL);
	XtVaSetValues(area->treeWindowForm,
	    XmNrightPosition, value,
	    NULL);
}

/***************************************************
  markSelectionArea - Set area selection values
****************************************************/
void markSelectionArea(ALINK *area,struct anyLine *line)
{
	/* Save selection data */
	if (!line){
		area->selectionLink = 0;
		area->selectionType = 0;
		area->selectionWindow = 0;
	} else {
		area->selectionLink = line->link;
		area->selectionType = line->linkType;
		area->selectionWindow = (void *)line->pwindow;
	}

	return;
}

/***************************************************
  axMakePixmap
****************************************************/
void axMakePixmap(Widget w)
{
	Pixel fg,bg;
	int depth;

	/*
	      * create icon pixmap
	      */
	if (!ALH_pixmap){

		/*
		           * get colors and depth
		           */
		XtVaGetValues(w,
		    XmNforeground,     &fg,
		    XmNbackground,     &bg,
		    XmNdepth,          &depth,
		    NULL);

		ALH_pixmap = XCreatePixmapFromBitmapData(XtDisplay(w),
		    RootWindowOfScreen(XtScreen(w)),
		    (char *)AH_bits,AH_width,AH_height,fg,bg,depth);

	}
	return;
}

/***************************************************
 changeBeepSeverityText
****************************************************/
void changeBeepSeverityText(ALINK *area)
{
	XmString    str;

	if (area->beepSeverity) {
		str = XmStringCreateSimple(alhAlarmSeverityString[psetup.beepSevr]);
		XtVaSetValues(area->beepSeverity,
		    XmNlabelString,            str,
		    NULL);
		XmStringFree(str);
	}
}

/***************************************************
 changeSilenceForeverText
****************************************************/
void changeSilenceForeverText(ALINK *area)
{
	XmString    str;

	if (area->silenceForever) {
		str = XmStringCreateSimple(silenceString[psetup.silenceForever]);
		XtVaSetValues(area->silenceForever,
		    XmNlabelString,            str,
		    NULL);
		XmStringFree(str);
	}
}

/***************************************************
 changeDisabledForcePVText
****************************************************/
void updateDisabledForcePVCount(ALINK *area,int value)
{
	area->disabledForcePVCount += value;
}

/***************************************************
 changeDisabledForcePVText
****************************************************/
void changeDisabledForcePVText(ALINK *area)
{
	XmString    str;
	char buff[24];

	if (area->disabledForcePVCountLabel) {
		if (area->disabledForcePVCount) {
			sprintf(buff,"%-.18s %4.4d",
				disabledForcePVCountString[1],area->disabledForcePVCount);
		} else sprintf(buff,"%-.18s",disabledForcePVCountString[0]);
		str = XmStringCreateSimple(buff);
		XtVaSetValues(area->disabledForcePVCountLabel,
		    XmNlabelString,            str,
		    NULL);
		XmStringFree(str);
	}
}

/******************************************************
  axUpdateDialogs
******************************************************/
void axUpdateDialogs(ALINK *area)
{
	/* update beepSeverity string on main window */
	changeBeepSeverityText(area);

	/* update silenceForever string on main window */
	changeSilenceForeverText(area);

	/* update disabledForcePVCount string on main window */
	changeDisabledForcePVText(area);

	/* update property sheet window if it is displayed */
	propUpdateDialog(area);

	/* update force mask window if it is displayed */
	forceMaskUpdateDialog(area);

	/* update forcePV window if it is displayed */
	forcePVUpdateDialog(area);

	/* update force mask window if it is displayed */
	maskUpdateDialog(area);

	/* update set beep severity window if it is displayed */
	beepSevrUpdateDialog(area);

	/* update noAck for one hour window if it is displayed */
	noAckUpdateDialog(area);

}

/***************************************************
  getSelectionLinkArea
****************************************************/
void *getSelectionLinkArea(ALINK *area)
{
	return area->selectionLink;
}

/***************************************************
  getSelectionLinkTypeArea
****************************************************/
int getSelectionLinkTypeArea(ALINK *area)
{
	return area->selectionType;
}


/******************************************************
  CreateActionButtons
******************************************************/
Widget createActionButtons(Widget parent,ActionAreaItem *actions,
int num_buttons)
{
	Widget mask_sheet, widget;
	int i;

	mask_sheet = XtVaCreateWidget("mask_sheet", xmFormWidgetClass, parent,
	    XmNfractionBase, TIGHTNESS*num_buttons - 1,
	    XmNleftOffset,   10,
	    XmNrightOffset,  10,
	    NULL);

	for (i = 0; i < num_buttons; i++) {
		widget = XtVaCreateManagedWidget(actions[i].label,
		    xmPushButtonWidgetClass, mask_sheet,
		    XmNmarginHeight,         0,
		    XmNleftAttachment,       i? XmATTACH_POSITION : XmATTACH_FORM,
		    XmNleftPosition,         TIGHTNESS*i,
		    XmNtopAttachment,        XmATTACH_FORM,
		    XmNbottomAttachment,     XmATTACH_FORM,
		    XmNrightAttachment,
		    i != num_buttons-1? XmATTACH_POSITION : XmATTACH_FORM,
		    XmNrightPosition,        TIGHTNESS*i + (TIGHTNESS-1),
		    XmNshowAsDefault,        i == 2,
		    XmNdefaultButtonShadowThickness, 1,
		    NULL);
		if (actions[i].callback)
			XtAddCallback(widget, XmNactivateCallback,
			    actions[i].callback, actions[i].data);
		if (i == 2) {
			/* Set the mask_sheet's default button to the third widget
			             * created (or, make the index a parameter to the function
			             * or have it be part of the data structure). Also, set the
			             * pane window constraint for max and min heights so this
			             * particular pane in the PanedWindow is not resizable.
			             */
			Dimension height, h;
			XtVaGetValues(mask_sheet, XmNmarginHeight, &h, NULL);
			XtVaGetValues(widget, XmNheight, &height, NULL);
			height += 2 * h;
			XtVaSetValues(mask_sheet,
			    XmNdefaultButton, widget,
			    XmNpaneMaximum,   height,
			    XmNpaneMinimum,   height,
			    NULL);
		}
	}

	XtManageChild(mask_sheet);

	return mask_sheet;
}
