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
/* acknowledge.c */

/************************DESCRIPTION***********************************
  This file contains routines for alarm acknowledgement
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#include <stdio.h>
#include <Xm/Xm.h>
#include "alh.h"
#include "alLib.h"
#include "line.h"
#include "ax.h"

extern int _passive_flag;	/* Passive flag. Albert */
extern int _global_flag;
extern int _DB_call_flag;

/*************************************************************    
 * channel acknowledgement
 ***************************************************************/
static void ackChan(CLINK *clink)
{
	struct chanData *cdata;

	if (clink == NULL) return;
	cdata = (struct chanData *)clink->pchanData;
	if (cdata->unackSevr == NO_ALARM) return;

   	if(_DB_call_flag)  alLog2DBAckChan(cdata->name);

	if (_global_flag) {
		alCaPutGblAck(cdata->chid,&cdata->unackSevr);
	} else {
		alSetUnackSevChan(clink,NO_ALARM);
	}

}

/*************************************************************    
 * group acknowledgement
 ***************************************************************/
static void ackGroup(GLINK *glink)
{
	SLIST *list;
	SNODE *pt;

	if (glink == NULL) return;
	if (glink->pgroupData->unackSevr == NO_ALARM) return;

	list = &(glink->chanList);
	pt = sllFirst(list);
	while (pt) {
		ackChan((CLINK *)pt);
		pt = sllNext(pt);
	}

	list = &(glink->subGroupList);
	pt = sllFirst(list);
	while (pt) {
		ackGroup((GLINK *)pt);
		pt = sllNext(pt);
	}
}

/***************************************************
  ack_callback
****************************************************/
void ack_callback(Widget widget, struct anyLine * line,
XmAnyCallbackStruct * cbs)
{
	GLINK *glink;
	CLINK *clink;

	ALINK *area;	/* We need it in passive mode only; Albert */
	if (_passive_flag) {
		XtVaGetValues(widget, XmNuserData, &area, NULL);
		createDialog(area->form_main, XmDIALOG_WARNING,
		    "You can't acknowledge alarms in passive mode.", " ");
		return;
	}
	if (line->linkType == GROUP) {
		glink = (GLINK *)(line->link);
		if (glink->pgroupData->unackSevr == NO_ALARM) return;
		alLogAckGroup(line);
		ackGroup(glink);
	} else if (line->linkType == CHANNEL) {
		clink = (CLINK *)(line->link);
		if (clink->pchanData->unackSevr == NO_ALARM) return;
		alLogAckChan(line);
		ackChan(clink);
	}
}

