/*
 $Log$
 Revision 1.8  1998/07/23 16:27:50  jba
 Changed XmStringCreateSimple to XmStringCreateLtoR.

 Revision 1.7  1998/06/03 12:57:14  evans
 Added destroy callback (killWidget) for errMsg.

 Revision 1.6  1998/06/02 19:40:50  evans
 Changed from using Fgmgr to using X to manage events and file
 descriptors.  (Fdmgr didn't work on WIN32.)  Uses XtAppMainLoop,
 XtAppAddInput, and XtAppAddTimeOut instead of Fdmgr routines.
 Updating areas is now in alCaUpdate, which is called every caDelay ms
 (currently 100 ms).  Added a general error message routine (errMsg)
 and an exception handler (alCAException).  Is working on Solaris and
 WIN32.

 Revision 1.5  1995/10/20 16:50:33  jba
 Modified Action menus and Action windows
 Renamed ALARMCOMMAND to SEVRCOMMAND
 Added STATCOMMAND facility
 Added ALIAS facility
 Added ALARMCOUNTFILTER facility
 Make a few bug fixes.

 * Revision 1.4  1995/06/22  19:40:23  jba
 * Started cleanup of file.
 *
 * Revision 1.3  1995/02/28  16:43:43  jba
 * ansi c changes
 *
 * Revision 1.2  1994/06/22  21:17:23  jba
 * Added cvs Log keyword
 *
 */

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

/* function prototypes */
static void killWidget(Widget w, XtPointer clientdata, XtPointer calldata);


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
     XmString        Xcurrentdir=0;
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
          XtVaGetValues(fileselectdialog, XmNdirectory, &Xcurrentdir, NULL);
          if (oldOk)     XtRemoveCallback(fileselectdialog,XmNokCallback,
                              (XtCallbackProc)oldOk     ,(XtPointer)oldOkParm);
          if (oldCancel) XtRemoveCallback(fileselectdialog,XmNcancelCallback,
                              (XtCallbackProc)oldCancel ,(XtPointer)oldCancelParm);
     }

     Xtitle=XmStringCreateLtoR(title,XmSTRING_DEFAULT_CHARSET);
     Xpattern=XmStringCreateLtoR(pattern,XmSTRING_DEFAULT_CHARSET);
     if ( !directory  && Xcurrentdir ) Xdirectory = Xcurrentdir;
     else Xdirectory = XmStringCreateLtoR(directory,XmSTRING_DEFAULT_CHARSET);

     XtVaSetValues(fileselectdialog,
          XmNuserData,      userParm,
          XmNdialogTitle,   Xtitle,
          XmNdirectory,     Xdirectory,
          XmNdirMask,       Xpattern,
          NULL);

     XmStringFree(Xtitle);
     XmStringFree(Xpattern);
     XmStringFree(Xdirectory);

     XtAddCallback(fileselectdialog,XmNokCallback, (XtCallbackProc)okCallback, (XtPointer)okParm);
     XtAddCallback(fileselectdialog,XmNcancelCallback, (XtCallbackProc)cancelCallback,(XtPointer)cancelParm);
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
     XmString        str,str1,str2,string;

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
          XtUnmanageChild(XmMessageBoxGetChild(dialog,XmDIALOG_HELP_BUTTON));
          XtSetSensitive(XmMessageBoxGetChild(dialog,XmDIALOG_HELP_BUTTON),FALSE);
          XtAddCallback(dialog,XmNokCallback, (XtCallbackProc)XtUnmanageChild,NULL);
     }

     switch(dialogType) {
          case XmDIALOG_WARNING:
               str=XmStringCreateLtoR("WarningDialog",XmSTRING_DEFAULT_CHARSET);
               break;
          case XmDIALOG_ERROR:
               str=XmStringCreateLtoR("ErrorDialog",XmSTRING_DEFAULT_CHARSET);
               break;
          case XmDIALOG_INFORMATION:
               str=XmStringCreateLtoR("InformationDialog",XmSTRING_DEFAULT_CHARSET);
               break;
          case XmDIALOG_MESSAGE:
               str=XmStringCreateLtoR("MessageDialog",XmSTRING_DEFAULT_CHARSET);
               break;
          case XmDIALOG_QUESTION:
               str=XmStringCreateLtoR("QuestionDialog",XmSTRING_DEFAULT_CHARSET);
               break;
          case XmDIALOG_WORKING:
               str=XmStringCreateLtoR("WorkDialog",XmSTRING_DEFAULT_CHARSET);
               break;
          default:
               str=XmStringCreateLtoR("Dialog",XmSTRING_DEFAULT_CHARSET);
               break;
     }
     
     str1 = XmStringCreateLtoR(message1,XmFONTLIST_DEFAULT_TAG);
     str2 = XmStringCreateLtoR(message2,XmFONTLIST_DEFAULT_TAG);
     string = XmStringConcat(str1,str2);

     XtVaSetValues(dialog,
          XmNdialogType,  dialogType,
          XmNdialogTitle, str,
          XmNmessageString, string,
          NULL);
     XmStringFree(str);
     XmStringFree(str1);
     XmStringFree(str2);
     XmStringFree(string);

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
     
     str2=XmStringCreateLtoR(message1,XmSTRING_DEFAULT_CHARSET);
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

/******************************************************
  Error handler
******************************************************/
void errMsg(const char *fmt, ...)
{
    Widget warningbox,child;
    XmString cstring;
    va_list vargs;
    static char lstring[1024];  /* DANGER: Fixed buffer size */
    int nargs=10;
    Arg args[10];
    
    va_start(vargs,fmt);
    vsprintf(lstring,fmt,vargs);
    va_end(vargs);
    
    if(lstring[0] != '\0') {
	XBell(display,50); XBell(display,50); XBell(display,50); 
	cstring=XmStringCreateLtoR(lstring,XmSTRING_DEFAULT_CHARSET);
	nargs=0;
	XtSetArg(args[nargs],XmNtitle,"Warning"); nargs++;
	XtSetArg(args[nargs],XmNmessageString,cstring); nargs++;
	warningbox=XmCreateWarningDialog(topLevelShell,"warningMessage",
	  args,nargs);
	XmStringFree(cstring);
	child=XmMessageBoxGetChild(warningbox,XmDIALOG_CANCEL_BUTTON);
	XtDestroyWidget(child);
	child=XmMessageBoxGetChild(warningbox,XmDIALOG_HELP_BUTTON);
	XtDestroyWidget(child);
	XtManageChild(warningbox);
	XtAddCallback(warningbox,XmNokCallback,killWidget,NULL);
#ifdef WIN32
	lprintf("%s\n",lstring);
#else
	fprintf(stderr,"%s\n",lstring);
#endif
    }
}

static void killWidget(Widget w, XtPointer clientdata, XtPointer calldata)
{
    XtDestroyWidget(w);
}
