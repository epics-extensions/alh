/*
 $Log$
 Revision 1.18  1998/08/05 18:20:07  jba
 Added silenceOneHour button.
 Moved silenceForever button to Setup menu.
 Added logging for operator silence changes.

 Revision 1.17  1998/07/29 17:27:37  jba
 Added "Unacknowledged Alarms Only" display filter.

 Revision 1.16  1998/06/22 18:42:13  jba
 Merged the new alh-options created at DESY MKS group:
  -D Disable Writing, -S Passive Mode, -T AlarmLogDated, -P Printing

 Revision 1.15  1998/06/02 19:40:49  evans
 Changed from using Fgmgr to using X to manage events and file
 descriptors.  (Fdmgr didn't work on WIN32.)  Uses XtAppMainLoop,
 XtAppAddInput, and XtAppAddTimeOut instead of Fdmgr routines.
 Updating areas is now in alCaUpdate, which is called every caDelay ms
 (currently 100 ms).  Added a general error message routine (errMsg)
 and an exception handler (alCAException).  Is working on Solaris and
 WIN32.

 Revision 1.14  1998/05/12 18:22:45  evans
 Initial changes for WIN32.

 Revision 1.13  1997/09/12 19:37:32  jba
 Added some prototypes.

 Revision 1.12  1997/09/09 22:24:24  jba
 Modified propUndo.

 Revision 1.11  1996/06/07 16:35:33  jba
 Added global alarm acknowledgement.

 Revision 1.10  1996/06/07 15:47:24  jba
 Added global alarm acknowledgement.

 * Revision 1.9  1996/03/25  15:46:12  jba
 * Removed unused alOpenLogFiles references.
 *
 * Revision 1.8  1995/11/13  22:31:22  jba
 * Added beepseverity command, ansi changes and other changes.
 *
 * Revision 1.7  1995/10/20  16:50:20  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 *
 * Revision 1.6  1995/06/22  19:40:20  jba
 * Started cleanup of file.
 *
 * Revision 1.5  1995/05/31  20:34:13  jba
 * Added name selection and arrow functions to Group window
 *
 * Revision 1.4  1995/05/30  16:07:42  jba
 * Removed  local routined blinking and alProcessX.
 * Renamed guidance_spawn_callback to processSpawn_callback.
 *
 * Revision 1.3  1995/03/24  16:35:51  jba
 * Bug fix and reorganized some files
 *
 * Revision 1.2  1994/06/22  21:17:02  jba
 * Added cvs Log keyword
 *
 */

static char *axSccsId = "@(#)ax.h	1.11\t12/15/93";

/* ax.h  */
/*
 *      Author:		Janet Anderson
 *      Date:		02-16-93
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
 * .01	mm-dd-yy		nnn	Description
 * .01	12-10-93		jba	Reflects file.c and dialog.c changes
 */

#ifndef INCaxh
#define INCaxh

#include <time.h>
#include <stdio.h>

#include <axSubW.h>
#include <line.h>
#include <alLib.h>
#include <axArea.h>

#ifdef __STDC__


/********************************************************************
  alInitialize.c   function prototypes
*********************************************************************/

void alInitialize( GLINK *proot);
void alInitializeGroupMasks( GLINK *proot);
int alInitializeGroupCounters( GLINK *pgroup);


/********************************************************************
  axArea.c   function prototypes
*********************************************************************/

Widget buildPulldownMenu( Widget parent, char *menu_title, char menu_mnemonic,
            int tearOff, MenuItem *items, XtPointer user_data);
void setupConfig( char *filename, int program, ALINK *areaOld);
void   markActiveWidget( ALINK *area, Widget newWidget);
void   createMainWindowWidgets( ALINK *area);
int    isTreeWindow( ALINK *area, void * subWindow);
void   scale_callback( Widget widget, ALINK *area, XmScaleCallbackStruct *cbs);
void   markSelectionArea( ALINK *area, struct anyLine *line);
void   changeBeepSeverityText( ALINK *area);
void   unmapArea_callback( Widget main, Widget w, XmAnyCallbackStruct *cbs);
void   axMakePixmap( Widget w);
void   axUpdateDialogs(ALINK *area);
void   *getSelectionLinkArea(ALINK *area);
int    getSelectionLinkTypeArea(ALINK *area);
Widget createActionButtons(Widget parent,ActionAreaItem *actions,int num_buttons);


