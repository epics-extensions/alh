/* alLog.c */

/************************DESCRIPTION***********************************
  Routines for logging messages
**********************************************************************/

static char *sccsId = "@@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#ifdef WIN32
#include <process.h>
#else
#include <unistd.h>
#include <sys/msg.h>
#endif

#ifdef CMLOG
#include <cmlog.h>
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

char *ackTransientsString[] = {"ackT","noackT"};

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

extern int DEBUG;
int alarmLogFileMaxRecords = 2000;   /* alarm log file maximum # records */
int alarmLogFileOffsetBytes = 0;  /* alarm log file current offset in bytes */
char alarmLogFileEndString[] = "           ";  /* alarm log file end of data string */
int alarmLogFileStringLength = 158;  /* alarm log file record length*/

FILE *fo;       /* write opmod file pointer */
FILE *fl;       /* write alarm log file pointer */

char buff[260];
extern char * alhAlarmSeverityString[];
extern char * alhAlarmStatusString[];

extern int _global_flag;
extern int _read_only_flag;         /* RO flag. Albert */
extern int tm_day_old;              /* Midnight switch. Albert */
extern int _printer_flag;           /* Printer flag. Albert */
extern int masterFlag;
extern int printerMsgQId;
extern int _lock_flag;

#ifndef WIN32
int write2MQ(int, char *);
int write2msgQ(int, char *);
#endif
int filePrintf(FILE *fPointer,char *buf,time_t *ptime,int typeOfRecord);

char *digit2month[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
		       "Sep","Oct","Nov","Dec"};

extern int _time_flag;              /* Dated flag. Albert */

extern int _DB_call_flag;
extern int DBMsgQId;
extern int _message_broadcast_flag;
extern int notsave;

#ifdef CMLOG
				/* CMLOG flags & variables */
extern int use_CMLOG_alarm;
extern int use_CMLOG_opmod;
cmlog_client_t cmlog;
char *cm_host;
extern ALINK *alhArea;
#endif

struct UserInfo {
char *loginid;
char *real_world_name;
char *myhostname;
char *displayName;
};

extern struct UserInfo userID; /* info about current operator */

extern  char applicationName[64];  /* Albert1 applicationName = mainGroupName will be send to DB */
extern  char deviceName[64];       /* Albert1 reserved;  will be send to DB */


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


#ifdef CMLOG
/***********************************************************************
 * open the CMLOG connection
 ***********************************************************************/
