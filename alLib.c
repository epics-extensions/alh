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
/* alLib.c */

/************************DESCRIPTION***********************************
  Routines for primative group & channel operation
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#define DEBUG_CALLBACKS 0

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sllLib.h"
#include "alLib.h"
#include "ax.h"

extern char * alhAlarmSeverityString[];
extern char * alhAlarmStatusString[];

/* global variables */
extern int _passive_flag;
extern int _global_flag;
extern int _DB_call_flag;
extern int DEBUG;
extern struct setup psetup;

/* forward declarations */
static void alarmCountFilter_callback(XtPointer cd, XtIntervalId *id);
static void alNewAlarmFilter(int stat,int sevr,int acks,int ackt,
	char *value,CLINK *clink);
static void alNewAlarmProcess(int stat,int sev, int acks,int ackt,
	char *value, CLINK *clink,time_t timeofday);
void alSetAckTChan(CLINK *clink,int ackt);
short alHighestBeepSeverity(int sevr[ALH_ALARM_NSEV], int beepSevr);


/**********************************************************************
      allocate main Group  
***********************************************************************/
struct mainGroup *alAllocMainGroup()
{
	struct mainGroup *pmainGroup;

	pmainGroup = (struct mainGroup *)calloc(1,sizeof(struct mainGroup));
	pmainGroup->p1stgroup = NULL;
	pmainGroup->modified = FALSE;
	return(pmainGroup);
}

/**********************************************************************
	 append glink at the end of subgrouplist 
***********************************************************************/
void alAddGroup(GLINK *parent,GLINK *glink)
{
	sllAdd(&(parent->subGroupList),(SNODE *)glink);
	glink->parent = parent;
}

/************************************************************************
	 append at  the end of Channel list 
*************************************************************************/
void alAddChan(GLINK *parent,CLINK *clink)
{
	sllAdd(&(parent->chanList),(SNODE *)clink);
	clink->parent = parent;
}

/**********************************************************************
	 insert a glink before another glink in the subgrouplist 
***********************************************************************/
void alPrecedeGroup(GLINK *parent,GLINK *sibling,GLINK *glink)
{
	if (parent == NULL || glink == NULL )  return;
	sllPrecede(&(parent->subGroupList),(SNODE *)sibling,(SNODE *)glink);
	glink->parent = parent;
}

/**********************************************************************
	 insert a chan before another chan in the subgrouplist 
***********************************************************************/
void alPrecedeChan(GLINK *parent,CLINK *sibling,CLINK *clink)
{

	if (parent == NULL || clink == NULL )  return;
	sllPrecede(&(parent->chanList),(SNODE *)sibling,(SNODE *)clink);
	clink->parent = parent;
}

/************************************************************************
	 delete a channel link from chanList 
***********************************************************************/
void alDeleteChan(CLINK *clink)
{
	struct chanData *cdata;

	if (clink != NULL) {
		cdata = clink->pchanData;
		if(clink->parent) sllRemove(&(clink->parent->chanList),(SNODE *)clink);

		if (cdata->name) free(cdata->name);
		if (strcmp(cdata->forcePVName,"-") != 0) free(cdata->forcePVName);
		if (strcmp(cdata->sevrPVName,"-") != 0) free(cdata->sevrPVName);
		if (cdata->command) free(cdata->command);
        if (cdata->countFilter){
            if (cdata->countFilter->alarmTimeHistory){
                free(cdata->countFilter->alarmTimeHistory);
            }
     		if (cdata->countFilter->timeoutId){
				XtRemoveTimeOut(cdata->countFilter->timeoutId);
			}
            free(cdata->countFilter);
        }
     	if (cdata->noAckTimerId){
			XtRemoveTimeOut(cdata->noAckTimerId);
		}
		if (cdata->alias) free(cdata->alias);

		removeSevrCommandList(&cdata->sevrCommandList);
		removeStatCommandList(&cdata->statCommandList);

		if (clink->guidanceLocation) free(clink->guidanceLocation);
		guidanceDeleteGuideList(&clink->GuideList);

		free(cdata);
		free(clink);
	}
}

/*******************************************************************
   assume that glink had been removed by calling
        alRemoveGroup(glink) 
    before calling this function to free pointers
********************************************************************/
void alDeleteGroup(GLINK *glink)
{
	SNODE *cnode,*gnode,*next;
	GLINK *pt;
	struct groupData *gdata;

	if (glink) {
		/* free all channels */
		cnode = sllFirst(&(glink->chanList));
		while (cnode) {
			next = cnode->next;
			alDeleteChan((CLINK *)cnode);
			cnode = next;
		}
		/* free all subgroups */
		gnode = sllFirst(&(glink->subGroupList));
		while (gnode) {
			next = gnode->next;
			pt = (GLINK *)gnode;
			if (pt) {
				if (pt->parent) sllRemove(&(pt->parent->subGroupList),(SNODE *)pt);
				alDeleteGroup(pt);
				gnode = next;
			}
		}
		gdata = glink->pgroupData;
		if (gdata->name) free(gdata->name);
		if (strcmp(gdata->forcePVName,"-") != 0) free(gdata->forcePVName);
		if (strcmp(gdata->sevrPVName,"-") != 0) free(gdata->sevrPVName);
		if (gdata->command) free(gdata->command);
     	if (gdata->noAckTimerId){
			XtRemoveTimeOut(gdata->noAckTimerId);
		}
		if (gdata->alias) free(gdata->alias);
		if (gdata->treeSym) free(gdata->treeSym);

		removeSevrCommandList(&gdata->sevrCommandList);

		if (glink->guidanceLocation) free(glink->guidanceLocation);
		guidanceDeleteGuideList(&glink->GuideList);

		free(gdata);
		free(glink);
	}
}

/***************************************************************
	 remove glink from subGroupList 
***************************************************************/
void alRemoveGroup(GLINK *glink)
{
	if (glink) {
		if (glink->parent) sllRemove(&(glink->parent->subGroupList),(SNODE *)glink);
		/*
		        glink->node.next = NULL;
		        glink->parent = NULL;
		*/
	}
}

/***************************************************************
	 remove clink from chanList 
***************************************************************/
void alRemoveChan(CLINK *clink)
{
	if (clink != NULL) {
		if (clink->parent ) sllRemove(&(clink->parent->chanList),(SNODE *)clink);
		/*
					clink->node.next = NULL;
					clink->parent = NULL;
		*/
	}
}

