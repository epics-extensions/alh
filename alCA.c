/*
 $Log$
 Revision 1.16  1998/06/09 17:10:48  evans
 Fixed a typo in the exception handler.

 Revision 1.15  1998/06/09 16:46:56  evans
 Changed exception handler to quit ater 25 exceptions to avoid swamping
 the interface.

 Revision 1.14  1998/06/02 19:40:44  evans
 Changed from using Fgmgr to using X to manage events and file
 descriptors.  (Fdmgr didn't work on WIN32.)  Uses XtAppMainLoop,
 XtAppAddInput, and XtAppAddTimeOut instead of Fdmgr routines.
 Updating areas is now in alCaUpdate, which is called every caDelay ms
 (currently 100 ms).  Added a general error message routine (errMsg)
 and an exception handler (alCAException).  Is working on Solaris and
 WIN32.

 Revision 1.13  1998/06/01 18:33:22  evans
 Modified the icon.

 Revision 1.12  1998/05/12 18:22:38  evans
 Initial changes for WIN32.

 Revision 1.11  1997/10/27 17:27:59  jba
 Moved write of sevr to sevrPVs and now write only when changed.

 Revision 1.10  1996/06/07 16:35:29  jba
 Added global alarm acknowledgement.

 * Revision 1.9  1996/03/25  15:47:28  jba
 * Added cast.
 *
 * Revision 1.8  1995/11/13  22:31:08  jba
 * Added beepseverity command, ansi changes and other changes.
 *
 * Revision 1.7  1995/10/20  16:49:47  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 * Revision 1.6  1995/05/30  16:06:07  jba
 * Add unused parm to alCaPendEvent and alProcessX for fdmgr_add_timeout
prototype.
 *
 * Revision 1.5  1995/02/28  16:43:25  jba
 * ansi c changes
 *
 * Revision 1.4  1994/06/22  21:16:23  jba
 * Added cvs Log keyword
 *
 */

#define DEBUG_CALLBACKS 0

static char *sccsId = "%W%\t%G%";

/*  alCA.c 
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
 * .01  06-12-91        bkc     Add the CA search for all defined sevrPVName 
 * .02  07-22-91        bkc     Add the CA search for all force & sevr PVNames,
 *                add force mask events and change connection 
 *                events 
 *                               
 * .03  10-04-91        bkc     Add the clear channel events,
 *                freeing the memory pointers, and
 *                adding the new froce PV events
 * .04  10-01-91        bkc     Take care initially canceled channel 
 * .05  07-16-92        jba     changed VALID_ALARM to INVALID_ALARM
 * .06  02-16-93        jba     removed static from ClearChannelAccessEvents and
 *                              ClearChannelAccessChids
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */
/* alCA.c */

#include <stdio.h>
#include <stdlib.h>

#include <alarm.h>
#include <cadef.h>

#include <sllLib.h>
#include <alLib.h>
#include <alh.h>
#include <ax.h>


#define AUTOMATIC    0

#define CA_PEND_EVENT_TIME     0.001    

/* Struct for file descriptor linked list */
struct FDLIST {
    struct FDLIST *prev;
    XtInputId inpid;
    int fd;
};

struct FDLIST *lastFdInList=(struct FDLIST *)0;

static char buff[81];

XtIntervalId  caTimeoutId=(XtIntervalId)0;

unsigned long caDelay = 100;     /* ms */

extern int DEBUG;
extern XtAppContext appContext;

/* external functions 

extern alLogConnection();
extern alForcePVChanEvent();
extern alForcePVGroupEvent();
extern void alChangeGroupMask();
extern void alOperatorForcePVChanEvent();
extern void alResetGroupMask();
extern void alLogForcePVGroup();
extern void alLogForcePVChan();
extern void alLogResetPVGroup();    
extern void alLogResetPVChan();    
extern long alHighestSeverity();
extern void alNewAlarm();  

*/

#ifdef __STDC__

static void NewAlarmEvent( struct event_handler_args args);
static void GroupForceEvent( struct event_handler_args args);
static void ChannelForceEvent( struct event_handler_args args);
static void AlarmChangeConnectionEvent( struct connection_handler_args args);
static void ForceGroupChangeConnectionEvent( struct connection_handler_args
args);
static void ForceChannelChangeConnectionEvent( struct connection_handler_args
args);
static void SevrGroupChangeConnectionEvent( struct connection_handler_args
args);
static void SevrChannelChangeConnectionEvent( struct connection_handler_args
args);
static void al_ca_error_code(char *alhName,char *caName,int status,char *
PVname);
static void registerCA(void *dummy, int fd, int opened);
static void alCaUpdate(XtPointer cd, XtIntervalId *id);
static void alProcessCA(XtPointer cd, int *source, XtInputId *id);
static void alCAException(struct exception_handler_args args);

#else

static void NewAlarmEvent();
static void GroupForceEvent();
static void ChannelForceEvent();
static void AlarmChangeConnectionEvent();
static void ForceGroupChangeConnectionEvent();
static void ForceChannelChangeConnectionEvent();
static void SevrGroupChangeConnectionEvent();
static void SevrChannelChangeConnectionEvent();
static void al_ca_error_code();
static void registerCA();
static void alCaUpdate();
static void alProcessCA();
static void alCAException();

