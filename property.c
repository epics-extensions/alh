/*
 $Log$
 Revision 1.2  1994/06/22 21:17:51  jba
 Added cvs Log keyword

 */

static char *sccsId = "@(#)property.c	1.9\t2/18/94";

/* property.c */
/* 
 *      Author: Janet Anderson
 *      Date:   02-16-93
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
 * .01  07-26-93        jba     initial implementation
 * .02  02-19-94        jba     Max name field length set to PVNAME_SIZE
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleBG.h>
#include <Xm/PanedW.h>
#include <Xm/Protocols.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>

#include <axArea.h>
#include <alLib.h>
#include <ax.h>

extern Pixel bg_pixel[ALARM_NSEV];

typedef struct {
    char *label;
    void (*callback)();
    XtPointer data;
} ActionAreaItem;

struct propWindow {
    void *area;
    int linkType;
    GCLINK *link;
    Widget propDialog;
    Widget nameLabelW;
    Widget nameTextW;
    Widget alarmMaskStringLabelW;
    Widget alarmMaskToggleButtonW[ALARM_NMASK];
    Widget maskFrameW;
    Widget severityPVnameTextW;
    Widget forcePVnameTextW;
    Widget forcePVmaskStringLabelW;
    Widget forceMaskToggleButtonW[ALARM_NMASK];
    Widget forcePVvalueTextW;
    Widget forcePVresetValueTextW;
    Widget processTextW;
    Widget guidanceTextW;

};


/* prototypes for static routines */
#ifdef __STDC__

static void propApplyCallback(Widget widget,struct propWindow *propWindow,XmAnyCallbackStruct *cbs);
static void propCancelCallback(Widget widget,struct propWindow *propWindow,XmAnyCallbackStruct *cbs);
static void propDismissCallback(Widget widget,struct propWindow *propWindow,XmAnyCallbackStruct *cbs);
static Widget propCreateDialog(ALINK*area,GCLINK *link,int linkType);
static Widget createActionButtons(Widget parent, ActionAreaItem *actions,int num_properties);
static void propUpdateDialogData(struct propWindow *propWindow, GCLINK *link,int linkType);
static void propUpdateDialogWidgets(struct propWindow *propWindow);

static GLINK *propCopyGroup(GLINK *glink);
static CLINK *propCopyChannel(CLINK *clink);
static void propFreeGroup( GLINK *glink);
static void propFreeChannel( CLINK *clink);
static void propMaskChange( Widget widget, int index, XmAnyCallbackStruct *cbs);


#else

static void propApplyCallback();
static void propCancelCallback();
static void propDismissCallback();
static Widget propCreateDialog();
static Widget createActionButtons();
static void propUpdateDialogData();
static void propUpdateDialogWidgets();
static GLINK *propCopyGroup();
static CLINK *propCopyChannel();
static void propFreeGroup();
static void propFreeChannel();
static void propMaskChange();

#endif /*__STDC__*/



/******************************************************
  propUpdateDialog
******************************************************/

void propUpdateDialog(area, link, linkType)
     ALINK  *area;
     GCLINK   *link;
     int      linkType;
{
     struct propWindow *propWindow;

     propWindow = area->propWindow;

     if (!propWindow)  return;

     if (!propWindow->propDialog || !XtIsManaged(propWindow->propDialog)) return;

     propWindow->link = link;
     propWindow->linkType = linkType;

     propUpdateDialogWidgets(propWindow);

}


/******************************************************
  propShowDialog
******************************************************/

void propShowDialog(area, link, linkType)
     ALINK    *area;
     GCLINK   *link;
     int      linkType;
{
     struct propWindow *propWindow;

     /* create propWindow and Dialog Widgets if necessary */
     if (!area->propWindow)  propCreateDialog(area, link, linkType);

     propWindow = (struct propWindow *)area->propWindow;

     /* update propWindow link info */
     propWindow->link = link; 
     propWindow->linkType = linkType;

     /* update Dialog Widgets */
     propUpdateDialogWidgets(propWindow);

     /* show Dialog */
     if (!propWindow->propDialog) return;
     if (!XtIsManaged(propWindow->propDialog)) {
          XtManageChild(propWindow->propDialog);
     }
     XMapWindow(XtDisplay(propWindow->propDialog),
                  XtWindow(XtParent(propWindow->propDialog)));

}

/******************************************************
  propUpdateDialogData
******************************************************/

static void propUpdateDialogData(propWindow, link, linkType)
     struct propWindow *propWindow;
     GCLINK   *link;
     int      linkType;
{

     if (propWindow->linkType == GROUP){
          propFreeGroup((GLINK *)propWindow->link); 
     }
     else if (propWindow->linkType == CHANNEL){
          propFreeChannel((CLINK *)propWindow->link); 
     }

     if (linkType == GROUP){
          propWindow->link = (GCLINK *)propCopyGroup((GLINK *)link); 
          propWindow->linkType = GROUP;
     }
     else if (linkType == CHANNEL){
          propWindow->link = (GCLINK *)propCopyChannel((CLINK *)link); 
          propWindow->linkType = CHANNEL;
     }

}

