/*
 $Log$
 Revision 1.6  1997/09/12 19:31:19  jba
 Change to get cut/paste clipboard working.

 Revision 1.5  1997/09/09 22:21:19  jba
 Changed Help menu and fixed Properties Window.

 Revision 1.4  1997/09/03 18:25:33  jba
 Removed calls to propShowDialog from menu insert channel/group.

 Revision 1.3  1995/10/20 16:50:09  jba
 Modified Action menus and Action windows
 Renamed ALARMCOMMAND to SEVRCOMMAND
 Added STATCOMMAND facility
 Added ALIAS facility
 Added ALARMCOUNTFILTER facility
 Make a few bug fixes.

 * Revision 1.2  1994/06/22  21:16:50  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)awAct.c	1.16\t12/15/93";

/* awAct.c */
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
 * .02  12-10-93        jba changes for new command line dir and file options 
 */

/*
******************************************************************
	routines defined in awAct.c
******************************************************************
         Routines for ACT specific menu, line, and callbacks
******************************************************************
-------------
|   PUBLIC  |
-------------
*

*************************************************************************
*/


#include <stdio.h>

#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>

#include "alh.h"
#include "ax.h"
#include "alLib.h"
#include "axArea.h"
#include "epicsVersion.h"

/* external variables */
extern ALINK *alhArea;

/* prototypes for static routines */
#ifdef __STDC__
static void actFileCallback( Widget widget, int item, XmAnyCallbackStruct *cbs);
static void actEditCallback( Widget widget, int item, XmAnyCallbackStruct *cbs);
static void actInsertCallback( Widget widget, int item, XmAnyCallbackStruct *cbs);
static void actViewCallback( Widget widget, int item, XmAnyCallbackStruct *cbs);
static void actHelpCallback( Widget widget, int item, XmAnyCallbackStruct *cbs);
static int checkActiveSelection(ALINK *area);
static int checkActiveSelectionMainGroup(ALINK *area);
#else
static void actFileCallback();
static void actEditCallback();
static void actInsertCallback();
static void actViewCallback();
static void actHelpCallback();
static int checkActiveSelection();
static int checkActiveSelectionMainGroup();
#endif /*__STDC__*/

#define KEEP 0
#define DELETE 1



/******************************************************
  actCreateMenu
******************************************************/

