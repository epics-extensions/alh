/*
 $Log$
 Revision 1.8  1998/06/02 19:40:46  evans
 Changed from using Fgmgr to using X to manage events and file
 descriptors.  (Fdmgr didn't work on WIN32.)  Uses XtAppMainLoop,
 XtAppAddInput, and XtAppAddTimeOut instead of Fdmgr routines.
 Updating areas is now in alCaUpdate, which is called every caDelay ms
 (currently 100 ms).  Added a general error message routine (errMsg)
 and an exception handler (alCAException).  Is working on Solaris and
 WIN32.

 Revision 1.7  1997/08/27 22:06:56  jba
 Fixed alLogConnection comment.

 Revision 1.6  1996/03/25 15:46:11  jba
 Removed unused alOpenLogFiles references.

 * Revision 1.5  1995/10/20  16:49:57  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 *
 * Revision 1.4  1995/06/22  19:48:48  jba
 * Added $ALIAS facility.
 *
 * Revision 1.3  1995/05/30  15:51:07  jba
 * Added ALARMCOMMAND facility
 *
 * Revision 1.2  1994/06/22  21:16:38  jba
 * Added cvs Log keyword
 *
 */

/* alLib.h */
/* share/src/alh        @(#)alLib.h	1.11     10/15/93 */
/*  alh  -  Alarm Handler
 *
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
 * .01  06-12-91        bkc     Change sevrchid for group and channel
 * .02  07-22-91        bkc     Change value, forcePVValue, resetPVValue types
 * .03  mm-dd-yy        iii     Comment
 *      ...
 */

#ifndef INCalLibh
#define INCalLibh 1

#ifndef INCalarmh
#  include <alarm.h>
#endif

#ifndef INCsllLibh
#  include <sllLib.h>
#endif

#ifndef INCLcadefh
#include <cadef.h>
#endif

#ifndef INCLdb_accessh
#include <db_access.h>
#endif

#ifndef INCLalhh
#include <alh.h>
#endif

#include <ellLib.h>

static char *alLibhSccsId = "@(#)alLib.h	1.11\t10/15/93";

typedef struct mask {			/* mask bit setting */
        unsigned Cancel    : 1;
        unsigned Disable   : 1;
        unsigned Ack       : 1;
        unsigned AckT      : 1;
        unsigned Log	   : 1;
        unsigned Unused    : 11;
        } MASK;

typedef struct countFilter {
        int    inputCount;
        int    inputSeconds;
        short  stat;        /* channel status from CA */
        short  sev;         /* current severity */
	    char   value[MAX_STRING_SIZE];	/* channel value from CA */
        int    curCount;
        int    alarmTime;
        void  *clink;
        XtIntervalId timeoutId;
        } COUNTFILTER;
 
/* group/channel data structure */
struct gcData {
        char *name;		/* group name */
	char *forcePVName;	/* force PV name */
	char *sevrPVName;	/* severity PV name */
	short PVValue;		/* PV value */	
	short forcePVValue;	/* PV value for force mask */	
	short resetPVValue;	/* PV value for reset mask */
	MASK forcePVMask;	/* force Mask */
	char *alias;	 	/* alias text */
	char *command;	 	/* command text */
	ELLLIST sevrCommandList;	/* alarm severity command list */
	short curSevr;		/* current severity */
	short unackSevr;	/* highest unack severity */
	chid forcechid;			/* forcePV channel id */
	evid forceevid;			/* forcePV channel evid */
	chid sevrchid;			/* group sevrPV channel id */
        };

