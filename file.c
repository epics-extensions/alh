/* file.c */

/************************DESCRIPTION***********************************
  File contains file handling routines
**********************************************************************/

static char *sccsId = "@(#) $Id$";
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>   /* Albert1 */
#include <signal.h>

#include <errno.h>

#ifndef WIN32
/* WIN32 does not have dirent.h used by opendir, closedir */
#include <unistd.h>  /* Albert1 */
#include <dirent.h>
#include <sys/msg.h>  
#else
#include <process.h>
#endif
#include <ctype.h>

#include <Xm/Xm.h>

#include "alh.h"
#include "alLib.h"
#include "axArea.h"
#include "ax.h"

/* default  file names */
#define DEFAULT_CONFIG  "ALH-default.alhConfig"
#define DEFAULT_ALARM   "ALH-default.alhAlarm"
#define DEFAULT_OPMOD   "ALH-default.alhOpmod"

int _read_only_flag=0;       /* Read-only flag. Albert */
int _passive_flag=0;         /* Passive flag.   Albert */

int _printer_flag=0;         /* Printer flag.               Albert */
int  printerMsgQKey;         /* Network printer MsgQKey.    Albert */
int  printerMsgQId;          /* Network printer MsgQId.     Albert */
 
int _time_flag=0;            /* Dated flag.             Albert */
int tm_day_old;              /* Day-variable for dated. Albert */

int _DB_call_flag=0;         /* Database(Oracle...) call. Albert */
int  DBMsgQKey;         /* Database MsgQKey.    Albert */
int  DBMsgQId;          /* database MsgQId.     Albert */

int _message_broadcast_flag=0;            /* Message Broadcast System Albert */
char messBroadcastLockFileName[250];   /* FN for lock file. Albert */
char messBroadcastInfoFileName[250];   /* FN for info file. Albert */
int  messBroadcastDeskriptor;          /* FD for lock file. Albert */
void broadcastMessTesting();
XtIntervalId broadcastMessTimeoutId=NULL;
int amIsender=0;
int notsave=0;     
char *rebootString="MIN  ALH  WILL  NOT  SAVE ALARM LOG!!!!";
int max_not_save_time=10;
void broadcastMess_exit_quit(int);
unsigned long broadcastMessDelay=2000;    /* in msec. periodical messages testing. Albert */   


int _lock_flag=0;                /* Flag for locking. Albert */
char lockFileName[250];          /* FN for lock file. Albert */
int lockFileDeskriptor;          /* FD for lock file. Albert */
unsigned long lockDelay=5000;    /* in msec. periodical masterStatus testing. Albert */
int masterFlag=1;                /* am I master for write operations? Albert */  
void masterTesting();            /* periodical calback for masterStatus testing. Albert */
extern Widget blinkToplevel;     /* for locking status marking */
char masterStr[30],slaveStr[30]; /* titles for Master/Slave with/whitout printer */
XtIntervalId lockTimeoutId=NULL;   
 
extern int DEBUG;

extern int alarmLogFileMaxRecords;  /* alarm log file maximum # records */
extern int alarmLogFileOffsetBytes; /* alarm log file current offset in bytes */
extern char alarmLogFileEndString[]; /* alarm log file end of data string */
extern int alarmLogFileEndStringLength; /* alarm log file end data string len*/

extern FILE *fl;		/* alarm log pointer */
extern FILE *fo;		/* opmod log pointer */

struct command_line_data
{
	char* configDir;
	char* logDir;
	char* configFile;
	char* logFile;
	char* opModFile;
	int alarmLogFileMaxRecords;
};
static struct command_line_data commandLine = { 
	NULL,NULL,NULL,NULL,NULL,0};

#define PARM_DEBUG			0
#define PARM_ACT			1
#define PARM_ALL_FILES_DIR		2
#define PARM_LOG_DIR			3
#define PARM_ALARM_LOG_FILE		4
#define PARM_OPMOD_LOG_FILE		5
#define PARM_ALARM_LOG_MAX		6
#define PARM_PRINTER			7
#define PARM_DATED			8
#define PARM_PASSIVE			9
#define PARM_READONLY			10
#define PARM_HELP			11
#define PARM_SILENT			12
#define PARM_LOCK			13
#define PARM_DATABASE			14
#define PARM_MESSAGE_BROADCAST		15

struct parm_data
{
	char* parm;
	int len;
	int id;
};