#endif /*__STDC__*/

/*
*-----------------------------------------------------------------
*    routines related to system startup , initialization & termination 
*-----------------------------------------------------------------
*
------------
|  PUBLIC  |
------------
void alCaInit()                 Initialize channel access
*
void alCaStart(proot)            Start and setup for channel access
    SLIST *proot;
*
void alCaStartEvents(proot)        Add change connection events
    SLIST *proot;
*
void alCaCancel(proot)            Disconnect channel access 
    SLIST *proot;
*
void alCaClearEvent(clink)        Remove a channel alarm event
    CLINK *clink;
* 
void alCaAddEvent(clink)        Add a channel alarm event
    CLINK *clink;
* 
void alCaPutSevr(clink)            ca_put SevrPVValue
    CLINK *clink;
*
void  alCaSearch(glink)            Add all channel access events
    SLIST *glink;
*
void  alCaAddEvents(proot)        Add change connection events
    SLIST *proot;
*
alReplaceGroupForceEvent(glink,str)     Replace new group force events
    GLINK *glink;
    char *str;
*
alReplaceChanForceEvent(clink,str)      Replace new channel force events

    CLINK *clink;
    char *str;
*
------------
|  PRIVATE  |
------------
static void registerCA(dummy,fd,condition)
*
static ClearChannelAccessEvents(glink)        Clear Channel access events
    SLIST *glink;        
*
static ClearChannelAccessChids(proot)         Clear channel access chids

    SLIST *proot;
*
static void NewAlarmEvent(args)            New alarm event call back func
tion
    struct event_handler_args args;
*
static void GroupForceEvent( args)        Force group event call back
    struct event_handler_args args;
*
static void ChannelForceEvent(args)        Force channel event call back
 
    struct event_handler_args args;
*
static void AlarmChangeConnectionEvent(args)    Add alarm not connected e
vent
    struct connection_handler_args args;
*
static void ForceGroupChangeConnectionEvent(args)    Add unconnected forc
e Group    
    struct connection_handler_args args;
*
static void ForceChannelChangeConnectionEvent(args)     Add unconnected f
orce channel
    struct connection_handler_args args;
*
static void SevrGroupChangeConnectionEvent(args)    Add unconnected sevr
group
    struct connection_handler_args args;
*
static void SevrChannelChangeConnectionEvent(args)    Add unconnected sev
r channel
    struct connection_handler_args args;

*/

/*****************************************************
 alCaUpdate -- Timer proc to update screen
****************************************************/
static void alCaUpdate(XtPointer cd, XtIntervalId *id)
{
     ALINK        *area;
     
#if DEBUG_CALLBACKS
    {
	static int n=0;
	
	printf("alCaUpdate: n=%d\n",n++);
    }
#endif
    
  /* Poll CA */
    ca_poll();

  /* Update areas */
    area = 0;
    if (areaList) area = (ALINK *)sllFirst(areaList);
    while (area) {
	if (area->pmainGroup && area->pmainGroup->p1stgroup){
	    alHighestSystemSeverity(area->pmainGroup->p1stgroup);
	    
	    if ( area->pmainGroup->modified ){
		if ( area->mapped && area->managed){
		    invokeDialogUpdate(area);
		    invokeSubWindowUpdate(area->treeWindow);
		    invokeSubWindowUpdate(area->groupWindow);
		}
		updateCurrentAlarmWindow(area);
		area->pmainGroup->modified = 0;
	    }
	}
	area = (ALINK *)sllNext(area);
    }
    
  /* Set the proc to be called again */
    caTimeoutId = XtAppAddTimeOut(appContext,caDelay,alCaUpdate,NULL);
}

/*****************************************************
 This function initializes channel access
****************************************************/
void alCaInit()
{
#if DEBUG_CALLBACKS
    {
	printf("alCaInit: caTimeoutId=%d\n",caTimeoutId);
    }
#endif
    
  /* Initialize channel access */
    SEVCHK(ca_task_initialize(),"alCaInit: error in ca_task_initialize");

  /* Register exception handler */
    SEVCHK(ca_add_exception_event(alCAException,NULL),
      "alCaInit: error in ca_add_exception_event");
    
  /* Register file descriptor callback */
    SEVCHK(ca_add_fd_registration(registerCA,NULL),
      "alCaInit: error in ca_add_fd_registration");

  /* Start the CA poll and update areas timer proc */
    caTimeoutId = XtAppAddTimeOut(appContext,caDelay,alCaUpdate,NULL);
#if DEBUG_CALLBACKS
	printf("          caTimeoutId=%d\n",caTimeoutId);
#endif
}

/********************************************************
 Callback when there is activity on a CA file descriptor
********************************************************/
static void alProcessCA(XtPointer cd, int *source, XtInputId *id)
{
#if DEBUG_CALLBACKS
    {
	static int n=0;

	printf("alProcessCA: n=%d\n",n++);
    }
#endif
    ca_poll();
}


