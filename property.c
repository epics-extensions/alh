/*
 $Log$
 Revision 1.9  1995/11/13 22:31:32  jba
 Added beepseverity command, ansi changes and other changes.

 * Revision 1.8  1995/10/20  16:50:52  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 *
 * Revision 1.7  1995/06/22  20:27:00  jba
 * Fixed alias overwriting Related process label.
 *
 * Revision 1.6  1995/06/22  19:48:57  jba
 * Added $ALIAS facility.
 *
 * Revision 1.5  1995/05/31  20:34:18  jba
 * Added name selection and arrow functions to Group window
 *
 * Revision 1.4  1995/05/30  15:55:06  jba
 * Added ALARMCOMMAND facility
 *
 * Revision 1.3  1995/02/28  16:43:50  jba
 * ansi c changes
 *
 * Revision 1.2  1994/06/22  21:17:51  jba
 * Added cvs Log keyword
 *
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

#include <stdlib.h>
#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/PanedW.h>
#include <Xm/Protocols.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleBG.h>

#include <axArea.h>
#include <alLib.h>
#include <ax.h>

extern Pixel bg_pixel[ALARM_NSEV];

GCLINK *undoLink;
GCLINK *holdLink;

struct propWindow {
    void *area;
    Widget menuButton;
    Widget propDialog;
    Widget nameLabelW;
    Widget nameTextW;
    Widget alarmMaskStringLabelW;
    Widget alarmMaskToggleButtonW[ALARM_NMASK];
    Widget resetMaskStringLabelW;
    Widget maskFrameW;
    Widget severityPVnameTextW;
    Widget countFilterFrame;
    Widget countFilterCountTextW;
    Widget countFilterSecondsTextW;
    Widget forcePVnameTextW;
    Widget forcePVmaskStringLabelW;
    Widget forceMaskToggleButtonW[ALARM_NMASK];
    Widget forcePVcurrentValueTextW;
    Widget forcePVforceValueTextW;
    Widget forcePVresetValueTextW;
    Widget aliasTextW;
    Widget processTextW;
    Widget statProcessTextW;
    Widget sevrProcessTextW;
    Widget guidanceTextW;

};


/* prototypes for static routines */
#ifdef __STDC__

static void propApplyCallback(Widget widget,struct propWindow *propWindow,XmAnyCallbackStruct *cbs);
static void propCancelCallback(Widget widget,struct propWindow *propWindow,XmAnyCallbackStruct *cbs);
static void propDismissCallback(Widget widget,struct propWindow *propWindow,XmAnyCallbackStruct *cbs);
static void propHelpCallback(Widget widget,struct propWindow *propWindow,XmAnyCallbackStruct *cbs);
static void propCreateDialog(ALINK*area);
static void propUpdateDialogWidgets(struct propWindow *propWindow);
static void propMaskChangeCallback( Widget widget, int index, XmAnyCallbackStruct *cbs);
static void propMoveData(GCLINK *fromLink,GCLINK *toLink,int linkType);
static void propEditableDialogWidgets(ALINK *area);

#else

static void propApplyCallback();
static void propCancelCallback();
static void propDismissCallback();
static void propHelpCallback();
static void propCreateDialog();
static void propUpdateDialogWidgets();
static void propMaskChangeCallback();
static void propMoveData();
static void propEditableDialogWidgets();

#endif /*__STDC__*/

/******************************************************
  propUpdateDialog
******************************************************/

void propUpdateDialog(area)
     ALINK  *area;
{
     struct propWindow *propWindow;

     propWindow = (struct propWindow *)area->propWindow;

     if (!propWindow)  return;

     if (!propWindow->propDialog || !XtIsManaged(propWindow->propDialog)) return;

     propUpdateDialogWidgets(propWindow);

}


/******************************************************
  propShowDialog
******************************************************/

void propShowDialog(area, menuButton)
     ALINK    *area;
     Widget   menuButton;
{
     struct propWindow *propWindow;

     propWindow = (struct propWindow *)area->propWindow;

     /* dismiss Dialog */
     if (propWindow && propWindow->propDialog && 
                        XtIsManaged(propWindow->propDialog)) {
          propDismissCallback(NULL, propWindow, NULL);
          return;
     }

     /* create propWindow and Dialog Widgets if necessary */
     if (!propWindow)  propCreateDialog(area);

     /* update propWindow link info */
     propWindow = (struct propWindow *)area->propWindow;
     propWindow->menuButton = menuButton;

     /* update Dialog Widgets */
     propUpdateDialogWidgets(propWindow);

     /* show Dialog */
     if (!propWindow->propDialog) return;
     if (!XtIsManaged(propWindow->propDialog)) {
          XtManageChild(propWindow->propDialog);
     }
     XMapWindow(XtDisplay(propWindow->propDialog),
          XtWindow(XtParent(propWindow->propDialog)));
     if (menuButton) XtVaSetValues(menuButton, XmNset, TRUE, NULL);

}

/******************************************************
  propUpdateDialogWidgets
******************************************************/

static void propUpdateDialogWidgets(propWindow)
     struct propWindow *propWindow;
{
     struct gcData *pgcData;
     struct chanData *pcData;
     GCLINK *link;
     int linkType;
     XmString string;
     char buff[MAX_STRING_LENGTH];
     char *str;
     SNODE *pt;
     int i=0;
     struct guideLink *guideLink;
     MASK mask;

     link =getSelectionLinkArea(propWindow->area);

     if (! propWindow || !propWindow->propDialog ||
           !XtIsManaged(propWindow->propDialog)) return;

