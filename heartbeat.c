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
/* heartbeat.c */

/************************DESCRIPTION***********************************
  Routines for the config file heartbeat
**********************************************************************/

#include <stdlib.h>

#include "ax.h"

static void alHeartbeatTimerCallback(XtPointer data, XtIntervalId *id)
{
	struct mainGroup *pmainGroup = (struct mainGroup *)data;

	if (!pmainGroup) return;
	if (!pmainGroup->heartbeatPV.chid) return;
	alCaPutHeartbeatValue(pmainGroup->heartbeatPV.chid,&(pmainGroup->heartbeatPV.caputValue));

	pmainGroup->heartbeatPV.timerId = XtAppAddTimeOut(appContext,
		(unsigned long)(1000*pmainGroup->heartbeatPV.caputRateInSeconds),
		(XtTimerCallbackProc)alHeartbeatTimerCallback,
		(XtPointer)data);
}

void alHeartbeatStart(void *data)
{
	struct mainGroup *pmainGroup = (struct mainGroup *)data;

	if (!pmainGroup) return;
	if (pmainGroup->heartbeatPV.timerId) {
		XtRemoveTimeOut(pmainGroup->heartbeatPV.timerId);
		pmainGroup->heartbeatPV.timerId = 0;
	}

	pmainGroup->heartbeatPV.timerId = XtAppAddTimeOut(appContext,
		(unsigned long)(1000*pmainGroup->heartbeatPV.caputRateInSeconds),
		(XtTimerCallbackProc)alHeartbeatTimerCallback,
		(XtPointer)data);
}


void alHeartbeatStop(void *data)
{
	struct mainGroup *pmainGroup = (struct mainGroup *)data;

	if (!pmainGroup) return;
	if (pmainGroup->heartbeatPV.timerId) {
		XtRemoveTimeOut(pmainGroup->heartbeatPV.timerId);
		pmainGroup->heartbeatPV.timerId = 0;
	}
}

void alHeartbeatPVRemove(struct mainGroup *pmainGroup)
{
	if (!pmainGroup) return;
	if (pmainGroup->heartbeatPV.timerId) {
		XtRemoveTimeOut(pmainGroup->heartbeatPV.timerId);
		pmainGroup->heartbeatPV.timerId = 0;
	}
	if (pmainGroup->heartbeatPV.name) {
		free(pmainGroup->heartbeatPV.name);
		pmainGroup->heartbeatPV.name = 0;
	}
	if (pmainGroup->heartbeatPV.chid) {
		alCaClearChannel(&(pmainGroup->heartbeatPV.chid));
		pmainGroup->heartbeatPV.chid = 0;
	}
}

void alHeartbeatPVAdd(struct mainGroup *pmainGroup,char* name,float rate,short value)
{
	if (!pmainGroup) return;
	alHeartbeatPVRemove(pmainGroup);
	pmainGroup->heartbeatPV.name = (char *)calloc(1,strlen(name)+1);
	strcpy(pmainGroup->heartbeatPV.name,name);
	pmainGroup->heartbeatPV.caputRateInSeconds = rate;
	pmainGroup->heartbeatPV.caputValue = value;
}