/********************************************************
 Callback to register file descriptors
********************************************************/
static void registerCA(void *dummy, int fd, int opened)
{
    struct FDLIST *cur,*next;
    int found;
    
#if DEBUG_CALLBACKS
    {
	printf("registerCA: fd=%d opened=%d\n",fd,opened);
    }
#endif

  /* Branch depending on whether the fd is opened or closed */
    if(opened) {
      /* Look for a linked list structure for this fd */
	cur=lastFdInList;
	while(cur) {
	    if(cur->fd == fd) {
		errMsg("Tried to add a second callback "
		  "for file descriptor %d",fd);
		return;
	    }
	    cur=cur->prev;
	}
      /* Allocate and fill a linked list structure for this fd */
	cur=(struct FDLIST *)calloc(1,sizeof(struct FDLIST));
	if(cur == NULL) {
	    errMsg("Could not allocate space to keep track of "
	      "file descriptor %d",fd);
	    return;
	}
	cur->prev=lastFdInList;
	cur->inpid=XtAppAddInput(appContext,fd,(XtPointer)XtInputReadMask,
	alProcessCA,NULL);
 	cur->fd=fd;
	lastFdInList=cur;
    } else {
      /* Find the linked list structure for this fd */
	found=0;
	cur=next=lastFdInList;
	while(cur) {
	    if(cur->fd == fd) {
		found=1;
		break;
	    }
	    next=cur;
	    cur=cur->prev;
	}
      /* Remove the callback */
	if(found) {
	    XtRemoveInput(cur->inpid);
	    if(cur == lastFdInList) lastFdInList=cur->prev;
	    else next->prev=cur->prev;
	    free(cur);
	} else {
	    errMsg("Error removing callback for file descriptor %d",fd);
	}
    }
}

/******************************************************************************
 *  This function initializes channel access and adds channel access events 
 *  for each channel monitored in the alarm configuration.
  ****************************************************************************/
void alCaStart(proot)
SLIST *proot;
{
    int returnValue;

    alCaSearch((SLIST *)proot);
    ca_flush_io();

    returnValue = ca_pend_io(CA_PEND_IO_SECONDS);
    if (returnValue != ECA_NORMAL && returnValue != ECA_TIMEOUT )
        SEVCHK(returnValue,"alCaStart: error in channel access search");

    /*
     *  add change connection events
     */
    alCaAddEvents((SLIST *)proot);
    ca_flush_io();
}



/*****************************************************************************
 *  This function adds change connection events 
 *  for each channel monitored in the alarm configuration.
 *****************************************************************************/
void alCaStartEvents(proot)
SLIST *proot;
{
    int returnValue;

    ca_flush_io();
    returnValue = ca_pend_io(CA_PEND_IO_SECONDS);
    if (returnValue != ECA_NORMAL && returnValue != ECA_TIMEOUT )
        SEVCHK(returnValue,"alCaStartEvents: error in channel access search");

    /*
     *  add change connection events
     */
    alCaAddEvents((SLIST *)proot);
    ca_flush_io();
}


/*****************************************************************
 *  This function closes all the channel links, groups & subwindows
 *****************************************************************/
void alCaCancel(proot)
SLIST *proot;
{

    /* 
     * note: if proot == NULL then probably never even fired up initial
     *     configuration file and ca...
     */
    if (proot != NULL ) {

        /* cancel all the channel access evid */
        ClearChannelAccessEvents(proot);
        SEVCHK(ca_pend_io(1.0),"alCaCancel: error in cancel channel access evid");

        /* cancel all the channel access chid */
        ClearChannelAccessChids(proot);
        SEVCHK(ca_pend_io(1.0),"alCaCancel: error in cancel channel access chid");

    }

}

/*****************************************************************
 *  This function closes all the channel links, groups & subwindows
 *****************************************************************/
void alCaStop()
{
    /* cancel timeout */
    if (caTimeoutId) {
	XtRemoveTimeOut(caTimeoutId);
        caTimeoutId = (XtIntervalId)0;
    }

    /* and close channel access */
    SEVCHK(ca_task_exit(),"alCaCancel: error in ca_task_exit");

}


/*************************************************************
 this should be called for enable a channel alarm monitor
 **************************************************************/
void   alCaAddEvent(clink)
CLINK *clink;
{
    struct chanData *cdata;
    int status;


    cdata = clink->pchanData;

    status=ca_add_masked_array_event(DBR_STSACK_STRING, 1,
        cdata->chid, 
        NewAlarmEvent,
        clink, 
        (float)0, (float)0, (float)0, &(cdata->evid), DBE_ALARM);
    if (status != ECA_NORMAL)
        al_ca_error_code("alCaAddEvent","ca_add_masked_array_event",
            status,cdata->name);

    if (DEBUG == 1)
        printf("*** ca_field_type(cdata->chid)=%d TYPENOTCONN=%d\n",
            ca_field_type(cdata->chid),TYPENOTCONN);
    if (ca_field_type(cdata->chid) == TYPENOTCONN)
        alNewAlarm(COMM_ALARM, INVALID_ALARM, cdata->value, clink);
}


/***********************************************************
 this should be called for cancelling a channel alarm monitor
************************************************************/
void alCaClearEvent(clink)
CLINK *clink;
{
    struct chanData *cdata;
    int status;

    cdata = clink->pchanData;

    if (cdata->evid) {
        status = ca_clear_event(cdata->evid);
        if (status != ECA_NORMAL)
            al_ca_error_code("alCaClearEvent","ca_clear_event",
                status,cdata->name);
        cdata->evid = NULL;
    }
}



