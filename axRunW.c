/* axRunW.c */

/************************DESCRIPTION***********************************
  This file contains all the routines related to the iconlike runtime window
  and silence buttons.  Functions include display, unmapping, remapping of main
  window, alarm beeping, and blinking of control button.
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#define DEBUG_CALLBACKS 0

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/Form.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>

#include "alh.h"
#include "axArea.h"
#include "ax.h"

/* prototypes for static routines */
static void unmapwindow_callback(Widget w,Widget main,
XmAnyCallbackStruct *call_data);
static void remapwindow_callback(Widget w,Widget main,
XmAnyCallbackStruct *call_data);
static void alChangeOpenCloseButtonToRemap(XtPointer main);
static void alChangeOpenCloseButtonToUnmap(XtPointer main);
static void alChangeOpenCloseButtonToOpen(XtPointer main);
static void alLoadFont(char *fontname,XFontStruct **font_info);
static void axExit_callback(Widget w,ALINK *area,
XmAnyCallbackStruct *call_data);
static void axExitArea_callback(Widget w,ALINK *area,
XmAnyCallbackStruct *call_data);
static void blinking(XtPointer cd, XtIntervalId *id);
static void silenceOneHourReset(void *area);

/*   global variables */
XmStringCharSet charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
char *fontname = "fixed";
extern struct setup psetup;
extern int DEBUG;
extern char * alarmSeverityString[];
extern char * alarmStatusString[];

/* some globals for blink Data */
extern Display *display;
Widget blinkButton;
Widget blinkToplevel; /* Albert1 for locking status marking*/
Pixel  blinkPixel;

XtIntervalId blinkTimeoutId = (XtIntervalId)0;
unsigned long blinkDelay = 1000;     /* ms */
int blinkCOUNT=0;

Dimension char_width;
XmFontList fontlist;
XFontStruct *font_info;

Pixel foreground, background;


/*** the following defines the mapping from alarm severity to color;
      as such, the "alarm.h" and "alarmString.h" files should be consulted
      and the code below made consistent with those definitions ***/

/*** GTA alarm definitions &  APS alarm definitions 
#ifdef GTA
char *bg_color[] = {"grey","yellow","red",};
char *bg_char[] = {" ", "Y", "R",};
#else
char *bg_color[] = {"lightblue","green","yellow","red","white","grey"};
char *bg_char[] = {" ", "I", "Y", "R", "W" };
#endif
***/

char *bg_color[] = {
	"lightblue","yellow","red","white","grey"};
char *bg_char[] = {
	" ", "Y", "R", "V" };

Pixel bg_pixel[ALARM_NSEV];

/* and so that channels and groups are distinguishable... */
char *channel_bg_color = "lightblue";
Pixel channel_bg_pixel;


/******************************************************
  COLOR
******************************************************/
static unsigned long COLOR(Widget w,char *name)
{
	XColor  color;
	Colormap cmap;

	cmap = DefaultColormap(XtDisplay(w),DefaultScreen(XtDisplay(w)));
	color.pixel = 0;
	XParseColor(XtDisplay(w), cmap, name, &color);
	XAllocColor(XtDisplay(w), cmap, &color);

	return(color.pixel);
}

/******************************************************
  Update runtime window
******************************************************/
void icon_update()
{

#if DEBUG_CALLBACKS
	printf("icon_update\n");
#endif

#if  XmVersion && XmVersion >= 1002
	XmChangeColor(blinkButton,bg_pixel[0]);
#else
	XtVaSetValues(blinkButton,XmNbackground,bg_pixel[0],NULL);
#endif

	/* Remove any existing timer */
	if (blinkTimeoutId) {
		XtRemoveTimeOut(blinkTimeoutId);
		blinkTimeoutId = NULL;
	}

	/* Restart the timer */
	blinkTimeoutId = XtAppAddTimeOut(appContext,blinkDelay,blinking,NULL);
}