/* Create ACT MenuBar */
Widget actCreateMenu(parent, user_data)
     Widget     parent;
     XtPointer  user_data;
{
     Widget     widget;
     static MenuItem file_menu[] = {
/*
         { "New",        &xmPushButtonGadgetClass, 'N', "Ctrl<Key>N", "Ctrl+N",
             actFileCallback, (XtPointer)MENU_FILE_NEW,      (MenuItem *)NULL },
*/
         { "Open ...",   &xmPushButtonGadgetClass, 'O', "Ctrl<Key>O", "Ctrl+O",
             actFileCallback, (XtPointer)MENU_FILE_OPEN,       (MenuItem *)NULL },
/*
         { "Close",       &xmPushButtonGadgetClass, 'C', NULL, NULL,
             actFileCallback, (XtPointer)MENU_FILE_CLOSE,       (MenuItem *)NULL },
         { "Close All",   &xmPushButtonGadgetClass, 'A', NULL, NULL,
             actFileCallback, (XtPointer)MENU_FILE_CLOSEALL,     (MenuItem *)NULL },
*/
         { "Save",       &xmPushButtonGadgetClass, 'S', "Ctrl<Key>S", "Ctrl+S",
             actFileCallback, (XtPointer)MENU_FILE_SAVE,       (MenuItem *)NULL },
         { "Save As ...",&xmPushButtonGadgetClass, 'A', NULL, NULL,
             actFileCallback, (XtPointer)MENU_FILE_SAVEAS,     (MenuItem *)NULL },
         { "",           &xmSeparatorGadgetClass, '\0', NULL, NULL,
             NULL,        NULL,                            (MenuItem *)NULL },
         { "Activate ALH ...",  &xmPushButtonGadgetClass, 'A', NULL, NULL,
             actFileCallback, (XtPointer)MENU_FILE_ALH,      (MenuItem *)NULL },
         { "",           &xmSeparatorGadgetClass, '\0', NULL, NULL,
             NULL,        NULL,                            (MenuItem *)NULL },
         { "Tree Report ...",  &xmPushButtonGadgetClass, 'P', NULL, NULL,
             actFileCallback, (XtPointer)MENU_FILE_PRINT,      (MenuItem *)NULL },
         { "",           &xmSeparatorGadgetClass, '\0', NULL, NULL,
             NULL,        NULL,                            (MenuItem *)NULL },
         { "Exit",       &xmPushButtonGadgetClass, 'X', "Ctrl<Key>X", "Ctrl+X",
             actFileCallback, (XtPointer)MENU_FILE_QUIT,       (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem edit_menu[] = {

/*   ACCELERATORS DONT WORK YET
         { "Undo",       &xmPushButtonGadgetClass, 'U', "Alt<Key>BackSpace<Key>", "Alt+BackSpace",
             actEditCallback, (XtPointer)MENU_EDIT_UNDO,   (MenuItem *)NULL },
         { "",           &xmSeparatorGadgetClass, '\0', NULL, NULL,
             NULL,        NULL,                        (MenuItem *)NULL },
         { "Cut",        &xmPushButtonGadgetClass, 't', "Shift<Key>Delete<Key>", "Shift+Del",
             actEditCallback, (XtPointer)MENU_EDIT_CUT,    (MenuItem *)NULL },
         { "Copy",       &xmPushButtonGadgetClass, 'C', "Ctrl<Key>Insert<Key>", "Ctrl+Ins",
             actEditCallback, (XtPointer)MENU_EDIT_COPY,   (MenuItem *)NULL },
         { "Paste",      &xmPushButtonGadgetClass, 'P', "Shift<Key>Insert<Key>", "Shift+Ins",
             actEditCallback, (XtPointer)MENU_EDIT_PASTE,  (MenuItem *)NULL },
*/
/*   UNDO DOES NOT WORK YET
         { "Undo",       &xmPushButtonGadgetClass, 'U', NULL, NULL,
             actEditCallback, (XtPointer)MENU_EDIT_UNDO,   (MenuItem *)NULL },
         { "",           &xmSeparatorGadgetClass, '\0', NULL, NULL,
             NULL,        NULL,                        (MenuItem *)NULL },
*/
         { "Cut",        &xmPushButtonGadgetClass, 't', NULL, NULL,
             actEditCallback, (XtPointer)MENU_EDIT_CUT,    (MenuItem *)NULL },
         { "Copy",       &xmPushButtonGadgetClass, 'C', NULL, NULL,
             actEditCallback, (XtPointer)MENU_EDIT_COPY,   (MenuItem *)NULL },
         { "Paste",      &xmPushButtonGadgetClass, 'P', NULL, NULL,
             actEditCallback, (XtPointer)MENU_EDIT_PASTE,  (MenuItem *)NULL },
         { "",           &xmSeparatorGadgetClass, '\0', NULL, NULL,
             NULL,        NULL,                        (MenuItem *)NULL },
         { "Clear",      &xmPushButtonGadgetClass, 'e', NULL, NULL,
             actEditCallback, (XtPointer)MENU_EDIT_CLEAR,      (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem insert_menu[] = {
         { "Group ...",       &xmPushButtonGadgetClass, 'G', NULL, NULL,
             actInsertCallback, (XtPointer)MENU_INSERT_GROUP,   (MenuItem *)NULL },
         { "Channel ...",        &xmPushButtonGadgetClass, 'C', NULL, NULL, 
             actInsertCallback, (XtPointer)MENU_INSERT_CHANNEL,     (MenuItem *)NULL },
         { "Runtime Include...", &xmPushButtonGadgetClass, 'F', NULL, NULL,
             actInsertCallback, (XtPointer)MENU_INSERT_INCLUDE, (MenuItem *)NULL },
         { "File ...", &xmPushButtonGadgetClass, 'F', NULL, NULL,
             actInsertCallback, (XtPointer)MENU_INSERT_FILE, (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem view_menu[] = {
         { "Expand One Level", &xmPushButtonGadgetClass, 'O', "None<Key>plus", "+",
             actViewCallback, (XtPointer)MENU_VIEW_EXPANDCOLLAPSE1,          (MenuItem *)NULL },
         { "Expand Branch",          &xmPushButtonGadgetClass, 'B', "None<Key>asterisk", "*",
             actViewCallback, (XtPointer)MENU_VIEW_EXPANDBRANCH,      (MenuItem *)NULL },
         { "Expand All",             &xmPushButtonGadgetClass, 'A',"Ctrl<Key>asterisk", "Ctrl+*",
             actViewCallback, (XtPointer)MENU_VIEW_EXPANDALL,         (MenuItem *)NULL },
         { "Collapse Branch",        &xmPushButtonGadgetClass, 'C', "None<Key>minus", "-",
             actViewCallback, (XtPointer)MENU_VIEW_COLLAPSEBRANCH,    (MenuItem *)NULL },
         { "",                       &xmSeparatorGadgetClass, '\0', NULL, NULL,
             NULL,    NULL,                         (MenuItem *)NULL },
         { "Properties Window",  &xmToggleButtonGadgetClass, 'W', NULL, NULL,
             actViewCallback, (XtPointer)MENU_VIEW_PROPERTIES,          (MenuItem *)NULL },
         {NULL},
     };
     
     static MenuItem help_menu[] = {
/* HELP NOT IMPLEMENTED YET
         { "Help Topics",       &xmPushButtonGadgetClass, 'H', NULL, NULL,
             actHelpCallback, (XtPointer)MENU_HELP_TOPICS,      (MenuItem *)NULL },
*/
#if  XmVersion && XmVersion >= 1002
         { "About ALH",         &xmPushButtonGadgetClass, 'A', NULL, NULL,
             actHelpCallback, (XtPointer)MENU_HELP_ABOUT, (MenuItem *)NULL },
#endif
         {NULL},
     };
     
/*
     static MenuItem bar_menu[] = {
           { "File",   &xmCascadeButtonGadgetClass, 'F', NULL, NULL,
               0, 0, file_menu },
           { "Edit",   &xmCascadeButtonGadgetClass, 'E', NULL, NULL,
               0, 0, edit_menu },
           { "Insert", &xmCascadeButtonGadgetClass, 'I', NULL, NULL,
               0, 0, insert_menu },
           { "View",   &xmCascadeButtonGadgetClass, 'V', NULL, NULL,
               0, 0, view_menu },
           { "Help",   &xmCascadeButtonGadgetClass, 'H', NULL, NULL,
               0, 0, help_menu },
         {NULL},
     };
*/

     Widget menubar;

     menubar = XmCreateMenuBar(parent, "menubar", NULL, 0);

     XtVaSetValues(menubar, XmNshadowThickness, 0, NULL);

     widget = buildPulldownMenu(menubar, "File",  'F', TRUE, file_menu, user_data);
     widget = buildPulldownMenu(menubar, "Edit",  'E', TRUE, edit_menu, user_data);
     widget = buildPulldownMenu(menubar, "Insert",'I', TRUE, insert_menu, user_data);
     widget = buildPulldownMenu(menubar, "View",  'V', TRUE, view_menu, user_data);
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
  actFileCallback
******************************************************/

static void actFileCallback(widget, item, cbs)
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
{
     ALINK  *area;

     XtVaGetValues(widget, XmNuserData, &area, NULL);

     switch (item){

        case MENU_FILE_NEW:
             break;
 
        case MENU_FILE_OPEN:

             /* Display the config_changed warning dialog */
             if (area->changed){
                  createActionDialog(area->form_main,XmDIALOG_WARNING,
                       "Latest configuration changes have not been saved.  Continue?" ,
                       (XtCallbackProc)actFileCallback,(XtPointer)MENU_FILE_OPEN_OK,(XtPointer)area);
                  break;
             } 
             area->managed = FALSE;
             createFileDialog(area->form_main,(void *)fileSetupCallback,
                  (XtPointer)FILE_CONFIG,(void *)fileCancelCallback,
                  (XtPointer)area, (XtPointer)area, "Config File",CONFIG_PATTERN,psetup.configDir);
             break;

        case MENU_FILE_OPEN_OK:

             area->managed = FALSE;
             createFileDialog(area->form_main,(void *)fileSetupCallback,
                  (XtPointer)FILE_CONFIG,(void *)fileCancelCallback,
                  (XtPointer)area, (XtPointer)area, "Config File",CONFIG_PATTERN,psetup.configDir);
             break;

        case MENU_FILE_CLOSE:
             break;
 
        case MENU_FILE_CLOSEALL:
             break;
 
        case MENU_FILE_SAVEAS:

             /* New Name for Save Config File  */
             createFileDialog(area->form_main,(void *)fileSetupCallback,
                  (XtPointer)FILE_SAVEAS,(void *)XtUnmanageChild,(XtPointer)0,
                  (XtPointer)area, "Save Config File",CONFIG_PATTERN,'\0');
             break;
 
        case MENU_FILE_SAVE:

             alLogSetupSaveConfigFile(psetup.configFile);
             alWriteConfig(psetup.configFile,area->pmainGroup);
             break;

        case MENU_FILE_ALH:

             if (alhArea) alhArea->managed = FALSE;
             setupConfig("",ALH,area);

             /* Display the config_changed warning dialog */
/*
             if (area->changed){
                  createActionDialog(area->form_main,XmDIALOG_WARNING,
                       "Latest configuration changes have not been saved.  Continue?" ,
                       (XtCallbackProc)actInvokeAlhCallback,
                       (XtPointer)area,(XtPointer)area);
                  break;
             }
*/
             break;


        case MENU_FILE_PRINT:

             /* Name for PrintTree Report  */
             createFileDialog(area->form_main,(void *)fileSetupCallback,
                  (XtPointer)FILE_PRINT,(void *)XtUnmanageChild,(XtPointer)0,
                  (XtPointer)area, "Report File ",TREEREPORT_PATTERN,'\0');
             break;

        case MENU_FILE_QUIT:

             if (area->changed){
                  /* Display the config_changed warning dialog */
                  createActionDialog(area->form_main,XmDIALOG_WARNING,
                  "Latest configuration changes have not been saved!  Exit Alarm Configuration Tool?",
                        (XtCallbackProc)exit_quit,
                        (XtPointer)area, (XtPointer)area);
                  break;
             }
             createActionDialog(area->form_main,XmDIALOG_WARNING,"Exit Alarm Configuration Tool?",
                   (XtCallbackProc)exit_quit,
                   (XtPointer)area, (XtPointer)area);
             break;

        default:

             createDialog(area->form_main,XmDIALOG_INFORMATION,"Selection not implemented yet."," ");
             break;
     }
}

/******************************************************
  actEditCallback
******************************************************/

static void actEditCallback(widget, item, cbs)
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
{
     ALINK  *area;
     GCLINK *link;
     GCLINK *next;
     GCLINK *configLink;
     int linkType;
     int command;
     GCLINK *tempLink;
     int tempLinkType;
     struct anyLine  *treeLine;
     struct anyLine  *groupLine;

     XtVaGetValues(widget, XmNuserData, &area, NULL);

     switch (item){

        case MENU_EDIT_CLEAR:

             if (area->changed){
                  /* Display the config_changed warning dialog */
                  createActionDialog(area->form_main,XmDIALOG_WARNING,
                       "Latest configuration changes have not been saved.  Continue?" ,
                       (XtCallbackProc)actEditCallback,
                       (XtPointer)MENU_EDIT_CLEAR_OK,(XtPointer)area);
                  break;
             }

        case MENU_EDIT_CLEAR_OK:

             editUndoSet((GCLINK *)sllFirst(area->pmainGroup),
                         GROUP, NULL, MENU_EDIT_UNDO_CLEAR, DELETE);
     
             area->managed = FALSE;
             area->pmainGroup->p1stgroup = NULL;
             setupConfig("",ACT,area);
             break;

        case MENU_EDIT_CUT:

             if (checkActiveSelection(area) ) break;

             if (checkActiveSelectionMainGroup(area) ) break;

             link = (GCLINK *)area->selectionLink;
             linkType = area->selectionType;

             editClipboardSet(link, linkType);

             editUndoSet(link, linkType, NULL, MENU_EDIT_UNDO_PASTE, DELETE);
     
             editCutLink(area,link,linkType);

             break;

        case MENU_EDIT_COPY:
             
             if (checkActiveSelection(area) ) break;

             link = (GCLINK *)area->selectionLink;
             linkType = area->selectionType;

             editClipboardSet(link, linkType);

             break;

        case MENU_EDIT_PASTE:

             if (checkActiveSelection(area) ) break;

             editClipboardGet(&link, &linkType);

             if (link) editPasteLink(area,link,linkType);

             break;

        case MENU_EDIT_UNDO:

             command = editUndoGetCommand();

             switch (command){

                case MENU_EDIT_UNDO_UPDATE_CLIPBOARD:

                     editClipboardGet(&tempLink, &tempLinkType);

                     editUndoGet(&link, &linkType, &configLink);

                     editClipboardSet(link, linkType);

                     editUndoSet(tempLink, tempLinkType, NULL, MENU_EDIT_UNDO_UPDATE_CLIPBOARD, KEEP);

                     break;

                case MENU_EDIT_UNDO_PASTE:

                     editUndoGet(&link, &linkType, &tempLink);
                 
                     next = (GCLINK *)sllNext((SNODE*)link);
                     if (next){
                          ((struct subWindow *)area->groupWindow)->selectionLink=next;
                          area->selectionLink=next;
                          area->selectionType=linkType;
                          area->selectionWindow=area->groupWindow;
                     } else {
                          ((struct subWindow *)area->treeWindow)->selectionLink=link->parent;
                          area->selectionLink=link->parent;
                          area->selectionType=GROUP;
                          area->selectionWindow=area->treeWindow;
                     }

                     editPasteLink(area,link,linkType);

                     /* restore selection */
                     groupLine = (struct anyLine *)link->lineGroupW;
                     treeLine = (struct anyLine *)link->lineTreeW;
                     if (groupLine){
                          markSelectedWidget( area->groupWindow, ((WLINE *)groupLine->wline)->name);
                          markSelection( area->groupWindow, groupLine);
                     } else if (treeLine) {
                          markSelectedWidget( area->treeWindow, ((WLINE *)treeLine->wline)->name);
                          markSelection( area->treeWindow, treeLine);
                     }

                     /* update dialog windows */
                     axUpdateDialogs(area);

                     /* update undo values */
                     editUndoSet(NULL, linkType, link, MENU_EDIT_UNDO_CUT, KEEP);

                     break;

                case MENU_EDIT_UNDO_PASTE_NOSELECT:

                     editUndoGet(&link, &linkType, &tempLink);
                 
                     editPasteLink(area,link,linkType);

                     /* update dialog windows */
                     axUpdateDialogs(area);

                     /* update undo values */
                     editUndoSet(NULL, linkType, link, MENU_EDIT_UNDO_CUT_NOSELECT, KEEP);

                     break;

                case MENU_EDIT_UNDO_CUT:

                     if (checkActiveSelection(area) ) break;

                     if (checkActiveSelectionMainGroup(area) ) break;

                     link = (GCLINK *)area->selectionLink;
                     linkType = area->selectionType;

                     editClipboardSet(link, linkType);

                     editUndoSet(link, linkType, NULL, MENU_EDIT_UNDO_PASTE, KEEP);
                    
                     editCutLink(area,link,linkType);
        
                     break;

                case MENU_EDIT_UNDO_CUT_NOSELECT:

                     editUndoGet(&tempLink, &linkType, &link);

                     editCutLink(area, link, linkType);

                     editUndoSet(link, linkType, NULL, MENU_EDIT_UNDO_PASTE_NOSELECT, KEEP);

                     /* update dialog windows */
                     axUpdateDialogs(area);

                     break;

                case MENU_EDIT_UNDO_PROPERTIES:

                     propUndo(area);

                     break;

                case MENU_EDIT_UNDO_CLEAR:

                     link = (GCLINK *)sllFirst(area->pmainGroup);
                     editUndoGet(&tempLink, &linkType, &configLink);
                     area->pmainGroup->p1stgroup = (GLINK *)tempLink;

                     editUndoSet(link, GROUP, 0, MENU_EDIT_UNDO_CLEAR, KEEP);

                     setParentLink(area->treeWindow,area->pmainGroup->p1stgroup);
                     setParentLink(area->groupWindow,area->pmainGroup->p1stgroup);
                     ((struct subWindow *)area->treeWindow)->viewOffset = 0;
                     setViewConfigCount(area->treeWindow,area->pmainGroup->p1stgroup->viewCount);

                     /* redraw main windows */
                     redraw(area->treeWindow,0);
                     defaultTreeSelection(area);

                     /* update dialog windows */
                     axUpdateDialogs(area);

                     break;

                default:
                     createDialog(area->form_main,XmDIALOG_INFORMATION,
                          "Selection not implemented yet."," ");
                     break;
             }

             break;

        default:

             createDialog(area->form_main,XmDIALOG_INFORMATION,"Selection not implemented yet."," ");
             break;
     }
}

/******************************************************
  actInsertCallback
******************************************************/

static void actInsertCallback(widget, item, cbs)
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
{
     ALINK   *area;
     GCLINK   *link;
     int linkType;


     XtVaGetValues(widget, XmNuserData, &area, NULL);

     switch (item){

        case MENU_INSERT_GROUP:

             if (checkActiveSelection(area) ) break;

             /* create a new Group */
             link = (GCLINK *)alCreateGroup();
             link->viewCount = 1;
             linkType = GROUP;

             editPasteLink(area,link,linkType);

             break;

        case MENU_INSERT_CHANNEL:

             if (checkActiveSelection(area) ) break;

             /* create a new Channel */
             link = (GCLINK *)alCreateChannel();
             linkType = CHANNEL;

             editPasteLink(area, link, linkType);

             break;

        case MENU_INSERT_FILE:

             if (checkActiveSelection(area) ) break;

             area->managed = FALSE;
             createFileDialog(area->form_main,(void *)fileSetupCallback,
                  (XtPointer)FILE_CONFIG_INSERT,(void *)XtUnmanageChild,(XtPointer)0,
                  (XtPointer)area, "Config File",CONFIG_PATTERN,psetup.configDir);
             break;

        default:

             createDialog(area->form_main,XmDIALOG_INFORMATION,"Selection not implemented yet."," ");
             break;
     }

}


/******************************************************
  actViewCallback
******************************************************/
static void actViewCallback(widget, item, cbs)
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

        case MENU_VIEW_PROPERTIES:

             propShowDialog(area, widget);
             break;

        default:

             createDialog(area->form_main,XmDIALOG_INFORMATION,"Selection not implemented yet."," ");
             break;
     }
}

/******************************************************
  actHelpCallback
******************************************************/


static void actHelpCallback(widget, item, cbs)
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
{
     ALINK  *area;

     XtVaGetValues(widget, XmNuserData, &area, NULL);

     switch (item){

        case MENU_HELP_ABOUT:
             createDialog(area->form_main,XmDIALOG_INFORMATION,"\nAlarm Configuration Tool\n\nDeveloped at Argonne National Laboratory\n\nAuthors: Ben-Chin Cha, Janet Anderson, Mark Anderson, and Marty Kraimer\n\n", EPICS_VERSION_STRING);

             break;

        default:

             createDialog(area->form_main,XmDIALOG_INFORMATION,"Help is not available in this release."," ");
             break;
     }
}

/******************************************************
  checkActiveSelection
******************************************************/

static int checkActiveSelection(area)
     ALINK *area;
{

     if (!area->selectionLink){
          createDialog(area->form_main,XmDIALOG_WARNING,
          "Please select an alarm group or channel first."," ");
          return 1;
     }
     return 0;
}

/******************************************************
  checkActiveSelectionMainGroup
******************************************************/

static int checkActiveSelectionMainGroup(area)
     ALINK *area;
{
     GCLINK *link;

     link = (GCLINK *)area->selectionLink;
     if (!link->parent){
          createDialog(area->form_main,XmDIALOG_ERROR,link->pgcData->name,
          " is main group and cannot be deleted");
          return 1;
     }
     return 0;
}