/*******************************************************************
	set pmainGroup
*******************************************************************/
void alSetPmainGroup(GLINK *glink,struct mainGroup *pmainGroup)
{
	CLINK *clink;
	SNODE *node;

	if (!glink) return;

	glink->pmainGroup = pmainGroup;

	/* update all channels */
	node = sllFirst(&(glink->chanList));
	while (node) {
		clink = (CLINK *)node;
		clink->pmainGroup = pmainGroup;
		node = sllNext(node);
	}

	/* update all subGroups */
	node = sllFirst(&(glink->subGroupList));
	while (node) {
		alSetPmainGroup((GLINK *)node, pmainGroup);
		node = sllNext(node);
	}

}

/*******************************************************************
	make a copy of a Group
*******************************************************************/
GLINK *alCopyGroup(GLINK *glink)
{
	CLINK *clink;
	GLINK *glinkNew;
	GLINK *glinkTemp;
	struct groupData *gdata;
	struct groupData *gdataNew;
	char *buff;
	SNODE *node;

#if 0

modify parents:    
	( int)              glink->mask[ALARM_NMASK];
modify parents:    
	( int)               glink->viewCount;

modify children:   
	(struct mainGroup *) glink->pmainGroup;
modify children:   
	(void *)             glink->pgroupData->treeSym;

#endif

	if (!glink) return 0;

	glinkNew = alCreateGroup();
	gdataNew = glinkNew->pgroupData;
	gdata = glink->pgroupData;

	/* copy viewCount and pmainGroup */
	glinkNew->viewCount = glink->viewCount;
	glinkNew->pmainGroup = glink->pmainGroup;

	/* copy command */
	buff = gdata->command;
	if (buff){
		gdataNew->command = (char*)calloc(1,strlen(buff)+1);
		strcpy(gdataNew->command,buff);
	}

	/* copy alias */
	buff = gdata->alias;
	if (buff){
		gdataNew->alias = (char*)calloc(1,strlen(buff)+1);
		strcpy(gdataNew->alias,buff);
	}

	/* copy sevr commands */
	copySevrCommandList(&gdata->sevrCommandList,&gdataNew->sevrCommandList);

	gdataNew->beepSevr = gdata->beepSevr;

	/* copy sevrPV info */
	buff = gdata->sevrPVName;
	if (buff){
		gdataNew->sevrPVName = (char*)calloc(1,strlen(buff)+1);
		strcpy(gdataNew->sevrPVName,buff);
	}
	gdataNew->PVValue = gdata->PVValue;
	;


	/* copy name */
	if (gdataNew->name) free(gdataNew->name);
	buff = gdata->name;
	if (buff){
		gdataNew->name = (char*)calloc(1,strlen(buff)+1);
		strcpy(gdataNew->name,buff);
	}

	/* copy treeSym */
	buff = gdata->treeSym;
	if (buff){
		gdataNew->treeSym = (char*)calloc(1,strlen(buff)+1);
		strcpy(gdataNew->treeSym,buff);
	}

	/* copy forcePV info */
	buff = gdata->forcePVName;
	if(strcmp(buff,"-") != 0){
		gdataNew->forcePVName = (char*)calloc(1,strlen(buff)+1);
		strcpy(gdataNew->forcePVName,buff);
	} else gdataNew->forcePVName = buff;
	gdataNew->forcePVMask = gdata->forcePVMask;
	gdataNew->forcePVValue = gdata->forcePVValue;
	gdataNew->resetPVValue = gdata->resetPVValue;

	/* copy guidance */
	buff = glink->guidanceLocation;
	if (buff) {
		glinkNew->guidanceLocation = (char*)calloc(1,strlen(buff)+1);
		strcpy(glinkNew->guidanceLocation,buff);
	}
	guidanceCopyGuideList(&glinkNew->GuideList,&glink->GuideList);

	/* copy all channels */
	node = sllFirst(&(glink->chanList));
	while (node) {
		clink = alCopyChan((CLINK *)node);
		;
		alAddChan(glinkNew, clink);
		node = sllNext(node);
	}

	/* copy all subGroups */
	node = sllFirst(&(glink->subGroupList));
	while (node) {
		glinkTemp = alCopyGroup((GLINK *)node);
		;
		alAddGroup(glinkNew, glinkTemp);
		node = sllNext(node);
	}

	return(glinkNew);

}

/*******************************************************************
	make a copy of a Channel
*******************************************************************/
CLINK *alCopyChan(CLINK *clink)
{
	CLINK *clinkNew;
	char *buff;
	struct chanData *cdataNew;
	struct chanData *cdata;
    int i;

#if 0

modify parents:    
	( int)              clink->mask[ALARM_NMASK];
modify parents:    
	( int)               clink->viewCount;

modify children:   
	(struct mainGroup *) clink->pmainGroup;
modify children:   
	(void *)             clink->pchanData->treeSym;

#endif

	if (!clink) return 0;

	clinkNew = alCreateChannel();
	cdataNew = clinkNew->pchanData;
	cdata = clink->pchanData;

	/* copy viewCount and pmainGroup */
	clinkNew->viewCount = clink->viewCount;
	clinkNew->pmainGroup = clink->pmainGroup;

	/* copy command */
	buff = cdata->command;
	if (buff){
		cdataNew->command = (char*)calloc(1,strlen(buff)+1);
		strcpy(cdataNew->command,buff);
	}

	/* copy countFilter */
	if (cdata->countFilter){
		cdataNew->countFilter = (COUNTFILTER *)calloc(1,sizeof(COUNTFILTER));
		cdataNew->countFilter->inputCount=cdata->countFilter->inputCount;
		cdataNew->countFilter->inputSeconds=cdata->countFilter->inputSeconds;
		cdataNew->countFilter->clink=cdata->countFilter->clink;
        if (cdataNew->countFilter->inputCount){
            cdataNew->countFilter->alarmTimeHistory =
                (time_t *)calloc(2*(cdataNew->countFilter->inputCount),sizeof(time_t));
            for (i=0;i<=2*(cdataNew->countFilter->inputCount)-1;i++){
            cdataNew->countFilter->alarmTimeHistory[i]=cdata->countFilter->alarmTimeHistory[i];
	        }
	    }
	}

	/* copy alias */
	buff = cdata->alias;
	if (buff){
		cdataNew->alias = (char*)calloc(1,strlen(buff)+1);
		strcpy(cdataNew->alias,buff);
	}

	/* copy sevr commands */
	copySevrCommandList(&cdata->sevrCommandList,&cdataNew->sevrCommandList);

	/* copy stat commands */
	copyStatCommandList(&cdata->statCommandList,&cdataNew->statCommandList);

	/* copy sevrPV info */
	buff = cdata->sevrPVName;
	if (buff){
		cdataNew->sevrPVName = (char*)calloc(1,strlen(buff)+1);
		strcpy(cdataNew->sevrPVName,buff);
	}
	cdataNew->PVValue = cdata->PVValue;

	cdataNew->beepSevr = cdata->beepSevr;

	/* copy name */
	if (cdataNew->name) free(cdataNew->name);
	buff = cdata->name;
	if (buff){
		cdataNew->name = (char*)calloc(1,strlen(buff)+1);
		strcpy(cdataNew->name,buff);
	}

	/* copy forcePV info */
	buff = cdata->forcePVName;
	if(strcmp(buff,"-") != 0){
		cdataNew->forcePVName = (char*)calloc(1,strlen(buff)+1);
		strcpy(cdataNew->forcePVName,buff);
	} else cdataNew->forcePVName = buff;
	cdataNew->forcePVMask = cdata->forcePVMask;
	cdataNew->forcePVValue = cdata->forcePVValue;
	cdataNew->resetPVValue = cdata->resetPVValue;

	/* copy guidance */
	buff = clink->guidanceLocation;
	if (buff) {
		clinkNew->guidanceLocation = (char*)calloc(1,strlen(buff)+1);
		strcpy(clinkNew->guidanceLocation,buff);
	}
	guidanceCopyGuideList(&clinkNew->GuideList,&clink->GuideList);

	return(clinkNew);

}

