/*
 $Log$
 Revision 1.8  1998/07/29 17:27:35  jba
 Added "Unacknowledged Alarms Only" display filter.

 Revision 1.7  1998/06/02 19:40:47  evans
 Changed from using Fgmgr to using X to manage events and file
 descriptors.  (Fdmgr didn't work on WIN32.)  Uses XtAppMainLoop,
 XtAppAddInput, and XtAppAddTimeOut instead of Fdmgr routines.
 Updating areas is now in alCaUpdate, which is called every caDelay ms
 (currently 100 ms).  Added a general error message routine (errMsg)
 and an exception handler (alCAException).  Is working on Solaris and
 WIN32.

 Revision 1.6  1998/05/12 18:22:42  evans
 Initial changes for WIN32.

 Revision 1.5  1997/09/09 22:20:03  jba
 Changed HELP selections on menu.

 Revision 1.4  1995/11/13 22:31:15  jba
 Added beepseverity command, ansi changes and other changes.

 * Revision 1.3  1995/10/20  16:50:07  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 *
 * Revision 1.2  1994/06/22  21:16:48  jba
 * Added cvs Log keyword
 *
 */

/* alh.h */

/* alh.h - Alarm Handler 
 *
 *      Author: 	Ben-chin Cha 
 *      Date:		05-04-92
 *
 *	Experimental Physics and Industrial Control System (EPICS)
 *
 *	Copyright 1991, the Regents of the University of California,
 *	and the University of Chicago Board of Governors.
 *
 *	This software was produced under  U.S. Government contracts:
 *	(W-7405-ENG-36) at the Los Alamos National Laboratory,
 *	and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *	Initial development by:
 *		The Controls and Automation Group (AT-8)
 *		Ground Test Accelerator
 *		Accelerator Technology Division
 *		Los Alamos National Laboratory
 *
 *	Co-developed with
 *		The Controls and Computing Group
 *		Accelerator Systems Division
 *		Advanced Photon Source
 *		Argonne National Laboratory
 *
 *
 * Modification Log:
 * -----------------
 * .nn	mm-dd-yy		nnn	Description
 * .01  12-10-93        jba modified FILE_ defines added configDir,logdDir
 *                      and fixed endif comment
 * .02  02-17-94        use dbDefs.h to define pvname and field name lengths
 */

#ifndef INCalhh
#define INCalhh

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>

#include <sllLib.h>

#include <dbDefs.h>

/* WIN32 differences */
#ifdef WIN32
/* Hummingbird extra functions including lprintf
 *   Needs to be included after Intrinsic.h for Exceed 5
 *   (Intrinsic.h is included in xtParams.h) */
# include <X11/XlibXtra.h>
/* Needed for access */
# include <io.h>
/* This is done in Exceed 6 but not in Exceed 5
 *   Need it to define printf as lprintf for Windows
 *   (as opposed to Console) apps */
# ifdef _WINDOWS
#  ifndef printf
#   define printf lprintf
#  endif
# endif
#else /* #ifdef WIN32 */
/* WIN32 does not have unistd.h */
# include <unistd.h>
#endif /* #ifdef WIN32 */
 
static char *alhhSccsId = "@(#)alh.h	1.10\t10/8/93";

/* default  file names */
#define DEFAULT_CONFIG  "ALH-default.alhConfig"
#define DEFAULT_ALARM   "ALH-default.alhAlarm"
#define DEFAULT_OPMOD   "ALH-default.alhOpmod"
#define NEW_CONFIG      "ALH-new.alhConfig"

/* size of name */
#define NAMEDEFAULT_SIZE  150           /* file name size  */
#define OPMODMESSAGE_SIZE 130           /* opmod log line size */
#define LINEMESSAGE_SIZE   60           /* window message line size */
#define PVNAME_SIZE  PVNAME_STRINGSZ+FLDNAME_SZ       /* PV name size from dbDefs.h */


