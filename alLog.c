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
#include <sys/msg.h>
#include <errno.h>
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
int ret;
extern char * alarmSeverityString[];
extern char * alarmStatusString[];

extern int _read_only_flag;         /* RO flag. Albert */
extern int tm_day_old;              /* Midnight switch. Albert */
extern int _printer_flag;           /* Printer flag. Albert */
extern int masterFlag;
extern int printerMsgQId;
extern int _lock_flag;

int write2MQ(int, char *);

char *digit2month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
		       "Sep","Oct","Nov","Dec"};
extern int _time_flag;              /* Dated flag. Albert */

extern int _DB_call_flag;
extern int DBMsgQId;
extern int DEBUG;
/***********************************************************************
 * log the channel alarm at the alarm logfile
 ***********************************************************************/
void alLogAlarm(time_t *ptimeofdayAlarm,struct chanData *cdata,int stat,
int sev,int h_unackStat,int h_unackSevr)
{
	int status=0;
	struct tm *tms;
	char printerBuff[250]; /* Albert */
	char DBbuf[250]; /* Albert */
	char buf[30];
	/* 158/154 chars put into buff */
/* new time format (like  "09-Feb-1999 12:21:12")  Albert1. */
	tms = localtime(ptimeofdayAlarm);  
        sprintf(buf,"%-.2d-%-3s-%-.4d %-.2d:%-.2d:%-.2d",
        tms->tm_mday,digit2month[tms->tm_mon],1900+tms->tm_year,
        tms->tm_hour,tms->tm_min,tms->tm_sec);
        buf[20]=0;

/* old time format (like  "Fri Feb 19 12:21:12 1999")
	str = ctime(ptimeofdayAlarm);
	*(str + strlen(str)-1) = '\0';
*/

	sprintf(buff,
	    "%-24s :  %-28s %-12s %-16s %-12s %-16s %-40.40s\n",
	    buf,cdata->name,
	    alarmStatusString[stat],alarmSeverityString[sev],
	    alarmStatusString[h_unackStat],
	    alarmSeverityString[h_unackSevr],
	    cdata->value);

	/* update file and Alarm Log text window */
	/*______ For AlLog dated & printer. Albert  ___________________________ */
	if (_time_flag)
	{
		if(tms->tm_mday != tm_day_old)
		{
			sprintf(buf,".%.4d-%.2d-%.2d",
			    1900+tms->tm_year,1+tms->tm_mon,tms->tm_mday);
			buf[11]=0;
			psetup.logFile[strlen(psetup.logFile) - 11] = 0;
			strncat(psetup.logFile, &buf[0], strlen(buf));
			fclose(fl);
			fl = fopen(psetup.logFile,"a");
			fclose(fl);
			if(_read_only_flag)  fl = fopen(psetup.logFile,"r");
			else if (_lock_flag) fl = fopen(psetup.logFile,"a");
                        else fl = fopen(psetup.logFile,"r+");

			/* The same with psetup.opModFile: */
			psetup.opModFile[strlen(psetup.opModFile) - 11] = 0;
			strncat(psetup.opModFile, &buf[0], strlen(buf));
			fclose(fo);
			fo = fopen(psetup.opModFile,"a");
			fclose(fo);
			if(_read_only_flag)  fo = fopen(psetup.opModFile,"r");
			else if (_lock_flag) fo = fopen(psetup.opModFile,"a");
                        else fo = fopen(psetup.opModFile,"r+");

			tm_day_old=tms->tm_mday;
			alarmLogFileOffsetBytes =0;
		}
	}
	if (_printer_flag)
	{
        	sprintf(printerBuff,
		"%d %-19s %-28s %-7.7s %-7.7s %s",sev+1,
		buf,cdata->name,
		alarmStatusString[stat],alarmSeverityString[sev],
		cdata->value);	
	}
        if(_DB_call_flag)
	  {
           	sprintf(DBbuf,
	    "%d %-24s :  %-28s %-12s %-16s %-12s %-16s %-40.40s\n",
	    sev+1,buf,cdata->name,
	    alarmStatusString[stat],alarmSeverityString[sev],
	    alarmStatusString[h_unackStat],
	    alarmSeverityString[h_unackSevr],
	    cdata->value);
	  }
	/*________________________ end for AlLog dated & other. Albert__________ */


	if (alarmLogFileMaxRecords) {
		if (alarmLogFileOffsetBytes != ftell(fl))
			fseek(fl,alarmLogFileOffsetBytes,SEEK_SET);
		if (alarmLogFileOffsetBytes >= alarmLogFileStringLength*alarmLogFileMaxRecords) {
			rewind(fl);
			status=truncateFile(psetup.logFile,alarmLogFileOffsetBytes);
			alarmLogFileOffsetBytes = 0;
		}

		if(masterFlag) fprintf(fl,"%s",buff);
		if(_printer_flag&&masterFlag) 
		  write2MQ(printerMsgQId, printerBuff); /* Albert */
                if(_DB_call_flag&&masterFlag) {
		  write2MQ(DBMsgQId, DBbuf);      /* Albert */
		}

		/*---------------
			        (void)fprintf(fl,"%-157s\n",alarmLogFileEndString);
		                fseek(fl,-alarmLogFileStringLength,SEEK_CUR);
				"%-24s :  %-28s %-12s %-16s %-12s %-16s %s\n",
		------------------*/
		alarmLogFileOffsetBytes = ftell(fl);
		fflush(fl);
	} else {
		if(masterFlag) fprintf(fl,"%s",buff);  /*Albert*/
		if(_printer_flag&&masterFlag) write2MQ(printerMsgQId,printerBuff);
                if(_DB_call_flag&&masterFlag) {
		  write2MQ(DBMsgQId, DBbuf);      /* Albert */
		}

		fflush(fl);
	}
	updateAlarmLog(ALARM_FILE,buff);
}

