/* awAct.c */

/************************DESCRIPTION***********************************
  Routines for alarm configuration tool (act) mode 
**********************************************************************/

static char *sccsId = "@@(#) $Id$";

#include <stdio.h>

#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>

#include "alh.h"
#include "ax.h"
#include "alLib.h"
#include "axArea.h"
#include "epicsVersion.h"
#include "version.h"

#define KEEP 0
#define ALH_DELETE 1

/* act menu definitions */
#define MENU_FILE_NEW		10100
#define MENU_FILE_OPEN		10101
#define MENU_FILE_OPEN_OK	10102
#define MENU_FILE_CLOSE		10103
#define MENU_FILE_CLOSEALL	10104
#define MENU_FILE_SAVE		10105
#define MENU_FILE_SAVEAS	10106
#define MENU_FILE_ALH		10107
#define MENU_FILE_PRINT		10108
#define MENU_FILE_QUIT		10109

#define MENU_VIEW_EXPANDCOLLAPSE1	10200
#define MENU_VIEW_EXPANDBRANCH		10201
#define MENU_VIEW_EXPANDALL			10202
#define MENU_VIEW_COLLAPSEBRANCH	10203
#define MENU_VIEW_PROPERTIES		10204

#define MENU_EDIT_UNDO			10600
#define MENU_EDIT_CUT			10601
#define MENU_EDIT_COPY			10602
#define MENU_EDIT_PASTE			10603
#define MENU_EDIT_CLEAR			10604
#define MENU_EDIT_CLEAR_OK		10605

#define MENU_INSERT_GROUP		10800
#define MENU_INSERT_CHANNEL		10801
#define MENU_INSERT_INCLUDE		10802
#define MENU_INSERT_FILE		10803
#define MENU_INSERT_SETTINGS	10804

#define MENU_HELP_HELP	10900
#define MENU_HELP_ABOUT	10906

/* external variables */
extern ALINK *alhArea;
extern char alhVersionString[60];

/* prototypes for static routines */
static void actFileCallback(Widget widget, XtPointer calldata, XtPointer cbs);
static void actEditCallback(Widget widget, XtPointer calldata, XtPointer cbs);
static void actInsertCallback(Widget widget, XtPointer calldata, XtPointer cbs);
static void actViewCallback(Widget widget, XtPointer calldata, XtPointer cbs);
static void actHelpCallback(Widget widget, XtPointer calldata, XtPointer cbs);
static int checkActiveSelection(ALINK *area);
static int checkActiveSelectionMainGroup(ALINK *area);