/*******************************************************************
	Create a new Group
*******************************************************************/
GLINK *alCreateGroup()
{
	GLINK *link;
	struct groupData *gdata;

	link = (GLINK *)calloc(1,sizeof(GLINK));
	link->pgroupData = (struct groupData *)calloc(1,sizeof(struct groupData));
	gdata = link->pgroupData;

	link->viewCount = 1;
	link->parent = NULL;
	gdata->name = (char *)calloc(1,PVNAME_SIZE+1);
	strcpy(gdata->name,"Unnamed_Group");
	alSetMask("-----",&(gdata->forcePVMask));
	gdata->forcePVValue = 1;
	gdata->resetPVValue = 0;
	gdata->forcePVName = "-";
	gdata->sevrPVName = "-";

	return(link);
}

/*******************************************************************
	Create a new Channel
*******************************************************************/
CLINK *alCreateChannel()
{
	CLINK *link;
	struct chanData *cdata;

	link = (CLINK *)calloc(1,sizeof(CLINK));
	link->pchanData = (struct chanData *)calloc(1,sizeof(struct chanData));
	cdata = link->pchanData;

	link->viewCount =1;
	cdata->name = (char *)calloc(1,PVNAME_SIZE+1);
	strcpy(cdata->name,"Unnamed_Channel");
	alSetMask("-----",&(cdata->curMask));
	cdata->defaultMask = cdata->curMask;
	cdata->forcePVMask = cdata->curMask;
	cdata->forcePVValue = 1;
	cdata->resetPVValue = 0;
	cdata->forcePVName = "-";
	cdata->sevrPVName = "-";

	return(link);
}

/************************************************************
 this function set Mask from input string
************************************************************/
void alSetMask(char *s4,MASK *mask)
{
	char *s;
	size_t i;
	mask->AckT = 0;
	mask->Log = 0;
	mask->Ack = 0;
	mask->Disable = 0;
	mask->Cancel = 0;
	mask->Unused = 0;
	for (i=0;i<strlen(s4);i++) {
		s = s4 +i;
		if (*s == 'T') mask->AckT = 1;
		else if (*s == 'L') mask->Log = 1;
		else if (*s == 'D') mask->Disable = 1;
		else if (*s == 'C') mask->Cancel = 1;
		else if (*s == 'A') mask->Ack = 1;
		else continue;
	}
}

/***********************************************************
	 get mask string  for a know MASK mask
*************************************************************/
void alGetMaskString(MASK mask,char *s)
{
	strcpy(s,"-----");
	if (mask.Cancel == 1) *s = 'C';
	if (mask.Disable == 1) *(s+1) = 'D';
	if (mask.Ack == 1) *(s+2) = 'A';
	if (mask.AckT == 1) *(s+3) = 'T';
	if (mask.Log == 1) *(s+4) = 'L';
	*(s+5) = '\0';
}

/******************************************************
  alarmCountFilter_callback
******************************************************/

static void alarmCountFilter_callback(XtPointer cd, XtIntervalId *id)
{
	COUNTFILTER *countFilter=(COUNTFILTER *)cd;
	time_t alarmTime;
    int j;

#if DEBUG_CALLBACKS
	{
		static int n=0;

		printf("alarmCountFilter_callback: n=%d\n",n++);
	}
#endif
	alarmTime = countFilter->alarmTime;
	countFilter->alarmTime=0;
	countFilter->timeoutId=0;
    /* reset alarm count filter when new alarm is processed */
    if (countFilter->inputCount){
        for (j=0;j<=2*(countFilter->inputCount)-1;j++){countFilter->alarmTimeHistory[j]=0;}
    }
    countFilter->countIndex=0;
	alNewAlarmProcess(countFilter->stat,countFilter->sev,
	    countFilter->acks,countFilter->ackt,
	    countFilter->value,countFilter->clink,alarmTime);
}


/******************************************************
  alNewEvent
******************************************************/

void alNewEvent(int stat,int sevr,int acks,int acktCA,char *value,CLINK *clink)
{
	struct chanData *cdata;
	time_t alarmTime;
	unsigned ackt;

	if (clink == NULL ) return;
	cdata = clink->pchanData;
	if (cdata == NULL ) return;

	if (cdata == NULL ) return;

    if (acktCA == -1) ackt = cdata->curMask.AckT;
	/* NOTE: ackt and acktCA have opposite meaning */
	else ackt = (acktCA+1)%2;

	if (cdata->countFilter) {
		alNewAlarmFilter(stat,sevr,acks,ackt,value,clink);
	} else {
		/* set time of alarm */
		alarmTime = time(0L);
		alNewAlarmProcess(stat,sevr,acks,ackt,value,clink,alarmTime);
	}
}


