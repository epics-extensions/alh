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
void alhRowWidgetsTree(gline)               Create line widgets for treeWindow
     struct groupLine  *gline;
*
void alhRowWidgetsGroup(line)               Create line widgets for groupWindow
     struct anyLine  *line;
*
void awUpdateRowWidgets(line)                 Update line widgets
     struct anyLine  *line;
*
*************************************************************************
*/

#include <stdlib.h>
#include <stdio.h>

#include <alarm.h>

#include <ax.h>
#include <alh.h>
#include <axArea.h>
#include <axSubW.h>
#include <line.h>
#include <alLib.h>
#include <sllLib.h>

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
extern char *bg_char[];
extern Pixel bg_pixel[];
extern Pixel channel_bg_pixel;
extern struct setup psetup;
extern Widget versionPopup;


#ifdef __STDC__

/* prototypes for static routines */
static void alhFileCallback( Widget widget, int item, XmAnyCallbackStruct *cbs);
static void alhActionCallback( Widget widget, int item, XmAnyCallbackStruct *cbs);
static void alhViewCallback( Widget widget, int item, XmAnyCallbackStruct *cbs);
static void alhSetupCallback( Widget widget, int item, XmAnyCallbackStruct *cbs);
static void alhHelpCallback( Widget widget, int item, XmAnyCallbackStruct *cbs);

#else 