static struct parm_data ptable[] = {
		{ "-v",		2,	PARM_DEBUG },
		{ "-c",		2,	PARM_ACT },
		{ "-f",		2,	PARM_ALL_FILES_DIR },
		{ "-l",		2,	PARM_LOG_DIR },
		{ "-a",		2,	PARM_ALARM_LOG_FILE },
		{ "-o",		2,	PARM_OPMOD_LOG_FILE },
		{ "-m",		2,	PARM_ALARM_LOG_MAX },
		{ "-P",		2,	PARM_PRINTER },
		{ "-T",		2,	PARM_DATED },
		{ "-S",		2,	PARM_PASSIVE },
		{ "-s",		2,	PARM_SILENT },
		{ "-D",		2,	PARM_READONLY },
		{ "-help",	5,	PARM_HELP },
		{ "-L",		2,	PARM_LOCK },     /* Albert1 */
 		{ "-B",		2,	PARM_MESSAGE_BROADCAST }, /* Albert1 */ 
              	{ "-O",		2,	PARM_DATABASE }, /* Albert1 */
	        { NULL,		-1,     -1 }};

/* forward declarations */
static void saveConfigFile_callback(Widget widget,char *filename,
XmAnyCallbackStruct *cbs);
static void printUsage(char *);
static void fileSetup(char *filename,ALINK *area,int fileType,int programId,
Widget widget);
static int checkFilename(char *filename,int fileType);
static int getCommandLineParms(int argc, char** argv);

/***************************************************
 exit and quit application
***************************************************/
void exit_quit(Widget w,ALINK *area,XmAnyCallbackStruct *call_data)
{
	GLINK *proot=0;
	if(_message_broadcast_flag && amIsender) {
        createDialog(blinkToplevel,XmDIALOG_MESSAGE,
		    "You sent a message\n","Wait a seconds before message will delivery\n");
	return;
	}

	alLogExit();
	fclose(fl);
	fclose(fo);
	if (area && area->pmainGroup) proot = area->pmainGroup->p1stgroup;

	/* 
	      * note: if pmainGroup or proot == NULL then probably never even fired up initial
	      *	 configuration file and ca...
	      */
	if (proot) {

		/* cancel all the channel access requests */
		if (programId==ALH) {
			alCaCancel((SLIST *)proot);
			alCaStop();
		}

		/* delete all the subgroups of proot & free proot */
		alDeleteGroup(proot);

	}

	if (programId==ACT) editClipboardSet(0,0);
	if (area  && area->pmainGroup) free(area->pmainGroup);
	if (area) free(area);
	XtDestroyWidget(topLevelShell);
#ifndef WIN32
	if(_lock_flag)  {
	  lockf(lockFileDeskriptor,     F_ULOCK, 0L); /* Albert1 */
	  if (lockTimeoutId) {
	    XtRemoveTimeOut(lockTimeoutId);
	  }
	if(_message_broadcast_flag)  {
	  lockf(messBroadcastDeskriptor, F_ULOCK, 0L); /* Albert1 */
	  if (broadcastMessTimeoutId) {
	    XtRemoveTimeOut(broadcastMessTimeoutId);
	  }
	}

	} 
#endif
	
	exit(0);
}

/******************************************************
 shorten file name without path
******************************************************/
char *shortfile(char *name)
{
	int len;
	char *shortname;

	len = strlen(name);
	shortname = name;
	while (len != 0) {
		if(*(name+len)== '/') {
			shortname = name+len+1;
			break;
		}
		len--;
	}
	return shortname;
}

/******************************************************
  checkFilename
******************************************************/
static int checkFilename(char *filename,int fileType)
{
	FILE *tt = 0;

	if ( filename[0] == '\0') return 2;

	if ( DEBUG == 1 )
		printf("\nFilename is %s \n", filename);

	switch (fileType) {
	case FILE_CONFIG:
	case FILE_CONFIG_INSERT:
		tt = fopen(filename,"r");
		if (!tt){
			return 2;
		}
		break;

	case FILE_SAVEAS:
		tt = fopen(filename,"r");
		if (tt){
			return 3;
		}

	case FILE_SAVE:
	case FILE_SAVEAS_OK:
	case FILE_PRINT:
		tt = fopen(filename,"w");
		if (!tt){
			return 4;
		}
		break;

	case FILE_OPMOD:
	case FILE_ALARMLOG:
		if(!_read_only_flag) tt = fopen(filename,"a");

		else {tt = fopen(filename,"r");
		if(!tt) strcpy(filename,"/tmp/AlhDisableWriting");
		tt = fopen(filename,"w");
		if (!tt) {
			fprintf(stderr,"can't read OPMOD or ALARMLOG and /tmp directory \n");
			exit(1);
		}
		}
		if (!tt){
			return 4;
		}
		break;
	}

	fclose(tt);
	return 0;
}

