/*
 $Log$
 Revision 1.2  1994/06/22 21:16:28  jba
 Added cvs Log keyword

 */

static char *sccsId = "@(#)alDebug.c	1.7\t9/9/93";

/* alDebug.c
 *
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
 * .01  10-04-91        bkc     Add an option of print address pointers 
 * .02  02-16-93        jba     Removed listWindows & freeWindow - old user interface routines
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

/* alDebug.c */

#include <stdio.h>

#include <sllLib.h>
#include <alLib.h>


struct freeList {
        struct freeList *next; };
typedef struct freeList FREELIST;



/*
-------------
|  PRIVATE  |
-------------
static void print_node_name(pgroup)	Print group configuration
GLINK *pgroup;
*
------------
|  PUBLIC  |
------------
void printConfig()			Print total configuration 
*
void listChanGroup(pgroup)		List alarm tree structure
GLINK *pgroup;
*
						sub windows
						group lines
						channel lines
*
void alListChanGroup(pgroup)			List alarm configuration layout
	SLIST *pgroup;
*
*/


/**************************************************************
	print tree node of configuration 
**************************************************************/ 
static void print_node_name(pgroup)
GLINK *pgroup;
{
SNODE *pt;
GLINK *glink,*node;
CLINK *clink;
SLIST *list,*clist;
        glink = pgroup;
        printf("\nGROUP:%s ",glink->pgroupData->name);
        clist = &(glink->chanList);
        list = &(glink->subGroupList);
                printf("\n");
                
        if (list->count) {
                printf("    SUBGROUP: ");
                pt = sllFirst(list);
                while (pt ) {
                node = (GLINK *)pt;
                printf("  %s ",node->pgroupData->name);
                pt = sllNext(pt);
                }
                printf("\n");
                }
        if (clist->count) {
                printf("    CHANNEL: ");
                pt = sllFirst(clist);
                while (pt)  {
                clink = (CLINK *)pt;
                printf("%s ",clink->pchanData->name);
                pt = sllNext(pt);
                }
                printf("\n");
                }
        
}

/*********************************************************
	list chan group tree structure
********************************************************/
void listChanGroup(pgroup)
GLINK *pgroup;
{
GLINK *glink,*pg;
SLIST *list;
SNODE *temp;
        if (pgroup == NULL) return;
        glink = pgroup;
        list = &(glink->subGroupList);
        print_node_name(glink);

        if (list->count) {
        temp = list->first;
        while (temp != NULL ) { 
                pg = (GLINK *)temp;
                listChanGroup(pg);
                temp = sllNext(temp);
                }
        }
}


/***********************************************************
	list chan group structure
*************************************************************/
void alListChanGroup(pgroup)
SLIST *pgroup;
{
CLINK *clink;
GLINK *glink;
SNODE *pt,*cpt;
        pt = sllFirst(pgroup);
        while (pt) {
                glink = (GLINK *)pt;
                printf("\nGROUP: %s  ",glink->pgroupData->name);
                cpt = sllFirst(&(glink->chanList));
                while (cpt) {
                        clink = (CLINK *)cpt;
                        printf("CHANNEL: %s  ",clink->pchanData->name);
                        cpt = sllNext(cpt);
                }
                alListChanGroup(&(glink->subGroupList));
                pt = sllNext(pt);
        }
}

/*************************************************************
	list groups successfully defined in configuration file
**************************************************************/
void printConfig(pmainGroup)		 
struct mainGroup *pmainGroup;
{
GLINK *proot;
            alListChanGroup((SLIST *)pmainGroup);
            proot = pmainGroup->p1stgroup;
            listChanGroup(proot);
}

/******************************************************
 * This function prints the channel access address allocated for
 * configuration file 
 ************************************************************/
void alCaAddressInfo(proot)
SLIST *proot;
{
SNODE *pt,*temp;
GLINK *glink;
CLINK *clink;
struct groupData *gdata;
struct chanData *cdata;

        temp = sllFirst(proot);

/* for each group in subGroupList */

        while (temp) { 
        glink = (GLINK *)temp; 
        gdata = glink->pgroupData;
        printf("Group    Name=%-30s, glink=%x\n", 
		gdata->name,glink);
        printf("  forcePVName=%-30s, forcechid=%x, forceevid=%x\n",
                gdata->forcePVName,gdata->forcechid,gdata->forceevid);
        printf("  sevrPVName =%-30s, sevrchid=%x\n",
                gdata->sevrPVName,gdata->sevrchid);


/* for each channel in chanList */
                
           pt = sllFirst(&(glink->chanList));
           while (pt)  {
                clink = (CLINK *)pt;
                cdata = clink->pchanData;

                printf("ChannelName=%-30s, chid=%x,      evid=%x,  clink=%x\n",
                        cdata->name,cdata->chid,cdata->evid,clink);
                printf("  forcePVName=%-30s, forcechid=%x, forceevid=%x\n",
                        cdata->forcePVName,cdata->forcechid,cdata->forceevid);
                printf("  sevrPVName =%-30s, sevrchid=%x\n",
                        cdata->sevrPVName,cdata->sevrchid);
                pt = sllNext(pt);
                }
        
        alCaAddressInfo(&(glink->subGroupList));
        temp = sllNext(temp);
        }
           
}

