/* alLog.c */

/************************DESCRIPTION***********************************
  Routines for logging messages
**********************************************************************/

static char *sccsId = "@@(#) $Id$";

/************************************************************************
	PUBLIC	Routines for logging messages:

alLogAlarm(cdata,stat,sev,h_unackStat,h_unackSevr)	Log new alarms
alLogGblAckChan(cdata)				Log global acknowledgement channel
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

************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#include "alh.h"
#include "alLib.h"
#include "line.h"
#include "ax.h"
#include "truncateFile.h"

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
	    "",    /* config file name */
	    "",    /* alarm log file name */
	    "",    /* opMod log file name */
	    "",    /* save config file name */
	    0,     /* silenceForever */
	    0,     /* silenceOneHour */
	    0,     /* silenceCurrent */
	    1,     /* 1,2,3,4,5 */
	    0,     /* system highest  sevr */
	    0,     /* system highest unack sevr */
	    0,     /* config files directory */
	    0};    /* log files directory */


int alarmLogFileMaxRecords = 2000;   /* alarm log file maximum # records */
int alarmLogFileOffsetBytes = 0;  /* alarm log file current offset in bytes */
char alarmLogFileEndString[] = "           ";  /* alarm log file end of data string */
int alarmLogFileStringLength = 158;  /* alarm log file record length*/

FILE *fo;       /* write opmod file pointer */
FILE *fl;       /* write alarm log file pointer */

time_t timeofday;
char buff[260],*str;

extern char * alarmSeverityString[];
extern char * alarmStatusString[];

extern int _read_only_flag;         /* RO flag. Albert */
extern int _tm_day_old;             /* DayofMonth. Albert */
extern int _printer_flag;           /* Printer flag. Albert */
extern char printerHostname[120];
extern int  printerPort;
extern char printerColorModel[120];
int write2printer(char *message,int len,int sev);
extern int _time_flag;              /* Dated flag. Albert */


/***********************************************************************
 * log the channel alarm at the alarm logfile
 ***********************************************************************/
void alLogAlarm(time_t *ptimeofdayAlarm,struct chanData *cdata,int stat,
int sev,int h_unackStat,int h_unackSevr)
{
	int status=0;
	/*___________________ For AlLog dated. Albert ___________________ */
	time_t timeofday;
	struct tm *tms;
	char buf[16];
	/*________________________ End. Albert___________________________ */
	/* 158 chars put into buff */
	str = ctime(ptimeofdayAlarm);
	*(str + strlen(str)-1) = '\0';

	sprintf(buff,
	    "%-24s :  %-28s %-12s %-16s %-12s %-16s %-40.40s\n",
	    str,cdata->name,
	    alarmStatusString[stat],alarmSeverityString[sev],
	    alarmStatusString[h_unackStat],
	    alarmSeverityString[h_unackSevr],
	    cdata->value);

	/* update file and Alarm Log text window */
	/*______ For AlLog dated. Albert  ___________________________ */
	if (_time_flag)
	{
		timeofday = time(0L);
		tms = localtime(&timeofday);
		if(tms->tm_mday != _tm_day_old)
		{
			sprintf(buf,".%.4d-%.2d-%.2d",
			    1900+tms->tm_year,1+tms->tm_mon,tms->tm_mday);
			buf[11]=0;
			psetup.logFile[strlen(psetup.logFile) - 11] = 0;
			strncat(psetup.logFile, &buf[0], strlen(buf));
			fclose(fl);
			fl = fopen(psetup.logFile,"w");
			fclose(fl);
			if(!_read_only_flag) fl = fopen(psetup.logFile,"r+");
			else fl = fopen(psetup.logFile,"r");
			_tm_day_old=tms->tm_mday;
			alarmLogFileOffsetBytes =0;
		}
	}
	/*________________________ end for AlLog dated. Albert__________ */
	if (alarmLogFileMaxRecords) {
		if (alarmLogFileOffsetBytes != ftell(fl))
			fseek(fl,alarmLogFileOffsetBytes,SEEK_SET);
		if (alarmLogFileOffsetBytes >= alarmLogFileStringLength*alarmLogFileMaxRecords) {
			rewind(fl);
			status=truncateFile(psetup.logFile,alarmLogFileOffsetBytes);
			alarmLogFileOffsetBytes = 0;
		}

		(void)fprintf(fl,"%s",buff);
		if(_printer_flag) write2printer(buff,158,sev);/* Albert */
		/*---------------
			        (void)fprintf(fl,"%-157s\n",alarmLogFileEndString);
		                fseek(fl,-alarmLogFileStringLength,SEEK_CUR);
				"%-24s :  %-28s %-12s %-16s %-12s %-16s %s\n",
		------------------*/
		alarmLogFileOffsetBytes = ftell(fl);
		fflush(fl);
	} else {
		(void)fprintf(fl,"%s",buff);
		if(_printer_flag) write2printer(buff,sizeof(buff),sev); /*Albert*/
		fflush(fl);
	}
	updateAlarmLog(ALARM_FILE,buff);
}

