/*
 $Log$
 Revision 1.5  1996/11/19 19:40:27  jba
 Fixed motif delete window actions, and fixed size of force PV window.

 Revision 1.4  1995/10/20 16:50:48  jba
 Modified Action menus and Action windows
 Renamed ALARMCOMMAND to SEVRCOMMAND
 Added STATCOMMAND facility
 Added ALIAS facility
 Added ALARMCOUNTFILTER facility
 Make a few bug fixes.

 * Revision 1.3  1995/06/22  19:46:52  jba
 * Started cleanup of file.
 *
 * Revision 1.2  1994/06/22  21:17:44  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)mask.c	1.4\t9/9/93";

/* mask.c 
 *
 *      Author: Ben-chin Cha
 *      Date:   12-20-90
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01  02-16-93        jba     Reorganized files for new user interface
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

/*
******************************************************************
	routines defined in mask.c
*******************************************************************
*
*	This file contains alh routines for modifying masks
*
******************************************************************
*/

#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/PanedW.h>
#include <Xm/Protocols.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>

#include <axArea.h>
#include <alLib.h>
#include <alh.h>
#include <ax.h>

typedef struct {
    char *label;
    int index;
} maskChoiceItem;

typedef struct {
    char *label;
    maskChoiceItem  choice[3];
} maskItem;

struct maskWindow {
    void *area;
    Widget menuButton;
    Widget maskDialog;
    Widget nameLabelW;
    Widget nameTextW;
};

/* prototypes for static routines */
#ifdef __STDC__

static void maskDismissCallback(Widget widget,struct maskWindow *maskWindow,XmAnyCallbackStruct *cbs);
static void maskHelpCallback(Widget widget,struct maskWindow *maskWindow,XmAnyCallbackStruct *cbs);
static void maskActivateCallback( Widget widget, int index, XmAnyCallbackStruct *cbs);
static void maskCreateDialog(ALINK*area);
static void maskUpdateDialogWidgets(struct maskWindow *maskWindow);

#else

static void maskDismissCallback();
static void maskHelpCallback();
static void maskActivateCallback();
static void maskCreateDialog();
static void maskUpdateDialogWidgets();

#endif /*__STDC__*/


/******************************************************
  maskUpdateDialog
******************************************************/

void maskUpdateDialog(area)
     ALINK  *area;
{
     struct maskWindow *maskWindow;

     maskWindow = (struct maskWindow *)area->maskWindow;

     if (!maskWindow)  return;

     if (!maskWindow->maskDialog || !XtIsManaged(maskWindow->maskDialog)) return;

     maskUpdateDialogWidgets(maskWindow);

}


/******************************************************
  maskShowDialog
******************************************************/

void maskShowDialog(area, menuButton)
     ALINK    *area;
     Widget   menuButton;
{
     struct maskWindow *maskWindow;

     maskWindow = (struct maskWindow *)area->maskWindow;

     /* dismiss Dialog */
     if (maskWindow && maskWindow->maskDialog && 
                        XtIsManaged(maskWindow->maskDialog)) {
          maskDismissCallback(NULL, maskWindow, NULL);
          return;
     }

     /* create maskWindow and Dialog Widgets if necessary */
     if (!maskWindow)  maskCreateDialog(area);

     /* update maskWindow */
     maskWindow = (struct maskWindow *)area->maskWindow;
     maskWindow->menuButton = menuButton;

     /* update Dialog Widgets */
     maskUpdateDialogWidgets(maskWindow);

     /* show Dialog */
     if (!maskWindow->maskDialog) return;
     if (!XtIsManaged(maskWindow->maskDialog)) {
          XtManageChild(maskWindow->maskDialog);
     }
     XMapWindow(XtDisplay(maskWindow->maskDialog),
          XtWindow(XtParent(maskWindow->maskDialog)));
     if (menuButton) XtVaSetValues(menuButton, XmNset, TRUE, NULL);

}

/******************************************************
  maskUpdateDialogWidgets
******************************************************/

static void maskUpdateDialogWidgets(maskWindow)
     struct maskWindow *maskWindow;
{
     struct gcData *pgcData;
     GCLINK *link;
     int linkType;
     XmString string;

     if (! maskWindow || !maskWindow->maskDialog ||
           !XtIsManaged(maskWindow->maskDialog)) return;

     link =getSelectionLinkArea(maskWindow->area);

     if (!link) {
          string = XmStringCreateSimple("");
          if (maskWindow->nameTextW )
              XtVaSetValues(maskWindow->nameTextW,XmNlabelString, string, NULL);
          XmStringFree(string);
          return;
     }

