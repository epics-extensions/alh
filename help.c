/* help.h */

/************************DESCRIPTION***********************************
*This files contains the  *PUBLIC*  routines for creating a
*dialog message box. The created message dialog consists 
*a message text widget and two push buttons-- 'Ok', and 
*'Help'.
*
*The presence of 'Help' button depends on the help textaul
*string array.  A single null string corresponds to a 'Help'
*button and causes remaining text shown on next page. Two 
*consecutive null strings indicates the end of help text.  
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#include <stdlib.h>

#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>

/* WIN32 differences */
#ifdef WIN32
/* Needs to be included before XlibXtra.h for Exceed 5 */
#include <stdio.h>
/* Hummingbird extra functions including lprintf
 *   Needs to be included after Intrinsic.h for Exceed 5
 *   (Intrinsic.h is included in xtParams.h) */
#include <X11/XlibXtra.h>
/* This is done in Exceed 6 but not in Exceed 5
 *   Need it to define printf as lprintf for Windows
 *   (as opposed to Console) apps */
#ifdef _WINDOWS
#ifndef printf
#define printf lprintf
#endif
#endif
#endif /* #ifdef WIN32 */

#include "ax.h"

/*******************************************************
	delete an existing widget
*******************************************************/
static void xs_ok_callback(Widget w,void *client_data,
XmAnyCallbackStruct *call_data)
{
	XtUnmanageChild(w);
	XtDestroyWidget(w);
}

/*************************************************************
 *  xs_str_array_to_xmstr() : convert char ** to
 *	compound string
 ************************************************************/
static XmString xs_str_array_to_xmstr(char *cs[],int n)
{
	XmString xmstr,xmsep, xmstr1,xmstr2 ;
	XmString str;
	int i;
	/*
	 	* if the array is empty just return an empty string.
	 	*/
	if (n <= 0)
		return(XmStringCreate("",XmSTRING_DEFAULT_CHARSET));

	xmstr = (XmString) NULL;
	xmsep =  XmStringSeparatorCreate();

	for (i=0;i<n;i++) {
		xmstr1=xmstr;
		if (i > 0) xmstr = XmStringConcat(xmstr1,xmsep);
		XmStringFree(xmstr1);
		xmstr1 = xmstr;
		xmstr2 = XmStringCreate(cs[i],XmSTRING_DEFAULT_CHARSET);
		xmstr = XmStringConcat(xmstr1,xmstr2);
		XmStringFree(xmstr1);
		XmStringFree(xmstr2);
	}
	XmStringFree(xmsep);
	return (xmstr);
}

/*************************************************************
 *  the help button call back function from a dialog widget
 ************************************************************/
void xs_help_callback(Widget w,char *str[],void *call_data)
{
	int 	i,n;
	Widget  dialog;
	XmString xmstr;
	Arg	wargs[5];
	char *title=NULL;
	char *buff=NULL;

	/*
	 * Get the title name
	 */
	XtVaGetValues(w, XmNuserData, &title,NULL);
	if (title){
		buff=(char *)calloc(1,strlen(title)+12);
		strcpy(buff,title);
		strcat(buff," Guidance");
		strcat(buff,"\0");
	}

	/* 
	 * Create the message dialog to display the help.
	 */
	n=0;
	if (buff){
		XtSetArg(wargs[n], XmNtitle, buff); 
		n++;
	}
	XtSetArg(wargs[n], XmNautoUnmanage, FALSE); 
	n++;
	XtSetArg(wargs[n], XmNallowShellResize, FALSE); 
	n++;
	dialog = XmCreateMessageDialog(w, "Help", wargs, n);
	free(buff);
	/*
	 * We won't use the cancel widget. Unmanage it.
	 */
	XtUnmanageChild(XmMessageBoxGetChild(dialog,
	    XmDIALOG_CANCEL_BUTTON));
	/*
	 * Retrieve the label widget and make the 
	 * text left justified
	 */

	/*
		label = XmMessageBoxGetChild(dialog, 
				XmDIALOG_MESSAGE_LABEL);
	
		n = 0;
		XtSetArg(wargs[n],XmNalignment,XmALIGNMENT_BEGINNING); n++;
		XtSetValues(label, wargs, n);
	*/

	/*
	 * Add an OK callback to pop down the dialog.
	 */
	XtAddCallback(dialog, XmNokCallback,(XtCallbackProc)xs_ok_callback, NULL);

	/*
	 * count the test up to the first NUll string.
	 */
	for (i=0;str[i][0] != '\0';i++)
		;
	/*
	 * convert the string array to an XmString array and set it as the label text.
	*/


	xmstr = xs_str_array_to_xmstr(str,i);

	n = 0;
	XtSetArg(wargs[n], XmNmessageString, xmstr); 
	n++;
	XtSetValues(dialog, wargs, n);

	XmStringFree(xmstr);

	/*
	 * if the next entryu in the help string array is also NULL,
	 * then this is the last message. Unmanage the help button.
	 */

	if (str[++i][0] == '\0')
		XtUnmanageChild( XmMessageBoxGetChild(dialog,
		    XmDIALOG_HELP_BUTTON));

		/*
		 * otherwise add a help callback function with the address of
		 * the next entryu in the help string as client_data.
		 */

	else {
		XtAddCallback(dialog, XmNhelpCallback,
		    (XtCallbackProc)xs_help_callback, &str[i]);

	}

	/*
	 * display the dialog.
	 */
	XtManageChild(dialog);

}


/******************************************************
  helpCallback
******************************************************/
void helpCallback(Widget widget,int item,XmAnyCallbackStruct *cbs)
{
	createDialog(widget,XmDIALOG_INFORMATION,
	    "Help is not available in this release."," ");
}

