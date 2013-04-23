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
/* awAlh.c */

/************************DESCRIPTION***********************************
  This file contains routines for alh menu.
**********************************************************************/

/******************************************************************
   Public routines defined in awAlh.c

Widget alhCreateMenu(parent, user_data)     Create alh pulldown Menu 
void alhFileCallback(widget, item, cbs)     File menu items callback
void alhActionCallback(widget, item, cbs)   Action menu items callback
void alhViewCallback(widget, item, cbs)     View menu items callback
void alhSetupCallback(widget, item, cbs)    Setup menu items callback
void alhHelpCallback(widget, item, cbs)     Help menu items callback
void awRowWidgets(line, area)  Create line widgets
void awUpdateRowWidgets(line)                 Update line widgets

**************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#ifndef CYGWIN32
#ifndef WIN32
#include <unistd.h>
#include <pwd.h>
#endif
#endif
#include <errno.h>
#include <sys/types.h>

#include <Xm/Xm.h>
#include <Xm/ArrowB.h>
#include <Xm/CascadeBG.h>
#include <Xm/DrawingA.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/SeparatoG.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/AtomMgr.h>
#include <Xm/DragDrop.h>
#include <Xm/CutPaste.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include "alarm.h"
#include "epicsVersion.h"

#include "ax.h"
#include "alh.h"
#include "axArea.h"
#include "axSubW.h"
#include "line.h"
#include "alLib.h"
#include "sllLib.h"
#include "version.h"
#include "alAudio.h"

/* menu definitions */
#define MENU_FILE_OPEN		10101
#define MENU_FILE_OPEN_OK	10102
#define MENU_FILE_CLOSE		10103
#define MENU_FILE_SAVEAS	10106
#define MENU_FILE_QUIT		10109

#define MENU_VIEW_EXPANDCOLLAPSE1	10200
#define MENU_VIEW_EXPANDBRANCH		10201
#define MENU_VIEW_EXPANDALL			10202
#define MENU_VIEW_COLLAPSEBRANCH	10203
#define MENU_VIEW_PROPERTIES		10204

#define	MENU_ACTION_ACK			10300
#define	MENU_ACTION_GUIDANCE	10301
#define	MENU_ACTION_PROCESS		10302
#define MENU_ACTION_FORCEPV		10303
#define MENU_ACTION_FORCE_MASK	10304
#define MENU_ACTION_MODIFY_MASK	10305
#define MENU_ACTION_BEEPSEVR	10306
#define MENU_ACTION_NOACKTIMER	10307


#define	MENU_VIEW_CONFIG		10400
#define	MENU_VIEW_OPMOD			10401
#define	MENU_VIEW_ALARMLOG		10402
#define	MENU_VIEW_CURRENT		10403
#define MENU_VIEW_CMLOG			10404

#define MENU_SETUP_BEEP_MINOR	10500
#define MENU_SETUP_BEEP_MAJOR	10501
#define MENU_SETUP_BEEP_INVALID	10502
#define MENU_SETUP_FILTER_NONE	10503
#define MENU_SETUP_FILTER_ACTIVE	10504
#define MENU_SETUP_FILTER_UNACK 	10505
#define MENU_SETUP_SILENCE_INTERVAL_5	10506
#define MENU_SETUP_SILENCE_INTERVAL_10	10507
#define MENU_SETUP_SILENCE_INTERVAL_15	10509
#define MENU_SETUP_SILENCE_INTERVAL_30	10510
#define MENU_SETUP_SILENCE_INTERVAL_60	10511
#define MENU_SETUP_SILENCE_FOREVER	10512
#define MENU_SETUP_ALARMLOG		10513
#define MENU_SETUP_OPMOD		10514

#define MENU_ACTION_BEEP_MINOR	10500
#define MENU_ACTION_BEEP_MAJOR	10501
#define MENU_ACTION_BEEP_INVALID	10502

#define MENU_HELP_HELP	10900
#define MENU_HELP_ABOUT	10906

/* external variables */
extern Pixel silenced_bg_pixel;
extern char alhVersionString[100];
extern char *bg_char[];
extern Pixel bg_pixel[];
extern Pixel channel_bg_pixel;
extern Pixel noack_bg_pixel;
extern struct setup psetup;
extern Widget versionPopup;
extern int _message_broadcast_flag; /* messages sending flag. Albert1*/
extern int messBroadcastDeskriptor;
extern char messBroadcastInfoFileName[250];
int messBroadcastLockDelay=60000; /* 1 min */
extern char *reloadMBString;
extern char *rebootString;
extern int max_not_save_time;
extern int amIsender;
extern int DEBUG;
extern int _main_window_flag;
extern int _mask_color_flag;
char FS_filename[128]; /* Filename      for FSBox. Albert*/

struct UserInfo {
char *loginid;
char *real_world_name;
char *myhostname;
char *displayName;
};

extern struct UserInfo userID; /* info about current operator */

/* prototypes for static routines */
static void alhFileCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhActionCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhViewCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhViewBrowserCallback( Widget widget, XtPointer item, XtPointer cbs); /* Albert1 */
static void messBroadcast(Widget widget, XtPointer item, XtPointer cbs);          /* Albert1 */
static void alhSetupCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void alhHelpCallback( Widget widget, XtPointer calldata, XtPointer cbs);
static void browserFBSDialogCbOk();     /* Ok-button     for FSBox. Albert*/
static void browserFBSDialogCbCancel(); /* Cancel-button for FSBox. Albert*/
static void writeMessBroadcast(Widget dialog, Widget text_w);
static void helpMessBroadcast(Widget,XtPointer,XtPointer);
static void cancelMessBroadcast(Widget w);
static void messBroadcastFileUnlock();

#ifdef CMLOG
static void awCMLOGstartBrowser(void);
#endif



static void drag (
  Widget w,
  XEvent *e,
  String *params,
  Cardinal numParams );

static void dummy (
  Widget w,
  XEvent *e,
  String *params,
  Cardinal numParams );

static int g_transInit = 1;
static XtTranslations g_parsedTrans;

static char g_dragTrans[] =
   "~Ctrl~Shift<Btn2Down>: startDrag()\n\
   Ctrl~Shift<Btn2Down>: dummy()\n\
   Shift~Ctrl<Btn2Down>: dummy()\n\
   Shift Ctrl<Btn2Down>: dummy()\n\
   Shift Ctrl<Btn2Up>: dummy()\n\
   Shift~Ctrl<Btn2Up>: dummy()";

static XtActionsRec g_dragActions[] = {
  { "startDrag", (XtActionProc) drag },
  { "dummy", (XtActionProc) dummy }
};

static int g_ddFixedFont_created = 0;
static XFontStruct *g_ddFixedFont = NULL;

static int g_ddgc_created = 0;
static GC g_ddgc = NULL;

static void dragFin (
  Widget w,
  XtPointer clientData,
  XtPointer call_data )
{

Widget icon;

  icon = NULL;

  XtVaGetValues( w, XmNsourcePixmapIcon, &icon, NULL );

  if ( icon ) {
    XtDestroyWidget( icon );
  }

}

static Boolean cvtSel (
  Widget w,
  Atom *selection,
  Atom *target,
  Atom *type_return,
  XtPointer *value_return,
  unsigned long *len_return,
  int *format_return )
{

struct anyLine *line;
int l;
char *pasteData;
char **value = (char**) value_return;
XSelectionRequestEvent* req;
Display* d = XtDisplay( w );

  if ( *selection != XA_PRIMARY ) {
    return FALSE;
  }

  req = XtGetSelectionRequest( w, *selection, (XtRequestId) NULL );

  XtVaGetValues( w, XmNuserData, &line, NULL );

  if ( !line ) {
    return FALSE;
  }

  if ( !line->pname ) {
    return FALSE;
  }

  l = strlen( line->pname );
  if ( l < 1 ) {
    return FALSE;
  }

  if (*target == XA_TARGETS(d)) {

    Atom* targetP;
    caddr_t std_targets;
    unsigned long std_length;

    XmuConvertStandardSelection( w, req->time, selection, target, type_return,
      &std_targets, &std_length, format_return );

    *value =
     (char*) XtMalloc( sizeof(Atom) * ( (unsigned) std_length + 5 ) );
    targetP = *( (Atom**) value );
    *targetP++ = XA_STRING;
    *targetP++ = XA_TEXT(d);
    *len_return = std_length + ( targetP - ( *(Atom **) value) );

    memcpy( (void*) targetP, (void*) std_targets,
     (size_t)( sizeof(Atom) * std_length ) );

    XtFree( (char*) std_targets );

    *type_return = XA_ATOM;
    *format_return = 32;

    return True;

  }

  if ( *target == XA_STRING ||
       *target == XA_TEXT(d) ||
       *target == XA_COMPOUND_TEXT(d) ) {

    if ( *target == XA_COMPOUND_TEXT(d) ) {
      *type_return = *target;
    }
    else {
      *type_return = XA_STRING;
    }

    pasteData = strdup( line->pname );

    *value_return = pasteData;
    *len_return = l;
    *format_return = 8;

    return TRUE;

  }

  if ( XmuConvertStandardSelection( w, req->time, selection, target,
   type_return, (XPointer *) value_return, len_return, format_return ) ) {
    return True;
  }

  return False;

}