/********************************************************
        update the values of sevrPV records  according 
        to current severity whenever alarm changes 
********************************************************/
void  alCaPutSevr(clink)
CLINK *clink;
{
    GLINK *glink;
    struct chanData *cdata;
    struct groupData *gdata;
    short  sevr;
    int status;

    /*
     *      update the channel's value if sevrPVName is defined
     */
    cdata = clink->pchanData;

    /* put value to PV */
    if (cdata->sevrchid && 
        ca_field_type(cdata->sevrchid) != TYPENOTCONN) {
        sevr = cdata->curSevr;
        status=ca_put(DBR_SHORT,cdata->sevrchid,&sevr);
        if (status!=ECA_NORMAL)
            al_ca_error_code("alCaPutSevr","ca_put",status,cdata->sevrPVName);

        if (DEBUG == 1)
            printf("alCaPutSevr :Chann %s, %s, %d, type=%d\n",
                cdata->name,cdata->sevrPVName,cdata->curSevr,
                ca_field_type(cdata->sevrchid));
    }

    /*
     *      update the group's value if sevrPVName is defined
     */
    glink = clink->parent;
    while (glink) {
        gdata = glink->pgroupData;

        /* put value to PV */
        if (gdata->sevrchid && 
            ca_field_type(gdata->sevrchid) != TYPENOTCONN) {
            sevr = alHighestSeverity(gdata->curSev);
            status=ca_put(DBR_SHORT,gdata->sevrchid,&sevr);
            if (status!=ECA_NORMAL)
                al_ca_error_code("alCaPutSevr","ca_put",status,
                    gdata->sevrPVName);
            if (DEBUG == 1)
                printf("alCaPutSevr :Group %s,  %s, %d, type=%d\n",
                    gdata->name,gdata->sevrPVName,
                    alHighestSeverity(gdata->curSev),
                    ca_field_type(gdata->sevrchid));
        }
        glink = glink->parent;
    }
}


/********************************************************
       put sevr value to sevrPV 
********************************************************/
void  alCaPutSevrValue(char *sevrPVName,chid sevrchid,int sevr)
{
    int status;

    if (sevrchid && ca_field_type(sevrchid) != TYPENOTCONN) {
        status=ca_put(DBR_SHORT,sevrchid,&sevr);
        if (status!=ECA_NORMAL)
            al_ca_error_code("alCaPutSevrValue","ca_put",status,sevrPVName);
    }
}


/********************************************************
        send global alarm acknowledgement
********************************************************/
void  alCaPutGblAck(clink)
CLINK *clink;
{
    struct chanData *cdata;
    int status;

    cdata = clink->pchanData;

    if (ca_field_type(cdata->chid) != TYPENOTCONN) {
        status=ca_put(DBR_PUT_ACKS,cdata->chid,&cdata->unackSevr);
        if (status!=ECA_NORMAL)
            al_ca_error_code("alCaPutAck","ca_put",status,cdata->name);
    }
}


/***************************************************************
        add alarm event handler for each channel
***************************************************************/
void  alCaSearch(proot)
SLIST *proot;
{
    SNODE *pt,*temp;
    SLIST *clist;
    GLINK *glink;
    CLINK *clink;
    struct groupData *gdata;
    struct chanData *cdata;
    int status;

    /* 1. Add CA search for each non-zero PV names defined in configu
        ration file*/
    temp = sllFirst(proot);

    /* for each group in subGroupList */
    while (temp) {
        glink = (GLINK *)temp;
        gdata = glink->pgroupData;

        /* add group force search */
        if ( strlen(gdata->forcePVName) > (size_t) 1) {

            status = ca_search(gdata->forcePVName,&(gdata->forcechid));
            if (status!=ECA_NORMAL)
                al_ca_error_code("alCaSearch","ca_search",
                    status,gdata->forcePVName);
        }

        /* add group sevr search */
        if ( strlen(gdata->sevrPVName) >  (size_t)1) {
            status = ca_search(gdata->sevrPVName,&(gdata->sevrchid));
            if (status!=ECA_NORMAL)
                al_ca_error_code("alCaSearch","ca_search",
                    status,gdata->sevrPVName);
        }

        /* for each channel in chanList */
        clist = &(glink->chanList);
        if (clist) {
            pt = sllFirst(clist);
            while (pt)  {
                clink = (CLINK *)pt;
                cdata = clink->pchanData;

                /* add alarm channel search */
                status = ca_search(cdata->name,&(cdata->chid));
                if (status!=ECA_NORMAL)
                    al_ca_error_code("alCaSearch","ca_search",
                        status,cdata->name);

                /* add channel force channel search */
                if (strlen(cdata->forcePVName) > (size_t) 1) {
                    status = ca_search(cdata->forcePVName,
                        &(cdata->forcechid));
                    if (status!=ECA_NORMAL)
                        al_ca_error_code("alCaSearch",
                            "ca_search",status,cdata->forcePVName);
                }

                /* add channel sevr search */
                if (strlen(cdata->sevrPVName) > (size_t) 1) {
                    status = ca_search(cdata->sevrPVName,
                        &(cdata->sevrchid));
                    if (status!=ECA_NORMAL)
                        al_ca_error_code("alCaSearch",
                            "ca_search",status,cdata->sevrPVName);
                }
                pt = sllNext(pt);
            }
        }

        /* for each group in subGroupList */
        alCaSearch(&(glink->subGroupList));
        temp = sllNext(temp);
    }
}


