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
/* guidance.c */

/************************DESCRIPTION***********************************
  This file contains routines for displaying guidance text
**********************************************************************/

#include <stdlib.h>
#include <Xm/Xm.h>

#include "alLib.h"
#include "ax.h"

/************************************************************************
 Guidance display callback
 ***********************************************************************/
void guidanceCallback(Widget w,GCLINK *gclink,XmAnyCallbackStruct *cbs)
{
	SNODE *pt;
	struct guideLink *guidelist;
	char *guidance_str[200];
	int i=0;

	if (guidanceExists(gclink)) {
		if (gclink->guidanceLocation ) {
			callBrowser(gclink->guidanceLocation);
		} else {
			pt = sllFirst(&(gclink->GuideList));
			i=0;
			while (pt) {
				guidelist = (struct guideLink *)pt;
				guidance_str[i] = guidelist->list;
				pt = sllNext(pt);
				i++;
			}
			guidance_str[i] = "";
			guidance_str[i+1] = "";

			xs_help_callback(w,guidance_str,cbs);
		}
	}
	else {
		if (gclink->pgcData->alias){
			createDialog(w,XmDIALOG_WARNING,"No guidance for ",
			    gclink->pgcData->alias);
		} else {
			createDialog(w,XmDIALOG_WARNING,"No guidance for ",
			    gclink->pgcData->name);
		}
	}
}

/************************************************************************
 Guidance exists test
 ***********************************************************************/
int guidanceExists(GCLINK *link)
{
	if (sllFirst(&(link->GuideList)) || link->guidanceLocation) return(TRUE);
	else return(FALSE);
}

/************************************************************************
 Delete guidance 
 ***********************************************************************/
void guidanceDeleteGuideList(SLIST *pGuideList)
{
	SNODE *node,*next;
	struct guideLink *guidelist;

	node = sllFirst(pGuideList);
	while (node) {
		next = sllNext(node);
		guidelist = (struct guideLink *)node;
		free(guidelist->list);
		free(guidelist);
		node = next;
	}
}

/************************************************************************
 Copy guidance 
 ***********************************************************************/
void guidanceCopyGuideList(SLIST *pToGuideList,SLIST *pFromGuideList)
{
	char *buff;
	struct guideLink *guideLink;
	SNODE *node;

	node = sllFirst(pFromGuideList);
	while (node) {
		buff = ((struct guideLink *)node)->list;
		guideLink = (struct guideLink *)calloc(1,sizeof(struct guideLink));
		guideLink->list = (char *)calloc(1,strlen(buff)+1);
		strcpy(guideLink->list,buff);
		sllAdd(pToGuideList,(SNODE *)guideLink);
		node = sllNext(node);
	}
}

