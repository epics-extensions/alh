/* alCaCommon.c */

/************************DESCRIPTION***********************************
  This file contains ca routines common to cdev and epics ca.
  This file also contains misc group and channel routines.
**********************************************************************/

static char *sccsId = "@(#) $Id$";

/******************************************************************
  Some misc. routines
******************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "alarm.h"
#include "fdmgr.h"
#include "cadef.h"

#include "sllLib.h"
#include "alLib.h"
#include "alh.h"
#include "ax.h"

extern SLIST *areaList;
extern int toBeConnectedCount;

/* Struct for file descriptor linked list */
struct FDLIST {
	struct FDLIST *prev;
	XtInputId inpid;
	int fd;
};

struct FDLIST *lastFdInList=(struct FDLIST *)0;

#define AUTOMATIC    0
#define UNKNOWN 0

/****************************************************************************
	initializes channel access and adds channel access events
	for each channel monitored in the alarm configuration.
****************************************************************************/
void alCaStart(SLIST *proot)
{
	toBeConnectedCount=0;
	alCaSearch(proot);
	alCaPend(2.0);
	alSetNotConnected(proot);
}

/**************************************************************
  Replace old group force event with the new force group event
*************************************************************/
void            alReplaceGroupForceEvent(GLINK *glink,char *str)
{
	struct groupData *gdata;

	gdata = glink->pgroupData;

	/* remove old force event */
	alCaClearEvent(&gdata->forceevid);

	/* change forcePvName  */
	if (strcmp(gdata->forcePVName, "-") != 0)
		free(gdata->forcePVName);
	gdata->forcePVName = (char *) calloc(1, strlen(str) + 1);
	strcpy(gdata->forcePVName, str);

	/* add new ca connection */
	alCaConnectForcePV(gdata->forcePVName,&gdata->forcechid,gdata->name);

	/* add new force group mask event */
	alCaAddForcePVEvent(gdata->forcechid,glink,&gdata->forceevid,GROUP);
}

/***************************************************************
  Replace the old force event with the new force channel event
**************************************************************/
void            alReplaceChanForceEvent(CLINK *clink,char *str)
{
	struct chanData *cdata;

	cdata = clink->pchanData;

	/* remove old force event */
	alCaClearEvent(&cdata->forceevid);

	/* change forcePvName  */
	if (strcmp(cdata->forcePVName, "-") != 0)
		free(cdata->forcePVName);
	cdata->forcePVName = (char *) calloc(1, strlen(str) + 1);
	strcpy(cdata->forcePVName, str);

	/* add new ca connection */
	alCaConnectForcePV(cdata->forcePVName,&cdata->forcechid,cdata->name);

	/* add new force group mask event */
	alCaAddForcePVEvent(cdata->forcechid,clink,&cdata->forceevid,CHANNEL);
}

/***************************************************************
	get first group or channel
***************************************************************/
static GCLINK *firstGroupChannel(SLIST *proot,int *plinkType)
{
	*plinkType = UNKNOWN;
	if (proot == NULL )  return NULL;

	if ((GCLINK *)sllFirst(proot)) *plinkType = GROUP;
	return (GCLINK *)sllFirst(proot);
}

/***************************************************************
	get next group or channel
***************************************************************/
GCLINK *nextGroupChannel(GCLINK *gclink,int *plinkType)
{
	SNODE *pt;
	GCLINK *next=0;
	GLINK *glink;

	if (gclink == NULL )  return NULL;

	if (*plinkType == GROUP) {

		glink=(GLINK *)gclink;

		/* get first child group*/
		next = (GCLINK *)sllFirst(&(glink->subGroupList));
		if (next) return next;

		/* get first channel */
		next = (GCLINK *)sllFirst(&(glink->chanList));
		if (next) {
			*plinkType = CHANNEL;
			return next;
		}

	}

	/* get next group or channel at same level*/
	next= 0;
	pt = (SNODE *)gclink;
	while (pt) {
		next = (GCLINK *)sllNext(pt);
		if (next) return next;
		pt =(SNODE *)((GCLINK *)pt)->parent;
		if (pt && *plinkType == GROUP ) {
			glink=(GLINK *)pt;
			next = (GCLINK *)sllFirst(&(glink->chanList));
			*plinkType = CHANNEL;
			if (next) return next;
		}
		*plinkType = GROUP;
	}
	if (!next) *plinkType = UNKNOWN;
	return next;
}

/***************************************************************
	add alarm event handler for each channel
***************************************************************/
void            alCaSearch(SLIST *proot)
{
	GCLINK *gclink;
	struct gcData *gcdata;
	struct chanData *cdata;
	int type;

	gclink = firstGroupChannel(proot,&type);
	while (gclink) {
		gcdata = gclink->pgcData;

		alCaConnectForcePV(gcdata->forcePVName,&gcdata->forcechid,gcdata->name);
		alCaAddForcePVEvent(gcdata->forcechid,gclink,&gcdata->forceevid,type);

		alCaConnectSevrPV(gcdata->sevrPVName,&gcdata->sevrchid,gcdata->name);

		if (type == CHANNEL) {
			cdata = (struct chanData *)gcdata;
			alCaConnectChannel(cdata->name,&cdata->chid,gclink);
			alCaAddEvent(cdata->chid,&cdata->evid,gclink);
		}

		gclink = nextGroupChannel(gclink,&type);
	}
}

