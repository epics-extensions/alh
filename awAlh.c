/*
 $Log$
 Revision 1.14  1998/08/05 18:20:06  jba
 Added silenceOneHour button.
 Moved silenceForever button to Setup menu.
 Added logging for operator silence changes.

 Revision 1.13  1998/07/29 17:27:36  jba
 Added "Unacknowledged Alarms Only" display filter.

 Revision 1.12  1998/07/07 20:51:02  jba
 Added alh versioning.

 Revision 1.11  1998/06/22 18:42:12  jba
 Merged the new alh-options created at DESY MKS group:
  -D Disable Writing, -S Passive Mode, -T AlarmLogDated, -P Printing

 Revision 1.10  1998/05/13 19:29:48  evans
 More WIN32 changes.

 Revision 1.9  1997/09/12 19:32:04  jba
 Added treeSym test.

 Revision 1.8  1997/09/09 22:21:56  jba
 Changed help menu selections.

 Revision 1.7  1995/11/13 22:31:17  jba
 Added beepseverity command, ansi changes and other changes.

 * Revision 1.6  1995/10/20  16:50:12  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 *
 * Revision 1.4  1995/06/22  19:48:50  jba
 * Added $ALIAS facility.
 *
 * Revision 1.3  1995/05/31  20:34:06  jba
 * Added name selection and arrow functions to Group window
 *
 * Revision 1.2  1994/06/22  21:16:54  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)awAlh.c	1.13\t12/15/93";

/* awAlh.c   */
/*
 *      Author:		Janet Anderson
 *      Date:		09-28-92
 *
 *	Experimental Physics and Industrial Control System (EPICS)
 *
 *	Copyright 1991, the Regents of the University of California,
 *	and the University of Chicago Board of Governors.
 *
 *	This software was produced under  U.S. Government contralhs:
 *	(W-7405-ENG-36) at the Los Alamos National Laboratory,
 *	and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *	Initial development by:
 *		The Controls and Automation Group (AT-8)
 *		Ground Test Accelerator
 *		Accelerator Technology Division
 *		Los Alamos National Laboratory
 *
 *	Co-developed with
 *		The Controls and Computing Group
 *		Accelerator Systems Division
 *		Advanced Photon Source
 *		Argonne National Laboratory
 *
 *
 * Modification Log:
 * -----------------
 * .01	mm-dd-yy		nnn	Description
 * .o2	12-10-93		jba changes for new command line dir and file options
 */

/*
******************************************************************
	routines defined in awAlh.c
******************************************************************
         Routines for ALH specific menu, line, and callbacks
******************************************************************
-------------
|   PUBLIC  |
-------------
*
Widget alhCreateMenu(parent, user_data)     Create alh pulldown Menu 
     Widget     parent;
     XtPointer  user_data;
*
void alhFileCallback(widget, item, cbs)     File menu items callback
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
*
void alhActionCallback(widget, item, cbs)   Action menu items callback
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
*
void alhViewCallback(widget, item, cbs)     View menu items callback
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
*
void alhSetupCallback(widget, item, cbs)    Setup menu items callback
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
*
void alhHelpCallback(widget, item, cbs)     Help menu items callback
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
*
void awRowWidgets(line, area)  Create line widgets
     struct anyLine  *line;
     void *area
*
void awUpdateRowWidgets(line)                 Update line widgets
     struct anyLine  *line;
*
*************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>

#include "alarm.h"

#include "ax.h"
#include "alh.h"
#include "axArea.h"
#include "axSubW.h"
#include "line.h"
#include "alLib.h"
#include "sllLib.h"
#include "epicsVersion.h"
#include "version.h"

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
#include <Xm/SeparatoG.h>
#include <Xm/ToggleBG.h>

/* external variables */
extern char alhVersionString[60];
extern char *bg_char[];
extern Pixel bg_pixel[];
extern Pixel channel_bg_pixel;
extern struct setup psetup;
extern Widget versionPopup;
extern int _time_flag; /* Dated flag. Albert*/
void dialogCbOk();     /* Ok-button     for FSBox. Albert*/
void dialogCbCansel(); /* Cansel-button for FSBox. Albert*/
char FS_filename[128]; /* Filename      for FSBox. Albert*/
#ifdef __STDC__

