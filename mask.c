/* $Id$ */

/*******************************************************************
*	This file contains alh routines for modifying masks
******************************************************************/

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

#include "axArea.h"
#include "alLib.h"
#include "alh.h"
#include "ax.h"

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

/* forward declarations */
static void maskDismissCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void maskHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs);
static void maskActivateCallback( Widget widget,XtPointer calldata,XtPointer cbs);
static void maskCreateDialog(ALINK*area);
static void maskUpdateDialogWidgets(struct maskWindow *maskWindow);


/******************************************************
  maskUpdateDialog
******************************************************/
void maskUpdateDialog(ALINK *area)
{
     struct maskWindow *maskWindow;

     maskWindow = (struct maskWindow *)area->maskWindow;

     if (!maskWindow)  return;

     if (!maskWindow->maskDialog ||
         !XtIsManaged(maskWindow->maskDialog)) return;

     maskUpdateDialogWidgets(maskWindow);
}

/******************************************************
  maskShowDialog
******************************************************/
void maskShowDialog(ALINK *area,Widget menuButton)
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
static void maskUpdateDialogWidgets(struct maskWindow *maskWindow)
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
static void maskCreateDialog(ALINK *area)
{
     struct maskWindow *maskWindow;

     Widget maskDialogShell, maskDialog;
     Widget form;
     Widget nameLabelW, nameTextW;
     Widget labelW, pushButtonW, prev;
     int    i,j;
     static ActionAreaItem buttonItems[] = {
         { "Dismiss", maskDismissCallback, NULL    },
         { "Help",    maskHelpCallback,    NULL    },
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
               pushButtonW = XtVaCreateManagedWidget(
                    maskItem[i].choice[j].label,
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
               XtAddCallback(pushButtonW, XmNactivateCallback,
                    (XtCallbackProc)maskActivateCallback,
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
static void maskHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
     struct maskWindow *maskWindow=(struct maskWindow *)calldata;

     char *message1 = 
         "This dialog window allows an operator"
         " to change individual mask field\n"
         "values for a group or channel.\n"
         "Changing a mask field value for a group"
         " means changing the mask field\n"
         "value for all channels in the group.\n"
         "  \n"
         "Press the Dismiss button to close the"
         " Modify Mask Settings dialog window.\n"
         "Press the Help    button to get this help description window.\n"
            ;
     char * message2 = "  ";

     createDialog(widget,XmDIALOG_INFORMATION, message1,message2);

}


/******************************************************
  maskDismissCallback
******************************************************/
static void maskDismissCallback(Widget widget,XtPointer calldata,
    XtPointer cbs)
{
     struct maskWindow *maskWindow=(struct maskWindow *)calldata;
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
static void maskActivateCallback(Widget widget,XtPointer calldata,
    XtPointer cbs)
{
     int index=(int)calldata;
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
          silenceCurrentReset(area);
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