/******************************************************
  fileCancelCallback
******************************************************/
void fileCancelCallback(Widget widget,ALINK *area,
XmFileSelectionBoxCallbackStruct *cbs)
{
	area->managed = TRUE;
	XtUnmanageChild(widget);
}

/******************************************************
  fileSetupCallback
******************************************************/
void fileSetupCallback(Widget widget,int client_data,
XmFileSelectionBoxCallbackStruct *cbs)
{
	char *filename;
	ALINK *area;
	int pgm;


	/* get the filename string */
	XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &filename);

	if ( DEBUG == 1 )
	   printf("\nfileSetupCallback: filename is %s \n", filename);

	/* get the area pointer */
	XtVaGetValues(widget, XmNuserData, &area, NULL);

	if (area) pgm = area->programId;
	else pgm = programId;
	fileSetup(filename,area,client_data,pgm,widget);
	XtFree(filename);

}

/******************************************************
  fileSetup
******************************************************/
static void fileSetup(char *filename,ALINK *area,int fileType,
int programId,Widget widget)
{
	int    error;
	char   fileTypeString[NAMEDEFAULT_SIZE];
	char   str[MAX_STRING_LENGTH];
	char  *dir=0;
	char  *pattern=0;
	char  *filename_dup;
	FILE  *tt;
	Widget fileSelectionBox;
	time_t timeofday;
	struct tm *tms;
	char buf[16];

	/* _______ For Dated AlLog File. Albert______________________________*/
	timeofday = time(0L);
	tms = localtime(&timeofday);
	sprintf(buf,".%.4d-%.2d-%.2d",
	    1900+tms->tm_year,1+tms->tm_mon,tms->tm_mday);
	buf[11]=0;
	tm_day_old = tms->tm_mday;
	if ( ((fileType == FILE_ALARMLOG)||(fileType == FILE_OPMOD))&&(_time_flag)  )
	{
		strncat(filename, &buf[0], strlen(buf));
	}
	/* _______ End. Albert______________________________*/
	error = checkFilename(filename,fileType);
	if (error){
		switch(fileType) {

		case FILE_CONFIG:
		case FILE_CONFIG_INSERT:
			pattern = CONFIG_PATTERN;
			dir = psetup.configDir;
			if (programId == ALH) {
				strcpy(fileTypeString,"Alarm Handler: Alarm Configuration File");
			} else {
				/* not error to start act with no filename */
				if ( filename[0] == '\0' && fileType == FILE_CONFIG ) error = 0;
				strcpy(fileTypeString,"Alarm Configuration Tool: Alarm Configuration File");
			}
			break;

		case FILE_OPMOD:
			pattern = OPMOD_PATTERN;
			dir = psetup.logDir;
			strcpy(fileTypeString,"OpMod Log File");
			break;

		case FILE_ALARMLOG:
			pattern = ALARMLOG_PATTERN;
			dir = psetup.logDir;
			strcpy(fileTypeString,"Alarm Log File");
			break;

		case FILE_PRINT:
			pattern = TREEREPORT_PATTERN;
			dir = NULL;
			strcpy(fileTypeString,"Print File");
			break;

		default:
			pattern = '\0';
			dir = psetup.configDir;
			strcpy(fileTypeString,"Filename");
			break;

		}

	}
	if (error){
		fileSelectionBox = widget;
		/* Display file selection box  */
		if ( XtIsShell(widget)) {
			fileSelectionBox = createFileDialog(widget,
			    (void *)fileSetupCallback, (XtPointer)fileType,
			    (void *)exit_quit,(XtPointer)FALSE,
			    (XtPointer)NULL,
			    fileTypeString, (String)pattern, dir);
		}

		/* Display file error dialog */
		switch (error){
		case 1:
			createDialog(fileSelectionBox,XmDIALOG_ERROR,filename," is a directory.");
			break;

		case 2:
			/* no warning if DEFAULT filename does not exist */
			if ( strcmp(shortfile(filename),DEFAULT_CONFIG) )
				createDialog(fileSelectionBox,XmDIALOG_ERROR,filename," open error.");
			break;

		case 3:
			strcpy(str, filename);
			strcat(str," already exists.  Overwrite?");
			filename_dup = malloc(strlen(filename)+1);
			if ( filename_dup ) strcpy(filename_dup,filename);
			createActionDialog(fileSelectionBox,XmDIALOG_WARNING, str ,
			    (XtCallbackProc)saveConfigFile_callback,
			    (XtPointer)filename_dup,(XtPointer)area);
			free(filename_dup);
			break;

		case 4:
			createDialog(fileSelectionBox,XmDIALOG_ERROR,filename," write error.");
			break;
		default:
			break;
		}
	} else {
		/* unmanage the fileSelection dialog */
		if ( !XtIsShell(widget))
			createFileDialog(0,0,0,0,0,0,0,0,0);

		switch(fileType) {

		case FILE_CONFIG:
			setupConfig(filename,programId,area);
			if(_lock_flag)                              /* Albert1 */
			  {
			    FILE *fp;
			    strcpy(lockFileName,psetup.configFile);
			    strcat(lockFileName,".LOCK");
			    if (!(fp=fopen(lockFileName,"a")))
			      {
				perror("Can't open locking file for w");
				exit(1);
			      }
                              fclose(fp);     
			    if((lockFileDeskriptor=open(lockFileName,O_RDWR,0644)) == 0)
			      { 
				perror("Can't open locking file for rw");
				exit(1);
			      }
			    if (DEBUG) fprintf(stderr,"INIT: deskriptor for %s=%d\n",
					       lockFileName,lockFileDeskriptor);
                            strcpy(masterStr,"Master");
                            strcpy(slaveStr,"Slave"); 
                            if(_printer_flag) {
			      strcat(masterStr," with printer");
			      strcat(slaveStr, " with printer");
			    }                           
			    masterTesting(); /* Albert */
			  }
			if(_message_broadcast_flag)                              /* Albert1 */
			  {
			    FILE *fpL, *fpI;
			    strcpy(messBroadcastLockFileName,psetup.configFile);
			    strcat(messBroadcastLockFileName,".MESSLOCK");
 			    strcpy(messBroadcastInfoFileName,psetup.configFile);
			    strcat(messBroadcastInfoFileName,".MESS");        
			    if ( (!(fpL=fopen(messBroadcastLockFileName,"a"))) 
				 || (!(fpI=fopen(messBroadcastInfoFileName,"a"))) )
			      {
				perror("Can't open messBroadcast file for w");
				exit(1);
			      }
                              fclose(fpL);
			      fclose(fpI);
			    if((messBroadcastDeskriptor=
				open(messBroadcastLockFileName,O_RDWR,0644)) == 0)
			      { 
				perror("Can't open messBroadcast file for rw");
				exit(1);
			      }
			    if (DEBUG) fprintf(stderr,"INIT: deskriptor for %s=%d\n",
					  messBroadcastLockFileName,messBroadcastDeskriptor);
                          broadcastMessTesting(widget);
			  signal(SIGINT,broadcastMess_exit_quit); 
			  }

#ifndef WIN32
			if( _printer_flag)
			  {
			    struct msqid_ds buf;  
			    printerMsgQId = msgget (printerMsgQKey, 0600|IPC_CREAT);
			    if(printerMsgQId == -1) {perror("printer:msgQ_create"); exit(1);}
			    else {
			      if(DEBUG) fprintf(stderr,"printerMsgQ with key=%d is OK\n",
						printerMsgQKey);
			      if (msgctl(printerMsgQId,IPC_STAT,&buf) != 1)
				{
				  if(DEBUG)fprintf(stderr,"printer:o=%d.%d,perm=%04o,max byte=%d\n",
						   buf.msg_perm.uid,buf.msg_perm.gid,
						   buf.msg_perm.mode,
						   buf.msg_qbytes);
				  if(DEBUG) fprintf(stderr,"printer:%d msgs = %d bytes on queue\n",
						    buf.msg_qnum, buf.msg_cbytes);
				}
			      else {perror("printer:msgctl()");  exit(1);}
			    }
			  }
			if( _DB_call_flag)
			  {
			    struct msqid_ds buf;  
			    DBMsgQId = msgget (DBMsgQKey, 0600|IPC_CREAT);
			    if(DBMsgQId == -1) {perror("DB:msgQ_create"); exit(1);}
			    else {
			      if(DEBUG) fprintf(stderr,"DB:msgQ with key=%d is OK\n",
						DBMsgQKey);
			      if (msgctl(DBMsgQId,IPC_STAT,&buf) != 1)
				{
				  if(DEBUG)fprintf(stderr,"DB:o=%d.%d,perm=%04o,max byte=%d\n",
						   buf.msg_perm.uid,buf.msg_perm.gid,
						   buf.msg_perm.mode,
						   buf.msg_qbytes);
				  if(DEBUG) fprintf(stderr,"DB:%d msgs = %d bytes on queue\n",
						    buf.msg_qnum, buf.msg_cbytes);
				}
			      else {perror("DB:msgctl()");  exit(1);}
			    }
			  }
			
#endif



			break;

		case FILE_ALARMLOG:
			if (fo) alLogSetupAlarmFile(filename);
			strcpy(psetup.logFile,filename);
			if (fl) fclose(fl); /* RO-flag. Albert */
			if(_read_only_flag)  fl = fopen(psetup.logFile,"r");
			else if((_time_flag)||(_lock_flag))  fl = fopen(psetup.logFile,"a");
			else fl = fopen(psetup.logFile,"r+");
                        if (!fl) perror("CAN'T OPEN LOG FILE"); /* Albert1 */
  			/*---------------- 
			                    if (alarmLogFileMaxRecords && alarmLogFileEndStringLength) {
			                        fseek(fl,0,SEEK_SET);
			                        while (fgets(str,sizeof(str),fl)) {
			                            if(strncmp(alarmLogFileEndString,str,
			                                     alarmLogFileEndStringLength)==0){
			                                fseek(fl,-strlen(str),SEEK_CUR);
			                                break;
			                            }
			                        }
			                        alarmLogFileOffsetBytes = ftell(fl);
			                    } else {
			                         fseek(fl,0,SEEK_END);
			                    }
			----------------*/
			break;

		case FILE_OPMOD:
			if (fo) alLogSetupOpmodFile(filename);
			strcpy(psetup.opModFile,filename);
			if (fo) fclose(fo);
			if(!_read_only_flag) fo=fopen(psetup.opModFile,"a");
			/* RO-option. Albert */
			else fo=fopen(psetup.opModFile,"r");
                        if (!fo) perror("CAN'T OPEN OP FILE"); /* Albert1 */
			break;

		case FILE_SAVEAS:
		case FILE_SAVE:
			filename_dup = malloc(strlen(filename)+1);
			if ( filename_dup ) strcpy(filename_dup,filename);
			saveConfigFile_callback(widget,filename_dup,(void *)NULL);
			free(filename_dup);
			break;

		case FILE_PRINT:
			tt = fopen(filename,"w");
			alPrintConfig(tt,area->pmainGroup);
			fclose(tt);
			break;

		case FILE_CONFIG_INSERT:
			editInsertFile(filename,area);
			break;
		}
	}
}

