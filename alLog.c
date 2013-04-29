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
/* alLog.c */

/************************DESCRIPTION***********************************
  Routines for logging messages
**********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include "alh.h"
#ifdef HAVE_SYSV_IPC
#include <sys/msg.h>
#endif


#ifdef CMLOG
#include <cmlog.h>
#endif

#include "ax.h"
#if 0
#include "alh.h"
#include "alLib.h"
#include "line.h"
#endif
#include <epicsVersion.h>
#if (EPICS_VERSION <= 3) && (EPICS_REVISION <= 13)
#include "truncateFile.h"
#else
#include <epicsStdio.h>
#endif

extern int DEBUG;
extern int _DB_call_flag;
extern int _global_flag;
extern int _lock_flag;
extern int _description_field_flag;
extern int _message_broadcast_flag;
extern int _printer_flag;           /* Printer flag. Albert */
extern int _read_only_flag;         /* RO flag. Albert */
extern int _time_flag;              /* Dated flag. Albert */
extern int _xml_flag;               /* Use XML-ish log format. SNS */
extern char * alhAlarmSeverityString[];
extern const char *ackTransientsString[];
extern char * alhAlarmStatusString[];
extern int DBMsgQId;
extern int masterFlag;
extern int notsave;
extern int printerMsgQId;
extern int tm_day_old;              /* Midnight switch. Albert */

extern struct UserInfo userID; /* info about current operator */
extern  char applicationName[64];  /* Albert1 applicationName = mainGroupName will be send to DB */
extern  char deviceName[64];       /* Albert1 reserved;  will be send to DB */

#ifdef CMLOG
				/* CMLOG flags & variables */
extern int use_CMLOG_alarm;
extern int use_CMLOG_opmod;
cmlog_client_t cmlog;
#endif
extern ALINK *alhArea;

const char *masksdata[] = {
	        "Add / Cancel",
	        "Enable / Disable",
	        "Ack / NoAck",
	        "Ack / NoAck Transient",
	        "Log / NoLog "
};
const char *mask_str[3] = {"OFF", "ON", "RESET"};

struct setup psetup = {         /* initial files & beeping setup */
	    "",    /* config file name */
	    "",    /* alarm log file name */
	    "",    /* opMod log file name */
	    "",    /* save config file name */
	    "",    /* sound wav file name */
	    "",    /* lock files basename */
	    0,     /* silenceForever */
	    0,     /* silenceOneHour */
	    0,     /* silenceCurrent */
	    1,     /* 1,2,3,4,5 */
	    0,     /* system highest  sevr */
	    0,     /* system highest unack sevr */
            0,     /* system highest unack sevr >= beep sevr */
            0,     /* new unack sevr after beep sevr tests */
	    0,     /* config files directory */
	    0};    /* log files directory */

int alarmLogFileMaxRecords = 2000;   /* alarm log file maximum # records */
int alarmLogFileOffsetBytes = 0;  /* alarm log file current offset in bytes */
const char alarmLogFileEndString[] = "           ";  /* alarm log file end of data string */
int alarmLogFileStringLength = 158;  /* alarm log file record length*/

FILE *fo=0;       /* write opmod file pointer */
FILE *fl=0;       /* write alarm log file pointer */

char buff[260];

const char *digit2month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
		       "Sep","Oct","Nov","Dec"};

struct UserInfo {
char *loginid;
char *real_world_name;
char *myhostname;
char *displayName;
};
#if 0


#define REGULAR_RECORD      1  /* usual alLog mess. */ 
#define CONNECT_ALARM       2  /* specific alLog format about connection lost */
#define STOP_LOGGING_ALARM  3  /* specific alLog format about stopping loging */
#define MESSAGE_QUEUE_ALARM 4  /* specific alLog format about MQ problems */
#define ACK_CHANNEL         5  /*  op_mod action which will be saving in DB */
#define ACK_GROUP           6  /*  Don't used now  */
#define CHANGE_MASK         7  /*  op_mod action which will be saving in DB */
#define CHANGE_MASK_GROUP   8  /*  op_mod action which will be saving in DB */
#define FORCE_MASK          9  /*  op_mod action which will be saving in DB */
#define FORCE_MASK_GROUP   10  /*  op_mod action which will be saving in DB */
#define ALARM_LOG_DB 1 /* We are write to ALARM_LOG database */
#define OP_MOD_DB    2 /* We are write to OP_MOD    database */

