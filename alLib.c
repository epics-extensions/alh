/*
 $Log$
 Revision 1.20  1998/07/07 20:49:02  jba
 Removed unused variable timeout.

 Revision 1.19  1998/06/02 19:40:45  evans
 Changed from using Fgmgr to using X to manage events and file
 descriptors.  (Fdmgr didn't work on WIN32.)  Uses XtAppMainLoop,
 XtAppAddInput, and XtAppAddTimeOut instead of Fdmgr routines.
 Updating areas is now in alCaUpdate, which is called every caDelay ms
 (currently 100 ms).  Added a general error message routine (errMsg)
 and an exception handler (alCAException).  Is working on Solaris and
 WIN32.

 Revision 1.18  1998/06/01 18:33:23  evans
 Modified the icon.

 Revision 1.17  1998/05/12 18:22:40  evans
 Initial changes for WIN32.

 Revision 1.16  1997/10/27 17:28:00  jba
 Moved write of sevr to sevrPVs and now write only when changed.

 Revision 1.15  1997/09/12 19:29:15  jba
 Removed unnecessary link tests.

 Revision 1.14  1997/03/31 16:45:16  jba
 Bug fix to SEVRCOMMAND for UP_ANY and DOWN_ANY.
 Bug fix to ALARMCOUNTFILTER.

 Revision 1.13  1997/01/16 16:34:59  jba
 Bug fix for ALARMCOUNTFILTER.

 Revision 1.12  1996/06/07 15:44:59  jba
 Added global alarm acknowledgement.

 Revision 1.11  1996/03/25 15:45:31  jba
 Bug fix for spawn of SEVRCOMMAND for channel and all parent groups

 * Revision 1.10  1995/11/13  22:31:13  jba
 * Added beepseverity command, ansi changes and other changes.
 *
 * Revision 1.9  1995/10/20  16:49:54  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 *
 * Revision 1.8  1995/06/22  19:48:46  jba
 * Added $ALIAS facility.
 *
 * Revision 1.7  1995/06/01  15:15:18  jba
 * Removed some comments
 *
 * Revision 1.6  1995/05/31  20:50:37  jba
 * Added test for change in severity before spawn of ALARMCOMMAND
 *
 * Revision 1.5  1995/05/30  15:55:10  jba
 * Added ALARMCOMMAND facility
 *
 * Revision 1.4  1995/03/24  16:35:46  jba
 * Bug fix and reorganized some files
 *
 * Revision 1.3  1995/02/28  16:43:32  jba
 * ansi c changes
 *
 * Revision 1.2  1994/06/22  21:16:35  jba
 * Added cvs Log keyword
 *
 */

#define DEBUG_CALLBACKS 0

static char *sccsId = "@(#)alLib.c	1.14\t10/15/93";

/*  @(#)alLib.c @(#)alLib.c	1.14  10/15/93
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
 * .01  06-12-91        bkc     Add the option of updating values of sevrPVName 
 *				channels due to new alarm
 * .02  02-16-93        jba     Added routines for new user interface
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */
/* alLib.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <sllLib.h>
#include <alLib.h>
#include <ax.h>


/* global variables */
extern int DEBUG;
extern int ALARM_COUNTER;
extern struct setup psetup;

/* function prototypes */
static void alarmCountFilter_callback(XtPointer cd, XtIntervalId *id);

/* external routines
extern   alCaAddEvent();
extern void updateLog();
extern processSpawn_callback();
*/

#ifdef __STDC__
     static void alNewAlarmProcess(int stat,int sev,char value[],
     CLINK *clink,time_t timeofday);
#else
     static void alNewAlarmProcess();
#endif /*__STDC__*/

