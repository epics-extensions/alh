/*
 $Log$
 Revision 1.3  1995/02/28 16:43:40  jba
 ansi c changes

 * Revision 1.2  1994/06/22  21:17:17  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)clipboardOps.c	1.4\t9/9/93";


/***
 ***  routines (as event handlers) to allow a user to select a
 ***    push button label and paste it into a text field
 ***
 ***	30 Nov. 1990  (MDA)
 ***
 ***
 ***
 *** usage:
 ***  for a push button (whose label you want to pass)
 ***
 ***	XtAddEventHandler(pushButton,ButtonPressMask,FALSE,
 ***		clipboardPut,mainWidget);
 ***
 ***
 ***  for a text field (who should receive the text)
 ***
 ***	XtAddEventHandler(textField,ButtonPressMask,FALSE,
 ***		clipboardPut,mainWidget);
 ***
 ***
 ***  Note:  the last argument to the XtAddEventHandler() call
 ***	must the be same widget (since the clipboard compares
 ***	display and window parameters).  Also Note:  the widget
 ***	passed as topWidget (communication widget) must be managed 
 ***	prior to these calls).
 ***
 ***/

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/CutPaste.h>
#include <Xm/Text.h>


#include "clipboardOps.h"




void clipboardPut(widget,topWidget,event)
  Widget widget, topWidget;
  XEvent *event;
{
  int result;
  long itemId, dataId;
  Arg args[4];
  Boolean done;
  int n, loopCount;

  XmString xmstring;
  XmStringContext context;
  char *text;
  XmStringCharSet charset;
  XmStringDirection dir;
  Boolean separator;


  if (event->xbutton.button != BUTTON_NUMBER) return;

  n = 0; 
  XtSetArg(args[n],XmNlabelString,&xmstring); n++; 
  XtGetValues(widget,args,n);

  XmStringInitContext(&context,xmstring);
  XmStringGetNextSegment(context,&text,&charset,&dir,&separator);
  XmStringFreeContext(context);


/* try for unlocked clipboard; if locked loop (and wait) */
  done = FALSE;
  loopCount = 0;
  while( !done ) {

     result = XmClipboardStartCopy(XtDisplay(topWidget),XtWindow(topWidget),
     XmStringCreateSimple("clipboard"), event->xbutton.time, topWidget, NULL, &itemId);

     done = (result == ClipboardSuccess);
     loopCount++;
     if ( (!done) && (loopCount > MAX_CLIPBOARD_TRIES) ) {
	fprintf(stderr, "clipboardPut: %d %s\n", MAX_CLIPBOARD_TRIES,
	    "attempts to write to the clipboard have failed - aborting");
	done = TRUE;
	return;
     }

  }


/* copy the data */
   while((result = XmClipboardCopy(XtDisplay(topWidget), XtWindow(topWidget),
	itemId, "STRING", text, strlen(text), 0, &dataId)) 
		!= ClipboardSuccess);


/* end the transaction */
  while((result = XmClipboardEndCopy(XtDisplay(topWidget), XtWindow(topWidget),
	itemId)) != ClipboardSuccess);

}


void clipboardGet(widget,topWidget,event)
  Widget widget, topWidget;
  XEvent *event;
{
  char string[CLIP_BUFFER_SIZE];

  unsigned long length;
  long id;
  int result;
  Boolean done;
  int loopCount;


  if (event->xbutton.button != BUTTON_NUMBER) return;

/* retrieve current contents of the clipboard */

  done = FALSE;
  loopCount = 0;

  while( !done ) {

    result = XmClipboardRetrieve(XtDisplay(topWidget),XtWindow(topWidget),
	"STRING", string, CLIP_BUFFER_SIZE, &length, &id);
    string[length] = '\0';

/* check the status return, loop if locked... */

    switch(result) {
	case ClipboardSuccess:	
		XmTextSetString(widget,string);
		done = TRUE;
		break;
	
	case ClipboardTruncate:	
		fprintf(stderr,
		   "clipboardGet: Insufficient space for clipboard data\n");
		done = TRUE;
		break;

	case ClipboardNoData:	
		done = TRUE;
		break;

	case ClipboardLocked:	
		loopCount++;
		if (loopCount > MAX_CLIPBOARD_TRIES) {
		    fprintf(stderr, "clipboardGet: %d %s %s\n", 
			MAX_CLIPBOARD_TRIES,
			"attempts to read from", 
			"the clipboard have failed - aborting");
		    done = TRUE;
		}
		break;

    }
  }

}