/* parameters for mask selection */
#define ALARM_NMASK     5
#define ALARMCANCEL     0
#define ALARMDISABLE    1
#define ALARMACK        2
#define ALARMACKT       3
#define ALARMLOG        4

#define MASK_OFF        0
#define MASK_ON         1
#define MASK_RESET      2

/* parameters for file specifications */
#define CONFIG_FILE     0
#define ALARM_FILE      1
#define OPMOD_FILE      2
#define N_LOG_FILES     3
#define INITIAL_FILE_LENGTH     2048

/* parameters for fileSetupCallback */
#define FILE_CONFIG			0
#define FILE_CONFIG_INSERT	2
#define FILE_SAVE			3
#define FILE_SAVEAS			4
#define FILE_SAVEAS_OK		5
#define FILE_ALARMLOG		6
#define FILE_OPMOD			7
#define FILE_PRINT			8

#define	MAX_TREE_DEPTH			50

#define CA_PEND_IO_SECONDS  5.0

/* define parameters for menu callbacks */
#define MENU_FILE_NEW		10100
#define MENU_FILE_OPEN		10101
#define MENU_FILE_OPEN_OK	10102
#define MENU_FILE_CLOSE		10103
#define MENU_FILE_CLOSEALL	10104
#define MENU_FILE_SAVE		10105
#define MENU_FILE_SAVEAS	10106
#define MENU_FILE_ALH		10107
#define MENU_FILE_PRINT		10108
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


#define	MENU_VIEW_CONFIG		10400
#define	MENU_VIEW_OPMOD			10401
#define	MENU_VIEW_ALARMLOG		10402
#define	MENU_VIEW_CURRENT		10403

#define MENU_SETUP_BEEP_MINOR	10500
#define MENU_SETUP_BEEP_MAJOR	10501
#define MENU_SETUP_BEEP_INVALID	10502
#define MENU_SETUP_FILTER_NONE	10503
#define MENU_SETUP_FILTER_ACTIVE	10504
#define MENU_SETUP_FILTER_UNACK 	10505
#define	MENU_SETUP_ALARMLOG		10506
#define	MENU_SETUP_OPMOD		10507

#define MENU_EDIT_UNDO			10600
#define MENU_EDIT_CUT			10601
#define MENU_EDIT_COPY			10602
#define MENU_EDIT_PASTE			10603
#define MENU_EDIT_CLEAR			10604
#define MENU_EDIT_CLEAR_OK		10605

#define MENU_EDIT_UNDO_PASTE_NOSELECT	10700
#define MENU_EDIT_UNDO_PASTE			10701
#define MENU_EDIT_UNDO_CUT				10702
#define MENU_EDIT_UNDO_CUT_NOSELECT		10703
#define MENU_EDIT_UNDO_UPDATE_CLIPBOARD	10704
#define MENU_EDIT_UNDO_PROPERTIES		10705
#define MENU_EDIT_UNDO_CLEAR			10706

#define MENU_INSERT_GROUP		10800
#define MENU_INSERT_CHANNEL		10801
#define MENU_INSERT_INCLUDE		10802
#define MENU_INSERT_FILE		10803
#define MENU_INSERT_SETTINGS	10804

#define MENU_HELP_TOPICS	10900
#define MENU_HELP_ABOUT	10906




typedef  void (*FUNPTR)();      /* define void function pointer */

#ifdef Mmax  /* just in case--we don't know, but these are commonly set */
#undef Mmax  /* by arbitrary unix systems.  Also, we cast to int! */
#endif
/* redefine "max" and "min" macros to take into account "unsigned" values */
#define Mmax(a,b) ((int)(a)>(int)(b)?(int)(a):(int)(b))
#define Mmin(a,b) ((int)(a)<(int)(b)?(int)(a):(int)(b))

#define AUTOMATIC   0
#define OPERATOR    1

#define GROUP     1
#define CHANNEL   2