/*********************************************************** 
     alNewAlarmFilter
************************************************************/
void alNewAlarmFilter(int stat,int sev,int acks,int ackt,char *value,CLINK *clink)
{
	struct chanData *cdata;
	int sevr_prev, i, j;
	time_t alarmTime;
	COUNTFILTER *countFilter;

	if (clink == NULL ) return;
	cdata = clink->pchanData;
	if (cdata == NULL ) return;

	/* set time of alarm */
	alarmTime = time(0L);

	countFilter = cdata->countFilter;
	sevr_prev = countFilter->sev;
	countFilter->stat = stat;
	countFilter->sev = sev;
	countFilter->acks = acks;
	countFilter->ackt = ackt;
	strcpy(countFilter->value,value);

 	/* Process the initial state */
	if (cdata->curStat==NO_ALARM && cdata->curSevr==ERROR_STATE ) {
		alNewAlarmProcess(stat,sev,acks,ackt,value,clink,alarmTime);
		return;
	}

 	/* Process if inputCount or inputSeconds is zero */
        if (countFilter->inputSeconds==0 || countFilter->inputCount==0) {
		alNewAlarmProcess(stat,sev,acks,ackt,value,clink,alarmTime);
		return;
	}

    /*remove timeout and call alNewAlarmProcess to update acks and ackt*/
    if (cdata->curSevr==0 && sev==0 ){
        if(acks==0) alNewAlarmProcess(stat,sev,acks,ackt,value,clink,alarmTime);
        if (countFilter->timeoutId){
            XtRemoveTimeOut(countFilter->timeoutId);
            countFilter->timeoutId=0;
        }
    }
    /* remove timeout and call alNewAlarmProcess */
    if (cdata->curSevr!=0 && sev!=0 ){
        alNewAlarmProcess(stat,sev,acks,ackt,value,clink,alarmTime);
        if (countFilter->timeoutId){
            XtRemoveTimeOut(countFilter->timeoutId);
            countFilter->timeoutId=0;
        }
    }
    /* if no timeout then add timeout */
    if ( cdata->curSevr==0 && sev!=0 ){
        if (countFilter->timeoutId==0) {
            countFilter->timeoutId = XtAppAddTimeOut(appContext,
                (unsigned long)countFilter->inputSeconds*1000,
                alarmCountFilter_callback,(XtPointer)countFilter);
            countFilter->alarmTime=alarmTime;
        }
    }


    /* process changes in acks and ackt and if no timeout then add timeout */
    if ( cdata->curSevr!=0 && sev==0 ){
        alNewAlarmProcess(cdata->curStat,cdata->curSevr,acks,ackt,value,clink,alarmTime);
        if (countFilter->timeoutId==0) {
            countFilter->timeoutId = XtAppAddTimeOut(appContext,
                (unsigned long)countFilter->inputSeconds*1000,
                alarmCountFilter_callback,(XtPointer)countFilter);
            countFilter->alarmTime=alarmTime;
        }
    }

    /* check in/out alarm count against */
    if ( (sevr_prev==0 && sev!=0) || (sevr_prev!=0 && sev==0)){
        i = countFilter->countIndex;
         if ( !countFilter->alarmTimeHistory || 
             (countFilter->alarmTimeHistory[i] &&
             (((int)difftime(alarmTime,countFilter->alarmTimeHistory[i]))<=countFilter->inputSeconds))){
            /* reset alarm count filter when new alarm is processed */
            if (countFilter->inputCount){
                for (j=0;j<=2*(countFilter->inputCount)-1;j++){countFilter->alarmTimeHistory[j]=0;}
            }
            if (countFilter->timeoutId){
                XtRemoveTimeOut(countFilter->timeoutId);
                countFilter->timeoutId=0;
            }
            countFilter->countIndex=0;
            alNewAlarmProcess(stat,sev,acks,ackt,value,clink,alarmTime);
        } else {
            countFilter->alarmTimeHistory[i] = alarmTime;
            countFilter->countIndex++;
            if (countFilter->countIndex >= 2*(countFilter->inputCount) ){
                countFilter->countIndex = 0;
            }
        }
    }
}

