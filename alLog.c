static char *sccsId = "@(#)alLog.c	1.12\t12/15/93";

/*  alLog.c    */
/*
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
 * .01  07-22-91        bkc     Add option of logging automatic force/reset 
 *				mask operation 
 *                              
 * .02  11-02-92        bkc     Add the include sys/time.h to this file
 * .03  02-16-93        jba     Modified alLogChanChangeMasks for new user interface
 * .04  12-10-93        jba     Modified psetup initialization
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

/*
--------------------------------------------------------------------------------------------
	PUBLIC	Routines for logging messages:
--------------------------------------------------------------------------------------------

alOpenLogFiles()					Open default files
alLogAlarm(cdata,stat,sev,h_unackStat,h_unackSevr)	Log new alarms
alLogAckChan(cline)					Log acknowledged channel
alLogAckGroup(gline)					Log acknowledged group
alLogChanChangeMasks(cdata)				Log change channel Masks
alLogForcePVGroup(glink,ind)				Log force PV group
alLogResetPVGroup(glink,ind) 				Log reset PV group
alLogForcePVChan(clink,ind)				Log force PV channel
alLogResetPVChan(clink,ind)				Log reset PV channel
alLogExit()						Log exit ALH
alLogChangeGroupMasks(glink,choosegroupData)		Log change group Masks
alLogSetupConfigFile(filename)				Log setup config file
alLogSetupAlarmFile(filename)				Log setup alarm log file
alLogSetupOpmodFile(filename)				Log setup opmod log file
alLogSetupSaveConfigFile(filename)			Log setup save config file

*/

#include <stdio.h>
#include <time.h>

#include <alh.h>
#include <alLib.h>
#include <line.h>
#include <alarmString.h>
#include <ax.h>

extern char *alarmStatusString[];
extern char *alarmSeverityString[];

#define OPERATOR 	1
#define AUTOMATIC 	0

#define  LOG_UNCONN_ALARM 		1
#define  LOG_UNCONN_FORCE_GROUP 	2
#define  LOG_UNCONN_FORCE_CHANNEL 	3
#define  LOG_UNCONN_SEVR_GROUP 		4
#define  LOG_UNCONN_SEVR_CHANNEL 	5

static char *masksdata[] = {
        "Summary ...",
        "Force Process Variable ...",
        "Force Mask ...",
        "Add / Cancel",
        "Enable / Disable",
        "Ack / NoAck",
        "Ack / NoAck Transient",
        "Log / NoLog "
};


struct setup psetup = {         /* initial files & beeping setup */
    "",
    "",
    "",
    "",
    0,
    1,
    1,
    0,
    0,
    0,
    0};




FILE *fo;       /* write opmod file pointer */
FILE *fl;       /* write alarm log file pointer */

time_t timeofday;
char buff[260],*str;


/***********************************************************************
 * open default log files
 ***********************************************************************/
void alOpenLogFiles()
{

/*
 * open default log files
 */
        fl = fopen(psetup.logFile,"a");
        if (fl == NULL) {
            fprintf(stderr,"main: can't open file  %s\n",psetup.logFile);
	    exit(-1);
        }

        fo = fopen(psetup.opModFile,"a");
        if (fo == NULL) {
            fprintf(stderr,"main: can't open file %s\n",psetup.opModFile);
            exit(-1);
        }

        timeofday = time(0L);
        str = ctime(&timeofday);
        *(str + strlen(str)-1) = '\0';
/*
        sprintf(buff,"%-26s Start--- %s\n",str,psetup.configFile);
*/
        sprintf(buff,"%-26s Start--- \n",str);
        fprintf(fo,"%s",buff);
	fflush(fo);

}


/***********************************************************************
 * log the channel alarm at the alarm logfile
 ***********************************************************************/
void alLogAlarm(ptimeofdayAlarm,cdata,stat,sev,h_unackStat,h_unackSevr)
time_t *ptimeofdayAlarm;
int stat,sev,h_unackStat,h_unackSevr;
struct chanData *cdata;
{
		str = ctime(ptimeofdayAlarm);
		*(str + strlen(str)-1) = '\0';

		sprintf(buff,
			"%-24s :  %-28s %-12s %-16s %-12s %-16s %s\n",
			str,cdata->name,
			alarmStatusString[stat],alarmSeverityString[sev],
			alarmStatusString[h_unackStat],
			alarmSeverityString[h_unackSevr],
			cdata->value);	

/* update file and Alarm Log text window */
		(void *)fprintf(fl,"%s",buff);
		fflush(fl);
		updateLog(ALARM_FILE,buff);         

}



	
/***********************************************************************
 * log the connection change on operation file
 ***********************************************************************/
