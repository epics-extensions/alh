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
/* alarm.c */

/************************DESCRIPTION***********************************
  Initial alhAlarm global variables
**********************************************************************/

#include "alh.h"
#include "ax.h"
#include "alarmString.h"

const char * alhAlarmSeverityString[ALH_ALARM_NSEV ];
const char * alhAlarmStatusString[ALH_ALARM_NSTATUS];
const char * alhErrorSeverityString[] = { "ERROR" };
const char * alhErrorStatusString[] = { "NOT_CONNECTED","NO_READ_ACCESS","NO_WRITE_ACCESS" };

void alhAlarmStringInit()
{
	int i;

	for (i=0;i<ALARM_NSEV;i++) {
		alhAlarmSeverityString[i]=alarmSeverityString[i];
	}
	alhAlarmSeverityString[ALARM_NSEV]=alhErrorSeverityString[0];

	for (i=0;i<ALARM_NSTATUS;i++) {
		alhAlarmStatusString[i]=alarmStatusString[i];
	}
	alhAlarmStatusString[ALARM_NSTATUS  ]=alhErrorStatusString[0];
	alhAlarmStatusString[ALARM_NSTATUS+1]=alhErrorStatusString[1];
	alhAlarmStatusString[ALARM_NSTATUS+2]=alhErrorStatusString[2];
}