/******************************************************
  alChangeOpenCloseButtonToRemap
******************************************************/
static void alChangeOpenCloseButtonToRemap(XtPointer main)
{
	XtRemoveCallback(blinkButton,XmNactivateCallback,
	    (XtCallbackProc)unmapwindow_callback,main);
	XtAddCallback(blinkButton,XmNactivateCallback,
	    (XtCallbackProc)remapwindow_callback,main);
}

/******************************************************
  alChangeOpenCloseButtonToUnmap
******************************************************/
static void alChangeOpenCloseButtonToUnmap(XtPointer main)
{
	XtRemoveCallback(blinkButton,XmNactivateCallback,
	    (XtCallbackProc)remapwindow_callback,main);
	XtAddCallback(blinkButton,XmNactivateCallback,
	    (XtCallbackProc)unmapwindow_callback,main);
}

/******************************************************
  alChangeOpenCloseButtonToOpen
******************************************************/
static void alChangeOpenCloseButtonToOpen(XtPointer main)
{
	XtRemoveCallback(blinkButton,XmNactivateCallback,
	    (XtCallbackProc)unmapwindow_callback,main);
	XtRemoveCallback(blinkButton,XmNactivateCallback,
	    (XtCallbackProc)remapwindow_callback,main);
	XtAddCallback(blinkButton,XmNactivateCallback,
	    (XtCallbackProc)createMainWindow_callback,main);
}

/******************************************************
  alLoadFont
******************************************************/
static void alLoadFont(char *fontname,XFontStruct **font_info)
{
	if ((*font_info = XLoadQueryFont(display,fontname)) == NULL) {
		*font_info = XLoadQueryFont(display,"fixed");
	}
}


/********************************************************
   trapExtraneousWarningsHandler
*******************************************************/
XtErrorMsgHandler trapExtraneousWarningsHandler(String message)
{
	if (message && *message) {
		if (!strcmp(message,"Attempt to remove non-existant passive grab"))
			return 0;
			else
			(void)fprintf(stderr,"Warning: %s\n",message);
	}
	return 0;
}

/*****************************************************
 Setup pixel and font data
*****************************************************/
void pixelData(Widget iconBoard,Widget bButton)
{
	Arg args[10];
	int n;
	XmFontList fontlist;
	XmString mstring;


	/*
	 	* get bg color pixel
	  	*/

	for (n=1;n<ALARM_NSEV;n++)
		bg_pixel[n] = COLOR(iconBoard,bg_color[n]);

	channel_bg_pixel = COLOR(iconBoard,channel_bg_color);

	/*
	 	* retrieve the colors of the iconBoard
	 	*/
	n= 0;
	XtSetArg(args[n], XmNforeground, &foreground); 
	n++;
	XtSetArg(args[n], XmNbackground, &background); 
	n++;
	XtGetValues(iconBoard,args,n);
	bg_pixel[0] = background;


	/*
	 	* get string width in pixel
	 	*/
	if (bButton){
		n=0;
		XtSetArg(args[n], XmNfontList, &fontlist); 
		n++;
		XtGetValues(bButton,args,n);
		mstring=XmStringCreateLtoR("W",charset);
		char_width = XmStringWidth(fontlist,mstring);
		XmStringFree(mstring);
	}


	/*
	 	* set other blink data structure elements
	 	*/
	if (psetup.highestUnackSevr > 0)
		blinkPixel = bg_pixel[psetup.highestUnackSevr];
		else
		blinkPixel = bg_pixel[psetup.highestSevr];

}