void alLogConnection(pvname,ind)
int ind;
char *pvname;
{
	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

switch (ind) {

 	/* ind = 1 alarm channel not connected */
	case   LOG_UNCONN_ALARM:

		sprintf(buff,"%-26s Not Connected (Channel  PVname): [%s]\n",
			str,pvname);
		break;

	/* ind = 2 Force Group channel not connected */
	case  LOG_UNCONN_FORCE_GROUP:

		sprintf(buff,"%-26s Not Connected (Force Gp PVName): [%s]\n",
			str,pvname);
		break;

	/* ind = 3 Force channel not connected */
	case  LOG_UNCONN_FORCE_CHANNEL:

		sprintf(buff,"%-26s Not Connected (Force Ch PVName): [%s]\n",
			str,pvname);
		break;

	/* ind = 4 Sevr Group channel not connected */
	case  LOG_UNCONN_SEVR_GROUP:

		sprintf(buff,"%-26s Not Connected (Sevr  Gp PVName): [%s]\n",
			str,pvname);
		break;

	/* ind = 5 Sevr   channel not connected */
	case  LOG_UNCONN_SEVR_CHANNEL:

		sprintf(buff,"%-26s Not Connected (Sevr  Ch PVName): [%s]\n",
			str,pvname);
		break;

 	}

	/* update file and Alarm Log text window */

		fprintf(fo,"%s",buff);
		fflush(fo);
		updateLog(OPMOD_FILE,buff); 

}



/***********************************************************************
 * log ackchan on operation file
 ***********************************************************************/
void alLogAckChan(cline)
struct chanLine *cline;
{
  timeofday = time(0L);
  str = ctime(&timeofday);
  *(str + strlen(str)-1) = '\0';

  sprintf(buff,"%-26s Ack Channel--- %-28s %-16s %-16s\n",str, cline->pname,
	alarmSeverityString[cline->unackSevr],
	alarmSeverityString[cline->curSevr]);
  fprintf(fo,"%s",buff);        /* update the file */
  fflush(fo);
  updateLog(OPMOD_FILE,buff);   /* update the text widget */

}

/***********************************************************************
 * log ackgroup on operation file
 ***********************************************************************/
void alLogAckGroup(gline)
struct groupLine *gline;
{
 
  timeofday = time(0L);
  str = ctime(&timeofday);
  *(str + strlen(str)-1) = '\0';

  sprintf(buff,"%-26s Ack Group---   %-28s %-16s %-16s\n",str, gline->pname,
	alarmSeverityString[gline->unackSevr],
	alarmSeverityString[alHighestSeverity(gline->curSev)]);
  fprintf(fo,"%s",buff);        /* update the file */
  fflush(fo);
  updateLog(OPMOD_FILE,buff);   /* update the text widget */
}

/***********************************************************************
 * log change channel Masks on operation file
 ***********************************************************************/
void alLogChanChangeMasks(clink,maskid,maskno)
CLINK *clink;
int maskno,maskid;
{

char buff1[6];


  timeofday = time(0L);
  str = ctime(&timeofday);
  *(str + strlen(str)-1) = '\0';

 	alGetMaskString(clink->pchanData->curMask,buff1);

  if (maskno == 0)
     sprintf(buff,"%-26s Chan  Mask ID ---[%-21s] OFF   [%s] <%s>\n",
        str,masksdata[3+maskid],clink->pchanData->name,buff1);
	
  if (maskno == 1)
     sprintf(buff,"%-26s Chan  Mask ID ---[%-21s] ON    [%s] <%s>\n",
        str,masksdata[3+maskid],clink->pchanData->name,buff1);
  if (maskno == 2)
     sprintf(buff,"%-26s Chan  Mask ID ---[%-21s] RESET [%s] <%s>\n",
        str,masksdata[3+maskid],clink->pchanData->name,buff1);

  fprintf(fo,"%s",buff);	/* update the file */
  fflush(fo);
  updateLog(OPMOD_FILE,buff); 	/* update the text widget */

}


/***********************************************************************
 * log PV force group Masks on operation file
 ***********************************************************************/