#endif

static int filePrintf(int fileType,char *buf,time_t *ptime,int typeOfRecord);
#ifdef HAVE_SYSV_IPC
static int write2MQ(int, char *);
static int write2msgQ(int, char *);
#endif

#ifdef CMLOG
/***********************************************************************
 * open the CMLOG connection
 ***********************************************************************/
void alCMLOGconnect(void)
{
   cmlog = cmlog_open("alh");
}

/***********************************************************************
 * close the CMLOG connection
 ***********************************************************************/
void alCMLOGdisconnect(void)
{
   cmlog_close(cmlog);
}
#endif

/***********************************************************************
 * log channel alarm to the alarm logfile
 ***********************************************************************/
void alLogAlarmMessage(time_t *ptimeofdayAlarm,int messageCode,CLINK* clink,const char* fmt,...)
{
    va_list vargs;
    static char text[1024];  /* DANGER: Fixed buffer size */
    struct chanData *cdata;

    if (clink == NULL ) return;
    cdata = clink->pchanData;
    if (cdata == NULL ) return;

    va_start(vargs,fmt);
    vsprintf(text,fmt,vargs);
    va_end(vargs);

    if(text[0] == '\0') sprintf(text," ");

#ifdef CMLOG
    cmlog_logmsg(cmlog,
        0,            /* verbosity */
        0,            /* dummy severity */
        messageCode,    /* code */
        "Alarm",            /* facility */
        "status=%s severity=%s device=%s message=%s "
        "text=%s domain=%s value=%s",
        alhAlarmStatusString[cdata->curStat],
        alhAlarmSeverityString[cdata->curSevr],
        cdata->name,
        cdata->name,
        (cdata->alias ? cdata->alias : "N/A"),
        text,
        (alhArea ? alhArea->blinkString : "N/A"),
        cdata->value);
#endif

    if (_xml_flag) /* Use XML-ish entries which are easier to parse. SNS */
    {
        if (!_description_field_flag)
        { 
            if (_global_flag)
                sprintf(buff,
                    "<pv>%s</pv> <value>%s</value> <status>%s</status> <severity>%s</severity> <status-noack>%s</status-noack> <severity-noack>%s</severity-noack>",
                    cdata->name, cdata->value,
                    alhAlarmStatusString[cdata->curStat],
                    alhAlarmSeverityString[cdata->curSevr],
                    alhAlarmSeverityString[cdata->unackSevr],
                    ackTransientsString[cdata->curMask.AckT]);
            else
                sprintf(buff,
                    "<pv>%s</pv> <value>%s</value> <status>%s</status> <severity>%s</severity>",
                    cdata->name, cdata->value,
                    alhAlarmStatusString[cdata->curStat],
                    alhAlarmSeverityString[cdata->curSevr]);
        }
        else
        {     /* _description_field_flag is ON */
            if (_global_flag)
                sprintf(buff,
                    "<pv>%s</pv> <desc>%s</desc> <value>%s</value> <status>%s</status> <severity>%s</severity> <status-noack>%s</status-noack> <severity-noack>%s</severity-noack>",
                    cdata->name,cdata->description,cdata->value,
                    alhAlarmStatusString[cdata->curStat],
                    alhAlarmSeverityString[cdata->curSevr],
                    alhAlarmSeverityString[cdata->unackSevr],
                    ackTransientsString[cdata->curMask.AckT]);
            else
                sprintf(buff,
                    "<pv>%s</pv> <desc>%s</desc> <value>%s</value> <status>%s</status> <severity>%s</severity>",
                    cdata->name,cdata->description,cdata->value,
                    alhAlarmStatusString[cdata->curStat],
                    alhAlarmSeverityString[cdata->curSevr]);
        }
    }
    else /* Original, non-XMLish format */
    {
        if (!_description_field_flag)
        {
            if (_global_flag)
                sprintf(buff, "%-28s %-12s %-16s %-12s %-5s %-40.40s",
                        cdata->name,
                        alhAlarmStatusString[cdata->curStat],
                        alhAlarmSeverityString[cdata->curSevr],
                        alhAlarmSeverityString[cdata->unackSevr],
                        ackTransientsString[cdata->curMask.AckT],
                        cdata->value);
            else
                sprintf(buff, "%-28s %-12s %-16s %-40.40s",
                        cdata->name,
                        alhAlarmStatusString[cdata->curStat],
                        alhAlarmSeverityString[cdata->curSevr],
                        cdata->value);
        }
        else
        {   /* _description_field_flag is ON */
            if (_global_flag)
                sprintf(buff, "%-28s %-28s %-40.40s %-12s %-16s %-12s %-5s",
                cdata->name,cdata->description,cdata->value,
                alhAlarmStatusString[cdata->curStat],
                alhAlarmSeverityString[cdata->curSevr],
                alhAlarmSeverityString[cdata->unackSevr],
                ackTransientsString[cdata->curMask.AckT]);
            else
                sprintf(buff, "%-28s %-28s %-40.40s %-12s %-16s",
                cdata->name,cdata->description,cdata->value,
                alhAlarmStatusString[cdata->curStat],
                alhAlarmSeverityString[cdata->curSevr]);
        }
    }
    filePrintf(ALARM_FILE,buff,ptimeofdayAlarm,messageCode);
}


