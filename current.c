static char *sccsId = "@(#)current.c	1.8\t9/14/93";

/* current.c	
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
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */
/***********************************************************
*
*    current.c
*
*This file contains the routines for viewing the current alarm history.

Routines defined in current.c:

-------------
|   PUBLIC  |
-------------
void currentAlarmHistoryWindow(area,widget)         Create/manage current alarm window
void updateCurrentAlarmString(string)               Update XmString with new alarm
void updateCurrentAlarmWindow(string)               Update window with new strings

-------------
|  PRIVATE  |
-------------
void closeCurrent_callback(w,popup_shell,call_data)  Close current alarm window
*
**********************************************************/
#include <stdio.h>
#include <malloc.h>

#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/DialogS.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/CascadeB.h>
#include <Xm/LabelG.h>
#include <Xm/Protocols.h>
#include <Xm/Text.h>

#include <alh.h>
#include <axArea.h>
#include <cadef.h>
#include <ax.h>
 
/* global variables */
struct currentData {
     time_t timeofday;
     char *name;
     int   sevr;
     int   stat;
     char  value[MAX_STRING_SIZE];
};
struct currentData currentAlarm[10];
int currentAlarmIndex;

extern char *alarmSeverityString[];
extern char *alarmStatusString[];

#ifdef __STDC__

void closeCurrent_callback( Widget w, Widget currentForm, caddr_t call_data);

#else

void closeCurrent_callback();

#endif /*__STDC__*/


/**************************************************************************
	create scroll window for file view
**************************************************************************/
void currentAlarmHistoryWindow(area,menuButton)
     ALINK *area;
     Widget menuButton;
{
     static Widget popup_shell,title,button;
     Widget previous;
     Atom         WM_DELETE_WINDOW;
     
     int i;
     
     if (!popup_shell) {

          popup_shell = XtAppCreateShell("Alarm Handler: Current Alarm History", programName,
               applicationShellWidgetClass, display, NULL, 0);

          /*  create current alarm view window */
/*
          popup_shell = XtVaCreatePopupShell("Alarm Handler: Current Alarm History",
               xmDialogShellWidgetClass,   area->runtimeForm,
               XmNautoUnmanage,            FALSE,
               NULL);
*/
          
          /*  create current alarm form window */
          area->currentAlarmForm = XtVaCreateManagedWidget("CurrentAlarm",
               xmFormWidgetClass,         popup_shell,
               XmNallowOverlap,           FALSE,
               XmNuserData,               menuButton,
               XmNnoResize,               TRUE,
               NULL);
          
          /* Modify the window manager menu "close" callback */
          XtVaSetValues(popup_shell,
               XmNdeleteResponse,       XmDO_NOTHING,
               NULL);
          WM_DELETE_WINDOW = XmInternAtom(XtDisplay(popup_shell),
               "WM_DELETE_WINDOW", False);
          XmAddWMProtocolCallback(popup_shell,WM_DELETE_WINDOW,
               (XtCallbackProc)closeCurrent_callback, (XtPointer)area->currentAlarmForm);
           
          /* add close button */
          button = XtVaCreateManagedWidget("Close",
               xmPushButtonWidgetClass, area->currentAlarmForm,
               XmNtopAttachment,          XmATTACH_FORM,
               XmNtopOffset,              5,
               XmNleftAttachment,         XmATTACH_FORM,
               XmNleftOffset,             5,
               XmNuserData,               menuButton,
               NULL);
          XtAddCallback(button, XmNactivateCallback,
               (XtCallbackProc)closeCurrent_callback, area->currentAlarmForm);
     
          previous = button;
          
          /* add title line */
          title = XtVaCreateManagedWidget( "    TIME_STAMP        PROCESS_VARIABLE_NAME        STATUS     SEVERITY   VALUE       ",
               xmLabelGadgetClass,        area->currentAlarmForm,
               XmNtopAttachment,          XmATTACH_FORM,
               XmNtopOffset,              10,
               XmNleftAttachment,         XmATTACH_WIDGET,
               XmNleftWidget,             button,
               XmNrightAttachment,        XmATTACH_FORM,
               NULL);
          
          
          /* create 10 label widgets  */
          for ( i=0;i<10;i++){
               area->currentAlarm[i] = XtVaCreateManagedWidget( "-----",
                    xmLabelGadgetClass,        area->currentAlarmForm,
                    XmNmarginHeight,           1,
                    XmNalignment,              XmALIGNMENT_BEGINNING,
                    XmNtopAttachment,          XmATTACH_WIDGET,
                    XmNtopWidget,              previous,
                    XmNleftAttachment,         XmATTACH_FORM,
                    XmNrightAttachment,        XmATTACH_FORM,
                    NULL);
               previous = area->currentAlarm[i];
          }

          /* create a blank line, because Form marginHeight wont work  */
          (void)XtVaCreateManagedWidget( " ",
               xmLabelGadgetClass,        area->currentAlarmForm,
               XmNalignment,              XmALIGNMENT_BEGINNING,
               XmNtopAttachment,          XmATTACH_WIDGET,
               XmNtopWidget,              previous,
               XmNleftAttachment,         XmATTACH_FORM,
               XmNrightAttachment,        XmATTACH_FORM,
               NULL);

     XtRealizeWidget(popup_shell);
     updateCurrentAlarmWindow(area);


     } else {

          if (XtIsManaged(area->currentAlarmForm)){
               XtVaSetValues(menuButton, XmNset, FALSE, NULL);
               XtUnmanageChild(area->currentAlarmForm);
               XUnmapWindow(XtDisplay(area->currentAlarmForm),
                  XtWindow(XtParent(area->currentAlarmForm)));
          } else {
               XtVaSetValues(menuButton, XmNset, TRUE, NULL);
               XtManageChild(area->currentAlarmForm); 
               XMapWindow(XtDisplay(area->currentAlarmForm),
                  XtWindow(XtParent(area->currentAlarmForm)));
               updateCurrentAlarmWindow(area);
          }
     }
     
}
     
     
     
