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
extern int ALARM_COUNTER;
extern struct setup psetup;

/* forward declarations */
static void alarmCountFilter_callback(XtPointer cd, XtIntervalId *id);
static void alNewAlarmProcess(int stat,int sev,char *value,
CLINK *clink,time_t timeofday);
void alSetAckTChan(CLINK *clink,int ackt);


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
		if (cdata->countFilter) free(cdata->countFilter);
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
	(short)              glink->mask[ALARM_NMASK];
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

#if 0

modify parents:    
	(short)              clink->mask[ALARM_NMASK];
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
	;


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

#if DEBUG_CALLBACKS
	{
		static int n=0;

		printf("alarmCountFilter_callback: n=%d\n",n++);
	}
#endif
	alarmTime = countFilter->alarmTime;
	countFilter->alarmTime=0;
	countFilter->timeoutId=0;
	countFilter->curCount=0;
	alNewAlarmProcess(countFilter->stat,countFilter->sev,
	    countFilter->value,countFilter->clink,alarmTime);
}


/******************************************************
  alNewEvent
******************************************************/

void alNewEvent(int stat,int sevr,int acks,int ackt,char *value,CLINK *clink)
{
	struct chanData *cdata;
	COUNTFILTER *countFilter;
	time_t alarmTime;
	unsigned newAckt;

	if (clink == NULL ) return;
	cdata = clink->pchanData;
	if (cdata == NULL ) return;

	if (_global_flag) {
		/* NOTE: ackt and curMask.AckT have opposite meaning */
		newAckt = (ackt+1)%2; 
		if (cdata->unackSevr != acks && cdata->curMask.Disable == 0 &&
			cdata->curMask.Ack != 1 ) {
			alSetUnackSevChan(clink,acks);
		}
		if (cdata->curMask.AckT != newAckt) {
			alSetAckTChan(clink,newAckt);
		}
		if (cdata->curStat == stat && cdata->curSevr == sevr &&
			cdata->curMask.Log == 0) {
			alarmTime = time(0L);
			alLogAlarm(&alarmTime,cdata,stat,sevr,acks,newAckt);
		}
	}
	countFilter = cdata->countFilter;
	if ((!countFilter && (cdata->curStat != stat || cdata->curSevr != sevr)) ||
	    (countFilter && (countFilter->stat != stat || countFilter->sev != sevr)))
	{
		alNewAlarm(stat,sevr,value,clink);
	}
}

/*********************************************************** 
     alNewAlarm
************************************************************/
void alNewAlarm(int stat,int sev,char *value,CLINK *clink)
{
	struct chanData *cdata;
	int sevr_prev;
	time_t alarmTime,alarmTime_prev;
	COUNTFILTER *countFilter;

	if (clink == NULL ) return;
	cdata = clink->pchanData;
	if (cdata == NULL ) return;
	countFilter = cdata->countFilter;

	/* set time of alarm */
	alarmTime = time(0L);
	if (!countFilter) {
		alNewAlarmProcess(stat,sev,value,clink,alarmTime);
		return;
	}

	sevr_prev = countFilter->sev;
	countFilter->stat = stat;
	countFilter->sev = sev;
	strcpy(countFilter->value,value);

	if (sevr_prev==sev){ /* only stat has changed */
		alNewAlarmProcess(stat,sev,value,clink,alarmTime);
		return;
	}

 	/* Process the initial connection */
	if (cdata->curStat==NO_ALARM && cdata->curSevr==ERROR_STATE ) {
		alNewAlarmProcess(stat,sev,value,clink,alarmTime);
		return;
	}

	if ((cdata->curSevr==0 && sevr_prev==0) || (cdata->curSevr!=0 && sev==0)){
		alarmTime_prev=countFilter->alarmTime;
		if (!alarmTime_prev || ((alarmTime-alarmTime_prev)>countFilter->inputSeconds)){
			if ( countFilter->timeoutId == 0 ) {
				countFilter->timeoutId = XtAppAddTimeOut(appContext,
				    (unsigned long)countFilter->inputSeconds*1000,
				    alarmCountFilter_callback,(XtPointer)countFilter);
			}
			countFilter->curCount=1;
			countFilter->alarmTime=alarmTime;
		} else {
			countFilter->curCount++;
			if (countFilter->curCount >= countFilter->inputCount) {
				if (countFilter->timeoutId)
					XtRemoveTimeOut(countFilter->timeoutId);
				countFilter->timeoutId=0;
				countFilter->curCount=0;
				countFilter->alarmTime=0;
				alNewAlarmProcess(stat,sev,value,clink,alarmTime);
			}
		}
	} else if ((cdata->curSevr==0 && sev==0) || (cdata->curSevr!=0 && sevr_prev==0)){
		if (countFilter->timeoutId){
			XtRemoveTimeOut(countFilter->timeoutId);
		}
		countFilter->timeoutId=0;
		if (cdata->curSevr) alNewAlarmProcess(stat,sev,value,clink,alarmTime);

	} else { /* change in alarm state */
		if (cdata->curSevr) alNewAlarmProcess(stat,sev,value,clink,alarmTime);
	}
}