/********************************************************************
  axRunW.c   function prototypes
*********************************************************************/

void icon_update(void);
XtErrorMsgHandler trapExtraneousWarningsHandler( String message);
void pixelData( Widget iconBoard, Widget bButton);
void createRuntimeWindow( ALINK *area);
void createMainWindow_callback( Widget w, ALINK *area, XmAnyCallbackStruct *call_data);
void unmapwindow_callback( Widget w, Widget main, XmAnyCallbackStruct *call_data);
void remapwindow_callback( Widget w, Widget main, XmAnyCallbackStruct *call_data);
void silenceCurrentReset(void *area);
void silenceCurrent_callback( Widget w, int notUsed, XmAnyCallbackStruct *call_data);
void silenceForeverChangeState();
void silenceOneHour_callback( Widget w, void * area, XmAnyCallbackStruct *call_data);


/********************************************************************
  axSubW.c   function prototypes
*********************************************************************/

void setParentLink( struct subWindow *subWindow, void *link);
void setLineRoutine( void *area, struct subWindow *subWindow,int program);
void markSelectedWidget( struct subWindow  *subWindow, Widget newWidget);
void initializeSubWindow( struct subWindow  *subWindow);
void setViewConfigCount( struct subWindow *subWindow, int count);
void invokeSubWindowUpdate( struct subWindow *subWindow);
struct subWindow *createSubWindow( void *area);
void createSubWindowWidgets( struct subWindow *subWindow, Widget parent);
int calcRowCount( struct subWindow *subWindow);
int calcRowYValue( struct subWindow *subWindow, int lineNo);
void adjustScrollBar( struct subWindow *subWindow);
void exposeResizeCallback( Widget widget, struct subWindow *subWindow,
     XEvent *cbs);
void defaultTreeSelection( ALINK * area);
void initSevrAbove( struct subWindow  *subWindow, void *link);
void initSevrBelow( struct subWindow  *subWindow, void *link);

/********************************************************************
  dialog.c   function prototypes
*********************************************************************/

Widget createFileDialog( Widget parent, void *okCallback, XtPointer okParm,
     void *cancelCallback, XtPointer cancelParm, XtPointer userParm,String title,
     String pattern, String dirSpec);
void createDialog( Widget parent, int dialogType, char *message1, char *message2);
void createActionDialog( Widget parent, int dialogType, String message1,
     XtCallbackProc okCallback, XtPointer okParm, XtPointer userParm);
void errMsg(const char *fmt, ...);

/********************************************************************
  file.c   function prototypes
*********************************************************************/

void exit_quit( Widget w, ALINK *area, XmAnyCallbackStruct *call_data);
void fileSetupCallback( Widget widget, int client_data, XmFileSelectionBoxCallbackStruct *cbs);
void fileCancelCallback( Widget widget, ALINK *area, XmFileSelectionBoxCallbackStruct *cbs);
void fileSetup(char *filename,ALINK *area,int fileType,int programId,Widget widget);
void fileSetupInit(Widget widget,int argc,char *argv[]);
char *shortfile(char *name);





/********************************************************************
  acknowledge.c   function prototypes
*********************************************************************/

void ack_callback( Widget widget, struct anyLine  *line, XmAnyCallbackStruct *cbs);


/********************************************************************
  awView.c   function prototypes
*********************************************************************/

void nameTreeW_callback( Widget pushButton, struct anyLine *line,
     XmPushButtonCallbackStruct *cbs);
void nameGroupW_callback( Widget pushButton, struct anyLine *line,
     XmPushButtonCallbackStruct *cbs);
void arrowTreeW_callback( Widget pushButton, void *glink,
     XmPushButtonCallbackStruct *cbs);
void arrowGroupW_callback( Widget pushButton, void *glink,
     XmPushButtonCallbackStruct *cbs);
