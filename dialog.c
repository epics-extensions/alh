static char *sccsId = "@(#)dialog.c	1.8\t2/3/94";

/* dialog.c	
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
 * .01  10-04-91        bkc     Redesign the setup window,
 *                              resolve problems with new config,
 *				separate force variables / process
 *				reposition the group force dialog box
 * .02  02-16-93        jba     Reorganized file for new user interface
 * .03  12-16-93        jba     createFileDialog now returns Widget
 * .04  02-06-94        jba     Fixed dialog parent test
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>

#include <alh.h>
#include <axArea.h>
#include <ax.h>

extern XmString xs_str_array_to_xmstr();

/****************************************************
*
*This file contains routines for creating dialogs
*
****************************************************
--------------
|   PUBLIC   |
--------------
*
Widget createFileDialog(parent,okCallback,      Create a fileSelectionBox
     okParm,cancelCallback,cancelParm,
     userParm,title,pattern,dirSpec)
     Widget          parent;
     void *  okCallback;
     XtPointer       okParm;
     void *  cancelCallback;
     XtPointer       cancelParm;
     XtPointer       userParm;
     String          title;
     String          pattern;
     String          dirSpec;
*
void createDialog(parent,dialogType,          Create a Dialog, any type
     message1,message2)
     Widget          parent;
     int             dialogType;
     char           *message1;
     char           *message2;
*
void createActionDialog(parent,dialogType,    Create an action Dialog, any type
     message1,okCallback,okParm,userParm)
     Widget          parent;
     int            dialogType;
     String         message1;
     XtCallbackProc okCallback;
     XtPointer      okParm;
     XtPointer      userParm;
****************************************************/


/******************************************************
  createFileDialog
******************************************************/

Widget createFileDialog(parent,okCallback,okParm,cancelCallback,cancelParm,userParm,title,pattern,directory)
     Widget          parent;
     void *  okCallback;
     XtPointer      okParm;
     void *  cancelCallback;
     XtPointer      cancelParm;
     XtPointer      userParm;
     String          title;
     String          pattern;
     String          directory;
{
     XmString        Xtitle;
     XmString        Xpattern;
     XmString        Xdirectory;
     static Widget   fileselectdialog = 0; /* make it static for reuse */
     static void *oldOk= NULL;
     static void *oldCancel=NULL;
     static XtPointer oldOkParm = 0;
     static XtPointer oldCancelParm = 0;

     /* parent = 0 means we want to unmanage the fileSelectdialog */
     if (!parent){
          if (fileselectdialog && XtIsManaged(fileselectdialog))
               XtUnmanageChild(fileselectdialog);
          return(fileselectdialog);
     }


     /* destroy runtimeToplevel fileselectdialog so we will not have exposure problems */
     if ( parent && fileselectdialog &&
          XtParent(XtParent(fileselectdialog)) != parent) {
          XtDestroyWidget(fileselectdialog); 
          fileselectdialog = 0;
     }

     /* "Open" was selected.  Create a Motif FileSelectionDialog w/callback */
     if (!fileselectdialog) {
          fileselectdialog = XmCreateFileSelectionDialog(parent,
               "file_sel", NULL, 0);
          XtVaSetValues(fileselectdialog,
               XmNallowShellResize, FALSE,
               NULL);
          XtAddCallback(fileselectdialog,XmNhelpCallback,(XtCallbackProc)helpCallback,(XtPointer)NULL);
     } else {
          if (oldOk)     XtRemoveCallback(fileselectdialog,XmNokCallback,
                              oldOk     ,(XtPointer)oldOkParm);
          if (oldCancel) XtRemoveCallback(fileselectdialog,XmNcancelCallback,
                              oldCancel ,(XtPointer)oldCancelParm);
     }

     Xtitle = XmStringCreateSimple(title);
     Xpattern = XmStringCreateSimple(pattern);
     Xdirectory = XmStringCreateSimple(directory);

     XtVaSetValues(fileselectdialog,
          XmNuserData,      userParm,
          XmNdialogTitle,   Xtitle,
          XmNdirectory,     Xdirectory,
          XmNdirMask,       Xpattern,
          NULL);

     XmStringFree(Xtitle);
     XmStringFree(Xpattern);
     XmStringFree(Xdirectory);

     XtAddCallback(fileselectdialog,XmNokCallback, okCallback, (XtPointer)okParm);
     XtAddCallback(fileselectdialog,XmNcancelCallback, cancelCallback,(XtPointer)cancelParm);
     oldOk = okCallback;
     oldCancel = cancelCallback;
     oldOkParm = okParm;
     oldCancelParm = cancelParm;

     XtManageChild(fileselectdialog);
     XFlush(display);
/*
     XtPopup(XtParent(fileselectdialog), XtGrabNone);
*/
     return(fileselectdialog);
}

/******************************************************
  createDialog
******************************************************/

