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
/* line.c */

/************************DESCRIPTION***********************************
  Routines for alloc, init, and update of a displayed line
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "alarm.h"

#include "alh.h"
#include "alLib.h"
#include "sllLib.h"
#include "axSubW.h"
#include "line.h"
#include "axArea.h"
#include "ax.h"

/* global variables */
extern char * alhAlarmSeverityString[];
extern char * alhAlarmStatusString[];
extern char *bg_char[];

static char buff[LINEMESSAGE_SIZE];

/********************************************************************* 
 *  This function returns a string with 5 characters which shows
 *  the group mask.  The input to this function is the group
 *  mask array.
 *********************************************************************/
void awGetMaskString(int mask[ALARM_NMASK],char *s)
{
	strcpy(s,"-----");
	if (mask[ALARMCANCEL] > 0) *s = 'C';
	if (mask[ALARMDISABLE] > 0) *(s+1) = 'D';
	if (mask[ALARMACK] > 0) *(s+2) = 'A';
	if (mask[ALARMACKT] > 0) *(s+3) = 'T';
	if (mask[ALARMLOG] > 0) *(s+4) = 'L';
	*(s+5) = '\0';
}

/*********************************************************************
 *  This function allocates space for new line  
 *********************************************************************/
struct anyLine *awAllocLine()
{
	struct anyLine *pLine;
	pLine = (struct anyLine *)calloc(1,sizeof(struct anyLine));
	return(pLine);
}

/********************************************************************* 
 *  This function updates the channel line mask , status & severity
 *********************************************************************/
void awUpdateChanLine(struct anyLine *chanLine)
{
	struct chanData *cdata;
	CLINK *clink;

	clink = (CLINK *)chanLine->link;
	if (!clink) return;
	cdata = clink->pchanData;
	chanLine->unackSevr = cdata->unackSevr;
	chanLine->curSevr = cdata->curSevr;
	chanLine->curStat = cdata->curStat;

	if (cdata->curMask.Disable == MASK_ON ||
		 cdata->curMask.Cancel == MASK_ON) {
		chanLine->curSevr = 0;
		chanLine->unackSevr = 0;
	}
	if (cdata->curMask.Ack == MASK_ON) {
		chanLine->unackSevr = 0;
	}
	alGetMaskString(cdata->curMask,buff);
	if (cdata->noAckTimerId ) buff[2]='H';
	sprintf(chanLine->mask,"<%s>",buff);

	strcpy(chanLine->message," ");

	if (!cdata->curMask.Disable && !cdata->curMask.Cancel &&
		(cdata->curSevr || cdata->unackSevr)){

		sprintf(chanLine->message,"<%s,%s>",
	    	alhAlarmStatusString[cdata->curStat],
	    	alhAlarmSeverityString[cdata->curSevr]);

		if (cdata->unackSevr){
			sprintf(buff,",<%s>",
				alhAlarmSeverityString[cdata->unackSevr]);
			strcat(chanLine->message,buff);
		}
	}
	if(cdata->highestBeepSevr>1) strcpy(chanLine->highestBeepSevrString,
			bg_char[cdata->highestBeepSevr]);
	else strcpy(chanLine->highestBeepSevrString," ");
}

/*************************************************************************** 
 *  This function updates the group line mask , status & severity, summary
 ***************************************************************************/
void awUpdateGroupLine(struct anyLine *groupLine)
{
	struct groupData *gdata;
	GLINK *glink;
	int i;

	glink = (GLINK *)groupLine->link;
	if (!glink) return;
	gdata = glink->pgroupData;
	awGetMaskString(gdata->mask,buff);
	if (gdata->noAckTimerId ) buff[2]='H';
	sprintf(groupLine->mask,"<%s>",buff);
	for (i=0;i<ALH_ALARM_NSEV;i++){
		groupLine->curSev[i] = gdata->curSev[i];
	}
	groupLine->unackSevr = alHighestSeverity(gdata->unackSev);
	groupLine->curSevr = alHighestSeverity(gdata->curSev);

	strcpy(groupLine->message," ");

	if (groupLine->unackSevr || alHighestSeverity(gdata->curSev)){
		sprintf(groupLine->message,"(%d,%d,%d,%d,%d)",
	    	gdata->curSev[ERROR_STATE],
	    	gdata->curSev[INVALID_ALARM],
	    	gdata->curSev[MAJOR_ALARM],
	    	gdata->curSev[MINOR_ALARM],
	    	gdata->curSev[NO_ALARM]);
	}
	if(gdata->highestBeepSevr>1) strcpy(groupLine->highestBeepSevrString,
			bg_char[gdata->highestBeepSevr]);
	else strcpy(groupLine->highestBeepSevrString," ");
}

/***************************************************
  Initializes all line fields
****************************************************/
void initLine(struct anyLine *line)
{
	line->link = NULL;
	line->cosCallback = NULL;
	line->linkType = 0;
	line->unackSevr = 0;
	line->curSevr = 0;
	line->pname = 0;
	line->alias = 0;
}

/***************************************************
 Initializes all subWindow lines
****************************************************/
void initializeLines(SNODE *lines)
{
	struct anyLine *line;

	line = (struct anyLine *)sllFirst(lines);
	while (line){
		initLine(line);
		line = (struct anyLine *)sllNext(line);
	}

}

