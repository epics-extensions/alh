/*
 $Log$
 Revision 1.10  1996/09/20 15:06:18  jba
 BEEPSEVERITY bug fix.

 Revision 1.9  1996/08/19 13:53:37  jba
 Minor usage and mask printed output changes.

 Revision 1.8  1995/11/13 22:31:24  jba
 Added beepseverity command, ansi changes and other changes.

 * Revision 1.7  1995/10/20  16:50:23  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 *
 * Revision 1.6  1995/06/01  19:47:06  jba
 * Configuration mode bug fix
 *
 * Revision 1.5  1995/05/31  20:29:55  jba
 * fixed comment
 *
 * Revision 1.4  1995/05/30  15:58:05  jba
 * Added ALARMCOMMAND facility
 *
 * Revision 1.3  1995/02/28  16:43:36  jba
 * ansi c changes
 *
 * Revision 1.2  1994/06/22  21:17:05  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)axArea.c	1.14\t12/21/93";

/* axArea.c */
/*
 *      Author:		Janet Anderson
 *      Date:		05-04-92
 *
 *	Experimental Physics and Industrial Control System (EPICS)
 *
 *	Copyright 1991, the Regents of the University of California,
 *	and the University of Chicago Board of Governors.
 *
 *	This software was produced under  U.S. Government contracts:
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
 */

/*************************************************************
	routines defined in axArea.c
*************************************************************
*
*-----------------------------------------------------------------
*    routines related to the area structure
*-----------------------------------------------------------------
*
 -------------
 |  PUBLIC   |
 -------------
Widget buildPulldownMenu(parent,menu_title, Generic menu creation
     menu_mnemonic, tearOff, items, user_data)
     Widget parent;
     char *menu_title;
     char menu_mnemonic;
     int tearOff;
     MenuItem *items;
     XtPointer user_data;
*
void setupConfig(filename,program, areaOld)    Setup area with new config.file
     char      *filename;
     int       program;
     ALINK     *areaOld;
*
void createMainWindowWidgets(area)    Create area Main Window widgets
     ALINK *area;
*
void markActiveWidget(area,newWidget) Mark active widget border
     ALINK        *area;
     Widget        newWidget;
*
ALINK *setupArea(areaOld)                    Initialize an area 
     ALINK *areaOld;

*
int isTreeWindow(area,subWindow)      Returns TRUE if area treeWindow 
ALINK *area;
void * subWindow;
*
void scale_callback(widget,area,cbs)  Scale moved callback
     Widget widget;
     ALINK                 *area;
     XmScaleCallbackStruct *cbs;
* 
void markSelectionArea(area,line)     Set area selection values
     ALINK *area;
     struct anyLine  *line;
*
void changeBeepSeverityText(area)
     ALINK *area;

 -------------
 |  PRIVATE  |
 -------------
*

****************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include <cadef.h>
#include <alarm.h>
#include <ALH.bit>

#include <alh.h>
#include <line.h>
#include <axArea.h>
#include <sllLib.h>
#include <ax.h>

#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/CascadeBG.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/Scale.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>

/* global variables */
extern int DEBUG;
extern SLIST *areaList;
extern Widget toggle_button,toggle_button1;
extern struct setup psetup;
extern Display *display;
extern char *alarmSeverityString[];

ALINK *alhArea;

/* prototypes for static routines */
#ifdef __STDC__

static ALINK *setupArea( ALINK *areaOld);

#else

static ALINK *setupArea();

#endif /*__STDC__*/



/******************************************************
  buildPulldownMenu
******************************************************/
#ifdef __STDC__
Widget buildPulldownMenu( Widget parent, char *menu_title, char menu_mnemonic,
            int tearOff, MenuItem *items, XtPointer user_data)
#else
Widget buildPulldownMenu(parent, menu_title, menu_mnemonic, tearOff, items, user_data)
     Widget parent;
     char *menu_title;
     char menu_mnemonic;
     int tearOff;
     MenuItem *items;
     XtPointer user_data;
#endif
{
     Widget PullDown, cascade, widget;
     int i;
     XmString str;

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
                *items[i].class, PullDown,
                XmNuserData,    user_data,
                NULL);

        /* Make spacing of toggle button items the same as pushButtons */
        if (items[i].class == &xmToggleButtonWidgetClass ||
                 items[i].class == &xmToggleButtonGadgetClass)
                 XtVaSetValues(widget, XmNmarginHeight, 1, NULL);

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
                (items[i].class == &xmToggleButtonWidgetClass ||
                 items[i].class == &xmToggleButtonGadgetClass)?
                    XmNvalueChangedCallback : /* ToggleButton class */
                    XmNactivateCallback,      /* PushButton class */
                items[i].callback, items[i].callback_data);
     }
     return cascade;
}


