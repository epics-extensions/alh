/*
 $Log$
 Revision 1.2  1994/06/22 21:17:10  jba
 Added cvs Log keyword

 */

static char *sccsId = "@(#)axRunW.c	1.8\t9/14/93";

/*axRunW.c      */
/*
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
 * .01  06-06-91        bkc     Modify the severity backgroud color
 * .02  08-22-91        bkc     Modify user interface to setup & groups 
 *				windows only 
 * .03  06-01-92        bkc     Modify the application resource name to Alh1
 *				 
 * .04  10-21-92        bkc     Include the sys/time.h for other system
 * .05  02-16-93        jba     Reorganized and modified files for new user interface
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

/************************************************************************************
*    axRunW.c
*
*This programs contains all the routines related to the iconlike runtime window
* and silence buttons.  Functions include display, unmapping, remapping of main
* window, alarm beeping, and blinking of control button.
*
*************************************************************************************
Routines defined in axRunW.c:

-------------
|   PUBLIC  |
-------------
*
void icon_update(bButton)                            Update runtime window
     Widget bButton;
*
void createRuntimeWindow(area)                       Create iconlike runtimeW
     ALINK *area;
*
void blinking()                                      Initiate runtimeW blinking 
*
void resetBeep()                                     Turn on beep option
*
void reInitBeep()                                    Reinitialize beep
*
void silenceForever_callback(w,toggleB,call_data)       Silence beep forever toggle
	Widget w;
    Widget toggleB;
	XmAnyCallbackStruct *call_data;
*
void beep_callback(w,beep,call_data)                 Silence current beep toggle
	Widget w;
	int beep;
	XmAnyCallbackStruct *call_data;
*
void createMainWindow_callback(w,area,call_data)     Create MainW widgets callback
     Widget w;
     ALINK *area;
     XmAnyCallbackStruct *call_data;
*
XtErrorMsgHandler trapExtraneousWarningsHandler(message)
     String message;
*
void pixelData(iconBoard,bButton)                     Setup pixel and font data
     Widget iconBoard;
     Widget bButton;
*
void unmapwindow_callback(w,main,call_data)           Unmap MainW callback
     Widget w;
     Widget main;
     XmAnyCallbackStruct *call_data;
*
void remapwindow_callback(w,main,call_data)           Remap MainW callback
     Widget w;
     Widget main;
     XmAnyCallbackStruct *call_data;
*
-------------
|  PRIVATE  |
-------------
*
static void alChangeOpenCloseButtonToRemap(main)
     XtPointer main;
*
static void alChangeOpenCloseButtonToUnmap(main)
     XtPointer main;
*
static void alChangeOpenCloseButtonToOpen(main)
     XtPointer main;
*
static void alLoadFont(fontname,font_info)
     char *fontname;
     XFontStruct **font_info;
*
static void toggled(widget,item,cbs)
     Widget               widget;
     int                  item;
     XmToggleButtonCallbackStruct *cbs;
*
static void okCallback(widget,area,cbs)
     Widget               widget;
     void *               area;
     XmAnyCallbackStruct *cbs;
*
***************************************************************************************/


#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/Form.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>

#include <alh.h>
#include <axArea.h>
#include <ax.h>

#include <alarm.h>
#include <fdmgr.h>

#ifdef __STDC__

/* prototypes for static routines */ 
static void alChangeOpenCloseButtonToRemap( XtPointer main);
static void alChangeOpenCloseButtonToUnmap( XtPointer main);
static void alChangeOpenCloseButtonToOpen( XtPointer main);
static void alLoadFont( char *fontname, XFontStruct **font_info);
static void toggled( Widget widget, int item, XmToggleButtonCallbackStruct *cbs);
static void okCallback( Widget widget, void * area, XmAnyCallbackStruct *cbs);
static void axExit_callback( Widget w, ALINK *area, XmAnyCallbackStruct *call_data);
static void axExitArea_callback( Widget w, ALINK *area, XmAnyCallbackStruct *call_data);

#else
static void alChangeOpenCloseButtonToRemap();
static void alChangeOpenCloseButtonToUnmap();
static void alChangeOpenCloseButtonToOpen();
static void alLoadFont();
static void toggled();
static void okCallback();
static void axExit_callback();
static void axExitArea_callback();

