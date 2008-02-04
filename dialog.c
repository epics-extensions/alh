/*************************************************************************\
* Copyright (c) 2002 The University of Chicago, as Operator of Argonne
* National Laboratory.
* Copyright (c) 2002 Deutches Elektronen-Synchrotron in der Helmholtz-
* Gemelnschaft (DESY).
* Copyright (c) 2002 Berliner Speicherring-Gesellschaft fuer Synchrotron-
* Strahlung mbH (BESSY).
* Copyright (c) 2002 Southeastern Universities Research Association, as
* Operator of Thomas Jefferson National Accelerator Facility.
* Copyright (c) 2002 The Regents of the University of California, as
* Operator of Los Alamos National Laboratory.
* This file is distributed subject to a Software License Agreement found
* in the file LICENSE that is included with this distribution. 
\*************************************************************************/
/* dialog.c */

/************************DESCRIPTION***********************************
  Creates  alh dialogs
**********************************************************************/

#include <stdio.h>
#include <time.h>

#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>

#include "ax.h"

#define TIME_SIZE 18

extern Display *display;
extern int _no_error_popup;

static Widget warningbox = NULL;

/* function prototypes */
static void killWidget(Widget w, XtPointer clientdata, XtPointer calldata);
static void killDialog(Widget w, XtPointer clientdata, XtPointer calldata);
static void logMessageString(Widget w, XtPointer clientdata, XtPointer calldata);