/**************************************************************************
	close Current Alarm History window 
**************************************************************************/
void closeCurrent_callback(w,currentForm,call_data)
     Widget w;
     Widget currentForm;
     caddr_t call_data;
{
     Widget menuButton;

     XtVaGetValues(currentForm, XmNuserData, &menuButton, NULL);

     XtVaSetValues(menuButton, XmNset, FALSE, NULL);
     XtUnmanageChild(currentForm);
     XUnmapWindow(XtDisplay(currentForm),
          XtWindow(XtParent(currentForm)));
}
     
/******************************************************************
      updateCurrent Alarm History strings
*****************************************************************/

void updateCurrentAlarmString(ptimeofday,name,value,stat,sev)
     time_t *ptimeofday;
     int stat,sev;
     char *name;
     char value[];
{
     int n;

     n = currentAlarmIndex;
     currentAlarm[n].timeofday=*ptimeofday;
     currentAlarm[n].name=name;
     currentAlarm[n].stat=stat;
     currentAlarm[n].sevr=sev;
     strncpy(currentAlarm[n].value,value,MAX_STRING_SIZE);  
     n = (n+1)%10;
     currentAlarmIndex = n;

}

/******************************************************************
      updateCurrent Alarm History Window
*****************************************************************/
void updateCurrentAlarmWindow(area)
     ALINK *area;
{
     int i,j=0;
     XmString xstr;
     char *str;
     char buff[100];
     
   
     if ( area->currentAlarmForm && XtIsManaged(area->currentAlarmForm) ) {

          j = currentAlarmIndex;
          for (i=0;i<10;i++){
               if (currentAlarm[j].name){
                    str = ctime(&(currentAlarm[j].timeofday));
                    *(str + strlen(str)-1) = '\0';
           
                    sprintf(buff,
                         "  %-24s :  %-28s %-10s %-10s %s",
                         str,
                         currentAlarm[j].name,
                         alarmStatusString[currentAlarm[j].stat],
                         alarmSeverityString[currentAlarm[j].sevr],
                         currentAlarm[j].value);  
               } else {
                    strcpy(buff,"  ");
               }
               xstr = XmStringCreateSimple(buff);
               XtVaSetValues(area->currentAlarm[i],
                   XmNlabelString, xstr,
                   NULL); 
               XmStringFree(xstr);
               j = (j+1)%10;
          }
     }
}

/******************************************************************
      reset Current Alarm History Window
*****************************************************************/
void resetCurrentAlarmWindow()
{
     int i,j=0;
     XmString xstr;
     char *str;
     char buff[100];
     
     for (i=0;i<10;i++){
          currentAlarm[j].name = NULL;
     }
}
