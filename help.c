/*
 $Log$
 Revision 1.5  1998/05/12 18:22:49  evans
 Initial changes for WIN32.

 Revision 1.4  1995/10/20 16:50:43  jba
 Modified Action menus and Action windows
 Renamed ALARMCOMMAND to SEVRCOMMAND
 Added STATCOMMAND facility
 Added ALIAS facility
 Added ALARMCOUNTFILTER facility
 Make a few bug fixes.

 * Revision 1.3  1995/06/22  19:46:22  jba
 * Put group/channel alias in guidance window title bar.
 *
 * Revision 1.2  1994/06/22  21:17:36  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)help.c	1.12\t10/1/93";

/***************************************************
*   help.c 
*
*This files contains the  *PUBLIC*  routines for creating a
*dialog message box. The created message dialog consists 
*a message text widget and two push buttons-- 'Ok', and 
*'Help'.
*
*The presence of 'Help' button depends on the help textaul
*string array.  A single null string corresponds to a 'Help'
*button and causes remaining text shown on next page. Two 
*consecutive null strings indicates the end of help text.  
*
********************************************************/
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
# include <stdio.h>
/* Hummingbird extra functions including lprintf
 *   Needs to be included after Intrinsic.h for Exceed 5
 *   (Intrinsic.h is included in xtParams.h) */
# include <X11/XlibXtra.h>
/* This is done in Exceed 6 but not in Exceed 5
 *   Need it to define printf as lprintf for Windows
 *   (as opposed to Console) apps */
# ifdef _WINDOWS
#  ifndef printf
#   define printf lprintf
#  endif
# endif
#endif /* #ifdef WIN32 */

#include <ax.h>

static char *setuphelpdata[] = { 
        "Setup...",
	"View...",
	"Help...",
	"Exit...",
 	"Silence Current...",
        "Silence Forever...",
        "Beep Condition...",
	"Open/Close Subwindows..." };

static char *silence_current_help_str[] = {
        "--SILENCE CURRENT--",
        "The Silence Current button toggles current alarm beeping  on / off.",
	"Occurance of a new alarm will turn on beeping again.", 
        "",""};

static char *silence_forever_help_str[] = {
        "--SILENCE FOREVER--",
        "The Silence Forever button toggles on / off the beeping", 
	"from current and future alarms.",
        "",""};

static char *beep_condition_help_str[] = {
	"--BEEP CONDITION--",
	"The Beep Condition consists three radio buttons, only one",
	" can be set to on. It determines the minimum condition for",
	"alarm beeping.",
	"","" };

static char *view_help_str[] = {
        "--VIEW--",
        "The View button provides a menu of choices for scroll window",
	"viewing of the  current configuration file, alarm log file,",
	"and operation log file.", 
        "",""};

static char *help_help_str[] = {
        "--HELP--",
        "The Help button from menubar let user to access the help ",
	"information about each push button in this window.",
	" ",
	"The Help button from pulldown help menu shows this message",
	"information.",
        "",""};

static char *exit_help_str[] = {
        "--EXIT--",
        "The Exit button  terminates the alarm handler.",
        " ",
        "The popup Ok button is used to confirm the termination.",
        "",""};

static char *open_close_help_str[] = {
        "--OPEN/CLOSE Group Windows",
        "The Open/Close button opens / closes the subgroup windows.",
        "",""};

static char *setup_help_str[] = {
        "--SETUP--",
        "The Setup button provides a menu of choices for changing",
	"the current default configuration file, alarm log file,",
	"operator modifications file or to save a new configuration ",
	"file. ",
	"","" };
 
static char *grouphelpdata[] = { 
        "Close...",
	"Options...",
	"Guidance...",
	"Help...",
 	"Ack Channel...",
        "Ack Group...",
	"Choose Channel...",
	"Choose Group..." };



static char *close_help_str[] = {
        "--CLOSE--",
	"The Close button closes an opened",
	"group window.",
	"","" };

static char *masks_help_str[] = {
        "--OPTIONS--",
        "The Options button provides a menu of choices for the display and",
        "modification of group or channel settings.",
        "",""};

static char *guidance_help_str[] = {
        "--GUIDANCE--",
        "The Guidance button opens a text information window ",
	"which provides guidance for Group or Channel alarms.",
        "",""};

static char *ackchan_help_str[] = {
        "--ACK CHAN--",
        "The channel Acknowledgement button is color coded",
        "to the highest extant alarm severity for this channel:",
	" ",
        "Yellow - Minor Alarm",
        "Red    - Major Alarm",
        "White  - Valid Alarm",
        " ",
        "A channel alarm is acknowledged by selecting this button.",
        "",""};

static char *ackgroup_help_str[] = {
        "--ACK GROUP--",
        "The group Acknowledgement button is color coded",
        "to the highest extant alarm severity in this group:",
	" ",
        "Yellow - Minor Alarm",
        "Red    - Major Alarm",
        "White  - Valid Alarm",
        " ",
        "A group alarm is acknowledged by selecting its acknowledgement",
        "button. All channels in the selected group are acknowledged.",
        "",""};

static char *choosechan_help_str[] = {
        "--CHOOSE CHAN--",
	"The Channel Name button provides a popup channel menu.",
	" ",
        "To choose the popup menu, first position the channel desired,",
	"then click the left mouse button.  The popup options will be",
	"posted.",
        " ",
        "The user should hold down the left button until the desired",
        "option of the submenu is selected.",
        "",""};

