/* $Id$ */

#include <stdio.h>

#include "sllLib.h"
#include "alLib.h"
#include "ax.h"


/*******************************************************************
 Initialize the necessary group counts and masks.
********************************************************************/
void alInitialize(GLINK *proot)
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

/******************************************************************************
 * derive initial group mask
 ******************************************************************************/
void alInitializeGroupMasks(GLINK *proot)
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
int alInitializeGroupCounters(GLINK *pgroup)
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

    return count;
}