void alLogForcePVGroup(glink,ind)
int ind;
GLINK *glink;
{
struct groupData *gdata;
char buff1[6];

if (ind == OPERATOR) {
	gdata = glink->pgroupData;

	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

 	awGetMaskString(gdata->mask,buff1);
	sprintf(buff,"%-26s OPERATOR:Group PV FORCE---[%s] <%s> [%d] [%s]\n",
		str,
		gdata->name,
		buff1,
		gdata->forcePVValue,
		gdata->forcePVName);

	fprintf(fo,"%s",buff);        /* update the file */
	updateLog(OPMOD_FILE,buff);   /* update the text widget */
	}

if (ind == AUTOMATIC) {
	gdata = glink->pgroupData;

	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

  	awGetMaskString(gdata->mask,buff1);
	sprintf(buff,"%-26s AUTOMATIC:Group PV FORCE---[%s] <%s> [%d] [%s]\n",
		str,
		gdata->name,
		buff1,
		gdata->forcePVValue,
		gdata->forcePVName);

	fprintf(fo,"%s",buff);        /* update the file */
	updateLog(OPMOD_FILE,buff);   /* update the text widget */
	}
  fflush(fo);
 
}


/***********************************************************************
 * log PV reset group Masks on operation file
 ***********************************************************************/
void alLogResetPVGroup(glink,ind)
int ind;
GLINK *glink;
{
struct groupData *gdata;
char buff1[6];

if (ind == OPERATOR) {

	gdata = glink->pgroupData;

	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

 	awGetMaskString(gdata->mask,buff1);
	sprintf(buff,"%-26s OPERATOR:Group PV RESET---[%s] <%s> [%d] [%s]\n",
		str,
		gdata->name,
		buff1,
		gdata->resetPVValue,
		gdata->forcePVName);
	fprintf(fo,"%s",buff);        /* update the file */
	updateLog(OPMOD_FILE,buff);   /* update the text widget */
 	}

if (ind == AUTOMATIC) {

	gdata = glink->pgroupData;

	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

 	awGetMaskString(gdata->mask,buff1);
	sprintf(buff,"%-26s AUTOMATIC:Group PV RESET---[%s] <%s> [%d] [%s]\n",
		str,
		gdata->name,
		buff1,
		gdata->resetPVValue,
		gdata->forcePVName);
	fprintf(fo,"%s",buff);        /* update the file */
	updateLog(OPMOD_FILE,buff);   /* update the text widget */
 	}

  fflush(fo);
}

/***********************************************************************
 * log PV force chan Masks on operation file
 ***********************************************************************/
void alLogForcePVChan(clink,ind)
int ind;
CLINK *clink;
{
struct chanData *cdata;
char buff1[6];

if (ind == OPERATOR) {

	cdata = clink->pchanData;

	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

	alGetMaskString(cdata->curMask,buff1);
	sprintf(buff,"%-26s OPERATOR:Chan  PV FORCE---[%s] <%s> [%d] [%s]\n",
		str,
		cdata->name,
		buff1,
		cdata->forcePVValue,
		cdata->forcePVName);
	fprintf(fo,"%s",buff);        /* update the file */
	updateLog(OPMOD_FILE,buff);   /* update the text widget */
 	}

if (ind == AUTOMATIC) {

	cdata = clink->pchanData;

	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

	alGetMaskString(cdata->curMask,buff1);
	sprintf(buff,"%-26s AUTOMATIC:Chan  PV FORCE---[%s] <%s> [%d] [%s]\n",
		str,
		cdata->name,
		buff1,
		cdata->forcePVValue,
		cdata->forcePVName);
	fprintf(fo,"%s",buff);        /* update the file */
	updateLog(OPMOD_FILE,buff);   /* update the text widget */
 	}

  fflush(fo);
}


/***********************************************************************
 * log PV reset chan Masks on operation file
 ***********************************************************************/
void alLogResetPVChan(clink,ind)
int ind;
CLINK *clink;
{
struct chanData *cdata;
char buff1[6];

if (ind == OPERATOR) {
	cdata = clink->pchanData;

	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

	alGetMaskString(cdata->curMask,buff1);
	sprintf(buff,"%-26s OPERATOR:Chan  PV RESET---[%s] <%s> [%d] [%s]\n",
		str,
		cdata->name,
		buff1,
		cdata->resetPVValue,
		cdata->forcePVName);
	fprintf(fo,"%s",buff);        /* update the file */
	updateLog(OPMOD_FILE,buff);   /* update the text widget */
 	}
if (ind == AUTOMATIC) {
	cdata = clink->pchanData;

	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

	alGetMaskString(cdata->curMask,buff1);
	sprintf(buff,"%-26s AUTOMATIC:Chan  PV RESET---[%s] <%s> [%d] [%s]\n",
		str,
		cdata->name,
		buff1,
		cdata->resetPVValue,
		cdata->forcePVName);
	fprintf(fo,"%s",buff);        /* update the file */
	updateLog(OPMOD_FILE,buff);   /* update the text widget */
 	}

  fflush(fo);
}