void createConfigDisplay( ALINK *area, int expansion);
void displayNewViewTree( ALINK *area, GLINK *glink, int command);
void redraw( struct subWindow *subWindow, int rowNumber);
void invokeLinkUpdate( GCLINK *link, int linkType);
void markSelection( struct subWindow *subWindow, struct anyLine *line);
void awViewAddNewAlarm(CLINK *clink,int prevViewCount,int viewCount);
void awViewNewGroup( ALINK *area, GCLINK *link);
void awViewNewChan( ALINK *area, GCLINK *link);
int awViewViewCount( GCLINK *link);
void invokeDialogUpdate( ALINK *area);


/********************************************************************
  guidance.c   function prototypes
*********************************************************************/

void guidance_callback( Widget widget, GCLINK *link, XmAnyCallbackStruct *cbs);


/********************************************************************
  process.c   function prototypes
*********************************************************************/

void processSpawn_callback( Widget w, char *command, void *call_data);
void relatedProcess_callback( void *widget, GCLINK *link, void *cbs);


/********************************************************************
  alFilter.c   function prototypes
*********************************************************************/

int alFilterAll( GCLINK  *gclink);
int alFilterAlarmsOnly( GCLINK  *gclink);
int alFilterUnackAlarmsOnly( GCLINK  *gclink);


/********************************************************************
  alLib.c   function prototypes
*********************************************************************/

struct mainGroup *alAllocMainGroup(void);
GLINK *alAllocGroup(void);
CLINK *alAllocChan(void);
void alAddGroup( GLINK *parent, GLINK *glink);
void alAddChan( GLINK *parent, CLINK *clink);
void alPrecedeGroup( GLINK *parent, GLINK *sibling, GLINK *glink);
void alPrecedeChan( GLINK *parent, CLINK *sibling, CLINK*clink);
void alDeleteChan( CLINK *clink);
void alDeleteGroup( GLINK *glink);
CLINK *alFindChannel( SLIST *pgroup, char *channame);
GLINK *alFindGroup( SLIST *pgroup, char *groupname);
void alInsertChan( CLINK *sibling, CLINK *clink);
void alInsertGroup( GLINK *sibling, GLINK *glink);
void alMoveGroup( GLINK *sibling, GLINK *glink);
void alRemoveGroup( GLINK *glink);
void alRemoveChan( CLINK *clink);
void alSetPmainGroup( GLINK *glink, struct mainGroup *pmainGroup);
GLINK *alCopyGroup(GLINK *glink);
CLINK *alCopyChan(CLINK *clink);
GLINK *alCreateGroup();
CLINK *alCreateChannel();
void alSetMask( char *s4, MASK *mask);
void alGetMaskString( MASK mask, char *s);
void alOrMask( MASK *m1,MASK *m2);
void alNewAlarm( int stat, int sev, char value[MAX_STRING_SIZE], CLINK *clink);
void alNewEvent(int stat,int sevr,int acks,char value[MAX_STRING_SIZE],CLINK *clink);
void alHighestSystemSeverity(GLINK * glink);
int alHighestSeverity( short sevr[ALARM_NSEV]);
void alAckChan( CLINK *clink);
void alAckGroup( GLINK *glink);
void alForceChanMask( CLINK *clink, int index, int op);
void alForceGroupMask( GLINK *glink, int index, int op);
void alUpdateGroupMask( CLINK *clink, int index, int op);
void alChangeChanMask( CLINK *clink, MASK mask);
void alChangeGroupMask( GLINK *glink, MASK mask);
void alResetGroupMask( GLINK *glink);
void alForcePVChanEvent( CLINK *clink, int value);
void alForcePVGroupEvent( GLINK *glink, int value);
char *alAlarmGroupName( GLINK *link);
int alGuidanceExists( GCLINK *link);
int alProcessExists( GCLINK *link);

/********************************************************************
  alLog.c   function prototypes
*********************************************************************/

void alLogAlarm( time_t *ptimeofdayAlarm, struct chanData *cdata, int stat,
     int sev, int h_unackStat, int h_unackSevr);