/*********************************************************** 
     alNewAlarmProcess
************************************************************/
static void alNewAlarmProcess(int stat,int sev,int acks,int ackt,char *value,
CLINK *clink,time_t timeofday)
{
	struct chanData *cdata;
	struct groupData *gdata;
	GLINK *glink;
	MASK mask;
	int stat_prev,sevr_prev,sevrHold;
	int viewCount=0;
	int prevViewCount=0;

	if (clink == NULL ) return;
	cdata = clink->pchanData;
	if (cdata == NULL ) return;

	if (sev >= ALH_ALARM_NSEV) sev = ALH_ALARM_NSEV-1;
	if (stat >= ALH_ALARM_NSTATUS) stat = ALH_ALARM_NSTATUS-1;
	mask = cdata->curMask;
	strncpy(cdata->value,value,MAX_STRING_SIZE-1);
	psetup.newUnackBeepSevr=0;

	if (_global_flag) {
		if (cdata->unackSevr != acks )  {
			if ( cdata->curMask.Disable == 0 && cdata->curMask.Ack == 0 ) {
				alSetUnackSevChan(clink,acks);
			} else {
				cdata->unackSevr = acks;
			}
		}
		if (cdata->curMask.AckT != ackt) {
			alSetAckTChan(clink,ackt);
		}
		if (cdata->curStat == stat && cdata->curSevr == sev &&
			cdata->curMask.Log == 0) {
			alLogAlarm(&timeofday,cdata,stat,sev,acks,ackt);
		}
	}

	if (cdata->curStat == stat && cdata->curSevr == sev) return;

	prevViewCount = awViewViewCount((void *)clink);

	stat_prev = cdata->curStat;
	sevr_prev = cdata->curSevr;

	cdata->curStat = stat;
	cdata->curSevr = sev;

	viewCount = awViewViewCount((void *)clink);
	clink->viewCount = viewCount;

 	/*
 	 * log the channel alarm at the alarm logfile
 	 */
 	if (mask.Log == 0) {
 	 	/* Don't log the initial connection */
 		if ( !(stat_prev==NO_ALARM && sevr_prev==ERROR_STATE )) {
			alLogAlarm(&timeofday,cdata,stat,sev,acks,ackt);
		}
	}

	/* 
	 * disabled alarm special handling 
	 */
	if (mask.Disable == 1) return;

	/*
	 * update current alarm history strings
	 */
	updateCurrentAlarmString(clink->pmainGroup->area,
		&timeofday,cdata->name,cdata->value, stat,sev);

	/*
	 *  set modification indicator for channel
	 */
	clink->modified = 1;
	clink->pmainGroup->modified = TRUE;

	/*
	 * spawn SEVRCOMMAND for the channel
	 * write the severity value to sevrPV channels
	 */

	if ( sev != sevr_prev ) {
		spawnSevrCommandList(&cdata->sevrCommandList,sev,sevr_prev);
		if (_global_flag && !_passive_flag) {
			if (cdata->sevrchid) alCaPutSevrValue(cdata->sevrchid,&cdata->curSevr);
		}
	}

	/*
	 * spawn STATCOMMAND for the channel
	 */

	if ( stat != stat_prev )
		spawnStatCommandList(&cdata->statCommandList,stat,stat_prev);

	/*
	 * spawn SEVRCOMMAND for all the parent groups
	 * update curSev[] of all the parent groups
	 * write the severity value to sevrPV channels
	 */
	glink = clink->parent;
	while (glink) {
		gdata = glink->pgroupData;
		gdata->curSev[sevr_prev]--;
		gdata->curSev[sev]++;
		sevrHold=gdata->curSevr;
		gdata->curSevr=alHighestSeverity(gdata->curSev);

		if ( sevrHold != gdata->curSevr ) {
			spawnSevrCommandList(&gdata->sevrCommandList,
			    gdata->curSevr,sevrHold);
			if (_global_flag && !_passive_flag) {
				if (gdata->sevrchid) alCaPutSevrValue(gdata->sevrchid,&gdata->curSevr);
			}
		}

		glink->modified = 1;
		glink = glink->parent;
	}

	/*
	 *  mark subWindows for add of a new line
	 */
	awViewAddNewAlarm(clink,prevViewCount,viewCount);

	/*
	 * alarm not required to acknowledge
	 */
	if (mask.Ack == 1) return;

	/*
	 * update unackSev[] and unackSevr of all parent groups
 	 * transient alarm not required to acknowledge
	 */
	if (!_global_flag) {
		if (sev >= cdata->unackSevr) {
			alSetUnackSevChan(clink,sev);
		} else {
			if (mask.AckT==1) {
				alSetUnackSevChan(clink,sev);
			}
		}
	}

	/*
	 * reset silenceCurrent state to FALSE
 	 */
	if (psetup.silenceCurrent && (psetup.newUnackBeepSevr>=psetup.beepSevr) ) 
		silenceCurrentReset(clink->pmainGroup->area);

}

/******************************************************************
	highest system severity used for  icon
*****************************************************************/
void alHighestSystemSeverity(GLINK *glink)
{
	psetup.highestSevr = alHighestSeverity(glink->pgroupData->curSev);
	psetup.highestUnackSevr = alHighestSeverity(glink->pgroupData->unackSev);
	psetup.highestUnackBeepSevr = alHighestBeepSeverity(glink->pgroupData->unackBeepSev,glink->pgroupData->beepSevr);
}

/******************************************************************
	highest severity 
*****************************************************************/
short alHighestSeverity(int sevr[ALH_ALARM_NSEV])
{
	short j=0;
	for (j=ALH_ALARM_NSEV-1;j>0;j--) {
		if (sevr[j] > 0) return(j);
	}
	return(0);
}

/******************************************************************
	highest beep severity 
*****************************************************************/
short alHighestBeepSeverity(int sevr[ALH_ALARM_NSEV], int beepSevr)
{
	short j=0;
	if (beepSevr == 0) beepSevr=1;
	for (j=ALH_ALARM_NSEV-1;j>=beepSevr;j--) {
		if (sevr[j] > 0) return(j);
	}
	return(0);
}

/*************************************************************    
 *This function forces the mask  in the channel according to the 
 *maskIndex and opCode a user selected from the mask menus.
 ***************************************************************/
void alForceChanMask(CLINK *clink,int index,int op)
{
	MASK mask;

	mask = clink->pchanData->curMask;

	switch (index) {
	case ALARMCANCEL:
		if (op == MASK_OFF) mask.Cancel = 0;
		else if (op == MASK_ON) mask.Cancel = 1;
		else if (op == MASK_RESET)
			mask.Cancel = 
			    clink->pchanData->defaultMask.Cancel;
		break;

	case ALARMDISABLE:
		if (op == MASK_OFF) mask.Disable = 0;
		else if (op == MASK_ON) mask.Disable = 1;
		else if (op == MASK_RESET)
			mask.Disable = 
			    clink->pchanData->defaultMask.Disable;
		break;

	case ALARMACK:
		if (op == MASK_OFF) mask.Ack = 0;
		else if (op == MASK_ON) mask.Ack = 1;
		else if (op == MASK_RESET)
			mask.Ack = 
			    clink->pchanData->defaultMask.Ack;
		break;

	case ALARMACKT:
		if (op == MASK_OFF) mask.AckT = 0;
		else if (op == MASK_ON) mask.AckT = 1;
		else if (op == MASK_RESET)
			mask.AckT = 
			    clink->pchanData->defaultMask.AckT;
		break;

	case ALARMLOG:
		if (op == MASK_OFF) mask.Log = 0;
		else if (op == MASK_ON) mask.Log = 1;
		else if (op == MASK_RESET)
			mask.Log = 
			    clink->pchanData->defaultMask.Log;
		break;

	}

	alChangeChanMask(clink,mask);

	if(_DB_call_flag)  alLog2DBMask(clink->pchanData->name);

}

/************************************************************************* 
 *This function forces the mask for all channels in the group according to the 
 *maskIndex and opCode a user selected from the mask menus.
 **********************************************************************/
void alForceGroupMask(GLINK *glink,int index,int op)
{
	GLINK *group;
	CLINK *clink;
	SNODE *pt;

	/*
	 * for all channels in this group
	 */
	pt = sllFirst(&(glink->chanList));
	while (pt) {
		clink = (CLINK *)pt;
		alForceChanMask(clink,index,op);
		pt = sllNext(pt);
	}
	/*
	 * for all subgroups
	 */
	pt = sllFirst(&(glink->subGroupList));
	while (pt) {
		group = (GLINK *)pt;
		alForceGroupMask(group,index,op);
		pt = sllNext(pt);
	}
}