/***************************************************************
    clear alarm event handler for each channel
***************************************************************/
void alCaSearchName(name, pchid)
char *name;
chid *pchid;
{
    int status;

    status = ca_search(name,pchid);
    if (status!=ECA_NORMAL)
        al_ca_error_code("alCaSearchName","ca_search",status,name);
    /*
        ca_flush_io();
    */
}



/***************************************************************
    clear alarm event handler for each channel
***************************************************************/
void ClearChannelAccessEvents(proot)
SLIST *proot;
{
    SNODE *pt,*temp;
    SLIST *clist;
    GLINK *glink;
    CLINK *clink;
    struct chanData *cdata;
    struct groupData *gdata;
    MASK mask;
    int status;


    temp = sllFirst(proot);

    /* for each group */
    while (temp) {
        glink = (GLINK *)temp;
        gdata = glink->pgroupData;

        if (gdata->forceevid != NULL) {
            status = ca_clear_event(gdata->forceevid);
            if (status != ECA_NORMAL)
                al_ca_error_code("ClearChannelAccessEvents",
                    "ca_clear_event",
                    status,gdata->forcePVName);
            gdata->forceevid = NULL;

        }

        /* for each channel in chanlist */
        clist = &(glink->chanList);
        if (clist) {
            pt = sllFirst(clist);
            while (pt)  {
                clink = (CLINK *)pt;
                cdata = clink->pchanData;
                mask = cdata->curMask;
                if (mask.Cancel == 0 && cdata->evid != NULL) {
                    status = ca_clear_event(cdata->evid);
                    if (status != ECA_NORMAL)
                        al_ca_error_code("ClearChannelAccessEvents",
                            "ca_clear_event",
                            status,cdata->name);
                    cdata->evid = NULL;
                }

                if (cdata->forceevid != NULL) {
                    status = ca_clear_event(cdata->forceevid);
                    if (status != ECA_NORMAL)
                        al_ca_error_code("ClearChannelAccessEvents",
                            "ca_clear_event",
                            status,cdata->forcePVName);
                    cdata->forceevid = NULL;
                }
                pt = sllNext(pt);
            }
        }


        /* for each group in subGroupList */
        ClearChannelAccessEvents(&(glink->subGroupList));
        temp = sllNext(temp);
    }
}


/***************************************************************
       clear all the  channel chids for restart  
***************************************************************/
void ClearChannelAccessChids(proot)
SLIST *proot;
{
    SNODE *pt,*temp;
    SLIST *clist;
    GLINK *glink;
    CLINK *clink;
    struct groupData *gdata;
    struct chanData *cdata;
    int status;

    temp = sllFirst(proot);

    /* for each group in subGroupList */

    while (temp) {
        glink = (GLINK *)temp;
        gdata = glink->pgroupData;

        /* cancel group force channel */
        if ( gdata->forcechid) {
            status = ca_clear_channel(gdata->forcechid);
            if (status!=ECA_NORMAL)
                al_ca_error_code("ClearChannelAccessChids",
                    "ca_clear_channel",
                    status,gdata->forcePVName);
        }

        /* cancel group sevr channel */
        if ( gdata->sevrchid ) {
            status = ca_clear_channel(gdata->sevrchid);
            if (status!=ECA_NORMAL)
                al_ca_error_code("ClearChannelAccessChids",
                    "ca_clear_channel",
                    status,gdata->sevrPVName);
        }

        /* for each channel in chanList */
        clist = &(glink->chanList);
        if (clist) {
            pt = sllFirst(clist);
            while (pt)  {
                clink = (CLINK *)pt;
                cdata = clink->pchanData;

                /* cancel alarm channel */
                if (cdata->chid) {
                    status = ca_clear_channel(cdata->chid);
                    if (status!=ECA_NORMAL)
                        al_ca_error_code("ClearChannelAccessChids",
                            "ca_clear_channel",
                            status,cdata->name);
                }

                /* cancel force channel */
                if (cdata->forcechid) {
                    status = ca_clear_channel(cdata->forcechid);
                    if (status!=ECA_NORMAL)
                        al_ca_error_code("ClearChannelAccessChids",
                            "ca_clear_channel",
                            status,cdata->forcePVName);
                }

                /* cancel sevr channel */
                if (cdata->sevrchid) {
                    status = ca_clear_channel(cdata->sevrchid);
                    if (status!=ECA_NORMAL)
                        al_ca_error_code("ClearChannelAccessChids",
                            "ca_clear_channel",
                            status,cdata->sevrPVName);
                }

                pt = sllNext(pt);
            }
        }

        /* for each group in subGroupList */
        ClearChannelAccessChids(&(glink->subGroupList));

        temp = sllNext(temp);
    }
}