#endif /*__STDC__*/

/*   global variables */ 
XmStringCharSet charset = (XmStringCharSet) XmSTRING_DEFAULT_CHARSET;
char *fontname = "fixed";
extern struct setup psetup;
extern Widget toggle_button,toggle_button1;
extern int DEBUG;
extern char *alarmSeverityString[];


/*** define some globals for blink Data ***/
extern Display *display;
Widget blinkButton;
Pixel  blinkPixel;
extern fdctx *pfdctx;
void *blinkTimeoutId;
/*
static struct timeval blinkDelay = {0, 500};
*/
static struct timeval blinkDelay = {1, 0};
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

char *bg_color[] = {"lightblue","yellow","red","white","grey"};
char *bg_char[] = {" ", "Y", "R", "V" };



Pixel bg_pixel[ALARM_NSEV];
 
/* and so that channels and groups are distinguishable... */
char *channel_bg_color = "lightblue";
Pixel channel_bg_pixel;
 

/******************************************************
  COLOR
******************************************************/

static unsigned long COLOR(w, name)
     Widget w;
     char *name;
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
  update icon button 
******************************************************/

void icon_update()
{

#if  XmVersion && XmVersion >= 1002
               XmChangeColor(blinkButton,bg_pixel[0]);
#else
               XtVaSetValues(blinkButton,XmNbackground,bg_pixel[0],NULL);
#endif

  if (blinkTimeoutId) {
	fdmgr_clear_timeout(pfdctx,blinkTimeoutId);
	blinkTimeoutId = NULL;
  }

  blinkTimeoutId = (void *)fdmgr_add_timeout(pfdctx,&blinkDelay,
	blinking,NULL); 
}

/******************************************************
  alChangeOpenCloseButtonToRemap
******************************************************/

static void alChangeOpenCloseButtonToRemap(main)
     XtPointer main;
{
  XtRemoveCallback(blinkButton,XmNactivateCallback,(XtCallbackProc)unmapwindow_callback,main);
  XtAddCallback(blinkButton,XmNactivateCallback,(XtCallbackProc)remapwindow_callback,main);
}

/******************************************************
  alChangeOpenCloseButtonToUnmap
******************************************************/

static void alChangeOpenCloseButtonToUnmap(main)
     XtPointer main;
{
  XtRemoveCallback(blinkButton,XmNactivateCallback,(XtCallbackProc)remapwindow_callback,main);
  XtAddCallback(blinkButton,XmNactivateCallback,(XtCallbackProc)unmapwindow_callback,main);
}

/******************************************************
  alChangeOpenCloseButtonToOpen
******************************************************/

static void alChangeOpenCloseButtonToOpen(main)
     XtPointer main;
{
  XtRemoveCallback(blinkButton,XmNactivateCallback,(XtCallbackProc)unmapwindow_callback,main);
  XtRemoveCallback(blinkButton,XmNactivateCallback,(XtCallbackProc)remapwindow_callback,main);
  XtAddCallback(blinkButton,XmNactivateCallback,(XtCallbackProc)createMainWindow_callback,main);
}


/******************************************************
  alLoadFont
******************************************************/

static void alLoadFont(fontname,font_info)
char *fontname;
XFontStruct **font_info;
{
  if ((*font_info = XLoadQueryFont(display,fontname)) == NULL)
        {
	*font_info = XLoadQueryFont(display,"fixed");
        }
}


/********************************************************
   trapExtraneousWarningsHandler
*******************************************************/

XtErrorMsgHandler trapExtraneousWarningsHandler(message)
  String message;
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
  pixelData
*****************************************************/