#define	MAX_STRING_LENGTH		150

/* define commands for modifying the current view */
#define EXPAND		0
#define EXPANDCOLLAPSE1	1
#define COLLAPSE	2
#define NOCHANGE	3

#define ACT  0
#define ALH  1

#define CA_CONNECT_NO   0
#define CA_CONNECT_YES  1

#define CONFIG_PATTERN "*.alhConfig"
#define OPMOD_PATTERN "*.alhOpmod"
#define ALARMLOG_PATTERN "*.alhAlarm"
#define TREEREPORT_PATTERN "*.alhReport"

typedef struct {
    char *label;
    void (*callback)(Widget, void *, void *);
    XtPointer data;
} ActionAreaItem;

#define TIGHTNESS  30

/* ------- start of defines for global variables    */
char               *programName;
SLIST              *areaList;
Display            *display;
XtAppContext        appContext;
Widget              topLevelShell;
Widget              productDescriptionShell;
Pixmap              ALH_pixmap;
int                 programId;


struct setup {
     char configFile[NAMEDEFAULT_SIZE];      /* config file name */
     char logFile[NAMEDEFAULT_SIZE];         /* alarm log file name */
     char opModFile[NAMEDEFAULT_SIZE];       /* opMod log file name */
     char saveFile[NAMEDEFAULT_SIZE];        /* save config file name */
     short nobeep;                   /* 1 - beepoff forever is true */
     short beep;                     /* 1 - beep on  0 - off */
     short beepSevr;                 /* 1,2,3,4,5 */
     short highestSevr;              /* system highest  sevr */
     short highestUnackSevr;         /* system highest unack sevr */
     char *configDir;                /* config files directory */
     char *logDir;                   /* log files directory */
 };

struct mainGroup {
     struct groupLink *p1stgroup;    /* main group pointer */
     int modified;
     void *area;
};

extern struct setup psetup;

/* GLOBAL WIDGET VARIABLES */
Widget toggle_button,toggle_button1;


/*************************************************************************/

/*
 * SOME CONVENIENCE ROUTINES
 */

#if FALSE


#ifndef IGNORE_FONT
static XmFontList
FONT_LIST(w, name)
Widget  w;
char    *name;
{
XFontStruct     *font;

font = XLoadQueryFont(XtDisplay(w), name);
if( font == NULL )
{
    XtWarning("Cannot find font, using default.");
    font = XLoadQueryFont(XtDisplay(w), "fixed");
}
return(XmFontListCreate(font, XmSTRING_DEFAULT_CHARSET)); 
}
#endif

#ifndef IGNORE_PIXMAP
static Pixmap
PIXMAP(w, name)
Widget  w;
char    *name;
{
Pixmap          bitmap;
Pixmap          pixmap;
unsigned long   bg, fg;
Arg                     args[20];
int                     i;
int                     width, height;
int                 xhot, yhot;
GC                      gc;
int                     depth;
Window          window;
window = DefaultRootWindow(XtDisplay(w));
i = 0;
XtSetArg(args[i], XmNbackground, &bg); i++;
XtSetArg(args[i], XmNforeground, &fg); i++;
XtSetArg(args[i], XmNdepth, &depth); i++;
XtGetValues(w, args, i);
if( XReadBitmapFile(XtDisplay(w), window,
   name, &width, &height, &bitmap, &xhot, &yhot)
      != BitmapSuccess) return(XmUNSPECIFIED_PIXMAP);
pixmap = XCreatePixmap(XtDisplay(w), window,
       width, height, depth);
gc = XCreateGC(XtDisplay(w), window, 0, 0);
XSetForeground(XtDisplay(w), gc, fg);
XSetBackground(XtDisplay(w), gc, bg);
XCopyPlane(XtDisplay(w), bitmap, pixmap, gc, 0,
       0, width, height, 0, 0, 0x00000001);
XFreeGC(XtDisplay(w), gc);
XFreePixmap(XtDisplay(w), bitmap);
return(pixmap);
}
#endif