void alCMLOGconnect(void)
{
   cmlog = cmlog_open("alh");
   cm_host = malloc(strlen(userID.myhostname)+strlen(userID.displayName)+4);
   sprintf(cm_host, "%s (%s)", userID.myhostname, userID.displayName);
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
 * log the channel alarm at the alarm logfile
 ***********************************************************************/
void alLogAlarm(time_t *ptimeofdayAlarm,struct chanData *cdata,int stat,
int sev,int unackSevr,int ackT)
{

#ifdef CMLOG
   char cm_text[80];
   if (use_CMLOG_alarm && masterFlag) {
	    if (_global_flag) {
		    sprintf(cm_text, "(%s / %s)",
		    alhAlarmSeverityString[unackSevr],
		    ackTransientsString[ackT]);
	    } else {
		    sprintf(cm_text, "( / )");
	    }

	    cmlog_logmsg(cmlog,
		    0,			/* verbosity */
		    0,			/* dummy severity */
		    REGULAR_RECORD,	/* code */
		    "alh-Alarm",	/* facility */
		    "host=%s status=%s severity=%s device=%s text=%s domain=%s value=%s",
		    cm_host,
		    alhAlarmStatusString[stat],
		    alhAlarmSeverityString[sev],
		    cdata->name,
		    cm_text,
		    (alhArea && alhArea->blinkString)?alhArea->blinkString:"unknown",
		    cdata->value);
   }
#endif

        if (_global_flag) {
		sprintf(buff,
			"%-28s %-12s %-16s %-12s %-5s %-40.40s\n",
			cdata->name,
			alhAlarmStatusString[stat],
			alhAlarmSeverityString[sev],
			alhAlarmSeverityString[unackSevr],
			ackTransientsString[ackT],
			cdata->value);
        } else {

		sprintf(buff,
			"%-28s %-12s %-16s %-40.40s\n",
			cdata->name,
			alhAlarmStatusString[stat],
			alhAlarmSeverityString[sev],
			cdata->value);
		}
		filePrintf(fl,buff,ptimeofdayAlarm,REGULAR_RECORD);
}

/***********************************************************************
 * log the connection and access changes to alarm log file
 ***********************************************************************/
void alLogConnection(const char *pvname,const char *ind)
{
#ifdef CMLOG
   if (use_CMLOG_alarm && masterFlag) {
	cmlog_logmsg(cmlog,
	    0,			/* verbosity */
	    0,			/* dummy severity */
	    CONNECT_ALARM,	/* code */
	    "alh-Alarm",	/* facility */
	    "host=%s status=%s severity=%s device=%s text=%s domain=%s",
	    cm_host,
	    "CONNECT",
	    "ERROR",
	    pvname,
	    ind,
	    (alhArea && alhArea->blinkString)?alhArea->blinkString:"unknown");
   }
#endif

	sprintf(buff,"%-31s: [%s]\n",ind,pvname);
	filePrintf(fl,buff,NULL,0);
}

/***********************************************************************
 * log ackchan on operation file
 ***********************************************************************/
void alLogAckChan(struct anyLine *line)
{
#ifdef CMLOG
    if (use_CMLOG_opmod) {
	char cm_text[80];
        if (_global_flag) {
		sprintf(cm_text, "Global Ack Channel (%s / %s)",
		    alhAlarmSeverityString[line->unackSevr]);
	} else {
		sprintf(cm_text, "Local Ack Channel ( / )");
	}

	cmlog_logmsg(cmlog,
	    1,			/* verbosity */
	    0,			/* dummy severity */
	    ACK_CHANNEL,	/* code */
	    "alh-Opmod",	/* facility */
	    "host=%s status=%s severity=%s device=%s text=%s domain=%s",
	    cm_host,
	    alhAlarmStatusString[line->curStat],
	    alhAlarmSeverityString[line->curSevr],
	    line->pname,
	    cm_text,
	    alhArea->blinkString);
   }
#endif

        if (_global_flag) {
	    sprintf(buff,"Global Ack Channel--- %-28s %-16s %-16s\n",line->pname,
		alhAlarmSeverityString[line->unackSevr],
		alhAlarmSeverityString[line->curSevr]);
	} else {
	    sprintf(buff,"Local Ack Channel--- %-28s %-16s %-16s\n",line->pname,
		alhAlarmSeverityString[line->unackSevr],
		alhAlarmSeverityString[line->curSevr]);
	}
	filePrintf(fo,buff,NULL,ACK_CHANNEL);  /* update the file */	
}

/***********************************************************************
 * log ackgroup on operation file
 ***********************************************************************/
void alLogAckGroup(struct anyLine *line)
{
#ifdef CMLOG
   if (use_CMLOG_opmod) {
	char cm_text[80];
        if (_global_flag) {
	    sprintf(cm_text, "Global Ack Group (%s)",
	    alhAlarmSeverityString[line->unackSevr]);
	} else {
	    sprintf(cm_text, "Local Ack Group (%s)",
	    alhAlarmSeverityString[line->unackSevr]);
	}

	cmlog_logmsg(cmlog,
	    1,			/* verbosity */
	    0,			/* dummy severity */
	    ACK_GROUP,		/* code */
	    "alh-Opmod",	/* facility */
	    "host=%s severity=%s device=%s text=%s domain=%s",
	    cm_host,
	    alhAlarmSeverityString[line->curSevr],
	    line->pname,
	    cm_text,
	    alhArea->blinkString);
   }
#endif

        if (_global_flag) {
	    sprintf(buff,"Global Ack Group---   %-28s %-16s %-16s\n",line->pname,
		alhAlarmSeverityString[line->unackSevr],
		alhAlarmSeverityString[alHighestSeverity(line->curSev)]);
	} else {
	    sprintf(buff,"Local Ack Group---   %-28s %-16s %-16s\n",line->pname,
		alhAlarmSeverityString[line->unackSevr],
		alhAlarmSeverityString[alHighestSeverity(line->curSev)]);
	}
	    filePrintf(fo,buff,NULL,0);        /* update the file */
}

/***********************************************************************
 * log change channel Masks on operation file
 ***********************************************************************/
void alLogChangeChanMasks(CLINK *clink,int maskno,int maskid)
{
	char buff1[6];
	alGetMaskString(clink->pchanData->curMask,buff1);

#ifdef CMLOG
   if (use_CMLOG_opmod) {
	char *cm_mask[3] = {"OFF", "ON", "RESET"};
	char cm_text[80];
	sprintf(cm_text, "Channel Mask [%s] %s <%s>",
	    masksdata[3+maskid],
	    cm_mask[maskno],
	    buff1);

	cmlog_logmsg(cmlog,
	    2,			/* verbosity */
	    0,			/* dummy severity */
	    CHANGE_MASK,	/* code */
	    "alh-Opmod",	/* facility */
	    "host=%s device=%s text=%s domain=%s",
	    cm_host,
	    clink->pchanData->name,
	    cm_text,
	    alhArea->blinkString);
   }
#endif

	if (maskno == 0)
		sprintf(buff,"Chan  Mask ID ---[%-21s] OFF   [%s] <%s>\n",
		    masksdata[3+maskid],clink->pchanData->name,buff1);

	if (maskno == 1)
		sprintf(buff,"Chan  Mask ID ---[%-21s] ON    [%s] <%s>\n",
		    masksdata[3+maskid],clink->pchanData->name,buff1);
	if (maskno == 2)
		sprintf(buff,"Chan  Mask ID ---[%-21s] RESET [%s] <%s>\n",
		    masksdata[3+maskid],clink->pchanData->name,buff1);
	filePrintf(fo,buff,NULL,CHANGE_MASK);        /* update the file */
}

/***********************************************************************
 * log PV force group Masks on operation file
 ***********************************************************************/
void alLogForcePVGroup(GLINK *glink,int ind)
{
	struct groupData *gdata;
	char buff1[6];

	gdata = glink->pgroupData;
	awGetMaskString(gdata->mask,buff1);

	if (ind == OPERATOR) {
#ifdef CMLOG
	   if (use_CMLOG_opmod) {
	      char cm_text[80];
	      sprintf(cm_text, "OPER Group PV FORCE <%s> [%d] [%s]",
		      buff1,
		      gdata->forcePVValue,
		      gdata->forcePVName);
	      
	      cmlog_logmsg(cmlog,
		      2,	           /* verbosity */
		      0,	           /* dummy severity */
		      CHANGE_MASK_GROUP,   /* code */
		      "alh-Opmod",         /* facility */
		      "host=%s device=%s text=%s domain=%s",
		      cm_host,
		      gdata->name,
		      cm_text,
		      alhArea->blinkString);
	   }
#endif
		sprintf(buff,"OPERATOR:Group PV FORCE---[%s] <%s> [%d] [%s]\n",
		    gdata->name,
		    buff1,
		    gdata->forcePVValue,
		    gdata->forcePVName);
		filePrintf(fo,buff,NULL,0);        /* update the file */
	}

	if (ind == AUTOMATIC) {
#ifdef CMLOG
	   if (use_CMLOG_opmod) {
	      char cm_text[80];
	      sprintf(cm_text, "AUTO Group PV FORCE <%s> [%d] [%s]",
		      buff1,
		      gdata->forcePVValue,
		      gdata->forcePVName);
	      
	      cmlog_logmsg(cmlog,
		      2,	          /* verbosity */
		      0,	          /* dummy severity */
		      FORCE_MASK_GROUP,   /* code */
		      "alh-Opmod",        /* facility */
		      "host=%s device=%s text=%s domain=%s",
		      cm_host,
		      gdata->name,
		      cm_text,
		      alhArea->blinkString);
	   }
#endif
		sprintf(buff,"AUTOMATIC:Group PV FORCE---[%s] <%s> [%d] [%s]\n",
		    gdata->name,
		    buff1,
		    gdata->forcePVValue,
		    gdata->forcePVName);
		filePrintf(fo,buff,NULL,0);        /* update the file */
	}
}


/***********************************************************************
 * log PV reset group Masks on operation file
 ***********************************************************************/
void alLogResetPVGroup(GLINK *glink,int ind)
{
	struct groupData *gdata;
	char buff1[6];

	gdata = glink->pgroupData;
	awGetMaskString(gdata->mask,buff1);

	if (ind == OPERATOR) {
#ifdef CMLOG
	   if (use_CMLOG_opmod) {
	      char cm_text[80];
	      sprintf(cm_text, "OPER Group PV RESET <%s> [%d] [%s]",
		      buff1,
		      gdata->resetPVValue,
		      gdata->forcePVName);
	      
	      cmlog_logmsg(cmlog,
		      2,	           /* verbosity */
		      0,	           /* dummy severity */
		      CHANGE_MASK_GROUP,   /* code */
		      "alh-Opmod",         /* facility */
		      "host=%s device=%s text=%s domain=%s",
		      cm_host,
		      gdata->name,
		      cm_text,
		      (alhArea && alhArea->blinkString)?alhArea->blinkString:"unknown");
	   }
#endif
		sprintf(buff,"OPERATOR:Group PV RESET---[%s] <%s> [%d] [%s]\n",
		    gdata->name,
		    buff1,
		    gdata->resetPVValue,
		    gdata->forcePVName);
		filePrintf(fo,buff,NULL,0);        /* update the file */
	}

	if (ind == AUTOMATIC) {
#ifdef CMLOG
	   if (use_CMLOG_opmod) {
	      char cm_text[80];
	      sprintf(cm_text, "AUTO Group PV RESET <%s> [%d] [%s]",
		      buff1,
		      gdata->resetPVValue,
		      gdata->forcePVName);
	      
	      cmlog_logmsg(cmlog,
		      2,	          /* verbosity */
		      0,	          /* dummy severity */
		      FORCE_MASK_GROUP,   /* code */
		      "alh-Opmod",        /* facility */
		      "host=%s device=%s text=%s domain=%s",
		      cm_host,
		      gdata->name,
		      cm_text,
		      (alhArea && alhArea->blinkString)?alhArea->blinkString:"unknown");
	   }
#endif
		sprintf(buff,"AUTOMATIC:Group PV RESET---[%s] <%s> [%d] [%s]\n",
		    gdata->name,
		    buff1,
		    gdata->resetPVValue,
		    gdata->forcePVName);
		filePrintf(fo,buff,NULL,0);        /* update the file */
	}

}

/***********************************************************************
 * log PV force chan Masks on operation file
 ***********************************************************************/
void alLogForcePVChan(CLINK *clink,int ind)
{
	struct chanData *cdata;
	char buff1[6];

	cdata = clink->pchanData;
	alGetMaskString(cdata->curMask,buff1);

	if (ind == OPERATOR) {
#ifdef CMLOG
	   if (use_CMLOG_opmod) {
	      char cm_text[80];
	      sprintf(cm_text, "OPER Channel PV FORCE <%s> [%d] [%s]",
		      buff1,
		      cdata->forcePVValue,
		      cdata->forcePVName);
	      
	      cmlog_logmsg(cmlog,
		      2,	           /* verbosity */
		      0,	           /* dummy severity */
		      CHANGE_MASK,         /* code */
		      "alh-Opmod",         /* facility */
		      "host=%s device=%s text=%s domain=%s",
		      cm_host,
		      cdata->name,
		      cm_text,
		      alhArea->blinkString);
	   }
#endif
		sprintf(buff,"OPERATOR:Chan  PV FORCE---[%s] <%s> [%d] [%s]\n",
		    cdata->name,
		    buff1,
		    cdata->forcePVValue,
		    cdata->forcePVName);
		filePrintf(fo,buff,NULL,0);        /* update the file */
	}

	if (ind == AUTOMATIC) {
#ifdef CMLOG
	   if (use_CMLOG_opmod) {
	      char cm_text[80];
	      sprintf(cm_text, "AUTO Channel PV FORCE <%s> [%d] [%s]",
		      buff1,
		      cdata->forcePVValue,
		      cdata->forcePVName);
	      
	      cmlog_logmsg(cmlog,
		      2,	          /* verbosity */
		      0,	          /* dummy severity */
		      FORCE_MASK,         /* code */
		      "alh-Opmod",        /* facility */
		      "host=%s device=%s text=%s domain=%s",
		      cm_host,
		      cdata->name,
		      cm_text,
		      alhArea->blinkString);
	   }
#endif
		sprintf(buff,"AUTOMATIC:Chan  PV FORCE---[%s] <%s> [%d] [%s]\n",
		    cdata->name,
		    buff1,
		    cdata->forcePVValue,
		    cdata->forcePVName);
		filePrintf(fo,buff,NULL,0);        /* update the file */
	}

}


/***********************************************************************
 * log PV reset chan Masks on operation file
 ***********************************************************************/
void alLogResetPVChan(CLINK *clink,int ind)
{
	struct chanData *cdata;
	char buff1[6];

	cdata = clink->pchanData;
	alGetMaskString(cdata->curMask,buff1);

	if (ind == OPERATOR) {
#ifdef CMLOG
	   if (use_CMLOG_opmod) {
	      char cm_text[80];
	      sprintf(cm_text, "OPER Channel PV RESET <%s> [%d] [%s]",
		      buff1,
		      cdata->resetPVValue,
		      cdata->forcePVName);
	      
	      cmlog_logmsg(cmlog,
		      2,	           /* verbosity */
		      0,	           /* dummy severity */
		      CHANGE_MASK,         /* code */
		      "alh-Opmod",         /* facility */
		      "host=%s device=%s text=%s domain=%s",
		      cm_host,
		      cdata->name,
		      cm_text,
		      alhArea->blinkString);
	   }
#endif
		sprintf(buff,"OPERATOR:Chan  PV RESET---[%s] <%s> [%d] [%s]\n",
		    cdata->name,
		    buff1,
		    cdata->resetPVValue,
		    cdata->forcePVName);
		filePrintf(fo,buff,NULL,0);        /* update the file */

	}
	if (ind == AUTOMATIC) {
#ifdef CMLOG
	   if (use_CMLOG_opmod) {
	      char cm_text[80];
	      sprintf(cm_text, "AUTO Channel PV RESET <%s> [%d] [%s]",
		      buff1,
		      cdata->resetPVValue,
		      cdata->forcePVName);
	      
	      cmlog_logmsg(cmlog,
		      2,	          /* verbosity */
		      0,	          /* dummy severity */
		      FORCE_MASK,         /* code */
		      "alh-Opmod",        /* facility */
		      "host=%s device=%s text=%s domain=%s",
		      cm_host,
		      cdata->name,
		      cm_text,
		      alhArea->blinkString);
	   }
#endif
		sprintf(buff,"AUTOMATIC:Chan  PV RESET---[%s] <%s> [%d] [%s]\n",
		    cdata->name,
		    buff1,
		    cdata->resetPVValue,
		    cdata->forcePVName);
		filePrintf(fo,buff,NULL,0);        /* update the file */

	}

}


/***********************************************************************
 * log exit on operation file
 ***********************************************************************/
void alLogExit()
{
	sprintf(buff,"Setup---Exit\n");
	filePrintf(fo,buff,NULL,0);        /* update the file */
}

/***********************************************************************
 * log change group mask selection 
 ***********************************************************************/
void alLogChangeGroupMasks(GLINK *glink,int maskno,int maskid)
{
	char buff1[6];
	char *mask_str[3] = {"OFF", "ON", "RESET"};
	awGetMaskString(glink->pgroupData->mask,buff1);

#ifdef CMLOG
   if (use_CMLOG_opmod) {
	char cm_text[80];
	sprintf(cm_text, "Group Mask [%s] %s <%s>",
	    masksdata[3+maskid],
	    mask_str[maskno],
	    buff1);

	cmlog_logmsg(cmlog,
	    2,			/* verbosity */
	    0,			/* dummy severity */
	    CHANGE_MASK_GROUP,	/* code */
	    "alh-Opmod",	/* facility */
	    "host=%s device=%s text=%s domain=%s",
	    cm_host,
	    glink->pgroupData->name,
	    cm_text,
	    alhArea->blinkString);
   }
#endif

	sprintf(buff,"Group Mask ID ---[%-21s] %-5s [%s] <%s>\n",
	    masksdata[3+maskid], mask_str[maskno],
	    glink->pgroupData->name,
	    buff1);
	filePrintf(fo,buff,NULL,0);        /* update the file */
}

/***********************************************************************
 * log setup config file 
 ***********************************************************************/
void alLogSetupConfigFile(char *filename)
{
	sprintf(buff,"Setup---Config File : %s -> %s\n",psetup.configFile,filename);
	filePrintf(fo,buff,NULL,0);        /* update the file */
}

/***********************************************************************
 * log setup alarm log file selection 
 ***********************************************************************/
void alLogSetupAlarmFile(char *filename)
{
	sprintf(buff,"Setup---Alarm Log File : %s -> %s\n",psetup.logFile,filename);
	filePrintf(fo,buff,NULL,0);        /* update the file */
}

/***********************************************************************
 * log setup operator's log file selection 
 ***********************************************************************/
void alLogSetupOpmodFile(char *filename)
{
	sprintf(buff,"Setup---OpMod File : %s -> %s\n",psetup.opModFile,filename);
	filePrintf(fo,buff,NULL,0);        /* update the file */
}

/***********************************************************************
 * log setup save configuration file selection 
 ***********************************************************************/
void alLogSetupSaveConfigFile(char *filename)
{
	sprintf(buff,"Setup---Save New Config: %s\n",filename);
	filePrintf(fo,buff,NULL,0);        /* update the file */
}

/***********************************************************************
 * log operator modifications 
 ***********************************************************************/
void alLogOpMod(char *text)
{
	if (text[strlen(text)-1] == '\n') sprintf(buff,"%s",text);
	else sprintf(buff,"%s\n",text);
	filePrintf(fo,buff,NULL,0);        /* update the file */
}

/***********************************************************************
 * log not_save started to alarm log file
 ***********************************************************************/
void alLogNotSaveStart(int not_save_time)
{
	sprintf(buff,"Stop log start  during %d min\n",not_save_time);
	filePrintf(fl,buff,NULL,STOP_LOGGING_ALARM);
}

/***********************************************************************
 * log not_save started to alarm log file
 ***********************************************************************/
void alLogNotSaveFinish()
{
	sprintf(buff,"Stop log finish\n");
	filePrintf(fl,buff,NULL,STOP_LOGGING_ALARM);
}

/***********************************************************************
 * log ackchan in special format on operation file and DB (need for forse
save all recordName if someone acknowledges group)
 ***********************************************************************/


void alLog2DBAckChan (char *name)
{
	sprintf(buff,"Ack Channel--- %-28s\n",name);
	filePrintf(fo,buff,NULL,ACK_GROUP);  /* update the file */	
}
/***********************************************************************
 * log changeMask in special format on operation file and DB (need for forse
save all recordName if someone acknowledges group)
 ***********************************************************************/
void alLog2DBMask (char *name)
{
	sprintf(buff,"Group Mask ID --- %-28s\n",name);
	filePrintf(fo,buff,NULL,CHANGE_MASK_GROUP);  /* update the file */	
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

int filePrintf(FILE *fPointer,char *buf,time_t *ptime,int typeOfRecord)
{
  int ret=0;
  int status;
  static char bufSave[512];
  static char DBbuff[1024];
  struct tm *tms;
  char buf_tmp[32];
  time_t timeofday;

  if(!fPointer) return (-1);

  if((!masterFlag) && (fPointer==fl)) return (0);
  if(_message_broadcast_flag && notsave && (fPointer==fl) ) return (0);

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
	  fclose(fl);
	  fl = fopen(psetup.logFile,"a");
	  fclose(fl);
	  if(_read_only_flag)  fl = fopen(psetup.logFile,"r");
	  else if (_lock_flag) fl = fopen(psetup.logFile,"a");
	  else fl = fopen(psetup.logFile,"r+");
	  
	  /* The same with psetup.opModFile: */
	  psetup.opModFile[strlen(psetup.opModFile) - 11] = 0;
	  strncat(psetup.opModFile, buf_tmp, strlen(buf_tmp));
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
  /*________________________ end for AlLog dated ______________________ */

  /* Time format (like  "09-Feb-1999 12:21:12"): */

  sprintf(buf_tmp,"%-.2d-%-3s-%-.4d %-.2d:%-.2d:%-.2d",
	  tms->tm_mday,digit2month[tms->tm_mon],1900+tms->tm_year,
	  tms->tm_hour,tms->tm_min,tms->tm_sec);
  buf_tmp[20]=0;

  sprintf(bufSave,"%-20s : %s",buf_tmp,buf);

	if (alarmLogFileMaxRecords&&(fPointer==fl)) 
	  {
	    if (alarmLogFileOffsetBytes != ftell(fl))
	      fseek(fl,alarmLogFileOffsetBytes,SEEK_SET);
	    if (alarmLogFileOffsetBytes >= alarmLogFileStringLength*alarmLogFileMaxRecords) {
	      rewind(fl);
	      status=truncateFile(psetup.logFile,alarmLogFileOffsetBytes);
	      alarmLogFileOffsetBytes = 0;
	    }
	  } 


  ret=fprintf(fPointer,"%s",bufSave);

  if (ret<0)  {
    fprintf(stderr,"Can't write %s to file=%s!!!\n",
	    bufSave,(fPointer==fl)?"LOGfile":"OpModFile" ); 
  }

  if (alarmLogFileMaxRecords&&(fPointer==fl)) alarmLogFileOffsetBytes = ftell(fl);

  fflush(fPointer);

  
  if(fPointer==fl)        updateAlarmLog(ALARM_FILE,bufSave);
  else if (fPointer==fo)  updateLog     (OPMOD_FILE,bufSave);
  else fprintf(stderr,"\nBad fPointer for writing\n");
  
  if( (_printer_flag) && (fPointer==fl) &&printerMsgQId ) 
    {
      sprintf(DBbuff,"%d %d %s %s",ALARM_LOG_DB, typeOfRecord+1,buf_tmp,buff); 
#ifndef WIN32
      write2MQ(printerMsgQId, DBbuff);
#endif
    }
  
  if(_DB_call_flag && DBMsgQId ) 
    { 
      if (fPointer==fo) /* write into AlarmOp Database */ 
	{
	  if(typeOfRecord ==0) return(ret);
	  sprintf(DBbuff,"%d %d %s %s %s %s %s %s %s",OP_MOD_DB,typeOfRecord,applicationName,
		  deviceName,userID.loginid,userID.myhostname,userID.displayName,buf_tmp,buff);
	}
      else if (fPointer==fl) /* write into AlarmLOG Database */ 
	{
	  if(typeOfRecord ==0) return(ret);
	  sprintf(DBbuff,"%d %d %s %s %s %s %s %s %s", ALARM_LOG_DB, typeOfRecord,applicationName,
		  deviceName,userID.loginid,userID.myhostname,userID.displayName,buf_tmp,buff);
	}
      else 
	{
          fprintf(stderr,"\nBad fPointer for writing\n"); 
	  return (ret);
	}
#ifndef WIN32
      write2MQ(DBMsgQId, DBbuff);      
#endif
    }
  
return (ret);
}

/***********************************************************************
  Send alarm to message Queue for printer or DB.
  After that it will be print in TCP-printer or save to DB. Albert 
***********************************************************************/

#ifndef WIN32
int write2MQ(int mq,char *message)
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
#endif