/***********************************************************************
 * log operator changes to the opMod logfile
 ***********************************************************************/
void alLogOpModMessage(int messageCode,GCLINK* gclink,const char* fmt,...)
{
    va_list vargs;
    static char text[1024];  /* DANGER: Fixed buffer size */
    struct gcData *gcdata=NULL;
    size_t len;

    if (gclink) gcdata = gclink->pgcData;

    va_start(vargs,fmt);
    vsprintf(text,fmt,vargs);
    va_end(vargs);

    if(text[0] == '\0') sprintf(text," ");

#ifdef CMLOG
    if (use_CMLOG_opmod) {

      if (!gcdata) return;
      cmlog_logmsg(cmlog,
        2,               /* verbosity */
        0,	           /* dummy severity */
	    messageCode,    /* code */
	    "Opmod",             /* facility */
	    "device=%s message=%s text=%s domain=%s",
	    gcdata->name,
	    (gcdata->alias ? gdata->alias : "N/A"),
	    cm_text,
	    (alhArea ? alhArea->blinkString : "N/A"));
    }
#endif
        len = strlen(text);
        if (text[len-1] == '\n' ) text[len-1]=' ';

	if (!alhArea || !alhArea->blinkString){
		sprintf(buff,"%s",text);
	} else {
		if (!gcdata){
			sprintf(buff,"%s: : %s",alhArea->blinkString,text);
		} else {
			sprintf(buff,"%s: %s:  %s",alhArea->blinkString,gcdata->name,text);
		}
	}

	filePrintf(OPMOD_FILE,buff,NULL,messageCode);
}


/***********************************************************************
 * log operator changes to the opMod logfile
 ***********************************************************************/
void alLogOpModAckMessage(int messageCode,GCLINK* gclink,const char* fmt,...)
{
    va_list vargs;
    static char text[1024];  /* DANGER: Fixed buffer size */
    struct gcData *gcdata=NULL;

    if (gclink) gcdata = gclink->pgcData;

    va_start(vargs,fmt);
    vsprintf(text,fmt,vargs);
    va_end(vargs);

    if(text[0] == '\0') sprintf(text," ");

#ifdef CMLOG
    if (use_CMLOG_opmod) {

      if (!gcdata) return;
      cmlog_logmsg(cmlog,
        1,			/* verbosity */
	    0,			/* dummy severity */
        messageCode,		/* code */
        "Opmod",	        /* facility */
        "severity=%s device=%s message=%s text=%s domain=%s",
        alhAlarmSeverityString[gcdata->curSevr],
        gcdata->pname,
        (gcdata->alias ? gcdata->alias : "N/A"),
        cm_text,
        (alhArea ? alhArea->blinkString : "N/A"));
    }
#endif
	if (!alhArea || !alhArea->blinkString){
		sprintf(buff," : : %s",text);
	} else {
		if (!gcdata){
			sprintf(buff,"%s: : %s %-16s",alhArea->blinkString,text,
				alhAlarmSeverityString[gcdata->curSevr]);
		} else {
			sprintf(buff,"%s: %s:  %s %-16s",alhArea->blinkString,gcdata->name,text,
				alhAlarmSeverityString[gcdata->curSevr]);
		}
	}

	filePrintf(OPMOD_FILE,buff,NULL,messageCode);
}