/************************************************************************** 
 * This function updates the parent group masks. 'index' gives the index of
 * group mask.  'op' specifies ON / OFF of new channel mask setting.    
 **************************************************************************/
static void alUpdateGroupMask(CLINK *clink,int index,int op)
{
	struct groupData *gdata;
	GLINK *parent;

	parent = clink->parent;

	switch (op) {

	case MASK_OFF:  /* turns off */
		while (parent) {
			gdata = parent->pgroupData;
			if (gdata->mask[index] > 0) gdata->mask[index]--;
			else 
				errMsg("Error:alUpdateGroupMask, mask[%d] < 1\n",index);
			parent->modified = 1;
			parent = parent->parent;
		}
		break;

	case MASK_ON:   /* turns on */
		while (parent) {
			gdata = parent->pgroupData;
			gdata->mask[index]++;
			parent->modified = 1;
			parent = parent->parent;
		}
		break;

	}

	clink->pmainGroup->modified = TRUE;

}

/**************************************************************************
 * This function checks each bit of the new mask  against to the current 
 * mask.  If the bit of mask to be changed then  call alUpdateGroupMask 
 * to update the parent group mask. Finally updates current channel mask.
 **************************************************************************/
void alChangeChanMask(CLINK *clink,MASK mask)
{
	struct chanData *cdata;
	struct groupData *gdata;
	GLINK *parent;
	int change=0,saveSevr;
	int sevrHold, unackSevrHold;
	char buff[80];

	cdata = clink->pchanData;

	/*
	 * check each bit of new mask with current mask
	 */

	if (mask.Cancel != cdata->curMask.Cancel) {

		alUpdateGroupMask(clink,ALARMCANCEL,mask.Cancel);
		cdata->curMask.Cancel = mask.Cancel;
		change =1;

		if (mask.Cancel == 1 ) {
			if (cdata->evid) alCaClearEvent(&cdata->evid);
			if (cdata->curMask.Disable  == 0) {
				if (cdata->unackSevr > NO_ALARM)
					alSetUnackSevChan(clink,NO_ALARM);
				if (cdata->curSevr > NO_ALARM)  {
					saveSevr = cdata->curSevr;
					cdata->curSevr = NO_ALARM;
					cdata->curStat = NO_ALARM;
					parent = clink->parent;
					while(parent) {
						gdata = (struct groupData *)(parent->pgroupData);
						gdata->curSev[saveSevr]--;
						gdata->curSev[cdata->curSevr]++;

						/*
						 * spawn SEVRCOMMAND for all the parent groups
						 * update curSev[] of all the parent groups
						 */
						sevrHold=gdata->curSevr;
						gdata->curSevr=alHighestSeverity(gdata->curSev);
						if ( sevrHold != gdata->curSevr ) {
							spawnSevrCommandList(&gdata->sevrCommandList,
							    gdata->curSevr,sevrHold);
						}

						parent->modified = 1;
						parent = parent->parent;
					}
				}
			}

		}

		if (mask.Cancel == 0 && cdata->evid == NULL) {
/*
			if (cdata->curMask.Disable  == 0) {
				if (cdata->unackSevr == NO_ALARM)
					alSetUnackSevChan(clink,ERROR_STATE);
				if (cdata->curSevr == NO_ALARM)  {
					cdata->curSevr = ERROR_STATE;
					cdata->curStat = NOT_CONNECTED;
					parent = clink->parent;
					while(parent) {
						gdata = (struct groupData *)(parent->pgroupData);
						gdata->curSev[cdata->curSevr]--;
						gdata->curSev[ERROR_STATE]++;
						parent->modified = 1;
						parent = parent->parent;
					}
				}
			}
*/
			if (cdata->curMask.Disable == 0 && cdata->curSevr > 0) {
				saveSevr = cdata->curSevr;
				cdata->curSevr = NO_ALARM;
				alNewAlarmProcess(cdata->curStat,saveSevr,cdata->unackSevr,
					cdata->curMask.AckT,cdata->value,clink,time(0L));
			}
			alCaAddEvent(cdata->chid,&cdata->evid,clink);
		}

	}

	if (mask.Disable != cdata->curMask.Disable) {
		alUpdateGroupMask(clink,ALARMDISABLE,mask.Disable);

		cdata->curMask.Disable = mask.Disable;

		change = 1;
		if (mask.Disable == 1 && mask.Cancel == 0) {
			if (cdata->unackSevr > 0)  {
				unackSevrHold = cdata->unackSevr;
				alSetUnackSevChan(clink,NO_ALARM);
				if (_global_flag) cdata->unackSevr = unackSevrHold;
			}

			if (cdata->curSevr > 0) {
				parent = clink->parent;
				while(parent) {
					gdata = (struct groupData *)(parent->pgroupData);
					gdata->curSev[cdata->curSevr]--;
					gdata->curSev[NO_ALARM]++;
					/* spawn SEVRCOMMAND for all the parent groups
					 * update curSev[] of all the parent groups
					 */
					sevrHold=gdata->curSevr;
					gdata->curSevr=alHighestSeverity(gdata->curSev);
					if ( sevrHold != gdata->curSevr ) {
						spawnSevrCommandList(&gdata->sevrCommandList,gdata->curSevr,sevrHold);
					}
					parent->modified = 1;
					parent = parent->parent;
				}
			}
		}

		if (mask.Disable == 0 && mask.Cancel == 0) {
			if (cdata->curSevr > 0) {
				sevrHold = cdata->curSevr;
				unackSevrHold = cdata->unackSevr;
				cdata->curSevr = NO_ALARM;
				cdata->unackSevr = NO_ALARM;
				alNewAlarmProcess(cdata->curStat,sevrHold,unackSevrHold,
					cdata->curMask.AckT,cdata->value,clink,time(0L));
			} else {
				if (cdata->unackSevr > 0 && _global_flag )  {
					ackChan(clink);
			  		sprintf(buff,"Auto ack of transient alarms on enable: %s\n", cdata->name);
					alLogOpMod(buff);
					cdata->unackSevr = NO_ALARM;
				}
			}
		}
	}

	if (mask.Ack != cdata->curMask.Ack) {
		alUpdateGroupMask(clink,ALARMACK,mask.Ack);

		cdata->curMask.Ack = mask.Ack;

		change = 1;
		if (mask.Ack == 1 ) {
			if (cdata->unackSevr > 0 ) {
				unackSevrHold = cdata->unackSevr;
				alSetUnackSevChan(clink,NO_ALARM);
				if (_global_flag) cdata->unackSevr = unackSevrHold;
			}
		}

		if (mask.Ack == 0 && mask.Cancel ==0 && mask.Disable == 0) {
			if (cdata->curSevr > 0 ) {
				/*
				 * update unackSev[] of all parent groups
				 */
				if (_global_flag){
					unackSevrHold = cdata->unackSevr;
					cdata->unackSevr = NO_ALARM;
					alSetUnackSevChan(clink,unackSevrHold);
				} else {
					alSetUnackSevChan(clink,cdata->curSevr);
				}
			    /*
     			* reset silenceCurrent state to FALSE
     			*/
    			if (psetup.silenceCurrent && 
					(psetup.newUnackBeepSevr>=psetup.beepSevr) )
        			silenceCurrentReset(clink->pmainGroup->area);
			} else {
				if (cdata->unackSevr > 0 && _global_flag )  {
					ackChan(clink);
			  		sprintf(buff,"Auto ack of transient alarms on Ack: %s\n", cdata->name);
					alLogOpMod(buff);
					cdata->unackSevr = NO_ALARM;
				}
			}
		}
	}

	if (mask.AckT != cdata->curMask.AckT) {
		alUpdateGroupMask(clink,ALARMACKT,mask.AckT);
		cdata->curMask.AckT = mask.AckT;
		change = 1;
		if (_global_flag && !_passive_flag) {
			/* NOTE: ackt and curMask.AckT have opposite meaning */
			short ackt = (mask.AckT+1)%2;
			alCaPutGblAckT(cdata->chid,&ackt);
		}
	}

	if (mask.Log != cdata->curMask.Log) {
		alUpdateGroupMask(clink,ALARMLOG,mask.Log);
		cdata->curMask.Log = mask.Log;
		change = 1;
	}


	/*
	 * set current mask to new mask
	 */

	if (change == 1) {
		clink->modified = 1;
		clink->pmainGroup->modified = TRUE;
	}



}

