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
/* ax.h */

/************************DESCRIPTION***********************************
  alh function prototypes
**********************************************************************/

#ifndef INCaxh
#define INCaxh

#include <time.h>
#include <stdio.h>

#include "alh.h"
#include "axSubW.h"
#include "line.h"
#include "alLib.h"
#include "axArea.h"

/********************************************************************
  alarm.c   function prototypes
*********************************************************************/

void alhAlarmStringInit();


/********************************************************************
  axArea.c   function prototypes
*********************************************************************/

Widget buildPulldownMenu( Widget parent, char *menu_title, char menu_mnemonic,
int tearOff, MenuItem *items, XtPointer user_data);
void setupConfig( char *filename, int program, ALINK *areaOld);
void   markActiveWidget( ALINK *area, Widget newWidget);
void   createMainWindowWidgets( ALINK *area);
int    isTreeWindow( ALINK *area, void * subWindow);
void   markSelectionArea( ALINK *area, struct anyLine *line);
void   changeBeepSeverityText( ALINK *area);
void   changeSilenceForeverText( ALINK *area);
void   updateDisabledForcePVCount(ALINK *area, int count);
void   unmapArea_callback( Widget main, Widget w, XmAnyCallbackStruct *cbs);
void   axMakePixmap( Widget w);
void   axUpdateDialogs(ALINK *area);
void   *getSelectionLinkArea(ALINK *area);
int    getSelectionLinkTypeArea(ALINK *area);
Widget createActionButtons(Widget parent,ActionAreaItem *actions,int num_buttons);
void showMainWindow(ALINK *area);


/********************************************************************
  axRunW.c   function prototypes
*********************************************************************/

void pixelData(Widget iconBoard);
XtErrorMsgHandler trapExtraneousWarningsHandler( String message);
void createRuntimeWindow( ALINK *area);
void silenceCurrentReset(void *area);
void silenceCurrent_callback( Widget w, ALINK* area, XmAnyCallbackStruct *call_data);
void silenceForeverChangeState( ALINK *area);
void silenceSelectedMinutes_callback( Widget w, ALINK* area, XmAnyCallbackStruct *call_data);
void changeTreeColor(Widget widget,Pixel color);
void silenceSelectedMinutesReset(void *area);


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
  browser.c   function prototypes
*********************************************************************/

int callBrowser(char *url);

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
void fatalErrMsg(const char *fmt, ...);

/********************************************************************
  file.c   function prototypes
*********************************************************************/

void exit_quit(Widget w, XtPointer clientdata, XtPointer calldata);
void fileSetupCallback( Widget widget, int client_data, XmFileSelectionBoxCallbackStruct *cbs);
void fileCancelCallback( Widget widget, ALINK *area, XmFileSelectionBoxCallbackStruct *cbs);
void fileSetupInit(Widget widget,int argc,char *argv[]);
char *shortfile(char *name);

/********************************************************************
  acknowledge.c   function prototypes
*********************************************************************/

void ack_callback( Widget widget, struct anyLine  *line, XmAnyCallbackStruct *cbs);
void ackChan(CLINK *clink);

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

void guidanceCallback( Widget widget, GCLINK *link, XmAnyCallbackStruct *cbs);
int guidanceExists(GCLINK *link);
int guidanceDisplay(GCLINK *link);
void guidanceCopyGuideList(SLIST *pToGuideList,SLIST *pFromGuideList);
void guidanceDeleteGuideList(SLIST *pGuideList);


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
void alAddGroup( GLINK *parent, GLINK *glink);
void alAddChan( GLINK *parent, CLINK *clink);
void alPrecedeGroup( GLINK *parent, GLINK *sibling, GLINK *glink);
void alPrecedeChan( GLINK *parent, CLINK *sibling, CLINK*clink);
void alDeleteChan( CLINK *clink);
void alDeleteGroup( GLINK *glink);
void alRemoveGroup( GLINK *glink);
void alRemoveChan( CLINK *clink);
void alSetPmainGroup( GLINK *glink, struct mainGroup *pmainGroup);
GLINK *alCopyGroup(GLINK *glink);
CLINK *alCopyChan(CLINK *clink);
GLINK *alCreateGroup();
CLINK *alCreateChannel();
void alSetMask( char *s4, MASK *mask);
void alGetMaskString( MASK mask, char *s);
void alNewAlarm( int stat, int sev, char *value, CLINK *clink);
void alSaveAlarmEvent(int stat,int sevr,int acks,int ackt,char *value,CLINK *clink);
void alConnectEvent(CLINK *clink);
void alNewEvent(int stat,int sevr,int acks,int ackt,char *value,CLINK *clink);
void alHighestSystemSeverity(GLINK * glink);
short alHighestSeverity( int sevr[ALH_ALARM_NSEV]);
void alForceChanMask( CLINK *clink, int index, int op);
void alForceGroupMask( GLINK *glink, int index, int op);
void alChangeChanMask( CLINK *clink, MASK mask);
void alChangeGroupMask( GLINK *glink, MASK mask);
void alResetGroupMask( GLINK *glink);
char *alAlarmGroupName( GLINK *link);
int alProcessExists( GCLINK *link);
void alSetUnackSevChan(CLINK *clink,int newSevr);
void alSetCurChanMask(CLINK *clink,MASK mask);
void alSetBeepSevrChan(CLINK *link,int sevr);
void alSetBeepSevrGroup(GLINK *link,int sevr);
void alRemoveNoAck1HrTimerGroup(GLINK *glink);
void alRemoveNoAck1HrTimerChan(CLINK *clink);