/***********************************************************************
 * log not_save started to alarm log file
 ***********************************************************************/
void alLogNotSaveStart(int not_save_time)
{
	sprintf(buff,"Stop log start  during %d min",not_save_time);
	filePrintf(ALARM_FILE,buff,NULL,STOP_LOGGING_ALARM);
}

/***********************************************************************
 * log not_save started to alarm log file
 ***********************************************************************/
void alLogNotSaveFinish()
{
	sprintf(buff,"Stop log finish");
	filePrintf(ALARM_FILE,buff,NULL,STOP_LOGGING_ALARM);
}

/***********************************************************************
 * log ackchan in special format on operation file and DB (need for forse
save all recordName if someone acknowledges group)
 ***********************************************************************/


void alLog2DBAckChan (char *name)
{
	sprintf(buff,"Ack Channel--- %-28s",name);
	filePrintf(OPMOD_FILE,buff,NULL,ACK_GROUP);  /* update the file */	
}

/***********************************************************************
 * log changeMask in special format on operation file and DB (need for forse
save all recordName if someone acknowledges group)
 ***********************************************************************/
void alLog2DBMask (char *name)
{
	sprintf(buff,"Group Mask ID --- %-28s",name);
	filePrintf(OPMOD_FILE,buff,NULL,CHANGE_MASK_GROUP);  /* update the file */	
}


/**********************************************************************
 Write info to HardDisk OpMod and AlarmLog files 

operations:
1)  add timeString before buffer
2)  fprintf(FILE);
3)  then fflush it
4)  and updateLog/updateAlarmLog

+ checking for non-overriting (master/slave)
+ if _DB_flag        write info to DB
+ if _printer_flag   write info to printer
+ if _time_flag and midnight change AlarmLog and OpMod extension
Parameters: 1) filePointer 
2) Buffer 
3) TimePointer (If we use (very often) current time   = NULL )
4) Special flag for distinguish usual format of alarm (type =REGULAR_RECORD=0)
        and unusual (type = {CONNECT_ALARM,STOP_LOGGING_ALARM...}

***********************************************************************/