/******************************************************
  saveConfigFile_callback
******************************************************/
static void saveConfigFile_callback(Widget widget,char *filename,
XmAnyCallbackStruct *cbs)
{
	ALINK   *area;

	XtVaGetValues(widget, XmNuserData, &area, NULL);

	alLogSetupSaveConfigFile(filename);
	/*
	     strcpy(psetup.saveFile,filename);
	*/

	if ( DEBUG == 1 )
		printf("\nSaving Config File to %s \n", filename);

	alWriteConfig(filename,area->pmainGroup);
	/* unmanage the warning dialog */
	XtUnmanageChild(widget);
	/* unmanage the fileSelection dialog */
	createFileDialog(0,0,0,0,0,0,0,0,0);
}

/******************************************************
  getCommandLineParms
******************************************************/
static int getCommandLineParms(int argc, char** argv)
{
	int i,j;
	int finished=0;
	int parm_error=0;

	alarmLogFileMaxRecords=commandLine.alarmLogFileMaxRecords=2000; /* Albert1 */
        
	for(i=1;i<argc && !parm_error;i++)
	{
		for(j=0;!finished && !parm_error && ptable[j].parm;j++)
		{
			if(strncmp(ptable[j].parm,argv[i],ptable[j].len)==0)
			{
				switch(ptable[j].id)
				{

				case PARM_DEBUG:
					DEBUG = TRUE;
					finished=1;
					break;
				case PARM_HELP:
					printUsage(argv[0]);
					finished=1;
					break;
				case PARM_ACT:
					strcpy(programName,"act");
					programId = ACT;
					finished=1;
					break;
				case PARM_SILENT:
					psetup.silenceForever=TRUE;
					finished=1;
					break;
				case PARM_ALL_FILES_DIR:
					if(++i>=argc) parm_error=1;
					else
					{
						if(argv[i][0]=='-') parm_error=1;
						else
						{
							commandLine.configDir=argv[i];
							finished=1;
						}
					}
					break;
				case PARM_LOG_DIR:
					if(++i>=argc) parm_error=1;
					else
					{
						if(argv[i][0]=='-') parm_error=1;
						else
						{
							commandLine.logDir=argv[i];
							finished=1;
						}
					}
					break;
				case PARM_ALARM_LOG_FILE:
					if(++i>=argc) parm_error=1;
					else
					{
						if(argv[i][0]=='-') parm_error=1;
						else
						{
							commandLine.logFile=argv[i];
							finished=1;
						}
					}
					break;
				case PARM_OPMOD_LOG_FILE:
					if(++i>=argc) parm_error=1;
					else
					{
						if(argv[i][0]=='-') parm_error=1;
						else
						{
							commandLine.opModFile=argv[i];
							finished=1;
						}
					}
					break;
				case PARM_ALARM_LOG_MAX:
					if(++i>=argc) parm_error=1;
					else
					{
						if(argv[i][0]=='-') parm_error=1;
						else
						{
							commandLine.alarmLogFileMaxRecords=atoi(argv[i]);
							if( (!commandLine.alarmLogFileMaxRecords)&&(strcmp(argv[i],"0") )) parm_error=1;
							alarmLogFileMaxRecords=commandLine.alarmLogFileMaxRecords;
							finished=1;
						}
					}
					break;
				case PARM_PRINTER: /* Printer parameters. Albert */
					if(++i>=argc) parm_error=1;
					else
					{
						if(argv[i][0]=='-') parm_error=1;
						else
						{
                                                        printerMsgQKey=atoi(argv[i]);
                                                        if(!printerMsgQKey) parm_error=1;
                                                        _printer_flag=1;
							finished=1;
						}
					}
					break;
				case PARM_DATED:
					_time_flag=1; /* Dated-option. Albert */
					finished=1;
					break;
				case PARM_PASSIVE:
					_read_only_flag=1; /* Passive-option. Albert */
					_passive_flag=1;
					finished=1;
					break;
				case PARM_READONLY:
					_read_only_flag=1;  /* RO-option. Albert */
					finished=1;
					break;
				case PARM_LOCK:
					_lock_flag=1;  /* locking system. Albert */
					finished=1;
					break;
				case PARM_DATABASE:   /* DATABASE-option. Albert */
					if(++i>=argc) parm_error=1;
					else
					{
						if(argv[i][0]=='-') parm_error=1;
						else
						{
                                                        DBMsgQKey=atoi(argv[i]);
                                                        if(!DBMsgQKey) parm_error=1;
                                                        _DB_call_flag=1;
							finished=1;
						}
					}
					break;
				case PARM_MESSAGE_BROADCAST:
					_message_broadcast_flag=1;/* Mess. Broadcast Albert */
					finished=1;
					break;
				default:
					parm_error=1;
					break;
				}
			}
		}
		if(ptable[j].parm==NULL)
		{
			if(i+1==argc)
			{
				if(argv[i][0]=='-') parm_error=1;
				else
				{
					commandLine.configFile=argv[i];
					finished=1;
				}
			}
			else parm_error=1;
		} else {
			finished=0;
		}
	}


if(commandLine.alarmLogFileMaxRecords&&_lock_flag)
  {
  fprintf(stderr,"use -m 0 option together with -L\n");
  parm_error=1;
  }

if(_printer_flag&&!_lock_flag)
  {
  fprintf(stderr,"use -P together with -L\n");
  parm_error=1;
  }

if(_DB_call_flag&&!_lock_flag)
  {
  fprintf(stderr,"use -O together with -L\n");
  parm_error=1;
  }

	if(parm_error)
	{
		printUsage(argv[0]);
		return 1;
	}
	return 0;
}

