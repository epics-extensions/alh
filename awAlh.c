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
/* awAlh.c */

/************************DESCRIPTION***********************************
  This file contains routines for alh menu.
**********************************************************************/

static char *sccsId = "@(#) $Id$";

/******************************************************************
   Public routines defined in awAlh.c

Widget alhCreateMenu(parent, user_data)     Create alh pulldown Menu 
void alhFileCallback(widget, item, cbs)     File menu items callback
void alhActionCallback(widget, item, cbs)   Action menu items callback
void alhViewCallback(widget, item, cbs)     View menu items callback
void alhSetupCallback(widget, item, cbs)    Setup menu items callback
void alhHelpCallback(widget, item, cbs)     Help menu items callback
void awRowWidgets(line, area)  Create line widgets
void awUpdateRowWidgets(line)                 Update line widgets

**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#include <pwd.h>
#endif
#include <errno.h>
#include <sys/types.h>

#include <Xm/Xm.h>
#include <Xm/ArrowB.h>
#include <Xm/CascadeBG.h>
#include <Xm/DrawingA.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/SeparatoG.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>

#include "alarm.h"
#include "epicsVersion.h"

#include "ax.h"
#include "alh.h"
#include "axArea.h"
#include "axSubW.h"
#include "line.h"
#include "alLib.h"
#include "sllLib.h"
#include "version.h"
#include "alAudio.h"

/* menu definitions */
#define MENU_FILE_OPEN		10101
#define MENU_FILE_OPEN_OK	10102
#define MENU_FILE_CLOSE		10103
#define MENU_FILE_SAVEAS	10106
#define MENU_FILE_QUIT		10109

#define MENU_VIEW_EXPANDCOLLAPSE1	10200
#define MENU_VIEW_EXPANDBRANCH		10201
#define MENU_VIEW_EXPANDALL			10202
#define MENU_VIEW_COLLAPSEBRANCH	10203
#define MENU_VIEW_PROPERTIES		10204

#define	MENU_ACTION_ACK			10300
#define	MENU_ACTION_GUIDANCE	10301
#define	MENU_ACTION_PROCESS		10302
#define MENU_ACTION_FORCEPV		10303
#define MENU_ACTION_FORCE_MASK	10304
#define MENU_ACTION_MODIFY_MASK	10305
#define MENU_ACTION_BEEPSEVR	10306
#define MENU_ACTION_NOACKTIMER	10307


#define	MENU_VIEW_CONFIG		10400
#define	MENU_VIEW_OPMOD			10401
#define	MENU_VIEW_ALARMLOG		10402
#define	MENU_VIEW_CURRENT		10403
#define MENU_VIEW_CMLOG			10404

#define MENU_SETUP_BEEP_MINOR	10500
#define MENU_SETUP_BEEP_MAJOR	10501
#define MENU_SETUP_BEEP_INVALID	10502
#define MENU_SETUP_FILTER_NONE	10503
#define MENU_SETUP_FILTER_ACTIVE	10504
#define MENU_SETUP_FILTER_UNACK 	10505
#define MENU_SETUP_SILENCE_FOREVER	10506
#define MENU_SETUP_ALARMLOG		10507
#define MENU_SETUP_OPMOD		10508

#define MENU_ACTION_BEEP_MINOR	10500
#define MENU_ACTION_BEEP_MAJOR	10501
#define MENU_ACTION_BEEP_INVALID	10502

#define MENU_HELP_HELP	10900
#define MENU_HELP_ABOUT	10906

/* external variables */
extern char alhVersionString[100];
extern char *bg_char[];
extern Pixel bg_pixel[];
extern Pixel channel_bg_pixel;
extern struct setup psetup;
extern Widget versionPopup;
extern int _message_broadcast_flag; /* messages sending flag. Albert1*/
extern int messBroadcastDeskriptor;
extern char messBroadcastInfoFileName[250];
int messBroadcastLockDelay=60000; /* 1 min */
extern char *rebootString;
extern int max_not_save_time;
extern int amIsender;
extern int DEBUG;
extern int _main_window_flag;
char FS_filename[128]; /* Filename      for FSBox. Albert*/

struct UserInfo {
char *loginid;
char *real_world_name;
char *myhostname;
char *displayName;
};

extern struct UserInfo userID; /* info about current operator */

/* prototypes for static routines */
static void alhFileCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhActionCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhViewCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhViewBrowserCallback( Widget widget, XtPointer item, XtPointer cbs); /* Albert1 */
static void messBroadcast(Widget widget, XtPointer item, XtPointer cbs);          /* Albert1 */
static void alhSetupCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhHelpCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void browserFBSDialogCbOk();     /* Ok-button     for FSBox. Albert*/
static void browserFBSDialogCbCancel(); /* Cancel-button for FSBox. Albert*/
static void writeMessBroadcast(Widget dialog, Widget text_w);
static void helpMessBroadcast(Widget w);
static void canselMessBroadcast(Widget w);
static void messBroadcastFileUnlock();

#ifdef CMLOG
static void awCMLOGstartBrowser(void);
#endif


/******************************************************
  alhCreateMenu
******************************************************/