static void alhFileCallback();
static void alhActionCallback();
static void alhViewCallback();
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
/*
         { "Exit ",      &xmPushButtonGadgetClass, 'x', "Ctrl<Key>X", "Ctrl+X",
             alhFileCallback, (XtPointer)MENU_FILE_QUIT,   (MenuItem *)NULL },
*/
         {NULL},
     };
     
     static MenuItem add_menu[] = {
         { "Add",     &xmPushButtonGadgetClass, 'A', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_ADD_ADD,     (MenuItem *)NULL },
         { "Cancel",  &xmPushButtonGadgetClass, 'C', NULL, NULL, 
             alhActionCallback, (XtPointer)MENU_ACTION_ADD_CANCEL,  (MenuItem *)NULL },
         { "Reset",   &xmPushButtonGadgetClass, 'R', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_ADD_RESET,   (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem enable_menu[] = {
         { "Enable",   &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_ENABLE_ENABLE,   (MenuItem *)NULL },
         { "Disable",  &xmPushButtonGadgetClass, '\0', NULL, NULL, 
             alhActionCallback, (XtPointer)MENU_ACTION_ENABLE_DISABLE,  (MenuItem *)NULL },
         { "Reset",    &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_ENABLE_RESET,    (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem ack_menu[] = {
         { "Ack",     &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_ACK_ACK,   (MenuItem *)NULL },
         { "NoAck",   &xmPushButtonGadgetClass, '\0', NULL, NULL, 
             alhActionCallback, (XtPointer)MENU_ACTION_ACK_NOACK,  (MenuItem *)NULL },
         { "Reset",   &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_ACK_RESET,    (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem ackt_menu[] = {
         { "Ack Transient",     &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_ACKT_ACK,   (MenuItem *)NULL },
         { "NoAck Transient",   &xmPushButtonGadgetClass, '\0', NULL, NULL, 
             alhActionCallback, (XtPointer)MENU_ACTION_ACKT_NOACK,  (MenuItem *)NULL },
         { "Reset",             &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_ACKT_RESET,    (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem log_menu[] = {
         { "Log",     &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_LOG_LOG,   (MenuItem *)NULL },
         { "NoLog",   &xmPushButtonGadgetClass, '\0', NULL, NULL, 
             alhActionCallback, (XtPointer)MENU_ACTION_LOG_NOLOG,  (MenuItem *)NULL },
         { "Reset",   &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_LOG_RESET,    (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem props_menu[] = {
         { "Summary ...",             &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_SUMMARY, (MenuItem *)NULL },
         { "Force Process Variable ...", &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_FORCEPV, (MenuItem *)NULL },
         { "Force Mask ...",          &xmPushButtonGadgetClass, '\0', NULL, NULL,
             alhActionCallback, (XtPointer)MENU_ACTION_FORCEPV_MASK, (MenuItem *)NULL },
         { "Add/Cancel Alarms",       &xmCascadeButtonGadgetClass, '\0', NULL, NULL,
             0, 0, add_menu },
         { "Enable/Disable Alarms",   &xmCascadeButtonGadgetClass, '\0', NULL, NULL,
             0, 0, enable_menu },
         { "Ack/NoAck Alarms",        &xmCascadeButtonGadgetClass, '\0', NULL, NULL,
             0, 0, ack_menu },
         { "Ack/NoAck Transient Alarms", &xmCascadeButtonGadgetClass, '\0', NULL, NULL,
             0, 0, ackt_menu },
         { "Log/NoLog Alarms",        &xmCascadeButtonGadgetClass, '\0', NULL, NULL,
             0, 0, log_menu },
         {NULL},
     };
     
     static MenuItem action_menu[] = {
         { "Acknowledge Alarm",      &xmPushButtonGadgetClass, 'A', "Ctrl<Key>A", "Ctrl+A",
             alhActionCallback, (XtPointer)MENU_ACTION_ACK,      (MenuItem *)NULL },
         { "Display Guidance",       &xmPushButtonGadgetClass, 'G', "Ctrl<Key>G", "Ctrl+G",
             alhActionCallback, (XtPointer)MENU_ACTION_GUIDANCE, (MenuItem *)NULL },
         { "Start Related Process",  &xmPushButtonGadgetClass, 'P', "Ctrl<Key>P", "Ctrl+P",
             alhActionCallback, (XtPointer)MENU_ACTION_PROCESS,  (MenuItem *)NULL },
         { "Modify Alarm Settings",  &xmPushButtonGadgetClass, 'S', NULL, NULL,
             0, 0, props_menu },
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
         { "Alarm Log File Window",         &xmToggleButtonGadgetClass, 'r', NULL, NULL,
             alhViewCallback, (XtPointer)MENU_VIEW_ALARMLOG,         (MenuItem *)NULL },
         { "Operation Log File Window",     &xmToggleButtonGadgetClass, 'O', NULL, NULL,
             alhViewCallback, (XtPointer)MENU_VIEW_OPMOD,         (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem setup_beep_menu[] = {
         { "Minor",      &xmPushButtonGadgetClass, 'M', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_MINOR,  (MenuItem *)NULL },
         { "Major",      &xmPushButtonGadgetClass, 'A', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_MAJOR,  (MenuItem *)NULL },
         { "Valid",      &xmPushButtonGadgetClass, 'V', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_INVALID,  (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem setup_menu[] = {
         { "Active Alarms Only", &xmToggleButtonGadgetClass, 'A', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_ACTIVE,     (MenuItem *)NULL },
         { "Beep Condition",        &xmPushButtonGadgetClass, 'B', NULL, NULL,
                                      0, 0,    (MenuItem *)setup_beep_menu },
         { "New Alarm Log File Name...",  &xmPushButtonGadgetClass, 'L', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_ALARMLOG,     (MenuItem *)NULL },
         { "New Oper. Log File Name...",  &xmPushButtonGadgetClass, 'O', NULL, NULL,
             alhSetupCallback, (XtPointer)MENU_SETUP_OPMOD,     (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem help_menu[] = {
         { "On Context",       &xmPushButtonGadgetClass, 'C', "Ctrl<Key>H", "Ctrl+H",
             alhHelpCallback, "expandOne",         (MenuItem *)NULL },
         { "On Windows",          &xmPushButtonGadgetClass, 'W', NULL, NULL,
             alhHelpCallback, "expandBranch",      (MenuItem *)NULL },
         { "On Keys",             &xmPushButtonGadgetClass, 'K', NULL, NULL,
             alhHelpCallback, "expandAll",         (MenuItem *)NULL },
         { "Index",        &xmPushButtonGadgetClass, 'I', NULL, NULL,
             alhHelpCallback, "collapseBranch",    (MenuItem *)NULL },
         { "On Help",              &xmPushButtonGadgetClass, 'H', NULL, NULL,
             alhHelpCallback, "nameOnly",          (MenuItem *)NULL },
         { "Tutorial",         &xmPushButtonGadgetClass, 'T', NULL, NULL,
             alhHelpCallback, "allProperties",     (MenuItem *)NULL },
         { "",                       &xmSeparatorGadgetClass,  '\0', NULL, NULL,
             NULL,    NULL,   (MenuItem *)NULL },
#if  XmVersion && XmVersion >= 1002
         { "Version",     &xmPushButtonGadgetClass, 'V', NULL, NULL,
             alhHelpCallback, (XtPointer)MENU_HELP_VERSION, (MenuItem *)NULL },
#endif
         {NULL},
     };
     
/*
     static MenuItem bar_menu[] = {
           { "File",     &xmCascadeButtonGadgetClass, 'F', NULL, NULL,
               0, 0, file_menu },
           { "Action",   &xmCascadeButtonGadgetClass, 'A', NULL, NULL,
               0, 0, action_menu },
           { "View",     &xmCascadeButtonGadgetClass, 'V', NULL, NULL,
               0, 0, view_menu },
           { "Setup",    &xmCascadeButtonGadgetClass, 'S', NULL, NULL,
               0, 0, setup_menu },
           { "Help",     &xmCascadeButtonGadgetClass, 'H', NULL, NULL,
               0, 0, help_menu },
         {NULL},
     };

*/
     Widget menubar,widget;

     menubar = XmCreateMenuBar(parent, "menubar",   NULL, 0);
/*
          XmNtearOffModel,             XmTEAR_OFF_ENABLED,
     XtVaSetValues(menubar, XmNtearOffModel, XmTEAR_OFF_ENABLED, NULL);
     menubar = XtVaCreateWidget("menubar",
          xmRowColumnWidgetClass,      parent,
          XmNrowColumnType,            XmMENU_BAR,
          XmNorientation,              XmHORIZONTAL,
          XmNshadowThickness,          2,
          XmNtopAttachment,            XmATTACH_FORM,
          XmNrightAttachment,          XmATTACH_FORM,
          XmNleftAttachment,           XmATTACH_FORM,
          NULL);
*/

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

static void alhFileCallback(widget, item, cbs)
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
{
     ALINK      *area;
     int         status;


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

static void alhActionCallback(widget, item, cbs)
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
{
     ALINK               *area;
     Widget               dialog;
     Widget               parent;
     GCLINK              *link;
     struct anyLine      *line;
     struct gcData       *gcdata;

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
             if (area->selectionType == GROUP){
                  if (sllFirst(&(link->GuideList))){
                       GroupGuidance_callback(area->toplevel,(GLINK *)link, cbs);
                  }
                  else {
                       createDialog(area->form_main,XmDIALOG_WARNING,"No guidance for group ",
                            link->pgcData->name);
                  }
             }
             else {
                  if (area->selectionType == CHANNEL){
                       if (sllFirst(&(link->GuideList))){
                            ChannelGuidance_callback(area->toplevel,(CLINK *)link, cbs);
                       }
                       else {
                            createDialog(area->form_main,XmDIALOG_WARNING,"No guidance for channel ",
                                 link->pgcData->name);
                       }
                  }
                  else {
                       createDialog(area->form_main,XmDIALOG_WARNING,
                            "Please select an alarm group or channel first."," ");
                  }
             }
             break;

        case MENU_ACTION_PROCESS:

             /* Start Related Process */
             link = (GCLINK *)area->selectionLink;
             if (link){
                  if (alProcessExists(link)){
                       relatedProcess_callback(widget,link, cbs);
                  } else {
                       createDialog(area->form_main,XmDIALOG_WARNING,"No related process for: ",
                            link->pgcData->name );
                  }

             } else {
                  createDialog(area->form_main,XmDIALOG_WARNING,
                       "Please select an alarm group or channel first."," ");
             }
             break;


          case MENU_ACTION_SUMMARY:

               parent = area->form_main;
               link = (GCLINK *)area->selectionLink;
               gcdata = link->pgcData;
               if (area->selectionType == GROUP){
                    dialog = createShowGroupMasksDialog(parent,(struct groupData *)gcdata);
                    XtManageChild(dialog);
                    break;
               }
               if (area->selectionType == CHANNEL){
                    dialog = createShowChanMasksDialog(parent,(struct chanData *)link->pgcData);
                    XtManageChild(dialog);
                    break;
               }
               createDialog(area->form_main,XmDIALOG_WARNING,
                    "Please select an alarm group or channel first."," ");
               break;

          case MENU_ACTION_FORCEPV:

               parent = area->form_main;
               link = (GCLINK *)area->selectionLink;
               if (area->selectionType == GROUP){
                    dialog = createForcePVGroupDialog(parent,(GLINK *)link);
                    XtManageChild(dialog);
                    break;
               }
               if (area->selectionType == CHANNEL){
                    dialog = createForcePVChanDialog(parent,(CLINK *)link);
                    XtManageChild(dialog);
                    break;
               }
               createDialog(area->form_main,XmDIALOG_WARNING,
                    "Please select an alarm group or channel first."," ");
               break;

          case MENU_ACTION_FORCEPV_MASK:

               parent = area->form_main;
               link = (GCLINK *)area->selectionLink;
               if (area->selectionType == GROUP){
                    dialog = createForceGMaskDialog(parent,(GLINK *)link);
                    XtManageChild(dialog);
                    break;
               }
               if (area->selectionType == CHANNEL){
                    dialog = createForceCMaskDialog(parent,(CLINK *)link);
                    XtManageChild(dialog);
                    break;
               }
               createDialog(parent,XmDIALOG_WARNING,
                    "Please select an alarm group or channel first."," ");
               break;

          case MENU_ACTION_ADD_ADD:
               changeMasks_callback(area,0);
               break;
          case MENU_ACTION_ADD_CANCEL:
               changeMasks_callback(area,1);
               break;
          case MENU_ACTION_ADD_RESET:
               changeMasks_callback(area,2);
               break;
          case MENU_ACTION_ENABLE_ENABLE:
               changeMasks_callback(area,10);
               break;
          case MENU_ACTION_ENABLE_DISABLE:
               changeMasks_callback(area,11);
               break;
          case MENU_ACTION_ENABLE_RESET:
               changeMasks_callback(area,12);
               break;
          case MENU_ACTION_ACK_ACK:
               changeMasks_callback(area,20);
               break;
          case MENU_ACTION_ACK_NOACK:
               changeMasks_callback(area,21);
               break;
          case MENU_ACTION_ACK_RESET:
               changeMasks_callback(area,22);
               break;
          case MENU_ACTION_ACKT_ACK:
               changeMasks_callback(area,30);
               break;
          case MENU_ACTION_ACKT_NOACK:
               changeMasks_callback(area,31);
               break;
          case MENU_ACTION_ACKT_RESET:
               changeMasks_callback(area,32);
               break;
          case MENU_ACTION_LOG_LOG:
               changeMasks_callback(area,40);
               break;
          case MENU_ACTION_LOG_NOLOG:
               changeMasks_callback(area,41);
               break;
          case MENU_ACTION_LOG_RESET:
               changeMasks_callback(area,42);
               break;


     }
}
 

/******************************************************
  alhViewCallback
******************************************************/

static void alhViewCallback(widget, item, cbs)
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
{
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

     }
}
 

/******************************************************
  alhSetupCallback
******************************************************/

static void alhSetupCallback(widget, item, cbs)
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
{
     ALINK      *area;


     XtVaGetValues(widget, XmNuserData, &area, NULL);

     switch (item){

        case MENU_SETUP_ACTIVE:
/*
             createDialog(area->form_main,XmDIALOG_INFORMATION,"Active Alarms Only",":  not implemented yet.");
*/
             if (XmToggleButtonGadgetGetState(widget)) {
                  area->viewFilter = alFilterAlarmsOnly;
             } else {
                  area->viewFilter = alFilterAll;
             }
             createConfigDisplay(area,EXPANDCOLLAPSE1);
             break;

        case MENU_SETUP_BEEP_MINOR:

             psetup.beepSevr = MINOR_ALARM;
             break;

        case MENU_SETUP_BEEP_MAJOR:

             psetup.beepSevr = MAJOR_ALARM;
             break;

        case MENU_SETUP_BEEP_INVALID:

             psetup.beepSevr = INVALID_ALARM;
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

static void alhHelpCallback(widget, item, cbs)
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
{
     ALINK  *area;

     XtVaGetValues(widget, XmNuserData, &area, NULL);

     switch (item){

        case MENU_HELP_VERSION:

             if (productDescriptionShell)  XtPopup(productDescriptionShell,XtGrabNone);
             break;

        default:

             createDialog(area->form_main,XmDIALOG_INFORMATION,"Help is not available in this release."," ");
             break;
     }
}

 
/******************************************************
  awRowWidgetsTree
******************************************************/

void awRowWidgetsTree(gline)
     struct groupLine  *gline;
{
/* this routine should have parms 'Widget parent' and 'void *area' and
    subWindow should be  changed to 'void * subWindow'   */

     Widget parent;
     struct subWindow *subWindow;

     XmString   str;
     void      *area;
     WLINE     *wline;
     GLINK     *glink;
     Dimension  width;
     Position   nextX;

     subWindow=((struct subWindow *)gline->pwindow);
     parent = subWindow->drawing_area;
     area = subWindow->area;

     wline=(WLINE *)gline->wline;
     glink = (GLINK *)gline->glink;

     /* create row widgets */
     if (wline->row_widget == NULL) {

          wline->row_widget = XtVaCreateWidget("rowWidget",
               xmDrawingAreaWidgetClass,  parent,
               XmNy,                        calcRowYValue(subWindow,gline->lineNo),
               XmNnavigationType,           XmNONE,
               XmNorientation,              XmHORIZONTAL,
               XmNmarginHeight,             0,
               XmNmarginWidth,              0,
               NULL);

          str = XmStringCreateSimple(glink->pgroupData->treeSym);
          wline->treeSym = XtVaCreateManagedWidget("treeSym",
               xmLabelGadgetClass,        wline->row_widget,
               XmNlabelString,            str,
               XmNmarginHeight,           0,
               NULL);
          XmStringFree(str);
          XtVaGetValues(wline->treeSym,XmNwidth,&width,NULL);
          nextX = width + 3;

          str = XmStringCreateSimple(bg_char[gline->unackSevr]);
          wline->ack = XtVaCreateManagedWidget("ack",
               xmPushButtonWidgetClass,   wline->row_widget,
               XmNmarginHeight,           0,
               XmNlabelString,            str,
               XmNuserData,               (XtPointer)area,
               XmNsensitive,              FALSE,
               XmNx,                      nextX,
               NULL);
          XmStringFree(str);
          XtAddCallback(wline->ack, XmNactivateCallback, 
               (XtCallbackProc)ack_callback, gline);
          XtVaGetValues(wline->ack,XmNwidth,&width,NULL);
          nextX = nextX + width + 3;
    
          str = XmStringCreateSimple(bg_char[gline->curSevr]);
          wline->sevr = XtVaCreateManagedWidget("sevr",
               xmLabelWidgetClass,        wline->row_widget,
               XmNlabelString,            str,
               XmNx,                      nextX,
               XmNy,                      2,
               NULL);
          XmStringFree(str);
          XtVaGetValues(wline->sevr,XmNwidth,&width,NULL);
          nextX = nextX + width + 3;
    
          str = XmStringCreateSimple(gline->pname);
          wline->name = XtVaCreateManagedWidget("pushButtonName",
               xmPushButtonWidgetClass,   wline->row_widget,
               XmNmarginHeight,           0,
               XmNlabelString,            str,
               XmNuserData,               (XtPointer)subWindow,
               XmNx,                      nextX,
               NULL);
          XmStringFree(str);
          XtAddCallback(wline->name, XmNactivateCallback, 
               (XtCallbackProc)nameTreeW_callback, gline);
          XtVaGetValues(wline->name,XmNwidth,&width,NULL);
          nextX = nextX + width + 3;
    
          wline->arrow = XtVaCreateWidget("pushButtonArrow",
               xmArrowButtonWidgetClass,   wline->row_widget,
               XmNshadowThickness,        0,
/*
               XmNmarginHeight,           0,
*/
               XmNarrowDirection,         XmARROW_RIGHT,
               XmNuserData,               (XtPointer)area,
               XmNx,                      nextX,
               XmNy,                      2,
               NULL);
          if(sllFirst(&(glink->subGroupList))){
               XtManageChild(wline->arrow);
               XtAddCallback(wline->arrow, XmNactivateCallback, 
                    (XtCallbackProc)arrow_callback, glink);
               XtVaGetValues(wline->arrow,XmNwidth,&width,NULL);
               nextX = nextX + width + 3;
          }

          wline->guidance = XtVaCreateWidget("G",
               xmPushButtonWidgetClass,   wline->row_widget,
               XmNmarginHeight,           0,
               XmNuserData,               (XtPointer)area,
               XmNx,                      nextX,
               NULL);
          if (alGuidanceExists((GCLINK *)glink)) {
               XtManageChild(wline->guidance);
               XtAddCallback(wline->guidance, XmNactivateCallback, 
                    (XtCallbackProc)guidance_callback, gline);
               XtVaGetValues(wline->guidance,XmNwidth,&width,NULL);
               nextX = nextX + width + 3;
          }

     
          wline->process = XtVaCreateWidget("P",
               xmPushButtonWidgetClass,   wline->row_widget,
               XmNmarginHeight,           0,
               XmNuserData,               (XtPointer)area,
               XmNx,                      nextX,
               NULL);
          if (alProcessExists((GCLINK *)glink) ){
               XtManageChild(wline->process);
               XtAddCallback(wline->process, XmNactivateCallback, 
                    (XtCallbackProc)relatedProcess_callback, glink);
               XtVaGetValues(wline->process,XmNwidth,&width,NULL);
               nextX = nextX + width + 3;
          }

     
          str = XmStringCreateSimple(gline->mask);
          wline->mask = XtVaCreateManagedWidget("mask",
               xmLabelWidgetClass,        wline->row_widget,
               XmNlabelString,            str,
               XmNx,                      nextX,
               XmNy,                      2,
               NULL);
          XmStringFree(str);
          XtVaGetValues(wline->mask,XmNwidth,&width,NULL);
          nextX = nextX + width + 3;

          str = XmStringCreateSimple(gline->message);
          wline->message = XtVaCreateManagedWidget("message",
               xmLabelWidgetClass,        wline->row_widget,
               XmNlabelString,            str,
               XmNx,                      nextX,
               XmNy,                      2,
               NULL);
          XmStringFree(str);

          awUpdateRowWidgets((struct anyLine *)gline);

          XtManageChild(wline->row_widget);
     }

     else

     /* else modify existing  row widgets */
     {
          if ( wline->row_widget && XtIsManaged(wline->row_widget)){
               XtUnmanageChild(wline->row_widget);
          }

          str = XmStringCreateSimple(glink->pgroupData->treeSym);
          XtVaSetValues(wline->treeSym,
               XmNlabelString,            str,
               NULL);
          XmStringFree(str);
          XtVaGetValues(wline->treeSym,XmNwidth,&width,NULL);
          nextX = width + 3;

          if (gline->unackSevr == FALSE) {
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
               (XtCallbackProc)ack_callback, gline);
          XtVaGetValues(wline->ack,XmNwidth,&width,NULL);
          nextX = nextX + width +3;
     

          XtVaSetValues(wline->sevr,XmNx,nextX,NULL);
          XtVaGetValues(wline->sevr,XmNwidth,&width,NULL);
          nextX = nextX + width +3;


          str = XmStringCreateSimple(gline->pname);
          XtVaSetValues(wline->name,
               XmNlabelString,            str,
               XmNx,                      nextX,
               NULL);
          XmStringFree(str);
          XtRemoveAllCallbacks(wline->name, XmNactivateCallback);
          XtAddCallback(wline->name, XmNactivateCallback, 
               (XtCallbackProc)nameTreeW_callback, gline);
          XtVaGetValues(wline->name,XmNwidth,&width,NULL);
          nextX = nextX + width +3;


          if (XtHasCallbacks(wline->arrow,XmNactivateCallback))
               XtRemoveAllCallbacks(wline->arrow, XmNactivateCallback);
          if(sllFirst(&(glink->subGroupList))){
               XtVaSetValues(wline->arrow,XmNx,nextX,NULL);
               XtManageChild(wline->arrow);
               XtAddCallback(wline->arrow, XmNactivateCallback,
                   (XtCallbackProc)arrow_callback, glink);
               XtVaGetValues(wline->arrow,XmNwidth,&width,NULL);
               nextX = nextX + width +3;
    
          } else {
               XtUnmanageChild(wline->arrow);
          }
     
          if (XtHasCallbacks(wline->guidance,XmNactivateCallback))
               XtRemoveAllCallbacks(wline->guidance, XmNactivateCallback);
          if (alGuidanceExists((GCLINK *)glink)) {
               XtVaSetValues(wline->guidance,XmNx,nextX,NULL);
               XtManageChild(wline->guidance);
               XtAddCallback(wline->guidance, XmNactivateCallback,
                    (XtCallbackProc)guidance_callback, gline);
               XtVaGetValues(wline->guidance,XmNwidth,&width,NULL);
               nextX = nextX + width +3;
          } else {
               XtUnmanageChild(wline->guidance);
          }
     
          if (XtHasCallbacks(wline->process,XmNactivateCallback))
               XtRemoveAllCallbacks(wline->process, XmNactivateCallback);
          if (alProcessExists((GCLINK *)glink) ){
               XtVaSetValues(wline->process,XmNx,nextX,NULL);
               XtManageChild(wline->process);
               XtAddCallback(wline->process, XmNactivateCallback,
                    (XtCallbackProc)relatedProcess_callback, glink);
               XtVaGetValues(wline->process,XmNwidth,&width,NULL);
               nextX = nextX + width +3;
          } else {
               XtUnmanageChild(wline->process);
          }
     
          XtVaSetValues(wline->mask,XmNx,nextX,NULL);
          XtVaGetValues(wline->mask,XmNwidth,&width,NULL);
          nextX = nextX + width +3;

          XtVaSetValues(wline->message,XmNx,nextX,NULL);

          awUpdateRowWidgets((struct anyLine *)gline);

          XtManageChild(wline->row_widget);

     }

}


/******************************************************
  awRowWidgetsGroup
******************************************************/

void awRowWidgetsGroup(line)
     struct anyLine  *line;
{
/* this routine should have parms 'Widget parent' and 'void *area' and
    subWindow should be  changed to 'void * subWindow'   */

     Widget parent;
     struct subWindow *subWindow;

     XmString str;
     WLINE  *wline;
     void *area;
     void *link;
     int nextX;
     Dimension width;
     Position x;


     subWindow=((struct subWindow *)line->pwindow);
     parent = subWindow->drawing_area;
     area=subWindow->area;

     wline=(WLINE *)line->wline;
     link = line->link;



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


          str = XmStringCreateSimple(bg_char[line->unackSevr]);
          wline->ack = XtVaCreateManagedWidget("ack",
               xmPushButtonWidgetClass,   wline->row_widget,
               XmNmarginHeight,           0,
               XmNlabelString,            str,
               XmNsensitive,              FALSE,
               XmNuserData,               (XtPointer)area,
               NULL);
          XmStringFree(str);
          XtAddCallback(wline->ack, XmNactivateCallback, 
               (XtCallbackProc)ack_callback, line);
          XtVaGetValues(wline->ack,XmNwidth,&width,XmNx,&x,NULL);
          nextX = x + width + 3;
    

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
     

          str = XmStringCreateSimple(line->pname);
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
          XtAddCallback(wline->name, XmNactivateCallback, 
               (XtCallbackProc)nameGroupW_callback, line);
          XtVaGetValues(wline->name,XmNwidth,&width,NULL);
          nextX = nextX + width + 3;

          wline->guidance = XtVaCreateWidget("G",
               xmPushButtonWidgetClass,   wline->row_widget,
               XmNmarginHeight,           0,
               XmNuserData,               (XtPointer)area,
               XmNx,                      nextX,
               NULL);

          if (alGuidanceExists(link)) {
               XtManageChild(wline->guidance);
               XtAddCallback(wline->guidance, XmNactivateCallback, 
                    (XtCallbackProc)guidance_callback, line);
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

          XtRemoveAllCallbacks(wline->ack, XmNactivateCallback);
          XtAddCallback(wline->ack, XmNactivateCallback, 
               (XtCallbackProc)ack_callback, line);
     

          str = XmStringCreateSimple(line->pname);
          XtVaSetValues(wline->name,
               XmNlabelString,            str,
               NULL);
          XmStringFree(str);
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
          XtRemoveAllCallbacks(wline->name, XmNactivateCallback);
          XtAddCallback(wline->name, XmNactivateCallback, 
               (XtCallbackProc)nameGroupW_callback, line);
          XtVaGetValues(wline->name,XmNwidth,&width,XmNx,&x,NULL);
          nextX = x + width + 3;

          if (XtHasCallbacks(wline->guidance,XmNactivateCallback))
               XtRemoveAllCallbacks(wline->guidance, XmNactivateCallback);
          if (alGuidanceExists(link)) {
               XtVaSetValues(wline->guidance,XmNx,nextX,NULL);
               XtManageChild(wline->guidance);
               XtAddCallback(wline->guidance, XmNactivateCallback, 
                    (XtCallbackProc)guidance_callback, line);
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