/*********************************************************** 
     alNewAlarmProcess
************************************************************/
static void alNewAlarmProcess(int stat,int sev,char *value,
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
	if (sev >= ALH_ALARM_NSEV) sev = ALH_ALARM_NSEV-1;
	if (stat >= ALH_ALARM_NSTATUS) stat = ALH_ALARM_NSTATUS-1;
	cdata = clink->pchanData;
	mask = cdata->curMask;

	prevViewCount = awViewViewCount((void *)clink);

	stat_prev = cdata->curStat;
	sevr_prev = cdata->curSevr;

	cdata->curStat = stat;
	cdata->curSevr = sev;
	strncpy(cdata->value,value,MAX_STRING_SIZE-1);

	viewCount = awViewViewCount((void *)clink);
	clink->viewCount = viewCount;

 	/*
 	 * log the channel alarm at the alarm logfile
 	 */
 	if (mask.Log == 0) {
 	 	/* Don't log the initial connection */
 		if ( !(stat_prev==NO_ALARM && sevr_prev==ERROR_STATE )) {
			alLogAlarm(&timeofday,cdata,stat,sev,
				cdata->unackSevr,cdata->curMask.AckT);
		}
	}

	if (DEBUG >=3) ALARM_COUNTER++;

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
	if (psetup.silenceCurrent && sev >= psetup.beepSevr)
		silenceCurrentReset(clink->pmainGroup->area);

	cdata->unackStat = stat;
}

/******************************************************************
	highest system severity used for  icon
*****************************************************************/
void alHighestSystemSeverity(GLINK *glink)
{
	int unack,curSevr;

	psetup.highestSevr = 0;
	psetup.highestUnackSevr = 0;
	curSevr = alHighestSeverity(glink->pgroupData->curSev);
	unack = alHighestSeverity(glink->pgroupData->unackSev);
	if (curSevr > psetup.highestSevr) psetup.highestSevr = curSevr;
	if (unack > psetup.highestUnackSevr)
		psetup.highestUnackSevr = unack;
}

/******************************************************************
	highest group severity 
*****************************************************************/
int alHighestSeverity(short sevr[ALH_ALARM_NSEV])
{
	int j=0;
	for (j=ALH_ALARM_NSEV-1;j>0;j--) {
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
	int sevrHold;

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
/*
					cdata->curSevr = NO_ALARM;
					cdata->curStat = NO_ALARM;
*/
					parent = clink->parent;
					while(parent) {
						gdata = (struct groupData *)(parent->pgroupData);
						gdata->curSev[cdata->curSevr]--;
						gdata->curSev[NO_ALARM]++;

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
				alNewAlarmProcess(cdata->curStat,saveSevr,cdata->value,clink,time(0L));
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
				alSetUnackSevChan(clink,NO_ALARM);
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
				saveSevr = cdata->curSevr;
				cdata->curSevr = NO_ALARM;
				alNewAlarmProcess(cdata->curStat,saveSevr,cdata->value,clink,time(0L));
			}
		}

	}

	if (mask.Ack != cdata->curMask.Ack) {
		alUpdateGroupMask(clink,ALARMACK,mask.Ack);

		cdata->curMask.Ack = mask.Ack;

		change = 1;
		if (mask.Ack == 1 ) {
			if (cdata->unackSevr > 0 ) {
				alSetUnackSevChan(clink,NO_ALARM);
			}
		}

		if (mask.Ack == 0 && mask.Cancel ==0 && mask.Disable == 0) {
			if (cdata->curSevr > 0 ) {
				/*
				 * update unackSev[] of all parent groups
				 */
				parent = clink->parent;
				while(parent) {
					gdata = (struct groupData *)(parent->pgroupData);
					gdata->curSev[cdata->curSevr]--;
					gdata->curSev[0]++;
					parent = parent->parent;
				}
				saveSevr = cdata->curSevr;
				cdata->curSevr = 0;
				alNewAlarmProcess(cdata->curStat,saveSevr,cdata->value,clink,time(0L));
			}


		}

	}
	if (mask.AckT != cdata->curMask.AckT) {
		alUpdateGroupMask(clink,ALARMACKT,mask.AckT);
		cdata->curMask.AckT = mask.AckT;
		change = 1;
		if (_global_flag) {
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
  alSetUnackSev
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
  alSetUnackSev
****************************************************/
void alSetUnackSevChan(CLINK *clink,int newSevr)
{
	int oldSevr;

	oldSevr = clink->pchanData->unackSevr;
	if (oldSevr == newSevr) return;
	clink->pchanData->unackSevr = newSevr;
	clink->modified = 1;
	clink->pmainGroup->modified = TRUE;
	if (clink->parent) alSetUnackSevGroup(clink->parent,newSevr,oldSevr);
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
	    alSetUnackSevChan(clink,cdata->curSevr);
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