static Boolean cvt (
  Widget w,
  Atom *selection,
  Atom *target,
  Atom *type_return,
  XtPointer *value_return,
  unsigned long *len_return,
  int *format_return )
{

Display *d;
struct anyLine *line;
Atom MOTIF_DROP;
size_t l;
char *dragData;

  d = XtDisplay( w );
  MOTIF_DROP = XmInternAtom( d, "_MOTIF_DROP", FALSE );

  if ( *selection != MOTIF_DROP ) {
    return FALSE;
  }

  if ( *target != XA_STRING ) {
    return FALSE;
  }

  XtVaGetValues( w, XmNclientData, &line, NULL );

  if ( !line->pname ) {
    return FALSE;
  }

  l = strlen( line->pname );
  if ( l < 1 ) {
    return FALSE;
  }

  dragData = strdup( line->pname );

  *type_return = *target;
  *value_return = dragData;
  *len_return = l+1;
  *format_return = 8;

  return TRUE;

}

/* 21 Aug 01 16:04:30 Thomas Birke (Thomas.Birke@mail.bessy.de)
 *
 *  Create a small pixmap to be set as the drag-icon and write the
 *  PV-names into the pixmap    
 */

static Widget mkDragIcon (
  Widget w,
  struct anyLine *line
) {

  Arg             args[8];
  Cardinal        n;
  Widget          sourceIcon;
  int             textWidth=0, maxWidth, maxHeight, fontHeight;
  unsigned long   fg, bg;
  XGCValues       gcValues;
  unsigned long   gcValueMask;
  char tmpStr[131+1], *str;

  Display *display = XtDisplay(w);
  int screenNum = DefaultScreen(display);

  Pixmap sourcePixmap = (Pixmap)NULL;

  if ( !g_ddFixedFont_created ) {
    g_ddFixedFont_created = 1;
    g_ddFixedFont = XLoadQueryFont( display, "fixed" );
  }

#define X_SHIFT 8
#define MARGIN  2

  bg = BlackPixel(display,screenNum);
  fg = WhitePixel(display,screenNum);

  fontHeight = g_ddFixedFont->ascent + g_ddFixedFont->descent;

  strcpy( tmpStr, "[N/A]" );
  str = strdup( line->pname );
  if ( str ) {
    strncpy( tmpStr, str, 131 );
    tmpStr[131] = 0;
  }

  textWidth = XTextWidth( g_ddFixedFont, tmpStr, strlen(tmpStr) );

  maxWidth = X_SHIFT + ( textWidth + MARGIN );
  maxHeight = fontHeight + 2 * MARGIN;
  
  sourcePixmap = XCreatePixmap(display,
   RootWindow(display, screenNum),
   maxWidth,maxHeight,
   DefaultDepth(display,screenNum) );

  if ( !g_ddgc_created ) {
    g_ddgc_created = 1;
    g_ddgc = XCreateGC( display, sourcePixmap, 0, NULL );
  }
  
  gcValueMask = GCForeground|GCBackground|GCFunction|GCFont;
  
  gcValues.foreground = bg;
  gcValues.background = bg;
  gcValues.function   = GXcopy;
  gcValues.font       = g_ddFixedFont->fid;
  
  XChangeGC( display, g_ddgc, gcValueMask, &gcValues );
  
  XFillRectangle( display, sourcePixmap, g_ddgc, 0, 0, maxWidth,
   maxHeight);

  XSetForeground( display, g_ddgc, fg );

  XDrawString( display, sourcePixmap, g_ddgc,
	       X_SHIFT, g_ddFixedFont->ascent + MARGIN, 
	       tmpStr, strlen(tmpStr) );
  
  n = 0;
  XtSetArg(args[n],XmNpixmap,sourcePixmap); n++;
  XtSetArg(args[n],XmNwidth,maxWidth); n++;
  XtSetArg(args[n],XmNheight,maxHeight); n++;
  XtSetArg(args[n],XmNdepth,DefaultDepth(display,screenNum)); n++;
  sourceIcon = XmCreateDragIcon(XtParent(w),"sourceIcon",args,n);

  return sourceIcon;

}

static int startDrag (
  struct anyLine *line,
  Widget w,
  XEvent *e )
{

Atom expList[1];
int status, n;
Arg args[10];
Widget dc;
Widget icon;

  /* attempt to put pv name into primary select buffer */
  if ( w ) {
    XtDisownSelection( w, XA_PRIMARY, CurrentTime );
    status = XtOwnSelection( w, XA_PRIMARY, CurrentTime,
     cvtSel, (XtLoseSelectionProc) 0, (XtSelectionDoneProc) 0 );
  }

  icon = mkDragIcon( w, line );
  if ( !icon ) return 0;
 
  expList[0] = XA_STRING;
  n = 0;
  XtSetArg( args[n], XmNexportTargets, expList ); n++;
  XtSetArg( args[n], XmNnumExportTargets, 1 ); n++;
  XtSetArg( args[n], XmNdragOperations, XmDROP_COPY ); n++;
  XtSetArg( args[n], XmNconvertProc, cvt ); n++;
  XtSetArg( args[n], XmNsourcePixmapIcon, icon ); n++;
  XtSetArg( args[n], XmNclientData, (XtPointer) line ); n++;
    
  dc = XmDragStart( w, e, args, n );
  XtAddCallback( dc, XmNdragDropFinishCallback, dragFin, (XtPointer) line );

  return 1;

}

static void drag (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

struct anyLine *line;
int stat;

  XtVaGetValues( w, XmNuserData, &line, NULL );

  stat = startDrag( line, w, e );

}

static void dummy (
   Widget w,
   XEvent *e,
   String *params,
   Cardinal numParams )
{

}

/******************************************************
  alhCreateMenu
******************************************************/