/*
**************************************************************
	routines defined in alLib.c
*************************************************************
*
*-----------------------------------------------------------------
	routines for primative group & channel operation
*-----------------------------------------------------------------
*struct mainGroup *alAllocMainGroup()	Allocate space for main group
	return:  pointer of mainGroup
*
GLINK *alAllocGroup()			Allocate space for group link 
	return:  pointer of GLINK	and group data
*
CLINK *alAllocChan()			Allocate space for channel link
	return:  pointer of CLINK	and channel data
*
void alAddGroup(parent,glink)		Append a group link to a parent's 
	GLINK *parent;			subgroup list 
	GLINK *glink;
*
void alAddChan(parent,clink)     	Append a channel link to a parent's 
	GLINK *parent;			channel list
	CLINK *clink;
*
void alDeleteChan(clink)			Delete a clink from channel list
	CLINK *clink;
*
void alDeleteGroup(glink)		Delete a glink and all associated 
	GLINK *glink;			subgroups and channels 
*
CLINK *alFindChannel(pgroup,channame)   Find the channel name from a group
	char *channame;			link list 
	SLIST *pgroup;
	return:  pointer of CLINK	if found
		 NULL			if not found
*
GLINK *alFindGroup(pgroup,groupname)	Find the group name from a group
	char *groupname;		link list 
	SLIST *pgroup;
	return:  pointer of GLINK	if found
		 NULL			if not found
*
void alInsertChan(sibling,clink)  	Insert the channel link after a
	CLINK *sibling;			sibling
	CLINK *clink;
*
void alInsertGroup(sibling,glink) 	Insert the group link after a 
	GLINK *sibling;			sibling
	GLINK *glink;
*
void alMoveGroup(sibling,glink)  	Move the group link after a 
	GLINK *sigling;			sibling
	GLINK *glink;
*
void alRemoveGroup(glink)		Remove the group link from its
	GLINK *glink;			parent
*
void alRemoveChan(clink)			Remove the channel link from it's
	CLINK *clink;			parent
*
CLINK *alCopyChan(clink)           Make a copy of a channel
    CLINK *clink;
*
GLINK *alCopyGroup(glink)          Make a copy of a group
    GLINK *glink;
*
GLINK *alCreateGroup()          Create a new group
*
CLINK *alCreateChannel()          Create a new channel
*
void alSetPmainGroup(glink, pmainGroup) Set pmainGroup in subgroups and channels
*

*-----------------------------------------------------------------
*    operation related routines after completion of configuration
*-----------------------------------------------------------------
*
void alSetMask(s4,mask)                  Set mask bit from string s4
        MASK *mask;                     return value stored in mask
        char *s4;
* 
void alGetMaskString(mask,s)		Get mask string from Mask bit setting
	MASK mask;
	char *s;
*
void alOrMask(m1,m2)			Or operation of two masks m1 & m2
	MASK *m1;
	MASK *m2;	store returned mask 
*
alNewAlarm(stat,sev,value,clink)	Update system data due to new alarm
	int stat;	
	int sev;	
	char value[MAX_STRING_SIZE];	
	CLINK *clink;
*
void alHighestSystemSeverity()		Set system highest alarm severity
*
int alHighestSeverity(sevr)		Set highest group alarm severity  
	short sevr[ALARM_NSEV];	
	return:  int
*
alAckChan(clink)			Acknowledge an alarmed channel and
	CLINK *clink;			update all affected parent groups
*
alAckGroup(glink)			Acknowledge a group 
	GLINK *glink;							
*
alForceChanMask(clink,index,op)		Force channel mask by bit selection
	int index;	BIT # of MASK
	int op;		MASK ON/OFF/RESET
	CLINK *clink;
*
alForceGroupMask(glink,index,op)	Force group mask by bit selection
	int index;	BIT # of MASK
	int op;		MASK ON/OFF/RESET
	GLINK *glink;
*
alUpdateGroupMask(clink,index,op)	Update all the parent group masks
	int index;	BIT # of MASK   
	int op;		MASK ON/OFF/RESET		
	CLINK *clink;
*
alChangeChanMask(clink,mask)		Change channel mask & adjust CA
	MASK mask;			
	CLINK *clink;
*
alChangeGroupMask(glink,mask)		Change group mask 
	MASK mask;			
	GLINK *glink;			
*
alForcePVChanEvent(clink,value)		Force / reset channel mask
	CLINK *clink;
	int value;
*
alForcePVGroupEvent(glink,value) 	Force / reset group mask
	GLINK *glink;
	int value;
*
*-----------------------------------------------------------------
*    new user interface routines
*    routines related to obtaining and reseting link information
*-----------------------------------------------------------------
* 
char *alAlarmGroupName(link)            Returns pointer to name
     GLINK *link;
*
int alGuidanceExists(link)              Returns True if guidance exists
     GCLINK *link;
*
int alProcessExists(link)               Returns True if related process
     GCLINK *link;                      exists
*
*/

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


