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
 *				add force mask events and change connection 
 *				events 
 *                               
 * .03  10-04-91        bkc     Add the clear channel events,
 *				freeing the memory pointers, and
 *				adding the new froce PV events
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
#include <fdmgr.h>
#include <cadef.h>

#include <sllLib.h>
#include <alLib.h>
#include <alh.h>
#include <ax.h>


#define AUTOMATIC	0

#define CA_PEND_EVENT_TIME	 0.001	

#define  LOG_UNCONN_ALARM            1
#define  LOG_UNCONN_FORCE_GROUP      2
#define  LOG_UNCONN_FORCE_CHANNEL    3
#define  LOG_UNCONN_SEVR_GROUP       4
#define  LOG_UNCONN_SEVR_CHANNEL     5

static char buff[81];

fdctx *pfdctx;			/* fdmgr context */
void *caTimeoutId;
static struct timeval caDelay = {10, 0};
 
extern int DEBUG;
extern XtAppContext appContext;
 
/* external functions 

extern alLogConnection();
extern alForcePVChanEvent();
extern alForcePVGroupEvent();
extern void alOperatorForcePVGroupEvent();
extern void alOperatorForcePVChanEvent();
extern void alOperatorResetPVGroupEvent();
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
static void ForceGroupChangeConnectionEvent( struct connection_handler_args args);
static void ForceChannelChangeConnectionEvent( struct connection_handler_args args);
static void SevrGroupChangeConnectionEvent( struct connection_handler_args args);
static void SevrChannelChangeConnectionEvent( struct connection_handler_args args);
static void al_ca_error_code(char *alhName,char *caName,int status,char *PVname);
static void registerCA(void *pfdctx, int fd, int condition);
void alCaPendEvent(void);

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
void alCaPendEvent();

#endif /*__STDC__*/

/*
*-----------------------------------------------------------------
*    routines related to system startup , initialization & termination 
*-----------------------------------------------------------------
*
------------
|  PUBLIC  |
------------
void alCaInit()			 	Initialize channel access
*
void alCaStart(proot)			Start and setup for channel access
	SLIST *proot;
*
void alCaStartEvents(proot)		Add change connection events
	SLIST *proot;
*
void alCaCancel(proot)			Disconnect channel access 
	SLIST *proot;
*
void alCaClearEvent(clink)		Remove a channel alarm event
	CLINK *clink;
* 
void alCaAddEvent(clink)		Add a channel alarm event
	CLINK *clink;
*
void alProcessCA()			ca_pend_event
* 
void alCaPutSevr(clink)			ca_put SevrPVValue
	CLINK *clink;
*
void  alCaSearch(glink)			Add all channel access events
	SLIST *glink;
*
void  alCaAddEvents(proot)		Add change connection events
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
static void registerCA(pfdctx,fd,condition)
*
static ClearChannelAccessEvents(glink)		Clear Channel access events
	SLIST *glink;		
*
static ClearChannelAccessChids(proot) 		Clear channel access chids
	SLIST *proot;
*
static void NewAlarmEvent(args)			New alarm event call back function
	struct event_handler_args args;
*
static void GroupForceEvent( args)		Force group event call back
	struct event_handler_args args;
*
static void ChannelForceEvent(args)		Force channel event call back  
	struct event_handler_args args;
*
static void AlarmChangeConnectionEvent(args)	Add alarm not connected event
	struct connection_handler_args args;
*
static void ForceGroupChangeConnectionEvent(args)	Add unconnected force Group	
	struct connection_handler_args args;
*
static void ForceChannelChangeConnectionEvent(args) 	Add unconnected force channel
	struct connection_handler_args args;
*
static void SevrGroupChangeConnectionEvent(args)	Add unconnected sevr group
	struct connection_handler_args args;
*
static void SevrChannelChangeConnectionEvent(args)	Add unconnected sevr channel
	struct connection_handler_args args;

*/

/*****************************************************
 This function initializes fdmgr
****************************************************/
void alFdmgrInit(display)
Display *display;
{
/*
 *  initialize fdmgr 
 */
	pfdctx = (void *) fdmgr_init();

/*
 * add X's fd to fdmgr ...
 */
	fdmgr_add_fd(pfdctx, ConnectionNumber(display), alProcessX, NULL);

}

/*****************************************************
 alCaPendEvent
****************************************************/
void alCaPendEvent()
{

     ca_pend_event(.00001);
     caTimeoutId = (void *)fdmgr_add_timeout(pfdctx,&caDelay,alCaPendEvent,NULL);
}