/******************************************************
  Create a fileSelectionBox
******************************************************/
Widget createFileDialog(Widget parent,void *okCallback,XtPointer okParm,
void *cancelCallback,XtPointer cancelParm,XtPointer userParm,
String title,String pattern,String directory)
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
    char file_sel[]="file_sel";

	/* parent = 0 means we want to unmanage the fileSelectdialog */
	if (!parent){
		if (fileselectdialog && XtIsManaged(fileselectdialog))
			XtUnmanageChild(fileselectdialog);
		return(fileselectdialog);
	}

	/* destroy runtimeToplevel fileselectdialog 
	        so we will not have exposure problems */
	if ( parent && fileselectdialog &&
	    XtParent(XtParent(fileselectdialog)) != parent) {
		XtDestroyWidget(fileselectdialog);
		fileselectdialog = 0;
	}

	/* "Open" was selected.  Create a Motif FileSelectionDialog w/callback */
	if (!fileselectdialog) {
		fileselectdialog = XmCreateFileSelectionDialog(parent,
		    file_sel, NULL, 0);
		XtVaSetValues(fileselectdialog,
		    XmNallowShellResize, FALSE,
		    NULL);
		XtAddCallback(fileselectdialog,XmNhelpCallback,
		    (XtCallbackProc)helpCallback,(XtPointer)NULL);
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
	    XmNpattern,       Xpattern,
	    (XtPointer)NULL);

	XmStringFree(Xtitle);
	XmStringFree(Xpattern);
	XmStringFree(Xdirectory);

	XtAddCallback(fileselectdialog,XmNokCallback,
	    (XtCallbackProc)okCallback, (XtPointer)okParm);
	XtAddCallback(fileselectdialog,XmNcancelCallback,
	    (XtCallbackProc)cancelCallback,(XtPointer)cancelParm);
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
void createDialog(Widget parent,int dialogType,char *message1,char *message2)
{
	XmString        str,str1,str2,str3,string,string2;
	Widget   dialog = 0;

	dialog = XmCreateMessageDialog(parent, "Dialog", NULL, 0);
	XtUnmanageChild(XmMessageBoxGetChild(dialog,XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(dialog,XmDIALOG_HELP_BUTTON));
	XtSetSensitive(XmMessageBoxGetChild(dialog,XmDIALOG_HELP_BUTTON),FALSE);
	XtAddCallback(dialog,XmNokCallback,killDialog,NULL);

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

	str3 = XmStringCreateLtoR("ALH ",XmFONTLIST_DEFAULT_TAG);
	string2 = XmStringConcat(str3,str);

	XtVaSetValues(dialog,
	    XmNdialogType,  dialogType,
	    XmNdialogTitle, string2,
	    XmNmessageString, string,
	    (XtPointer)NULL);
	XmStringFree(str);
	XmStringFree(str1);
	XmStringFree(str2);
	XmStringFree(string);

	XtManageChild(dialog);
	XFlush(display);
}

/******************************************************
  createActionDialog
******************************************************/
void createActionDialog(Widget parent,int dialogType,char *message1,
XtCallbackProc okCallback,XtPointer okParm,XtPointer userParm)
{
	static Widget         dialog = 0; /* make it static for reuse */
	XmString              str,str1,str2,str3;
	static XtCallbackProc oldOkCallback = 0;
	static XtPointer      oldOkParm = 0;

	if (dialog && XtIsManaged(dialog)) XtUnmanageChild(dialog);
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

	str1 = XmStringCreateLtoR("ALH ",XmFONTLIST_DEFAULT_TAG);
	str3 = XmStringConcat(str1,str);

	str2=XmStringCreateLtoR(message1,XmSTRING_DEFAULT_CHARSET);
	XtVaSetValues(dialog,
	    XmNuserData,      userParm,
	    XmNdialogType,  dialogType,
	    XmNdialogTitle, str3,
	    XmNmessageString, str2,
	    (XtPointer)NULL);
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
  Error message popup
******************************************************/
void errMsg(const char *fmt, ...)
{
	XmString cstring,cstringOld,cstringNew;
	va_list vargs;
	static char lstring[1024];  /* DANGER: Fixed buffer size */
	static int warningboxMessages = 0;
	int nargs=10;
	Arg args[10];
    size_t len;
    struct tm * tms;
    time_t timeofday;

    timeofday = time(0L);
    tms = localtime(&timeofday);
    len = strftime(lstring,TIME_SIZE,"%Y/%m/%d %H:%M ",tms);
	va_start(vargs,fmt);
	vsprintf(&lstring[TIME_SIZE-1],fmt,vargs);
	va_end(vargs);

	if(lstring[TIME_SIZE-1] == '\0') return;

	alLogOpModMessage(0,0,&lstring[TIME_SIZE-1]);

	if (_no_error_popup) {
		return;
	}

	if (warningbox) {

		if (warningboxMessages > 30) return;

		cstring=XmStringCreateLtoR(lstring,XmSTRING_DEFAULT_CHARSET);
		XtVaGetValues(warningbox, XmNmessageString, &cstringOld, NULL);
		cstringNew = XmStringConcat(cstringOld,cstring);
		XmStringFree(cstring);
		XmStringFree(cstringOld);
		if (warningboxMessages == 30){
			cstring=XmStringCreateLtoR(
				"\nOnly first 30 messages are displayed and logged\n",
				XmSTRING_DEFAULT_CHARSET);
			cstringOld = cstringNew;
			cstringNew = XmStringConcat(cstringOld,cstring);
			XmStringFree(cstring);
			XmStringFree(cstringOld);
		}
		XtVaSetValues(warningbox, XmNmessageString, cstringNew, NULL);
		XmStringFree(cstringNew);
		warningboxMessages += 1;
		XtManageChild(warningbox);
	} else {
		XBell(display,50); 
		XBell(display,50); 
		XBell(display,50);
		cstring=XmStringCreateLtoR(lstring,XmSTRING_DEFAULT_CHARSET);
		nargs=0;
		XtSetArg(args[nargs],XmNtitle,"ALH Warning"); 
		nargs++;
		XtSetArg(args[nargs],XmNmessageString,cstring); 
		nargs++;
		warningbox=XmCreateWarningDialog(topLevelShell,"warningMessage",
		    args,nargs);
		XmStringFree(cstring);
		XtDestroyWidget(XmMessageBoxGetChild(warningbox,XmDIALOG_CANCEL_BUTTON));
		XtDestroyWidget(XmMessageBoxGetChild(warningbox,XmDIALOG_HELP_BUTTON));
		/*XtAddCallback(warningbox,XmNokCallback,logMessageString,NULL);*/
		XtAddCallback(warningbox,XmNokCallback,killWidget,NULL);
		warningboxMessages = 1;
		XtManageChild(warningbox);
	}
}

/******************************************************
  Fatal error message popup
******************************************************/
void fatalErrMsg(const char *fmt, ...)
{
	va_list vargs;
	static char lstring[1024];  /* DANGER: Fixed buffer size */

	va_start(vargs,fmt);
	vsprintf(lstring,fmt,vargs);
	va_end(vargs);

	if(lstring[0] == '\0') return;

	errMsg(lstring);
	if (warningbox) XtAddCallback(warningbox,XmNokCallback,exit_quit,NULL);
}

/******************************************************
  Static callback routine for logging a widget's messageString to opMod log file
******************************************************/
static void logMessageString(Widget w, XtPointer clientdata, XtPointer calldata)
{
	XmString cstring;
	XmStringContext context;
	char *lstring;
	XmStringCharSet tag;
	XmStringDirection direction;
	Boolean separator;

	XtVaGetValues(w, XmNmessageString, &cstring, NULL);
	if (!XmStringInitContext(&context,cstring)) {
		XmStringFree(cstring);
		return;
	}

	while (XmStringGetNextSegment(context,&lstring,&tag,&direction,&separator)){
		alLogOpModMessage(0,0,lstring);
		XtFree(lstring);
		lstring=0;
	}
	if (lstring) XtFree(lstring);
	XmStringFree(cstring);
	XmStringFreeContext(context);
    XtFree(tag);
}

/******************************************************
  Static callback routine for destroying a widget 
******************************************************/
static void killWidget(Widget w, XtPointer clientdata, XtPointer calldata)
{
	XtDestroyWidget(w);
	if (w == warningbox) {
		warningbox = NULL;
	}
}

/******************************************************
  Static callback routine for destroying a widget 
******************************************************/
static void killDialog(Widget w, XtPointer clientdata, XtPointer calldata)
{
	XtDestroyWidget(w);
}