/* Create ALH MenuBar */
Widget alhCreateMenu(Widget parent,XtPointer user_data)
{
	static MenuItem file_menu[] = {
		         { "Open ...",   PushButtonGadgetClass, 'O', "Ctrl<Key>O", "Ctrl+O",
		             alhFileCallback, (XtPointer)MENU_FILE_OPEN,   (MenuItem *)NULL, 0 },
		         { "Save As ...",PushButtonGadgetClass, 'v', NULL, NULL,
		             alhFileCallback, (XtPointer)MENU_FILE_SAVEAS, (MenuItem *)NULL , 0},
		         { "",           SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,    NULL,   (MenuItem *)NULL },
		         { "Close",      PushButtonGadgetClass, 'C', NULL, NULL,
		             alhFileCallback, (XtPointer)MENU_FILE_CLOSE,  (MenuItem *)NULL, 0 },
		         { "",           SeparatorGadgetClass, '\0', NULL, NULL,
		             NULL,    NULL,   (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	static MenuItem action_menu[] = {
		         { "Acknowledge Alarm",      PushButtonGadgetClass, 'A', "Ctrl<Key>A", "Ctrl+A",
		             alhActionCallback, (XtPointer)MENU_ACTION_ACK,      (MenuItem *)NULL, 0 },
		         { "Display Guidance",       PushButtonGadgetClass, 'G', "Ctrl<Key>G", "Ctrl+G",
		             alhActionCallback, (XtPointer)MENU_ACTION_GUIDANCE, (MenuItem *)NULL, 0 },
		         { "Start Related Process",  PushButtonGadgetClass, 'P', "Ctrl<Key>P", "Ctrl+P",
		             alhActionCallback, (XtPointer)MENU_ACTION_PROCESS,  (MenuItem *)NULL, 0 },
		         { "Force Process Variable ...", ToggleButtonGadgetClass, 'V', "Ctrl<Key>V", "Ctrl+V",
		             alhActionCallback, (XtPointer)MENU_ACTION_FORCEPV, (MenuItem *)NULL, 0 },
		         { "Force Mask ...",          ToggleButtonGadgetClass, 'M',"Ctrl<Key>M", "Ctrl+M",
		             alhActionCallback, (XtPointer)MENU_ACTION_FORCE_MASK, (MenuItem *)NULL, 0 },
		         { "Modify Mask Settings ...",  ToggleButtonGadgetClass, 'S', "Ctrl<Key>S", "Ctrl+S",
		             alhActionCallback, (XtPointer)MENU_ACTION_MODIFY_MASK, (MenuItem *)NULL, 0 },
		         { "Beep Severity ...",  ToggleButtonGadgetClass, 'B', "Ctrl<Key>B", "Ctrl+B",
		             alhActionCallback, (XtPointer)MENU_ACTION_BEEPSEVR, (MenuItem *)NULL, 0 },
		         { "NoAck for One Hour ...",  ToggleButtonGadgetClass, 'N', "Ctrl<Key>N", "Ctrl+N",
		             alhActionCallback, (XtPointer)MENU_ACTION_NOACKTIMER, (MenuItem *)NULL, 0 },
		         {NULL},
		     	};
	/* Albert Kagarmanov new */
#ifndef CYGWIN32
#ifndef WIN32
	static MenuItem setup_broadcast_mess_menu[] = {
		         { "Common Message",      PushButtonGadgetClass, 'C', NULL, NULL,
		             messBroadcast, (XtPointer)0,  (MenuItem *)NULL, 0 },
		         { "Stop Logging Message",      PushButtonGadgetClass, 'S', NULL, NULL,
		             messBroadcast, (XtPointer)1,  (MenuItem *)NULL, 0 },
		         { "Reload",      PushButtonGadgetClass, 'R', NULL, NULL,
		             messBroadcast, (XtPointer)2,  (MenuItem *)NULL, 0 },

		         { "About",      PushButtonGadgetClass, 'H', NULL, NULL,
		             helpMessBroadcast, (XtPointer)0,  (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

#endif
#endif
/* end Albert Kagarmanov new */
/* ******************************************** Albert1 : ************************************ */
static MenuItem action_menuNew[] = {
		         { "Acknowledge Alarm",      PushButtonGadgetClass, 'A', "Ctrl<Key>A", "Ctrl+A",
		             alhActionCallback, (XtPointer)MENU_ACTION_ACK,      (MenuItem *)NULL, 0 },
		         { "Display Guidance",       PushButtonGadgetClass, 'G', "Ctrl<Key>G", "Ctrl+G",
		             alhActionCallback, (XtPointer)MENU_ACTION_GUIDANCE, (MenuItem *)NULL, 0 },
		         { "Start Related Process",  PushButtonGadgetClass, 'P', "Ctrl<Key>P", "Ctrl+P",
		             alhActionCallback, (XtPointer)MENU_ACTION_PROCESS,  (MenuItem *)NULL, 0 },
		         { "Force Process Variable ...", ToggleButtonGadgetClass, 'V', "Ctrl<Key>V", "Ctrl+V",
		             alhActionCallback, (XtPointer)MENU_ACTION_FORCEPV, (MenuItem *)NULL, 0 },
		         { "Force Mask ...",          ToggleButtonGadgetClass, 'M',"Ctrl<Key>M", "Ctrl+M",
		             alhActionCallback, (XtPointer)MENU_ACTION_FORCE_MASK, (MenuItem *)NULL, 0 },
		         { "Modify Mask Settings ...",  ToggleButtonGadgetClass, 'S', "Ctrl<Key>S", "Ctrl+S",
		             alhActionCallback, (XtPointer)MENU_ACTION_MODIFY_MASK, (MenuItem *)NULL, 0 },
		         { "Beep Severity ...",  ToggleButtonGadgetClass, 'B', "Ctrl<Key>B", "Ctrl+B",
		             alhActionCallback, (XtPointer)MENU_ACTION_BEEPSEVR, (MenuItem *)NULL, 0 },
		         { "NoAck for One Hour ...",  ToggleButtonGadgetClass, 'N', "Ctrl<Key>N", "Ctrl+N",
		             alhActionCallback, (XtPointer)MENU_ACTION_NOACKTIMER, (MenuItem *)NULL, 0 },
			 /* Albert1 For MESSAGE BROADCAST: */
#ifndef CYGWIN32
#ifndef WIN32
		         { "Send Message ...",  PushButtonGadgetClass, 'B', "Ctrl<Key>B", "Ctrl+B",
		             0, 0, (MenuItem *)setup_broadcast_mess_menu, 0 },
#endif
#endif
		         {NULL},
		     	};
/* ******************************************** End Albert1 ********************************** */

	static MenuItem view_menu[] = {
		         { "Expand One Level",       PushButtonGadgetClass, 'L', "None<Key>plus", "+",
		             alhViewCallback, (XtPointer)MENU_VIEW_EXPANDCOLLAPSE1,   (MenuItem *)NULL, 0 },
		         { "Expand Branch",          PushButtonGadgetClass, 'B', "None<Key>asterisk", "*",
		             alhViewCallback, (XtPointer)MENU_VIEW_EXPANDBRANCH,      (MenuItem *)NULL, 0 },
		         { "Expand All",             PushButtonGadgetClass, 'A', "Ctrl<Key>asterisk", "Ctrl+*",
		             alhViewCallback, (XtPointer)MENU_VIEW_EXPANDALL,         (MenuItem *)NULL, 0 },
		         { "Collapse Branch",        PushButtonGadgetClass, 'C', "None<Key>minus", "-",
		             alhViewCallback, (XtPointer)MENU_VIEW_COLLAPSEBRANCH,    (MenuItem *)NULL, 0 },
		         { "",                       SeparatorGadgetClass,  '\0', NULL, NULL,
		             NULL,    NULL,   (MenuItem *)NULL, 0 },
		         { "Current Alarm History Window",  ToggleButtonGadgetClass, 'H', NULL, NULL,
		             alhViewCallback, (XtPointer)MENU_VIEW_CURRENT,         (MenuItem *)NULL, 0 },
		         { "Configuration File Window",     ToggleButtonGadgetClass, 'f', NULL, NULL,
		             alhViewCallback, (XtPointer)MENU_VIEW_CONFIG,           (MenuItem *)NULL, 0 },
#ifdef CMLOG
                         { "Start CMLOG Log Browser", PushButtonGadgetClass, 's', NULL, NULL,
                             alhViewCallback, (XtPointer)MENU_VIEW_CMLOG, (MenuItem *)NULL, 0 },
#endif
		         { "Alarm Log File Window",         ToggleButtonGadgetClass, 'r', NULL, NULL,
		             alhViewCallback, (XtPointer)MENU_VIEW_ALARMLOG,         (MenuItem *)NULL, 0 },
		         { "Browser For Alarm Log",         ToggleButtonGadgetClass, 's', NULL, NULL,
		            alhViewBrowserCallback, (XtPointer)MENU_VIEW_ALARMLOG,         (MenuItem *)NULL, 0 },
		         { "Operation Log File Window",     ToggleButtonGadgetClass, 'O', NULL, NULL,
		             alhViewCallback, (XtPointer)MENU_VIEW_OPMOD,         (MenuItem *)NULL, 0 },
		         { "Browser For Operation Log",         ToggleButtonGadgetClass, 'e', NULL, NULL,
		             alhViewBrowserCallback, (XtPointer)MENU_VIEW_OPMOD,         (MenuItem *)NULL, 0 },
		         { "Group/Channel Properties Window", ToggleButtonGadgetClass, 'W', NULL, NULL,
		             alhViewCallback, (XtPointer)MENU_VIEW_PROPERTIES,    (MenuItem *)NULL, 0 },

		         {NULL},
		     	};

	static MenuItem setup_beep_menu[] = {
		         { "Minor",      PushButtonGadgetClass, 'M', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_MINOR,  (MenuItem *)NULL, 0 },
		         { "Major",      PushButtonGadgetClass, 'A', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_MAJOR,  (MenuItem *)NULL, 0 },
		         { "Invalid",      PushButtonGadgetClass, 'V', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_BEEP_INVALID,  (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	static MenuItem setup_silence_interval_menu[] = {
		         { "5 minutes",      PushButtonGadgetClass, 0, NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_SILENCE_INTERVAL_5,  (MenuItem *)NULL, 0 },
		         { "10 minutes",      PushButtonGadgetClass, 0, NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_SILENCE_INTERVAL_10,  (MenuItem *)NULL, 0 },
		         { "15 minutes",      PushButtonGadgetClass, 0, NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_SILENCE_INTERVAL_15,  (MenuItem *)NULL, 0 },
		         { "30 minutes",      PushButtonGadgetClass, 0, NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_SILENCE_INTERVAL_30,  (MenuItem *)NULL, 0 },
		         { "1 hour",      PushButtonGadgetClass, 0, NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_SILENCE_INTERVAL_60,  (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	static MenuItem setup_filter_menu[] = {
		         { "No filter",      PushButtonGadgetClass, 'N', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_FILTER_NONE,  (MenuItem *)NULL, 0 },
		         { "Active Alarms Only",      PushButtonGadgetClass, 'A', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_FILTER_ACTIVE,  (MenuItem *)NULL, 0 },
		         { "Unacknowledged Alarms Only",      PushButtonGadgetClass, 'U', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_FILTER_UNACK,  (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	static MenuItem setup_menu[] = {
		         { "Display Filter...",        PushButtonGadgetClass, 'F', NULL, NULL,
		                                      0, 0,    (MenuItem *)setup_filter_menu, 0 },
		         { "ALH Beep Severity...",       PushButtonGadgetClass, 'B', NULL, NULL,
		                                      0, 0,    (MenuItem *)setup_beep_menu, 0 },
#ifdef AUDIO_BEEP
		         { "Audio Setup...",       ToggleButtonGadgetClass, 'D', NULL, NULL,
		             alhAudioSetupCallback, NULL,  (MenuItem *)NULL, 0 },
#endif
		         { "Select silence interval...",PushButtonGadgetClass, 'S', NULL, NULL,
		                               0, 0,    (MenuItem *)setup_silence_interval_menu, 0 },
		         { "Silence Forever",  ToggleButtonGadgetClass, 'S', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_SILENCE_FOREVER,(MenuItem *)NULL, 0 },
		         { "New Alarm Log File Name...",  PushButtonGadgetClass, 'L', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_ALARMLOG,     (MenuItem *)NULL, 0 },
		         { "New Oper. Log File Name...",  PushButtonGadgetClass, 'O', NULL, NULL,
		             alhSetupCallback, (XtPointer)MENU_SETUP_OPMOD,     (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	static MenuItem help_menu[] = {
		         { "Help",       PushButtonGadgetClass, 'H', "Ctrl<Key>H", "Ctrl+H",
		             alhHelpCallback, (XtPointer)MENU_HELP_HELP, (MenuItem *)NULL, 0 },
		         { "About ALH",     PushButtonGadgetClass, 'A', NULL, NULL,
		             alhHelpCallback, (XtPointer)MENU_HELP_ABOUT, (MenuItem *)NULL, 0 },
		         {NULL},
		     	};

	Widget menubar,widget;

	/* Set "Silence Forever" toggleButton initial state  */
	setup_menu[2].initial_state = psetup.silenceForever;

	menubar = XmCreateMenuBar(parent, "menubar",   NULL, 0);

	widget = buildPulldownMenu(menubar, "File",     'F', TRUE, file_menu, user_data);
        if(!_message_broadcast_flag)  /* Albert1 */
	widget = buildPulldownMenu(menubar, "Action",   'A', TRUE, action_menu, user_data);
        else
        widget = buildPulldownMenu(menubar, "Action",   'A', TRUE, action_menuNew, user_data);
        
	widget = buildPulldownMenu(menubar, "View",     'V', TRUE, view_menu, user_data);

	widget = buildPulldownMenu(menubar, "Setup",    'S', TRUE, setup_menu, user_data);

	widget = buildPulldownMenu(menubar, "Help",     'H', TRUE, help_menu, user_data);



	/* Make sure Help on MenuBar item is right adjusted */
	XtVaSetValues(menubar, XmNtopAttachment, XmATTACH_FORM, NULL);
	XtVaSetValues(menubar, XmNrightAttachment, XmATTACH_FORM, NULL);
	XtVaSetValues(menubar, XmNleftAttachment, XmATTACH_FORM, NULL);
	XtVaSetValues(menubar, XmNmenuHelpWidget, widget, NULL);

	XtManageChild(menubar);

	return(menubar);
}


/******************************************************
  alhFileCallback
******************************************************/
static void alhFileCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	ALINK      *area;
	int item=(long)calldata;

	XtVaGetValues(widget, XmNuserData, &area, NULL);

	switch (item){

	case MENU_FILE_OPEN:

		/* New Name for Config File  */

		/* Display the config_changed warning dialog */
		if (area->changed){
			createActionDialog(area->form_main,XmDIALOG_WARNING,
			    "Config file settings have Changed.  Do you wish to continue?",
			    (XtCallbackProc)alhFileCallback,
			    (XtPointer)MENU_FILE_OPEN_OK, (XtPointer)area);
			break;
		}
		area->managed = FALSE;
		createFileDialog(area->form_main,
		    (void *)fileSetupCallback, (XtPointer)FILE_CONFIG,
		    (void *)fileCancelCallback,(XtPointer)area,
		    (XtPointer)area,
		    "Alarm Configuration File",CONFIG_PATTERN,psetup.configDir);
		break;

	case MENU_FILE_OPEN_OK:

		area->managed = FALSE;
		createFileDialog(area->form_main,
		    (void *)fileSetupCallback, (XtPointer)FILE_CONFIG,
		    (void *)fileCancelCallback,(XtPointer)area,
		    (XtPointer)area,
		    "Alarm Configuration File",CONFIG_PATTERN,psetup.configDir);
		break;

	case MENU_FILE_SAVEAS:
		/* New Name for Save Config File  */

		createFileDialog(area->form_main,
		    (void *)fileSetupCallback, (XtPointer)FILE_SAVEAS,
		    (void *)XtUnmanageChild,(XtPointer)0,
		    (XtPointer)area,
		    "Save Alarm Configuration File",CONFIG_PATTERN,psetup.configDir);
		break;

	case MENU_FILE_CLOSE:

		/* "Close" was selected. */
		if (_main_window_flag)
			exit_quit(area->toplevel,(XtPointer)area,(XtPointer)area);
		else
			unmapArea_callback(area->toplevel,area->form_main,(XmAnyCallbackStruct *)cbs);
		break;

	case MENU_FILE_QUIT:

		createActionDialog(area->toplevel,XmDIALOG_WARNING,
		    "Exit Alarm Handler?",(XtCallbackProc)exit_quit,
		    (XtPointer)area, (XtPointer)area);
		break;
	}
}


/******************************************************
  alhActionCallback
******************************************************/

static void alhActionCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	int item=(long)calldata;
	ALINK               *area;
	Widget               parent;
	GCLINK              *link;
	struct anyLine      *line;
	WLINE               *wline;

	XtVaGetValues(widget, XmNuserData, &area, NULL);

	switch (item){

	case MENU_ACTION_ACK:

		/* Acknowledge Alarm */
		link = (GCLINK *)area->selectionLink;

		if (link){
			line = (struct anyLine *)link->lineTreeW;
			if (line && line->pwindow == (void *)area->selectionWindow ){

				ack_callback(widget,line,(XmAnyCallbackStruct *)cbs);
			}
			else {
				line = (struct anyLine *)link->lineGroupW;
				if (line && line->pwindow == (void *)area->selectionWindow ){
					ack_callback(widget,line,(XmAnyCallbackStruct *)cbs);
				}
			}
		} else {
			createDialog(area->form_main,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_GUIDANCE:

		/* Display Guidance */
		link = (GCLINK *)area->selectionLink;
		line = (struct anyLine *)link->lineTreeW;
		if (! line || line->pwindow != (void *)area->selectionWindow )
			line = (struct anyLine *)link->lineGroupW;
		if (line){
			wline=(WLINE *)line->wline;
			guidanceCallback(wline->guidance,(GCLINK *)link,(XmAnyCallbackStruct *) cbs);
		}
		else {
			createDialog(area->form_main,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_PROCESS:

		/* Start Related Process */
		link = (GCLINK *)area->selectionLink;
		if (link){
			if (alProcessExists(link)){
				relatedProcess_callback(widget,link, cbs);
			} else {
				if (((GCLINK *)link)->pgcData->alias){
					createDialog(area->form_main,XmDIALOG_WARNING,"No related process for ",
					    link->pgcData->alias);
				} else {
					createDialog(area->form_main,XmDIALOG_WARNING,"No related process for ",
					    link->pgcData->name);
				}
			}
		} else {
			createDialog(area->form_main,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;


	case MENU_ACTION_FORCEPV:

		if (area->selectionLink) {
			forcePVShowDialog(area, widget);
		} else {
			parent = area->form_main;
			createDialog(parent,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_FORCE_MASK:

		if (area->selectionLink) {
			forceMaskShowDialog(area, widget);
		} else {
			parent = area->form_main;
			createDialog(parent,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_MODIFY_MASK:

		if (area->selectionLink) {
			maskShowDialog(area, widget);
		} else {
			parent = area->form_main;
			createDialog(parent,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_BEEPSEVR:

		if (area->selectionLink) {
			beepSevrShowDialog(area, widget);
		} else {
			parent = area->form_main;
			createDialog(parent,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	case MENU_ACTION_NOACKTIMER:

		if (area->selectionLink) {
			noAckShowDialog(area, widget);
		} else {
			parent = area->form_main;
			createDialog(parent,XmDIALOG_WARNING,
			    "Please select an alarm group or channel first."," ");
		}
		break;

	}
}

 
/******************************************************
  alhViewCallback
******************************************************/
static void alhViewCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	int item=(long)calldata;
	ALINK   *area;
	void   *link;
	struct subWindow *treeWindow;

	XtVaGetValues(widget, XmNuserData, &area, NULL);
	treeWindow = (struct subWindow *)area->treeWindow;

	switch (item){

	case MENU_VIEW_EXPANDCOLLAPSE1:

		/* Expand 1 level */
		link = treeWindow->selectionLink;
		if (link) displayNewViewTree(area,link,EXPANDCOLLAPSE1);
		else createDialog(area->form_main,XmDIALOG_WARNING,"Please select an alarm group first."," ");
		break;

	case MENU_VIEW_EXPANDBRANCH:

		/* Expand Branch */
		link = treeWindow->selectionLink;
		if (link )displayNewViewTree(area,link,EXPAND);
		else createDialog(area->form_main,XmDIALOG_WARNING,"Please select an alarm group first."," ");
		break;

	case MENU_VIEW_EXPANDALL:

		/* Expand all */
		displayNewViewTree(area,(GLINK *)sllFirst(area->pmainGroup),EXPAND);
		break;

	case MENU_VIEW_COLLAPSEBRANCH:

		/* Collapse branch */
		link = treeWindow->selectionLink;
		if (link )displayNewViewTree(area,link,COLLAPSE);
		else createDialog(area->form_main,XmDIALOG_WARNING,"Please select an alarm group first."," ");
		break;

	case MENU_VIEW_CURRENT:

		/* Display Current Alarm History */
		currentAlarmHistoryWindow(area,widget);
		break;

	case MENU_VIEW_CONFIG:

		/* Display Alarm Configuration File */
		fileViewWindow(area->form_main,CONFIG_FILE,widget);
		break;

	case MENU_VIEW_ALARMLOG:

		/* Display Alarm Log File */
		fileViewWindow(area->form_main,ALARM_FILE,widget);
		break;

	case MENU_VIEW_OPMOD:

		/* Display Operation Log File */
		fileViewWindow(area->form_main,OPMOD_FILE,widget);
		break;
#ifdef CMLOG
	case MENU_VIEW_CMLOG:

		/* Start CMLOG Browser */
		awCMLOGstartBrowser();
		break;
#endif
	case MENU_VIEW_PROPERTIES:

		/* Display Group/Chan Properties */
		propShowDialog(area, widget);
		break;

	}
}
 
/******************************************************
  FileSelect for AlhView. Albert
******************************************************/
static void alhViewBrowserCallback(Widget widget,XtPointer item,XtPointer cbs)
{
	ALINK   *area;
	Widget dialog;
	XmString Xpattern,Xtitle,Xcurrentdir;
        int ch = (long) item;
	switch ( ch )
	  {
	  case MENU_VIEW_ALARMLOG:
	    Xtitle=XmStringCreateSimple("Alarm Log File");          
	    Xpattern = XmStringCreateSimple(psetup.logFile);
	    Xcurrentdir = XmStringCreateSimple(psetup.logDir); 
	    break;
	    
	  case MENU_VIEW_OPMOD:
	    Xtitle=XmStringCreateSimple("Operator File");          
	    Xpattern = XmStringCreateSimple(psetup.opModFile);
	    Xcurrentdir = XmStringCreateSimple(psetup.logDir);  /* Albert1 ???? */  
	    break;

	  default:
            perror("bad item");
	    return;
	  }     

		XtVaGetValues(widget, XmNuserData, &area, NULL);
		dialog=XmCreateFileSelectionDialog(area->form_main,"dialog",NULL,0);
	switch ( ch )
	  {
	  case MENU_VIEW_ALARMLOG:
	    	XtVaSetValues(dialog,XmNuserData,ALARM_FILE,NULL);
	   	 break;
	  case MENU_VIEW_OPMOD:
	    	XtVaSetValues(dialog,XmNuserData,OPMOD_FILE,NULL);
	   	 break;
	    
	  }     
		XtAddCallback(dialog,XmNokCallback,(XtCallbackProc)browserFBSDialogCbOk,widget);
		XtAddCallback(dialog,XmNcancelCallback,(XtCallbackProc)browserFBSDialogCbCancel,NULL);
		XtUnmanageChild(XmFileSelectionBoxGetChild(dialog,
		    XmDIALOG_HELP_BUTTON));

		XtVaSetValues(dialog,
		    XmNdirectory,     Xcurrentdir,
		    XmNdialogTitle,   Xtitle,
		    XmNdirMask,    Xpattern,
                    XmNfileTypeMask,		XmFILE_REGULAR,
		    (XtPointer)NULL);
		XtManageChild(dialog);
		XmStringFree(Xpattern);
		XmStringFree(Xtitle);
}

/******************************************************
  browserFBSDialogCbCancel
******************************************************/
static void browserFBSDialogCbCancel(Widget w,int client_data,
XmSelectionBoxCallbackStruct *call_data)
{
	XtUnmanageChild(w);
}

/******************************************************
  browserFBSDialogCbOk
******************************************************/
static void browserFBSDialogCbOk(Widget w,Widget wdgt,
XmSelectionBoxCallbackStruct *call_data)
{
	ALINK   *area;
	char *s;
	int fileType;
	XmStringGetLtoR(call_data->value,XmSTRING_DEFAULT_CHARSET,&s);
	strcpy(FS_filename,s);
	XtFree(s);
	XtVaGetValues(w, XmNuserData, &fileType, NULL);
	XtVaGetValues(wdgt, XmNuserData, &area, NULL);
	browser_fileViewWindow(area->form_main,fileType,wdgt); /* Albert1 */
	XtUnmanageChild(w);
}

#ifndef CYGWIN32
#ifndef WIN32
struct messBroadcastData
{
int type;
ALINK   *area;
}
messBroadcastData;


/******************************************************
Send Message widget
******************************************************/
static void messBroadcast(Widget widget,XtPointer item,XtPointer cbs)  /* Albert1 */
{
    Widget dialog, text_w;
    ALINK   *area;
    static struct messBroadcastData mBD;
    int itemi = (long) item;

    XtVaGetValues(widget, XmNuserData, &area, NULL);

    if(amIsender)
      {
	createDialog(area->form_main,XmDIALOG_INFORMATION,
		     "You send some message before. \n" "\n"
		     "Please wait a few seconds \n","");
	return;
      }
         if ( lockf(messBroadcastDeskriptor, F_TLOCK, 0L) < 0 ) {
	  if ((errno == EAGAIN || errno == EACCES )) {
	      if(DEBUG) fprintf(stderr,"file is busy;Deskriptor=%d\n",messBroadcastDeskriptor);
	      createDialog(area->form_main,XmDIALOG_INFORMATION,
			   "Some other operator type a message. \n" "\n"
			   "Please wait a few seconds \n","");
	      return;
	  }
	  else {
	    perror("lockf Error!!!!"); /* Albert1 exit ?????? */
	  }
	 }
	else 
	  {
	    if(DEBUG) fprintf(stderr,"file is free;Deskriptor=%d\n",messBroadcastDeskriptor);	    

	    dialog = XmCreatePromptDialog(area->form_main, "dialog", NULL, 0);	    
	    XtVaSetValues(dialog,XtVaTypedArg, XmNselectionLabelString, XmRString,
			  "Type message (See help for detail):", 40,NULL);
	    
	    XtAddCallback(dialog, XmNcancelCallback,(XtCallbackProc)cancelMessBroadcast, NULL);
	    XtAddCallback(dialog, XmNhelpCallback,(XtCallbackProc)helpMessBroadcast, NULL);
	    text_w = XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT);
	    if(itemi == 1)
	      {
		XtVaSetValues(text_w, XmNvalue,"0 min no write to LOG file", NULL);
	      }
	    if(itemi == 2)
	      {
		if(DEBUG) fprintf(stderr,"second item\n");
		XtVaSetValues(text_w, XmNvalue,"Reload config. Reason:", NULL);
	      }

	       mBD.area =area;
	       mBD.type =itemi;

	    XtVaSetValues(dialog,XmNuserData,&mBD,NULL);
	    XtAddCallback(dialog, XmNokCallback,(XtCallbackProc)writeMessBroadcast, text_w);
	    XtManageChild(dialog);

	  }
}

ALINK   *ar;

static void writeMessBroadcast(Widget dialog, Widget text_w)
{
FILE *fp;
time_t timeID,time_tmp;
void messBroadcastFileUnlock();
static char string[256];
char *MessString;
char *blank;

int notsave_time;
static char buff[500];
static struct  messBroadcastData *mBDpt;

int type;

    memset(string,0,256);
    XtVaGetValues(dialog, XmNuserData,&mBDpt, NULL);

    type=mBDpt->type;
    if (( type <0) && ( type >2)) return;
    
    ar= mBDpt->area;

    if ( (fp=fopen(messBroadcastInfoFileName,"w")) == NULL )
      {
	createDialog(ar->form_main,XmDIALOG_INFORMATION,"can't open ",messBroadcastInfoFileName);
	lockf(messBroadcastDeskriptor, F_ULOCK, 0L);
	return;
          }

    
    timeID=time(0L); 
    time_tmp=timeID;

    MessString = XmTextFieldGetString(text_w);
    if(type == 0)       {strcpy(string,"0 "); strcat(string,MessString);}
    else if (type == 1)  strcpy(string,MessString);
    else if (type == 2)  {strcpy(string,reloadMBString);strcat(string,MessString);}    

    if( (blank=strchr(string,' ')) == NULL )
      {
	createDialog(ar->form_main,XmDIALOG_INFORMATION,"Wrong format"," ");
	lockf(messBroadcastDeskriptor, F_ULOCK, 0L);
	return;
      }
    *blank=0;
    blank++;
    notsave_time=atoi(string);/*problem to distinguish illegal number and 0-number:*/
    if(notsave_time==0) 
      {
	if(! (string[0] - '0'== 0)&&(string[1] - ' '== 0) )
	  {
	    if(DEBUG) printf("Real ! zerro=%s\n",string);	
	    notsave_time =-1;
	  }
	else {if(DEBUG) printf("Real zerro=%s\n",string); }
      }

    if( (notsave_time < 0) || (notsave_time > max_not_save_time) ) 
      {
	createDialog(ar->form_main,XmDIALOG_INFORMATION,"time so big!!!"," ");
	lockf(messBroadcastDeskriptor, F_ULOCK, 0L);
	return;     
      }

    if(notsave_time != 0)
      {
	sprintf(buff,"%d %s %s\nDate is %sFROM: User=%s [%s] host=%s display=%s",notsave_time,
		rebootString, blank,ctime(&time_tmp),userID.loginid,
		userID.real_world_name,userID.myhostname,userID.displayName);
      }
    else
      {
	sprintf(buff,"%s\nDate is %sFROM: User=%s [%s] host=%s display=%s",
		blank,ctime(&time_tmp),userID.loginid,
		userID.real_world_name,userID.myhostname,userID.displayName);
      }

    fprintf(fp,"%ld\n%s",timeID,buff);
    createDialog(ar->form_main,XmDIALOG_MESSAGE,"Broadcast Message: \n""\n""\n",buff);

    fclose(fp);	    
    XtFree(string);
    amIsender=1;
    XtAppAddTimeOut(appContext,messBroadcastLockDelay,messBroadcastFileUnlock,NULL);
    
}

static void messBroadcastFileUnlock()
{
  FILE *fp;
  if(DEBUG) fprintf(stderr,"delete messBroadcastInfoFileName...\n");
  if ( (fp=fopen(messBroadcastInfoFileName,"w")) == NULL )
    {
      perror("Loop:can't open messBroadcastInfoFileName!!!!");
    }
  if (fp) fclose (fp);
  amIsender=0;
  lockf(messBroadcastDeskriptor, F_ULOCK, 0L);
}

static void helpMessBroadcast(Widget w,XtPointer item,XtPointer cbs)
{
createDialog(XtParent(w),XmDIALOG_INFORMATION,
"This message will be send all other operators\n"
"which work with the same alh configuration.\n"
"\n"
"1) If you choose 'Stop Logging Message'\n"
"           first symbol must be a number:\n"
"\n"
"         ''time_in_min ''any other text ''  ''\n"
"\n"
"         After that  ALH will NOT save alarms during 'time_in_min'\n"
"\n"
"         It's very usefull before rebooting IOC \n"
"         (all other will know about that) \n"
"         or if you don't like save alarms during this short\n"
"         (no more than 10 min) time.\n"
"\n"
"\n"
"\n"
"2) If you choose 'Reload'-part all alh-proceses (including your current process)\n"
"will reload config file.\n"
"\n"
"\n"
"\n"
"3) If you choose 'Common Message'-part\n"
"         it's mean any other information messages without stop logging.\n"
"\n"
"         You can use it for any other communications\n" 
"         (like Hi All!, Need Help!,No meeting today!, ...)",
"");
}

static void cancelMessBroadcast(Widget w)
{
XtUnmanageChild(w);
 lockf(messBroadcastDeskriptor, F_ULOCK, 0L);
}

#endif
#endif
 
/******************************************************
  alhSetupCallback
******************************************************/
static void alhSetupCallback( Widget widget, XtPointer calldata, XtPointer cbs)
{
	int item=(long)calldata;
	ALINK      *area;
	XmString silence_string;

	XtVaGetValues(widget, XmNuserData, &area, NULL);

	switch (item){

	case MENU_SETUP_FILTER_NONE:
		area->viewFilter = alFilterAll;
		createConfigDisplay(area,EXPANDCOLLAPSE1);
		break;

	case MENU_SETUP_FILTER_ACTIVE:
		area->viewFilter = alFilterAlarmsOnly;
		createConfigDisplay(area,EXPANDCOLLAPSE1);
		break;

	case MENU_SETUP_FILTER_UNACK:
		area->viewFilter = alFilterUnackAlarmsOnly;
		createConfigDisplay(area,EXPANDCOLLAPSE1);
		break;

	case MENU_SETUP_BEEP_MINOR:

		psetup.beepSevr = MINOR_ALARM;
		changeBeepSeverityText(area);
		break;

	case MENU_SETUP_BEEP_MAJOR:

		psetup.beepSevr = MAJOR_ALARM;
		changeBeepSeverityText(area);
		break;

	case MENU_SETUP_BEEP_INVALID:

		psetup.beepSevr = INVALID_ALARM;
		changeBeepSeverityText(area);
		break;

	case MENU_SETUP_SILENCE_INTERVAL_5:

		if (area->silenceMinutes != 5) {
			area->silenceMinutes=5;
			silenceSelectedMinutesReset(area);
		}
		silence_string = XmStringCreateLocalized ("Silence 5 minutes");
		XtVaSetValues(area->silenceSelectedMinutes, XmNlabelString, silence_string, NULL);
		XmStringFree (silence_string);
		alLogOpModMessage(0,0,"Silence interval set to 5 minutes");
		break;

	case MENU_SETUP_SILENCE_INTERVAL_10:

		if (area->silenceMinutes != 10) {
			area->silenceMinutes=10;
			silenceSelectedMinutesReset(area);
		}
		silence_string = XmStringCreateLocalized ("Silence 10 minutes");
		XtVaSetValues(area->silenceSelectedMinutes, XmNlabelString, silence_string, NULL);
		XmStringFree (silence_string);
		alLogOpModMessage(0,0,"Silence interval set to 10 minutes");
		break;

	case MENU_SETUP_SILENCE_INTERVAL_15:

		if (area->silenceMinutes != 15) {
			area->silenceMinutes=15;
			silenceSelectedMinutesReset(area);
		}
		silence_string = XmStringCreateLocalized ("Silence 15 minutes");
		XtVaSetValues(area->silenceSelectedMinutes, XmNlabelString, silence_string, NULL);
		XmStringFree (silence_string);
		alLogOpModMessage(0,0,"Silence interval set to 15 minutes");
		break;

	case MENU_SETUP_SILENCE_INTERVAL_30:

		if (area->silenceMinutes != 30) {
			area->silenceMinutes=30;
			silenceSelectedMinutesReset(area);
		}
		silence_string = XmStringCreateLocalized ("Silence 30 minutes");
		XtVaSetValues(area->silenceSelectedMinutes, XmNlabelString, silence_string, NULL);
		XmStringFree (silence_string);
		alLogOpModMessage(0,0,"Silence interval set to 30 minutes");
		break;

	case MENU_SETUP_SILENCE_INTERVAL_60:

		if (area->silenceMinutes != 60) {
			area->silenceMinutes=60;
			silenceSelectedMinutesReset(area);
		}
		silence_string = XmStringCreateLocalized ("Silence 1 hour");
		XtVaSetValues(area->silenceSelectedMinutes, XmNlabelString, silence_string, NULL);
		XmStringFree (silence_string);
		alLogOpModMessage(0,0,"Silence interval set to 1 hour");
		break;

	case MENU_SETUP_SILENCE_FOREVER:

		silenceForeverChangeState(area);
		break;

	case MENU_SETUP_ALARMLOG:

		/* New Name for Alarm Log File  */
		createFileDialog(area->form_main,
		    (void *)fileSetupCallback,(XtPointer)FILE_ALARMLOG,
		    (void *)XtUnmanageChild,(XtPointer)0,
		    (XtPointer)area, 
		    "Alarm Log File",ALARMLOG_PATTERN,psetup.logDir);
		break;

	case MENU_SETUP_OPMOD:

		/* New Name for Operations Log File  */
		createFileDialog(area->form_main,
		    (void *)fileSetupCallback,(XtPointer)FILE_OPMOD,
		    (void *)XtUnmanageChild,(XtPointer)0,
		    (XtPointer)area,
		    "Operator Modification File", OPMOD_PATTERN,psetup.logDir);
		break;

	}
}

 
/******************************************************
  alhHelpCallback
******************************************************/
static void alhHelpCallback(Widget widget,XtPointer calldata,XtPointer cbs)
{
	int item=(long)calldata;
	ALINK  *area;

	XtVaGetValues(widget, XmNuserData, &area, NULL);

	switch (item){

#ifdef ALH_HELP_URL
	case MENU_HELP_HELP:

		callBrowser(ALH_HELP_URL);
		break;
#endif

	case MENU_HELP_ABOUT:

		createDialog(area->form_main,XmDIALOG_INFORMATION,
		    "\nAlarm Handler\n\n" ALH_CREDITS_STRING ,alhVersionString);

		break;

	default:

		createDialog(area->form_main,XmDIALOG_INFORMATION,
		    "Help is not available in this release."," ");
		break;
	}
}

 
/******************************************************
  awRowWidgets
******************************************************/
void awRowWidgets(struct anyLine *line,void *area)
{

	void *subWindow;
	XmString str;
	WLINE  *wline;
	void *link;
	GLINK *glink;
	Position nextX;
	Dimension width;
	Widget parent;
	Pixel backgroundColor=1;
        Pixel bgMask;
        
	subWindow=line->pwindow;
	parent = ((struct subWindow *)subWindow)->drawing_area;
	wline=(WLINE *)line->wline;
	link = line->link;
	glink = (GLINK *)line->link;

	/* create row widgets */
	if (wline->row_widget == NULL) {


		wline->row_widget = XtVaCreateWidget(NULL,
		    xmDrawingAreaWidgetClass,  parent,
		    XmNy,                        calcRowYValue(subWindow,line->lineNo),
		    XmNnavigationType,           XmNONE,
		    XmNorientation,              XmHORIZONTAL,
		    XmNmarginHeight,             0,
		    XmNmarginWidth,              0,
		    (XtPointer)NULL);
		nextX = 0;

		XtVaGetValues(parent,XmNbackground,&backgroundColor,NULL);
		XtVaSetValues(wline->row_widget,XmNbackground,backgroundColor,NULL);

		if ( isTreeWindow(area,subWindow) && line->linkType == GROUP) {
			if ( glink->pgroupData->treeSym) {
				str = XmStringCreateSimple(glink->pgroupData->treeSym);
				wline->treeSym = XtVaCreateManagedWidget("treeSym",
				    xmLabelWidgetClass,        wline->row_widget,
				    XmNlabelString,            str,
				    XmNmarginHeight,           0,
				    NULL);
				XmStringFree(str);
				XtVaGetValues(wline->treeSym,XmNwidth,&width,NULL);
				nextX = width + 3;
			}
		}

		str = XmStringCreateSimple(bg_char[line->unackSevr]);
		wline->ack = XtVaCreateManagedWidget("ack",
		    xmPushButtonWidgetClass,   wline->row_widget,
		    XmNmarginHeight,           0,
		    XmNlabelString,            str,
		    XmNsensitive,              FALSE,
		    XmNuserData,               (XtPointer)area,
		    XmNx,                      nextX,
		    NULL);
		XmStringFree(str);
		XtAddCallback(wline->ack, XmNactivateCallback, 
		    (XtCallbackProc)ack_callback, line);
		XtVaGetValues(wline->ack,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;


		str = XmStringCreateSimple(bg_char[line->curSevr]);
		wline->sevr = XtVaCreateManagedWidget("sevr",
		    xmLabelWidgetClass,        wline->row_widget,
		    XmNlabelString,            str,
		    XmNx,                      nextX,
		    XmNy,                      2,
		    NULL);
		XmStringFree(str);
		XtVaGetValues(wline->sevr,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		/* load actions once */
                if ( g_transInit ) {
                  g_transInit = 0;
                  g_parsedTrans = XtParseTranslationTable( g_dragTrans );
                  XtAppAddActions( appContext, g_dragActions, XtNumber(g_dragActions) );
                }

		str = XmStringCreateSimple(line->alias);
		wline->name = XtVaCreateManagedWidget("pushButtonName",
		    xmPushButtonWidgetClass,   wline->row_widget,
		    XmNmarginHeight,           0,
		    XmNlabelString,            str,
		    XmNuserData,               (XtPointer) line,
		    XmNx,                      nextX,
		    NULL);
		XmStringFree(str);
                XtOverrideTranslations( wline->name, g_parsedTrans );
		if (line->linkType == CHANNEL) {
#if  XmVersion && XmVersion >= 1002
			XmChangeColor(wline->name,channel_bg_pixel);
#else
			XtVaSetValues(wline->name,XmNbackground,channel_bg_pixel,NULL);
#endif
		}
		if ( isTreeWindow(area,subWindow) ) {
			XtAddCallback(wline->name, XmNactivateCallback,
			    (XtCallbackProc)nameTreeW_callback, line);
		} else {
			XtAddCallback(wline->name, XmNactivateCallback,
			    (XtCallbackProc)nameGroupW_callback, line);
		}
		XtVaGetValues(wline->name,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		wline->arrow = XtVaCreateWidget("pushButtonArrow",
		    xmArrowButtonWidgetClass,   wline->row_widget,
		    XmNshadowThickness,        0,
		    XmNarrowDirection,         XmARROW_RIGHT,
		    XmNuserData,               (XtPointer)area,
		    XmNx,                      nextX,
		    XmNy,                      2,
		 /*   XmNbackground,             backgroundColor,*/
		    (XtPointer)NULL);
		if (line->linkType == GROUP && sllFirst(&(glink->subGroupList))){
			XtManageChild(wline->arrow);
			if ( isTreeWindow(area,subWindow) ) {
				XtAddCallback(wline->arrow, XmNactivateCallback,
				    (XtCallbackProc)arrowTreeW_callback, link);
			} else {
				XtAddCallback(wline->arrow, XmNactivateCallback,
				    (XtCallbackProc)arrowGroupW_callback, link);
			}
			XtVaGetValues(wline->arrow,XmNwidth,&width,NULL);
			nextX = nextX + width + 3;
		}

		wline->guidance = XtVaCreateWidget("G",
		    xmPushButtonWidgetClass,   wline->row_widget,
		    XmNmarginHeight,           0,
		    XmNuserData,               (XtPointer)line->alias,
		    XmNx,                      nextX,
		/*    XmNbackground,             backgroundColor,*/
		    NULL);

		if (guidanceExists(link)) {
			XtManageChild(wline->guidance);
			XtAddCallback(wline->guidance, XmNactivateCallback, 
			    (XtCallbackProc)guidanceCallback, link);
			XtVaGetValues(wline->guidance,XmNwidth,&width,NULL);
			nextX = nextX + width + 3;
		}

		wline->process = XtVaCreateWidget("P",
		    xmPushButtonWidgetClass,   wline->row_widget,
		    XmNmarginHeight,           0,
		    XmNuserData,               (XtPointer)area,
		    XmNx,                      nextX,
		  /*  XmNbackground,             backgroundColor, */
		    NULL);

		if (alProcessExists(link) ){
			XtManageChild(wline->process);
			XtAddCallback(wline->process, XmNactivateCallback, 
			    (XtCallbackProc)relatedProcess_callback, link);
			XtVaGetValues(wline->process,XmNwidth,&width,NULL);
			nextX = nextX + width + 3;
		}

                /* A.Luedeke : Added color when mask is silencing: 'C', 'D', 'A' or 'H' */
                if (_mask_color_flag&&(line->mask[1]!='-'||line->mask[2]!='-'||line->mask[3]!='-')) {bgMask=noack_bg_pixel;} else {bgMask=bg_pixel[0];} /* A.L.: added color */
		str = XmStringCreateSimple(line->mask);
		wline->mask = XtVaCreateManagedWidget("mask",
		    xmLabelWidgetClass,        wline->row_widget,
		    XmNlabelString,            str,
		    XmNx,                      nextX,
		    XmNy,                      2,
	/*	    XmNbackground,             backgroundColor,*/
		    NULL);
		XmStringFree(str);
		XtVaGetValues(wline->mask,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;
#if  XmVersion && XmVersion >= 1002
		XmChangeColor(wline->mask,bgMask); /* A.L.: added color */
#else
		XtVaSetValues(wline->mask,XmNbackground,bgMask,NULL); /* A.L.: added color */
#endif

		str = XmStringCreateSimple(line->highestBeepSevrString);
		wline->highestbeepsevr = XtVaCreateManagedWidget("highestbeepsevr",
		    xmLabelWidgetClass,        wline->row_widget,
		    XmNlabelString,            str,
		    XmNx,                      nextX,
		    XmNy,                      2,
		    XmNbackground,             backgroundColor,
		    NULL);
		XmStringFree(str);
		XtVaGetValues(wline->highestbeepsevr,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		str = XmStringCreateSimple(line->message);
		wline->message = XtVaCreateManagedWidget("message",
		    xmLabelWidgetClass,        wline->row_widget,
		    XmNlabelString,            str,
		    XmNx,                      nextX,
		    XmNy,                      2,
/*		    XmNbackground,             backgroundColor, */
		    NULL);
		XmStringFree(str);
		XtVaGetValues(wline->message,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		awUpdateRowWidgets(line);

                if (psetup.silenceSelectedMinutes) {
                    changeTreeColor(wline->row_widget,silenced_bg_pixel);
                } else {
                    changeTreeColor(wline->row_widget,bg_pixel[0]);
                }

		XtManageChild(wline->row_widget);
	}

	else

	/* else modify existing  row widgets */
	{
		if ( wline->row_widget && XtIsManaged(wline->row_widget)){
			XtUnmanageChild(wline->row_widget);
		}

		nextX = 0;
		if ( isTreeWindow(area,subWindow) && line->linkType == GROUP) {
			str = XmStringCreateSimple(glink->pgroupData->treeSym);
			XtVaSetValues(wline->treeSym,
			    XmNlabelString,            str,
			    NULL);
			XmStringFree(str);
			XtVaGetValues(wline->treeSym,XmNwidth,&width,NULL);
			nextX = width + 3;
		}

		if (line->unackSevr == FALSE) {
			XtVaSetValues(wline->ack,
			    XmNsensitive,            FALSE,
			    NULL);
		}
		else {
			XtVaSetValues(wline->ack,
			    XmNsensitive,            TRUE,
			    NULL);
		}

		XtVaSetValues(wline->ack,XmNx,nextX,NULL);
		XtRemoveAllCallbacks(wline->ack, XmNactivateCallback);
		XtAddCallback(wline->ack, XmNactivateCallback, 
		    (XtCallbackProc)ack_callback, line);
		XtVaGetValues(wline->ack,XmNwidth,&width,NULL);
		nextX = nextX + width +3;


		XtVaSetValues(wline->sevr,XmNx,nextX,NULL);
		XtVaGetValues(wline->sevr,XmNwidth,&width,NULL);
		nextX = nextX + width +3;

		str = XmStringCreateSimple(line->alias);
		XtVaSetValues(wline->name,
		    XmNlabelString,            str,
		    XmNx,                      nextX,
		    NULL);
		XmStringFree(str);
		if ( ! isTreeWindow(area,subWindow)) {
			if (line->linkType == CHANNEL) {
#if  XmVersion && XmVersion >= 1002
				XmChangeColor(wline->name,channel_bg_pixel);
#else
				XtVaSetValues(wline->name,XmNbackground,channel_bg_pixel,NULL);
#endif
			}
			if (line->linkType == GROUP) {
#if  XmVersion && XmVersion >= 1002
				XmChangeColor(wline->name,bg_pixel[0]);
#else
				XtVaSetValues(wline->name,XmNbackground,bg_pixel[0],NULL);
#endif
			}
		}
		XtRemoveAllCallbacks(wline->name, XmNactivateCallback);
		if ( isTreeWindow(area,subWindow) ) {
			XtAddCallback(wline->name, XmNactivateCallback,
			    (XtCallbackProc)nameTreeW_callback, line);
		} else {
			XtAddCallback(wline->name, XmNactivateCallback,
			    (XtCallbackProc)nameGroupW_callback, line);
		}
		XtVaGetValues(wline->name,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		if (XtHasCallbacks(wline->arrow,XmNactivateCallback))
			XtRemoveAllCallbacks(wline->arrow, XmNactivateCallback);
		if (line->linkType == GROUP && sllFirst(&(glink->subGroupList))){
			XtVaSetValues(wline->arrow,XmNx,nextX,NULL);
			XtManageChild(wline->arrow);
			if ( isTreeWindow(area,subWindow) ) {
				XtAddCallback(wline->arrow, XmNactivateCallback,
				    (XtCallbackProc)arrowTreeW_callback, link);
			} else {
				XtAddCallback(wline->arrow, XmNactivateCallback,
				    (XtCallbackProc)arrowGroupW_callback, link);
			}
			XtVaGetValues(wline->arrow,XmNwidth,&width,NULL);
			nextX = nextX + width +3;

		} else {
			XtUnmanageChild(wline->arrow);
		}

		if (XtHasCallbacks(wline->guidance,XmNactivateCallback))
			XtRemoveAllCallbacks(wline->guidance, XmNactivateCallback);
		if (guidanceExists(link)) {
			XtVaSetValues(wline->guidance,XmNx,nextX,NULL);
			XtVaSetValues(wline->guidance,XmNuserData,line->alias,NULL);
			XtManageChild(wline->guidance);
			XtAddCallback(wline->guidance, XmNactivateCallback, 
			    (XtCallbackProc)guidanceCallback, link);
			XtVaGetValues(wline->guidance,XmNwidth,&width,NULL);
			nextX = nextX + width + 3;
		} else {
			XtUnmanageChild(wline->guidance);
		}

		if (XtHasCallbacks(wline->process,XmNactivateCallback))
			XtRemoveAllCallbacks(wline->process, XmNactivateCallback);
		if (alProcessExists(link) ){
			XtVaSetValues(wline->process,XmNx,nextX,NULL);
			XtManageChild(wline->process);
			XtAddCallback(wline->process, XmNactivateCallback, 
			    (XtCallbackProc)relatedProcess_callback, link);
			XtVaGetValues(wline->process,XmNwidth,&width,NULL);
			nextX = nextX + width + 3;
		} else {
			XtUnmanageChild(wline->process);
		}

		XtVaSetValues(wline->mask,XmNx,nextX,NULL);
		XtVaGetValues(wline->mask,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		XtVaSetValues(wline->highestbeepsevr,XmNx,nextX,NULL);
		XtVaGetValues(wline->highestbeepsevr,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		XtVaSetValues(wline->message,XmNx,nextX,NULL);
		XtVaGetValues(wline->message,XmNwidth,&width,NULL);
		nextX = nextX + width + 3;

		awUpdateRowWidgets(line);

		XtManageChild(wline->row_widget);

	}

}


/******************************************************
  awUpdateRowWidgets
******************************************************/
void awUpdateRowWidgets(struct anyLine *line)
{
	XmString str;
	WLINE *wline;
	Pixel bg;
        Pixel bgMask;
        Pixel backgroundColor;
	XmString strOld;
	Boolean sensitiveOld;

	wline=(WLINE *)line->wline;

	XtVaGetValues(wline->ack,
	    XmNbackground,            &bg,
	    XmNlabelString,           &strOld,
	    XmNsensitive,             &sensitiveOld,
	    (XtPointer)NULL);

	if (line->unackSevr == FALSE) {
		if (sensitiveOld == TRUE)
			XtVaSetValues(wline->ack,
			    XmNsensitive,            FALSE,
			    NULL);
	}
	else {
		if (sensitiveOld == FALSE)
			XtVaSetValues(wline->ack,
			    XmNsensitive,            TRUE,
			    NULL);
	}
	str = XmStringCreateSimple(bg_char[line->unackSevr]);
	if (!XmStringCompare(str,strOld))
		XtVaSetValues(wline->ack,
		    XmNlabelString,            str,
		    NULL);
	XmStringFree(strOld);
	XmStringFree(str);

	if (bg != bg_pixel[line->unackSevr]){
#if  XmVersion && XmVersion >= 1002
		XmChangeColor(wline->ack,bg_pixel[line->unackSevr]);
#else
		XtVaSetValues(wline->ack,XmNbackground,bg_pixel[line->unackSevr],NULL);
#endif
	}

	XtVaGetValues(wline->sevr,
	    XmNbackground,            &bg,
	    XmNlabelString,           &strOld,
	    NULL);

	str = XmStringCreateSimple(bg_char[line->curSevr]);
	if (!XmStringCompare(str,strOld))
		XtVaSetValues(wline->sevr,
		    XmNlabelString,            str,
		    NULL);
	XmStringFree(str);
	XmStringFree(strOld);

	if (bg != bg_pixel[line->curSevr]){
#if  XmVersion && XmVersion >= 1002
		XmChangeColor(wline->sevr,bg_pixel[line->curSevr]);
#else
		XtVaSetValues(wline->sevr,XmNbackground,bg_pixel[line->curSevr],NULL);
#endif
	}

	XtVaGetValues(wline->mask,
	    XmNlabelString,           &strOld,
            XmNbackground,            &backgroundColor,
	    NULL);

	str = XmStringCreateSimple(line->mask);
        /* A.Luedeke : Added color when mask is silencing: 'C', 'D', 'A' or 'H' */

        if (_mask_color_flag&&(line->mask[1]!='-'||line->mask[2]!='-'||line->mask[3]!='-')) {bgMask=noack_bg_pixel;} else {bgMask=bg_pixel[0];} /* A.L.: added color */
	if (!XmStringCompare(str,strOld)) {
		XtVaSetValues(wline->mask,
		    XmNlabelString,            str,
		    NULL);
#if  XmVersion && XmVersion >= 1002
		XmChangeColor(wline->mask,bgMask); /* A.L.: added color */
#else
		XtVaSetValues(wline->mask,XmNbackground,bgMask,NULL); /* A.L.: added color */
#endif
        }
	XmStringFree(str);
	XmStringFree(strOld);

	XtVaGetValues(wline->message,
	    XmNlabelString,           &strOld,
	    NULL);
	str = XmStringCreateSimple(line->message);
	if (!XmStringCompare(str,strOld))
		XtVaSetValues(wline->message,
		    XmNlabelString,            str,
		    NULL);
	XmStringFree(str);
	XmStringFree(strOld);

	XtVaGetValues(wline->highestbeepsevr,
	    XmNlabelString,           &strOld,
	    NULL);
	str = XmStringCreateSimple(line->highestBeepSevrString);
	if (!XmStringCompare(str,strOld))
		XtVaSetValues(wline->highestbeepsevr,
		    XmNlabelString,            str,
		    NULL);
	XmStringFree(str);
	XmStringFree(strOld);

}


/*+**************************************************************************
 *
 * Function:	awCMLOGstartBrowser
 *
 * Description:	Starts the standard Motif CMLOG browser
 *
 **************************************************************************-*/

#ifdef CMLOG
static void awCMLOGstartBrowser (void)
{
   char command[256];
   
   switch (fork()) {
				/* Error */
   case -1:
      perror("Cannot fork() to start CMLOG browser");
      return;
				/* Child */
   case 0:
      if (psetup.configDir)
	 sprintf(command, CMLOG_BROWSER" -f %s/"CMLOG_CONFIG" &", psetup.configDir);
      else
	 sprintf(command, CMLOG_BROWSER" -f "CMLOG_CONFIG" &");

      system(command);
      exit(0);
      break;
				/* Parent */
   default:
      break;
   }
   return;
}
#endif