     if (!link) {

          XmTextFieldSetString(propWindow->nameTextW, "");
          string = XmStringCreateSimple("-----");
          XtVaSetValues(propWindow->alarmMaskStringLabelW, XmNlabelString, string, NULL);
          XmStringFree(string);
          if (programId == ALH) {
               string = XmStringCreateSimple("-");
               XtVaSetValues(propWindow->resetMaskStringLabelW, XmNlabelString, string, NULL);
               XmStringFree(string);
          }
          if (programId != ALH) {
               XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[0],FALSE,TRUE);
               XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[1],FALSE,TRUE);
               XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[2],FALSE,TRUE);
               XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[3],FALSE,TRUE);
               XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[4],FALSE,TRUE);
          }
          XmTextFieldSetString(propWindow->severityPVnameTextW,"");
          XmTextFieldSetString(propWindow->countFilterCountTextW,"");
          XmTextFieldSetString(propWindow->countFilterSecondsTextW,"");
          XmTextFieldSetString(propWindow->forcePVnameTextW,"");
          string = XmStringCreateSimple("-----");
          XtVaSetValues(propWindow->forcePVmaskStringLabelW, XmNlabelString, string, NULL);
          XmStringFree(string);
          if (programId == ALH) {
               string = XmStringCreateSimple("");
               XtVaSetValues(propWindow->forcePVcurrentValueTextW, XmNlabelString, string, NULL);
               XmStringFree(string);
          }
          XmTextFieldSetString(propWindow->forcePVforceValueTextW,"");
          XmTextFieldSetString(propWindow->forcePVresetValueTextW,"");
          XmTextFieldSetString(propWindow->aliasTextW,"");
          XmTextFieldSetString(propWindow->processTextW,"");
          XmTextFieldSetString(propWindow->sevrProcessTextW,"");
          XmTextFieldSetString(propWindow->statProcessTextW,"");
          XmTextSetString(propWindow->guidanceTextW, "");
         
          return;
     }

     pgcData = link->pgcData;
     linkType =getSelectionLinkTypeArea(propWindow->area);
     if (linkType == CHANNEL) pcData = (struct chanData *)pgcData;

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
     Current Alarm Mask 
     --------------------------------- */
     if (linkType == GROUP) awGetMaskString(((struct groupData *)pgcData)->mask,buff);
     else  alGetMaskString(pcData->curMask,buff);
     string = XmStringCreateSimple(buff);
     XtVaSetValues(propWindow->alarmMaskStringLabelW, XmNlabelString, string, NULL);
     XmStringFree(string);

     if (programId != ALH) {
          if (linkType == GROUP) XtSetSensitive(propWindow->maskFrameW, FALSE);
          else XtSetSensitive(propWindow->maskFrameW, TRUE);

          alSetMask(buff,&mask);
          if (mask.Cancel == 1 )
               XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[0],TRUE,TRUE);
          else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[0],FALSE,TRUE);
          if (mask.Disable == 1 )
               XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[1],TRUE,TRUE);
          else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[1],FALSE,TRUE);
          if (mask.Ack == 1 )
               XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[2],TRUE,TRUE);
          else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[2],FALSE,TRUE);
          if (mask.AckT == 1 )
               XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[3],TRUE,TRUE);
          else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[3],FALSE,TRUE);
          if (mask.Log == 1 )
               XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[4],TRUE,TRUE);
          else XmToggleButtonSetState(propWindow->alarmMaskToggleButtonW[4],FALSE,TRUE);
     }

     /* ---------------------------------
     Reset Mask 
     --------------------------------- */
     if (programId == ALH ) {
          if ( linkType == CHANNEL) {
               alGetMaskString(pcData->defaultMask,buff);
               string = XmStringCreateSimple(buff);
          } else {
               string = XmStringCreateSimple("");
          }
          XtVaSetValues(propWindow->resetMaskStringLabelW, XmNlabelString, string, NULL);
          XmStringFree(string);
     }

     /* ---------------------------------
     Severity Process Variable
     --------------------------------- */
     if(strcmp(pgcData->sevrPVName,"-") != 0)
          XmTextFieldSetString(propWindow->severityPVnameTextW,pgcData->sevrPVName);
     else XmTextFieldSetString(propWindow->severityPVnameTextW,"");

     /* ---------------------------------
     Alarm Count Filter 
     --------------------------------- */
     if (linkType == GROUP) XtSetSensitive(propWindow->countFilterFrame, FALSE);
     else {
          XtSetSensitive(propWindow->countFilterFrame, TRUE);
          if(pcData->countFilter) {
               sprintf(buff,"%d",pcData->countFilter->inputCount);
               XmTextFieldSetString(propWindow->countFilterCountTextW,buff);
               sprintf(buff,"%d",pcData->countFilter->inputSeconds);
               XmTextFieldSetString(propWindow->countFilterSecondsTextW,buff);
          }
     }

     /* ---------------------------------
     Force Process Variable
     --------------------------------- */
     if(strcmp(pgcData->forcePVName,"-") != 0)
          XmTextFieldSetString(propWindow->forcePVnameTextW,pgcData->forcePVName);
     else XmTextFieldSetString(propWindow->forcePVnameTextW,"");

     alGetMaskString(pgcData->forcePVMask,buff);
     string = XmStringCreateSimple(buff);
     XtVaSetValues(propWindow->forcePVmaskStringLabelW, XmNlabelString, string, NULL);
     XmStringFree(string);
     if (programId != ALH) {
          mask = pgcData->forcePVMask;
          if (mask.Cancel == 1 )
               XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[0],TRUE,TRUE);
          else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[0],FALSE,TRUE);
          if (mask.Disable == 1 )
               XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[1],TRUE,TRUE);
          else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[1],FALSE,TRUE);
          if (mask.Ack == 1 )
               XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[2],TRUE,TRUE);
          else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[2],FALSE,TRUE);
          if (mask.AckT == 1 )
               XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[3],TRUE,TRUE);
          else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[3],FALSE,TRUE);
          if (mask.Log == 1 )
               XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[4],TRUE,TRUE);
          else XmToggleButtonSetState(propWindow->forceMaskToggleButtonW[4],FALSE,TRUE);
     }

