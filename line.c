static char *sccsId = "@(#)line.c	1.6\t9/14/93";

/* line.c */
/* 
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
 * .01  06-06-91        bkc     Modified group alarm summary: add VALID alarm,
 *                              remove INFO alarm
 * .02  07-16-92        jba     changed VALID_ALARM to INVALID_ALARM
 * .03  02-16-93        jba     Reorganized files for new user interface
 * .04  mm-dd-yy        iii     Comment
 *      ...
 */

/*
******************************************************************
	routines defined in line.c
******************************************************************
         Routines for alloc, init, and update of a displayed line
******************************************************************
-------------
|   PUBLIC  |
-------------
*
void awUpdateChanLine(chanLine)		Update chanline message
	struct chanLine *chanLine;

*
void awUpdateGroupLine(groupLine)	Update groupline message
	struct groupLine *groupLine;

*
struct groupLine *
awAllocGroupLine()			Allocate group line

*
struct chanLine *
awAllocChanLine()			Allocate channel line


*
void awGroupMessage(groupLine)		Prepare group line message
	struct groupLine *groupLine;

*
void awChanMessage(pchanLine)		prepare channel line message
	struct chanLine *pchanLine;

*
void initLine(line)                     Initializes all line fields
     struct anyLine *line;

*
void initializeLines(lines)             Initializes all subWindow lines
     SNODE *lines;

*
void awGetMaskString(mask,s)		Get group mask string
	short mask[ALARM_NMASK];
	char *s;

*************************************************************************
*/


#include <stdio.h>
#include <stdlib.h>

#include <alh.h>
#include <alLib.h>
#include <sllLib.h>
#include <axSubW.h>
#include <line.h>
#include <axArea.h>
#include <ax.h>

extern char *alarmSeverityString[];
extern char *alarmStatusString[];

static char buff[LINEMESSAGE_SIZE];

#ifndef INCalarmh
#  include <alarm.h>
#endif


/********************************************************************* 
 *
 *
 *		void awGetMaskString(mask,s)
 *		short mask[ALARM_NMASK];
 *		char *s;
 *
 *  This function returns a string with 5 characters which shows
 *  the group mask.  The input to this function is the group
 *  mask array.
 *
 *********************************************************************/
void awGetMaskString(mask,s)
short mask[ALARM_NMASK];
char *s;
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
 *
 *		struct chanLine *awAllocChanLine()
 *
 *  This function allocates space for new chanLine  
 *
 *********************************************************************/
struct chanLine *awAllocChanLine()
{
struct chanLine *pLine;
        pLine = (struct chanLine *)calloc(1,sizeof(struct chanLine));
        return(pLine);
}

/*********************************************************************
 *
 *		struct groupLine *awAllocGroupLine()
 *
 *  This function allocates space for new groupLine 
 *
 *********************************************************************/
struct groupLine *awAllocGroupLine()
{
struct groupLine *pLine;
        pLine = (struct groupLine *)calloc(1,sizeof(struct groupLine));
        return(pLine);
}

/********************************************************************* 
 *		awUpdateChanLine(chanLine)
 *		struct chanLine *chanLine;
 *
 *  This function updates the channel line mask , status & severity
 *		
 *		  -  update chanLine data
 *		        unackSevr
 *		        unackStat
 *		        curSevr
 *		        curStat
 *		        mask
 *		        message
 *
 *********************************************************************/
void awUpdateChanLine(chanLine)
struct chanLine *chanLine;
{
struct chanData *cdata;
CLINK *clink;
        clink = (CLINK *)chanLine->clink;
        if (!clink) return;
        cdata = clink->pchanData;
        chanLine->unackSevr = cdata->unackSevr;
        chanLine->curSevr = cdata->curSevr;
        chanLine->unackStat = cdata->unackStat;
        chanLine->curStat = cdata->curStat;
	if (cdata->curMask.Disable == MASK_ON)  
		chanLine->curSevr = 0;
        alGetMaskString(cdata->curMask,buff);
        sprintf(chanLine->mask,"<%s> ",buff);
        awChanMessage(chanLine);
}