/*************************************************************************
	 allocate space for grouplink 
*************************************************************************/
GLINK *alAllocGroup()
{
GLINK *glink;
                
        glink = (GLINK *)calloc(1,sizeof(GLINK));
/*      sllInit(&(glink->subGroupList));
        sllInit(&(glink->chanList)); 
        glink->parent = NULL;
*/
        glink->pgroupData = (struct groupData *)calloc(1,sizeof(struct groupData));
        return(glink);
}

/************************************************************************
	 allocate space for channel link 
************************************************************************/
CLINK *alAllocChan()
{
CLINK *clink;

        clink = (CLINK *)calloc(1,sizeof(CLINK));
        clink->pchanData = (struct chanData *)calloc(1,sizeof(struct chanData)); 
/*       clink->parent = NULL; */
        return(clink);
}

/**********************************************************************
	 append glink at the end of subgrouplist 
***********************************************************************/
void alAddGroup(parent,glink)
GLINK *parent,*glink;
{
        sllAdd(&(parent->subGroupList),(SNODE *)glink);
	glink->parent = parent;
}


/************************************************************************
	 append at  the end of Channel list 
*************************************************************************/
void alAddChan(parent,clink)     
GLINK *parent;
CLINK *clink;
{
        sllAdd(&(parent->chanList),(SNODE *)clink);
        clink->parent = parent;
}

/**********************************************************************
	 insert a glink before another glink in the subgrouplist 
***********************************************************************/
void alPrecedeGroup(parent,sibling,glink)
GLINK *parent;
GLINK *sibling,*glink;
{
	    if (parent == NULL || glink == NULL )  return;
        sllPrecede(&(parent->subGroupList),(SNODE *)sibling,(SNODE *)glink);
        glink->parent = parent;
}

/**********************************************************************
	 insert a chan before another chan in the subgrouplist 
***********************************************************************/
void alPrecedeChan(parent,sibling,clink)
GLINK *parent;
CLINK *sibling,*clink;
{

	    if (parent == NULL || clink == NULL )  return;
        sllPrecede(&(parent->chanList),(SNODE *)sibling,(SNODE *)clink);
        clink->parent = parent;
}

/************************************************************************
	 delete a channel link from chanList 
***********************************************************************/
void alDeleteChan(clink)
CLINK *clink;
{
    SNODE *pt,*next;
    struct guideLink *guidelist; 
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

        pt = sllFirst(&clink->GuideList);
        while (pt) {
              next = sllNext(pt);
              guidelist = (struct guideLink *)pt;
              free(guidelist->list);
              free(guidelist);
              pt = next;
        }
		free(cdata);
		free(clink);
	}
}


/*******************************************************************
   assume that glink had been removed by calling
        alRemoveGroup(glink) 
    before calling this function to free pointers
********************************************************************/
void alDeleteGroup(glink)
GLINK *glink;
{
SNODE *snode,*cnode,*gnode,*next;
GLINK *pt;
struct guideLink *guidelist; 
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

        snode = sllFirst(&glink->GuideList);
        while (snode) {
              next = sllNext(snode);
              guidelist = (struct guideLink *)snode;
              free(guidelist->list);
              free(guidelist);
              snode = next;
        }
        free(gdata);
        free(glink);
	}
}