/***************************************************************
        add channel access events for each channel
***************************************************************/
void  alCaAddEvents(proot)
SLIST *proot;
{
    SNODE *pt,*temp;
    SLIST *clist;
    GLINK *glink;
    CLINK *clink;
    struct groupData *gdata;
    struct chanData *cdata;
    int status;
    MASK mask;

    /*   Add the alarm events & change connection events */
    temp = sllFirst(proot);
    while (temp) {
        glink = (GLINK *)temp;

        /* for the subgroup */
        gdata = glink->pgroupData;

        if (strlen(gdata->forcePVName) > (size_t) 1) {

            /* add force group mask event */
            status = ca_add_masked_array_event(DBR_SHORT,1,
                gdata->forcechid,
                GroupForceEvent,
                glink,
                (float)0,(float)0,(float)0,
                &(gdata->forceevid),
                DBE_VALUE);
            if (status != ECA_NORMAL)
                al_ca_error_code("alCaAddEvents","ca_add_masked_array_event",
                    status,gdata->forcePVName);

            /* add change connection event for forcePV */
            ca_puser(gdata->forcechid) = gdata;
            ca_change_connection_event(gdata->forcechid,
                ForceGroupChangeConnectionEvent);
        }

        /* add change connection event for group sevrPV */
        if (strlen(gdata->sevrPVName) > (size_t) 1) {

            ca_puser(gdata->sevrchid) = gdata;
            ca_change_connection_event(gdata->sevrchid,
                SevrGroupChangeConnectionEvent);
        }

        /* for each channel in chanList */
        clist = &(glink->chanList);
        if (clist) {
            pt = sllFirst(clist);
            while (pt)  {
                clink = (CLINK *)pt;
                cdata = clink->pchanData;

                mask = cdata->curMask;
                if (mask.Cancel == 0) {
                    status = ca_add_masked_array_event(DBR_STSACK_STRING,
                        1,
                        cdata->chid,
                        NewAlarmEvent,
                        clink,
                        (float)0,(float)0,(float)0,
                        &(cdata->evid),
                        DBE_ALARM);
                    if (status != ECA_NORMAL)
                        al_ca_error_code("alCaAddEvents",
                            "ca_add_masked_array_event",
                            status,cdata->name);

                    /* set new comm alarm if it is no
                        t connected */
                    /* comm status  stat = 9 */
                    /* valid alarm  sevr = 3   ? 4 */

                    if (ca_field_type(cdata->chid) ==
                        TYPENOTCONN) {
                        alNewAlarm(COMM_ALARM, INVALID_ALARM,
                            cdata->value, clink);
                    }
                }

                /* add change connection event */
                ca_puser(cdata->chid) =  clink;
                ca_change_connection_event(cdata->chid,
                    AlarmChangeConnectionEvent);

                if (strlen(cdata->forcePVName) > (size_t) 1) {

                    /* add force channel mask event */

                    status = ca_add_masked_array_event(DBR_SHORT,
                        1,
                        cdata->forcechid,
                        ChannelForceEvent,
                        clink,
                        (float)0,(float)0,(float)0,
                        &(cdata->forceevid),
                        DBE_VALUE);
                    if (status != ECA_NORMAL)
                        al_ca_error_code("alCaAddEvents",
                            "ca_add_masked_array_event",
                            status,cdata->forcePVName);

                    /* add change connection event fo
                        r forcePV */

                    ca_puser(cdata->forcechid) = cdata;
                    ca_change_connection_event(cdata->forcechid,
                        ForceChannelChangeConnectionEvent);
                }

                /* add change connection event for channel sevrPV */
                if (strlen(cdata->sevrPVName) > (size_t) 1) {
                    ca_puser(cdata->sevrchid) = cdata;
                    ca_change_connection_event(cdata->sevrchid,
                        SevrChannelChangeConnectionEvent);
                }
                pt = sllNext(pt);
            }
        }

        /* for each subGroupList */
        alCaAddEvents(&(glink->subGroupList));
        temp = sllNext(temp);
    }
}


/***********************************************************
    NewAlarmEvent get channel sevr, status, value
    then invoke alNewAlarm  & update value of SevrPVName
***********************************************************/
static void NewAlarmEvent(args)
struct event_handler_args args;
{
    CLINK *clink;
    struct chanData *cdata;
    int stat,sevr;
    int ackt,acks;
    char value[MAX_STRING_SIZE];

    stat= ((struct dbr_stsack_string *)args.dbr)->status;
    sevr = ((struct dbr_stsack_string *)args.dbr)->severity;
    ackt = ((struct dbr_stsack_string *)args.dbr)->ackt;
    acks = ((struct dbr_stsack_string *)args.dbr)->acks;
    strcpy(value,((struct dbr_stsack_string *)args.dbr)->value);

    clink = (CLINK *)args.usr;
    cdata = clink->pchanData;

    switch (args.status) {
    case ECA_NORMAL:
        alNewEvent(stat,sevr,acks,value,clink);
        break;
    case ECA_NORDACCESS:
        alNewAlarm(READ_ACCESS_ALARM,INVALID_ALARM,value,clink);
        break;
    default:
        al_ca_error_code("NewAlarmEvent","args.status",args.status,
            cdata->name);
        alNewAlarm(COMM_ALARM,INVALID_ALARM,value,clink);
    }
}


/*******************************************************************
  Group Force Event 
*******************************************************************/
static void GroupForceEvent( args)
struct event_handler_args args;
{
    GLINK *glink;
    struct groupData *gdata;
    short value;
    short *p;