/*
     if (programId == ALH) {
          sprintf(buff,"%d",pgcData->PVValue);
          string = XmStringCreateSimple(buff);
          XtVaSetValues(propWindow->forcePVcurrentValueTextW, XmNlabelString, string, NULL);
          XmStringFree(string);
     }
*/

     sprintf(buff,"%d",pgcData->forcePVValue);
     XmTextFieldSetString(propWindow->forcePVforceValueTextW,buff);

     sprintf(buff,"%d",pgcData->resetPVValue);
     XmTextFieldSetString(propWindow->forcePVresetValueTextW,buff);

     /* ---------------------------------
     Alias
     --------------------------------- */
     XmTextFieldSetString(propWindow->aliasTextW,pgcData->alias);

     /* ---------------------------------
     Related Process Command
     --------------------------------- */
     XmTextFieldSetString(propWindow->processTextW,pgcData->command);

     /* ---------------------------------
     Sevr Command
     --------------------------------- */
     getStringSevrCommandList(&pgcData->sevrCommandList,&str);
     XmTextSetString(propWindow->sevrProcessTextW,str);

     /* ---------------------------------
     Stat Command
     --------------------------------- */
     if (linkType == GROUP) XtSetSensitive(propWindow->statProcessTextW, FALSE);
     else {
          XtSetSensitive(propWindow->statProcessTextW, TRUE);
          getStringStatCommandList(&pcData->statCommandList,&str);
          XmTextSetString(propWindow->statProcessTextW,str);
     }


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
     str = (char*)calloc(i+1,sizeof(char)); 
     pt = sllFirst(&(link->GuideList));
     while (pt) {
           guideLink = (struct guideLink *)pt;
           strcat(str,guideLink->list);
           pt = sllNext(pt);
           if (pt) strcat(str,"\n");
     }

     XmTextSetString(propWindow->guidanceTextW, str);
     free(str);

}

/******************************************************
  propCreateDialog
******************************************************/