/* group data structure */
struct groupData {
        char *name;		/* group name */
	char *forcePVName;	/* force PV name */
	char *sevrPVName;	/* severity PV name */
	short PVValue;		/* PV value */	
	short forcePVValue;		/* PV value for force mask */	
	short resetPVValue;		/* PV value for reset mask */
	MASK forcePVMask;		/* force Mask */
	char *alias;	 	/* alias text */
	char *command;	 		/* command text */
	ELLLIST sevrCommandList;	/* severity command list */
	short curSevr;			/* current highestseverity from CA */
	short unackSevr;		/* highest unack severity */
	chid forcechid;			/* forcePV channel id */
	evid forceevid;			/* forcePV channel evid */
	chid sevrchid;			/* group sevrPV channel id */
	char *treeSym;                  /* tree symbols for treeWindow display */
	short mask[ALARM_NMASK];	/* no. of channels of masked types*/
	short curSev[ALARM_NSEV];  	/* channels of different severity */
	short unackSev[ALARM_NSEV];  	/* channels of unacknowledged sev */
        };

/* channel data structure */
struct chanData {
        char *name; 		/* channel name */         
	char *forcePVName;	/* force PV name */
	char *sevrPVName;	/* severity PV name */
	short PVValue;		/* PV value */	
	short forcePVValue;		/* PV value for force mask */
	short resetPVValue;		/* PV value for reset mask */
	MASK forcePVMask;		/* forced mask setting */
	char *alias;	 	/* alias text */
	char *command;			/* command text */	
	ELLLIST sevrCommandList;	/* severity command list */
	short curSevr;			/* channel severity from CA */
	short unackSevr;		/* highest unack severity */
	chid forcechid;		 	/* forcePV channel id */
	evid forceevid;			/* forcePV channel evid */
	chid sevrchid;		 	/* sevrPV channel id */
	ELLLIST statCommandList;	/* alarm status command list */
    COUNTFILTER *countFilter;	/* alarm count filter */
	MASK curMask;			/* current mask setting */
	MASK defaultMask;		/* default mask setting */
	char value[MAX_STRING_SIZE];	/* channel value from CA */
	short curStat;			/* channel status from CA */
	short unackStat;		/* unack status */
	chid chid;			/* chid from CA search */
	evid evid;			/* evid from CA add event */
        };

/* group link  */
struct groupLink {
    SNODE node;			/* single link list node type */
	SLIST GuideList;		/* guidance link list */
    struct groupLink *parent;	/* parent groupLink pointer */
    struct groupData *pgroupData;	/* group data  pointer */
    struct mainGroup *pmainGroup;   /* mainGroup pointer */
	void *lineTreeW;		/* line address Tree Window*/
	void *lineGroupW;		/* line address Group Window*/
	int viewCount;	 		/* count of open groups */
	int modified;			/* modified indicator */
    SLIST subGroupList;		/* subgroup link list */
    SLIST chanList;			/* channel link list */
    };

/* channel link */
struct chanLink {
    SNODE node;			/* single link list node type */
	SLIST GuideList;		/* guidance link list */
    struct groupLink *parent;	/* parent groupLinke pointer */
    struct chanData *pchanData;	/* channel data  pointer */
    struct mainGroup *pmainGroup;   /* mainGroup pointer */
	void *lineTreeW;		/* line address Tree Window*/
	void *lineGroupW;		/* line address Group Window*/
	int viewCount;	 		/* open/closed indicator */
	int modified;			/* modified indicator */

       };

/* any link */
struct anyLink {
    SNODE node;			/* single link list node type */
	SLIST GuideList;		/* guidance link list */
    struct groupLink *parent;	/* parent groupLinke pointer */
    struct gcData *pgcData;  	/* channel data  pointer */
    struct mainGroup *pmainGroup;   /* mainGroup pointer */
	void *lineTreeW;		/* line address Tree Window*/
	void *lineGroupW;		/* line address Group Window*/
	int viewCount;	 		/* open/closed count or indicator */
	int modified;			/* modified indicator */
       };

/* guidance link */
struct guideLink {
	SNODE node;			/* single link list node type */
	char *list;			/* guidance list address */
	};

typedef struct groupLink GLINK;
typedef struct chanLink CLINK;
typedef struct anyLink GCLINK;