     pgcData = link->pgcData;
     linkType =getSelectionLinkTypeArea(maskWindow->area);

     /* ---------------------------------
     Group/Channel Name 
     --------------------------------- */
     if (linkType == GROUP) string = XmStringCreateSimple("Group Name:");
     else string = XmStringCreateSimple("Channel Name:");
     XtVaSetValues(maskWindow->nameLabelW, XmNlabelString, string, NULL);
     XmStringFree(string);

     if (pgcData->alias){
          string = XmStringCreateSimple(pgcData->alias);
     } else {
          string = XmStringCreateSimple(pgcData->name);
     }
     XtVaSetValues(maskWindow->nameTextW, XmNlabelString, string, NULL);
     XmStringFree(string);

}

/******************************************************
  maskCreateDialog
******************************************************/

static void maskCreateDialog(area)
     ALINK    *area;
{
     struct maskWindow *maskWindow;

     Widget maskDialogShell, maskDialog;
     Widget form;
     Widget nameLabelW, nameTextW;
     Widget labelW, pushButtonW, prev;
     int    i,j;
     static ActionAreaItem buttonItems[] = {
         { "Dismiss", maskDismissCallback, NULL    },
         { "Help",    maskHelpCallback,    "Help Button" },
     };
     static maskItem maskItem[] = {
         { "Add/Cancel Alarms",         {"Add",    0,"Cancel",  1,"Reset", 2}},
         { "Enable/Disable Alarms",     {"Enable",10,"Disable",11,"Reset",12}},
         { "Ack/NoAck Alarms",          {"Ack",   20,"NoAck",  21,"Reset",22}},
         { "Ack/NoAck Transient Alarms",{"Ack",   30 ,"NoAck", 31,"Reset",32}},
         { "Log/NoLog Alarms",          {"Log",   40 ,"NoLog", 41,"Reset",42}},
     };
     int num_buttons = 3;
     int num_rows = 5;

     if (!area) return;

     maskWindow = (struct maskWindow *)area->maskWindow;

     if (maskWindow && maskWindow->maskDialog){
          if (XtIsManaged(maskWindow->maskDialog)) return;
          else XtManageChild(maskWindow->maskDialog);
     }

     maskWindow = (struct maskWindow *)calloc(1,sizeof(struct maskWindow)); 
     area->maskWindow = (void *)maskWindow;
     maskWindow->area = (void *)area;

     maskDialogShell = XtVaCreatePopupShell("Modify Mask Settings",
         transientShellWidgetClass, area->toplevel, 
         XmNallowShellResize, TRUE,
         NULL);

     /* Modify the window manager menu "close" callback */
     {
        Atom         WM_DELETE_WINDOW;
        XtVaSetValues(maskDialogShell,
             XmNdeleteResponse, XmDO_NOTHING, NULL);
        WM_DELETE_WINDOW = XmInternAtom(XtDisplay(maskDialogShell),
             "WM_DELETE_WINDOW", False);
        XmAddWMProtocolCallback(maskDialogShell,WM_DELETE_WINDOW,
           (XtCallbackProc)maskDismissCallback, (XtPointer)maskWindow);
     }

     maskDialog = XtVaCreateWidget("maskDialog",
         xmPanedWindowWidgetClass, maskDialogShell,
         XmNallowResize,      TRUE,
         XmNsashWidth,        1,
         XmNsashHeight,       1,
         XmNuserData,         area,
         NULL);

     form = XtVaCreateWidget("control_area",
          xmFormWidgetClass, maskDialog,
          XmNfractionBase,    TIGHTNESS*(num_buttons+3) - 1,
          XmNallowResize,     TRUE,
          NULL);

     /* ---------------------------------
     Group/Channel Name 
     --------------------------------- */
     nameLabelW = XtVaCreateManagedWidget("nameLabelW",
          xmLabelGadgetClass, form,
          XmNalignment,       XmALIGNMENT_END,
          XmNtopAttachment,   XmATTACH_FORM,
          XmNrightAttachment, XmATTACH_POSITION,
          XmNrightPosition,   (TIGHTNESS*(num_buttons+3) - 1)/2,
          XmNrecomputeSize,   True,
          NULL);

     nameTextW = XtVaCreateManagedWidget("nameTextW",
          xmLabelGadgetClass, form,
          XmNalignment,       XmALIGNMENT_BEGINNING,
          XmNleftAttachment,  XmATTACH_WIDGET,
          XmNleftWidget,      nameLabelW,
          XmNrightAttachment, XmATTACH_NONE,
          XmNrecomputeSize,   True,
          NULL);


     prev = nameLabelW;
     for (i = 0; i < num_rows; i++){
          labelW = XtVaCreateManagedWidget(maskItem[i].label,
               xmLabelGadgetClass,  form,
               XmNleftAttachment,   XmATTACH_FORM,
               XmNtopAttachment,    XmATTACH_WIDGET,
               XmNtopWidget,        prev,
               XmNtopOffset,        10,
               NULL);
          for (j = 0; j < num_buttons; j++){
               pushButtonW = XtVaCreateManagedWidget(maskItem[i].choice[j].label,
                    xmPushButtonWidgetClass, form,
                    XmNuserData,             (XtPointer)area,
                    XmNleftAttachment,       XmATTACH_POSITION,
                    XmNleftPosition,         TIGHTNESS*(j+3),
                    XmNtopAttachment,        XmATTACH_WIDGET,
                    XmNtopWidget,            prev,
                    XmNtopOffset,       5,
                    XmNrightAttachment,
                    j != num_buttons-1? XmATTACH_POSITION : XmATTACH_FORM,
                    XmNrightPosition,        TIGHTNESS*(j+3) + (TIGHTNESS-1),
                    NULL);
               XtAddCallback(pushButtonW, XmNactivateCallback,(XtCallbackProc)maskActivateCallback,
                    (XtPointer)maskItem[i].choice[j].index);
          }
          prev=labelW;
     }


     XtManageChild(form);

     /* Set the client data "Dismiss" and "Help" button's callbacks. */
     buttonItems[0].data = (XtPointer)maskWindow;
     buttonItems[1].data = (XtPointer)maskWindow;

     (void)createActionButtons(maskDialog, buttonItems, XtNumber(buttonItems));

     XtManageChild(maskDialog);

     maskWindow->maskDialog = maskDialog;
     maskWindow->nameLabelW = nameLabelW;
     maskWindow->nameTextW = nameTextW;

     XtRealizeWidget(maskDialogShell);

}