/*********************************************************************** 
 * This function changes the mask of all the channels in a group to new mask.
 ***********************************************************************/
void alChangeGroupMask(GLINK *glink,MASK mask)
{
	CLINK *clink;
	GLINK *subgroup;
	SNODE *pt;

	if (glink == NULL) return;
	/*
	 * change each channel's mask  to new mask
	 */

	pt = sllFirst(&glink->chanList);
	while (pt) {
		clink = (CLINK *)pt;
		alChangeChanMask(clink,mask);
		pt = sllNext(pt);
	}

	/*
	 * change each subgroup's mask to new mask
	 */

	pt = sllFirst(&glink->subGroupList);
	while (pt) {
		subgroup = (GLINK *)pt;
		alChangeGroupMask(subgroup,mask);
		pt = sllNext(pt);
	}

}

/*********************************************************************** 
 * This function changes the mask of all the channels in a group to new mask.
 ***********************************************************************/
void alResetGroupMask(GLINK *glink)
{
	CLINK *clink;
	GLINK *subgroup;
	SNODE *pt;
	MASK mask;

	if (glink == NULL) return;
	/*
	 * change each channel's mask  to the reset mask
	 */

	pt = sllFirst(&glink->chanList);
	while (pt) {
		clink = (CLINK *)pt;
		mask = clink->pchanData->defaultMask;
		alChangeChanMask(clink,mask);
		pt = sllNext(pt);
	}

	/*
	 * change each subgroup's mask to new mask
	 */

	pt = sllFirst(&glink->subGroupList);
	while (pt) {
		subgroup = (GLINK *)pt;
		alResetGroupMask(subgroup);
		pt = sllNext(pt);
	}

}

/***************************************************
  alAlarmGroupName
****************************************************/
char *alAlarmGroupName(GLINK *link)
{
	if (link){
		if (link->pgroupData->alias) return(link->pgroupData->alias);
		else return(link->pgroupData->name);
	}
	return(0);
}

/***************************************************
  alProcessExists
****************************************************/
int alProcessExists(GCLINK *link)
{
	if (link->pgcData->command)  return(TRUE);
	return(FALSE);
}

/***************************************************
  alSetUnackSevGroup
****************************************************/
static void alSetUnackSevGroup(GLINK *glink,int newSevr,int oldSevr)
{
	struct groupData * gdata;

	gdata = (struct groupData *)glink->pgroupData;
	gdata->unackSev[oldSevr]--;
	gdata->unackSev[newSevr]++;
	gdata->unackSevr = alHighestSeverity(gdata->unackSev);
	glink->modified = 1;
	if (glink->parent) alSetUnackSevGroup(glink->parent,newSevr,oldSevr);
}

/***************************************************
  alSetUnackBeepSevGroup
****************************************************/
static void alSetUnackBeepSevGroup(GLINK *glink,int newSevr,int oldSevr)
{
	struct groupData * gdata;
	int osev=0;
	int nsev=0;

	gdata = (struct groupData *)glink->pgroupData;
	gdata->unackBeepSev[oldSevr]--;
	gdata->unackBeepSev[newSevr]++;
	gdata->unackBeepSevr = alHighestBeepSeverity(gdata->unackBeepSev,gdata->beepSevr);
	if (oldSevr >= gdata->beepSevr) osev=oldSevr; 
	if (newSevr >= gdata->beepSevr) nsev=newSevr;
	if (glink->parent) alSetUnackBeepSevGroup(glink->parent,nsev,osev);
	else psetup.newUnackBeepSevr=nsev;
}

/***************************************************
  alSetUnackSevChan
****************************************************/
void alSetUnackSevChan(CLINK *clink,int newSevr)
{
	int oldSevr;
	int osev=0;
	int nsev=0;

	oldSevr = clink->pchanData->unackSevr;
	if (oldSevr == newSevr) return;
	clink->pchanData->unackSevr = newSevr;
	clink->modified = 1;
	clink->pmainGroup->modified = TRUE;
	clink->pchanData->unackBeepSevr = newSevr;
	if (clink->parent) alSetUnackSevGroup(clink->parent,newSevr,oldSevr);
	if (oldSevr >= clink->pchanData->beepSevr) osev=oldSevr; 
	if (newSevr >= clink->pchanData->beepSevr) nsev=newSevr;
	if (clink->parent) alSetUnackBeepSevGroup(clink->parent,nsev,osev);
}