/******************************************************
  propUpdateDialogWidgets
******************************************************/

static void propUpdateDialogWidgets(propWindow)
     struct propWindow *propWindow;
{
     struct gcData *pgcData;
     GCLINK *link;
     int linkType;
     XmString string;
     char buff[MAX_STRING_LENGTH];
     char *str;
     SNODE *pt;
     int i=0;
     struct guideLink *guideLink;
     MASK mask;

     link = propWindow->link;

     if (!link) {

          XmTextFieldSetString(propWindow->nameTextW, '\0');
          string = XmStringCreateSimple("-----");
          XtVaSetValues(propWindow->alarmMaskStringLabelW, XmNlabelString, string, NULL);
          XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[0],FALSE);
          XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[1],FALSE);
          XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[2],FALSE);
          XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[3],FALSE);
          XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[4],FALSE);
          XmTextFieldSetString(propWindow->severityPVnameTextW,'\0');
          XmTextFieldSetString(propWindow->forcePVnameTextW,'\0');
          string = XmStringCreateSimple("-----");
          XtVaSetValues(propWindow->forcePVmaskStringLabelW, XmNlabelString, string, NULL);
          XmTextFieldSetString(propWindow->forcePVvalueTextW,'\0');
          XmTextFieldSetString(propWindow->forcePVresetValueTextW,'\0');
          XmTextFieldSetString(propWindow->processTextW,'\0');
          XmTextSetString(propWindow->guidanceTextW, '\0');
         
          return;
     }

     pgcData = link->pgcData;
     linkType = propWindow->linkType;

     /* ---------------------------------
     Group/Channel Name 
     --------------------------------- */
     if (linkType == GROUP) string = XmStringCreateSimple("Group Name");
     else string = XmStringCreateSimple("Channel Name");
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
     Alarm Mask 
     --------------------------------- */
     if (linkType == GROUP) XtSetSensitive(propWindow->maskFrameW, FALSE);
     else XtSetSensitive(propWindow->maskFrameW, TRUE);

     if (linkType == GROUP) awGetMaskString(((struct groupData *)pgcData)->mask,buff);
     else  alGetMaskString(((struct chanData *)pgcData)->defaultMask,buff);
     string = XmStringCreateSimple(buff);
     XtVaSetValues(propWindow->alarmMaskStringLabelW, XmNlabelString, string, NULL);
     XmStringFree(string);
     alSetMask(buff,&mask);
     if (mask.Cancel == 1 )
          XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[0],TRUE);
     else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[0],FALSE);
     if (mask.Disable == 1 )
          XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[1],TRUE);
     else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[1],FALSE);
     if (mask.Ack == 1 )
          XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[2],TRUE);
     else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[2],FALSE);
     if (mask.AckT == 1 )
          XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[3],TRUE);
     else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[3],FALSE);
     if (mask.Log == 1 )
          XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[4],TRUE);
     else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[4],FALSE);

     /* ---------------------------------
     Severity Process Variable
     --------------------------------- */
     if(strcmp(pgcData->sevrPVName,"-") != 0)
          XmTextFieldSetString(propWindow->severityPVnameTextW,pgcData->sevrPVName);
     else XmTextFieldSetString(propWindow->severityPVnameTextW,'\0');

     /* ---------------------------------
     Force Process Variable
     --------------------------------- */
     if(strcmp(pgcData->forcePVName,"-") != 0)
          XmTextFieldSetString(propWindow->forcePVnameTextW,pgcData->forcePVName);
     else XmTextFieldSetString(propWindow->forcePVnameTextW,'\0');

     alGetMaskString(pgcData->forcePVMask,buff);
     string = XmStringCreateSimple(buff);
     XtVaSetValues(propWindow->forcePVmaskStringLabelW, XmNlabelString, string, NULL);
     XmStringFree(string);
     mask = pgcData->forcePVMask;
     if (mask.Cancel == 1 )
          XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[0],TRUE);
     else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[0],FALSE);
     if (mask.Disable == 1 )
          XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[1],TRUE);
     else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[1],FALSE);
     if (mask.Ack == 1 )
          XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[2],TRUE);
     else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[2],FALSE);
     if (mask.AckT == 1 )
          XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[3],TRUE);
     else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[3],FALSE);
     if (mask.Log == 1 )
          XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[4],TRUE);
     else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[4],FALSE);

     sprintf(buff,"%d",pgcData->forcePVValue);
     XmTextFieldSetString(propWindow->forcePVvalueTextW,buff);

     sprintf(buff,"%d",pgcData->resetPVValue);
     XmTextFieldSetString(propWindow->forcePVresetValueTextW,buff);

     /* ---------------------------------
     Related Process Command
     --------------------------------- */
     XmTextFieldSetString(propWindow->processTextW,pgcData->command);

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
     str = (char*)calloc(1,i+1); 
     pt = sllFirst(&(link->GuideList));
     i=0;
     while (pt) {
           guideLink = (struct guideLink *)pt;
           strcat(str,guideLink->list);
           pt = sllNext(pt);
           if (pt) strcat(str,"\n");
           i++;
     }

     XmTextSetString(propWindow->guidanceTextW, str);
     free(str);

}

