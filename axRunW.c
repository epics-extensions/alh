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
/* axRunW.c */

#include <stdlib.h>

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

/************************DESCRIPTION***********************************
  This file contains all the routines related to the iconlike runtime window
  and silence buttons.  Functions include display, unmapping, remapping of main
  window, alarm beeping, and blinking of control button.
**********************************************************************/

#define BLINK_DELAY 1000     /* ms */
static XtIntervalId blinkTimeoutId = (XtIntervalId)0;
static char *bg_color[] = {"lightblue","yellow","red","white","white","grey"};

static char *channel_bg_color = "lightblue";
static char *silenced_bg_color = "lightpink";


/* global variabless */
Widget blinkToplevel; /* Albert1 for locking status marking*/
extern Display *display;
extern struct setup psetup;
extern char *programName;
extern Pixmap ALH_pixmap;
Pixel bg_pixel[ALH_ALARM_NSEV];
Pixel channel_bg_pixel;
Pixel silenced_bg_pixel;
const char *bg_char[] = {" ", "Y", "R", "V", "E"," " };

/* forward declarations */
static void axExit_callback(Widget w,ALINK *area,XmAnyCallbackStruct *call_data);
static void axExitArea_callback(Widget w,ALINK *area,XmAnyCallbackStruct *call_data);
static void blinking(XtPointer pointer, XtIntervalId *id);
static void createMainWindow_callback(Widget w,ALINK *area,XmAnyCallbackStruct *call_data);
static void icon_update(Widget blinkButton);
static void silenceOneHourReset(void *area);
static void changeTreeColor(Widget widget,Pixel color);
char *Strncat(
  char *dest,
  const char *src,
  int max );

static void topWinEventHandler (
  Widget w,
  XtPointer client,
  XEvent *e,
  Boolean *continueToDispatch ) {

XConfigureEvent *ce;
ALINK *area;

  area = (ALINK *) client;

  *continueToDispatch = True;

  if ( e->type == ConfigureNotify ) {

    ce = (XConfigureEvent *) e;
    
    /*printf( "ce->width = %-d\n", ce->width);*/
    /*printf( "ce->height = %-d\n", ce->height);*/
    
    area->w = ce->width;
    area->h = ce->height;

  }

}