CLINK *alFindChannel(),*alAllocChan();
GLINK *alFindGroup(),*alAllocGroup();
void alGetMaskString();
 
#define alGroupParent(GROUP) \
        (((GLINK *)(GROUP))->parent)

#endif

/*
----------------------------------------------------------------------------------------------
Routines for create, delete and modify groups and channels: defined in alLib.c
------------------------------------------------------------------------------------------------
struct mainGroup *alAllocMainGroup()   		Allocate space for main group
GLINK *alAllocGroup()                   	Allocate space for group link 
CLINK *alAllocChan()                    	Allocate space for channel link
void alAddGroup(parent,glink)            	Append a group link to a parent's 
void alAddChan(parent,clink)             	Append a channel link to a parent's 
void alPrecedeGroup(parent,sibling,glink)   Preceed a group before another group in subgrouplist
void alPrecedeChan(parent,sibling,clink)    Preceed a chan before another chan in the subgrouplist
void alDeleteChan(clink)                 	Delete a clink from channel list
void alDeleteChan(clink)                 	Delete a clink from channel list
void alDeleteChan(clink)                 	Delete a clink from channel list
CLINK *alFindChannel(pgroup,channame)   	Find the channel name from a group
void alDeleteGroup(glink)                	Delete a glink and all associated 
GLINK *alFindGroup(pgroup,groupname)    	Find the group name from a group
void alInsertChan(sibling,clink)         	Insert the channel link after a
void alInsertGroup(sibling,glink)        	Insert the group link after a 
void alMoveGroup(sibling,glink)          	Move the group link after a 
void alRemoveGroup(glink)                	Remove the group link from its
void alRemoveChan(clink)                 	Remove the channel link from it's

--------------------------------------------------------------------------------------------
		Mask manipulation routines: defined in alLib.c
--------------------------------------------------------------------------------------------

int alSetMask(s4,mask)                  		Set mask bit from string s4
void alGetMaskString(mask,s)             		Get mask string from Mask bit setting
void alOrMask(m1,m2)                     		Or operation of two masks m1 & m2

--------------------------------------------------------------------------------------------
		Action routines: defined in alLib.c
--------------------------------------------------------------------------------------------

alNewAlarm(stat,sev,value,clink)        		Update system data due to new alarm
int alHighestSystemSeverity()           		Set system highest alarm severity
int alHighestSeverity(sevr)             		Set highest group alarm severity  
alAckChan(clink)                        		Acknowledge an alarmed channel and
alAckGroup(glink)                       			Acknowledge a group 
alForceChanMask(clink,index,op)         		Force channel mask by bit selection
alForceGroupMask(glink,index,op)        		Force group mask by bit selection
alUpdateGroupMask(clink,index,op)       		Update all the parent group masks
alChangeChanMask(clink,mask)            		Change channel mask & adjust CA
alChangeGroupMask(glink,mask)           		Change group mask 
alForcePVChanEvent(clink,value)         		Force / reset channel mask
alForcePVGroupEvent(glink,value)        		Force / reset group mask
alResetGroupMask(glink)                         Reset all channel masks for a group 


--------------------------------------------------------------------------------------------
		Debugging routines: defined in alDebug.c
--------------------------------------------------------------------------------------------

static int print_node_name(pgroup)              Print group configuration
int listChanGroup(pgroup)                 	List alarm tree structure
alListChanGroup(pgroup)                 	List alarm configuration layout
void printConfig()                              Print total configuration

--------------------------------------------------------------------------------------------
		Initialization routines: defined in alInitialize.c
--------------------------------------------------------------------------------------------

alInitialize()                       		Initialize alarm configuration
alInitializeGroupMasks(proot)           	Initialize group masks 
alInitializeGroupCounters(pgroup)              	Count number of channels in all sub


--------------------------------------------------------------------------------------------
		Alarm configuration routines: defined in alConfig.c
--------------------------------------------------------------------------------------------

alGetConfig(filename)               			Get alarm system configuration
alWriteGroupConfig(pgroup)              		Write group configuration
alWriteConfig(filename)              		 	Write system configuration
static int print_error(buf,message)       		Print error message
static void GetGroupLine(buf,pglink)        		Read a group line from config file
static void GetChannelLine(buf,pclink)        		Read a channel line from config file
static void GetOptionalLine(buf,pclink)        		Read an optional  line from config file

--------------------------------------------------------------------------------------------
		Routines for log messages: defined in alLog.c
--------------------------------------------------------------------------------------------

alLogAlarm(cdata,stat,sev,h_unackStat,h_unackSevr)	Log new alarms
alLogAckChan(cline)					Log acknowledged channel
alLogAckGroup(gline)					Log acknowledged group
alLogChanChangeMasks(cdata)				Log change channel Masks
alLogForcePVGroup(glink,ind)				Log force PV group
alLogResetPVGroup(glink,ind) 				Log reset PV group
alLogForcePVChan(clink,ind)				Log force PV channel
alLogResetPVChan(clink,ind)				Log reset PV channel
alLogConnection(pname,text)				Log unconnected pvname
alLogExit()						Log exit ALH
alLogChangeGroupMasks(glink,choosegroupData)		Log change group Masks
alLogSetupConfigFile(filename)				Log setup config file
alLogSetupAlarmFile(filename)				Log setup alarm log file
alLogSetupOpmodFile(filename)				Log setup opmod log file
alLogSetupSaveConfigFile(filename)			Log setup save config file

--------------------------------------------------------------------------------------------
		Routines for communicating with Channel Access: defined in alCA.c
--------------------------------------------------------------------------------------------

alCaStart()                             	Start and setup channel access
alCaCancel()                              	Disconnect channel access
alCaAddEvent(clink)                 		Add channel access alarm event 
alCaClearEvent(clink)				Clear channel access alarm event
void alCaPutSevr(clink)          		Update SevrPVValue
void alProcessCA()                      	ca_pend_io
void  alCaSearch(glink)           		Add all channel access searchs
void  alCaAddEvents(glink)  			Add CA connection events
static void registerCA(dummy,fd,condition)
static ClearChannelAccessEvents(glink)          Clear Channel access events
static void AlarmChangeConnectionEvent(args)    Add alarm not connected event
static void NewAlarmEvent(args)                 New alarm event call back 
static void GroupForceEvent( args)		Add force group event
static void ChannelForceEvent(args)		Add force channel event
static void ForceGroupChangeConnectionEvent(args)      Add unconnected force Group     
static void ForceChannelChangeConnectionEvent(args)    Add unconnected force channel
static void SevrGroupChangeConnectionEvent(args)       Add unconnected sevr group
static void SevrChannelChangeConnectionEvent(args)     Add unconnected sevr channel

--------------------------------------------------------------------------------------------
		Routines for modifying the configuration view: defined in alView.c
--------------------------------------------------------------------------------------------

int alViewAdjustGroup(glink,viewFilter)                Returns groupWindow viewCount
int alViewAdjustTree(glink,command,viewFilter)         Adjusts treeW view and returns new viewCount
GCLINK * alViewNext(glink,plinkType)                   Return next treeWindow item open for view
GCLINK * alViewNextGroup(link,plinkType)               Returns next groupWindow item
GCLINK * alViewNth(glinkStart,plinkType,n)             Returns Nth treeWindow item open
GCLINK * alViewNthGroup(link,plinkType,n)              Returns Nth groupWindow item
int alViewMaxSevrN(glinkStart,n)                       Gets treeWindow max sevr in range
int alViewMaxSevrNGroup(linkStart,n)                   Gets groupWindow max sevr in range
static void treeView(glink,command,viewFilter)         Recursive routine to modify view

--------------------------------------------------------------------------------------------
*/