/***********************************************************************
 * log exit on operation file
 ***********************************************************************/
void alLogExit()
{

  timeofday = time(0L);
  str = ctime(&timeofday);
  *(str + strlen(str)-1) = '\0';

  sprintf(buff,"%-26s Setup---Exit\n",str);
  fprintf(fo,"%s",buff);        /* update the file */
  fflush(fo);
  updateLog(OPMOD_FILE,buff);   /* update the text widget */
 }

/***********************************************************************
 * log change group mask selection 
 ***********************************************************************/
void alLogChangeGroupMasks(glink,maskno,maskid)
GLINK *glink;
int maskno;
int maskid;
{
char buff1[6];

  timeofday = time(0L);
  str = ctime(&timeofday);
  *(str + strlen(str)-1) = '\0';

 	awGetMaskString(glink->pgroupData->mask,buff1);

  if (maskno == 0) 
  sprintf(buff,"%-26s Group Mask ID ---[%-21s] OFF   [%s] <%s>\n",
	str,masksdata[3+maskid],glink->pgroupData->name,
	buff1);
  if (maskno == 1) 
	sprintf(buff,"%-26s Group Mask ID ---[%-21s] ON    [%s] <%s>\n",
	   str,masksdata[3+maskid],glink->pgroupData->name,
	   buff1);
  if (maskno == 2) 
	sprintf(buff,"%-26s Group Mask ID ---[%-21s] RESET [%s] <%s>\n",
	   str,masksdata[3+maskid],glink->pgroupData->name,
	   buff1);

  fprintf(fo,"%s",buff);        /* update the file */
  fflush(fo);
  updateLog(OPMOD_FILE,buff);   /* update the text widget */
}



/***********************************************************************
 * log setup config file 
 ***********************************************************************/
void alLogSetupConfigFile(filename)
char *filename;
{
           timeofday = time(0L);
           str = ctime(&timeofday);
           *(str + strlen(str)-1) = '\0';
	   sprintf(buff,"%-26s Setup---Config File : %s -> %s\n",
	   	str,psetup.configFile,filename);
	   fprintf(fo,"%s",buff);        /* update the file */
  fflush(fo);
	   updateLog(OPMOD_FILE,buff);   /* update the text widget */
}


/***********************************************************************
 * log setup alarm log file selection 
 ***********************************************************************/
void alLogSetupAlarmFile(filename)
char *filename;
{
	   timeofday = time(0L);
	   str = ctime(&timeofday);
	   *(str + strlen(str)-1) = '\0';
	   sprintf(buff,"%-26s Setup---Alarm Log File : %s -> %s\n",
		str,psetup.logFile,filename);
	   fprintf(fo,"%s",buff);        /* update the file */
  fflush(fo);
	   updateLog(OPMOD_FILE,buff);   /* update the text widget */
}

/***********************************************************************
 * log setup operator's log file selection 
 ***********************************************************************/
void alLogSetupOpmodFile(filename)
char *filename;
{
	   timeofday = time(0L);
	   str = ctime(&timeofday);
	   *(str + strlen(str)-1) = '\0';
	   sprintf(buff,"%-26s Setup---OpMod File : %s -> %s\n",
	   	str,psetup.opModFile,filename);
	   fprintf(fo,"%s",buff);        /* update the file */
  fflush(fo);
	   updateLog(OPMOD_FILE,buff);   /* update the text widget */
}

/***********************************************************************
 * log setup save configuration file selection 
 ***********************************************************************/
void alLogSetupSaveConfigFile(filename)
char *filename;
{
	   timeofday = time(0L);
	   str = ctime(&timeofday);
	   *(str + strlen(str)-1) = '\0';
	   sprintf(buff,"%-26s Setup---Save New Config: %s\n",
	   	str,filename);
	   fprintf(fo,"%s",buff);        /* update the file */
  fflush(fo);
	   updateLog(OPMOD_FILE,buff);   /* update the text widget */
}