static char *choosegroup_help_str[] = {
        "--CHOOSE GROUP--",
        "The Group Name button opens a group's sub window.",
        " ",
        "A close window button is available on the subwindow menubar to",
        "close the sub window.",
        "",""};


#ifdef __STDC__

static void SetupHelpSelection( Widget w, int index);
static void GroupHelpSelection( Widget w, int index);

#else

static void SetupHelpSelection();
static void GroupHelpSelection();

#endif /*__STDC__*/

 


/*******************************************************
	delete an existing widget
*******************************************************/
void xs_ok_callback(w,client_data,call_data)
Widget w;
void *client_data;
XmAnyCallbackStruct *call_data;
{
	XtUnmanageChild(w);
	XtDestroyWidget(w);
}


/*************************************************************
 *  xs_str_array_to_xmstr() : convert char ** to
 *	compound string
 ************************************************************/
XmString xs_str_array_to_xmstr(cs,n)
char *cs[];
int n;
{
XmString xmstr;
int i;
/*
 * if the array is empty just return an empty string.
 */
if (n <= 0) 
	return(XmStringCreate("",XmSTRING_DEFAULT_CHARSET));

xmstr = (XmString) NULL;

for (i=0;i<n;i++) {
	if (i > 0)
		xmstr = XmStringConcat(xmstr, XmStringSeparatorCreate());
	xmstr = XmStringConcat(xmstr,
			XmStringCreate(cs[i],XmSTRING_DEFAULT_CHARSET));
	}
	return (xmstr);
}




/*************************************************************
 *  the help button call back function from a dialog widget
 ************************************************************/
void xs_help_callback(w,str,call_data)
Widget w;
char *str[];
void *call_data;
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
         XtSetArg(wargs[n], XmNtitle, buff); n++;
    }
	XtSetArg(wargs[n], XmNautoUnmanage, FALSE); n++;
	XtSetArg(wargs[n], XmNallowShellResize, FALSE); n++;
	dialog = XmCreateMessageDialog(w, "Help", wargs, n);
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
	XtSetArg(wargs[n], XmNmessageString, xmstr); n++;
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

/**************************************************
 create help menu for setup window
***************************************************/
void CreateGroupWindowHelpMenu(window_depth,w)
int window_depth;
  Widget w;
{
  Arg wargs[5];
  Widget menu,buttons[8];
  Widget button; 		/*cascadebutton */
  int i,n;

/* 
 * prepare Help pulldown menu here
 */

  menu = XmCreatePulldownMenu(w,"Help",NULL,0);

/*
 * add help buttons
 */
  n=0;
  XtSetArg(wargs[n], XmNsubMenuId, menu); n++;
  button = XtCreateManagedWidget("Help",xmCascadeButtonWidgetClass,
                        w, wargs, n);
        
  for (i=0;i<XtNumber(grouphelpdata); i++) {
        buttons[i] = XtCreateManagedWidget(grouphelpdata[i],
                        xmPushButtonWidgetClass, menu, NULL, 0);
        GroupHelpSelection(buttons[i],i);
  }

}



/**************************************************
 create help menu for setup window
***************************************************/
void CreateSetupWindowHelpMenu(w)
  Widget w;
{
  Arg wargs[5];
  Widget menu,buttons[7];
  Widget button; 		/*cascadebutton */
  int i;

/* 
 * prepare Help pulldown menu here
 */

  menu = XmCreatePulldownMenu(w,"Help",NULL,0);

/*
 * add help buttons
 */
  XtSetArg(wargs[0], XmNsubMenuId, menu);
  button = XtCreateManagedWidget("Help",xmCascadeButtonWidgetClass,
                        w, wargs, 1);
        
  for (i=0;i<XtNumber(setuphelpdata); i++) {
        buttons[i] = XtCreateManagedWidget(setuphelpdata[i],
                        xmPushButtonWidgetClass, menu, NULL, 0);
        SetupHelpSelection(buttons[i],i);
  }

}

/****************************************************
  setup help  selection callback lists
****************************************************/
static void SetupHelpSelection(w,index)
Widget w;
int index;
{

switch (index) {
        case 0:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,setup_help_str);
        break;

        case 1:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,view_help_str);
        break;

        case 2:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,help_help_str);
        break;

        case 3:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,exit_help_str);
        break;

        case 4:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,silence_current_help_str);
        break;

        case 5:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,silence_forever_help_str);
        break;

        case 6:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,beep_condition_help_str);
        break;

        case 7:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,open_close_help_str);
        break;

        }
}
        

/***************************************************
  guidance selection callback lists
***************************************************/
static void GroupHelpSelection(w,index)
Widget w;
int index;
{

switch (index) {
        case 0:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,close_help_str);
        break;

        case 1:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,masks_help_str);
        break;

        case 2:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,guidance_help_str);
        break;

        case 3:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,help_help_str);
        break;

        case 4:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,ackchan_help_str);
        break;

        case 5:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,ackgroup_help_str);
        break;

        case 6:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,choosechan_help_str);
        break;
        case 7:
        XtAddCallback(w,XmNactivateCallback,
                (XtCallbackProc)xs_help_callback,choosegroup_help_str);
        break;
        }
}
/******************************************************
  helpCallback
******************************************************/

void helpCallback(widget, item, cbs)
     Widget widget;
     int item;
     XmAnyCallbackStruct *cbs;
{

     createDialog(widget,XmDIALOG_INFORMATION,"Help is not available in this release."," ");
}

 