#ifndef IGNORE_MENU_POST
static void
MENU_POST(p, m, e)
Widget  m;
XButtonEvent *e;
{
Arg args[2];
int argcnt;
int button;

    argcnt = 0;
    XtSetArg(args[argcnt], XmNwhichButton, &button);
    argcnt++;
    XtGetValues(m, args, argcnt);
    if( e->button != button) return;
    XmMenuPosition(m, e);
    XtManageChild(m);
}
#endif


#ifndef IGNORE_STRING_TABLE
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef __STDC__
static XmString* STRING_TABLE(int count, XmString *array, ...)
#else
static XmString* STRING_TABLE(va_alist)
va_dcl
#endif
{
    va_list     ap;
#ifndef __STDC__
    int         count;
    XmString    *array;
#endif
    int     i;

#ifdef __STDC__
    va_start(ap, count);
#else
    va_start(ap);
    count = va_arg(ap, int);
#endif
    array = (XmString*)XtMalloc(count * sizeof(XmString*));
    for(i = 0;  i < count; i++ )
    {
	array[i] = XmStringCreateLtoR(va_arg(ap, char*),
	XmSTRING_DEFAULT_CHARSET);
    }
    va_end(ap);

    return(array);
}
#endif

#endif  

#endif /* INCalhh */

/*************************************************************************************

--------------------------------------------------------------------------------------
               -------------- ALH File Information  ----------------
--------------------------------------------------------------------------------------


     NOTE:  See h files for information on specific routines


     ALH.bit          ALH icon definition

     acknowledge.c    Alarm acknowledgement routines
     alCA.c           Config routines for communicating with Channel Access
     alConfig.c       Routines related to read/write of alarm configuration
     alDebug.c        Config debugging routines
     alInitialize.c   Config routines for Alarm configuration initialization
     alLib.c          Config routines to create, delete and modify groups/channels
     alLog.c          Config routines for logging of alarms and messages
     alView.c         Config routines for modifying the configuration view
     alh.c            ALH Main program
     awAct.c          Interface routines for ACT menu, subW line, and callbacks
     awAlh.c          Interface routines for ALH  menu, subW line, and callbacks
     awView.c         X routines for modifying and displaying the config view
     axArea.c         X routines for setup of area and area Main Window
     axRunW.c         X routines related to the iconlike runtime window
     axSubW.c         X routines for tree and groupContents subWindows
     clipboardOps.c   Routines allow cut and paste of pushbutton label
     current.c        Routines for current alarm history (last 10 alarms)
     dialog.c         Routines for creating dialogs
     file.c           Routines for file handling and alh exit
     force.c          Routines for force group/channel masks and forcePV changes
     guidance.c       Routines for displaying group or channel guidance
     help.c           Routines for display of ALH help text
     line.c           Routines for alloc, init, and update of a subWindow line
     mask.c           Routines for modifying group/channel masks
     process.c        Routines for spawning a related process
     scroll.c         Routines for viewing files using a scrolled window
     showmask.c       Routines for showing group/channel masks summary 
     sllLib.c         Routines for single link list operations
     testalarm.c      Routines for generation of a testalarm dialog

     alLib.h          Defs of MASK,GLINK,CLINK,GCLINK,groupData,chanData,gcData
     alarmString.h    String values for alarm status and severity
     alh.h            #defines, masksdata, setup,mainGroup ...
     axArea.h A       Defs of ALINK, MenuItem
     axSubW.h         Defs of subWindow structure
     clipboardOps.h   #defines 
     fallback.h       Def of X fallback resource values
     line.h           Defs of groupLine,chanLine,anyLine,WLINE
     sllLib.h         Defs of SNODE,SLIST, sllFirst,sllInit,sllLast,sllNext

**************************************************************************************/