void alLogConnection( const char *pvname,const char *ind);
void alLogGblAckChan( struct chanData *cdata);
void alLogAckChan( struct chanLine *cline);
void alLogAckGroup( struct groupLine *gline);
void alLogChanChangeMasks( CLINK *clink, int maskno, int maskid);
void alLogForcePVGroup( GLINK *glink, int ind);
void alLogResetPVGroup( GLINK *glink, int ind);
void alLogForcePVChan( CLINK *clink, int ind);
void alLogResetPVChan( CLINK *clink, int ind);
void alLogOpMod(char *);
void alLogExit(void);
void alLogChangeGroupMasks( GLINK *glink, int maskno, int maskid);
void alLogSetupConfigFile( char *filename);
void alLogSetupAlarmFile( char *filename);
void alLogSetupOpmodFile( char *filename);
void alLogSetupSaveConfigFile( char *filename);

/********************************************************************
  alView.c   function prototypes
*********************************************************************/

int alViewAdjustGroupW( GLINK *glink, int (*viewFilter)());
int alViewAdjustTreeW( GLINK *glink, int command, int (*viewFilter)());
GCLINK *alViewNextTreeW( GLINK  *glink, int *plinkType);
GCLINK *alViewNextGroupW( GCLINK  *link, int *plinkType);
GCLINK *alViewNthTreeW( GLINK *glinkStart, int *plinkType, int n);
GCLINK *alViewNthGroupW( GLINK *link, int *plinkType, int n);
int alViewMaxSevrNGroupW( GCLINK *linkStart, int n);
int alViewMaxSevrNTreeW( GLINK *glinkStart, int n);


/********************************************************************
  awAct.c   function prototypes
*********************************************************************/

Widget actCreateMenu( Widget parent, XtPointer user_data);


/********************************************************************
  awEdit.c   function prototypes
*********************************************************************/

void editUndoSet( GCLINK *link, int linkType, GCLINK *configLink,
     int command, int delete);
void editUndoGet( GCLINK **plink, int *plinkType, 
     GCLINK **pconfigLink);
int editUndoGetCommand(void);
void editClipboardGet( GCLINK **plink, int *plinkType);
void editClipboardSet( GCLINK *link, int linkType);
void editCutLink( ALINK *area, GCLINK *link, int linkType);
void editInsertFile( char *filename, ALINK *area);
void editPasteLink( ALINK *area, GCLINK *newLink, int linkType);


/********************************************************************
  awAlh.c   function prototypes
*********************************************************************/

Widget alhCreateMenu( Widget parent, XtPointer user_data);
void awRowWidgets( struct anyLine *gline, void *area);
void awUpdateRowWidgets( struct anyLine  *line);

/********************************************************************
  line.c   function prototypes
*********************************************************************/

void awGetMaskString( short mask[ALARM_NMASK], char *s);
struct chanLine *awAllocChanLine(void);
struct groupLine *awAllocGroupLine(void);
void awUpdateChanLine( struct chanLine *chanLine);
void awUpdateGroupLine( struct groupLine *groupLine);
void awGroupMessage( struct groupLine *groupLine);
void awChanMessage( struct chanLine *pchanLine);
void initLine( struct anyLine *line);
void initializeLines( SNODE *lines);

/********************************************************************
  current.c   function prototypes
*********************************************************************/

void currentAlarmHistoryWindow( ALINK *area, Widget menuButton);
void updateCurrentAlarmString( time_t *ptimeofday, char *name,
     char value[], int stat,int sev);
void updateCurrentAlarmWindow( ALINK *area);
void resetCurrentAlarmWindow();



/********************************************************************
  showmask.c   function prototypes
*********************************************************************/

void forceMaskShowDialog(ALINK *area,Widget menuButton);
void forceMaskUpdateDialog(ALINK *area);

/********************************************************************
  force.c   function prototypes
*********************************************************************/

void forcePVShowDialog(ALINK *area,Widget menuButton);
void forcePVUpdateDialog(ALINK *area);
void alOperatorForcePVChanEvent( CLINK *clink, MASK pvMask);


/********************************************************************
  alCA.c   function prototypes
*********************************************************************/

