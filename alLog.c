/*
 $Log$
 Revision 1.14  1998/09/01 20:33:22  jba
 Made changes for WIN32 build (using Exceed5)

 Revision 1.13  1998/08/05 18:20:04  jba
 Added silenceOneHour button.
 Moved silenceForever button to Setup menu.
 Added logging for operator silence changes.

 Revision 1.12  1998/07/23 18:22:34  jba
 Log the connection and access rights changes to alarm log file

 Revision 1.11  1998/07/23 18:17:00  jba
 ANSI c changes.

 Revision 1.10  1998/06/22 18:42:11  jba
 Merged the new alh-options created at DESY MKS group:
  -D Disable Writing, -S Passive Mode, -T AlarmLogDated, -P Printing

 Revision 1.9  1998/05/13 19:29:47  evans
 More WIN32 changes.

 Revision 1.8  1998/05/12 18:22:41  evans
 Initial changes for WIN32.

 Revision 1.7  1997/08/27 22:03:31  jba
 Fixed parm order.

 Revision 1.6  1997/01/09 14:38:13  jba
 Added alarmLog circular file facility.

 Revision 1.5  1996/06/07 15:46:43  jba
 Added global alarm acknowledgement.
 Simplified log output.

 Revision 1.4  1996/03/25 15:42:59  jba
 Removed unused alOpenLogFiles routine.

 * Revision 1.3  1995/10/20  16:50:00  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 *
 * Revision 1.2  1994/06/22  21:16:40  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@@(#)alLog.c	1.12\t12/15/93";

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

*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

#include <alh.h>
#include <alLib.h>
#include <line.h>
#include <ax.h>
#include <truncateFile.h>

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
void alLogAlarm(ptimeofdayAlarm,cdata,stat,sev,h_unackStat,h_unackSevr)
time_t *ptimeofdayAlarm;
int stat,sev,h_unackStat,h_unackSevr;
struct chanData *cdata;
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
 * global log ackchan received on operation file
 ***********************************************************************/
void alLogGblAckChan(cdata)
struct chanData *cdata;
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

/*  
Call to independent alh_printer process. Albert.
*/

int write2printer(message,len,sev) /* Write message to TCP-printer. Albert */
char *message;
int len; 
int sev;  /*"NO_ALARM","MINOR","MAJOR","INVALID" */
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