/****************************************************************
	find the channelname in a given group
**************************************************************/
CLINK *alFindChannel(pgroup,channame)
SLIST *pgroup;
char *channame;
{
SNODE *temp,*pt;
GLINK *glink;
CLINK *clink;
        temp = sllFirst(pgroup);
        while (temp) {
       	 	glink = (GLINK *)temp;
                pt = sllFirst(&(glink->chanList));
                while (pt) {
                	clink = (CLINK *)pt;
               	 	if (strcmp(clink->pchanData->name,channame)==0){
                        return(clink);
                        /* break; */}
                	pt = sllNext(pt);
                }
        clink = alFindChannel(&(glink->subGroupList),channame);
        if (clink) return(clink);
        temp = sllNext(temp);
        }
        return(NULL);
}


/****************************************************************
	find the groupname in a given group
**************************************************************/
GLINK *alFindGroup(pgroup,groupname)
SLIST *pgroup;
char *groupname;
{
SNODE *temp;
GLINK *glink,*sublink;
        temp = sllFirst(pgroup);
        while (temp ) {
        	glink = (GLINK *)temp;
        	if (strcmp(groupname,glink->pgroupData->name) == 0 ) 
                return(glink);
        	sublink = alFindGroup(&(glink->subGroupList),groupname);
        	if (sublink) return(sublink);
        	temp = sllNext(temp);      
        }
        return(NULL);
}

/**************************************************************
	 this function insert clink after sibling 
***************************************************************/
void alInsertChan(sibling,clink)  
CLINK *sibling,*clink;
{
        sllInsert(&(sibling->parent->chanList),(SNODE *)sibling,(SNODE *)clink);
        clink->parent = sibling->parent;
}


/**************************************************************
	 this function insert glink after sibling 
**************************************************************/
void alInsertGroup(sibling,glink) 
GLINK *sibling,*glink;
{
        sllInsert(&(sibling->parent->subGroupList),(SNODE *)sibling,(SNODE *)glink);
        glink->parent = sibling->parent;
}

/*************************************************************
	 move glink after sibling 
*************************************************************/
void alMoveGroup(sibling,glink)  
GLINK *sibling,*glink;
{
        alRemoveGroup(glink);
        alInsertGroup(sibling,glink);
}

/***************************************************************
	 remove glink from subGroupList 
***************************************************************/
void alRemoveGroup(glink)
GLINK *glink;
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
void alRemoveChan(clink)
CLINK *clink;
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
void alSetPmainGroup(glink, pmainGroup)
	GLINK *glink;
    struct mainGroup *pmainGroup;
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
GLINK *alCopyGroup(glink)
	GLINK *glink;
{
	CLINK *clink;
	GLINK *glinkNew;
	GLINK *glinkTemp;
	struct groupData *gdata;
	struct groupData *gdataNew;
	char *buff;
	struct guideLink *guideLink;
	SNODE *node;

#if 0

 modify parents:    (short)              glink->mask[ALARM_NMASK];
 modify parents:    ( int)               glink->viewCount;

 modify children:   (struct mainGroup *) glink->pmainGroup;
 modify children:   (void *)             glink->pgroupData->treeSym;

#endif

    if (!glink) return 0;

	glinkNew = alAllocGroup();
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
    gdataNew->PVValue = gdata->PVValue;;


	/* copy name */
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
	node = sllFirst(&(glink->GuideList));
	while (node) {
        buff = ((struct guideLink *)node)->list;
		guideLink = (struct guideLink *)calloc(1,sizeof(struct guideLink));
		guideLink->list = (char *)calloc(1,strlen(buff)+1);
		strcpy(guideLink->list,buff);
		sllAdd(&(glinkNew->GuideList),(SNODE *)guideLink);
		node = sllNext(node);
	}


	/* copy all channels */
	node = sllFirst(&(glink->chanList));
	while (node) {
		clink = alCopyChan((CLINK *)node);;
		alAddChan(glinkNew, clink);
		node = sllNext(node);
	}

	/* copy all subGroups */
	node = sllFirst(&(glink->subGroupList));
	while (node) {
		glinkTemp = alCopyGroup((GLINK *)node);;
		alAddGroup(glinkNew, glinkTemp);
		node = sllNext(node);
	}

	return(glinkNew);

}