/* Create ALH MenuBar */
Widget alhCreateMenu(Widget parent,XtPointer user_data)
{
	static MenuItem file_menu[] = {
		         { "Open ...",   PushButtonGadgetClass, 'O', "Ctrl<Key>O", "Ctrl+O",
		             alhFileCallback, (XtPointer)MENU_FILE_OPEN,   (MenuItem *)NULL, 0 },
		         { "Save As ...",PushButtonGadgetClass, 'v', NULL, NULL,
		             alhFileCallback, (XtPointer)MENU_FILE_SAVEAS, (MenuItem *)NULL , 0},
		         { "",           SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,    NULL,   (MenuItem *)NULL },
		         { "Close",      PushButtonGadgetClass, 'C', NULL, NULL,
		             alhFileCallback, (XtPointer)MENU_FILE_CLOSE,  (MenuItem *)NULL, 0 },
		         { "",           SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,    NULL,   (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	static MenuItem action_menu[] = {
		         { "Acknowledge Alarm",      PushButtonGadgetClass, 'A', "Ctrl<Key>A", "Ctrl+A",
		             alhActionCallback, (XtPointer)MENU_ACTION_ACK,      (MenuItem *)NULL, 0 },
		         { "Display Guidance",       PushButtonGadgetClass, 'G', "Ctrl<Key>G", "Ctrl+G",
		             alhActionCallback, (XtPointer)MENU_ACTION_GUIDANCE, (MenuItem *)NULL, 0 },
		         { "Start Related Process",  PushButtonGadgetClass, 'P', "Ctrl<Key>P", "Ctrl+P",
		             alhActionCallback, (XtPointer)MENU_ACTION_PROCESS,  (MenuItem *)NULL, 0 },
		         { "Force Process Variable ...", ToggleButtonGadgetClass, 'V', "Ctrl<Key>V", "Ctrl+V",
		             alhActionCallback, (XtPointer)MENU_ACTION_FORCEPV, (MenuItem *)NULL, 0 },
		         { "Force Mask ...",          ToggleButtonGadgetClass, 'M',"Ctrl<Key>M", "Ctrl+M",
		             alhActionCallback, (XtPointer)MENU_ACTION_FORCE_MASK, (MenuItem *)NULL, 0 },
		         { "Modify Mask Settings ...",  ToggleButtonGadgetClass, 'S', "Ctrl<Key>S", "Ctrl+S",
		             alhActionCallback, (XtPointer)MENU_ACTION_MODIFY_MASK, (MenuItem *)NULL, 0 },
		         { "ALH Beep Severity ...",  ToggleButtonGadgetClass, 'B', "Ctrl<Key>B", "Ctrl+B",
		             alhActionCallback, (XtPointer)MENU_ACTION_BEEPSEVR, (MenuItem *)NULL, 0 },
		         { "NoAck for One Hour ...",  ToggleButtonGadgetClass, 'N', "Ctrl<Key>N", "Ctrl+N",
		             alhActionCallback, (XtPointer)MENU_ACTION_NOACKTIMER, (MenuItem *)NULL, 0 },
		         {NULL},
		     	};
/* ******************************************** Albert1 : ************************************ */
static MenuItem action_menuNew[] = {
		         { "Acknowledge Alarm",      PushButtonGadgetClass, 'A', "Ctrl<Key>A", "Ctrl+A",
		             alhActionCallback, (XtPointer)MENU_ACTION_ACK,      (MenuItem *)NULL, 0 },
		         { "Display Guidance",       PushButtonGadgetClass, 'G', "Ctrl<Key>G", "Ctrl+G",
		             alhActionCallback, (XtPointer)MENU_ACTION_GUIDANCE, (MenuItem *)NULL, 0 },
		         { "Start Related Process",  PushButtonGadgetClass, 'P', "Ctrl<Key>P", "Ctrl+P",
		             alhActionCallback, (XtPointer)MENU_ACTION_PROCESS,  (MenuItem *)NULL, 0 },
		         { "Force Process Variable ...", ToggleButtonGadgetClass, 'V', "Ctrl<Key>V", "Ctrl+V",
		             alhActionCallback, (XtPointer)MENU_ACTION_FORCEPV, (MenuItem *)NULL, 0 },
		         { "Force Mask ...",          ToggleButtonGadgetClass, 'M',"Ctrl<Key>M", "Ctrl+M",
		             alhActionCallback, (XtPointer)MENU_ACTION_FORCE_MASK, (MenuItem *)NULL, 0 },
		         { "Modify Mask Settings ...",  ToggleButtonGadgetClass, 'S', "Ctrl<Key>S", "Ctrl+S",
		             alhActionCallback, (XtPointer)MENU_ACTION_MODIFY_MASK, (MenuItem *)NULL, 0 },
		         { "ALH Beep Severity ...",  ToggleButtonGadgetClass, 'B', "Ctrl<Key>B", "Ctrl+B",
		             alhActionCallback, (XtPointer)MENU_ACTION_BEEPSEVR, (MenuItem *)NULL, 0 },
		         { "NoAck for One Hour ...",  ToggleButtonGadgetClass, 'N', "Ctrl<Key>N", "Ctrl+N",
		             alhActionCallback, (XtPointer)MENU_ACTION_NOACKTIMER, (MenuItem *)NULL, 0 },
			 /* Albert1 For MESSAGE BROADCAST: */
#ifndef WIN32
		         { "Send Message ...",  ToggleButtonGadgetClass, 'B', "Ctrl<Key>B", "Ctrl+B",
		             messBroadcast, (XtPointer) NULL, (MenuItem *)NULL, 0 },
#endif
		         {NULL},
		     	};
/* ******************************************** End Albert1 ********************************** */

	static MenuItem view_menu[] = {
		         { "Expand One Level",       PushButtonGadgetClass, 'L', "None<Key>plus", "+",
		             alhViewCallback, (XtPointer)MENU_VIEW_EXPANDCOLLAPSE1,   (MenuItem *)NULL, 0 },
		         { "Expand Branch",          PushButtonGadgetClass, 'B', "None<Key>asterisk", "*",
		             alhViewCallback, (XtPointer)MENU_VIEW_EXPANDBRANCH,      (MenuItem *)NULL, 0 },
		         { "Expand All",             PushButtonGadgetClass, 'A', "Ctrl<Key>asterisk", "Ctrl+*",
		             alhViewCallback, (XtPointer)MENU_VIEW_EXPANDALL,         (MenuItem *)NULL, 0 },
		         { "Collapse Branch",        PushButtonGadgetClass, 'C', "None<Key>minus", "-",
		             alhViewCallback, (XtPointer)MENU_VIEW_COLLAPSEBRANCH,    (MenuItem *)NULL, 0 },
		         { "",                       SeparatorGadgetClass,  '\0', NULL, NULL,
		             NULL,    NULL,   (MenuItem *)NULL, 0 },
		         { "Current Alarm History Window",  ToggleButtonGadgetClass, 'H', NULL, NULL,
		             alhViewCallback, (XtPointer)MENU_VIEW_CURRENT,         (MenuItem *)NULL, 0 },
		         { "Configuration File Window",     ToggleButtonGadgetClass, 'f', NULL, NULL,
		             alhViewCallback, (XtPointer)MENU_VIEW_CONFIG,           (MenuItem *)NULL, 0 },
#ifdef CMLOG
                         { "Start CMLOG Log Browser", PushButtonGadgetClass, 's', NULL, NULL,
                             alhViewCallback, (XtPointer)MENU_VIEW_CMLOG, (MenuItem *)NULL, 0 },
#endif
		         { "Alarm Log File Window",         ToggleButtonGadgetClass, 'r', NULL, NULL,
		             alhViewCallback, (XtPointer)MENU_VIEW_ALARMLOG,         (MenuItem *)NULL, 0 },
		         { "Browser For Alarm Log",         ToggleButtonGadgetClass, 's', NULL, NULL,
		            alhViewBrowserCallback, (XtPointer)MENU_VIEW_ALARMLOG,         (MenuItem *)NULL, 0 },
		         { "Operation Log File Window",     ToggleButtonGadgetClass, 'O', NULL, NULL,
		             alhViewCallback, (XtPointer)MENU_VIEW_OPMOD,         (MenuItem *)NULL, 0 },
		         { "Browser For Operation Log",         ToggleButtonGadgetClass, 'e', NULL, NULL,
		             alhViewBrowserCallback, (XtPointer)MENU_VIEW_OPMOD,         (MenuItem *)NULL, 0 },
		         { "Group/Channel Properties Window", ToggleButtonGadgetClass, 'W', NULL, NULL,
		             alhViewCallback, (XtPointer)MENU_VIEW_PROPERTIES,    (MenuItem *)NULL, 0 },

		         {NULL},
		     	};

	static MenuItem setup_beep_menu[] = {
		         { "Minor",      PushButtonGadgetClass, 'M', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_MINOR,  (MenuItem *)NULL, 0 },
		         { "Major",      PushButtonGadgetClass, 'A', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_MAJOR,  (MenuItem *)NULL, 0 },
		         { "Invalid",      PushButtonGadgetClass, 'V', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_INVALID,  (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	static MenuItem setup_filter_menu[] = {
		         { "No filter",      PushButtonGadgetClass, 'N', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_FILTER_NONE,  (MenuItem *)NULL, 0 },
		         { "Active Alarms Only",      PushButtonGadgetClass, 'A', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_FILTER_ACTIVE,  (MenuItem *)NULL, 0 },
		         { "Unacknowledged Alarms Only",      PushButtonGadgetClass, 'U', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_FILTER_UNACK,  (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	static MenuItem setup_menu[] = {
		         { "Display Filter...",        PushButtonGadgetClass, 'F', NULL, NULL,
		                                      0, 0,    (MenuItem *)setup_filter_menu, 0 },
		         { "ALH Beep Severity...",       PushButtonGadgetClass, 'B', NULL, NULL,
		                                      0, 0,    (MenuItem *)setup_beep_menu, 0 },
#ifdef AUDIO_BEEP
		         { "Audio Setup...",       ToggleButtonGadgetClass, 'D', NULL, NULL,
		             alhAudioSetupCallback, NULL,  (MenuItem *)NULL, 0 },
#endif
		         { "Silence Forever",  ToggleButtonGadgetClass, 'S', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_SILENCE_FOREVER,(MenuItem *)NULL, 0 },
		         { "New Alarm Log File Name...",  PushButtonGadgetClass, 'L', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_ALARMLOG,     (MenuItem *)NULL, 0 },
		         { "New Oper. Log File Name...",  PushButtonGadgetClass, 'O', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_OPMOD,     (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	static MenuItem help_menu[] = {
		         { "Help",       PushButtonGadgetClass, 'H', "Ctrl<Key>H", "Ctrl+H",
		             alhHelpCallback, (XtPointer)MENU_HELP_HELP, (MenuItem *)NULL, 0 },
		         { "About ALH",     PushButtonGadgetClass, 'A', NULL, NULL,
		             alhHelpCallback, (XtPointer)MENU_HELP_ABOUT, (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	Widget menubar,widget;

	/* Set "Silence Forever" toggleButton initial state  */
	setup_menu[2].initial_state = psetup.silenceForever;

	menubar = XmCreateMenuBar(parent, "menubar",   NULL, 0);

	widget = buildPulldownMenu(menubar, "File",     'F', TRUE, file_menu, user_data);
        if(!_message_broadcast_flag)  /* Albert1 */
	widget = buildPulldownMenu(menubar, "Action",   'A', TRUE, action_menu, user_data);
        else
        widget = buildPulldownMenu(menubar, "Action",   'A', TRUE, action_menuNew, user_data);
        
	widget = buildPulldownMenu(menubar, "View",     'V', TRUE, view_menu, user_data);

	widget = buildPulldownMenu(menubar, "Setup",    'S', TRUE, setup_menu, user_data);

	widget = buildPulldownMenu(menubar, "Help",     'H', TRUE, help_menu, user_data);



	/* Make sure Help on MenuBar item is right adjusted */
	XtVaSetValues(menubar, XmNtopAttachment, XmATTACH_FORM, NULL);
	XtVaSetValues(menubar, XmNrightAttachment, XmATTACH_FORM, NULL);
	XtVaSetValues(menubar, XmNleftAttachment, XmATTACH_FORM, NULL);
	XtVaSetValues(menubar, XmNmenuHelpWidget, widget, NULL);

	XtManageChild(menubar);

	return(menubar);
}


/******************************************************
  alhFileCallback
******************************************************/
static void alhFileCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	ALINK      *area;
	int item=(int)calldata;

	XtVaGetValues(widget, XmNuserData, &area, NULL);

	switch (item){

	case MENU_FILE_OPEN:

		/* New Name for Config File  */

		/* Display the config_changed warning dialog */
		if (area->changed){
			createActionDialog(area->form_main,XmDIALOG_WARNING,
			    "Config file settings have Changed.  Do you wish to continue?",
			    (XtCallbackProc)alhFileCallback,
			    (XtPointer)MENU_FILE_OPEN_OK, (XtPointer)area);
			break;
		}
		area->managed = FALSE;
		createFileDialog(area->form_main,
		    (void *)fileSetupCallback, (XtPointer)FILE_CONFIG,
		    (void *)fileCancelCallback,(XtPointer)area,
		    (XtPointer)area,
		    "Alarm Configuration File",CONFIG_PATTERN,psetup.configDir);
		break;

	case MENU_FILE_OPEN_OK:

		area->managed = FALSE;
		createFileDialog(area->form_main,
		    (void *)fileSetupCallback, (XtPointer)FILE_CONFIG,
		    (void *)fileCancelCallback,(XtPointer)area,
		    (XtPointer)area,
		    "Alarm Configuration File",CONFIG_PATTERN,psetup.configDir);
		break;

	case MENU_FILE_SAVEAS:
		/* New Name for Save Config File  */

		createFileDialog(area->form_main,
		    (void *)fileSetupCallback, (XtPointer)FILE_SAVEAS,
		    (void *)XtUnmanageChild,(XtPointer)0,
		    (XtPointer)area,
		    "Save Alarm Configuration File",CONFIG_PATTERN,psetup.configDir);
		break;

	case MENU_FILE_CLOSE:

		/* "Close" was selected. */
		if (_main_window_flag)
			exit_quit(area->toplevel,(XtPointer)area,(XtPointer)area);
		else
			unmapArea_callback(area->toplevel,area->form_main,(XmAnyCallbackStruct *)cbs);
		break;

	case MENU_FILE_QUIT:

		createActionDialog(area->toplevel,XmDIALOG_WARNING,
		    "Exit Alarm Handler?",(XtCallbackProc)exit_quit,
		    (XtPointer)area, (XtPointer)area);
		break;
	}
}


/******************************************************
  alhActionCallback
******************************************************/

static void alhActionCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	int item=(int)calldata;
	ALINK               *area;
	Widget               parent;
	GCLINK              *link;
	struct anyLine      *line;
	WLINE               *wline;

	XtVaGetValues(widget, XmNuserData, &area, NULL);

	switch (item){

	case MENU_ACTION_ACK:

		/* Acknowledge Alarm */
		link = (GCLINK *)area->selectionLink;

		if (link){
			line = (struct anyLine *)link->lineTreeW;
			if (line && line->pwindow == (void *)area->selectionWindow ){

				ack_callback(widget,line,(XmAnyCallbackStruct *)cbs);
			}
			else {
				line = (struct anyLine *)link->lineGroupW;
				if (line && line->pwindow == (void *)area->selectionWindow ){
					ack_callback(widget,line,(XmAnyCallbackStruct *)cbs);
				}
			}
		} else {
			createDialog(area->form_main,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_GUIDANCE:

		/* Display Guidance */
		link = (GCLINK *)area->selectionLink;
		line = (struct anyLine *)link->lineTreeW;
		if (! line || line->pwindow != (void *)area->selectionWindow )
			line = (struct anyLine *)link->lineGroupW;
		if (line){
			wline=(WLINE *)line->wline;
			guidanceCallback(wline->guidance,(GCLINK *)link,(XmAnyCallbackStruct *) cbs);
		}
		else {
			createDialog(area->form_main,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_PROCESS:

		/* Start Related Process */
		link = (GCLINK *)area->selectionLink;
		if (link){
			if (alProcessExists(link)){
				relatedProcess_callback(widget,link, cbs);
			} else {
				if (((GCLINK *)link)->pgcData->alias){
					createDialog(area->form_main,XmDIALOG_WARNING,"No related process for ",
					    link->pgcData->alias);
				} else {
					createDialog(area->form_main,XmDIALOG_WARNING,"No related process for ",
					    link->pgcData->name);
				}
			}
		} else {
			createDialog(area->form_main,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;


	case MENU_ACTION_FORCEPV:

		if (area->selectionLink) {
			forcePVShowDialog(area, widget);
		} else {
			parent = area->form_main;
			createDialog(parent,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_FORCE_MASK:

		if (area->selectionLink) {
			forceMaskShowDialog(area, widget);
		} else {
			parent = area->form_main;
			createDialog(parent,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_MODIFY_MASK:

		if (area->selectionLink) {
			maskShowDialog(area, widget);
		} else {
			parent = area->form_main;
			createDialog(parent,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_BEEPSEVR:

		if (area->selectionLink) {
			beepSevrShowDialog(area, widget);
		} else {
			parent = area->form_main;
			createDialog(parent,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_NOACKTIMER:

		if (area->selectionLink) {
			noAckShowDialog(area, widget);
		} else {
			parent = area->form_main;
			createDialog(parent,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	}
}

 
/******************************************************
  alhViewCallback
******************************************************/
static void alhViewCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	int item=(int)calldata;
	ALINK   *area;
	void   *link;
	struct subWindow *treeWindow;

	XtVaGetValues(widget, XmNuserData, &area, NULL);
	treeWindow = (struct subWindow *)area->treeWindow;

	switch (item){

	case MENU_VIEW_EXPANDCOLLAPSE1:

		/* Expand 1 level */
		link = treeWindow->selectionLink;
		if (link) displayNewViewTree(area,link,EXPANDCOLLAPSE1);
		else createDialog(area->form_main,XmDIALOG_WARNING,"Please select an alarm group first."," ");
		break;

	case MENU_VIEW_EXPANDBRANCH:

		/* Expand Branch */
		link = treeWindow->selectionLink;
		if (link )displayNewViewTree(area,link,EXPAND);
		else createDialog(area->form_main,XmDIALOG_WARNING,"Please select an alarm group first."," ");
		break;

	case MENU_VIEW_EXPANDALL:

		/* Expand all */
		displayNewViewTree(area,(GLINK *)sllFirst(area->pmainGroup),EXPAND);
		break;

	case MENU_VIEW_COLLAPSEBRANCH:

		/* Collapse branch */
		link = treeWindow->selectionLink;
		if (link )displayNewViewTree(area,link,COLLAPSE);
		else createDialog(area->form_main,XmDIALOG_WARNING,"Please select an alarm group first."," ");
		break;

	case MENU_VIEW_CURRENT:

		/* Display Current Alarm History */
		currentAlarmHistoryWindow(area,widget);
		break;

	case MENU_VIEW_CONFIG:

		/* Display Alarm Configuration File */
		fileViewWindow(area->form_main,CONFIG_FILE,widget);
		break;

	case MENU_VIEW_ALARMLOG:

		/* Display Alarm Log File */
		fileViewWindow(area->form_main,ALARM_FILE,widget);
		break;

	case MENU_VIEW_OPMOD:

		/* Display Operation Log File */
		fileViewWindow(area->form_main,OPMOD_FILE,widget);
		break;
#ifdef CMLOG
	case MENU_VIEW_CMLOG:

		/* Start CMLOG Browser */
		awCMLOGstartBrowser();
		break;
#endif
	case MENU_VIEW_PROPERTIES:

		/* Display Group/Chan Properties */
		propShowDialog(area, widget);
		break;

	}
}
 
/******************************************************
  FileSelect for AlhView. Albert
******************************************************/
static void alhViewBrowserCallback(Widget widget,XtPointer item,XtPointer cbs)
{
	ALINK   *area;
	Widget dialog;
	XmString Xpattern,Xtitle,Xcurrentdir;
        int ch = (int) item;
	switch ( ch )
	  {
	  case MENU_VIEW_ALARMLOG:
	    Xtitle=XmStringCreateSimple("Alarm Log File");          
	    Xpattern = XmStringCreateSimple(psetup.logFile);
	    Xcurrentdir = XmStringCreateSimple(psetup.logDir); 
	    break;
	    
	  case MENU_VIEW_OPMOD:
	    Xtitle=XmStringCreateSimple("Operator File");          
	    Xpattern = XmStringCreateSimple(psetup.opModFile);
	    Xcurrentdir = XmStringCreateSimple(psetup.logDir);  /* Albert1 ???? */  
	    break;

	  default:
            perror("bad item");
	    return;
	  }     

		XtVaGetValues(widget, XmNuserData, &area, NULL);
		dialog=XmCreateFileSelectionDialog(area->form_main,"dialog",NULL,0);
	switch ( ch )
	  {
	  case MENU_VIEW_ALARMLOG:
	    	XtVaSetValues(dialog,XmNuserData,ALARM_FILE,NULL);
	   	 break;
	  case MENU_VIEW_OPMOD:
	    	XtVaSetValues(dialog,XmNuserData,OPMOD_FILE,NULL);
	   	 break;
	    
	  }     
		XtAddCallback(dialog,XmNokCallback,(XtCallbackProc)browserFBSDialogCbOk,widget);
		XtAddCallback(dialog,XmNcancelCallback,(XtCallbackProc)browserFBSDialogCbCancel,NULL);
		XtUnmanageChild(XmFileSelectionBoxGetChild(dialog,
		    XmDIALOG_HELP_BUTTON));

		XtVaSetValues(dialog,
		    XmNdirectory,     Xcurrentdir,
		    XmNdialogTitle,   Xtitle,
		    XmNdirMask,    Xpattern,
                    XmNfileTypeMask,		XmFILE_REGULAR,
		    NULL);
		XtManageChild(dialog);
		XmStringFree(Xpattern);
		XmStringFree(Xtitle);
}

/******************************************************
  browserFBSDialogCbCancel
******************************************************/
static void browserFBSDialogCbCancel(Widget w,int client_data,
XmSelectionBoxCallbackStruct *call_data)
{
	XtUnmanageChild(w);
}

/******************************************************
  browserFBSDialogCbOk
******************************************************/
static void browserFBSDialogCbOk(Widget w,Widget wdgt,
XmSelectionBoxCallbackStruct *call_data)
{
	ALINK   *area;
	char *s;
	int fileType;
	XmStringGetLtoR(call_data->value,XmSTRING_DEFAULT_CHARSET,&s);
	strcpy(FS_filename,s);
	XtFree(s);
	XtVaGetValues(w, XmNuserData, &fileType, NULL);
	XtVaGetValues(wdgt, XmNuserData, &area, NULL);
	browser_fileViewWindow(area->form_main,fileType,wdgt); /* Albert1 */
	XtUnmanageChild(w);
}

#ifndef WIN32
/******************************************************
Send Message widget
******************************************************/
static void messBroadcast(Widget widget,XtPointer item,XtPointer cbs)  /* Albert1 */
{
    Widget dialog, text_w;
    ALINK   *area;
    XtVaGetValues(widget, XmNuserData, &area, NULL);

    if(amIsender)
      {
	createDialog(area->form_main,XmDIALOG_INFORMATION,
		     "You send some message before. \n" "\n"
		     "Please wait a few seconds \n","");
	return;
      }
         if ( lockf(messBroadcastDeskriptor, F_TLOCK, 0L) < 0 ) {
	  if ((errno == EAGAIN || errno == EACCES )) {
	      if(DEBUG) fprintf(stderr,"file is busy;Deskriptor=%d\n",messBroadcastDeskriptor);
	      createDialog(area->form_main,XmDIALOG_INFORMATION,
			   "Some other operator type a message. \n" "\n"
			   "Please wait a few seconds \n","");
	      return;
	  }
	  else {
	    perror("lockf Error!!!!"); /* Albert1 exit ?????? */
	  }
	 }
	else 
	  {
	    if(DEBUG) fprintf(stderr,"file is free;Deskriptor=%d\n",messBroadcastDeskriptor);	    

	    dialog = XmCreatePromptDialog(area->form_main, "dialog", NULL, 0);	    
	    XtVaSetValues(dialog,XtVaTypedArg, XmNselectionLabelString, XmRString,
			  "Type message (See help for detail):", 40,NULL);
	    
	    XtAddCallback(dialog, XmNcancelCallback,(XtCallbackProc)canselMessBroadcast, NULL);
	    XtAddCallback(dialog, XmNhelpCallback,(XtCallbackProc)helpMessBroadcast, NULL);
	    text_w = XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT);
	    XtVaSetValues(text_w, XmNvalue,"0 type your message", NULL);
	    XtVaSetValues(dialog,XmNuserData,area,NULL);
	    XtAddCallback(dialog, XmNokCallback,(XtCallbackProc)writeMessBroadcast, text_w);
	    XtManageChild(dialog);

	  }
}

static void writeMessBroadcast(Widget dialog, Widget text_w)
{
FILE *fp;
time_t timeID,time_tmp;
void messBroadcastFileUnlock();
char *string;
char *blank;

int notsave_time;
char buff[500];
ALINK   *ar;

    XtVaGetValues(dialog, XmNuserData, &ar, NULL);

    if ( (fp=fopen(messBroadcastInfoFileName,"w")) == NULL )
      {
	createDialog(ar->form_main,XmDIALOG_INFORMATION,"can't open ",messBroadcastInfoFileName);
	lockf(messBroadcastDeskriptor, F_ULOCK, 0L);
	return;
          }

    /* NOTE: all clients MUST have the same UNIX time !!!!  ?????  */
    timeID=time(0L); 
    time_tmp=time(0L); 
    
    string = XmTextFieldGetString(text_w);

    if( (blank=strchr(string,' ')) == NULL )
      {
	createDialog(ar->form_main,XmDIALOG_INFORMATION,"Wrong format"," ");
	lockf(messBroadcastDeskriptor, F_ULOCK, 0L);
	return;
      }
    *blank=0;
    blank++;
    notsave_time=atoi(string);
    if( (notsave_time < 0) || (notsave_time > max_not_save_time) ) 
      {
	createDialog(ar->form_main,XmDIALOG_INFORMATION,"time so big!!!"," ");
	lockf(messBroadcastDeskriptor, F_ULOCK, 0L);
	return;     
      }

    if(notsave_time != 0)
      {
	sprintf(buff,"%d %s %s\nDate is %sFROM: User=%s [%s] host=%s display=%s",notsave_time,
		rebootString, blank,ctime(&time_tmp),userID.loginid,
		userID.real_world_name,userID.myhostname,userID.displayName);
      }
    else
      {
	sprintf(buff,"%s\nDate is %sFROM: User=%s [%s] host=%s display=%s",
		blank,ctime(&time_tmp),userID.loginid,
		userID.real_world_name,userID.myhostname,userID.displayName);
      }

    fprintf(fp,"%ld\n%s",timeID,buff);
    createDialog(ar->form_main,XmDIALOG_MESSAGE,"Broadcast Message: \n""\n""\n",buff);

    fclose(fp);	    
    XtFree(string);
    amIsender=1;
    XtAppAddTimeOut(appContext,messBroadcastLockDelay,messBroadcastFileUnlock,NULL);
    
}

static void messBroadcastFileUnlock()
{
  FILE *fp;
  if(DEBUG) fprintf(stderr,"delete messBroadcastInfoFileName...\n");
  if ( (fp=fopen(messBroadcastInfoFileName,"w")) == NULL )
    {
      perror("Loop:can't open messBroadcastInfoFileName!!!!");
    }
  if (fp) fclose (fp);
  amIsender=0;
  lockf(messBroadcastDeskriptor, F_ULOCK, 0L);
}

static void helpMessBroadcast(Widget w)
{
createDialog(w,XmDIALOG_INFORMATION,
"This message will be pass all other operators which work with the same alh configuration. \n"
"  \n"
"ALH has one special type of messages : \n"
"  \n"
"  ''time_in_min ''any other text ''  ''  \n"
"  \n"
"After this message ALH doesn't save alarms during time_in_min \n"
"  \n"
"Please, use this type of message before rebooting IOC for communications with other people.\n"
"  \n"
"  \n"
"  \n"
"If time_in_min = 0 it's mean any other information messages without cansel saving alarmLog. \n"
"  \n"
"You can use it for any other communications (like Hello all)",
"");
}

static void canselMessBroadcast(Widget w)
{
XtUnmanageChild(w);
 lockf(messBroadcastDeskriptor, F_ULOCK, 0L);
}

#endif
 
/******************************************************
  alhSetupCallback
******************************************************/
static void alhSetupCallback( Widget widget, XtPointer calldata, XtPointer cbs)
{
	int item=(int)calldata;
	ALINK      *area;


	XtVaGetValues(widget, XmNuserData, &area, NULL);

	switch (item){

	case MENU_SETUP_FILTER_NONE:
		area->viewFilter = alFilterAll;
		createConfigDisplay(area,EXPANDCOLLAPSE1);
		break;

	case MENU_SETUP_FILTER_ACTIVE:
		area->viewFilter = alFilterAlarmsOnly;
		createConfigDisplay(area,EXPANDCOLLAPSE1);
		break;

	case MENU_SETUP_FILTER_UNACK:
		area->viewFilter = alFilterUnackAlarmsOnly;
		createConfigDisplay(area,EXPANDCOLLAPSE1);
		break;

	case MENU_SETUP_BEEP_MINOR:

		psetup.beepSevr = MINOR_ALARM;
		changeBeepSeverityText(area);
		break;

	case MENU_SETUP_BEEP_MAJOR:

		psetup.beepSevr = MAJOR_ALARM;
		changeBeepSeverityText(area);
		break;

	case MENU_SETUP_BEEP_INVALID:

		psetup.beepSevr = INVALID_ALARM;
		changeBeepSeverityText(area);
		break;

	case MENU_SETUP_SILENCE_FOREVER:

		silenceForeverChangeState(area);
		break;

	case MENU_SETUP_ALARMLOG:

		/* New Name for Alarm Log File  */
		createFileDialog(area->form_main,
		    (void *)fileSetupCallback,(XtPointer)FILE_ALARMLOG,
		    (void *)XtUnmanageChild,(XtPointer)0,
		    (XtPointer)area, 
		    "Alarm Log File",ALARMLOG_PATTERN,psetup.logDir);
		break;

	case MENU_SETUP_OPMOD:

		/* New Name for Operations Log File  */
		createFileDialog(area->form_main,
		    (void *)fileSetupCallback,(XtPointer)FILE_OPMOD,
		    (void *)XtUnmanageChild,(XtPointer)0,
		    (XtPointer)area,
		    "Operator Modification File", OPMOD_PATTERN,psetup.logDir);
		break;

	}
}

 
/******************************************************
  alhHelpCallback
******************************************************/
static void alhHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	int item=(int)calldata;
	ALINK  *area;

	XtVaGetValues(widget, XmNuserData, &area, NULL);

	switch (item){

#ifdef ALH_HELP_URL
	case MENU_HELP_HELP:

		callBrowser(ALH_HELP_URL);
		break;
#endif

	case MENU_HELP_ABOUT:

		createDialog(area->form_main,XmDIALOG_INFORMATION,
		    "\nAlarm Handler\n\n" ALH_CREDITS_STRING ,alhVersionString);

		break;

	default:

		createDialog(area->form_main,XmDIALOG_INFORMATION,
		    "Help is not available in this release."," ");
		break;
	}
}

 
/******************************************************
  awRowWidgets
******************************************************/
void awRowWidgets(struct anyLine *line,void *area)
{

	void *subWindow;
	XmString str;
	WLINE  *wline;
	void *link;
	GLINK *glink;
	Position nextX;
	Dimension width;
	Widget parent;

	subWindow=line->pwindow;
	parent = ((struct subWindow *)subWindow)->drawing_area;
	wline=(WLINE *)line->wline;
	link = line->link;
	glink = (GLINK *)line->link;

	/* create row widgets */
	if (wline->row_widget == NULL) {


		wline->row_widget = XtVaCreateWidget(NULL,
		    xmDrawingAreaWidgetClass,  parent,
		    XmNy,                        calcRowYValue(subWindow,line->lineNo),
		    XmNnavigationType,           XmNONE,
		    XmNorientation,              XmHORIZONTAL,
		    XmNmarginHeight,             0,
		    XmNmarginWidth,              0,
		    NULL);
		nextX = 0;

		if ( isTreeWindow(area,subWindow) && line->linkType == GROUP) {
			if ( glink->pgroupData->treeSym) {
				str = XmStringCreateSimple(glink->pgroupData->treeSym);
				wline->treeSym = XtVaCreateManagedWidget("treeSym",
				    xmLabelGadgetClass,        wline->row_widget,
				    XmNlabelString,            str,
				    XmNmarginHeight,           0,
				    NULL);
				XmStringFree(str);
				XtVaGetValues(wline->treeSym,XmNwidth,&width,NULL);
				nextX = width + 3;
			}
		}

		str = XmStringCreateSimple(bg_char[line->unackSevr]);
		wline->ack = XtVaCreateManagedWidget("ack",
		    xmPushButtonWidgetClass,   wline->row_widget,
		    XmNmarginHeight,           0,
		    XmNlabelString,            str,
		    XmNsensitive,              FALSE,
		    XmNuserData,               (XtPointer)area,
		    XmNx,                      nextX,
		    NULL);
		XmStringFree(str);
		XtAddCallback(wline->ack, XmNactivateCallback, 
		    (XtCallbackProc)ack_callback, line);
		XtVaGetValues(wline->ack,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;


		str = XmStringCreateSimple(bg_char[line->curSevr]);
		wline->sevr = XtVaCreateManagedWidget("sevr",
		    xmLabelWidgetClass,        wline->row_widget,
		    XmNlabelString,            str,
		    XmNx,                      nextX,
		    XmNy,                      2,
		    NULL);
		XmStringFree(str);
		XtVaGetValues(wline->sevr,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;


		str = XmStringCreateSimple(line->alias);
		wline->name = XtVaCreateManagedWidget("pushButtonName",
		    xmPushButtonWidgetClass,   wline->row_widget,
		    XmNmarginHeight,           0,
		    XmNlabelString,            str,
		    XmNuserData,               (XtPointer)subWindow,
		    XmNx,                      nextX,
		    NULL);
		XmStringFree(str);
		if (line->linkType == CHANNEL) {
#if  XmVersion && XmVersion >= 1002
			XmChangeColor(wline->name,channel_bg_pixel);
#else
			XtVaSetValues(wline->name,XmNbackground,channel_bg_pixel,NULL);
#endif
		}
		if ( isTreeWindow(area,subWindow) ) {
			XtAddCallback(wline->name, XmNactivateCallback,
			    (XtCallbackProc)nameTreeW_callback, line);
		} else {
			XtAddCallback(wline->name, XmNactivateCallback,
			    (XtCallbackProc)nameGroupW_callback, line);
		}
		XtVaGetValues(wline->name,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		wline->arrow = XtVaCreateWidget("pushButtonArrow",
		    xmArrowButtonWidgetClass,   wline->row_widget,
		    XmNshadowThickness,        0,
		    XmNarrowDirection,         XmARROW_RIGHT,
		    XmNuserData,               (XtPointer)area,
		    XmNx,                      nextX,
		    XmNy,                      2,
		    NULL);
		if (line->linkType == GROUP && sllFirst(&(glink->subGroupList))){
			XtManageChild(wline->arrow);
			if ( isTreeWindow(area,subWindow) ) {
				XtAddCallback(wline->arrow, XmNactivateCallback,
				    (XtCallbackProc)arrowTreeW_callback, link);
			} else {
				XtAddCallback(wline->arrow, XmNactivateCallback,
				    (XtCallbackProc)arrowGroupW_callback, link);
			}
			XtVaGetValues(wline->arrow,XmNwidth,&width,NULL);
			nextX = nextX + width + 3;
		}

		wline->guidance = XtVaCreateWidget("G",
		    xmPushButtonWidgetClass,   wline->row_widget,
		    XmNmarginHeight,           0,
		    XmNuserData,               (XtPointer)line->alias,
		    XmNx,                      nextX,
		    NULL);

		if (guidanceExists(link)) {
			XtManageChild(wline->guidance);
			XtAddCallback(wline->guidance, XmNactivateCallback, 
			    (XtCallbackProc)guidanceCallback, link);
			XtVaGetValues(wline->guidance,XmNwidth,&width,NULL);
			nextX = nextX + width + 3;
		}

		wline->process = XtVaCreateWidget("P",
		    xmPushButtonWidgetClass,   wline->row_widget,
		    XmNmarginHeight,           0,
		    XmNuserData,               (XtPointer)area,
		    XmNx,                      nextX,
		    NULL);

		if (alProcessExists(link) ){
			XtManageChild(wline->process);
			XtAddCallback(wline->process, XmNactivateCallback, 
			    (XtCallbackProc)relatedProcess_callback, link);
			XtVaGetValues(wline->process,XmNwidth,&width,NULL);
			nextX = nextX + width + 3;
		}

		str = XmStringCreateSimple(line->mask);
		wline->mask = XtVaCreateManagedWidget("mask",
		    xmLabelWidgetClass,        wline->row_widget,
		    XmNlabelString,            str,
		    XmNx,                      nextX,
		    XmNy,                      2,
		    NULL);
		XmStringFree(str);
		XtVaGetValues(wline->mask,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		str = XmStringCreateSimple(line->message);
		wline->message = XtVaCreateManagedWidget("message",
		    xmLabelWidgetClass,        wline->row_widget,
		    XmNlabelString,            str,
		    XmNx,                      nextX,
		    XmNy,                      2,
		    NULL);
		XmStringFree(str);

		awUpdateRowWidgets(line);

		XtManageChild(wline->row_widget);
	}

	else

	/* else modify existing  row widgets */
	{
		if ( wline->row_widget && XtIsManaged(wline->row_widget)){
			XtUnmanageChild(wline->row_widget);
		}

		nextX = 0;
		if ( isTreeWindow(area,subWindow) && line->linkType == GROUP) {
			str = XmStringCreateSimple(glink->pgroupData->treeSym);
			XtVaSetValues(wline->treeSym,
			    XmNlabelString,            str,
			    NULL);
			XmStringFree(str);
			XtVaGetValues(wline->treeSym,XmNwidth,&width,NULL);
			nextX = width + 3;
		}

		if (line->unackSevr == FALSE) {
			XtVaSetValues(wline->ack,
			    XmNsensitive,            FALSE,
			    NULL);
		}
		else {
			XtVaSetValues(wline->ack,
			    XmNsensitive,            TRUE,
			    NULL);
		}

		XtVaSetValues(wline->ack,XmNx,nextX,NULL);
		XtRemoveAllCallbacks(wline->ack, XmNactivateCallback);
		XtAddCallback(wline->ack, XmNactivateCallback, 
		    (XtCallbackProc)ack_callback, line);
		XtVaGetValues(wline->ack,XmNwidth,&width,NULL);
		nextX = nextX + width +3;


		XtVaSetValues(wline->sevr,XmNx,nextX,NULL);
		XtVaGetValues(wline->sevr,XmNwidth,&width,NULL);
		nextX = nextX + width +3;

		str = XmStringCreateSimple(line->alias);
		XtVaSetValues(wline->name,
		    XmNlabelString,            str,
		    XmNx,                      nextX,
		    NULL);
		XmStringFree(str);
		if ( ! isTreeWindow(area,subWindow)) {
			if (line->linkType == CHANNEL) {
#if  XmVersion && XmVersion >= 1002
				XmChangeColor(wline->name,channel_bg_pixel);
#else
				XtVaSetValues(wline->name,XmNbackground,channel_bg_pixel,NULL);
#endif
			}
			if (line->linkType == GROUP) {
#if  XmVersion && XmVersion >= 1002
				XmChangeColor(wline->name,bg_pixel[0]);
#else
				XtVaSetValues(wline->name,XmNbackground,bg_pixel[0],NULL);
#endif
			}
		}
		XtRemoveAllCallbacks(wline->name, XmNactivateCallback);
		if ( isTreeWindow(area,subWindow) ) {
			XtAddCallback(wline->name, XmNactivateCallback,
			    (XtCallbackProc)nameTreeW_callback, line);
		} else {
			XtAddCallback(wline->name, XmNactivateCallback,
			    (XtCallbackProc)nameGroupW_callback, line);
		}
		XtVaGetValues(wline->name,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		if (XtHasCallbacks(wline->arrow,XmNactivateCallback))
			XtRemoveAllCallbacks(wline->arrow, XmNactivateCallback);
		if (line->linkType == GROUP && sllFirst(&(glink->subGroupList))){
			XtVaSetValues(wline->arrow,XmNx,nextX,NULL);
			XtManageChild(wline->arrow);
			if ( isTreeWindow(area,subWindow) ) {
				XtAddCallback(wline->arrow, XmNactivateCallback,
				    (XtCallbackProc)arrowTreeW_callback, link);
			} else {
				XtAddCallback(wline->arrow, XmNactivateCallback,
				    (XtCallbackProc)arrowGroupW_callback, link);
			}
			XtVaGetValues(wline->arrow,XmNwidth,&width,NULL);
			nextX = nextX + width +3;

		} else {
			XtUnmanageChild(wline->arrow);
		}

		if (XtHasCallbacks(wline->guidance,XmNactivateCallback))
			XtRemoveAllCallbacks(wline->guidance, XmNactivateCallback);
		if (guidanceExists(link)) {
			XtVaSetValues(wline->guidance,XmNx,nextX,NULL);
			XtVaSetValues(wline->guidance,XmNuserData,line->alias,NULL);
			XtManageChild(wline->guidance);
			XtAddCallback(wline->guidance, XmNactivateCallback, 
			    (XtCallbackProc)guidanceCallback, link);
			XtVaGetValues(wline->guidance,XmNwidth,&width,NULL);
			nextX = nextX + width + 3;
		} else {
			XtUnmanageChild(wline->guidance);
		}

		if (XtHasCallbacks(wline->process,XmNactivateCallback))
			XtRemoveAllCallbacks(wline->process, XmNactivateCallback);
		if (alProcessExists(link) ){
			XtVaSetValues(wline->process,XmNx,nextX,NULL);
			XtManageChild(wline->process);
			XtAddCallback(wline->process, XmNactivateCallback, 
			    (XtCallbackProc)relatedProcess_callback, link);
			XtVaGetValues(wline->process,XmNwidth,&width,NULL);
			nextX = nextX + width + 3;
		} else {
			XtUnmanageChild(wline->process);
		}

		XtVaSetValues(wline->mask,XmNx,nextX,NULL);
		XtVaGetValues(wline->mask,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		XtVaSetValues(wline->message,XmNx,nextX,NULL);

		awUpdateRowWidgets(line);

		XtManageChild(wline->row_widget);

	}

}


/******************************************************
  awUpdateRowWidgets
******************************************************/
void awUpdateRowWidgets(struct anyLine *line)
{
	XmString str;
	WLINE *wline;
	Pixel bg;
	XmString strOld;
	Boolean sensitiveOld;

	wline=(WLINE *)line->wline;

	XtVaGetValues(wline->ack,
	    XmNbackground,            &bg,
	    XmNlabelString,           &strOld,
	    XmNsensitive,             &sensitiveOld,
	    NULL);

	if (line->unackSevr == FALSE) {
		if (sensitiveOld == TRUE)
			XtVaSetValues(wline->ack,
			    XmNsensitive,            FALSE,
			    NULL);
	}
	else {
		if (sensitiveOld == FALSE)
			XtVaSetValues(wline->ack,
			    XmNsensitive,            TRUE,
			    NULL);
	}
	str = XmStringCreateSimple(bg_char[line->unackSevr]);
	if (!XmStringCompare(str,strOld))
		XtVaSetValues(wline->ack,
		    XmNlabelString,            str,
		    NULL);
	XmStringFree(strOld);
	XmStringFree(str);

	if (bg != bg_pixel[line->unackSevr]){
#if  XmVersion && XmVersion >= 1002
		XmChangeColor(wline->ack,bg_pixel[line->unackSevr]);
#else
		XtVaSetValues(wline->ack,XmNbackground,bg_pixel[line->unackSevr],NULL);
#endif
	}

	XtVaGetValues(wline->sevr,
	    XmNbackground,            &bg,
	    XmNlabelString,           &strOld,
	    NULL);

	str = XmStringCreateSimple(bg_char[line->curSevr]);
	if (!XmStringCompare(str,strOld))
		XtVaSetValues(wline->sevr,
		    XmNlabelString,            str,
		    NULL);
	XmStringFree(str);
	XmStringFree(strOld);

	if (bg != bg_pixel[line->curSevr]){
#if  XmVersion && XmVersion >= 1002
		XmChangeColor(wline->sevr,bg_pixel[line->curSevr]);
#else
		XtVaSetValues(wline->sevr,XmNbackground,bg_pixel[line->curSevr],NULL);
#endif
	}

	XtVaGetValues(wline->mask,
	    XmNlabelString,           &strOld,
	    NULL);

	str = XmStringCreateSimple(line->mask);
	if (!XmStringCompare(str,strOld))
		XtVaSetValues(wline->mask,
		    XmNlabelString,            str,
		    NULL);
	XmStringFree(str);
	XmStringFree(strOld);

	XtVaGetValues(wline->message,
	    XmNlabelString,           &strOld,
	    NULL);

	str = XmStringCreateSimple(line->message);
	if (!XmStringCompare(str,strOld))
		XtVaSetValues(wline->message,
		    XmNlabelString,            str,
		    NULL);
	XmStringFree(str);
	XmStringFree(strOld);
}


/*+**************************************************************************
 *
 * Function:	awCMLOGstartBrowser
 *
 * Description:	Starts the standard Motif CMLOG browser
 *
 **************************************************************************-*/

#ifdef CMLOG
static void awCMLOGstartBrowser (void)
{
   char command[256];
   
   switch (fork()) {
				/* Error */
   case -1:
      perror("Cannot fork() to start CMLOG browser");
      return;
				/* Child */
   case 0:
      if (psetup.configDir)
	 sprintf(command, CMLOG_BROWSER" -f %s/"CMLOG_CONFIG" &", psetup.configDir);
      else
	 sprintf(command, CMLOG_BROWSER" -f "CMLOG_CONFIG" &");

      system(command);
      exit(0);
      break;
				/* Parent */
   default:
      break;
   }
   return;
}
#endif
