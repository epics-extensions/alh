#ifndef INCclipboardOpsh
#define INCclipboardOpsh 1

static char *clipboardOpshSccsId = "@(#)clipboardOps.h	1.4\t12/15/93";


/***
 ***  routines (as event handlers) to allow a user to select a
 ***    push button label and paste it into a text field
 ***
 ***    30 Nov. 1990  (MDA)
 ***
 ***
 ***
 *** usage:
 ***  for a push button (whose label you want to pass)
 ***
 ***
 ***    XtAddEventHandler(pushButton,ButtonPressMask,FALSE,
 ***            clipboardPut,mainWidget);
 ***
 ***
 ***  for a text field (who should receive the text)
 ***
 ***    XtAddEventHandler(textField,ButtonPressMask,FALSE,
 ***            clipboardPut,mainWidget);
 ***
 ***
 ***  Note:  the last argument to the XtAddEventHandler() call
 ***    must the be same widget (since the clipboard compares
 ***    display and window parameters).  Also Note:  the widget
 ***    passed as topWidget (communication widget) must be managed
 ***    prior to these calls).
 ***
 ***/


#define CLIP_BUFFER_SIZE 80
#define BUTTON_NUMBER 2

#define MAX_CLIPBOARD_TRIES 50

extern void clipboardPut();
extern void clipboardGet();
 
#endif /* INCclipboardOpsh */