/******************************************************
 Create ACT MenuBar
******************************************************/
Widget actCreateMenu(Widget parent,XtPointer user_data)
{
	Widget     widget;
	static MenuItem file_menu[] = {
		/*
		         { "New",        PushButtonGadgetClass, 'N', "Ctrl<Key>N", "Ctrl+N",
		             actFileCallback, (XtPointer)MENU_FILE_NEW,      (MenuItem *)NULL },
		*/
		         { "Open ...",   PushButtonGadgetClass, 'O', "Ctrl<Key>O", "Ctrl+O",
		             actFileCallback, (XtPointer)MENU_FILE_OPEN,       (MenuItem *)NULL },
		/*
		         { "Close",       PushButtonGadgetClass, 'C', NULL, NULL,
		             actFileCallback, (XtPointer)MENU_FILE_CLOSE,       (MenuItem *)NULL },
		         { "Close All",   PushButtonGadgetClass, 'A', NULL, NULL,
		             actFileCallback, (XtPointer)MENU_FILE_CLOSEALL,     (MenuItem *)NULL },
		*/
		         { "Save",       PushButtonGadgetClass, 'S', "Ctrl<Key>S", "Ctrl+S",
		             actFileCallback, (XtPointer)MENU_FILE_SAVE,       (MenuItem *)NULL },
		         { "Save As ...",PushButtonGadgetClass, 'A', NULL, NULL,
		             actFileCallback, (XtPointer)MENU_FILE_SAVEAS,     (MenuItem *)NULL },
		         { "",           SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,        NULL,                            (MenuItem *)NULL },
		         { "Activate ALH ...",  PushButtonGadgetClass, 'A', NULL, NULL,
		             actFileCallback, (XtPointer)MENU_FILE_ALH,      (MenuItem *)NULL },
		         { "",           SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,        NULL,                            (MenuItem *)NULL },
		         { "Tree Report ...",  PushButtonGadgetClass, 'P', NULL, NULL,
		             actFileCallback, (XtPointer)MENU_FILE_PRINT,      (MenuItem *)NULL },
		         { "",           SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,        NULL,                            (MenuItem *)NULL },
		         { "Exit",       PushButtonGadgetClass, 'X', "Ctrl<Key>X", "Ctrl+X",
		             actFileCallback, (XtPointer)MENU_FILE_QUIT,       (MenuItem *)NULL },
		         {NULL},
		     	};

	static MenuItem edit_menu[] = {

		/*   ACCELERATORS DONT WORK YET
		         { "Undo",       PushButtonGadgetClass, 'U', "Alt<Key>BackSpace<Key>", "Alt+BackSpace",
		             actEditCallback, (XtPointer)MENU_EDIT_UNDO,   (MenuItem *)NULL },
		         { "",           SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,        NULL,                        (MenuItem *)NULL },
		         { "Cut",        PushButtonGadgetClass, 't', "Shift<Key>Delete<Key>", "Shift+Del",
		             actEditCallback, (XtPointer)MENU_EDIT_CUT,    (MenuItem *)NULL },
		         { "Copy",       PushButtonGadgetClass, 'C', "Ctrl<Key>Insert<Key>", "Ctrl+Ins",
		             actEditCallback, (XtPointer)MENU_EDIT_COPY,   (MenuItem *)NULL },
		         { "Paste",      PushButtonGadgetClass, 'P', "Shift<Key>Insert<Key>", "Shift+Ins",
		             actEditCallback, (XtPointer)MENU_EDIT_PASTE,  (MenuItem *)NULL },
		*/
		/*   UNDO DOES NOT WORK YET
		         { "Undo",       PushButtonGadgetClass, 'U', NULL, NULL,
		             actEditCallback, (XtPointer)MENU_EDIT_UNDO,   (MenuItem *)NULL },
		         { "",           SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,        NULL,                        (MenuItem *)NULL },
		*/
		         { "Cut",        PushButtonGadgetClass, 't', NULL, NULL,
		             actEditCallback, (XtPointer)MENU_EDIT_CUT,    (MenuItem *)NULL },
		         { "Copy",       PushButtonGadgetClass, 'C', NULL, NULL,
		             actEditCallback, (XtPointer)MENU_EDIT_COPY,   (MenuItem *)NULL },
		         { "Paste",      PushButtonGadgetClass, 'P', NULL, NULL,
		             actEditCallback, (XtPointer)MENU_EDIT_PASTE,  (MenuItem *)NULL },
		         { "",           SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,        NULL,                        (MenuItem *)NULL },
		         { "Clear",      PushButtonGadgetClass, 'e', NULL, NULL,
		             actEditCallback, (XtPointer)MENU_EDIT_CLEAR,      (MenuItem *)NULL },
		         {NULL},
		     	};

	static MenuItem insert_menu[] = {
		         { "Group ...",       PushButtonGadgetClass, 'G', NULL, NULL,
		             actInsertCallback, (XtPointer)MENU_INSERT_GROUP,   (MenuItem *)NULL },
		         { "Channel ...",        PushButtonGadgetClass, 'C', NULL, NULL, 
		             actInsertCallback, (XtPointer)MENU_INSERT_CHANNEL,     (MenuItem *)NULL },
		         { "Runtime Include...", PushButtonGadgetClass, 'F', NULL, NULL,
		             actInsertCallback, (XtPointer)MENU_INSERT_INCLUDE, (MenuItem *)NULL },
		         { "File ...", PushButtonGadgetClass, 'F', NULL, NULL,
		             actInsertCallback, (XtPointer)MENU_INSERT_FILE, (MenuItem *)NULL },
		         {NULL},
		     	};

	static MenuItem view_menu[] = {
		         { "Expand One Level", PushButtonGadgetClass, 'O', "None<Key>plus", "+",
		             actViewCallback, (XtPointer)MENU_VIEW_EXPANDCOLLAPSE1,          (MenuItem *)NULL },
		         { "Expand Branch",          PushButtonGadgetClass, 'B', "None<Key>asterisk", "*",
		             actViewCallback, (XtPointer)MENU_VIEW_EXPANDBRANCH,      (MenuItem *)NULL },
		         { "Expand All",             PushButtonGadgetClass, 'A',"Ctrl<Key>asterisk", "Ctrl+*",
		             actViewCallback, (XtPointer)MENU_VIEW_EXPANDALL,         (MenuItem *)NULL },
		         { "Collapse Branch",        PushButtonGadgetClass, 'C', "None<Key>minus", "-",
		             actViewCallback, (XtPointer)MENU_VIEW_COLLAPSEBRANCH,    (MenuItem *)NULL },
		         { "",                       SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,    NULL,                         (MenuItem *)NULL },
		         { "Properties Window",  ToggleButtonGadgetClass, 'W', NULL, NULL,
		             actViewCallback, (XtPointer)MENU_VIEW_PROPERTIES,          (MenuItem *)NULL },
		         {NULL},
		     	};

	static MenuItem help_menu[] = {
		/* HELP NOT IMPLEMENTED YET
		         { "Help Topics",       PushButtonGadgetClass, 'H', NULL, NULL,
		             actHelpCallback, (XtPointer)MENU_HELP_TOPICS,      (MenuItem *)NULL },
		*/
		#if  XmVersion && XmVersion >= 1002
		         { "About ALH",         PushButtonGadgetClass, 'A', NULL, NULL,
		             actHelpCallback, (XtPointer)MENU_HELP_ABOUT, (MenuItem *)NULL },
		#endif
		         {NULL},
		     	};

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
static void actFileCallback(Widget widget, XtPointer calldata, XtPointer cbs)
{
	ALINK  *area;
	int item=(int)calldata;

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
static void actEditCallback(Widget widget, XtPointer calldata, XtPointer cbs)
{
	int item=(int)calldata;
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
		    GROUP, NULL, MENU_EDIT_UNDO_CLEAR, ALH_DELETE);

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

		editUndoSet(link, linkType, NULL, MENU_EDIT_UNDO_PASTE, ALH_DELETE);

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
static void actInsertCallback(Widget widget, XtPointer calldata, XtPointer cbs)
{
	int item=(int)calldata;
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
		    (XtPointer)FILE_CONFIG_INSERT,(void *)XtUnmanageChild,
		    (XtPointer)0,(XtPointer)area, "Config File",CONFIG_PATTERN,
		    psetup.configDir);
		break;

	default:

		createDialog(area->form_main,XmDIALOG_INFORMATION,
		    "Selection not implemented yet."," ");
		break;
	}

}