/******************************************************
  createRuntimeWindow
******************************************************/
void createRuntimeWindow(ALINK *area)
{
	char   *alhTitle={
		"Alarm Handler"	};
	XmString str;
	char   *app_name;

	if (!area->runtimeToplevel){
		/*
		           * create toplevel setup window
		           */
		app_name = (char*) calloc(1,strlen(programName)+5);
		strcpy(app_name, programName);
		strcat(app_name, "-run");

		area->runtimeToplevel = XtAppCreateShell( app_name, programName,
		    applicationShellWidgetClass, display, NULL, 0);

		blinkToplevel = area->runtimeToplevel;  /* Albert1 for locking status marking*/
		free(app_name);

		XtVaSetValues(area->runtimeToplevel, XmNtitle, alhTitle, NULL);

		/*
		           create bulletin board widget
		          */
		area->runtimeForm = XtVaCreateManagedWidget("bulletinBoard1",
		    xmFormWidgetClass, area->runtimeToplevel,
		    XmNborderWidth, (Dimension)1,
		    XmNmarginWidth, (short)5,
		    XmNmarginHeight, (short)5,
		    XmNresizePolicy, XmRESIZE_NONE,
		    NULL);

		if (!ALH_pixmap) axMakePixmap(area->runtimeForm);

		XtVaSetValues(area->runtimeToplevel,
		    XmNiconPixmap,          ALH_pixmap,
		    XmNiconName,            area->blinkString,
		    XmNallowShellResize,    TRUE,
		    XmNuserData,            (XtPointer)area,
		    NULL);

		/*
		           * Modify the window manager menu "close" callback
		           */
		{
			Atom         WM_DELETE_WINDOW;
			XtVaSetValues(area->runtimeToplevel,
			    XmNdeleteResponse,       XmDO_NOTHING,
			    NULL);
			WM_DELETE_WINDOW = XmInternAtom(XtDisplay(area->runtimeToplevel),
			    "WM_DELETE_WINDOW", False);
			if (programId == ALH)
				XmAddWMProtocolCallback(area->runtimeToplevel,WM_DELETE_WINDOW,
				    (XtCallbackProc) axExit_callback, (XtPointer)area );
			else XmAddWMProtocolCallback(area->runtimeToplevel,WM_DELETE_WINDOW,
			    (XtCallbackProc) axExitArea_callback, (XtPointer)area );

		}

		/*
		          create control button  
		          */
		str  = XmStringCreateLtoR( "--- No config file specified. ---",
		    XmSTRING_DEFAULT_CHARSET);
		area->blinkButton = XtVaCreateManagedWidget("iconButton",
		    xmPushButtonWidgetClass,  area->runtimeForm,
		    XmNalignment,              XmALIGNMENT_CENTER,
		    XmNtopAttachment,          XmATTACH_FORM,
		    XmNbottomAttachment,       XmATTACH_FORM,
		    XmNleftAttachment,         XmATTACH_FORM,
		    XmNrightAttachment,        XmATTACH_FORM,
		    /*
		               XmNrecomputeSize,          False,
		     */
		XmNactivateCallback,       NULL,
		    XmNuserData,               (XtPointer)area,
		    XmNlabelString,            str,
		    NULL);
		XmStringFree(str);

		XtAddCallback(area->blinkButton,XmNactivateCallback,
		    (XtCallbackProc)createMainWindow_callback, area);

		blinkButton = area->blinkButton;

		alLoadFont(fontname,&font_info);

		XtRealizeWidget(area->runtimeToplevel);

	} else {
		XMapWindow(XtDisplay(area->runtimeToplevel),
		    XtWindow(area->runtimeToplevel));
	}

	pixelData(area->runtimeForm,area->blinkButton);

	/*  update blinkButton string */
	str = XmStringCreateSimple(area->blinkString);
	XtVaSetValues(area->blinkButton,
	    XmNlabelString,         str,
	    NULL);
	XmStringFree(str);

	/* reinitialize silence beep */
	silenceCurrentReset(area);
	silenceOneHourReset(area);

	changeBeepSeverityText(area);

	icon_update();
}