static void propCreateDialog(area)
     ALINK    *area;
{
     struct propWindow *propWindow;

     Widget propDialogShell, propDialog, severityPVnameTextW;
     Widget rowcol, form, maskFrameW;
     Widget nameLabelW, nameTextW;
     Widget guidanceScrolledW;
     Widget statProcessScrolledW;
     Widget sevrProcessScrolledW;
     Widget forcePVlabel, severityPVlabel;
     Widget alarmMaskToggleButtonW[ALARM_NMASK];
     Widget forceMaskToggleButtonW[ALARM_NMASK];
     Widget aliasLabel, aliasTextW;
     Widget processLabel, processTextW;
     Widget sevrProcessLabel, sevrProcessTextW;
     Widget statProcessLabel, statProcessTextW;
     Widget forcePVcurrentValueTextW=0;
     Widget forcePVforceValueLabel,forcePVnameTextW, forcePVforceValueTextW,
            forcePVresetValueTextW, forcePVresetValueLabel;
     Widget forcePVmaskStringLabelW, frame2, rowcol2, frame3,
            rowcol3, guidanceLabel, guidanceTextW;
     Widget alarmMaskLabel, alarmMaskStringLabelW;
     Widget forceMaskLabel, forcePVnameLabel;
     Widget resetMaskLabel=0, resetMaskStringLabelW=0;
     Widget prev;
     int i;
     Widget countFilterFrame, form4, countFilterLabel,
            countFilterCountLabel,countFilterCountTextW,
            countFilterSecondsLabel, countFilterSecondsTextW;
     Pixel textBackground;
     XmString string;
     static ActionAreaItem prop_items_act[] = {
         { "Apply",   propApplyCallback,   NULL    },
         { "Cancel",  propCancelCallback,  NULL    },
         { "Dismiss", propDismissCallback, NULL    },
         { "Help",    propHelpCallback,    "Help Button" },
     };
     static ActionAreaItem prop_items_alh[] = {
         { "Dismiss", propDismissCallback, NULL    },
         { "Help",    propHelpCallback,    "Help Button" },
     };
     static String maskFields[] = {
         "Cancel Alarm", 
         "Disable Alarm",
         "NoAck Alarm",
         "NoAck Transient Alarm",
         "NoLog Alarm"
     };

     if (!area) return;

     propWindow = (struct propWindow *)area->propWindow;

     if (propWindow && propWindow->propDialog){
          if (XtIsManaged(propWindow->propDialog)) return;
          else XtManageChild(propWindow->propDialog);
     }

     if (programId != ALH) textBackground = bg_pixel[3];
     else textBackground = bg_pixel[0];

     propWindow = (struct propWindow *)calloc(1,sizeof(struct propWindow)); 
     area->propWindow = (void *)propWindow;
     propWindow->area = (void *)area;

     propDialogShell = XtVaCreatePopupShell("Alarm Handler Properties",
         transientShellWidgetClass, area->toplevel, NULL, 0);

     propDialog = XtVaCreateWidget("propDialog",
         xmPanedWindowWidgetClass, propDialogShell,
         XmNsashWidth,  1,
         XmNsashHeight, 1,
         XmNuserData,   area,
         NULL);

     (void)XtVaCreateWidget("control_area", xmRowColumnWidgetClass, propDialog, NULL);
     form = XtVaCreateWidget("control_area", xmFormWidgetClass, propDialog, NULL);

     /* ---------------------------------
     Group/Channel Name 
     --------------------------------- */
     nameLabelW = XtVaCreateManagedWidget("nameLabelW",
          xmLabelGadgetClass, form,
          XmNrecomputeSize,   True,
          XmNtopAttachment,   XmATTACH_FORM,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);

     nameTextW = XtVaCreateManagedWidget("nameTextW",
          xmTextFieldWidgetClass, form,
          XmNspacing,          0,
          XmNmarginHeight,     0,
          XmNcolumns,         30,
          XmNrecomputeSize,   True,
          XmNmaxLength,       PVNAME_SIZE,
          XmNbackground,      textBackground,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       nameLabelW,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);

     XtAddCallback(nameTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     /* ---------------------------------
     Current Alarm Mask 
     --------------------------------- */
     if (programId != ALH) string = XmStringCreateSimple("Alarm Mask:  ");
     else string = XmStringCreateSimple("Current Mask:  ");
     alarmMaskLabel = XtVaCreateManagedWidget("alarmMaskLabel",
          xmLabelGadgetClass, form,
          XmNlabelString,     string,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       nameTextW,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);
     XmStringFree(string);
     prev = alarmMaskLabel;

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

     /* ---------------------------------
     Reset Mask 
     --------------------------------- */
     if (programId == ALH ) {
          string = XmStringCreateSimple("Reset Mask:  ");
          resetMaskLabel = XtVaCreateManagedWidget("resetMaskLabel",
               xmLabelGadgetClass, form,
               XmNlabelString,     string,
               XmNtopAttachment,   XmATTACH_WIDGET,
               XmNtopWidget,       alarmMaskLabel,
               XmNleftAttachment,  XmATTACH_FORM,
               NULL);
          XmStringFree(string);
          prev = resetMaskLabel;
     
          string = XmStringCreateSimple("     ");
          resetMaskStringLabelW = XtVaCreateManagedWidget("resetMaskStringLabelW",
               xmLabelGadgetClass, form,
               XmNlabelString,     string,
               XmNtopAttachment,   XmATTACH_WIDGET,
               XmNtopWidget,       alarmMaskLabel,
               XmNleftAttachment,  XmATTACH_WIDGET,
               XmNleftWidget,      resetMaskLabel,
               NULL);
          XmStringFree(string);
     }

     if (programId != ALH) {
          maskFrameW = XtVaCreateManagedWidget("maskFrameW",
               xmFrameWidgetClass, form,
               XmNtopAttachment,   XmATTACH_WIDGET,
               XmNtopWidget,       prev,
               XmNleftAttachment,  XmATTACH_FORM,
               NULL);
          prev=maskFrameW;
     
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
                    (XtCallbackProc)propMaskChangeCallback, (XtPointer)i);
          }
     
          XtManageChild(rowcol);
     }

     /* ---------------------------------
     Alarm Count Filter
     --------------------------------- */
     countFilterFrame = XtVaCreateManagedWidget("countFilterFrame",
          xmFrameWidgetClass, form,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       prev,
          XmNleftAttachment,  XmATTACH_FORM,
          XmNrightAttachment, XmATTACH_POSITION,
          XmNrightPosition,    50,
          NULL);

     form4 = XtVaCreateWidget("form4",
          xmFormWidgetClass, countFilterFrame,
          XmNspacing,          0,
          XmNmarginHeight,     0,
          NULL);

     string = XmStringCreateSimple("Alarm Count Filter             ");
     countFilterLabel = XtVaCreateManagedWidget("countFilterLabel",
          xmLabelGadgetClass, form4,
          XmNlabelString,            string,
          XmNtopAttachment,          XmATTACH_FORM,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     string = XmStringCreateSimple("Count: ");
     countFilterCountLabel = XtVaCreateManagedWidget("countFilterCountLabel",
          xmLabelGadgetClass, form4,
          XmNlabelString,            string,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              countFilterLabel,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     countFilterCountTextW = XtVaCreateManagedWidget("countFilterCountTextW",
          xmTextFieldWidgetClass, form4,
          XmNspacing,                0,
          XmNmarginHeight,           0,
          XmNcolumns,                3,
          XmNmaxLength,              3,
          XmNbackground,             textBackground,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              countFilterLabel,
          XmNleftAttachment,         XmATTACH_WIDGET,
          XmNleftWidget,             countFilterCountLabel,
          NULL);

     XtAddCallback(countFilterCountTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     string = XmStringCreateSimple("Seconds: ");
     countFilterSecondsLabel = XtVaCreateManagedWidget("countFilterSecondsLabel",
          xmLabelGadgetClass, form4,
          XmNlabelString,            string,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              countFilterLabel,
          XmNleftAttachment,         XmATTACH_WIDGET,
          XmNleftWidget,             countFilterCountTextW,
          NULL);
     XmStringFree(string);

     countFilterSecondsTextW = XtVaCreateManagedWidget("countFilterSecondsTextW",
          xmTextFieldWidgetClass, form4,
          XmNspacing,                0,
          XmNmarginHeight,           0,
          XmNcolumns,                3,
          XmNmaxLength,              3,
          XmNbackground,             textBackground,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              countFilterLabel,
          XmNleftAttachment,         XmATTACH_WIDGET,
          XmNleftWidget,             countFilterSecondsLabel,
          NULL);

     XtAddCallback(countFilterSecondsTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     /* Form is full -- now manage */
     XtManageChild(form4);

     /* ---------------------------------
     Severity Process Variable
     --------------------------------- */
     string = XmStringCreateSimple("Severity Process Variable Name");
     severityPVlabel = XtVaCreateManagedWidget("severityPVlabel",
          xmLabelGadgetClass, form,
          XmNlabelString,    string,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       countFilterFrame,
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
     prev = forceMaskLabel;

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

/*
     if (programId == ALH) {
          string = XmStringCreateSimple("Current Value:  ");
          forcePVcurrentValueLabel = XtVaCreateManagedWidget("forcePVcurrentValueLabel",
               xmLabelGadgetClass, rowcol2,
               XmNlabelString,            string,
               XmNtopAttachment,          XmATTACH_WIDGET,
               XmNtopWidget,              forceMaskLabel,
               XmNleftAttachment,         XmATTACH_FORM,
               NULL);
          XmStringFree(string);
          prev = forcePVcurrentValueLabel;
     
          string = XmStringCreateSimple("     ");
          forcePVcurrentValueTextW = XtVaCreateManagedWidget("forcePVcurrentValueTextW",
               xmLabelGadgetClass, rowcol2,
               XmNlabelString,            string,
               XmNtopAttachment,          XmATTACH_WIDGET,
               XmNtopWidget,              forceMaskLabel,
               XmNleftAttachment,         XmATTACH_WIDGET,
               XmNleftWidget,             forcePVcurrentValueLabel,
               NULL);
          XmStringFree(string);
     }
*/

     if (programId != ALH) {
          frame3 = XtVaCreateManagedWidget("frame3",
               xmFrameWidgetClass, rowcol2,
               XmNtopAttachment,          XmATTACH_WIDGET,
               XmNtopWidget,              forcePVmaskStringLabelW,
               XmNleftAttachment,         XmATTACH_FORM,
               NULL);
          prev = frame3;
     
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
                    (XtCallbackProc)propMaskChangeCallback, (XtPointer)i);
          }
     
          XtManageChild(rowcol3);
     }

     string = XmStringCreateSimple("Force Value: ");
     forcePVforceValueLabel = XtVaCreateManagedWidget("forcePVvalue",
          xmLabelGadgetClass, rowcol2,
          XmNlabelString,            string,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              prev,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     forcePVforceValueTextW = XtVaCreateManagedWidget("forcePVforceValueTextW",
          xmTextFieldWidgetClass, rowcol2,
          XmNspacing,                0,
          XmNmarginHeight,           0,
          XmNcolumns,                5,
          XmNmaxLength,              5,
          XmNbackground,             textBackground,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              prev,
          XmNleftAttachment,         XmATTACH_WIDGET,
          XmNleftWidget,             forcePVforceValueLabel,
          NULL);

     XtAddCallback(forcePVforceValueTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     string = XmStringCreateSimple("Reset Value: ");
     forcePVresetValueLabel = XtVaCreateManagedWidget("forcePVresetValueLabel",
          xmLabelGadgetClass, rowcol2,
          XmNlabelString,            string,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              forcePVforceValueLabel,
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
          XmNtopWidget,              forcePVforceValueTextW,
          XmNleftAttachment,         XmATTACH_WIDGET,
          XmNleftWidget,             forcePVresetValueLabel,
          NULL);

     XtAddCallback(forcePVresetValueTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     /* RowColumn is full -- now manage */
     XtManageChild(rowcol2);

     /* ---------------------------------
     Alias
     --------------------------------- */
     string = XmStringCreateSimple("Alias");
     aliasLabel = XtVaCreateManagedWidget("aliasLabel",
          xmLabelGadgetClass,        form,
          XmNlabelString,            string,
          XmNcolumns,                80,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              severityPVnameTextW,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     aliasTextW = XtVaCreateManagedWidget("aliasTextW",
          xmTextFieldWidgetClass, form,
          XmNspacing,                0,
          XmNmarginHeight,           0,
          XmNbackground,             textBackground,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              aliasLabel,
          XmNleftAttachment,         XmATTACH_FORM,
          XmNrightAttachment,        XmATTACH_FORM,
          NULL);

     XtAddCallback(aliasTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     /* ---------------------------------
     Related Process Command
     --------------------------------- */
     string = XmStringCreateSimple("Related Process Command");
     processLabel = XtVaCreateManagedWidget("processLabel",
          xmLabelGadgetClass,        form,
          XmNlabelString,            string,
          XmNcolumns,                80,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              aliasTextW,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     processTextW = XtVaCreateManagedWidget("processTextW",
          xmTextFieldWidgetClass, form,
          XmNspacing,                0,
          XmNmarginHeight,           0,
          XmNbackground,             textBackground,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              processLabel,
          XmNleftAttachment,         XmATTACH_FORM,
          XmNrightAttachment,        XmATTACH_FORM,
          NULL);

     XtAddCallback(processTextW, XmNactivateCallback,
          (XtCallbackProc)XmProcessTraversal, (XtPointer)XmTRAVERSE_NEXT_TAB_GROUP);

     /* ---------------------------------
     Sevr Command
     --------------------------------- */
     string = XmStringCreateSimple("Alarm Severity Commands");
     sevrProcessLabel = XtVaCreateManagedWidget("sevrProcessLabel",
          xmLabelGadgetClass, form,
          XmNlabelString,     string,
          XmNcolumns,                80,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              processTextW,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     /* Create Scrolled Window  */
     sevrProcessScrolledW = XtVaCreateManagedWidget("sevrScrolledW",
          xmScrolledWindowWidgetClass, form,
          XmNscrollingPolicy,        XmAUTOMATIC,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              sevrProcessLabel,
          XmNleftAttachment,         XmATTACH_FORM,
          XmNrightAttachment,        XmATTACH_FORM,
          NULL);

     sevrProcessTextW = XtVaCreateManagedWidget("sevrProcessTextW",
          xmTextWidgetClass, sevrProcessScrolledW,
          XmNeditMode,               XmMULTI_LINE_EDIT,
          XmNbackground,             textBackground,
          NULL);

     /* ---------------------------------
     Stat Command
     --------------------------------- */
     string = XmStringCreateSimple("Alarm Status Commands");
     statProcessLabel = XtVaCreateManagedWidget("statProcessLabel",
          xmLabelGadgetClass, form,
          XmNlabelString,     string,
          XmNcolumns,                80,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              sevrProcessScrolledW,
          XmNleftAttachment,         XmATTACH_FORM,
          NULL);
     XmStringFree(string);

     /* Create Scrolled Window  */
     statProcessScrolledW = XtVaCreateManagedWidget("statScrolledW",
          xmScrolledWindowWidgetClass, form,
          XmNscrollingPolicy,        XmAUTOMATIC,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              statProcessLabel,
          XmNleftAttachment,         XmATTACH_FORM,
          XmNrightAttachment,        XmATTACH_FORM,
          NULL);

     statProcessTextW = XtVaCreateManagedWidget("statProcessTextW",
          xmTextWidgetClass,statProcessScrolledW,
          XmNeditMode,               XmMULTI_LINE_EDIT,
          XmNbackground,             textBackground,
          NULL);

     /* ---------------------------------
     Guidance Text
     --------------------------------- */
     string = XmStringCreateSimple("Guidance                   ");
     guidanceLabel = XtVaCreateManagedWidget("guidanceLabel",
          xmLabelGadgetClass, form,
          XmNlabelString,     string,
          XmNtopAttachment,   XmATTACH_WIDGET,
          XmNtopWidget,       statProcessScrolledW,
          XmNleftAttachment,  XmATTACH_FORM,
          NULL);
     XmStringFree(string);
 
     /* Create Scrolled Window  */
     guidanceScrolledW = XtVaCreateManagedWidget("guidanceScrolledW",
          xmScrolledWindowWidgetClass, form,
          XmNscrollingPolicy,        XmAUTOMATIC,
          XmNtopAttachment,          XmATTACH_WIDGET,
          XmNtopWidget,              guidanceLabel,
          XmNleftAttachment,         XmATTACH_FORM,
          XmNrightAttachment,        XmATTACH_FORM,
          XmNbottomAttachment,       XmATTACH_FORM,
          NULL);
     guidanceTextW = XtVaCreateManagedWidget("guidanceTextW",
          xmTextWidgetClass, guidanceScrolledW,
          XmNeditMode,               XmMULTI_LINE_EDIT,
          XmNbackground,             textBackground,
          XmNresizeHeight,           TRUE,
          XmNresizeWidth,            TRUE,
          NULL);
/*
     n=0;
     XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
     XtSetArg(args[n], XmNbackground, textBackground); n++;
     XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
     XtSetArg(args[n], XmNtopWidget, guidanceLabel); n++;
     XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
     XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
     XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
     guidanceTextW = XmCreateScrolledText(form,"guidanceTextW",args,n);
     XtManageChild(guidanceTextW);

*/
     XtManageChild(form);

     if (programId != ALH) {
          /* Set the client data "Apply", "Cancel", "Dismiss" and "Help" button's callbacks. */
          prop_items_act[0].data = (XtPointer)propWindow;
          prop_items_act[1].data = (XtPointer)propWindow;
          prop_items_act[2].data = (XtPointer)propWindow;
          prop_items_act[3].data = (XtPointer)propWindow;
     
          (void)createActionButtons(propDialog, prop_items_act, XtNumber(prop_items_act));
     } else {
          prop_items_alh[0].data = (XtPointer)propWindow;
          prop_items_alh[1].data = (XtPointer)propWindow;

          (void)createActionButtons(propDialog, prop_items_alh, XtNumber(prop_items_alh));
     }

     XtManageChild(propDialog);

     propWindow->propDialog = propDialog;
     propWindow->nameLabelW = nameLabelW;
     propWindow->nameTextW = nameTextW;
     propWindow->alarmMaskStringLabelW = alarmMaskStringLabelW;
     propWindow->resetMaskStringLabelW = resetMaskStringLabelW;
     propWindow->maskFrameW = maskFrameW;
     propWindow->severityPVnameTextW = severityPVnameTextW;
     propWindow->countFilterFrame = countFilterFrame;
     propWindow->countFilterCountTextW = countFilterCountTextW;
     propWindow->countFilterSecondsTextW = countFilterSecondsTextW;
     propWindow->forcePVnameTextW = forcePVnameTextW;
     propWindow->forcePVmaskStringLabelW = forcePVmaskStringLabelW;
     propWindow->forcePVcurrentValueTextW = forcePVcurrentValueTextW;
     propWindow->forcePVforceValueTextW = forcePVforceValueTextW;
     propWindow->forcePVresetValueTextW = forcePVresetValueTextW;
     propWindow->aliasTextW = aliasTextW;
     propWindow->processTextW = processTextW;
     propWindow->sevrProcessTextW = sevrProcessTextW;
     propWindow->statProcessTextW = statProcessTextW;
     propWindow->guidanceTextW = guidanceTextW;
     if (programId != ALH) {
          for (i = 0; i < ALARM_NMASK; i++){
               propWindow->alarmMaskToggleButtonW[i] = alarmMaskToggleButtonW[i];
               propWindow->forceMaskToggleButtonW[i] = forceMaskToggleButtonW[i];
          }
     }

     /* update propWindow link info */
     propEditableDialogWidgets(area);

     XtRealizeWidget(propDialogShell);

}

/******************************************************
  propMaskChangeCallback
******************************************************/

static void propMaskChangeCallback(widget, index, cbs)
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
     short f1, f2;
     int rtn, rtn2;
     struct anyLine *line;
     struct chanData *cdata;
     XmString string;
     char *buff;
     struct gcData *pgcData;
     GCLINK *link;
     int linkType;
     MASK mask;
     struct guideLink *guideLink;
     SNODE *pt;

     link =getSelectionLinkArea(propWindow->area);
     if (!link) return;
     linkType =getSelectionLinkTypeArea(propWindow->area);
     pgcData = link->pgcData;

     /* create 2 links of type channel- for undo purposes */
     /* NOTE: a type channel link with its chanData
              can hold all the property data */
     if (!undoLink) {
         undoLink = (GCLINK *)alAllocChan();
         holdLink = (GCLINK *)alAllocChan();
     }

     propMoveData(link,undoLink,linkType);

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
          if (programId != ALH) cdata->defaultMask = cdata->curMask;
     }

     /* ---------------------------------
     Severity Process Variable
     --------------------------------- */
     free(pgcData->sevrPVName);
     buff = XmTextFieldGetString(propWindow->severityPVnameTextW);
     if (strlen(buff)) pgcData->sevrPVName = buff;
     else pgcData->sevrPVName = "-";

     /* ---------------------------------
     Alarm Count Filter
     --------------------------------- */
     if (linkType == CHANNEL) {
          cdata = (struct chanData *)pgcData;
          free(cdata->countFilter);
          cdata->countFilter = 0;
          buff = XmTextFieldGetString(propWindow->countFilterCountTextW);
          rtn = sscanf(buff,"%hd",&f1);
          buff = XmTextFieldGetString(propWindow->countFilterCountTextW);
          rtn2 = sscanf(buff,"%hd",&f2);
          if (rtn == 1 || rtn2 ==1 ){
              cdata->countFilter = (COUNTFILTER *)calloc(1,sizeof(COUNTFILTER));
              cdata->countFilter->clink=link;
              if (rtn == 1 ) cdata->countFilter->inputCount = f1;
              else cdata->countFilter->inputCount = 1;
              if (rtn2 == 1 ) cdata->countFilter->inputSeconds = f2;
              else cdata->countFilter->inputSeconds = 1;
          }
     }

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
     buff = XmTextFieldGetString(propWindow->forcePVforceValueTextW);
     rtn = sscanf(buff,"%hd",&f1);
     if (rtn == 1) pgcData->forcePVValue = f1;
     else pgcData->forcePVValue = 1;

     /*  update link field  - resetPVValue */
     buff = XmTextFieldGetString(propWindow->forcePVresetValueTextW);
     rtn = sscanf(buff,"%hd",&f1);
     if (rtn == 1) pgcData->resetPVValue = f1;
     else pgcData->resetPVValue = 0;

     /* ---------------------------------
     Alias
     --------------------------------- */
     if (pgcData->alias) free(pgcData->alias);
     buff = XmTextFieldGetString(propWindow->aliasTextW);
     if (strlen(buff)) pgcData->alias = buff;
     else pgcData->alias = 0;

     /* ---------------------------------
     Related Process Command
     --------------------------------- */
     if (pgcData->command) free(pgcData->command);
     buff = XmTextFieldGetString(propWindow->processTextW);
     if (strlen(buff)) pgcData->command = buff;
     else pgcData->command = 0;

     /* ---------------------------------
     Sevr Commands
     --------------------------------- */
     removeSevrCommandList(&(pgcData->sevrCommandList));

     buff = XmTextGetString(propWindow->sevrProcessTextW);
     if (strlen(buff)){
          while (TRUE) {
               addNewSevrCommand(&pgcData->sevrCommandList,buff);
               buff=strchr(buff,'\n');
               if ( !buff ) break;
               *buff='\0';
               buff++;
          }
     }

     /* ---------------------------------
     Stat Commands
     --------------------------------- */
     if (linkType == CHANNEL) {
          cdata = (struct chanData *)pgcData;
          removeStatCommandList(&(cdata->sevrCommandList));

          buff = XmTextGetString(propWindow->statProcessTextW);
          if (strlen(buff)){
               while (TRUE) {
                    addNewSevrCommand(&cdata->statCommandList,buff);
                    buff=strchr(buff,'\n');
                    if ( !buff ) break;
                    *buff='\0';
                    buff++;
               }
          }
     }

     /* ---------------------------------
     Guidance Text
     --------------------------------- */
     pt = sllFirst(&(link->GuideList));
     while (pt) {
           guideLink = (struct guideLink *)pt;
           pt = sllNext(pt);
           free(guideLink->list);
           free(guideLink);
     }
     sllInit(&(link->GuideList));

     buff = XmTextGetString(propWindow->guidanceTextW);
     if (strlen(buff)){
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
     Update dialog windows
     --------------------------------- */
     /*  update properties dialog window field */
     axUpdateDialogs(propWindow->area);

     /*  update group line data and tree window */
     line = link->lineGroupW;
     if (line && line->link==link ){
          line->pname = ((GCLINK *)link)->pgcData->name;
          if (((GCLINK *)link)->pgcData->alias){
              line->alias = ((GCLINK *)link)->pgcData->alias;
          } else {
              line->alias = ((GCLINK *)link)->pgcData->name;
          }
          markSelectedWidget(((ALINK *)propWindow->area)->groupWindow, 0);
          awRowWidgets(line, propWindow->area);
          markSelectedWidget( ((ALINK *)propWindow->area)->groupWindow, ((WLINE *)line->wline)->name);
     }

     /*  update tree line data and group window */
     line = link->lineTreeW;
     if (line && line->link==link ){
          line->pname = ((GCLINK *)link)->pgcData->name;
          if (((GCLINK *)link)->pgcData->alias){
              line->alias = ((GCLINK *)link)->pgcData->alias;
          } else {
              line->alias = ((GCLINK *)link)->pgcData->name;
          }
          awRowWidgets(line, propWindow->area);
     }

     /* ---------------------------------
     set undo data
     --------------------------------- */
     editUndoSet(undoLink,linkType, link, MENU_EDIT_UNDO_PROPERTIES, FALSE);

}

/******************************************************
  propHelpCallback
******************************************************/

static void propHelpCallback(widget, propWindow, cbs)
     Widget widget;
     struct propWindow *propWindow;
     XmAnyCallbackStruct *cbs;
{

     char *messageALH1 =
         "This dialog window allows an operator to view the alarm properties\n"
         "for a group or channel.\n"
         "  \n"
         "Press the Dismiss button to close the Properties dialog window.\n"
         "Press the Help    button to get this help description window.\n"
            ;
     char * messageALH2 = "  ";

     char *messageACT1 =
         "This dialog window allows an operator to view and change alarm properties\n"
         "for a group or channel.\n"
         "  \n"
         "Press the Apply   button to change the properties for the selected group or channel.\n"
         "Press the Reset   button to reset the properties to their initial values.\n"
         "Press the Dismiss button to close the Properties dialog window.\n"
         "Press the Help    button to get this help description window.\n"
            ;
     char * messageACT2 = "  ";

     if (programId == ALH) {
          createDialog(widget,XmDIALOG_INFORMATION, messageALH1,messageALH2);
     } else {
          createDialog(widget,XmDIALOG_INFORMATION, messageACT1,messageACT2);
     }

}

/******************************************************
  propDismissCallback
******************************************************/

static void propDismissCallback(widget, propWindow, cbs)
     Widget widget;
     struct propWindow *propWindow;
     XmAnyCallbackStruct *cbs;
{
     Widget propDialog;

     propDialog = propWindow->propDialog;
     XtUnmanageChild(propDialog);
     XUnmapWindow(XtDisplay(propDialog), XtWindow(XtParent(propDialog)));
     if (propWindow->menuButton)
          XtVaSetValues(propWindow->menuButton, XmNset, FALSE, NULL);
}

/******************************************************
  propCancelCallback
******************************************************/

static void propCancelCallback(widget, propWindow, cbs)
     Widget widget;
     struct propWindow *propWindow;
     XmAnyCallbackStruct *cbs;
{
     propUpdateDialog((ALINK *)(propWindow->area));
}

/******************************************************
  propUndo
******************************************************/

void propUndo(area, link, linkType, undoLink)
     void *area;
     GCLINK *link;
     int linkType;
     GCLINK *undoLink;
{
     struct anyLine *line;

     if (!link) return;

     propMoveData(link,holdLink,linkType);
     propMoveData(undoLink,link,linkType);
     propMoveData(holdLink,undoLink,linkType);

     /* ---------------------------------
     Update dialog windows
     --------------------------------- */
     axUpdateDialogs(area);

     /* ---------------------------------
     Update tree line data and tree window 
     --------------------------------- */
     line = link->lineGroupW;
     if (line && line->link==link ){
          line->pname = ((GCLINK *)link)->pgcData->name;
          if (((GCLINK *)link)->pgcData->alias){
              line->alias = ((GCLINK *)link)->pgcData->alias;
          } else {
              line->alias = ((GCLINK *)link)->pgcData->name;
          }
          awRowWidgets(line,area);
     }

     /* ---------------------------------
     Update group line data and group window
     --------------------------------- */
     line = link->lineTreeW;
     if (line && line->link==link ){
          line->pname = ((GCLINK *)link)->pgcData->name;
          if (((GCLINK *)link)->pgcData->alias){
              line->alias = ((GCLINK *)link)->pgcData->alias;
          } else {
              line->alias = ((GCLINK *)link)->pgcData->name;
          }
          awRowWidgets(line,area);
     }

     /* ---------------------------------
     set undo data
     --------------------------------- */
     editUndoSet(undoLink,linkType, link, MENU_EDIT_UNDO_PROPERTIES, FALSE);

}

/******************************************************
  propMoveData
******************************************************/

static void propMoveData(fromLink, toLink, linkType)
     GCLINK *fromLink;
     GCLINK *toLink;
     int linkType;
{
     struct chanData *cfromData;
     struct chanData *ctoData;
     struct gcData *fromData;
     struct gcData *toData;
     struct guideLink *guideLink;
     SNODE *pt;

     if (!toLink || !fromLink) return;

     fromData = fromLink->pgcData;
     toData = toLink->pgcData;

     /* ---------------------------------
     Group/Channel Name 
     --------------------------------- */
     free(toData->name);
     toData->name = fromData->name;
     fromData->name = NULL;

     /* ---------------------------------
     Alarm Mask 
     --------------------------------- */
     if (linkType == CHANNEL) {
          ctoData = (struct chanData *)toData;
          cfromData = (struct chanData *)fromData;
          ctoData->curMask = cfromData->curMask;
          ctoData->defaultMask = cfromData->defaultMask;
     }

     /* ---------------------------------
     Severity Process Variable
     --------------------------------- */
     free(toData->sevrPVName);
     toData->sevrPVName = fromData->sevrPVName;
     fromData->sevrPVName = NULL;

     /* ---------------------------------
     Alarm Count Filter 
     --------------------------------- */
     if (linkType == CHANNEL) {
          ctoData = (struct chanData *)toData;
          cfromData = (struct chanData *)fromData;
          free(ctoData->countFilter);
          ctoData->countFilter = cfromData->countFilter;
          cfromData->countFilter = NULL;
     }

     /* ---------------------------------
     Force Process Variable
     --------------------------------- */
     free(toData->forcePVName);
     toData->forcePVName = fromData->forcePVName;
     fromData->forcePVName = NULL;
     toData->forcePVMask = fromData->forcePVMask;
     toData->forcePVValue = fromData->forcePVValue;
     toData->resetPVValue = fromData->resetPVValue;

     /* ---------------------------------
     Alias
     --------------------------------- */
     free(toData->alias);
     toData->alias = fromData->alias;
     fromData->alias = NULL;

     /* ---------------------------------
     Related Process Command
     --------------------------------- */
     free(toData->command);
     toData->command = fromData->command;
     fromData->command = NULL;

     /* ---------------------------------
     Sevr Command List
     --------------------------------- */
     removeSevrCommandList(&toData->sevrCommandList);
     toData->sevrCommandList = fromData->sevrCommandList;
     ellInit(&fromData->sevrCommandList);

     /* ---------------------------------
     Stat Command List
     --------------------------------- */
     if (linkType == CHANNEL) {
          ctoData = (struct chanData *)toData;
          cfromData = (struct chanData *)fromData;
          removeStatCommandList(&ctoData->statCommandList);
          ctoData->statCommandList = cfromData->statCommandList;
          ellInit(&cfromData->statCommandList);
     }


     /* ---------------------------------
     Guidance Text
     --------------------------------- */
     pt = sllFirst(&(toLink->GuideList));
     while (pt) {
           guideLink = (struct guideLink *)pt;
           pt = sllNext(pt);
           free(guideLink->list);
           free(guideLink);
     }
     toLink->GuideList = fromLink->GuideList;
     sllInit(&fromLink->GuideList);

}


/******************************************************
  propEditableDialogWidgets
******************************************************/

static void propEditableDialogWidgets(area)
     ALINK  *area;
{
     struct propWindow *propWindow;

     propWindow = (struct propWindow *)area->propWindow;

     if (programId == ALH) {
          XtVaSetValues(propWindow->nameTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->severityPVnameTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->countFilterCountTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->countFilterSecondsTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->forcePVnameTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->forcePVforceValueTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->forcePVresetValueTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->aliasTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->processTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->sevrProcessTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->statProcessTextW,XmNeditable, FALSE, NULL);
          XtVaSetValues(propWindow->guidanceTextW,XmNeditable, FALSE, NULL);
     } else {
          XtVaSetValues(propWindow->nameTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->severityPVnameTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->countFilterCountTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->countFilterSecondsTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->forcePVnameTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->forcePVforceValueTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->forcePVresetValueTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->aliasTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->processTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->sevrProcessTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->statProcessTextW,XmNeditable, TRUE, NULL);
          XtVaSetValues(propWindow->guidanceTextW,XmNeditable, TRUE, NULL);
     }
     return;
}