/******************************************************
  maskHelpCallback
******************************************************/

static void maskHelpCallback(widget, maskWindow, cbs)
     Widget widget;
     struct maskWindow *maskWindow;
     XmAnyCallbackStruct *cbs;
{

     char *message1 = 
         "This dialog window allows an operator to change individual mask field\n"
         "values for a group or channel.\n"
         "Changing a mask field value for a group means changing the mask field\n"
         "value for all channels in the group.\n"
         "  \n"
         "Press the Dismiss button to close the Modify Mask Settings dialog window.\n"
         "Press the Help    button to get this help description window.\n"
            ;
     char * message2 = "  ";

     createDialog(widget,XmDIALOG_INFORMATION, message1,message2);

}


/******************************************************
  maskDismissCallback
******************************************************/

static void maskDismissCallback(widget, maskWindow, cbs)
     Widget widget;
     struct maskWindow *maskWindow;
     XmAnyCallbackStruct *cbs;
{
     Widget maskDialog;

     maskDialog = maskWindow->maskDialog;
     XtUnmanageChild(maskDialog);
     XUnmapWindow(XtDisplay(maskDialog), XtWindow(XtParent(maskDialog)));
     if (maskWindow->menuButton)
          XtVaSetValues(maskWindow->menuButton, XmNset, FALSE, NULL);

}


/***************************************************
  maskActivateCallback
****************************************************/

static void maskActivateCallback(widget, index, cbs)
     Widget widget;
     int index;
     XmAnyCallbackStruct *cbs;
{
     ALINK *area;
     void *link;
     int maskid,maskno;
     int linkType;


     maskid = index/ 10;
     maskno = index % 10;

     XtVaGetValues(widget, XmNuserData, &area, NULL);
     link =getSelectionLinkArea(area);

     /* Change mask */
     if (link){
          linkType =getSelectionLinkTypeArea(area);
          if (linkType == GROUP){
               alForceGroupMask(link,maskid,maskno);
               alLogChangeGroupMasks(link,maskno,maskid);
          } else {
               alForceChanMask(link,maskid,maskno);
               alLogChanChangeMasks(link,maskno,maskid);
          }
          resetBeep();
     } else {
          createDialog(widget,XmDIALOG_WARNING,
               "Please select an alarm group or channel first."," ");
     }

     /* ---------------------------------
     Update all dialog Windows
     --------------------------------- */
/*
     axUpdateDialogs(area);
*/

}