void createDialog(parent,dialogType,message1,message2)
     Widget          parent;
     int             dialogType;
     char           *message1;
     char           *message2;
{
     static Widget   dialog = 0; /* make it static for reuse */
     XmString        str;
     XmString        str2;
     char            buff[MAX_STRING_LENGTH];
     int             n;


     if (dialog) XtUnmanageChild(dialog);
     if (!dialogType ) return;

     /* destroy runtimeToplevel dialog so dialog is positioned properly */
     if ( !parent ) return;
     if ( parent && dialog &&
          XtParent(XtParent(dialog)) != parent) {
          XtDestroyWidget(dialog); 
          dialog = 0;
     }

     if (!dialog) {
          dialog = XmCreateMessageDialog(parent, "Dialog", NULL, 0);
          XtUnmanageChild(XmMessageBoxGetChild(dialog,XmDIALOG_CANCEL_BUTTON));
          XtSetSensitive(XmMessageBoxGetChild(dialog,XmDIALOG_HELP_BUTTON),FALSE);
          XtAddCallback(dialog,XmNokCallback, (XtCallbackProc)XtUnmanageChild,NULL);
     }

     switch(dialogType) {
          case XmDIALOG_WARNING:
               str = XmStringCreateSimple("WarningDialog");
               break;
          case XmDIALOG_ERROR:
               str = XmStringCreateSimple("ErrorDialog");
               break;
          case XmDIALOG_INFORMATION:
               str = XmStringCreateSimple("InformationDialog");
               break;
          case XmDIALOG_MESSAGE:
               str = XmStringCreateSimple("MessageDialog");
               break;
          case XmDIALOG_QUESTION:
               str = XmStringCreateSimple("QuestionDialog");
               break;
          case XmDIALOG_WORKING:
               str = XmStringCreateSimple("WorkingDialog");
               break;
          default:
               str = XmStringCreateSimple("Dialog");
               break;
     }
     
     strcpy(buff,message1);
     n=strlen(message1);
     strncat(buff,message2,MAX_STRING_LENGTH-n);
     str2 = XmStringCreateSimple(buff);

     XtVaSetValues(dialog,
          XmNdialogType,  dialogType,
          XmNdialogTitle, str,
          XmNmessageString, str2,
          NULL);
     XmStringFree(str);
     XmStringFree(str2);

     XtManageChild(dialog);
     XFlush(display);
/*
     XmUpdateDisplay(dialog);
*/
}

/******************************************************
  createActionDialog
******************************************************/

void createActionDialog(parent,dialogType,message1,okCallback,okParm,userParm)
     Widget          parent;
     int            dialogType;
     char           *message1;
     XtCallbackProc okCallback;
     XtPointer      okParm;
     XtPointer      userParm;
{
     static Widget         dialog = 0; /* make it static for reuse */
     XmString              str;
     XmString              str2;
     static XtCallbackProc oldOkCallback = 0;
     static XtPointer      oldOkParm = 0;

     if (dialog) XtUnmanageChild(dialog);
     if (!dialogType ) return;

     /* destroy runtimeToplevel dialog so dialog is positioned properly */
     if ( parent && dialog &&
          XtParent(XtParent(dialog)) != parent) {
          XtDestroyWidget(dialog); 
          dialog = 0;
     }

     if (!dialog) {
          dialog = XmCreateMessageDialog(parent, "Dialog", NULL, 0);
          XtSetSensitive(XmMessageBoxGetChild(dialog,XmDIALOG_HELP_BUTTON),FALSE);
          XtAddCallback(dialog,XmNcancelCallback,(XtCallbackProc) XtUnmanageChild,NULL);

     } else {
          XtRemoveCallback(dialog,XmNokCallback,oldOkCallback,(XtPointer)oldOkParm);
     }

     switch(dialogType) {
          case XmDIALOG_WARNING:
               str = XmStringCreateSimple("WarningDialog");
               break;
          case XmDIALOG_ERROR:
               str = XmStringCreateSimple("ErrorDialog");
               break;
          case XmDIALOG_INFORMATION:
               str = XmStringCreateSimple("InformationDialog");
               break;
          case XmDIALOG_MESSAGE:
               str = XmStringCreateSimple("MessageDialog");
               break;
          case XmDIALOG_QUESTION:
               str = XmStringCreateSimple("QuestionDialog");
               break;
          case XmDIALOG_WORKING:
               str = XmStringCreateSimple("WorkingDialog");
               break;
          default:
               str = XmStringCreateSimple("InformationDialog");
               break;
     }
     
     str2 = XmStringCreateSimple(message1);
     XtVaSetValues(dialog,
          XmNuserData,      userParm,
          XmNdialogType,  dialogType,
          XmNdialogTitle, str,
          XmNmessageString, str2,
          NULL);
     XmStringFree(str);
     XmStringFree(str2);

     XtAddCallback(dialog,XmNokCallback,okCallback,okParm);
     oldOkCallback = okCallback;
     oldOkParm = okParm;

     XtManageChild(dialog);
     XFlush(display);
/*
     XmUpdateDisplay(dialog);
*/
     return;
}