/******************************************************
  printUsage
******************************************************/
static void printUsage(char *pgm)
{
	fprintf(stderr,
	    "\nusage: %s [-csDSTLOB] [-f filedir] [-l logdir] [-a alarmlogfile] [-o opmodlogfile] [-m alarmlogmaxrecords] [-P key] [Xoptions] [configfile] \n",
	    pgm);
	fprintf(stderr,"\n\tconfigfile\tAlarm configuration filename\n");
	fprintf(stderr,"\n\t-c\t\tAlarm Configuration Tool mode\n");
	fprintf(stderr,"\n\t-f filedir\tDirectory for all files\n");
	fprintf(stderr,"\n\t-l logdir\tDirectory for log files\n");
	fprintf(stderr,"\n\t-a alarmlogfile\tAlarm log filename\n");
	fprintf(stderr,"\n\t-o opmodlogfile\tOpMod log filename\n");
	fprintf(stderr,"\n\t-m maxrecords\talarm log file max records (default 2000)\n");
	fprintf(stderr,"\n\t-s\t\tSilent mode (no alarm beeping)\n");
	fprintf(stderr,"\n\t-D\t\tDisable Writing\n");
	fprintf(stderr,"\n\t-S\t\tPassive Mode\n");
	fprintf(stderr,"\n\t-T\t\tAlarmLogDated\n");
	fprintf(stderr,"\n\t-P key\tPrint to TCP printer\n");
	fprintf(stderr,"\n\t-L\t\tLocking system\n");
	fprintf(stderr,"\n\t-O\t\tDatabase call\n");
	fprintf(stderr,"\n\t-B\t\tMessage BroadcastSystem\n");
	exit(1);
}