/********************************************************************
  alLog.c   function prototypes
*********************************************************************/

void alLogAlarmMessage(time_t *ptimeofdayAlarm,int messageCode,CLINK* clink,const char *fmt,...);
void alLogOpModMessage(int messageCode,GCLINK* clink,const char *fmt,...);
void alLogOpModAckMessage(int messageCode,GCLINK* gclink,const char* fmt,...);
void alLog2DBAckChan (char *name);
void alLogNotSaveStart(int not_save_time);
void alLogNotSaveFinish();
void alLog2DBMask (char *name);

#ifdef CMLOG
void alCMLOGconnect(void);
void alCMLOGdisconnect(void);
#endif

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

void awGetMaskString( int mask[ALARM_NMASK], char *s);
struct anyLine *awAllocLine(void);
void awUpdateChanLine( struct anyLine *chanLine);
void awUpdateGroupLine( struct anyLine *groupLine);
void initLine( struct anyLine *line);
void initializeLines( SNODE *lines);

/********************************************************************
  current.c   function prototypes
*********************************************************************/

void currentAlarmHistoryWindow( ALINK *area, Widget menuButton);
void updateCurrentAlarmString( ALINK *area, time_t *ptimeofday,
char *name, char value[], int stat,int sev);
void updateCurrentAlarmWindow( ALINK *area);
void resetCurrentAlarmWindow(ALINK *area);

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
void alForcePVValueEvent(void *usr,double value);
void alForcePVDelete(FORCEPV**);
FORCEPV* alForcePVCopy(FORCEPV*);
void alForcePVClearCA(FORCEPV*);
void alForcePVSetNotConnected(FORCEPV*,char*);

/********************************************************************
  alCA.c   function prototypes
*********************************************************************/

void alCaPend(double sec);
short alCaIsConnected(chid chid);
void alCaFlushIo(void);
void alCaInit(void);
void alCaPoll(void);
void alCaStop(void);
void alCaConnectChannel(char *name,chid *pchid,void *puser);
void alCaConnectForcePV(char *name,chid *pchid,void *puser);
void alCaConnectSevrPV(char *name,chid *pchid,void *puser);
void alCaConnectAckPV(char *name,chid *pchid,void *puser);
void alCaConnectHeartbeatPV(char *name,chid *pchid,void *puser);
void alCaClearChannel(chid *pchid);
void alCaClearEvent(evid *pevid);
void alCaAddEvent(chid chid,evid *pevid,void *clink);
void alCaAddForcePVEvent(chid chid,void *link,evid *pevid);
void alCaPutGblAck(chid chid,short *psevr);
void alCaPutSevrValue(chid chid,short *psevr);
void alCaPutHeartbeatValue(chid chid,short *value);
void alCaPutGblAckT(chid chid, short *pstate);
void alCaPutAckValue(chid chid, short *psevr);
void getDescriptionRecord(char *name,char *description,chid descriptionFieldCaId);

/********************************************************************
  alCaCommon.c   function prototypes
*********************************************************************/
void alCaCancel(struct mainGroup *pmainGroup);
void registerCA(void *dummy, int fd, int opened);
void alUpdateAreas();
void alSetNotConnected(struct mainGroup *pmainGroup);
void alPutGblAckT(struct mainGroup *pmainGroup);

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
void updateAlarmLog( int fileIndex, char *string);
void browser_fileViewWindow(Widget w,int option,Widget menuButton);

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

void     xs_help_callback( Widget w, char *str[], void *call_data);
void     helpCallback( Widget w, int item, XmAnyCallbackStruct *cbs);

/********************************************************************
  beepSevr.c   function prototypes
*********************************************************************/

void beepSevrUpdateDialog(ALINK *area);
void beepSevrShowDialog(ALINK *area, Widget widget);

/********************************************************************
  noAck.c   function prototypes
*********************************************************************/

void noAckUpdateDialog(ALINK *area);
void noAckShowDialog(ALINK *area, Widget widget);

/********************************************************************
  property.c   function prototypes
*********************************************************************/

void propUpdateDialog(ALINK *area);
void propShowDialog(ALINK *area, Widget widget);
void propUndo(void *area);


/********************************************************************
  os/<osclass>/alAudio.c   function prototypes
*********************************************************************/

void alBeep(Display *displayBB);

/********************************************************************
  heartbeat.c   function prototypes
*********************************************************************/

void alHeartbeatStart(void* data);
void alHeartbeatStop(void* data);
void alHeartbeatPVRemove(struct mainGroup *pmainGroup);
void alHeartbeatPVAdd(struct mainGroup *pmainGroup,char* name,float rate,short value);




#endif /* INCaxh */
/* alCaCommon.c */