/******************************************************
  createMainWindow_callback
******************************************************/
void createMainWindow_callback(Widget w,ALINK *area,
XmAnyCallbackStruct *call_data)
{
	if (area->toplevel == 0){
		area->mapped = FALSE;
		createMainWindowWidgets(area);
		XtRealizeWidget(area->toplevel);
	}
	if (area->mapped == FALSE){
		XMapWindow(XtDisplay(area->toplevel),XtWindow(area->toplevel));
		area->mapped = TRUE;
		redraw(area->treeWindow,0);

		/* mark first line as treeWindow selection */
		defaultTreeSelection(area);
	}
	else {
		XRaiseWindow(XtDisplay(area->toplevel), XtWindow(area->toplevel));
		redraw(area->treeWindow,0);
	}

	/* remove this callback from callback list */
	/*
	     XtRemoveCallback(w,XmNactivateCallback,(XtCallbackProc)createMainWindow_callback,area);
	     XtAddCallback(w,XmNactivateCallback,(XtCallbackProc)unmapwindow_callback,area->toplevel);
	*/
}

/******************************************************
  Initiate runtime window blinking 
******************************************************/
static void blinking(XtPointer cd, XtIntervalId *id)
{
	Display *displayBB;
	static Boolean blinking2State = FALSE;
	int restart=0;


#if DEBUG_CALLBACKS
	{
		static int n=0;

		printf("blinking: n=%d blinking2State=%d"
		    " psetup.silenceForever=%d psetup.silenceCurrent=%d\n"
		    " psetup.highestSev=%d blinkPixel=%d bg_pixel[0]=%d\n"
		    " psetup.highestUnackSevr=%d psetup.beepSevr=%d\n",
		    n++,blinking2State,psetup.silenceForever,psetup.silenceCurrent,
		    psetup.highestSevr,blinkPixel,bg_pixel[0],
		    psetup.highestUnackSevr,psetup.beepSevr);
	}
#endif

	displayBB = XtDisplay(blinkButton);

	if (!blinking2State) {

		if (psetup.highestSevr > 0 || blinkPixel != bg_pixel[0]) {

			if (psetup.highestUnackSevr > 0)
				blinkPixel = bg_pixel[psetup.highestUnackSevr];
				else
				blinkPixel = bg_pixel[psetup.highestSevr];

#if  XmVersion && XmVersion >= 1002
			XmChangeColor(blinkButton,blinkPixel);
#else
			XtVaSetValues(blinkButton,XmNbackground,blinkPixel,NULL);
#endif
			restart=1;
		}
		if (!psetup.silenceForever &&
		    !psetup.silenceOneHour && 
		    !psetup.silenceCurrent && 
		    (psetup.highestUnackSevr >= psetup.beepSevr)) {
			XBell(displayBB,0);
			XRaiseWindow(displayBB,XtWindow(XtParent(XtParent(blinkButton))));
			restart=1;
		}
		restart=1;     /* This all needs to be fixed */
		if(restart) {
			blinkTimeoutId = XtAppAddTimeOut(appContext,blinkDelay,
			    blinking,NULL);
		}
		blinking2State = TRUE;

	} else {

		if (psetup.highestUnackSevr > 0) {
#if  XmVersion && XmVersion >= 1002
			XmChangeColor(blinkButton,bg_pixel[0]);
#else
			XtVaSetValues(blinkButton,XmNbackground,bg_pixel[0],NULL);
#endif
		}
		blinkTimeoutId = XtAppAddTimeOut(appContext,blinkDelay,
		    blinking,NULL);
		blinking2State = FALSE;
	}

	XFlush(displayBB);

	if ( DEBUG == 1 ){
		blinkCOUNT++;
		if ((blinkCOUNT % 1000)==0) printf("blink times = %d \n",blinkCOUNT);
	}
}

/******************************************************
 axExit_callback
******************************************************/
static void axExit_callback(Widget w,ALINK *area,
XmAnyCallbackStruct *call_data)
{
	createActionDialog(area->runtimeToplevel,XmDIALOG_WARNING,
	    "Exit Alarm Handler?",(XtCallbackProc)exit_quit,
	    (XtPointer)area, (XtPointer)area);
}

