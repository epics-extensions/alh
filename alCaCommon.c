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
/* alCaCommon.c */

/************************DESCRIPTION***********************************
  This file contains ca routines common to cdev and epics ca.
  This file also contains misc group and channel routines.
**********************************************************************/

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
extern int _transients_flag;
extern int _global_flag;
extern int _passive_flag;
extern int _description_field_flag;

/* Struct for file descriptor linked list */
struct FDLIST {
	struct FDLIST *prev;
	XtInputId inpid;
	int fd;
};

struct FDLIST *lastFdInList=(struct FDLIST *)0;

#define AUTOMATIC    0
#define UNKNOWN 0

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
static GCLINK *nextGroupChannel(GCLINK *gclink,int *plinkType)
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


/*****************************************************************
   alSetNotConnected
 *****************************************************************/
void alSetNotConnected(struct mainGroup *pmainGroup)
{
	GCLINK *gclink;
	struct gcData *gcdata;
	struct chanData *cdata;
	int type;

	if (!toBeConnectedCount) return;

	if (pmainGroup->heartbeatPV.chid && !alCaIsConnected(pmainGroup->heartbeatPV.chid) ) {
		errMsg("Heartbeat PV %s Not Connected\n",pmainGroup->heartbeatPV.name);
	}

	if (!pmainGroup) return;

	gclink = firstGroupChannel((SLIST *)pmainGroup,&type);
	while (gclink) {
		gcdata = gclink->pgcData;
		alForcePVSetNotConnected(gcdata->pforcePV,gcdata->name);
		if (gcdata->sevrchid && !alCaIsConnected(gcdata->sevrchid) ) {
			errMsg("Severity PV %s for %s Not Connected\n",
				gcdata->sevrPVName, gcdata->name);
		}
		if (type == CHANNEL ) {
			cdata = ((struct chanData *)gcdata);
			if (cdata && cdata->ackPVId && !alCaIsConnected(cdata->ackPVId) ) {
				errMsg("Acknowledge PV %s for %s Not Connected\n",
					cdata->ackPVName, cdata->name);
			}
			if (cdata  && cdata->chid  && !alCaIsConnected(cdata->chid)) {
				alNewEvent(NOT_CONNECTED,ERROR_STATE,0,-1,"",(CLINK *)gclink);
			}
		}
		gclink = nextGroupChannel(gclink,&type);
	}
}

/*****************************************************************
   alPutGblAckT
 *****************************************************************/
void alPutGblAckT(struct mainGroup *pmainGroup)
{
	GCLINK *gclink;
	struct chanData *cdata;
	int type;

	if (!pmainGroup) return;
	if ( !_transients_flag || !_global_flag ||_passive_flag) return;

	gclink = firstGroupChannel((SLIST*)pmainGroup,&type);
	while (gclink) {
		if (type == CHANNEL) {
			cdata = ((CLINK *)gclink)->pchanData;
			if ( alCaIsConnected(cdata->chid)) {
			/*NOTE: ackt and defaultMask.AckT have opposite meaning */
				short ackt = (cdata->defaultMask.AckT+1)%2;
				alCaPutGblAckT(cdata->chid,&ackt);
			}
		}
		gclink = nextGroupChannel(gclink,&type);
	}
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
	#define alhInputMask XtInputReadWinsock
#else
	#define alhInputMask XtInputReadMask
#endif

	/* Branch depending on whether the fd is opened or closed */
	if(opened) {
		/* Look for a linked list structure for this fd */
		cur=lastFdInList;
		while(cur) {
			if(cur->fd == fd) {
				errMsg("Tried to add a second callback for file descriptor %d\n",fd);
				return;
			}
			cur=cur->prev;
		}
		/* Allocate and fill a linked list structure for this fd */
		cur=(struct FDLIST *)calloc(1,sizeof(struct FDLIST));
		if(cur == NULL) {
			errMsg("Could not allocate space to keep track of file descriptor %d\n",fd);
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
			errMsg("Error removing callback for file descriptor %d\n",fd);
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

/*****************************************************************
	close all the channel links, groups & subwindows
 *****************************************************************/
void alCaCancel(struct mainGroup *pmainGroup)
{
	GCLINK *gclink;
	struct gcData *gcdata;
	int type;

	if (!pmainGroup) return;

	if (pmainGroup->heartbeatPV.chid) alCaClearChannel(&(pmainGroup->heartbeatPV.chid));

	gclink = firstGroupChannel((SLIST *)pmainGroup,&type);
	while (gclink) {
		gcdata = gclink->pgcData;
		alForcePVClearCA(gcdata->pforcePV);
		alCaClearChannel(&gcdata->sevrchid);
		if (type == CHANNEL) {
			alCaClearEvent(&((struct chanData *)gcdata)->evid);
			alCaClearChannel(&((struct chanData *)gcdata)->chid);
			alCaClearChannel(&((struct chanData *)gcdata)->ackPVId);
			if(_description_field_flag) 
			  alCaClearChannel(&((struct chanData *)gcdata)->descriptionId);
		}
		gclink = nextGroupChannel(gclink,&type);
	}
	alCaPoll();
}