/******************************************************
  fileSetupInit
******************************************************/
void fileSetupInit( widget, argc, argv)
Widget widget;
int argc;
char *argv[];
{
	int    len;
	char   configFile[NAMEDEFAULT_SIZE];
	char   logFile[NAMEDEFAULT_SIZE];
	char   opModFile[NAMEDEFAULT_SIZE];
	char   *name = NULL;
	programId = ALH;
	programName = (char *)calloc(1,4);
	strcpy(programName,"alh");


	/* get optional command line parameters */
	getCommandLineParms(argc,argv);

	if (DEBUG) printf("programName=%s\n",programName);

	if (commandLine.configDir)
		psetup.configDir=commandLine.configDir;
		else
		psetup.configDir=getenv("ALARMHANDLER");

	if (commandLine.logDir)
		psetup.logDir=commandLine.logDir;

	if (psetup.configDir && !psetup.logDir)
		psetup.logDir=psetup.configDir;

	/* ----- initialize and setup opMod file ----- */
	if (psetup.logDir) {
		strncpy(psetup.opModFile,psetup.logDir,NAMEDEFAULT_SIZE-1);
		strcat(psetup.opModFile,"/");
	}
	if (commandLine.opModFile) {
		strncpy(opModFile,commandLine.opModFile,NAMEDEFAULT_SIZE-1);
	} else {
		strcpy(opModFile,DEFAULT_OPMOD);
	}
	name = opModFile;
	if ( name[0] == '/' || (name[0] == '.' && name[1] == '.') ||
	    (name[0] == '.' && name[1] == '/')) {
		strncpy(psetup.opModFile,opModFile,NAMEDEFAULT_SIZE);
	} else {
		len = strlen(psetup.opModFile);
		strncat(psetup.opModFile,opModFile,NAMEDEFAULT_SIZE-len);
	}
	if (DEBUG == 1 ) printf("\nOpMod File is %s \n", psetup.opModFile);
	fileSetup(psetup.opModFile,NULL,FILE_OPMOD,programId,widget);

	/* ----- initialize and setup alarm log file ----- */
	if (psetup.logDir) {
		strncpy(psetup.logFile,psetup.logDir,NAMEDEFAULT_SIZE-1);
		strcat(psetup.logFile,"/");
	}
	if (commandLine.logFile) {
		strncpy(logFile,commandLine.logFile,NAMEDEFAULT_SIZE-1);
	} else {
		strcpy(logFile,DEFAULT_ALARM);
	}
	name = logFile;
	if ( name[0] == '/' || (name[0] == '.' && name[1] == '.') ||
	    (name[0] == '.' && name[1] == '/')) {
		strncpy(psetup.logFile,logFile,NAMEDEFAULT_SIZE);
	} else {
		len = strlen(psetup.logFile);
		strncat(psetup.logFile,logFile,NAMEDEFAULT_SIZE-len);
	}
	if (DEBUG == 1 ) printf("\nAlarmLog File is %s \n", psetup.logFile);
	fileSetup(psetup.logFile,NULL,FILE_ALARMLOG,programId,widget);

	/* ----- initialize and setup config file ----- */
	if (psetup.configDir) {
		strncpy(psetup.configFile,psetup.configDir,NAMEDEFAULT_SIZE-1);
		strcat(psetup.configFile,"/");
	}
	if (commandLine.configFile) {
		strncpy(configFile,commandLine.configFile,NAMEDEFAULT_SIZE-1);
	} else {
		strcpy(configFile,DEFAULT_CONFIG);
	}
	name = configFile;
	if ( name[0] == '/' || (name[0] == '.' && name[1] == '.') ||
	    (name[0] == '.' && name[1] == '/')) {
		strncpy(psetup.configFile,configFile,NAMEDEFAULT_SIZE);
	} else {
		len = strlen(psetup.configFile);
		strncat(psetup.configFile,configFile,NAMEDEFAULT_SIZE-len);
	}
	if (DEBUG == 1 ) printf("\nConfig File is %s \n", psetup.configFile);
	fileSetup(psetup.configFile,NULL,FILE_CONFIG,programId,widget);


}
/* *******************************Albert1: ************************************* */
void masterTesting()
{
#ifndef WIN32
        if ( lockf(lockFileDeskriptor, F_TLOCK, 0L) < 0 ) {
	  if ((errno == EAGAIN || errno == EACCES )) {
	      masterFlag=0;
	      if(DEBUG) fprintf(stderr,"I'm slave;lockFileDeskriptor=%d\n",lockFileDeskriptor);
	      XtVaSetValues(blinkToplevel,XmNtitle,slaveStr,NULL);
	  }
	  else {
	    perror("lockf Error!!!!"); /* Albert1 exit ?????? */
	  }
	}
	else 
	  {
	    masterFlag=1;
	    if(DEBUG) fprintf(stderr,"I'm master;lockFileDeskriptor=%d\n",lockFileDeskriptor);
            XtVaSetValues(blinkToplevel,XmNtitle,masterStr,NULL);
	  }
	
	lockTimeoutId = XtAppAddTimeOut(appContext, lockDelay,(XtTimerCallbackProc)masterTesting , NULL);

#endif
}

