/* $Id$ */

/******************************************************************
  Routines for alloc, init, and update of a displayed line
******************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <alarm.h>

#include "alh.h"
#include "alLib.h"
#include "sllLib.h"
#include "axSubW.h"
#include "line.h"
#include "axArea.h"
#include "ax.h"

/* global variables */
extern char * alarmSeverityString[];
extern char * alarmStatusString[];

static char buff[LINEMESSAGE_SIZE];


/********************************************************************* 
 *  This function returns a string with 5 characters which shows
 *  the group mask.  The input to this function is the group
 *  mask array.
 *********************************************************************/
void awGetMaskString(short mask[ALARM_NMASK],char *s)
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
 *  This function allocates space for new chanLine  
 *********************************************************************/
struct chanLine *awAllocChanLine()
{
struct chanLine *pLine;
        pLine = (struct chanLine *)calloc(1,sizeof(struct chanLine));
        return(pLine);
}

/*********************************************************************
 *  This function allocates space for new groupLine 
 *********************************************************************/
struct groupLine *awAllocGroupLine()
{
        struct groupLine *pLine;
        pLine = (struct groupLine *)calloc(1,sizeof(struct groupLine));
        return(pLine);
}

/********************************************************************* 
 *  This function updates the channel line mask , status & severity
 *********************************************************************/
void awUpdateChanLine(struct chanLine *chanLine)
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
        sprintf(chanLine->mask,"<%s>",buff);
        awChanMessage(chanLine);
}

/*************************************************************************** 
 *  This function updates the group line mask , status & severity, summary
 ***************************************************************************/
void awUpdateGroupLine(struct groupLine *groupLine)
{
        struct groupData *gdata;
        GLINK *glink;
        int i;

        glink = (GLINK *)groupLine->glink;
        if (!glink) return;
        gdata = glink->pgroupData;
          awGetMaskString(gdata->mask,buff); 
          sprintf(groupLine->mask,"<%s>",buff);
        for (i=0;i<ALARM_NSEV;i++) 
                groupLine->curSev[i] = gdata->curSev[i];
        groupLine->unackSevr = alHighestSeverity(gdata->unackSev);
        groupLine->curSevr = alHighestSeverity(gdata->curSev);
        awGroupMessage(groupLine);
}


/***************************************************
 prepare groupLine message from groupData
****************************************************/
void awGroupMessage(struct groupLine *groupLine)
{
        struct groupData *pData;
        GLINK *glink;

        glink = ((GLINK *)groupLine->glink);
        if (!glink) return;
        pData = glink->pgroupData;

        groupLine->unackSevr = 
                        alHighestSeverity(pData->unackSev);                     

          awGetMaskString(pData->mask,buff); 
          sprintf(groupLine->mask,"<%s>",buff);

        
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
void awChanMessage(struct chanLine *pchanLine)
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