/*****************************************************
 This function only initializes channel access
****************************************************/
void alCaInit()
{
/*
 *  initialize channel access
 */
        SEVCHK(ca_task_initialize(),"alCaInit: error in ca_task_initialize");

/*
 * and add CA fd to fdmgr ...
 */
	SEVCHK(ca_add_fd_registration(registerCA,pfdctx),
		"alCaInit: error in ca_add_fd_registration");

     caTimeoutId = (void *)fdmgr_add_timeout(pfdctx,&caDelay,alCaPendEvent,NULL);

}

/********************************************************
 *	functions to register channel access file descriptor with XT
 * 	and perform CA handling when events exist on that event stream
 *	 
********************************************************/
void alProcessCA()
{
  ca_flush_io();
  ca_pend_event(CA_PEND_EVENT_TIME);
}

void alProcessX()
{
XEvent event;

  while (XtAppPending(appContext)) {
	XtAppNextEvent(appContext,&event);
	XtDispatchEvent(&event);
	}
}


static void registerCA(pfdctx,fd,condition)
  void *pfdctx;
  int fd;
  int condition;
{


	if (condition) {
		fdmgr_add_fd(pfdctx,fd,alProcessCA,NULL);
		}
	else {
		fdmgr_clear_fd(pfdctx,fd);
		}
}


/******************************************************************************
 *  This function initializes channel access and adds channel access events 
 *  for each channel monitored in the alarm configuration.
 ******************************************************************************/
void alCaStart(proot)
SLIST *proot;
{
     int returnValue;

/*
 *  initialize channel access
 */

/*
        alCaInit();
*/

/*
 * add alarm ,forcePV ,sevrPV channal access search and Events
 */

     alCaSearch((SLIST *)proot);
     ca_flush_io();

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



/******************************************************************************
 *  This function adds change connection events 
 *  for each channel monitored in the alarm configuration.
 ******************************************************************************/
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
 *	 configuration file and ca...
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

   /* cancel registration of the CA file descriptors */
/*
      SEVCHK(ca_add_fd_registration(registerCA,pfdctx),
		"alCaCancel:  error in ca_add_fd_registration");
*/

     /* cancel timeout */
     if (caTimeoutId) {
          fdmgr_clear_timeout(pfdctx,caTimeoutId);
          caTimeoutId = NULL;
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
void NewAlarmEvent();
int status;


        cdata = clink->pchanData;

    status=ca_add_masked_array_event(DBR_STS_STRING, 1,
		cdata->chid, 
		NewAlarmEvent,
		clink, 
		(float)0, (float)0, (float)0, &(cdata->evid), DBE_ALARM);
	al_ca_error_code("alCaAddEvent","ca_add_masked_array_event",status,cdata->name);

if (DEBUG == 1) 
printf("*** ca_field_type(cdata->chid)=%d TYPENOTCONN=%d\n",ca_field_type(cdata->chid),TYPENOTCONN);
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
	al_ca_error_code("alCaClearEvent","ca_clear_event",status,cdata->name);
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
	                    al_ca_error_code("alCaPutSevr","ca_put",status,gdata->sevrPVName);
if (DEBUG == 1) 
printf("alCaPutSevr :Group %s,  %s, %d, type=%d\n",
gdata->name,gdata->sevrPVName,alHighestSeverity(gdata->curSev),
ca_field_type(gdata->sevrchid));

                        }
                glink = glink->parent;
                }


/*        SEVCHK(ca_flush_io(),
                "alCaPutSevr : error ca_flush_io"); */

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

/* 1. Add CA search for each non-zero PV names defined in configuration file*/


        temp = sllFirst(proot);