    glink = (GLINK *)args.usr;
    gdata = glink->pgroupData;
    if (strlen(gdata->forcePVName) > (size_t) 1) {
        switch (args.status) {
        case ECA_NORMAL:
            p = (short *)args.dbr;
            value = *p;
            gdata->PVValue = value;
            if (value == gdata->forcePVValue) {
                alChangeGroupMask(glink,gdata->forcePVMask);
                ca_poll();
                alLogForcePVGroup(glink,AUTOMATIC);
            }
            if (value == gdata->resetPVValue) {
                alResetGroupMask(glink);
                ca_poll();
                alLogResetPVGroup(glink,AUTOMATIC);
            }
            glink->pmainGroup->modified = 1;
            break;
        default:
            al_ca_error_code("GroupForceEvent","args.status",
                args.status,
                gdata->forcePVName);
        }
    }
}

/******************************************************************
  Channel Force Event
******************************************************************/
static void ChannelForceEvent(args)
struct event_handler_args args;
{
    CLINK *clink;
    struct chanData *cdata;
    short value;
    short *p;

    clink = (CLINK *)args.usr;
    cdata = clink->pchanData;
    if (strlen(cdata->forcePVName) > (size_t) 1) {

#ifdef ACCESS_SECURITY
        if (args.status != ECA_NORMAL) {
            al_ca_error_code("ChannelForceEvent","args.status",
                args.status,
                cdata->forcePVName);
            return;
        }
#endif /*ACCESS_SECURITY*/

        p = (short *)args.dbr;

        value = *p;
        cdata->PVValue = value;

        if (value == cdata->forcePVValue) {

            /* change   channel mask to force mask  */
            alOperatorForcePVChanEvent(clink,cdata->forcePVMask);

            ca_poll();

            /* log automatic channel force mask event ? */
            alLogForcePVChan(clink,AUTOMATIC);
        }

        if (value == cdata->resetPVValue) {
            /* change channel mask to default mask  */
            alOperatorForcePVChanEvent(clink,cdata->defaultMask);

            ca_poll();

            /* log automatic channel reset mask event ? */
            alLogResetPVChan(clink,AUTOMATIC);
        }
    }

    /*    awInvokeCallback();  */
    clink->pmainGroup->modified = 1;
}


/*******************************************************************
  Alarm change connection handler event
*******************************************************************/
static void AlarmChangeConnectionEvent(args)
struct connection_handler_args args;
{
    CLINK *clink;
    struct chanData *cdata;

    /* set new comm alarm if it is not connected */
    if (ca_field_type(args.chid) == TYPENOTCONN) {
        /* comm status  stat = 9 */
        /* valid alarm  sevr = 3   ? 4 */

        clink = (CLINK *)ca_puser(args.chid);
        cdata = clink->pchanData;

        alNewAlarm(COMM_ALARM,INVALID_ALARM, cdata->value, clink);
        alLogConnection(cdata->name, "Not Connected (Channel  PVname)");
    }
}


/*********************************************************************
 forcePV group change of connection handler event
**********************************************************************/
static void ForceGroupChangeConnectionEvent(args)
struct connection_handler_args args;
{
    struct groupData *gdata;

    if (args.op == CA_OP_CONN_DOWN) {
        gdata = (struct groupData *)ca_puser(args.chid);
        sprintf(buff," %s--(%s)",gdata->name,gdata->forcePVName);
        alLogConnection(buff, "Not Connected (Force Gp PVName)");
    }
}


/*********************************************************************
 forcePV channel change of connection handler event
**********************************************************************/
static void ForceChannelChangeConnectionEvent(args)
struct connection_handler_args args;
{
    struct chanData *cdata;

    if (args.op == CA_OP_CONN_DOWN) {
        cdata = (struct chanData *)ca_puser(args.chid);
        sprintf(buff," %s--(%s)",cdata->name,cdata->forcePVName);
        alLogConnection(buff, "Not Connected (Force Ch PVName)");
    }
}


/*********************************************************************
 sevrPV group change of connection handler event
**********************************************************************/
static void SevrGroupChangeConnectionEvent(args)
struct connection_handler_args args;
{
    struct groupData *gdata;

    if (args.op == CA_OP_CONN_DOWN) {
        gdata = (struct groupData *)ca_puser(args.chid);
        sprintf(buff," %s--(%s)",gdata->name,gdata->sevrPVName);
        alLogConnection(buff, "Not Connected (Sevr  Gp PVName)");
    }
}


/*********************************************************************
 sevr channel change of connection handler event
**********************************************************************/
static void SevrChannelChangeConnectionEvent(args)
struct connection_handler_args args;
{
    struct chanData *cdata;

    if (args.op == CA_OP_CONN_DOWN) {
        cdata = (struct chanData *)ca_puser(args.chid);
        sprintf(buff," %s--(%s)",cdata->name,cdata->sevrPVName);
        alLogConnection(buff,"Not Connected (Sevr  Ch PVName)");
    }
}


/*********************************************************************
 al_ca_error_code
**********************************************************************/
static void al_ca_error_code(alhName,caName,status,PVname)
char *alhName;
char *caName;
int status;
char *PVname;
{
    if (status == ECA_NORMAL)
        printf("CA success in %s called from %s for %s\nMessage: [%s]\n",
            caName,
            alhName,
            PVname,
            ca_message(status));

    else {
    /*
         printf(" %s: [%s]\n",
          PVname,
          ca_message(status));
    */
    printf("CA failure in %s called from %s for %s\nMessage: [%s]\n",
        caName,
        alhName,
        PVname,
        ca_message(status));
    }
}


