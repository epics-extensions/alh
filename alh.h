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
/* alh.h */

/************************DESCRIPTION***********************************
 Alh header file of #defines and structure definitions
**********************************************************************/

#ifndef INCalhh
#define INCalhh

#include <stdio.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>

#include "alarm.h"
#include "cadef.h"

#ifdef EXCEED
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
#endif /* #ifdef WIN32 */
#endif

#include "sllLib.h"
#include "dbDefs.h"

#ifndef FLDNAME_SZ
#define FLDNAME_SZ 4  /*Field Name Size*/
#endif

/* ------- Definitions for channels in error state ----- */
/* ALARM_NSEV and ALARM_NSTATUS defined in epics/base alarm.h */

#define ERROR_STATE      ALARM_NSEV
#define ALH_ALARM_NSEV     ALARM_NSEV + 1

#define NOT_CONNECTED            ALARM_NSTATUS
#define NO_READ_ACCESS           ALARM_NSTATUS +1
#define NO_WRITE_ACCESS          ALARM_NSTATUS +2
#define ALH_ALARM_NSTATUS  ALARM_NSTATUS + 3
/* ----------------------------------------------------- */

/* size of name */
#define NAMEDEFAULT_SIZE  150           /* file name size  */
#define OPMODMESSAGE_SIZE 130           /* opmod log line size */
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
#define INITIAL_FILE_LENGTH     204800

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

/* define parameters for menu callbacks */
#define MENU_EDIT_UNDO_PASTE_NOSELECT   10700
#define MENU_EDIT_UNDO_PASTE            10701
#define MENU_EDIT_UNDO_CUT              10702
#define MENU_EDIT_UNDO_CUT_NOSELECT     10703
#define MENU_EDIT_UNDO_UPDATE_CLIPBOARD 10704
#define MENU_EDIT_UNDO_PROPERTIES       10705
#define MENU_EDIT_UNDO_CLEAR            10706

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

#define MAX_STRING_LENGTH	500

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
	void (*callback)(Widget, XtPointer, XtPointer);
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

struct heartbeatPV {
    char *name;
    chid chid;
    float caputRateInSeconds;
    short caputValue;
    XtIntervalId timerId;
};

struct setup {
	char configFile[NAMEDEFAULT_SIZE];      /* config file name */
	char logFile[NAMEDEFAULT_SIZE];         /* alarm log file name */
	char opModFile[NAMEDEFAULT_SIZE];       /* opMod log file name */
	char saveFile[NAMEDEFAULT_SIZE];        /* save config file name */
	short silenceForever;                   /* 1 - beepoff forever is true */
	short silenceOneHour;                   /* 1 - beepoff one hour is true */
	short silenceCurrent;                     /* 1 - current beep on  0 - off */
	short beepSevr;                 /* 1,2,3,4,5 */
	short highestSevr;              /* system highest  sevr */
	short highestUnackSevr;         /* system highest unack sevr */
	short highestUnackBeepSevr;     /* system highest unack sevr >= beep sevr */
	short newUnackBeepSevr;         /* new unack sevr after beep sevr tests */
	char *configDir;                /* config files directory */
	char *logDir;                   /* log files directory */
};

struct mainGroup {
	struct groupLink *p1stgroup;    /* main group pointer */
	int modified;
	void *area;
	struct heartbeatPV heartbeatPV; /* heartbeat PV info */
};

extern struct setup psetup;

/* log message types */
#define REGULAR_RECORD      1  /* usual alLog mess. */
#define CONNECT_ALARM       2  /* specific alLog format about connection lost */
#define STOP_LOGGING_ALARM  3  /* specific alLog format about stopping loging */
#define MESSAGE_QUEUE_ALARM 4  /* specific alLog format about MQ problems */
#define ACK_CHANNEL         5  /*  op_mod action which will be saving in DB */
#define ACK_GROUP           6  /*  Don't used now  */
#define CHANGE_MASK         7  /*  op_mod action which will be saving in DB */
#define CHANGE_MASK_GROUP   8  /*  op_mod action which will be saving in DB */
#define FORCE_MASK          9  /*  op_mod action which will be saving in DB */
#define FORCE_MASK_GROUP   10  /*  op_mod action which will be saving in DB */
#define ALARM_LOG_DB 1 /* We are write to ALARM_LOG database */
#define OP_MOD_DB    2 /* We are write to OP_MOD    database */



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
     line.h           Defs of anyLine,WLINE
     sllLib.h         Defs of SNODE,SLIST, sllFirst,sllInit,sllLast,sllNext

**************************************************************************************/