/***************************************************
  alSetBeepSevrChan
****************************************************/
void alSetBeepSevrChan(CLINK *clink,int beepSevr)
{
	int oldBeepSevr;
	int sev=0;
	int osev=0;
	int nsev=0;

	oldBeepSevr = clink->pchanData->beepSevr;
	clink->pchanData->beepSevr = beepSevr;
	sev = clink->pchanData->unackSevr;
	if (sev >= oldBeepSevr) osev=sev; 
	if (sev >= beepSevr) nsev=sev;
	if (osev == nsev) return;
	clink->modified = 1;
	clink->pmainGroup->modified = TRUE;
	if (clink->parent) alSetUnackBeepSevGroup(clink->parent,nsev,osev);
}


/***************************************************
  alSetUnackBeepSevCountGroup
****************************************************/
static void alSetUnackBeepSevCountGroup(GLINK *glink,int newSevr,int oldSevr,int count)
{
	struct groupData * gdata;
	int osev=0;
	int nsev=0;
	int j;

	gdata = (struct groupData *)glink->pgroupData;
	gdata->unackBeepSev[oldSevr]=gdata->unackBeepSev[oldSevr] - count;
	gdata->unackBeepSev[newSevr]=gdata->unackBeepSev[newSevr] + count;

	gdata->unackBeepSevr = alHighestBeepSeverity(gdata->unackBeepSev,gdata->beepSevr);
	if (oldSevr >= gdata->beepSevr) osev=oldSevr; 
	if (newSevr >= gdata->beepSevr) nsev=newSevr;
	if (glink->parent) alSetUnackBeepSevCountGroup(glink->parent,nsev,osev,count);
	else psetup.newUnackBeepSevr=nsev;
}

/***************************************************
  alSetBeepSevrGroup
****************************************************/
void alSetBeepSevrGroup(GLINK *glink,int beepSevr)
{
	struct groupData * gdata;
	int oldBeepSevr;
	int i, count;
	int osev, nsev;
	int start, end;

	gdata = (struct groupData *)glink->pgroupData;
	oldBeepSevr = gdata->beepSevr;
	if (oldBeepSevr == beepSevr) return;
	gdata->beepSevr = beepSevr;
	gdata->unackBeepSevr = alHighestBeepSeverity(gdata->unackBeepSev,gdata->beepSevr);
	glink->modified = 1;
	glink->pmainGroup->modified = TRUE;
	if ( oldBeepSevr < beepSevr) { start=oldBeepSevr; end=beepSevr; }
	else { start=beepSevr; end=oldBeepSevr; }
	for (i=start; i<end; i++) {
		count = gdata->unackSev[i];
		osev=0;
		nsev=0;
		if (i >= oldBeepSevr) osev=i; 
		if (i >= beepSevr) nsev=i;
		if (glink->parent && osev != nsev ) alSetUnackBeepSevCountGroup(glink->parent,nsev,osev,count);
	}
}


/***************************************************
  set new channel unacknowledge transients setting
****************************************************/
void alSetAckTChan(CLINK *clink,int newAckT)
{
	struct chanData *cdata;
	int oldAckT;
 
	cdata = clink->pchanData;

	oldAckT = cdata->curMask.AckT;
	if (oldAckT == newAckT) return;

	if (newAckT == FALSE && (cdata->unackSevr > cdata->curSevr)) {
		if (!_global_flag ||
			( cdata->curMask.Disable == 0 && cdata->curMask.Ack == 0 ) ) {
		    alSetUnackSevChan(clink,cdata->curSevr);
		}
	}

	alUpdateGroupMask(clink,ALARMACKT,newAckT);
	cdata->curMask.AckT = newAckT;

	clink->modified = 1;
	clink->pmainGroup->modified = TRUE;
}



/**************************************************************************
 * This function checks each bit of the new mask  against to the current 
 * mask.  If the bit of mask to be changed then  call alUpdateGroupMask 
 * to update the parent group mask. Finally updates current channel mask.
 **************************************************************************/
void alSetCurChanMask(CLINK *clink,MASK mask)
{
	struct chanData *cdata;
	cdata = clink->pchanData;

	if (mask.Cancel != cdata->curMask.Cancel) {
		alUpdateGroupMask(clink,ALARMCANCEL,mask.Cancel);
		cdata->curMask.Cancel = mask.Cancel;
	}
	if (mask.Disable != cdata->curMask.Disable) {
		alUpdateGroupMask(clink,ALARMDISABLE,mask.Disable);
		cdata->curMask.Disable = mask.Disable;
	}
	if (mask.Ack != cdata->curMask.Ack) {
		alUpdateGroupMask(clink,ALARMACK,mask.Ack);
		cdata->curMask.Ack = mask.Ack;
	}
	if (mask.AckT != cdata->curMask.AckT) {
		alUpdateGroupMask(clink,ALARMACKT,mask.AckT);
		cdata->curMask.AckT = mask.AckT;
	}
	if (mask.Log != cdata->curMask.Log) {
		alUpdateGroupMask(clink,ALARMLOG,mask.Log);
		cdata->curMask.Log = mask.Log;
	}
}

/************************************************************************* 
 *Remove the noAck timer for a  channel 
 **********************************************************************/
void alRemoveNoAck1HrTimerChan(CLINK *clink)
{
	if (clink->pchanData->noAckTimerId){
		XtRemoveTimeOut(clink->pchanData->noAckTimerId);
		clink->pchanData->noAckTimerId = 0;
	}
}

/************************************************************************* 
 *Remove the noAck timer for group and group's channels and subgroups 
 **********************************************************************/
void alRemoveNoAck1HrTimerGroup(GLINK *glink)
{
	GLINK *group;
	CLINK *clink;
	SNODE *pt;

	if (glink->pgroupData->noAckTimerId){
		XtRemoveTimeOut(glink->pgroupData->noAckTimerId);
		glink->pgroupData->noAckTimerId = 0;
	}
	/*
	 * for all channels in this group
	 */
	pt = sllFirst(&(glink->chanList));
	while (pt) {
		clink = (CLINK *)pt;
		alRemoveNoAck1HrTimerChan(clink);
		pt = sllNext(pt);
	}
	/*
	 * for all subgroups
	 */
	pt = sllFirst(&(glink->subGroupList));
	while (pt) {
		group = (GLINK *)pt;
		alRemoveNoAck1HrTimerGroup(group);
		pt = sllNext(pt);
	}
}