void pixelData(iconBoard,bButton)
     Widget iconBoard;
     Widget bButton;
{
Arg args[10];
int n;
XmFontList fontlist;


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
  XtSetArg(args[n], XmNforeground, &foreground); n++;
  XtSetArg(args[n], XmNbackground, &background); n++;
  XtGetValues(iconBoard,args,n);
  bg_pixel[0] = background;


/*
 * get string width in pixel
 */
  if (bButton){
  n=0;
  XtSetArg(args[n], XmNfontList, &fontlist); n++;
  XtGetValues(bButton,args,n);
  char_width = XmStringWidth(fontlist,XmStringCreateLtoR("W",charset));

  alLoadFont(fontname,&font_info);
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

void createRuntimeWindow(area)
     ALINK *area;
{
     char   *alhTitle={"Alarm Handler"};
     XmString str;

     if (!area->runtimeToplevel){
          /*
           * create toplevel setup window
           */
          area->runtimeToplevel = XtAppCreateShell( programName, programName,
               applicationShellWidgetClass, display, NULL, 0);
     
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
               XmNiconName,            "ALHicon",
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
          str  = XmStringCreateLtoR( "--- No config file specified. ---",XmSTRING_DEFAULT_CHARSET);
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
          resetBeep();
     
          XtRealizeWidget(area->runtimeToplevel);

     } else {
          XMapWindow(XtDisplay(area->runtimeToplevel),
             XtWindow(area->runtimeToplevel));
     }

     pixelData(area->runtimeForm,area->blinkButton);
     
     /*  update blinkButton string */
     str = XmStringCreateSimple(area->blinkString);
     XtVaSetValues(area->blinkButton,
          XmNlabelString,            str,
          NULL);
     XmStringFree(str);

     if (area->silenceForever)
          reInitBeep(area->silenceForever,area->silenceCurrent);

     icon_update();


}

/******************************************************
  createMainWindow_callback
******************************************************/

void createMainWindow_callback(w,area,call_data)
     Widget w;
     ALINK *area;
     XmAnyCallbackStruct *call_data;
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
  blinking
******************************************************/

void blinking()
{
     Display *displayBB;
     static Boolean blinking2State = FALSE;

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
          }
          if (psetup.nobeep == FALSE && psetup.beep == TRUE && 
               (psetup.highestUnackSevr >= psetup.beepSevr)) {
               XBell(displayBB,0);
               XRaiseWindow(displayBB,XtWindow(XtParent(XtParent(blinkButton))));
          }
          blinkTimeoutId = (void *)fdmgr_add_timeout(pfdctx,&blinkDelay,
	       blinking, NULL); 
          blinking2State = TRUE;

     } else {

          if (psetup.highestUnackSevr > 0) {
#if  XmVersion && XmVersion >= 1002
               XmChangeColor(blinkButton,bg_pixel[0]);
#else
               XtVaSetValues(blinkButton,XmNbackground,bg_pixel[0],NULL);
#endif
          }
          blinkTimeoutId = (void *)fdmgr_add_timeout(pfdctx,&blinkDelay,
               blinking, NULL); 
          blinking2State = FALSE;

     }

     XFlush(displayBB);

     blinkCOUNT++;
     if ((blinkCOUNT % 1000) == 0 ) printf("blink times = %d \n",blinkCOUNT);

}

/******************************************************
 axExit_callback
******************************************************/

static void axExit_callback(w,area,call_data)
     Widget w;
     ALINK *area;
     XmAnyCallbackStruct *call_data;
{
     createActionDialog(area->runtimeToplevel,XmDIALOG_WARNING,
         "Exit Alarm Handler?",(XtCallbackProc)exit_quit,
         (XtPointer)area, (XtPointer)area);
}


/******************************************************
 axExitArea_callback
******************************************************/