/******************************************************
  createRuntimeWindow
******************************************************/
void createRuntimeWindow(ALINK *area)
{

	char buff[LINEMESSAGE_SIZE], labelStr[80+1];
	MASK mask;
	int i;
	const char   *alhTitle={"Alarm Handler"};
	XmString str;
	XFontStruct *btnFont;
	XmFontListEntry fontListEntry;
	XmFontList fontList;
	char *envPtr;

	if (!area->runtimeToplevel){
		/* create toplevel setup window */
		area->runtimeToplevel = XtAppCreateShell( "alh-run", programName,
		    applicationShellWidgetClass, display, NULL, 0);

		blinkToplevel = area->runtimeToplevel;  /* Albert1 for locking status marking*/

		XtVaSetValues(area->runtimeToplevel, XmNtitle, alhTitle, NULL);

		/* create bulletin board widget */
		area->runtimeForm = XtVaCreateManagedWidget("bulletinBoard1",
		    xmFormWidgetClass, area->runtimeToplevel,
		    XmNborderWidth, (Dimension)1,
		    XmNmarginWidth, (short)5,
		    XmNmarginHeight, (short)5,
		    XmNresizePolicy, XmRESIZE_NONE,
		    NULL);

		if (!ALH_pixmap) axMakePixmap(area->runtimeForm);

		XtVaSetValues(area->runtimeToplevel,
#ifndef WIN32
		    XmNiconPixmap,          ALH_pixmap,
#endif
		    XmNiconName,            area->blinkString,
		    XmNallowShellResize,    TRUE,
		    XmNuserData,            (XtPointer)area,
		    NULL);

		/* Modify the window manager menu "close" callback */
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

		envPtr = getenv( "ALHMAINFONT" );
		if ( envPtr ) {
		  btnFont = XLoadQueryFont(display,envPtr);
		  fontListEntry = XmFontListEntryCreate( "btnfont", XmFONT_IS_FONT, btnFont );
		  fontList = XmFontListAppendEntry( NULL, fontListEntry );
		}
		else {
		  fontList = NULL;
		}
		
		/* create control button */
		str  = XmStringCreateLtoR( "--- No config file specified. ---",
		    XmSTRING_DEFAULT_CHARSET);
		area->blinkButton = XtVaCreateManagedWidget("iconButton",
		    xmPushButtonWidgetClass,  area->runtimeForm,
		    XmNalignment,              XmALIGNMENT_CENTER,
		    XmNtopAttachment,          XmATTACH_FORM,
		    XmNbottomAttachment,       XmATTACH_FORM,
		    XmNleftAttachment,         XmATTACH_FORM,
		    XmNrightAttachment,        XmATTACH_FORM,
		XmNactivateCallback,       NULL,
		    XmNuserData,               (XtPointer)area,
		    XmNlabelString,            str,
		    XmNfontList,               fontList,
		    NULL);
		XmStringFree(str);

		XtAddCallback(area->blinkButton,XmNactivateCallback,
		    (XtCallbackProc)createMainWindow_callback, area);

		XtRealizeWidget(area->runtimeToplevel);
		
		XtAddEventHandler( area->runtimeToplevel, StructureNotifyMask, False,
		 topWinEventHandler, (XtPointer) area );
		
		
	} else {
		XMapWindow(XtDisplay(area->runtimeToplevel),
		    XtWindow(area->runtimeToplevel));
	}

	pixelData(area->runtimeForm);

	/*  update blinkButton string */
	str = XmStringCreateSimple(area->blinkString);
	XtVaSetValues(area->blinkButton,
	    XmNlabelString,         str,
	    NULL);
	XmStringFree(str);

	/* reinitialize silence beep */
	silenceCurrentReset(area);
	silenceOneHourReset(area);

	icon_update(area->blinkButton);
	
	mask.Cancel = mask.Disable = mask.Ack = mask.AckT = mask.Log = 0;
	for ( i=0; i<ALARM_NMASK; i++ ) {
	  if ( i == ALARMCANCEL ) {
	    mask.Cancel = ( area->pmainGroup->p1stgroup->pgroupData->mask[i] > 0 );
	  }
	  else if ( i == ALARMDISABLE ) {
	    mask.Disable = ( area->pmainGroup->p1stgroup->pgroupData->mask[i] > 0 );
	  }
	  else if ( i == ALARMACK ) {
	    mask.Ack = ( area->pmainGroup->p1stgroup->pgroupData->mask[i] > 0 );
	  }
	  else if ( i == ALARMACKT ) {
	    mask.AckT = ( area->pmainGroup->p1stgroup->pgroupData->mask[i] > 0 );
	  }
	  else if ( i == ALARMLOG ) {
	    mask.Log = ( area->pmainGroup->p1stgroup->pgroupData->mask[i] > 0 );
	  }
	}
	
	alGetMaskString(mask,buff);
	
        strncpy( labelStr, area->blinkString, 80 );
	labelStr[80] = 0;

	if ( strcmp( buff, "-----" ) != 0 ) {

	  Strncat( labelStr, "  <", 80 );
	  labelStr[80] = 0;
	
	  Strncat( labelStr, buff, 80 );
	  labelStr[80] = 0;

          Strncat( labelStr, ">", 80 );
	  labelStr[80] = 0;

	}

	str = XmStringCreateSimple(labelStr);
	XtVaSetValues(area->blinkButton,
	    XmNlabelString,         str,
	    NULL);
	XmStringFree(str);
		
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

	alLogOpModMessage(0,0,"Setup---Exit");
	XUnmapWindow(XtDisplay(area->runtimeToplevel),
	    XtWindow(area->runtimeToplevel));
	if (area->toplevel) {
		XUnmapWindow(XtDisplay(area->toplevel),XtWindow(area->toplevel));
	}
	area->mapped = FALSE;
	area->managed = FALSE;
	if (area->programId == ALH)  alCaCancel(area->pmainGroup);

	/* Delete the current config */
	if (area->pmainGroup){
		proot = sllFirst(area->pmainGroup);
		if (proot) alDeleteGroup((GLINK *)proot);
	}
	alHeartbeatPVRemove(area->pmainGroup);
	free(area->pmainGroup);
	area->pmainGroup = NULL;
}

/******************************************************
  createMainWindow_callback
******************************************************/
static void createMainWindow_callback(Widget w,ALINK *area,
	XmAnyCallbackStruct *call_data)
{
	showMainWindow(area);
}

/******************************************************
  Update runtime window
******************************************************/
static void icon_update(Widget blinkButton)
{

	XmChangeColor(blinkButton,bg_pixel[0]);

	/* Remove any existing timer */
	if (blinkTimeoutId) {
		XtRemoveTimeOut(blinkTimeoutId);
		blinkTimeoutId = 0;
	}

	/* Restart the timer */
	blinkTimeoutId = XtAppAddTimeOut(appContext,BLINK_DELAY,blinking,(XtPointer)blinkButton);
}

/******************************************************
  Initiate runtime window blinking 
******************************************************/
static void blinking(XtPointer pointer, XtIntervalId *id)
{
	Widget blinkButton = (Widget)pointer;
	Display *displayBB;
	static Pixel  blinkPixel = 0;
	static Boolean blinking2State = FALSE;

	if (!blinkPixel) blinkPixel =  bg_pixel[0];
	displayBB = XtDisplay(blinkButton);

	if (!blinking2State) {

		if (psetup.highestSevr > 0 ||
			 psetup.highestUnackSevr > 0 ||
			 blinkPixel != bg_pixel[0]) {

			if (psetup.highestUnackSevr > 0)
				blinkPixel = bg_pixel[psetup.highestUnackSevr];
			else
				blinkPixel = bg_pixel[psetup.highestSevr];

			XmChangeColor(blinkButton,blinkPixel);
		}
		if (!psetup.silenceForever &&
		    !psetup.silenceOneHour && 
		    !psetup.silenceCurrent && 
		    (psetup.highestUnackBeepSevr >= psetup.beepSevr)) {

			alBeep(displayBB);

			XRaiseWindow(displayBB,XtWindow(XtParent(XtParent(blinkButton))));
		}
		blinkTimeoutId = XtAppAddTimeOut(appContext,BLINK_DELAY,
		    blinking,blinkButton);
		blinking2State = TRUE;

	} else {
		if (psetup.highestUnackSevr > 0) {
			XmChangeColor(blinkButton,bg_pixel[0]);
		}
		blinkTimeoutId = XtAppAddTimeOut(appContext,BLINK_DELAY,
		    blinking,blinkButton);
		blinking2State = FALSE;
	}
	XFlush(displayBB);
}