/*******************************************************************
	make a copy of a Channel
*******************************************************************/
CLINK *alCopyChan(clink)
	CLINK *clink;
{
	CLINK *clinkNew;
	char *buff;
	struct chanData *cdataNew;
	struct chanData *cdata;
	struct guideLink *guideLink;
	SNODE *node;

#if 0

 modify parents:    (short)              clink->mask[ALARM_NMASK];
 modify parents:    ( int)               clink->viewCount;

 modify children:   (struct mainGroup *) clink->pmainGroup;
 modify children:   (void *)             clink->pchanData->treeSym;

#endif

    if (!clink) return 0;

	clinkNew = alAllocChan();
    clinkNew->pchanData = (struct chanData *)calloc(1,sizeof(struct chanData));
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
    cdataNew->PVValue = cdata->PVValue;;


	/* copy name */
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
	node = sllFirst(&(clink->GuideList));
	while (node) {
        buff = ((struct guideLink *)node)->list;
		guideLink = (struct guideLink *)calloc(1,sizeof(struct guideLink));
		guideLink->list = (char *)calloc(1,strlen(buff)+1);
		strcpy(guideLink->list,buff);
		sllAdd(&(clinkNew->GuideList),(SNODE *)guideLink);
		node = sllNext(node);
	}

	return(clinkNew);

}