/***************************************************
  setupConfig
****************************************************/

void setupConfig(filename, program, areaOld)
     char      *filename;
     int       program;
     ALINK     *areaOld;
{
     ALINK     *area;
     struct mainGroup *pmainGroup = NULL;
     XmString    str;
     SNODE *proot;
     static int firstTime = TRUE;

/*****************   ERROR:   the following line has problems  !!!!!!!
       2- it doesn't show message the second time it is managed 
     if (areaOld)
          createDialog(areaOld->form_main,XmDIALOG_WORKING,"Reading configuration file:",filename);
     XFlush did not work.  Need to read about XtAppNextEvent,XtDispatchEvent
*************************/

     /* initialize channel access */
     if (program == ALH) {
          if (firstTime) {
               firstTime = FALSE;
               alCaInit();
          }
          else  if (programId == ALH ) {
               alCaStop();
               alCaInit();
          }
     }

     /* Log new configfile filename */
     alLogSetupConfigFile(filename);

     /* Reset data for Current alarm window */
     resetCurrentAlarmWindow();

     /* create main group */
     pmainGroup = alAllocMainGroup();
     if (!pmainGroup ) {
         if (areaOld)
              createDialog(areaOld->form_main,XmDIALOG_ERROR,"mainGroup allocation error: ",filename);
         return;
     }

     /* reinitialize beep severity */
     psetup.beepSevr = 1;

     /* Read the config file  or create a minimal config file  */
     if (filename[0] != '\0'){
          if ( program == ALH) alGetConfig(pmainGroup,filename,CA_CONNECT_YES);
          else alGetConfig(pmainGroup,filename,CA_CONNECT_NO);

          strcpy(psetup.configFile,filename);
     } else{
          if (program == ACT) alCreateConfig(pmainGroup);
          else {
               pmainGroup->p1stgroup = alCopyGroup(areaOld->pmainGroup->p1stgroup);
               alSetPmainGroup(pmainGroup->p1stgroup, pmainGroup);
               alCaStart((SLIST *)pmainGroup);
          }
     }
     if ( sllFirst(pmainGroup)) {

          proot = sllFirst(pmainGroup);

          /* initialize group counters and group masks */
          alInitialize((GLINK *)proot);

          /*  initialize unused area  */
          area = setupArea(0);

          /* initialize subWindow create/modify line routines  */
          setLineRoutine(area,area->treeWindow,program);
          setLineRoutine(area,area->groupWindow,program);

          pmainGroup->area = area;
          area->programId = program;
          area->pmainGroup = pmainGroup;
          area->changed = FALSE;

          /* activate runtime window for ALH */
          if (program == ALH ){
               area->blinkString = alAlarmGroupName((GLINK *)proot);
               alHighestSystemSeverity((GLINK *)proot);
               createRuntimeWindow(area);
          }

          /* create/display main window for ACT */
          if (program == ACT )  createMainWindow_callback(0,area,0); 

          createConfigDisplay(area,EXPANDCOLLAPSE1);

          if (program == ALH ) alhArea = area;

          area->managed = TRUE;

          if ( DEBUG == 1 ){
               printf("\n\n####### Start of printConfig ###########\n");
               printConfig(area->pmainGroup);
               printf(  "\n####### End of printConfig   ###########\n\n");
          }
     
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

          if (areaOld) {
               if (programId == ALH ) {
                    alCaStart((SLIST *)areaOld->pmainGroup);
               }

               areaOld->managed = TRUE;

               if (areaOld->form_main )
               createDialog(areaOld->form_main,XmDIALOG_WARNING,"Configuration file error: ",filename);
               else if (areaOld->runtimeForm)
               createDialog(areaOld->runtimeForm,XmDIALOG_WARNING,"Configuration file error: ",filename);
          }
     }

      return;
}

/******************************************************
  markActiveWidget
******************************************************/

void markActiveWidget(area,newWidget)
     ALINK        *area;
     Widget        newWidget;
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
  setupArea
******************************************************/

static ALINK *setupArea(areaOld)
     ALINK *areaOld;
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
          

          /* cancel channel access */
