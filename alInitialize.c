/*
 $Log$
 Revision 1.2  1994/06/22 21:16:32  jba
 Added cvs Log keyword

 */

static char *sccsId = "@(#)alInitialize.c	1.7\t10/1/93";

/* @(#)alInitialize.c  @(#)alInitialize.c	1.7 10/1/93
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
 * .01  02-16-93        jba     Modified alInitialize for new user interface
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */
/* alInitialize.c */

#include <stdio.h>

#include "sllLib.h"
#include "alLib.h"
#include <ax.h>


/*
*-----------------------------------------------------------------
*    routines related to system initialization 
*-----------------------------------------------------------------
*
-------------
|  PUBLIC   |
-------------
void alInitialize(proot)			Initialize alarm configuration
     GLINK *proot;
*
void alInitializeGroupMasks(proot)		Initialize group masks 
	GLINK *proot;
*

int alInitializeGroupCounters(pgroup)	Count number of channels in all sub
	GLINK *pgroup;			groups
*
*/

/*******************************************************************
 *  This function initializes the necessary  group data .
 *******************************************************************/
void alInitialize(proot)
     GLINK *proot;
{


     /*
      * count channels in each group
      */
     alInitializeGroupCounters(proot);  

     /* 
      * count masks in each group from default channel mask ? *   
      */

     alInitializeGroupMasks(proot);
                     
}


/*******************************************************************
 	derive initial group mask
*******************************************************************/
void alInitializeGroupMasks(proot)
GLINK *proot;
{
SNODE *pt;
CLINK *clink;
GLINK *glink,*parent;
MASK mask;
        if (proot == NULL) return;

        pt = sllFirst(&proot->chanList);
        while (pt) {
                clink = (CLINK *)pt;
                mask = clink->pchanData->curMask;

                if ( mask.Cancel == 1 ) {
                        parent = clink->parent;
                        while (parent) {
                                parent->pgroupData->mask[ALARMCANCEL]++;
                                parent = parent->parent;
                                }
                        }

                if ( mask.Disable == 1 ) {
                        parent = clink->parent;
                        while (parent) {
                                parent->pgroupData->mask[ALARMDISABLE]++;
                                parent = parent->parent;
                                }
                        }

                if ( mask.Ack == 1 ) {
                        parent = clink->parent;
                        while (parent) {
                                parent->pgroupData->mask[ALARMACK]++;
                                parent = parent->parent;
                                }
                        }

                if ( mask.AckT == 1 ) {
                        parent = clink->parent;
                        while (parent) {
                                parent->pgroupData->mask[ALARMACKT]++;
                                parent = parent->parent;
                                }
                        }

                if ( mask.Log == 1 ) {
                        parent = clink->parent;
                        while (parent) {
                                parent->pgroupData->mask[ALARMLOG]++;
                                parent = parent->parent;
                                }
                        }

                pt = sllNext(pt);
                }
                        
        pt = sllFirst(&proot->subGroupList);
        while (pt) {
                glink = (GLINK *)pt;
                alInitializeGroupMasks(glink);
                pt = sllNext(pt);
                }
                
}

/***********************************************************
	count no of channels in each subgroup	
*************************************************************/
int alInitializeGroupCounters(pgroup)
GLINK *pgroup;
{
GLINK *glink,*pg;
SLIST *list,*glist;
SNODE *temp;
int count=0;

    if (pgroup == NULL) return 0;
    glink = pgroup;
	list = &(glink->chanList);
	if (sllFirst(list)) {
        count = list->count;
	}
    glist = &(glink->subGroupList);

    if (glist->count) {
         temp = glist->first;
         while (temp != NULL ) { 
             pg = (GLINK *)temp;
             count += alInitializeGroupCounters(pg);
             temp = sllNext(temp);
         }
    }
	glink->pgroupData->curSev[NO_ALARM] += count;
	glink->pgroupData->unackSev[NO_ALARM] += count;

/*
	printf("\n%s  , channels = %d",glink->pgroupData->name,
		glink->pgroupData->curSev[NO_ALARM]);
*/
    return count;
}