void alFdmgrInit(Display *display);
void alCaInit(void);
void alCaStop(void);
void alCaStart(SLIST *proot);
void alCaStartEvents(SLIST *proot);
void alCaCancel(SLIST *proot);
void alCaAddEvent( CLINK *clink);
void alCaClearEvent( CLINK *clink);
void  alCaPutSevr( CLINK *clink);
void  alCaSearch( SLIST *proot);
void alCaSearchName( char *name, chid *pchid);
void ClearChannelAccessEvents( SLIST *proot);
void ClearChannelAccessChids( SLIST *proot);
void  alCaAddEvents( SLIST *proot);
void alReplaceGroupForceEvent( GLINK *glink, char *str);
void alReplaceChanForceEvent( CLINK *clink, char *str);
void alCaPutGblAck(CLINK *clink);
void alCaPutSevrValue(char *sevrPVName,chid sevrchid,int sevr);

/********************************************************************
  alConfig.c   function prototypes
*********************************************************************/

void alGetConfig( struct mainGroup * pmainGroup, char *filename, int caConnect);
void alCreateConfig( struct mainGroup * pmainGroup);
void alPrintConfig(FILE *fw,struct mainGroup *pmainGroup);
void alWriteConfig( char *filename, struct mainGroup *pmainGroup);
void addNewSevrCommand(ELLLIST *pList,char *str);
void addNewStatCommand(ELLLIST *pList,char *str);
void removeSevrCommandList(ELLLIST *pList);
void removeStatCommandList(ELLLIST *pList);
void copySevrCommandList(ELLLIST *pListOld,ELLLIST *pListNew);
void copyStatCommandList(ELLLIST *pListOld,ELLLIST *pListNew);
void spawnSevrCommandList(ELLLIST *pList,int sev,int sevr_prev);
void spawnStatCommandList(ELLLIST *pList,int sev,int sevr_prev);
void getStringSevrCommandList(ELLLIST *pList,char **pstr);
void getStringStatCommandList(ELLLIST *pList,char **pstr);

/********************************************************************
  scroll.c   function prototypes
*********************************************************************/

void fileViewWindow( Widget w, int option, Widget menuButton);
void updateLog( int fileIndex, char *string);
void updateAlarmLog(int fileIndex, char *string);


/********************************************************************
  testalarm.c   function prototypes
*********************************************************************/

void done_dialog( Widget  w, Widget  dialog, XmAnyCallbackStruct *call_data);
void hide_dialog( Widget  w, Widget  dialog, XmAnyCallbackStruct *call_data);
Widget createGetTestAlarm_dialog( Widget parent, XtPointer userData);
void show_dialog( Widget  w, Widget  dialog, XmAnyCallbackStruct *call_data);


/********************************************************************
  mask.c   function prototypes
*********************************************************************/

void maskShowDialog(ALINK *area,Widget menuButton);
void maskUpdateDialog(ALINK *area);


/********************************************************************
  help.c   function prototypes
*********************************************************************/

void     xs_ok_callback( Widget w, void *client_data, XmAnyCallbackStruct *call_data);
void     xs_help_callback( Widget w, char *str[], void *call_data);
void     helpCallback( Widget w, int item, XmAnyCallbackStruct *cbs);


/********************************************************************
  clipboardOps.c   function prototypes
*********************************************************************/

void clipboardPut( Widget widget, Widget topWidget, XEvent *event);
void clipboardGet( Widget widget, Widget topWidget, XEvent *event);
 
/********************************************************************
  alDebug.c   function prototypes
*********************************************************************/

void alCaAddressInfo( SLIST *proot);
void printConfig( struct mainGroup *pmainGroup);
 
/********************************************************************
  property.c   function prototypes
*********************************************************************/

void propUpdateDialog(ALINK *area);
void propShowDialog(ALINK *area, Widget widget);
void propUndo(void *area);
 
 
#else

/********************************************************************
  alInitialize.c   function prototypes
*********************************************************************/

void alInitialize();
void alInitializeGroupMasks();
int alInitializeGroupCounters();


/********************************************************************
  axArea.c   function prototypes
*********************************************************************/