/* prototypes for static routines */
static void alhFileCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhActionCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhViewCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhViewCallback1( Widget widget, XtPointer item, XtPointer cbs);
static void alhSetupCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhHelpCallback( Widget widget, XtPointer calldata, XtPointer cbs);

#else 

static void alhFileCallback();
static void alhActionCallback();
static void alhViewCallback();
static void alhViewCallback1();
static void alhSetupCallback();
static void alhHelpCallback();

#endif /*__STDC__*/



/******************************************************
  alhCreateMenu
******************************************************/

/* Create ALH MenuBar */
Widget alhCreateMenu(parent, user_data)
     Widget     parent;
     XtPointer  user_data;
{
     static MenuItem file_menu[] = {
         { "Open ...",   &xmPushButtonGadgetClass, 'O', "Ctrl<Key>O", "Ctrl+O",
             alhFileCallback, (XtPointer)MENU_FILE_OPEN,   (MenuItem *)NULL },
         { "Save As ...",&xmPushButtonGadgetClass, 'v', NULL, NULL,
             alhFileCallback, (XtPointer)MENU_FILE_SAVEAS, (MenuItem *)NULL },
         { "",           &xmSeparatorGadgetClass, '\0', NULL, NULL,
             NULL,    NULL,   (MenuItem *)NULL },
         { "Close",      &xmPushButtonGadgetClass, 'C', NULL, NULL,
             alhFileCallback, (XtPointer)MENU_FILE_CLOSE,  (MenuItem *)NULL },
         { "",           &xmSeparatorGadgetClass, '\0', NULL, NULL,
             NULL,    NULL,   (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem action_menu[] = {
         { "Acknowledge Alarm",      &xmPushButtonGadgetClass, 'A', "Ctrl<Key>A", "Ctrl+A",
             alhActionCallback, (XtPointer)MENU_ACTION_ACK,      (MenuItem *)NULL },
         { "Display Guidance",       &xmPushButtonGadgetClass, 'G', "Ctrl<Key>G", "Ctrl+G",
             alhActionCallback, (XtPointer)MENU_ACTION_GUIDANCE, (MenuItem *)NULL },
         { "Start Related Process",  &xmPushButtonGadgetClass, 'P', "Ctrl<Key>P", "Ctrl+P",
             alhActionCallback, (XtPointer)MENU_ACTION_PROCESS,  (MenuItem *)NULL },
         { "Force Process Variable ...", &xmToggleButtonGadgetClass, 'V', "Ctrl<Key>V", "Ctrl+V",
             alhActionCallback, (XtPointer)MENU_ACTION_FORCEPV, (MenuItem *)NULL },
         { "Force Mask ...",          &xmToggleButtonGadgetClass, 'M',"Ctrl<Key>M", "Ctrl+M",
             alhActionCallback, (XtPointer)MENU_ACTION_FORCE_MASK, (MenuItem *)NULL },
         { "Modify Mask Settings ...",  &xmToggleButtonGadgetClass, 'S', "Ctrl<Key>S", "Ctrl+S",
             alhActionCallback, (XtPointer)MENU_ACTION_MODIFY_MASK, (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem view_menu[] = {
         { "Expand One Level",       &xmPushButtonGadgetClass, 'L', "None<Key>plus", "+",
             alhViewCallback, (XtPointer)MENU_VIEW_EXPANDCOLLAPSE1,   (MenuItem *)NULL },
         { "Expand Branch",          &xmPushButtonGadgetClass, 'B', "None<Key>asterisk", "*",
             alhViewCallback, (XtPointer)MENU_VIEW_EXPANDBRANCH,      (MenuItem *)NULL },
         { "Expand All",             &xmPushButtonGadgetClass, 'A', "Ctrl<Key>asterisk", "Ctrl+*",
             alhViewCallback, (XtPointer)MENU_VIEW_EXPANDALL,         (MenuItem *)NULL },
         { "Collapse Branch",        &xmPushButtonGadgetClass, 'C', "None<Key>minus", "-",
             alhViewCallback, (XtPointer)MENU_VIEW_COLLAPSEBRANCH,    (MenuItem *)NULL },
         { "",                       &xmSeparatorGadgetClass,  '\0', NULL, NULL,
             NULL,    NULL,   (MenuItem *)NULL },
         { "Current Alarm History Window",  &xmToggleButtonGadgetClass, 'H', NULL, NULL,
             alhViewCallback, (XtPointer)MENU_VIEW_CURRENT,         (MenuItem *)NULL },
         { "Configuration File Window",     &xmToggleButtonGadgetClass, 'f', NULL, NULL,
             alhViewCallback, (XtPointer)MENU_VIEW_CONFIG,           (MenuItem *)NULL },
/* Next Callback for FSBox adding. Albert */
         { "Alarm Log File Window",         &xmToggleButtonGadgetClass, 'r', NULL, NULL,
             alhViewCallback1, (XtPointer)MENU_VIEW_ALARMLOG,         (MenuItem *)NULL },
/* End. Albert */
         { "Operation Log File Window",     &xmToggleButtonGadgetClass, 'O', NULL, NULL,
             alhViewCallback, (XtPointer)MENU_VIEW_OPMOD,         (MenuItem *)NULL },
         { "Group/Channel Properties Window", &xmToggleButtonGadgetClass, 'W', NULL, NULL,
             alhViewCallback, (XtPointer)MENU_VIEW_PROPERTIES,    (MenuItem *)NULL },

         {NULL},
     };
     
     static MenuItem setup_beep_menu[] = {
         { "Minor",      &xmPushButtonGadgetClass, 'M', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_MINOR,  (MenuItem *)NULL },
         { "Major",      &xmPushButtonGadgetClass, 'A', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_MAJOR,  (MenuItem *)NULL },
         { "Invalid",      &xmPushButtonGadgetClass, 'V', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_INVALID,  (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem setup_filter_menu[] = {
         { "No filter",      &xmPushButtonGadgetClass, 'N', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_FILTER_NONE,  (MenuItem *)NULL },
         { "Active Alarms Only",      &xmPushButtonGadgetClass, 'A', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_FILTER_ACTIVE,  (MenuItem *)NULL },
         { "Unacknowledged Alarms Only",      &xmPushButtonGadgetClass, 'U', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_FILTER_UNACK,  (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem setup_menu[] = {
         { "Display Filter...",        &xmPushButtonGadgetClass, 'F', NULL, NULL,
                                      0, 0,    (MenuItem *)setup_filter_menu },
         { "Beep Severity...",       &xmPushButtonGadgetClass, 'B', NULL, NULL,
                                      0, 0,    (MenuItem *)setup_beep_menu },
         { "Silence Forever",  &xmToggleButtonGadgetClass, 'S', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_SILENCE_FOREVER,(MenuItem *)NULL },
         { "New Alarm Log File Name...",  &xmPushButtonGadgetClass, 'L', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_ALARMLOG,     (MenuItem *)NULL },
         { "New Oper. Log File Name...",  &xmPushButtonGadgetClass, 'O', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_OPMOD,     (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem help_menu[] = {
/* HELP NOT IMPLEMENTED YET
         { "Help Topics",       &xmPushButtonGadgetClass, 'H', "Ctrl<Key>H", "Ctrl+H",
             alhHelpCallback, (XtPointer)MENU_HELP_TOPICS, (MenuItem *)NULL },
*/
#if  XmVersion && XmVersion >= 1002
         { "About ALH",     &xmPushButtonGadgetClass, 'A', NULL, NULL,
             alhHelpCallback, (XtPointer)MENU_HELP_ABOUT, (MenuItem *)NULL },
#endif
         {NULL},
     };
     
     Widget menubar,widget;

     menubar = XmCreateMenuBar(parent, "menubar",   NULL, 0);

     widget = buildPulldownMenu(menubar, "File",     'F', TRUE, file_menu, user_data);
     widget = buildPulldownMenu(menubar, "Action",   'A', TRUE, action_menu, user_data);
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

static void alhFileCallback( Widget widget, XtPointer calldata, XtPointer cbs)
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
             unmapArea_callback(area->toplevel,area->form_main,cbs);
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

static void alhActionCallback( Widget widget, XtPointer calldata, XtPointer cbs)
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

                       ack_callback(widget,line,cbs);
                  }
                  else {
                       line = (struct anyLine *)link->lineGroupW;
                       if (line && line->pwindow == (void *)area->selectionWindow ){
                            ack_callback(widget,line,cbs);
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
                  if (sllFirst(&(link->GuideList))){
                       guidance_callback(wline->guidance,(GCLINK *)link, cbs);
                  }
                  else {
                       if (((GCLINK *)link)->pgcData->alias){
                            createDialog(area->form_main,XmDIALOG_WARNING,"No guidance for ",
                                 link->pgcData->alias);
                       } else {
                            createDialog(area->form_main,XmDIALOG_WARNING,"No guidance for ",
                                 link->pgcData->name);
                       }
                  }
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

     }
}
 

/******************************************************
  alhViewCallback
******************************************************/

static void alhViewCallback( Widget widget, XtPointer calldata, XtPointer cbs)
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

        case MENU_VIEW_PROPERTIES:

             /* Display Group/Chan Properties */
             propShowDialog(area, widget);
             break;

     }
}
 
/* ___________ FileSelect for AlhView. Albert ________________________ */
static void alhViewCallback1(widget, item, cbs)
    Widget widget;
    XtPointer item;
    XtPointer cbs;
{
    ALINK   *area;
    Widget dialog;
    Arg al[10];
    int ac=0;
    XmString Xpattern,Xtitle,Xcurrentdir;
    char *pattern=ALARMLOG_PATTERN;
    char *title="Alarm Log File";
    if (!_time_flag) alhViewCallback(widget, item, cbs);
    else 
    {   
    XtVaGetValues(widget, XmNuserData, &area, NULL); 
    dialog=XmCreateFileSelectionDialog(area->form_main,"dialog",al,ac);
    XtAddCallback(dialog,XmNokCallback,dialogCbOk,widget);
    XtAddCallback(dialog,XmNcancelCallback,dialogCbCansel,NULL);
    XtUnmanageChild(XmFileSelectionBoxGetChild(dialog,
        XmDIALOG_HELP_BUTTON));
    Xtitle = XmStringCreateSimple(title);
    Xpattern = XmStringCreateSimple(psetup.logFile);
    Xcurrentdir = XmStringCreateSimple(psetup.configDir);
    XtVaSetValues(dialog,
          XmNdirectory,     Xcurrentdir,
          XmNdialogTitle,   Xtitle,
          XmNdirMask,       Xpattern,
          NULL);
    XtManageChild(dialog);
    XmStringFree(Xpattern);
    XmStringFree(Xtitle);
    }
}

void dialogCbCansel(w,client_data,call_data)
    Widget w;
    int client_data;
    XmSelectionBoxCallbackStruct *call_data;
{
    XtUnmanageChild(w);
}

void dialogCbOk(w,wdgt,call_data)
    Widget w;
    Widget wdgt;
    XmSelectionBoxCallbackStruct *call_data;
{
    ALINK   *area;
    char *s;
    XmStringGetLtoR(call_data->value,XmSTRING_DEFAULT_CHARSET,&s);
    strcpy(FS_filename,s);
    XtFree(s);
    XtVaGetValues(wdgt, XmNuserData, &area, NULL);
    fileViewWindow(area->form_main,ALARM_FILE,wdgt);
    XtUnmanageChild(w);
}
/* _____________________ End. Albert ___________________ */
 



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

             silenceForeverChangeState();
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

static void alhHelpCallback( Widget widget, XtPointer calldata, XtPointer cbs)
{
     int item=(int)calldata;
     ALINK  *area;

     XtVaGetValues(widget, XmNuserData, &area, NULL);

     switch (item){

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

void awRowWidgets(line, area)
     struct anyLine  *line;
     void *area;
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

          if (alGuidanceExists(link)) {
               XtManageChild(wline->guidance);
               XtAddCallback(wline->guidance, XmNactivateCallback, 
                    (XtCallbackProc)guidance_callback, link);
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
          if (alGuidanceExists(link)) {
               XtVaSetValues(wline->guidance,XmNx,nextX,NULL);
               XtVaSetValues(wline->guidance,XmNuserData,line->alias,NULL);
               XtManageChild(wline->guidance);
               XtAddCallback(wline->guidance, XmNactivateCallback, 
                    (XtCallbackProc)guidance_callback, link);
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

void awUpdateRowWidgets(line)
     struct anyLine  *line;
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