/******************************************************
 axExitArea_callback
******************************************************/
static void axExitArea_callback(Widget w,ALINK *area,
XmAnyCallbackStruct *call_data)
{
	SNODE *proot;

	alLogExit();
	XUnmapWindow(XtDisplay(area->runtimeToplevel),
	    XtWindow(area->runtimeToplevel));
	if (area->toplevel) {
		XUnmapWindow(XtDisplay(area->toplevel),XtWindow(area->toplevel));
	}
	area->mapped = FALSE;
	area->managed = FALSE;
	if (area->programId == ALH)  alCaCancel((SLIST *)area->pmainGroup);

	/* Delete the current config */
	if (area->pmainGroup){
		proot = sllFirst(area->pmainGroup);
		if (proot) alDeleteGroup((GLINK *)proot);
	}

	area->pmainGroup = NULL;
}

/******************************************************
 unmapwindow_callback
******************************************************/
static void unmapwindow_callback(Widget w,Widget main,XmAnyCallbackStruct *call_data)
{
	ALINK *area;

	XtVaGetValues(w, XmNuserData, &area, NULL);

	XUnmapWindow(XtDisplay(main),XtWindow(main));

	alChangeOpenCloseButtonToRemap((XtPointer)main);

	area->mapped = FALSE;
}

/******************************************************
 remapwindow_callback
******************************************************/
static void remapwindow_callback(Widget w,Widget main,XmAnyCallbackStruct *call_data)
{
	ALINK *area;

	if (main) {

		XtVaGetValues(w, XmNuserData, &area, NULL);

		XMapWindow(XtDisplay(main),XtWindow(main));

		alChangeOpenCloseButtonToUnmap((XtPointer)main);

		area->mapped = TRUE;
	}
	else {
		alChangeOpenCloseButtonToOpen((XtPointer)main);
	}
}

/***********************************************
 Reset silenceCurrent to on state
************************************************/
void silenceCurrentReset(void *area)
{
	if (psetup.silenceCurrent) {
		psetup.silenceCurrent = FALSE;
		if (((ALINK*)area)->silenceCurrent) {
			XmToggleButtonGadgetSetState(((ALINK*)area)->silenceCurrent,
			    FALSE,FALSE);
		}
	}
}

/***********************************************
 reset silenceOneHour
************************************************/
static void silenceOneHourReset(void *area)
{
	if (psetup.silenceOneHour) {
		XmToggleButtonGadgetSetState(((ALINK*)area)->silenceOneHour,FALSE,FALSE);
		silenceOneHour_callback(((ALINK*)area)->silenceOneHour,area,NULL);
	}
}

/***************************************************
 silenceCurrent button toggle callback
****************************************************/
void silenceCurrent_callback(Widget w,int userdata,
XmAnyCallbackStruct *call_data)
{
	psetup.silenceCurrent = psetup.silenceCurrent?FALSE:TRUE;
	if (psetup.silenceCurrent)
		alLogOpMod("Silence Current set to TRUE");
		else
		alLogOpMod("Silence Current set to FALSE");
}

/***************************************************
 silenceOneHour button toggle callback
****************************************************/
void silenceOneHour_callback(Widget w,void * area,
XmAnyCallbackStruct *call_data)
{
	static XtIntervalId intervalId = NULL;
	int seconds = 3600;

	psetup.silenceOneHour = psetup.silenceOneHour?FALSE:TRUE;
	if (psetup.silenceOneHour) {
		intervalId = XtAppAddTimeOut(appContext,
		    (unsigned long)(1000*seconds),
		    (XtTimerCallbackProc)silenceOneHourReset,
		    (XtPointer)area);
		alLogOpMod("Silence One Hour set to TRUE");
	} else {
		if (intervalId) {
			XtRemoveTimeOut(intervalId);
			intervalId = NULL;
		}
		alLogOpMod("Silence One Hour set to FALSE");
	}
}

/***************************************************
 silenceForeverChangeState
****************************************************/
void silenceForeverChangeState()
{
	psetup.silenceForever = psetup.silenceForever?FALSE:TRUE;
	if (psetup.silenceForever)
		alLogOpMod("Silence Forever set to TRUE");
		else
		alLogOpMod("Silence Forever set to FALSE");
}