static int filePrintf(int fileType,char *buf,time_t *ptime,int typeOfRecord)
{
  int ret=0;
  int status;
  static char bufSave[1024];
  static char DBbuff[1024];
  struct tm *tms;
  char buf_tmp[1024];
  time_t timeofday;

  if(!fileType) return (-1);

  if ((_lock_flag && !masterFlag) && (fileType==ALARM_FILE)) return (0);
  if(_message_broadcast_flag && notsave && (fileType==ALARM_FILE) ) return (0);

  if (ptime == NULL)             /* Current time */
    {
      timeofday = time(0L);
      tms = localtime(&timeofday);
    }
  else tms = localtime(ptime);   /* Calculate time  from event */  


  /*______ For AlLog dated  __________________________________________ */
  if (_time_flag)
    {
      if(tms->tm_mday != tm_day_old)
	{
	  sprintf(buf_tmp,".%.4d-%.2d-%.2d",
		  1900+tms->tm_year,1+tms->tm_mon,tms->tm_mday);
	  buf_tmp[11]=0;
	  
	  psetup.logFile[strlen(psetup.logFile) - 11] = 0;
	  strncat(psetup.logFile, buf_tmp, strlen(buf_tmp));
	  if (fl) fclose(fl);
	  fl = fopen(psetup.logFile,"a");
	  if (fl) fclose(fl);
          fl=0;
	  if(_read_only_flag)  fl = fopen(psetup.logFile,"r");
	  else if (_lock_flag) fl = fopen(psetup.logFile,"a");
	  else fl = fopen(psetup.logFile,"r+");
	  
	  /* The same with psetup.opModFile: */
	  psetup.opModFile[strlen(psetup.opModFile) - 11] = 0;
	  strncat(psetup.opModFile, buf_tmp, strlen(buf_tmp));
	  if (fo) fclose(fo);
	  fo = fopen(psetup.opModFile,"a");
	  if (fo) fclose(fo);
	  if(_read_only_flag)  fo = fopen(psetup.opModFile,"r");
	  else if (_lock_flag) fo = fopen(psetup.opModFile,"a");
	  else fo = fopen(psetup.opModFile,"r+");
	  
	  tm_day_old=tms->tm_mday;
	  alarmLogFileOffsetBytes =0;
	}
    }
  /*________________________ end for AlLog dated ______________________ */

  /* Time format (like  "09-Feb-1999 12:21:12"): */
    if (_xml_flag)
    {
        sprintf(buf_tmp,"<date>%-.2d-%-3s-%-.4d</date> <time>%-.2d:%-.2d:%-.2d</time>",
                tms->tm_mday,digit2month[tms->tm_mon],1900+tms->tm_year,
                tms->tm_hour,tms->tm_min,tms->tm_sec);
        sprintf(bufSave,"<entry>%s %s</entry>\n",buf_tmp,buf);
    }
    else
    {
        sprintf(buf_tmp,"%-.2d-%-3s-%-.4d %-.2d:%-.2d:%-.2d",
                tms->tm_mday,digit2month[tms->tm_mon],1900+tms->tm_year,
                tms->tm_hour,tms->tm_min,tms->tm_sec);
        buf_tmp[20]=0;
        sprintf(bufSave,"%-20s : %s\n",buf_tmp,buf);
    }

    /* Write into Alarm log file*/


    if (fileType==ALARM_FILE) {
	if (alarmLogFileMaxRecords && fl) {
	    if (alarmLogFileOffsetBytes != ftell(fl))
	      fseek(fl,alarmLogFileOffsetBytes,SEEK_SET);
	    if (alarmLogFileOffsetBytes >= alarmLogFileStringLength*alarmLogFileMaxRecords) {
	      rewind(fl);
	      status=truncateFile(psetup.logFile,alarmLogFileOffsetBytes);
	      alarmLogFileOffsetBytes = 0;
	    }
	} 
        if (!fl) ret=0;
        else ret=fprintf(fl,"%s",bufSave);
        if (ret<0 && !_read_only_flag)  {
            fprintf(stderr,"Can't write '%s' to file=%s!!!\n", bufSave,"LOGfile"); 
            errMsg("Error writing '%s' to file=%s!!!\n", bufSave,"LOGfile"); 
        }
        if (alarmLogFileMaxRecords && fl){
            if (!alarmLogFileOffsetBytes) alarmLogFileStringLength=ftell(fl);
            alarmLogFileOffsetBytes = ftell(fl);
        }
        if (fl) fflush(fl);
        updateAlarmLog(ALARM_FILE,bufSave);
    }

    /* Write into Op Mod log file*/

    else if (fileType==OPMOD_FILE) {
        if (!fo) ret=0;
        else ret=fprintf(fo,"%s",bufSave);
        if (ret<0 && !_read_only_flag)  {
            errMsg("Error writing '%s' to file=%s!!!\n", bufSave,"OpModFile"); 
        }
        if (fo) fflush(fo);
        updateLog(OPMOD_FILE,bufSave);
    }
    else fprintf(stderr,"\nBad file type for writing\n");
  

    if( (_printer_flag) && (fileType==ALARM_FILE) &&printerMsgQId ) {
      sprintf(DBbuff,"%d %d %s %s",ALARM_LOG_DB, typeOfRecord+1,buf_tmp,buff); 
#ifdef HAVE_SYSV_IPC
      write2MQ(printerMsgQId, DBbuff);
#endif
    }
  
  if(_DB_call_flag && DBMsgQId ) 
    { 
      if (fileType==OPMOD_FILE) /* write into AlarmOp Database */ 
	{
	  if(typeOfRecord ==0) return(ret);
	  sprintf(DBbuff,"%d %d %s %s %s %s %s %s %s",OP_MOD_DB,typeOfRecord,applicationName,
		  deviceName,userID.loginid,userID.myhostname,userID.displayName,buf_tmp,buff);
	}
      else if (fileType==ALARM_FILE) /* write into AlarmLOG Database */ 
	{
	  if(typeOfRecord ==0) return(ret);
	  sprintf(DBbuff,"%d %d %s %s %s %s %s %s %s", ALARM_LOG_DB, typeOfRecord,applicationName,
		  deviceName,userID.loginid,userID.myhostname,userID.displayName,buf_tmp,buff);
	}
      else 
	{
          fprintf(stderr,"\nBad file type for writing\n"); 
	  return (ret);
	}
#ifdef HAVE_SYSV_IPC
      write2MQ(DBMsgQId, DBbuff);      
#endif
    }
  
return (ret);
}