/***********************************************************************
 * log the connection and access changes to alarm log file
 ***********************************************************************/
void alLogConnection(const char *pvname,const char *ind)
{
        struct tm *tms;
        char buf[30];

        timeofday = time(0L);

        /* new time format (like  "09-Feb-1999 12:21:12")  Albert1. */
        tms = localtime(&timeofday);  
	sprintf(buf,"%-.2d-%-3s-%-.4d %-.2d:%-.2d:%-.2d",
		tms->tm_mday,digit2month[tms->tm_mon],1900+tms->tm_year,
		tms->tm_hour,tms->tm_min,tms->tm_sec);
	buf[20]=0;
  
	/* old time format (like  "Fri Feb 19 12:21:12 1999")
	   str = ctime(&timeofday);
	   *(str + strlen(str)-1) = '\0';
	   */
	sprintf(buff,"%-26s %-31s: [%s]\n", buf,ind,pvname);
  
	/* update file and Alarm Log text window */
	if(masterFlag) fprintf(fl,"%s",buff);
	fflush(fl);
	updateAlarmLog(ALARM_FILE,buff);
  
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
  Send alarm to message Queue for printer or DB.
  After that it will be print in TCP-printer or save to DB. Albert 
  NOTE: We send sevirity first.
***********************************************************************/

int write2MQ(int mq,char *message)
{
  char buf[60];
  static int lostFlag=0;
  static int lostCount=0;
  static char tFirst[20], tLast[20];
  int ret;

  ret=write2msgQ(mq,message);
  if (ret == 1 )
    {
      if(!lostFlag) { strncpy(tFirst,&message[2],20); tFirst[19]=0;}
       lostFlag=1;
       strncpy(tLast,&message[2],20); tLast[19]=0;
       lostCount++;
       if(DEBUG) printf("lostCount=%d\n",lostCount);
        return(1);
    }
  else if (ret == - 1 ) return (-1);
  else 
    {
      if(lostFlag) 
	{
        sprintf(buf,"%d MQ lost %d messages from=%s to=%s !!!!!!\n",
		1,lostCount+1,tFirst,tLast);
        lostCount=0;
        lostFlag=0;

        if(DEBUG) printf("after reconnect buf=%s\n",buf);
        if (write2msgQ(mq,buf)== 1)  /* bad connection with printer or DB*/
	  {
	          perror("msgQsendError_very_unstable");
                  return (1); 
	  }

	}
    }
  return(0);
}


int write2msgQ(int mq, char *mes)
{
  if (msgsnd(mq,mes,strlen(mes),IPC_NOWAIT /* 0*/) == -1 )
    {
      perror("msgQsendError");
      if(errno == EAGAIN ) return (1);  /* Queue is full */
      return(-1);
    }
  return(0);
}