/***********************************************************************
 * log the connection and access changes to alarm log file
 ***********************************************************************/
void alLogConnection(const char *pvname,const char *ind)
{
	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

	sprintf(buff,"%-26s %-31s: [%s]\n", str,ind,pvname);

	/* update file and Alarm Log text window */
	fprintf(fl,"%s",buff);
	fflush(fl);
	updateLog(OPMOD_FILE,buff);

}

/***********************************************************************
 * log ackchan on operation file
 ***********************************************************************/
void alLogAckChan(struct chanLine *cline)
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
 * global log ackchan received on operation file
 ***********************************************************************/
void alLogGblAckChan(struct chanData *cdata)
{
	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';

	sprintf(buff,"%-26s Gbl Ack Channel--- %-28s %-16s %-16s\n",str,cdata->name,
	    alarmSeverityString[cdata->unackSevr],
	    alarmSeverityString[cdata->curSevr]);
	fprintf(fo,"%s",buff);        /* update the file */
	fflush(fo);
	updateLog(OPMOD_FILE,buff);   /* update the text widget */

}

/***********************************************************************
 * log ackgroup on operation file
 ***********************************************************************/
void alLogAckGroup(struct groupLine *gline)
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
void alLogChanChangeMasks(CLINK *clink,int maskid,int maskno)
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
void alLogForcePVGroup(GLINK *glink,int ind)
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
void alLogResetPVGroup(GLINK *glink,int ind)
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
void alLogForcePVChan(CLINK *clink,int ind)
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
void alLogResetPVChan(CLINK *clink,int ind)
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
void alLogChangeGroupMasks(GLINK *glink,int maskno,int maskid)
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
void alLogSetupConfigFile(char *filename)
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
void alLogSetupAlarmFile(char *filename)
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
void alLogSetupOpmodFile(char *filename)
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
void alLogSetupSaveConfigFile(char *filename)
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

/***********************************************************************
 * log operator modifications 
 ***********************************************************************/
void alLogOpMod(char *text)
{
	timeofday = time(0L);
	str = ctime(&timeofday);
	*(str + strlen(str)-1) = '\0';
	sprintf(buff,"%-26s %s\n", str,text);
	fprintf(fo,"%s",buff);        /* update the file */
	fflush(fo);
	updateLog(OPMOD_FILE,buff);   /* update the text widget */
}


/***********************************************************************
  Call to independent alh_printer process. Albert.
  Write message to TCP-printer. Albert
  sev values are "NO_ALARM","MINOR","MAJOR","INVALID" 
***********************************************************************/
int write2printer(char *message,int len, int sev)
{
	char cmd_buf[250];
	int pid;
	/*cmd_buf=alh_printer ip_addr port printerColorMode  len_mes sev message*/
	sprintf(cmd_buf,"alh_printer %s %d %s %d %d \"%s\"",printerHostname,
	    printerPort,printerColorModel,len,sev,message);
#ifdef WIN32
	{
		static int first=1;
		static char *ComSpec;
		int status;

		/* Get ComSpec for the command shell (should be defined) */
		if (first) {
			first=0;
			ComSpec = getenv("ComSpec");
		}
		if (!ComSpec) {
			errMsg("processSpawn_callback: Cannot find command processor\n");
			return;
		}
		status = _spawnl(_P_DETACH, ComSpec, ComSpec, "/C", cmd_buf, NULL);
	}
#else
	if ((pid=fork ()))
	{
		execl("/bin/sh","sh","-c",cmd_buf,0);
		exit(0);
	}
#endif
	return 0;
}