/******************************************************
  propCreateDialog
******************************************************/

static Widget propCreateDialog(area, link, linkType)
     ALINK    *area;
     GCLINK   *link;
     int      linkType;
{
     struct propWindow *propWindow;

     Widget propDialogShell, propDialog, rc, forcePV_text_w, severityPVnameTextW,property_s;
     Widget rc3, rowcol, form, maskFrameW, alarmMaskStringLabelW;
     Widget nameLabelW, nameTextW;
     Widget scrolledW;
     Widget forcePVlabel, severityPVlabel;
     Widget alarmMaskToggleButtonW[ALARM_NMASK];
     Widget forceMaskToggleButtonW[ALARM_NMASK];
     Widget processLabel, processTextW;
     Widget forcePVvalueLabel,forcePVnameTextW, forcePVvalueTextW, forcePVresetValueTextW, forcePVreset;
     Widget forcePVmaskStringLabelW, frame2, rowcol2, frame3, rowcol3, guidanceLabel, guidanceTextW;
     Widget toggle,alarmMaskLabel,forceMaskLabel,forcePVnameLabel;
     int i;
     Pixel textBackground;
     static char text[100];
     XmString string;
     static ActionAreaItem prop_items[] = {
         { "Apply",   propApplyCallback,   NULL    },
         { "Cancel",  propCancelCallback,  NULL    },
         { "Dismiss", propDismissCallback, NULL    },
         { "Help",    helpCallback,    "Help Button" },
     };
     static String maskFields[] = {
         "Cancel Alarm", 
         "Disable Alarm",
         "NoAck Alarm",
         "NoAck Transient Alarm",
         "NoLog Alarm"
     };
     Atom         WM_DELETE_WINDOW;


     if (!area) return;

     propWindow = (struct propWindow *)area->propWindow;

     if (propWindow && propWindow->propDialog){
          if (XtIsManaged(propWindow->propDialog)) return;
          else XtManageChild(propWindow->propDialog);
     }

     textBackground = bg_pixel[3];

     propWindow = (struct propWindow *)calloc(1,sizeof(struct propWindow)); 
     area->propWindow = (void *)propWindow;
     propWindow->area = (void *)area;
/*
     propDialogShell = XtAppCreateShell("Alarm Handler Properties",programName,
         applicationShellWidgetClass, display, NULL, 0);
*/
     propDialogShell = XtVaCreatePopupShell("Alarm Handler Properties",
         transientShellWidgetClass, area->toplevel, NULL, 0);

     /* Modify the window manager menu "close" callback */
/*
     XtVaSetValues(propDialogShell,
          XmNdeleteResponse,       XmDO_NOTHING,
          NULL);

     WM_DELETE_WINDOW = XmInternAtom(XtDisplay(propDialogShell),
          "WM_DELETE_WINDOW", False);
     XmAddWMProtocolCallback(propDialogShell,WM_DELETE_WINDOW,
          (XtCallbackProc)propDismissCallback, (XtPointer)propWindow);
*/


     propDialog = XtVaCreateWidget("propDialog",
         xmPanedWindowWidgetClass, propDialogShell,
         XmNsashWidth,  1,
         XmNsashHeight, 1,
         XmNuserData,   area,
         NULL);

     rc = XtVaCreateWidget("control_area", xmRowColumnWidgetClass, propDialog, NULL);
     form = XtVaCreateWidget("control_area", xmFormWidgetClass, propDialog, NULL);

     /* ---------------------------------
     Group/Channel Name 
     --------------------------------- */
     nameLabelW = XtVaCreateManagedWidget("nameLabelW",
          xmLabelGadgetClass, form,
          XmNtopAttachment,   XmATTACH_FORM,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);

     nameTextW = XtVaCreateManagedWidget("nameTextW",
          xmTextFieldWidgetClass, form,
          XmNspacing,          0,
          XmNmarginHeight,     0,
          XmNcolumns,         30,
          XmNmaxLength,       PVNAME_SIZE,
          XmNbackground,      textBackground,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       nameLabelW,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);

     XtAddCallback(nameTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     /* ---------------------------------
     Alarm Mask 
     --------------------------------- */
     string = XmStringCreateSimple("Alarm Mask:  ");
     alarmMaskLabel = XtVaCreateManagedWidget("alarmMaskLabel",
          xmLabelGadgetClass, form,
          XmNlabelString,     string,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       nameTextW,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);
     XmStringFree(string);

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

     maskFrameW = XtVaCreateManagedWidget("maskFrameW",
          xmFrameWidgetClass, form,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       alarmMaskLabel,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);

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
               (XtCallbackProc)propMaskChange, (XtPointer)i);
     }

     XtManageChild(rowcol);

     /* ---------------------------------
     Severity Process Variable
     --------------------------------- */
     string = XmStringCreateSimple("Severity Process Variable Name");
     severityPVlabel = XtVaCreateManagedWidget("severityPVlabel",
          xmLabelGadgetClass, form,
          XmNlabelString,    string,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       maskFrameW,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);
     XmStringFree(string);
 
     severityPVnameTextW = XtVaCreateManagedWidget("severityPVnameTextW",
          xmTextFieldWidgetClass, form,
          XmNspacing,          0,
          XmNmarginHeight,     0,
          XmNcolumns,         30,
          XmNmaxLength,       PVNAME_SIZE,
          XmNbackground,      textBackground,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       severityPVlabel,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);
 
     XtAddCallback(severityPVnameTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     /* ---------------------------------
     Force Process Variable
     --------------------------------- */
     string = XmStringCreateSimple("Force Process Variable         ");
     forcePVlabel = XtVaCreateManagedWidget("forcePVlabel",
          xmLabelGadgetClass, form,
          XmNlabelString,     string,
          XmNtopAttachment,   XmATTACH_FORM,
          XmNleftAttachment,  XmATTACH_POSITION,
          XmNleftPosition,    50,
          NULL);
     XmStringFree(string);

     frame2 = XtVaCreateManagedWidget("frame2",
          xmFrameWidgetClass, form,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       forcePVlabel,
          XmNleftAttachment,  XmATTACH_POSITION,
          XmNleftPosition,    50,
          NULL);

     rowcol2 = XtVaCreateWidget("rowcol2",
          xmFormWidgetClass, frame2,
          XmNspacing,          0,
          XmNmarginHeight,     0,
          NULL);

     string = XmStringCreateSimple("Force Process Variable Name    ");
     forcePVnameLabel = XtVaCreateManagedWidget("forcePVnameLabel",
          xmLabelGadgetClass, rowcol2,
          XmNlabelString,            string,
          XmNtopAttachment,          XmATTACH_FORM,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     forcePVnameTextW = XtVaCreateManagedWidget("forcePVnameTextW",
          xmTextFieldWidgetClass, rowcol2,
          XmNspacing,                0,
          XmNmarginHeight,           0,
          XmNcolumns,                30,
          XmNmaxLength,              PVNAME_SIZE,
          XmNbackground,             textBackground,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              forcePVnameLabel,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);

     XtAddCallback(forcePVnameTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     string = XmStringCreateSimple("Force Mask:  ");
     forceMaskLabel = XtVaCreateManagedWidget("forceMaskLabel",
          xmLabelGadgetClass, rowcol2,
          XmNlabelString,            string,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              forcePVnameTextW,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     string = XmStringCreateSimple("-----");
     forcePVmaskStringLabelW = XtVaCreateManagedWidget("forcePVmaskStringLabelW",
          xmLabelGadgetClass, rowcol2,
          XmNlabelString,            string,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              forcePVnameTextW,
          XmNleftAttachment,         XmATTACH_WIDGET,
          XmNleftWidget,             forceMaskLabel,
          NULL);
     XmStringFree(string);

     frame3 = XtVaCreateManagedWidget("frame3",
          xmFrameWidgetClass, rowcol2,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              forcePVmaskStringLabelW,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);

     rowcol3 = XtVaCreateWidget("rowcol2",
         xmRowColumnWidgetClass, frame3,
         XmNspacing,          0,
         XmNmarginHeight,     0,
         NULL);

     for (i = 0; i < ALARM_NMASK; i++){
          forceMaskToggleButtonW[i] = XtVaCreateManagedWidget(maskFields[i],
             xmToggleButtonGadgetClass, rowcol3,
             XmNmarginHeight,     0,
             XmNuserData,         (XtPointer)forcePVmaskStringLabelW,
             NULL);
          XtAddCallback(forceMaskToggleButtonW[i], XmNvalueChangedCallback,
               (XtCallbackProc)propMaskChange, (XtPointer)i);
     }

     XtManageChild(rowcol3);

     string = XmStringCreateSimple("Force Value: ");
     forcePVvalueLabel = XtVaCreateManagedWidget("forcePVvalue",
          xmLabelGadgetClass, rowcol2,
          XmNlabelString,            string,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              frame3,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     forcePVvalueTextW = XtVaCreateManagedWidget("forcePVvalueTextW",
          xmTextFieldWidgetClass, rowcol2,
          XmNspacing,                0,
          XmNmarginHeight,           0,
          XmNcolumns,                5,
          XmNmaxLength,              5,
          XmNbackground,             textBackground,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              frame3,
          XmNleftAttachment,         XmATTACH_WIDGET,
          XmNleftWidget,             forcePVvalueLabel,
          NULL);

     XtAddCallback(forcePVvalueTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     string = XmStringCreateSimple("Reset Value: ");
     forcePVreset = XtVaCreateManagedWidget("forcePVreset",
          xmLabelGadgetClass, rowcol2,
          XmNlabelString,            string,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              forcePVvalueLabel,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     forcePVresetValueTextW = XtVaCreateManagedWidget("forcePVresetValueTextW",
          xmTextFieldWidgetClass, rowcol2,
          XmNspacing,                0,
          XmNmarginHeight,           0,
          XmNcolumns,                5,
          XmNmaxLength,              5,
          XmNbackground,             textBackground,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              forcePVvalueTextW,
          XmNleftAttachment,         XmATTACH_WIDGET,
          XmNleftWidget,             forcePVreset,
          NULL);

     XtAddCallback(forcePVresetValueTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     /* RowColumn is full -- now manage */
     XtManageChild(rowcol2);

     /* ---------------------------------
     Related Process Command
     --------------------------------- */
     processTextW = XtVaCreateManagedWidget("processTextW",
          xmTextFieldWidgetClass, form,
          XmNspacing,                0,
          XmNmarginHeight,           0,
          XmNbackground,             textBackground,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              frame3,
          XmNleftAttachment,         XmATTACH_FORM,
          XmNrightAttachment,        XmATTACH_FORM,
          NULL);

     XtAddCallback(processTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     string = XmStringCreateSimple("Related Process Command");
     processLabel = XtVaCreateManagedWidget("processLabel",
          xmLabelGadgetClass, form,
          XmNlabelString,     string,
          XmNcolumns,                80,
          XmNbottomAttachment,       XmATTACH_WIDGET,
          XmNbottomWidget,           processTextW,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     /* ---------------------------------
     Guidance Text
     --------------------------------- */
     string = XmStringCreateSimple("Guidance                   ");
     guidanceLabel = XtVaCreateManagedWidget("guidanceLabel",
          xmLabelGadgetClass, form,
          XmNlabelString,     string,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       processTextW,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);
     XmStringFree(string);
 
     /* Create Scrolled Window  */
     scrolledW = XtVaCreateManagedWidget("scrolledW",
          xmScrolledWindowWidgetClass, form,
          XmNscrollingPolicy,        XmAUTOMATIC,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              guidanceLabel,
          XmNleftAttachment,         XmATTACH_FORM,
          XmNrightAttachment,        XmATTACH_FORM,
          XmNbottomAttachment,       XmATTACH_FORM,
          NULL);

     guidanceTextW = XtVaCreateManagedWidget("guidanceTextW",
          xmTextWidgetClass, scrolledW,
          XmNeditMode,               XmMULTI_LINE_EDIT,
          XmNbackground,             textBackground,
          NULL);

     XtManageChild(form);

     /* Set the client data "Apply", "Cancel" and "Dismiss" button's callbacks. */
     prop_items[0].data = (XtPointer)propWindow;
     prop_items[1].data = (XtPointer)propWindow;
     prop_items[2].data = (XtPointer)propWindow;

     property_s = createActionButtons(propDialog, prop_items, XtNumber(prop_items));

     XtManageChild(propDialog);

     propWindow->propDialog = propDialog;
     propWindow->nameLabelW = nameLabelW;
     propWindow->nameTextW = nameTextW;
     propWindow->alarmMaskStringLabelW = alarmMaskStringLabelW;
     propWindow->maskFrameW = maskFrameW;
     propWindow->severityPVnameTextW = severityPVnameTextW;
     propWindow->forcePVnameTextW = forcePVnameTextW;
     propWindow->forcePVmaskStringLabelW = forcePVmaskStringLabelW;
     propWindow->forcePVvalueTextW = forcePVvalueTextW;
     propWindow->forcePVresetValueTextW = forcePVresetValueTextW;
     propWindow->processTextW = processTextW;
     propWindow->guidanceTextW = guidanceTextW;
     for (i = 0; i < ALARM_NMASK; i++){
          propWindow->alarmMaskToggleButtonW[i] = alarmMaskToggleButtonW[i];
          propWindow->forceMaskToggleButtonW[i] = forceMaskToggleButtonW[i];
     }

     XtRealizeWidget(propDialogShell);

}

/******************************************************
  propMaskChange
******************************************************/

static void propMaskChange(widget, index, cbs)
     Widget widget;
     int index;
     XmAnyCallbackStruct *cbs;
{
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
}

/******************************************************
  propApplyCallback
******************************************************/

static void propApplyCallback(widget, propWindow, cbs)
     Widget widget;
     struct propWindow *propWindow;
     XmAnyCallbackStruct *cbs;
{
     short f1;
     int rtn;
     struct anyLine *line;
     struct chanData *cdata;
     XmString string;
     char *buff;
     char str[ALARM_NMASK];
     struct gcData *pgcData;
     GCLINK *link;
     int linkType;
     GCLINK *holdLink;
     int holdLinkType;
     MASK mask;
     struct guideLink *guideLink;
     SNODE *pt;
     int i;

     link = propWindow->link;
     if (!link) return;
     linkType = propWindow->linkType;
     pgcData = link->pgcData;

     /* copy  the link -  for undo purposes */
     if (linkType == GROUP){
          holdLink = (GCLINK *)propCopyGroup((GLINK *)link);
          holdLinkType = GROUP;
     }
     else if (linkType == CHANNEL){
          holdLink = (GCLINK *)propCopyChannel((CLINK *)link);
          holdLinkType = CHANNEL;
     }

     /* ---------------------------------
     Group/Channel Name 
     --------------------------------- */
     free(pgcData->name);
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
          alChangeChanMask((CLINK *)link,mask);
          cdata->defaultMask = cdata->curMask;
     }

     /* ---------------------------------
     Severity Process Variable
     --------------------------------- */
     free(pgcData->sevrPVName);
     buff = XmTextFieldGetString(propWindow->severityPVnameTextW);
     if (strlen(buff)) pgcData->sevrPVName = buff;
     else pgcData->sevrPVName = "-";

     /* ---------------------------------
     Force Process Variable
     --------------------------------- */
     /*  update link field  - forcePVName */
     free(pgcData->forcePVName);
     buff = XmTextFieldGetString(propWindow->forcePVnameTextW);
     if (strlen(buff)) pgcData->forcePVName = buff;
     else pgcData->forcePVName = "-";

     /*  update link field  - forcePVMask */
     XtVaGetValues(propWindow->forcePVmaskStringLabelW, XmNlabelString, &string, NULL);
     XmStringGetLtoR(string,XmFONTLIST_DEFAULT_TAG,&buff);
     XmStringFree(string);
     alSetMask(buff,&(pgcData->forcePVMask));

     /*  update link field  - forcePVValue */
     buff = XmTextFieldGetString(propWindow->forcePVvalueTextW);
     rtn = sscanf(buff,"%hd",&f1);
     if (rtn == 1) pgcData->forcePVValue = f1;
     else pgcData->forcePVValue = 1;

     /*  update link field  - resetPVValue */
     buff = XmTextFieldGetString(propWindow->forcePVresetValueTextW);
     rtn = sscanf(buff,"%hd",&f1);
     if (rtn == 1) pgcData->resetPVValue = f1;
     else pgcData->resetPVValue = 0;

     /* ---------------------------------
     Related Process Command
     --------------------------------- */
     if (pgcData->command) free(pgcData->command);
     buff = XmTextFieldGetString(propWindow->processTextW);
     if (strlen(buff)) pgcData->command = buff;
     else pgcData->command = 0;

     /* ---------------------------------
     Guidance Text
     --------------------------------- */
     pt = sllFirst(&(link->GuideList));
     while (pt) {
           guideLink = (struct guideLink *)pt;
           pt = sllNext(pt);
           sllRemove(&(link->GuideList),(SNODE *)guideLink);
           free(guideLink->list);
           free(guideLink);
     }

     buff = XmTextGetString(propWindow->guidanceTextW);
     if (strlen(buff)){
          pt = sllFirst(&(link->GuideList));
          i=0;
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
     Update properties dialog Window
     --------------------------------- */
     /*  update properties dialog window field */
     propUpdateDialog(propWindow->area,link,linkType);

     /*  update group line data and tree window */
     line = link->lineGroupW;
     if (line && line->link==link ){
          line = link->lineGroupW;
          line->pname = ((GCLINK *)link)->pgcData->name;
          markSelectedWidget(((ALINK *)propWindow->area)->groupWindow, 0);
          awRowWidgetsGroup(line);
          markSelectedWidget( ((ALINK *)propWindow->area)->groupWindow, ((WLINE *)line->wline)->name);
     }

     /*  update tree line data and group window */
     line = link->lineTreeW;
     if (line && line->link==link ){
          line->pname = ((GCLINK *)link)->pgcData->name;
          awRowWidgetsTree( (struct groupLine  *)line);
     }

     editUndoSet(holdLink,holdLinkType,link, MENU_EDIT_UNDO_PROPERTIES, TRUE);

}

/******************************************************
  propDismissCallback
******************************************************/

static void propDismissCallback(widget, propWindow, cbs)
     Widget widget;
     struct propWindow *propWindow;
     XmAnyCallbackStruct *cbs;
{
     Widget menuButton;
     Widget propDialog;

     propDialog = propWindow->propDialog;
/*
     XtVaGetValues(currentForm, XmNuserData, &area, NULL);

     XtVaSetValues(menuButton, XmNset, FALSE, NULL);
*/
     XtUnmanageChild(propDialog);
     XUnmapWindow(XtDisplay(propDialog), XtWindow(XtParent(propDialog)));
}

/******************************************************
  propCancelCallback
******************************************************/

static void propCancelCallback(widget, propWindow, cbs)
     Widget widget;
     struct propWindow *propWindow;
     XmAnyCallbackStruct *cbs;
{
     ALINK    *area;
     GCLINK   *link;
     int      linkType;

     area = propWindow->area; 
     link = propWindow->link; 
     linkType = propWindow->linkType;

     propUpdateDialog(area,link,linkType);
}


/******************************************************
  CreateActionButtons
******************************************************/

#define TIGHTNESS 20

static Widget createActionButtons(parent, actions, num_properties)
     Widget parent;
     ActionAreaItem *actions;
     int num_properties;
{
    Widget prop_sheet, widget;
    int i;

    prop_sheet = XtVaCreateWidget("prop_sheet", xmFormWidgetClass, parent,
        XmNfractionBase, TIGHTNESS*num_properties - 1,
        XmNleftOffset,   10,
        XmNrightOffset,  10,
        NULL);

    for (i = 0; i < num_properties; i++) {
        widget = XtVaCreateManagedWidget(actions[i].label,
            xmPushButtonWidgetClass, prop_sheet,
            XmNleftAttachment,       i? XmATTACH_POSITION : XmATTACH_FORM,
            XmNleftPosition,         TIGHTNESS*i,
            XmNtopAttachment,        XmATTACH_FORM,
            XmNbottomAttachment,     XmATTACH_FORM,
            XmNrightAttachment,
                    i != num_properties-1? XmATTACH_POSITION : XmATTACH_FORM,
            XmNrightPosition,        TIGHTNESS*i + (TIGHTNESS-1),
            XmNshowAsDefault,        i == 0,
            XmNdefaultButtonShadowThickness, 1,
            NULL);
        if (actions[i].callback)
            XtAddCallback(widget, XmNactivateCallback,
                actions[i].callback, actions[i].data);
        if (i == 0) {
            /* Set the prop_sheet's default button to the first widget
             * created (or, make the index a parameter to the function
             * or have it be part of the data structure). Also, set the
             * pane window constraint for max and min heights so this
             * particular pane in the PanedWindow is not resizable.
             */
            Dimension height, h;
            XtVaGetValues(prop_sheet, XmNmarginHeight, &h, NULL);
            XtVaGetValues(widget, XmNheight, &height, NULL);
            height += 2 * h;
            XtVaSetValues(prop_sheet,
                XmNdefaultButton, widget,
                XmNpaneMaximum,   height,
                XmNpaneMinimum,   height,
                NULL);
        }
    }

    XtManageChild(prop_sheet);

    return prop_sheet;
}
/******************************************************
  propCopyGroup
******************************************************/

static GLINK *propCopyGroup(glink)
     GLINK   *glink;
{
	CLINK *clink;
	GLINK *glinkNew;
	GLINK *glinkTemp;
	struct groupData *gdata;
	struct groupData *gdataNew;
	char *buff;
	struct guideLink *guideLink;
	SNODE *node;

	glinkNew = alAllocGroup();
    glinkNew->pgroupData = (struct groupData *)calloc(1,sizeof(struct groupData));
	gdataNew = glinkNew->pgroupData;
	gdata = glink->pgroupData;

	/* copy viewCount and pmainGroup */
	glinkNew->viewCount = glink->viewCount;
	glinkNew->pmainGroup = glink->pmainGroup;

	/* copy command */
	buff = gdata->command;
	if (buff){
		gdataNew->command = (char*)calloc(1,strlen(buff)+1);
		strcpy(gdataNew->command,buff);
	}

	/* copy sevrPV info */
	buff = gdata->sevrPVName;
	if(strcmp(buff,"-") != 0){
		gdataNew->sevrPVName = (char*)calloc(1,strlen(buff)+1);
		strcpy(gdataNew->sevrPVName,buff);
	} else gdataNew->sevrPVName = buff;
    gdataNew->PVValue = gdata->PVValue;;


	/* copy name */
	buff = gdata->name;
	if (buff){
		gdataNew->name = (char*)calloc(1,strlen(buff)+1);
		strcpy(gdataNew->name,buff);
	}

	/* copy forcePV info */
	buff = gdata->forcePVName;
	if(strcmp(buff,"-") != 0){
		gdataNew->forcePVName = (char*)calloc(1,strlen(buff)+1);
		strcpy(gdataNew->forcePVName,buff);
	} else gdataNew->forcePVName = buff;
	gdataNew->forcePVMask = gdata->forcePVMask;
	gdataNew->forcePVValue = gdata->forcePVValue;
	gdataNew->resetPVValue = gdata->resetPVValue;

	/* copy guidance */
	node = sllFirst(&(glink->GuideList));
	while (node) {
        buff = ((struct guideLink *)node)->list;
		guideLink = (struct guideLink *)calloc(1,sizeof(struct guideLink));
		guideLink->list = (char *)calloc(1,strlen(buff)+1);
		strcpy(guideLink->list,buff);
		sllAdd(&(glinkNew->GuideList),(SNODE *)guideLink);
		node = sllNext(node);
	}

	/* copy list pointers */
	glinkNew->chanList = glink->chanList;
	glinkNew->subGroupList = glink->subGroupList;

	return(glinkNew);

}

/******************************************************
  propCopyChannel
******************************************************/

static CLINK *propCopyChannel(clink)
     CLINK   *clink;
{

	return(alCopyChan(clink));

}

/******************************************************
  propFreeGroup
******************************************************/

static void propFreeGroup(glink)
     GLINK   *glink;
{
     SNODE *snode,*cnode,*gnode,*next;
     GLINK *pt;
     struct guideLink *guideLink; 

     if (glink->pgroupData->name) free(glink->pgroupData->name);
     if (strcmp(glink->pgroupData->forcePVName,"-") != 0) free(glink->pgroupData->forcePVName);
     if (strcmp(glink->pgroupData->sevrPVName,"-") != 0) free(glink->pgroupData->sevrPVName);
     if (glink->pgroupData->command) free(glink->pgroupData->command);

     snode = sllFirst(&glink->GuideList);
     while (snode) {
           next = sllNext(snode);
           guideLink = (struct guideLink *)snode;
           free(guideLink->list);
           free(guideLink);
           snode = next;
     }
     free(glink);
}

/******************************************************
  propFreeChannel
******************************************************/

static void propFreeChannel(clink)
     CLINK   *clink;
{
    SNODE *pt,*next;
    struct guideLink *guideLink; 

	if (clink != NULL) {
        if (clink->pchanData->name) free(clink->pchanData->name);
        if (strcmp(clink->pchanData->forcePVName,"-") != 0) free(clink->pchanData->forcePVName);
        if (strcmp(clink->pchanData->sevrPVName,"-") != 0) free(clink->pchanData->sevrPVName);
        if (clink->pchanData->command) free(clink->pchanData->command);

        pt = sllFirst(&clink->GuideList);
        while (pt) {
              next = sllNext(pt);
              guideLink = (struct guideLink *)pt;
              free(guideLink->list);
              free(guideLink);
              pt = next;
        }
		free(clink);
	}
}

/******************************************************
  propUndo
******************************************************/

void propUndo(area, link, linkType, newLink)
     void *area;
     GCLINK *link;
     int linkType;
     GCLINK *newLink;
{
     struct anyLine *line;
     struct chanData *cdata;
     struct chanData *cnewdata;
     struct gcData *pgcData;
     struct gcData *newData;
     GCLINK *holdLink;
     int holdLinkType;
     struct guideLink *guideLink;
     SNODE *pt;

     if (!link) return;

     pgcData = link->pgcData;
     newData = newLink->pgcData;

     /* copy  the link -  for undo purposes */
     if (linkType == GROUP){
          holdLink = (GCLINK *)propCopyGroup((GLINK *)link);
          holdLinkType = GROUP;
     }
     else if (linkType == CHANNEL){
          holdLink = (GCLINK *)propCopyChannel((CLINK *)link);
          holdLinkType = CHANNEL;
     }

     /* ---------------------------------
     Group/Channel Name 
     --------------------------------- */
     free(pgcData->name);
     pgcData->name = newData->name;

     /* ---------------------------------
     Alarm Mask 
     --------------------------------- */
     if (linkType == CHANNEL) {
          cdata = (struct chanData *)pgcData;
          cnewdata = (struct chanData *)newData;
          cdata->curMask = cnewdata->curMask;
          cdata->defaultMask = cnewdata->defaultMask;
     }

     /* ---------------------------------
     Severity Process Variable
     --------------------------------- */
     free(pgcData->sevrPVName);
     pgcData->sevrPVName = newData->sevrPVName;

     /* ---------------------------------
     Force Process Variable
     --------------------------------- */
     free(pgcData->forcePVName);
     pgcData->forcePVName = newData->forcePVName;
     pgcData->forcePVMask = newData->forcePVMask;
     pgcData->forcePVValue = newData->forcePVValue;
     pgcData->resetPVValue = newData->resetPVValue;

     /* ---------------------------------
     Related Process Command
     --------------------------------- */
     pgcData->command = newData->command;

     /* ---------------------------------
     Guidance Text
     --------------------------------- */
     pt = sllFirst(&(link->GuideList));
     while (pt) {
           guideLink = (struct guideLink *)pt;
           pt = sllNext(pt);
           sllRemove(&(link->GuideList),(SNODE *)guideLink);
           free(guideLink->list);
           free(guideLink);
     }

     link->GuideList = newLink->GuideList;

     /* ---------------------------------
     Update properties dialog Window
     --------------------------------- */
     propUpdateDialog(area,link,linkType);

     /* ---------------------------------
     Update tree line data and tree window 
     --------------------------------- */
     line = link->lineGroupW;
     if (line && line->link==link ){
          line->pname = ((GCLINK *)link)->pgcData->name;
          awRowWidgetsGroup(line);
     }

     /* ---------------------------------
     Update group line data and group window
     --------------------------------- */
     line = link->lineTreeW;
     if (line && line->link==link ){
          line->pname = ((GCLINK *)link)->pgcData->name;
          awRowWidgetsTree( (struct groupLine  *)line);
     }

     /* ---------------------------------
     set undo data
     --------------------------------- */
     editUndoSet(holdLink,holdLinkType, link, MENU_EDIT_UNDO_PROPERTIES, FALSE);

     /* ---------------------------------
     free newLink 
     --------------------------------- */
     free(newLink);
}