/**************************************************************
  Replace old group force event with the new force group event
*************************************************************/
void alReplaceGroupForceEvent(glink,str)
GLINK *glink;
char *str;
{

    int status;
    struct groupData *gdata;

    gdata = glink->pgroupData;

    /* remove old force event */
    if (gdata->forceevid) {
        status = ca_clear_event(gdata->forceevid);
        if (status != ECA_NORMAL)
            al_ca_error_code("alReplaceGroupForceEvent","ca_clear_event",
                status,gdata->forcePVName);
        gdata->forceevid = NULL;
    }

    /* change forcePvName  */
    if(strcmp(gdata->forcePVName,"-") !=0) free(gdata->forcePVName);
    gdata->forcePVName = (char *)calloc(1,strlen(str)+1);
    strcpy(gdata->forcePVName,str);

    /* add new ca search */
    status = ca_search(gdata->forcePVName,&(gdata->forcechid));
    if (status != ECA_NORMAL) {
        al_ca_error_code("alReplaceGroupForceEvent","ca_search",
            status,gdata->forcePVName);
    }


    /* add new force group mask event */
    status = ca_add_masked_array_event(DBR_SHORT,1,
        gdata->forcechid,
        GroupForceEvent,
        glink,
        (float)0,(float)0,(float)0,
        &(gdata->forceevid),
        DBE_VALUE);
    if (status != ECA_NORMAL) {
        al_ca_error_code("alReplaceGroupForceEvent","ca_add_masked_array_event",
            status,gdata->forcePVName);
    }

    /* add change connection event for forcePV */
    ca_puser(gdata->forcechid) = gdata;
    ca_change_connection_event(gdata->forcechid,
        ForceGroupChangeConnectionEvent);
}


/***************************************************************
  Replace the old force event with the new force channel event
**************************************************************/
void alReplaceChanForceEvent(clink,str)
CLINK *clink;
char *str;
{

    int status;
    struct chanData *cdata;

    cdata = clink->pchanData;

    /* remove old force event */
    if (cdata->forceevid) {
        status = ca_clear_event(cdata->forceevid);
        if (status != ECA_NORMAL)
            al_ca_error_code("alReplaceChanForceEvent","ca_clear_event",
                status,cdata->forcePVName);
        cdata->forceevid = NULL;
    }

    /* change forcePvName  */
    if(strcmp(cdata->forcePVName,"-") !=0) free(cdata->forcePVName);
    cdata->forcePVName = (char *)calloc(1,strlen(str)+1);
    strcpy(cdata->forcePVName,str);

    /* add new ca search */
    status = ca_search(cdata->forcePVName,&(cdata->forcechid));
    if (status != ECA_NORMAL) {
        al_ca_error_code("alReplaceChanForceEvent","ca_search",status,
            cdata->forcePVName);
    }

    /* add new force chan mask event */
    status = ca_add_masked_array_event(DBR_SHORT,1,
        cdata->forcechid,
        ChannelForceEvent,
        clink,
        (float)0,(float)0,(float)0,
        &(cdata->forceevid),
        DBE_VALUE);
    if (status != ECA_NORMAL) {
        al_ca_error_code("alReplaceChanForceEvent","ca_add_masked_array_event",
            status,cdata->forcePVName);
    }

    /* add change connection event for forcePV */
    ca_puser(cdata->forcechid) = cdata;
    ca_change_connection_event(cdata->forcechid,
        ForceGroupChangeConnectionEvent);
}

static void alCAException(struct exception_handler_args args)
{
#define MAX_EXCEPTIONS 25    
    static int nexceptions=0;
    static int ended=0;

    if(ended) return;
    if(nexceptions++ > MAX_EXCEPTIONS) {
	ended=1;
	errMsg("alCAException: Channel Access Exception:\n"
	  "Too many exceptions [%d]\n"
	  "No more will be handled\n"
	  "Please fix the problem and restart ALH",
	  MAX_EXCEPTIONS);
	ca_add_exception_event(NULL, NULL);
	return;
    }
    
    errMsg("alCAException: Channel Access Exception:\n"
      "  Channel Name: %s\n"
      "  Native Type: %s\n"
      "  Native Count: %hu\n"
      "  Access: %s%s\n"
      "  IOC: %s\n"
      "  Message: %s\n"
      "  Context: %s\n"
      "  Requested Type: %s\n"
      "  Requested Count: %ld\n"
      "  Source File: %s\n"
      "  Line number: %u",
      args.chid?ca_name(args.chid):"Unavailable",
      args.chid?dbf_type_to_text(ca_field_type(args.chid)):"Unavailable",
      args.chid?ca_element_count(args.chid):0,
      args.chid?(ca_read_access(args.chid)?"R":""):"Unavailable",
      args.chid?(ca_write_access(args.chid)?"W":""):"",
      args.chid?ca_host_name(args.chid):"Unavailable",
      ca_message(args.stat)?ca_message(args.stat):"Unavailable",
      args.ctx?args.ctx:"Unavailable",
      dbf_type_to_text(args.type),
      args.count,
      args.pFile?args.pFile:"Unavailable",
      args.pFile?args.lineNo:0);
}