/***********************************************************************
  Send alarm to message Queue for printer or DB.
  After that it will be print in TCP-printer or save to DB. Albert 
***********************************************************************/

#ifdef HAVE_SYSV_IPC
static int write2MQ(int mq,char *message)
{
  char buf[256];
  static int lostFlag=0;
  static int lostCount=0;
  static time_t tFirst, tLast;
  static char sFirst[32], sLast[32];
  time_t timeofday;
  struct tm *tms;
  static char buf_tmp[32];
  int ret;

  ret=write2msgQ(mq,message);
  if (ret == 1 )
    {
      if(!lostFlag) {tFirst=time(NULL);}
       lostFlag=1;
       tLast=time(NULL);
       lostCount++;
       if(DEBUG) printf("lostCount=%d\n",lostCount);
        return(1);
    }
  else if (ret == - 1 ) return (-1);
  else 
    {
      if(lostFlag) 
	{
	  timeofday = time(0L);
	  tms = localtime(&timeofday);
	  sprintf(buf_tmp,"%-.2d-%-3s-%-.4d %-.2d:%-.2d:%-.2d",
		  tms->tm_mday,digit2month[tms->tm_mon],1900+tms->tm_year,
		  tms->tm_hour,tms->tm_min,tms->tm_sec);
	  buf_tmp[20]=0;

	  tms = localtime(&tFirst);
	  sprintf(sFirst,"%-.2d-%-3s-%-.4d %-.2d:%-.2d:%-.2d",
		  tms->tm_mday,digit2month[tms->tm_mon],1900+tms->tm_year,
		  tms->tm_hour,tms->tm_min,tms->tm_sec);
	  sFirst[20]=0;

	  tms = localtime(&tLast);
	  sprintf(sLast,"%-.2d-%-3s-%-.4d %-.2d:%-.2d:%-.2d",
		  tms->tm_mday,digit2month[tms->tm_mon],1900+tms->tm_year,
		  tms->tm_hour,tms->tm_min,tms->tm_sec);
	  sLast[20]=0;

	  sprintf(buf,"%d %d %s %s %s %s %s %s MQ problem: MQ lost %d messages from=%s to=%s\n",
		  ALARM_LOG_DB,MESSAGE_QUEUE_ALARM, applicationName,deviceName,
		  userID.loginid,userID.myhostname,userID.displayName,buf_tmp,
		  lostCount+1,sFirst,sLast);
	  lostCount=0;
	  lostFlag=0;
	  
	  if(DEBUG) printf("after reconnect buf=%s\n",buf);
	  if (write2msgQ(mq,buf)== 1)  /* bad connection with printer or DB*/
	    {
	      perror("msgQsendError_is_very_unstable");
	      return (1); 
	    }
	  
	}
    }
  return(0);
}


static int write2msgQ(int mq, char *mes)
{
  if (msgsnd(mq,mes,strlen(mes),IPC_NOWAIT /* 0*/) == -1 )
    {
      perror("msgQsendError");
      if(errno == EAGAIN ) return (1);  /* Queue is full */
      return(-1);
    }
  return(0);
}
#endif