Widget buildPulldownMenu();
void  setupConfig();
void   markActiveWidget();
void   createMainWindowWidgets();
int    isTreeWindow();
void   scale_callback();
void   markSelectionArea();
void   changeBeepSeverityText();
void   unmapArea_callback();
void   axMakePixmap();
void   axUpdateDialogs();
void   *getSelectionLinkArea();
int    getSelectionLinkTypeArea();
Widget createActionButtons();

/********************************************************************
  axRunW.c   function prototypes
*********************************************************************/

void icon_update();
XtErrorMsgHandler trapExtraneousWarningsHandler();
void pixelData();
void createRuntimeWindow();
void createMainWindow_callback();
void unmapwindow_callback();
void remapwindow_callback();
void silenceCurrentReset();
void silenceCurrent_callback();
void silenceForeverChangeState();
void silenceOneHour_callback();

/********************************************************************
  axSubW.c   function prototypes
*********************************************************************/

void setParentLink();
void setLineRoutine();
void markSelectedWidget();
void initializeSubWindow();
void setViewConfigCount();
void invokeSubWindowUpdate();
struct subWindow *createSubWindow();
void createSubWindowWidgets();
int calcRowCount();
int calcRowYValue();
void adjustScrollBar();
void exposeResizeCallback();
void defaultTreeSelection();
void initSevrAbove();
void initSevrBelow();

/********************************************************************
  dialog.c   function prototypes
*********************************************************************/
Widget createFileDialog();
void createDialog();
void createActionDialog();

/********************************************************************
  file.c   function prototypes
*********************************************************************/
void exit_quit();
void fileSetupCallback();
void fileCancelCallback();
void fileSetup();
void fileSetupInit();
char *shortfile();

/********************************************************************
  acknowledge.c   function prototypes
*********************************************************************/
void ack_callback();

/********************************************************************
  awView.c   function prototypes
*********************************************************************/
void nameTreeW_callback();
void nameGroupW_callback();
void arrowTreeW_callback();
void arrowGroupW_callback();
void createConfigDisplay();
void displayNewViewTree();
void redraw();
void invokeLinkUpdate();
void markSelection();
void awViewAddNewAlarm();
void awViewNewGroup();
void awViewNewChan();
int awViewViewCount();
void invokeDialogUpdate();


/********************************************************************
  guidance.c   function prototypes
*********************************************************************/

void guidance_callback();

/********************************************************************
  process.c   function prototypes
*********************************************************************/

void processSpawn_callback();
void relatedProcess_callback();

/********************************************************************
  alFilter.c   function prototypes
*********************************************************************/

int alFilterAll();
int alFilterUnackAlarmsOnly();

/********************************************************************
  alLib.c   function prototypes
*********************************************************************/

struct mainGroup *alAllocMainGroup();
GLINK *alAllocGroup();
CLINK *alAllocChan();
void alAddGroup();
void alAddChan();
void alPrecedeGroup();
void alPrecedeChan();
void alDeleteChan();
void alDeleteGroup();
CLINK *alFindChannel();
GLINK *alFindGroup();
void alInsertChan();
void alInsertGroup();
void alMoveGroup();
void alRemoveGroup();
void alRemoveChan();
void alSetPmainGroup();
GLINK *alCopyGroup();
CLINK *alCopyChan();
GLINK *alCreateGroup();
CLINK *alCreateChannel();
void alSetMask();
void alGetMaskString();
void alOrMask();
void alNewAlarm();
void alNewEvent();
void alHighestSystemSeverity();
int alHighestSeverity();
void alAckChan();
void alAckGroup();
void alForceChanMask();
void alForceGroupMask();
void alUpdateGroupMask();
void alChangeChanMask();
void alChangeGroupMask();
void alResetGroupMask();
void alForcePVChanEvent();
void alForcePVGroupEvent();
char *alAlarmGroupName();
int alGuidanceExists();
int alProcessExists();

/********************************************************************
  alLog.c   function prototypes
*********************************************************************/

void alLogAlarm();
void alLogConnection();
void alLogAckChan();
void alLogAckGroup();
void alLogChanChangeMasks();
void alLogForcePVGroup();
void alLogResetPVGroup();
void alLogForcePVChan();
void alLogResetPVChan();
void alLogOpMod();
void alLogExit();
void alLogChangeGroupMasks();
void alLogSetupConfigFile();
void alLogSetupAlarmFile();
void alLogSetupOpmodFile();
void alLogSetupSaveConfigFile();