/*
          alCaCancel((SLIST *)area->pmainGroup);
*/

          /* Delete the current config */
          if (area->pmainGroup){

               proot = sllFirst(area->pmainGroup);
               if (proot) alDeleteGroup((GLINK *)proot);
          }

          area->pmainGroup = NULL;

     /* or create a new work area */
     } else {
          area=(ALINK *)calloc(1,sizeof(ALINK));
          sllAdd(areaList,(SNODE *)area);

          /* Set alarm  filter to default value */
          area->viewFilter = alFilterAll;

          area->blinkString = NULL;

          area->runtimeToplevel = NULL;
          area->treeWindow = createSubWindow(area);
          area->groupWindow = createSubWindow(area);


     }

     /* Make NULL the selected group for Area */
     area->selectionLink = NULL;

     /* initialize the subWindows */
     initializeSubWindow(area->treeWindow);
     initializeSubWindow(area->groupWindow);

     return area;
}

/******************************************************
  createMainWindowWidgets
******************************************************/

void createMainWindowWidgets(area)
     ALINK *area;
{
     char   *actTitle={"Alarm Configuration Tool"};
     char   *alhTitle={"Alarm Handler"};
     XmString    str;
     Widget dialog;
     if (area->toplevel) return;

     /* create toplevel shell */
     area->toplevel = XtAppCreateShell(programName, programName,
           applicationShellWidgetClass, display, NULL, 0);

     if (area->programId == ACT){
          XtVaSetValues(area->toplevel, XmNtitle, actTitle, NULL);
     } else {
          XtVaSetValues(area->toplevel, XmNtitle, alhTitle, NULL);
     }

     /* Create form_main for toplevel */
     area->form_main = XtVaCreateManagedWidget("form_main",
          xmFormWidgetClass,         area->toplevel,
          XmNuserData,               (XtPointer)area,
          NULL);

     pixelData(area->form_main,NULL);


     if (!ALH_pixmap) axMakePixmap(area->form_main);

     XtVaSetValues(area->toplevel, XmNiconPixmap, ALH_pixmap, NULL);


     /* Modify the window manager menu "close" callback */
     {
        Atom         WM_DELETE_WINDOW;
        XtVaSetValues(area->toplevel,
             XmNdeleteResponse,       XmDO_NOTHING,
             NULL);
        WM_DELETE_WINDOW = XmInternAtom(XtDisplay(area->form_main),
             "WM_DELETE_WINDOW", False);
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

     /* Create group alarm decoder label for the messageArea */
     area->label_mask = XtVaCreateManagedWidget(
         "Mask <CDATL>: <Cancel,Disable,noAck,noackT,noLog>",
          xmLabelGadgetClass,        area->messageArea,
          XmNmarginHeight,           3,
          XmNalignment,              XmALIGNMENT_BEGINNING,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              area->scale,
          XmNleftAttachment,         XmATTACH_POSITION,
          XmNleftPosition,           1,
          NULL);

     /* Create group alarm decoder label for the messageArea */
     area->label_groupAlarm = XtVaCreateManagedWidget(
         "Group Alarm Counts: (INVALID,MAJOR,MINOR,NOALARM)",
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
         "Channel Alarm Data: Current<Status,Severity>,Highest Unack<Status,Severity>",
          xmLabelGadgetClass,        area->messageArea,
          XmNmarginHeight,           3,
          XmNalignment,              XmALIGNMENT_BEGINNING,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              area->label_groupAlarm,
          XmNleftAttachment,         XmATTACH_POSITION,
          XmNleftPosition,           1,
          NULL);

     /* Create filenameTitle label for the messageArea */
     area->label_filenameTitle = XtVaCreateManagedWidget("Filename:",
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


     /* Create a Silence Forever Toggle Button in the messageArea */
     area->silenceForever = XtVaCreateManagedWidget("SilenceForever",
          xmToggleButtonGadgetClass, area->messageArea,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              area->scale,
          XmNrightAttachment,        XmATTACH_FORM,
          XmNuserData,               (XtPointer)area,
          NULL);

     /* Set global variable */
     toggle_button1 = area->silenceForever;


     /* Create a Silence Current Toggle Button in the messageArea */
     area->silenceCurrent = XtVaCreateManagedWidget("SilenceCurrent",
          xmToggleButtonGadgetClass, area->messageArea,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              area->silenceForever,
          XmNrightAttachment,        XmATTACH_FORM,
          XmNuserData,               (XtPointer)area,
          NULL);

     /* Create BeepSeverity string for the messageArea */
     str = XmStringCreateSimple(alarmSeverityString[psetup.beepSevr]);
     area->beepSeverity = XtVaCreateManagedWidget("beepSeverity",
          xmLabelGadgetClass,        area->messageArea,
          XmNlabelString,            str,
          XmNshadowThickness,        2,
          XmNalignment,              XmALIGNMENT_BEGINNING,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              area->silenceCurrent,
          XmNrightAttachment,        XmATTACH_FORM,
          NULL);
     XmStringFree(str);

     /* Create BeepSeverityLabel string for the messageArea */
     str = XmStringCreateSimple("Beep Severity:");
     area->beepSeverityLabel = XtVaCreateManagedWidget("beepSeverityLabel",
          xmLabelGadgetClass,        area->messageArea,
          XmNshadowThickness,        2,
          XmNlabelString,            str,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              area->silenceCurrent,
          XmNrightAttachment,        XmATTACH_WIDGET,
          XmNrightWidget,            area->beepSeverity,
          NULL);
     XmStringFree(str);

     /* add a test alarm button */
     if (DEBUG == -1) {
         Widget button = XtVaCreateManagedWidget("Generate Test Alarm",
          xmPushButtonWidgetClass, area->messageArea,
          XmNlabelString,
          XmStringCreateLtoR("Generate Test Alarm...",XmSTRING_DEFAULT_CHARSET),
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              area->beepSeverity,
          XmNrightAttachment,        XmATTACH_FORM,
          NULL);

         dialog = createGetTestAlarm_dialog(button,(XtPointer)area);
         XtAddCallback(button, XmNactivateCallback, (XtCallbackProc)show_dialog, dialog);

     }

     /* Set global variable */
     toggle_button = area->silenceCurrent;

     /* NOTE silenceForever callback needs silenceCurrent widget id */
     XtAddCallback(area->silenceForever, XmNvalueChangedCallback,
          (XtCallbackProc)silenceForever_callback, area->silenceCurrent);

     XtAddCallback(area->silenceCurrent, XmNvalueChangedCallback, (XtCallbackProc)beep_callback, 0);

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

/***************************************************
 isTreeWindow
****************************************************/

int isTreeWindow(area,subWindow)
     ALINK *area;
     void * subWindow;
{
     if (area->treeWindow == subWindow ) return(TRUE);
     else return(FALSE);
}
/******************************************************
  unmapArea_callback
******************************************************/

void unmapArea_callback(main,w,cbs)
     Widget main;
     Widget w;
     XmAnyCallbackStruct *cbs;
{
     ALINK *area;

     XtVaGetValues(w, XmNuserData, &area, NULL);

     XUnmapWindow(XtDisplay(main),XtWindow(main));

     area->mapped = FALSE;

}
/******************************************************
  scale_callback
******************************************************/

void scale_callback(widget,area,cbs)
     Widget widget;
     ALINK                 *area;
     XmScaleCallbackStruct *cbs;
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
  markSelectionArea
****************************************************/

void markSelectionArea(area,line)
     ALINK *area;
     struct anyLine  *line;
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

void axMakePixmap(w)
     Widget w;
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

void changeBeepSeverityText(area)
     ALINK *area;
{
     XmString    str;

     if (area->beepSeverity) {
          str = XmStringCreateSimple(alarmSeverityString[psetup.beepSevr]);
          XtVaSetValues(area->beepSeverity,
               XmNlabelString,            str,
               NULL);
          XmStringFree(str);
     }
}


/******************************************************
  axUpdateDialogs
******************************************************/

void axUpdateDialogs(area)
     ALINK *area;
{
     /* update beepSeverity string on main window */
     changeBeepSeverityText(area);

     /* update property sheet window if it is displayed */
     propUpdateDialog(area);

     /* update force mask window if it is displayed */
     forceMaskUpdateDialog(area);

     /* update forcePV window if it is displayed */
     forcePVUpdateDialog(area);

     /* update force mask window if it is displayed */
     maskUpdateDialog(area);

}

/***************************************************
  getSelectionLinkArea
****************************************************/

void *getSelectionLinkArea(area)
     ALINK *area;
{
     return area->selectionLink;
}

/***************************************************
  getSelectionLinkTypeArea
****************************************************/

int getSelectionLinkTypeArea(area)
     ALINK *area;
{
     return area->selectionType;
}


/******************************************************
  CreateActionButtons
******************************************************/

Widget createActionButtons(parent, actions, num_buttons)
     Widget parent;
     ActionAreaItem *actions;
     int num_buttons;
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