/*****************************************************************
	close all the channel links, groups & subwindows
 *****************************************************************/
void alCaCancel(SLIST *proot)
{
	GCLINK *gclink;
	struct gcData *gcdata;
	int type;

	if (!proot) return;

	gclink = firstGroupChannel(proot,&type);
	while (gclink) {
		gcdata = gclink->pgcData;
		alCaClearEvent(&gcdata->forceevid);
		alCaClearChannel(&gcdata->forcechid);
		alCaClearChannel(&gcdata->sevrchid);
		if (type == CHANNEL) {
			alCaClearEvent(&((struct chanData *)gcdata)->evid);
			alCaClearChannel(&((struct chanData *)gcdata)->chid);
		}
		gclink = nextGroupChannel(gclink,&type);
	}
	alCaPoll();
}

/*****************************************************************
   alSetNotConnected
 *****************************************************************/
void alSetNotConnected(SLIST *proot)
{
	GCLINK *gclink;
	struct gcData *gcdata;
	int type;
	char buff[81];

	if (!proot) return;
	if (!toBeConnectedCount) return;

	gclink = firstGroupChannel(proot,&type);
	while (gclink) {
		gcdata = gclink->pgcData;
		if ( gcdata->forcechid && !alCaIsConnected(gcdata->forcechid) ) {
			sprintf(buff, "%s--(%s)", gcdata->name, gcdata->forcePVName);
			alLogConnection(buff, "Not Connected (Force    PVName)");
		}
		if (gcdata->sevrchid && !alCaIsConnected(gcdata->sevrchid) ) {
			sprintf(buff, "%s--(%s)", gcdata->name, gcdata->sevrPVName);
			alLogConnection(buff, "Not Connected (Sevr     PVName)");
		}
		if (type == CHANNEL && ((struct chanData *)gcdata)->chid &&
		    !alCaIsConnected(((struct chanData *)gcdata)->chid)) {
			alNewAlarm(COMM_ALARM,INVALID_ALARM,"0",(CLINK *)gclink);
		}
		gclink = nextGroupChannel(gclink,&type);
	}
}

/******************************************************************
	Channel Force Event
******************************************************************/
void alChannelForceEvent(CLINK *clink,short value)
{
	struct chanData *cdata;

	cdata = clink->pchanData;
	if (strlen(cdata->forcePVName) <= (size_t) 1) return;

	cdata->PVValue = value;
	if (value == cdata->forcePVValue) {
		alOperatorForcePVChanEvent(clink, cdata->forcePVMask);
		alCaFlushIo();
		alLogForcePVChan(clink, AUTOMATIC);
	}
	if (value == cdata->resetPVValue) {
		alOperatorForcePVChanEvent(clink, cdata->defaultMask);
		alCaFlushIo();
		alLogResetPVChan(clink, AUTOMATIC);
	}
	clink->pmainGroup->modified = 1;
}




/*******************************************************************
    Group Force Event
*******************************************************************/
void     alGroupForceEvent(GLINK *glink,short value)
{
	struct groupData *gdata;

	gdata = glink->pgroupData;
	if (strlen(gdata->forcePVName) <= (size_t) 1) return;

	gdata->PVValue = value;
	if (value == gdata->forcePVValue) {
		alChangeGroupMask(glink, gdata->forcePVMask);
		alCaFlushIo();
		alLogForcePVGroup(glink, AUTOMATIC);
	}
	if (value == gdata->resetPVValue) {
		alResetGroupMask(glink);
		alCaFlushIo();
		alLogResetPVGroup(glink, AUTOMATIC);
	}
	glink->pmainGroup->modified = 1;
}

/********************************************************
 Callback when there is activity on a CA file descriptor
********************************************************/
static void alProcessCA(XtPointer cd, int *source, XtInputId *id)
{
	alCaPoll();
}

/********************************************************
 Callback to register file descriptors
********************************************************/
void registerCA(void *dummy, int fd, int opened)
{
	struct FDLIST *cur,*next;
	int found;
#ifdef WIN32
	int alhInputMask=XtInputReadWinsock;
#else
	int alhInputMask=XtInputReadMask;
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
		cur->inpid=XtAppAddInput(appContext,fd,(XtPointer)alhInputMask,
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

/********************************************************
 Update all areas
********************************************************/
void alUpdateAreas()
{
	ALINK *area;

	area = 0;
	if (areaList) area = (ALINK *)sllFirst(areaList);
	while (area) {
		if (area->pmainGroup && area->pmainGroup->p1stgroup){
			alHighestSystemSeverity(area->pmainGroup->p1stgroup);

			if ( area->pmainGroup->modified ){
				if ( area->mapped && area->managed){
					/*invokeDialogUpdate(area);*/
					invokeSubWindowUpdate(area->treeWindow);
					invokeSubWindowUpdate(area->groupWindow);
				}
				if  (area->managed) updateCurrentAlarmWindow(area);
				area->pmainGroup->modified = 0;
			}
		}
		area = (ALINK *)sllNext(area);
	}
}