/********************************************************************
  alView.c   function prototypes
*********************************************************************/

int alViewAdjustGroupW();
int alViewAdjustTreeW();
GCLINK *alViewNextTreeW();
GCLINK *alViewNextGroupW();
GCLINK *alViewNthTreeW();
GCLINK *alViewNthGroupW();
int alViewMaxSevrNGroupW();
int alViewMaxSevrNTreeW();


/********************************************************************
  awAct.c   function prototypes
*********************************************************************/

Widget actCreateMenu();


/********************************************************************
  awEdit.c   function prototypes
*********************************************************************/

void editUndoSet();
void editUndoGet();
int editUndoGetCommand();
void editClipboardGet();
void editClipboardSet();
void editCutLink();
void editInsertFile();
void editPasteLink();


/********************************************************************
  awAlh.c   function prototypes
*********************************************************************/

Widget alhCreateMenu();
void awRowWidgets();
void awUpdateRowWidgets();

/********************************************************************
  line.c   function prototypes
*********************************************************************/

void awGetMaskString();
struct chanLine *awAllocChanLine();
struct groupLine *awAllocGroupLine();
void awUpdateChanLine();
void awUpdateGroupLine();
void awGroupMessage();
void awChanMessage();
void initLine();
void initializeLines();

/********************************************************************
  current.c   function prototypes
*********************************************************************/

void currentAlarmHistoryWindow();
void updateCurrentAlarmString();
void updateCurrentAlarmWindow();
void resetCurrentAlarmWindow();

/********************************************************************
  showmask.c   function prototypes
*********************************************************************/

void forceMaskShowDialog();
void forceMaskUpdateDialog();

/********************************************************************
  force.c   function prototypes
*********************************************************************/

void forcePVShowDialog();
void forcePVUpdateDialog();
void alOperatorForcePVChanEvent();

/********************************************************************
  alCA.c   function prototypes
*********************************************************************/

void alFdmgrInit();
void alCaInit();
void alCaStop();
void alCaStart();
void alCaStartEvents();
void alCaCancel();
void alCaAddEvent();
void alCaClearEvent();
void alCaPutSevr();
void alCaSearch();
void alCaSearchName();
void ClearChannelAccessEvents();
void ClearChannelAccessChids();
void alCaAddEvents();
void alReplaceGroupForceEvent();
void alReplaceChanForceEvent();
void alProcessCA();         /* process CA events */
void alCaPutGblAck();

/********************************************************************
  alConfig.c   function prototypes
*********************************************************************/

void alGetConfig();
void alCreateConfig();
void alPrintConfig();
void alWriteConfig();
void addNewSevrCommand();
void addNewStatCommand();
void removeSevrCommandList();
void removeStatCommandList();
void copySevrCommandList();
void copyStatCommandList();
void spawnSevrCommandList();
void spawnStatCommandList();
void getStringSevrCommandList();
void getStringStatCommandList();

/********************************************************************
  scroll.c   function prototypes
*********************************************************************/

void fileViewWindow();
void updateLog();
void updateAlarmLog();

/********************************************************************
  testalarm.c   function prototypes
*********************************************************************/

void done_dialog();
void hide_dialog();
Widget createGetTestAlarm_dialog();
void show_dialog();

/********************************************************************
  mask.c   function prototypes
*********************************************************************/

void maskShowDialog();
void maskUpdateDialog();

/********************************************************************
  help.c   function prototypes
*********************************************************************/

void     xs_ok_callback();
void     xs_help_callback();
void     helpCallback();

/********************************************************************
  clipboardOps.c   function prototypes
*********************************************************************/

void clipboardPut();
void clipboardGet();

/********************************************************************
  alDebug.c   function prototypes
*********************************************************************/

void alCaAddressInfo();
void printConfig();
 
/********************************************************************
  property.c   function prototypes
*********************************************************************/

void propUpdateDialog();
void propShowDialog();
void propUndo();
 

#endif /*__STDC__*/

#endif