static void axExitArea_callback(w,area,call_data)
     Widget w;
     ALINK *area;
     XmAnyCallbackStruct *call_data;
{
     SNODE *proot;

     alLogExit();
     XUnmapWindow(XtDisplay(area->runtimeToplevel),XtWindow(area->runtimeToplevel));
     XUnmapWindow(XtDisplay(area->toplevel),XtWindow(area->toplevel));
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

void unmapwindow_callback(w,main,call_data)
     Widget w;
     Widget main;
     XmAnyCallbackStruct *call_data;
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

void remapwindow_callback(w,main,call_data)
     Widget w;
     Widget main;
     XmAnyCallbackStruct *call_data;
{
     ALINK *area;

	if (main) {

             XtVaGetValues(w, XmNuserData, &area, NULL);

             XMapWindow(XtDisplay(main),XtWindow(main));
	
             alChangeOpenCloseButtonToUnmap((XtPointer)main);

             area->mapped = TRUE;
	}
	else
	alChangeOpenCloseButtonToOpen((XtPointer)main);	
}

/***********************************************
 turn on beep option 
************************************************/
void resetBeep()
{
/*   reset beep option if nobeep = false */

  if (psetup.beep == FALSE) { 
	psetup.beep = TRUE; 

	XmToggleButtonGadgetSetState(toggle_button,FALSE,FALSE);

	}
}

/**********************************************
 reinitialize beep variables
**********************************************/
void reInitBeep(toggle_button1,toggle_button)
     Widget toggle_button1;
     Widget toggle_button;
{
	   psetup.nobeep = FALSE;
	   psetup.beep = TRUE;
  	   XmToggleButtonGadgetSetState(toggle_button1,FALSE,FALSE);
  	   XmToggleButtonGadgetSetState(toggle_button,FALSE,FALSE);
}


/***************************************************
 silence button call back
****************************************************/

void beep_callback(w,beep,call_data)
     Widget w;
     int beep;
     XmAnyCallbackStruct *call_data;
{
	psetup.beep = psetup.beep ^ TRUE;
}

/***************************************************
 silenceForever_callback 
****************************************************/

void silenceForever_callback(w,toggleB,call_data)
     Widget w;
     Widget toggleB;
     XmAnyCallbackStruct *call_data;
{
	psetup.nobeep = psetup.nobeep ^ TRUE;
	if (psetup.nobeep == TRUE)  {
		psetup.beep = TRUE;
		XmToggleButtonGadgetSetState(toggleB,FALSE,FALSE);
		}

	if (psetup.nobeep == FALSE)  {
		if (XmToggleButtonGadgetGetState(toggleB)) 
			psetup.beep = FALSE;
		else
		psetup.beep = TRUE; 
		}
}
/***************************************************
 toggled 
****************************************************/

static void toggled(widget,item,cbs)
     Widget               widget;
     int                  item;
     XmToggleButtonCallbackStruct *cbs;
{
     Widget dialog;

     if (cbs->set == FALSE) return;

     dialog=XtParent(XtParent(widget));
     XtVaSetValues(dialog,XmNuserData,item+1,NULL);
}
/***************************************************
 okCallback 
****************************************************/

static void okCallback(widget,area,cbs)
     Widget               widget;
     void *               area;
     XmAnyCallbackStruct *cbs;
{
     int item;

     XtVaGetValues(widget,XmNuserData,&item,NULL);
     psetup.beepSevr = item;
     XtUnmanageChild(widget);
     XtDestroyWidget(widget);
}

/***************************************************
 createBeepSevrDialog 
****************************************************/

void createBeepSevrDialog(area,widget)
     void                *area;
     Widget               widget;
{
     Widget dialog, radioBox;
     XmString one,two,three,str;

    
     dialog = XmCreateMessageDialog(widget, "Dialog", NULL, 0);
     str = XmStringCreateSimple("Beep Condition");
     XtVaSetValues(dialog,
           XmNdialogTitle, str,
           XmNmessageAlignment, XmALIGNMENT_CENTER,
           XmNwidth, 250,
           XmNheight, 250,
           NULL);
     XmStringFree(str);

     XtSetSensitive(XmMessageBoxGetChild(dialog,XmDIALOG_HELP_BUTTON),FALSE);
     XtAddCallback(dialog,XmNcancelCallback, (XtCallbackProc)XtUnmanageChild,NULL);
     XtAddCallback(dialog,XmNokCallback,(XtCallbackProc)okCallback,area);

     /* RadioBox  */
     one = XmStringCreateSimple(alarmSeverityString[1]);
     two = XmStringCreateSimple(alarmSeverityString[2]);
     three = XmStringCreateSimple(alarmSeverityString[3]);
     radioBox = XmVaCreateSimpleRadioBox(dialog,"Severity Condition",
          psetup.beepSevr-1,(XtCallbackProc)toggled,
          XmVaRADIOBUTTON, one, NULL, NULL, NULL,
          XmVaRADIOBUTTON, two, NULL, NULL, NULL,
          XmVaRADIOBUTTON, three, NULL, NULL, NULL,
          NULL);
     XmStringFree(one);
     XmStringFree(two);
     XmStringFree(three);
     
     XtManageChild(radioBox);
     XtManageChild(dialog);
}