void broadcastMessTesting(Widget w)
{
FILE *fp;
static char messID[30];
char firstLine[30];
char messBuff[500];
char buff[250];
char *blank;
int notsave_time;
void notsaveProc();

broadcastMessTimeoutId=XtAppAddTimeOut(appContext,broadcastMessDelay,(XtTimerCallbackProc)broadcastMessTesting,w);
if ( (fp=fopen(messBroadcastInfoFileName,"r")) == NULL )
  {
    perror("broadcastMessTesting: can't open messBroadcastInfoFileName!!!!");
    return;
  }
if (fgets(firstLine,32,fp)==NULL) {fclose(fp); return;}

if(strcmp(firstLine,messID) == 0) {fclose(fp); return;}
strcpy(messID,firstLine);
memset(messBuff,0,250);
fgets (messBuff, 250, fp);  /* Mess */
fgets(buff,250,fp);
strcat(messBuff,buff);      /* Date */
fgets(buff,250,fp);
strcat(messBuff,buff);      /* From */ 
fclose(fp);

if(!amIsender) 
  {
    createDialog(w,XmDIALOG_INFORMATION,"\nBROADCAST MESSAGE:\n",messBuff);
    XBell(XtDisplay(w),50);
  }

if ( (blank=strchr( (const char *) messBuff, ' ')) == NULL ) return;
if(strncmp(blank+1,rebootString,strlen(rebootString)-1 )==0) {
  *blank=0;
  notsave_time=atoi(messBuff);
  if(DEBUG) fprintf(stderr,"notsave_time=%d",notsave_time);
  if( (notsave_time <= 0) || (notsave_time > max_not_save_time) ) return;
  notsave=1;
  XtAppAddTimeOut(appContext,notsave_time*1000*60 ,(XtTimerCallbackProc) notsaveProc, w);

}
}

void notsaveProc(Widget w) 
{ 
time_t time_tmp;

notsave=1;
time_tmp=time(0L);
createDialog(w,XmDIALOG_MESSAGE,ctime(&time_tmp),"We start save alarmLog again \n");
}

void broadcastMess_exit_quit(int unused)
{
exit_quit(NULL,NULL,NULL);
signal(SIGINT,broadcastMess_exit_quit);
}
/* *******************************End of Albert1 ************************************* */





