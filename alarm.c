/* alarm.c */

/************************DESCRIPTION***********************************
  Initial alhAlarm global variables
**********************************************************************/

static char *sccsId = "@@(#) $Id$";

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