/*************************************************************************** 
 *		awUpdateGroupine(groupLine)
 *		struct groupLine *groupLine;
 *
 *  This function updates the group line mask , status & severity, summary
 *
 *		  -  update groupLine data
 *		        unackSevr
 *		        mask
 *		        curSev
 *		        message
 *
 ***************************************************************************/
void awUpdateGroupLine(groupLine)
struct groupLine *groupLine;
{
struct groupData *gdata;
GLINK *glink;
int i;
        glink = (GLINK *)groupLine->glink;
        if (!glink) return;
        gdata = glink->pgroupData;
          awGetMaskString(gdata->mask,buff); 
          sprintf(groupLine->mask,"<%s> ",buff);
        for (i=0;i<ALARM_NSEV;i++) 
                groupLine->curSev[i] = gdata->curSev[i];
        groupLine->unackSevr = alHighestSeverity(gdata->unackSev);
        groupLine->curSevr = alHighestSeverity(gdata->curSev);
        awGroupMessage(groupLine);
}


/***************************************************
 prepare groupLine message from groupData
****************************************************/
void awGroupMessage(groupLine)
struct groupLine *groupLine;
{
struct groupData *pData;
GLINK *glink;
        glink = ((GLINK *)groupLine->glink);
        if (!glink) return;
        pData = glink->pgroupData;

        groupLine->unackSevr = 
                        alHighestSeverity(pData->unackSev);                     

          awGetMaskString(pData->mask,buff); 
          sprintf(groupLine->mask,"<%s> ",buff);

        
        strcpy(groupLine->message," ");
        if (groupLine->unackSevr == 0 && alHighestSeverity(pData->curSev) == 0)
                return;
/*        
#ifdef GTA
        sprintf(buff,"MAJOR=%d,MINOR=%d,NOALARM=%d",
                pData->curSev[MAJOR],pData->curSev[MINOR],
		pData->curSev[NO_ALARM]);
#else
	sprintf(buff,"COMM=%d,MAJOR=%d,MINOR=%d,INFO=%d,NOALARM=%d",
                pData->curSev[INVALID_ALARM],
		pData->curSev[MAJOR_ALARM],pData->curSev[MINOR_ALARM],
	 	pData->curSev[INFO_ALARM],pData->curSev[NO_ALARM]);
#endif
*/
	sprintf(buff,"(%d,%d,%d,%d)",
                pData->curSev[INVALID_ALARM],
		pData->curSev[MAJOR_ALARM],pData->curSev[MINOR_ALARM],
	  	pData->curSev[NO_ALARM]);

        strcpy(groupLine->message,buff);
        
}



/***************************************************
 prepare chanLine message from chanData
****************************************************/
void awChanMessage(pchanLine)
struct chanLine *pchanLine;
{
MASK mask;
struct chanData *pData;
CLINK *clink;

        clink = ((CLINK *)pchanLine->clink);
        if (!clink) return;
        pData = clink->pchanData;

        mask = pData->curMask;

        strcpy(pchanLine->message," ");
        if (mask.Disable ) return;

        if (pData->curSevr == 0 && pData->unackSevr == 0) return;

        sprintf(pchanLine->message,"<%s,%s>",
        alarmStatusString[pData->curStat],
        alarmSeverityString[pData->curSevr]);
                
        if (pData->unackSevr == 0) return;

                
        sprintf(buff,",<%s,%s>",
        alarmStatusString[pData->unackStat],
        alarmSeverityString[pData->unackSevr]);
        strcat(pchanLine->message,buff);
                
 
}

/***************************************************
  initLine
****************************************************/
 
void initLine(line)
     struct anyLine *line;
{
          line->link = NULL;
          line->cosCallback = NULL;
          line->linkType = 0;
          line->unackSevr = 0;
          line->curSevr = 0;
          line->pname = 0;
}


/***************************************************
  initializeLines
****************************************************/
 
void initializeLines(lines)
     SNODE *lines;
{
     struct anyLine *line;

     line = (struct anyLine *)sllFirst(lines);
     while (line){
          initLine(line);
          line = (struct anyLine *)sllNext(line);
     }
                   
}