/* for each group in subGroupList */

        while (temp) { 
        glink = (GLINK *)temp; 
        gdata = glink->pgroupData;

		/* add group force search */

		if ( strlen(gdata->forcePVName) > 1) {

		status = ca_search(gdata->forcePVName,&(gdata->forcechid));
                if (status!=ECA_NORMAL) 
	            al_ca_error_code("alCaSearch","ca_search",status,gdata->forcePVName);


			}

		/* add group sevr search */

		if ( strlen(gdata->sevrPVName) > 1) {
		status = ca_search(gdata->sevrPVName,&(gdata->sevrchid));
                if (status!=ECA_NORMAL) 
	            al_ca_error_code("alCaSearch","ca_search",status,gdata->sevrPVName);
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
	            al_ca_error_code("alCaSearch","ca_search",status,cdata->name);
	


	/* add channel force channel search */

		if (strlen(cdata->forcePVName) > 1) {

		status = ca_search(cdata->forcePVName,&(cdata->forcechid));
                if (status!=ECA_NORMAL) 
	            al_ca_error_code("alCaSearch","ca_search",status,cdata->forcePVName);
    
  			}


	/* add channel sevr search */

		if (strlen(cdata->sevrPVName) > 1) {

		status = ca_search(cdata->sevrPVName,&(cdata->sevrchid));
                if (status!=ECA_NORMAL) 
	            al_ca_error_code("alCaSearch","ca_search",status,cdata->sevrPVName);
 
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
	            al_ca_error_code("ClearChannelAccessEvents","ca_clear_event",status,gdata->forcePVName);
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
	        al_ca_error_code("ClearChannelAccessEvents","ca_clear_event",status,cdata->name);
			cdata->evid = NULL;
				}

			if (cdata->forceevid != NULL) {
			status = ca_clear_event(cdata->forceevid);
			if (status != ECA_NORMAL) 
	        al_ca_error_code("ClearChannelAccessEvents","ca_clear_event",status,cdata->forcePVName);
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
	        al_ca_error_code("ClearChannelAccessChids","ca_clear_channel",status,gdata->forcePVName);
                }

       /* cancel group sevr channel */

        if ( gdata->sevrchid ) {
                status = ca_clear_channel(gdata->sevrchid);
                if (status!=ECA_NORMAL) 
	            al_ca_error_code("ClearChannelAccessChids","ca_clear_channel",status,gdata->sevrPVName);
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
	            al_ca_error_code("ClearChannelAccessChids","ca_clear_channel",status,cdata->name);
        
		}


       /* cancel force channel */

           if (cdata->forcechid) {
                status = ca_clear_channel(cdata->forcechid);
                if (status!=ECA_NORMAL) 
	            al_ca_error_code("ClearChannelAccessChids","ca_clear_channel",status,cdata->forcePVName);
    
                        }


       /* cancel sevr channel */

            if (cdata->sevrchid) {
                status = ca_clear_channel(cdata->sevrchid);
                if (status!=ECA_NORMAL) 
	            al_ca_error_code("ClearChannelAccessChids","ca_clear_channel",status,cdata->sevrPVName);
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
                
        if (strlen(gdata->forcePVName) > 1) {

		/* add force group mask event */

		status = ca_add_masked_array_event(DBR_SHORT,1,
				gdata->forcechid,
				GroupForceEvent,
				glink,
				(float)0,(float)0,(float)0,
				&(gdata->forceevid),
				DBE_VALUE);
		if (status != ECA_NORMAL) 
	            al_ca_error_code("alCaAddEvents","ca_add_masked_array_event",status,gdata->forcePVName);

		/* add change connection event for forcePV */

 
		ca_puser(gdata->forcechid) = gdata; 
        	ca_change_connection_event(gdata->forcechid,
			ForceGroupChangeConnectionEvent);


                }

	/* add change connection event for group sevrPV */

        if (strlen(gdata->sevrPVName) > 1) {

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
		status = ca_add_masked_array_event(DBR_STS_STRING,1,
				cdata->chid,
				NewAlarmEvent,
				clink,
				(float)0,(float)0,(float)0,
				&(cdata->evid),
				DBE_ALARM);
		if (status != ECA_NORMAL) 
	        al_ca_error_code("alCaAddEvents","ca_add_masked_array_event",status,cdata->name);


		/* set new comm alarm if it is not connected */
        	/* comm status  stat = 9 */
        	/* valid alarm  sevr = 3   ? 4 */

        	if (ca_field_type(cdata->chid) == TYPENOTCONN) {
        		alNewAlarm(COMM_ALARM, INVALID_ALARM, cdata->value, clink);
			}
		}

		/* add change connection event */

		ca_puser(cdata->chid) =  clink;

         	ca_change_connection_event(cdata->chid,
			AlarmChangeConnectionEvent);	
	



        if (strlen(cdata->forcePVName) > 1 ) {

		/* add force channel mask event */

		status = ca_add_masked_array_event(DBR_SHORT,1,
				cdata->forcechid,
				ChannelForceEvent,
				clink,
				(float)0,(float)0,(float)0,
				&(cdata->forceevid),
				DBE_VALUE);
		if (status != ECA_NORMAL) 
	        al_ca_error_code("alCaAddEvents","ca_add_masked_array_event",status,cdata->forcePVName);

		/* add change connection event for forcePV */

		ca_puser(cdata->forcechid) = cdata; 
         	ca_change_connection_event(cdata->forcechid,
			ForceChannelChangeConnectionEvent);

 		}

	/* add change connection event for channel sevrPV */

        	if (strlen(cdata->sevrPVName) > 1 ) {
 
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
/*
int stat,sevr;
char value[MAX_STRING_SIZE];

	cdata->type = args.chid->type;

	

	strcpy(value,((struct dbr_sts_string *)args.dbr)->value);

        stat = ((struct dbr_sts_string *)args.dbr)->status;
        sevr = ((struct dbr_sts_string *)args.dbr)->severity;

        	alNewAlarm(stat,sevr,value,clink);
*/
        clink = (CLINK *)args.usr;
        cdata = clink->pchanData;

		if (args.status != ECA_NORMAL) {
			if (args.status == ECA_NORDACCESS) 
        		alNewAlarm(READ_ACCESS_ALARM,INVALID_ALARM,
					((struct dbr_sts_string *)args.dbr)->value,
					args.usr);
			else if (args.status == ECA_NOWTACCESS)   
        		alNewAlarm(WRITE_ACCESS_ALARM,INVALID_ALARM,
					((struct dbr_sts_string *)args.dbr)->value,
					args.usr);
			else {
	       		al_ca_error_code("NewAlarmEvent","args.status",args.status,cdata->name);
        		alNewAlarm(COMM_ALARM,INVALID_ALARM,
					((struct dbr_sts_string *)args.dbr)->value,
					args.usr);
			}
        } else {

        	alNewAlarm(
                   ((struct dbr_sts_string *)args.dbr)->status,
                   ((struct dbr_sts_string *)args.dbr)->severity,
	           ((struct dbr_sts_string *)args.dbr)->value,
                   args.usr);
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
	if (strlen(gdata->forcePVName) > 1) {

    if (args.status != ECA_NORMAL) 
        al_ca_error_code("GroupForceEvent","args.status",args.status,gdata->forcePVName);
	
	p = (short *)args.dbr;

	value = *p;
	gdata->PVValue = value;

	if (value == gdata->forcePVValue) {

		/* change every subgroup channel to group force mask  */

    		/*alForcePVGroupEvent(glink,gdata->forcePVValue); */
		
		alOperatorForcePVGroupEvent(glink,gdata->forcePVMask);

		alProcessCA();

		/* log automatic group force mask event ? */

		alLogForcePVGroup(glink,AUTOMATIC);

		}


	if (value == gdata->resetPVValue) {

		/* change every channel in the group to default mask  */

		/* alForcePVGroupEvent(glink,gdata->resetPVValue); */

		alOperatorResetPVGroupEvent(glink);

		alProcessCA();

		/* log automatic group reset mask event ? */

		alLogResetPVGroup(glink,AUTOMATIC);	
		}

	}

/*	awInvokeCallback();  */
 
 	glink->pmainGroup->modified = 1;
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
	if (strlen(cdata->forcePVName) > 1) {

    if (args.status != ECA_NORMAL) {
        al_ca_error_code("ChannelForceEvent","args.status",args.status,cdata->forcePVName);
        return;
    }
	
	p = (short *)args.dbr;

	value = *p;
	cdata->PVValue = value;

	if (value == cdata->forcePVValue) {

	/* change   channel mask to force mask  */

		/* alForcePVChanEvent(clink,cdata->forcePVValue); */

       		alOperatorForcePVChanEvent(clink,cdata->forcePVMask);

		alProcessCA();

	/* log automatic channel force mask event ? */

		alLogForcePVChan(clink,AUTOMATIC);

		}


	if (value == cdata->resetPVValue) {

	/* change every channel in the group to default mask  */

		/* alForcePVChanEvent(clink,cdata->resetPVValue); */

       		alOperatorForcePVChanEvent(clink,cdata->defaultMask);

		alProcessCA();

	/* log automatic channel reset mask event ? */

		alLogResetPVChan(clink,AUTOMATIC);
	
		}

	}


/*	awInvokeCallback();  */
 
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

/*
 		printf("Alarm  PVName: [%s] not connected\n",
		ca_name(args.chid));
*/

		clink = (CLINK *)ca_puser(args.chid);
 		cdata = clink->pchanData;
 
        	alNewAlarm(COMM_ALARM,INVALID_ALARM, cdata->value, clink); 
		
	/* log the not connected in log file */

		sprintf(buff,"(%s)",cdata->name );
		alLogConnection(buff, LOG_UNCONN_ALARM);

		}

}



/*********************************************************************
 force group change of connection handler event
**********************************************************************/
static void ForceGroupChangeConnectionEvent(args)
struct connection_handler_args args;
{
struct groupData *gdata;

	if (args.op == CA_OP_CONN_DOWN) {
/*
		printf("Group forcePVName: [%s] not connected\n",
		ca_name(args.chid));
*/
	/* log the not connected in log file */

		gdata = (struct groupData *)ca_puser(args.chid);

 		sprintf(buff," %s--(%s)",gdata->name,gdata->forcePVName);
 		alLogConnection(buff, LOG_UNCONN_FORCE_GROUP);


		}

}
	

/*********************************************************************
 force channel change of connection handler event
**********************************************************************/
static void ForceChannelChangeConnectionEvent(args)
struct connection_handler_args args;
{
struct chanData *cdata;
	if (args.op == CA_OP_CONN_DOWN) {
/*
		printf("Channel forcePVName: [%s] not connected\n",
		ca_name(args.chid));
*/

	/* log the not connected in log file */


		cdata = (struct chanData *)ca_puser(args.chid);

		sprintf(buff," %s--(%s)",cdata->name,cdata->forcePVName);
 		alLogConnection(buff, LOG_UNCONN_FORCE_CHANNEL);

		
		}

}


/*********************************************************************
 sevr group change of connection handler event
**********************************************************************/
static void SevrGroupChangeConnectionEvent(args)
struct connection_handler_args args;
{
struct groupData *gdata;

	if (args.op == CA_OP_CONN_DOWN) {
/*
		printf("Group sevrPVName: [%s] not connected\n",
		ca_name(args.chid));
*/

	/* log the not connected in log file */

		gdata = (struct groupData *)ca_puser(args.chid);

		sprintf(buff," %s--(%s)",gdata->name,gdata->sevrPVName);
		alLogConnection(buff, LOG_UNCONN_SEVR_GROUP);

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
/*
		printf("Channel sevrPVName: [%s] not connected\n",
		ca_name(args.chid));
*/

	/* log the not connected in log file */

		cdata = (struct chanData *)ca_puser(args.chid);

		sprintf(buff," %s--(%s)",cdata->name,cdata->sevrPVName);
		alLogConnection(buff, LOG_UNCONN_SEVR_CHANNEL);

		}

}



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
      ca_message_text[CA_EXTRACT_MSG_NO(status)]);

else {
/*
     printf(" %s: [%s]\n",
      PVname,
      ca_message_text[CA_EXTRACT_MSG_NO(status)]);
*/
     printf("CA failure in %s called from %s for %s\nMessage: [%s]\n",
      caName,
      alhName,
      PVname,
      ca_message_text[CA_EXTRACT_MSG_NO(status)]);
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
	    al_ca_error_code("alReplaceGroupForceEvent","ca_clear_event",status,gdata->forcePVName);
	gdata->forceevid = NULL;
		}

	/* change forcePvName  */

    if(strcmp(gdata->forcePVName,"-") !=0) free(gdata->forcePVName);
    gdata->forcePVName = (char *)calloc(1,strlen(str)+1);
	strcpy(gdata->forcePVName,str);

	/* add new ca search */

	status = ca_search(gdata->forcePVName,&(gdata->forcechid));
	if (status != ECA_NORMAL) {
	    al_ca_error_code("alReplaceGroupForceEvent","ca_search",status,gdata->forcePVName);
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
	    al_ca_error_code("alReplaceGroupForceEvent","ca_add_masked_array_event",status,gdata->forcePVName);
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
	    al_ca_error_code("alReplaceChanForceEvent","ca_clear_event",status,cdata->forcePVName);
	cdata->forceevid = NULL;
		}

	/* change forcePvName  */

    if(strcmp(cdata->forcePVName,"-") !=0) free(cdata->forcePVName);
    cdata->forcePVName = (char *)calloc(1,strlen(str)+1);
	strcpy(cdata->forcePVName,str);

	/* add new ca search */

	status = ca_search(cdata->forcePVName,&(cdata->forcechid));
	if (status != ECA_NORMAL) {
	    al_ca_error_code("alReplaceChanForceEvent","ca_search",status,cdata->forcePVName);
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
	    al_ca_error_code("alReplaceChanForceEvent","ca_add_masked_array_event",status,cdata->forcePVName);
		}

	/* add change connection event for forcePV */

	ca_puser(cdata->forcechid) = cdata; 
        ca_change_connection_event(cdata->forcechid,
			ForceGroupChangeConnectionEvent);


}