/***********************************************
 Reset silenceCurrentReset  (to on state)
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
 reset silenceOneHourReset
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
void silenceCurrent_callback(Widget w,ALINK *area,
XmAnyCallbackStruct *call_data)
{
	psetup.silenceCurrent = psetup.silenceCurrent?FALSE:TRUE;
	if (psetup.silenceCurrent)
		alLogOpModMessage(0,0,"Silence Current set to TRUE");
	else
		alLogOpModMessage(0,0,"Silence Current set to FALSE");
}

/***************************************************
 silenceOneHour button toggle callback
****************************************************/
void silenceOneHour_callback(Widget w,ALINK* area,
XmAnyCallbackStruct *call_data)
{
	static XtIntervalId intervalId = 0;
	int seconds = 3600;
    Boolean getState;

	psetup.silenceOneHour = psetup.silenceOneHour?FALSE:TRUE;
	if (psetup.silenceOneHour) {
		intervalId = XtAppAddTimeOut(appContext,
		    (unsigned long)(1000*seconds),
		    (XtTimerCallbackProc)silenceOneHourReset,
		    (XtPointer)area);
		XmChangeColor(area->messageArea,silenced_bg_pixel);
		changeTreeColor(area->treeWindowForm,silenced_bg_pixel);
		changeTreeColor(area->groupWindowForm,silenced_bg_pixel);
		changeTreeColor(area->scale,silenced_bg_pixel);
		alLogOpModMessage(0,0,"Silence One Hour set to TRUE");
	} else {
		if (intervalId) {
			XtRemoveTimeOut(intervalId);
			intervalId = 0;
		}
		XmChangeColor(area->messageArea,bg_pixel[0]);
		changeTreeColor(area->treeWindowForm,bg_pixel[0]);
		changeTreeColor(area->groupWindowForm,bg_pixel[0]);
		changeTreeColor(area->scale,bg_pixel[0]);
		alLogOpModMessage(0,0,"Silence One Hour set to FALSE");
	}
}

/***************************************************
 changeTreeColor
****************************************************/
static void changeTreeColor(Widget widget,Pixel color)
{
    int i;
    Widget *children;
    Cardinal numChildren;
	char *name;

	if (!widget || !color) return;
	XtVaGetValues(widget,XmNnumChildren,&numChildren, NULL);
	if (numChildren > 0) {
		XtVaGetValues(widget,XmNchildren,&children, NULL);
		for(i=0; i < (int)numChildren; i++) {
			changeTreeColor(children[i],color);
		}
	}
	name = XtName(widget);
	if( !name || 
		( strcmp(name,"ack") &&
		strcmp(name,"sevr") &&
		strcmp(name,"pushButtonName")) ) {
			XmChangeColor(widget,color);
	}
}


/***************************************************
 silenceForeverChangeState
****************************************************/
void silenceForeverChangeState(ALINK *area)
{
	psetup.silenceForever = psetup.silenceForever?FALSE:TRUE;
	if (psetup.silenceForever)
		alLogOpModMessage(0,0,"Silence Forever set to TRUE");
	else
		alLogOpModMessage(0,0,"Silence Forever set to FALSE");
	changeSilenceForeverText(area);
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

/******************************************************
  COLOR
******************************************************/
static unsigned long COLOR(Display *dsply,char *name)
{
	XColor  color;
	Colormap cmap;

	cmap = DefaultColormap(dsply,DefaultScreen(dsply));
	color.pixel = 0;
	XParseColor(dsply, cmap, name, &color);
	XAllocColor(dsply, cmap, &color);
	return(color.pixel);
}

/*****************************************************
 Setup pixel and font data
*****************************************************/
void pixelData(Widget iconBoard)
{
	Display *dsply;
	int n;

	/* get bg color pixel */
	dsply = XtDisplay(iconBoard);
	for (n=1;n<ALH_ALARM_NSEV;n++)
		bg_pixel[n] = COLOR(dsply,bg_color[n]);

	channel_bg_pixel = COLOR(dsply,channel_bg_color);
    silenced_bg_pixel = COLOR(dsply,silenced_bg_color);

	/* retrieve the background color of the iconBoard */
	XtVaGetValues(iconBoard, XmNbackground, &bg_pixel[0], NULL);
}