/*******************************************************************
	Create a new Group
*******************************************************************/
GLINK *alCreateGroup()
{
     GLINK *link;
     struct groupData *gdata;

     link = alAllocGroup();
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

     link = alAllocChan();
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
void alSetMask(s4,mask)
MASK *mask;
char *s4;
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
void alGetMaskString(mask,s)
MASK mask;
char *s;
{
strcpy(s,"-----");
if (mask.Cancel == 1) *s = 'C';
if (mask.Disable == 1) *(s+1) = 'D';
if (mask.Ack == 1) *(s+2) = 'A';
if (mask.AckT == 1) *(s+3) = 'T';
if (mask.Log == 1) *(s+4) = 'L';
*(s+5) = '\0';
}





/***********************************************************
	 input MASK m1,m2      return MASK m1 | m2 
*************************************************************/
/*   get group summary mask */
void alOrMask(m1,m2)
MASK *m1,*m2;
{
MASK x1,x2;
x1 = *m1; x2 = *m2;
x2.Log = x1.Log | x2.Log;
x2.Ack = x1.Ack | x2.Ack;
x2.AckT = x1.AckT | x2.AckT;
x2.Disable = x1.Disable | x2.Disable;
x2.Cancel = x1.Cancel | x2.Cancel;
*m2 = x2;
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

void alNewEvent(stat,sevr,acks,value,clink)
     int stat,sevr,acks;
     char value[MAX_STRING_SIZE];	
     CLINK *clink;
{
     struct chanData *cdata;
     COUNTFILTER *countFilter;

     cdata = clink->pchanData;
     countFilter = cdata->countFilter;
     if ((!countFilter && (cdata->curStat != stat || cdata->curSevr != sevr)) ||
        (countFilter && (countFilter->stat != stat || countFilter->sev != sevr)) )
     {
          alNewAlarm(stat,sevr,value,clink);
     } 
     else
     if (cdata->unackSevr > 0)
     {
          alLogGblAckChan(cdata);
          alAckChan(clink);
     }
}

/*********************************************************** 
     alNewAlarm
************************************************************/
void alNewAlarm(stat,sev,value,clink)
     int stat,sev;
     char value[MAX_STRING_SIZE];	
     CLINK *clink;
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
static void alNewAlarmProcess(stat,sev,value,clink,timeofday)
int stat,sev;
char value[MAX_STRING_SIZE];	
CLINK *clink;
time_t timeofday;
{
struct chanData *cdata;
struct groupData *gdata;
GLINK *glink;
MASK mask;
int stat_prev,sevr_prev,h_unackSevr,h_unackStat,sevrHold;
int viewCount=0;
int prevViewCount=0;

        if (clink == NULL ) return;
        if (sev >= ALARM_NSEV) sev = ALARM_NSEV-1;
        if (stat >= ALARM_NSTATUS) stat = ALARM_NSTATUS-1;
        cdata = clink->pchanData;
        mask = cdata->curMask;

        prevViewCount = awViewViewCount((void *)clink);

        stat_prev = cdata->curStat;
        sevr_prev = cdata->curSevr;

        cdata->curStat = stat;
        cdata->curSevr = sev;
	strcpy(cdata->value,value);

        viewCount = awViewViewCount((void *)clink);
        clink->viewCount = viewCount;

	if (sev > cdata->unackSevr) {
		h_unackSevr = sev;
		h_unackStat = stat;
		}
		else {
		h_unackSevr = cdata->unackSevr;
		h_unackStat = cdata->unackStat;
		}

	if (DEBUG >=3) ALARM_COUNTER++;
	
/*
 * log the channel alarm at the alarm logfile
 */
        if (mask.Log == 0)
		 alLogAlarm(&timeofday,cdata,stat,sev,h_unackStat,h_unackSevr);

/* 
 * disabled alarm special handling 
 */
        if (mask.Disable ==1 ) return;

/*
 * update current alarm history strings
 */
	updateCurrentAlarmString(&timeofday,cdata->name,cdata->value, stat,sev);

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
        alCaPutSevrValue(cdata->sevrPVName,cdata->sevrchid,sev);
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
            alCaPutSevrValue(gdata->sevrPVName,gdata->sevrchid,gdata->curSevr);
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
        if ( mask.Ack == 1) return;

/*
 * transient alarm not required to acknowledge
 */     
        if ( mask.AckT == 1 && cdata->unackSevr > 0 && sev == 0) 
                { alAckChan(clink);
                  return;
                }
 
/*
 * ring beep
 */
 	if (psetup.nobeep == FALSE && sev >= psetup.beepSevr) 
		resetBeep();

/*
 * update unackSev[] of all parent groups and log it
 */

        if ( sev > cdata->unackSevr) {
        glink = clink->parent;
        while (glink) {
			glink->pgroupData->unackSev[cdata->unackSevr]--;
			glink->pgroupData->unackSev[sev]++; 
            glink->pgroupData->unackSevr = alHighestSeverity(glink->pgroupData->unackSev);
			glink->modified = 1;
			glink = glink->parent;
		}               

        }



        if (cdata->unackSevr < sev)  {
                cdata->unackSevr = sev;
                cdata->unackStat = stat;
                }


/*
 *  set psetup.highestSevr for icon and update display
 */


/*	awInvokeCallback();  */

 
        
}

/******************************************************************
	highest system severity used for  icon
*****************************************************************/
/*

----  use this routine if there can be  many areas ---

void alHighestSystemSeverity()
{
GLINK *glink,*pg;
CLINK *pc;
SLIST *list;
SNODE *pt;
int unack,curSevr;

	glink = pmainGroup->p1stgroup;
	psetup.highestSevr = 0;
	psetup.highestUnackSevr = 0;
	list = &(glink->subGroupList);
	pt = list->first;
	while(pt) {
		pg = (GLINK *)pt;
		curSevr = alHighestSeverity(glink->pgroupData->curSev);
		unack = alHighestSeverity(glink->pgroupData->unackSev);
		if (curSevr > psetup.highestSevr) psetup.highestSevr = curSevr;
		if (unack > psetup.highestUnackSevr) 
			psetup.highestUnackSevr = unack;
		pt = sllNext(pt);
		}
	list = &(glink->chanList);
	pt = list->first;
	while(pt) {
		pc = (CLINK *)pt;
		if (pc->pchanData->curSevr > psetup.highestSevr)
			psetup.highestSevr = pc->pchanData->curSevr;
		if (pc->pchanData->unackSevr > psetup.highestUnackSevr)
			psetup.highestUnackSevr = pc->pchanData->unackSevr;
		pt = sllNext(pt);
		}

}
*/

/******************************************************************
	highest system severity used for  icon
*****************************************************************/
void alHighestSystemSeverity(glink)
GLINK *glink;
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
int alHighestSeverity(sevr)
short sevr[ALARM_NSEV];
{
int j=0;
        for (j=ALARM_NSEV-1;j>0;j--) {
        if (sevr[j] > 0) return(j);
        }
	return(0);
}


/*************************************************************    
 * decrement GroupData after channel acknowledgement
 ***************************************************************/
void alAckChan(clink)
CLINK *clink;
{
GLINK *parent;
struct chanData *cdata;
struct groupData *gdata;

/*
 * if no alarm exists
 */
        cdata = (struct chanData *)clink->pchanData;
	if (cdata->unackSevr == 0) return;


/*
 * update all parent groups
 */             
        parent = (GLINK *)clink->parent; 
        while (parent) {
                gdata = (struct groupData *)parent->pgroupData;
                gdata->unackSev[cdata->unackSevr]--;
                gdata->unackSev[0]++;
                gdata->unackSevr = alHighestSeverity(gdata->unackSev);
		parent->modified = 1;
                parent = parent->parent;                      
                }               
        
/* 
 * reset after acknowledgement
 */
        cdata->unackSevr = 0;
        cdata->unackStat = 0;
	clink->modified = 1;

	clink->pmainGroup->modified = TRUE;

}



/*************************************************************    
	ackgroup  update al data structure
 ***************************************************************/
void alAckGroup(glink)
GLINK *glink;
{
CLINK *clink;
GLINK *group;
struct chanData *cdata;
SLIST *list;
SNODE *pt;

        if (glink == NULL) return;

        list = &(glink->chanList);
        pt = sllFirst(list);
        while (pt) {
                clink = (CLINK *)pt;
                cdata = clink->pchanData;
                if (cdata->unackSevr > 0) {
                     alCaPutGblAck(clink);
                     alAckChan(clink);
                }
                pt = sllNext(pt);
        }

        list = &(glink->subGroupList);
        pt = sllFirst(list);
        while (pt) {
                group = (GLINK *)pt;
                if (alHighestSeverity(group->pgroupData->unackSev) > 0) 
                	alAckGroup(group);                
                pt = sllNext(pt);
                }

}



/*************************************************************    
 *This function forces the mask  in the channel according to the 
 *maskIndex and opCode a user selected from the mask menus.
 ***************************************************************/
void alForceChanMask(clink,index,op)
int index,op;
CLINK *clink;
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

}





/************************************************************************* 
 *This function forces the mask for all channels in the group according to the 
 *maskIndex and opCode a user selected from the mask menus.
 **********************************************************************/
void alForceGroupMask(glink,index,op)
int index,op;
GLINK *glink;
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
void alUpdateGroupMask(clink,index,op)
int index,op;
CLINK *clink;
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
			printf("Error:alUpdateGroupMask, mask[%d] < 1",index);
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
void alChangeChanMask(clink,mask)
  MASK mask;
  CLINK *clink;
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

	    if (cdata->evid) 
		alCaClearEvent(clink);
	    

	    if (cdata->curMask.Disable  == 0) {

		if (cdata->unackSevr > NO_ALARM) 
		    alAckChan(clink);
		if (cdata->curSevr > NO_ALARM)  {
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

	    cdata->curSevr = NO_ALARM;

	}

	if (mask.Cancel == 0)
	    if (cdata->evid == NULL) 
		  alCaAddEvent(clink);

  }



if (mask.Disable != cdata->curMask.Disable) {
	alUpdateGroupMask(clink,ALARMDISABLE,mask.Disable);

	cdata->curMask.Disable = mask.Disable;

	change = 1;
	if (mask.Disable == 1 && mask.Cancel == 0) {
		if (cdata->unackSevr > 0)  {
			alAckChan(clink);
			}

		if (cdata->curSevr > 0) {
			parent = clink->parent;
			while(parent) {
				gdata = (struct groupData *)(parent->pgroupData);
				gdata->curSev[cdata->curSevr]--;
				gdata->curSev[0]++;
                /*
                 * spawn SEVRCOMMAND for all the parent groups
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
			cdata->curSevr = 0;
			alNewAlarm(cdata->curStat,saveSevr,cdata->value,clink);

			}
		}

	}

if (mask.Ack != cdata->curMask.Ack) {
	alUpdateGroupMask(clink,ALARMACK,mask.Ack);

	cdata->curMask.Ack = mask.Ack;

	change = 1;
	if (mask.Ack == 1 ) {
		if (cdata->unackSevr > 0 ) alAckChan(clink);
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
            alNewAlarm(cdata->curStat,saveSevr,cdata->value,clink);
        }


	}

}
if (mask.AckT != cdata->curMask.AckT) {
	alUpdateGroupMask(clink,ALARMACKT,mask.AckT);

	cdata->curMask.AckT = mask.AckT;

	change = 1;
	}

if (mask.Log != cdata->curMask.Log) {
	alUpdateGroupMask(clink,ALARMLOG,mask.Log);

	cdata->curMask.Log = mask.Log;

	change = 1;
	}


/*
 * set current mask to new mask
 */

	cdata->curMask = mask;
	if (change == 1) {
		clink->modified = 1;
		clink->pmainGroup->modified = TRUE;
		}


	
}



/*********************************************************************** 
 * This function changes the mask of all the channels in a group to new mask.
 ***********************************************************************/
void alChangeGroupMask(glink,mask)
MASK mask;
GLINK *glink;
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
void alResetGroupMask(glink)
GLINK *glink;
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


/***********************************************************************
 *  obsolete
 * This function forces or resets channel mask and updates all 
 * the parent groups mask values.
 ***********************************************************************/
void alForcePVChanEvent(clink,value)
CLINK *clink;
int value;
{
struct chanData *cdata;
MASK mask;

	cdata = clink->pchanData;

/* 
 * if value = cdata->forcePVValue  force channel mask
 */

	if (value == cdata->forcePVValue)  {
		
		mask = cdata->forcePVMask;

		/*alOrMask(&cdata->curMask,&mask);
		*/

		alChangeChanMask(clink,mask);

		}

/* 
 * if value = cdata->resetPVValue  reset channel mask to default
 */

	if (value == cdata->resetPVValue)  {
		
		mask = cdata->defaultMask;

		alChangeChanMask(clink,mask);

		}

}

/***********************************************************************
 * Obsolete
 * This function force/reset the masks of  all the channels in
 * the glink.  This function perform the automatic force mask 
 * process.
 ***********************************************************************/
void alForcePVGroupEvent(glink,value)  
GLINK *glink;
int value;
{
struct groupData *gdata;
GLINK *subgroup;
CLINK *clink;
MASK mask;
SNODE *pt;

	if (glink == NULL) return;

	gdata = glink->pgroupData;

	pt = sllFirst(&glink->chanList);

/*
 * for each channel force to group mask
 */
	
	if (value == gdata->forcePVValue)  {
			mask = gdata->forcePVMask;
			alChangeGroupMask(glink,mask);

			/*alOrMask(&clink->pchanData->curMask,&mask);
			*/		
		}

/*
 * for each channel reset to default mask
 */
	
	if (value == gdata->resetPVValue)  {
		
		while (pt) {
			clink = (CLINK *)pt;
			mask = clink->pchanData->defaultMask;
			alChangeChanMask(clink,mask);
			pt = sllNext(pt);
			}

/*
 * for each subgroup in this group
 */

	pt = sllFirst(&glink->subGroupList);

	while (pt) {
		
		/* force all the subgroup mask
		 */
		subgroup = (GLINK *)pt;
		alForcePVGroupEvent(subgroup,value);
		pt = sllNext(pt);
		}

	}


}

/***************************************************
  alAlarmGroupName
****************************************************/

char *alAlarmGroupName(link)
     GLINK *link;
{
     if (link){
         if (link->pgroupData->alias) return(link->pgroupData->alias);
         else return(link->pgroupData->name);
     }
     return(0);
}


/***************************************************
  alGuidanceExists
****************************************************/

int alGuidanceExists(link)
     GCLINK *link;
{
     if (sllFirst(&(link->GuideList) ))  return(TRUE);
     return(FALSE);
}

/***************************************************
  alProcessExists
****************************************************/

int alProcessExists(link)
     GCLINK *link;
{
     if (link->pgcData->command)  return(TRUE);
     return(FALSE);
}