/******************************************************
  actViewCallback
******************************************************/
static void actViewCallback(Widget widget, XtPointer calldata, XtPointer cbs)
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
		else createDialog(area->form_main,XmDIALOG_WARNING,
		    "Please select an alarm group first."," ");
		break;

	case MENU_VIEW_EXPANDBRANCH:

		/* Expand Branch */
		link = treeWindow->selectionLink;
		if (link )displayNewViewTree(area,link,EXPAND);
		else createDialog(area->form_main,XmDIALOG_WARNING,
		    "Please select an alarm group first."," ");
		break;

	case MENU_VIEW_EXPANDALL:

		/* Expand all */
		displayNewViewTree(area,(GLINK *)sllFirst(area->pmainGroup),EXPAND);
		break;

	case MENU_VIEW_COLLAPSEBRANCH:

		/* Collapse branch */
		link = treeWindow->selectionLink;
		if (link )displayNewViewTree(area,link,COLLAPSE);
		else createDialog(area->form_main,XmDIALOG_WARNING,
		    "Please select an alarm group first."," ");
		break;

	case MENU_VIEW_PROPERTIES:

		propShowDialog(area, widget);
		break;

	default:

		createDialog(area->form_main,XmDIALOG_INFORMATION,
		    "Selection not implemented yet."," ");
		break;
	}
}


/******************************************************
  actHelpCallback
******************************************************/
static void actHelpCallback(Widget widget, XtPointer calldata, XtPointer cbs)
{
	int item=(int)calldata;
	ALINK  *area;

	XtVaGetValues(widget, XmNuserData, &area, NULL);

	switch (item){

	case MENU_HELP_ABOUT:
		createDialog(area->form_main,XmDIALOG_INFORMATION,
		    "\nAlarm Configuration Tool\n\n" ALH_CREDITS_STRING,
		    alhVersionString);

		break;

	default:

		createDialog(area->form_main,XmDIALOG_INFORMATION,
		    "Help is not available in this release."," ");
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
