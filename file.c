/* file.c */

/************************DESCRIPTION***********************************
  File contains file handling routines
**********************************************************************/

static char *sccsId = "@(#) $Id$";
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#ifndef WIN32
/* WIN32 does not have dirent.h used by opendir, closedir */
#include <sys/msg.h>  
#include <sys/stat.h>  
#include <pwd.h>
#include <unistd.h>  
#include <dirent.h>
#include <fcntl.h>
#else
#include <process.h>
#endif
#include <ctype.h>

#include <Xm/Xm.h>

#include "alh.h"
#include "version.h"
#include "alLib.h"
#include "axArea.h"
#include "ax.h"

/* default  file names */
#define DEFAULT_CONFIG  "ALH-default.alhConfig"
#define DEFAULT_ALARM   "ALH-default.alhAlarm"
#define DEFAULT_OPMOD   "ALH-default.alhOpmod"

struct UserInfo {
char *loginid;
char *real_world_name;
char *myhostname;
char *displayName;
};

struct UserInfo userID; 

int _no_error_popup=0;       /* No popup window. Error messages logged to opMod  file. */

int _global_flag=0;          /* Global execution mode. */
int _transients_flag=0;      /* Do ca_put of config file value for ackt */

int _main_window_flag=0;     /* Flag: Start with main window */
                             /* Default display filter function */
int (*default_display_filter)(GCLINK *) = alFilterAll;

int _read_only_flag=0;       /* Read-only flag.          Albert    */
int _passive_flag=0;         /* Passive flag.            Albert    */

int _printer_flag=0;         /* Printer flag.            Albert    */
int  printerMsgQKey;         /* Network printer MsgQKey. Albert    */
int  printerMsgQId;          /* Network printer MsgQId.  Albert    */
 
int _time_flag=0;            /* Dated flag.              Albert    */
int tm_day_old;              /* Day-variable for dated.  Albert    */

int _DB_call_flag=0;         /* Database(Oracle...) call. Albert   */
int  DBMsgQKey;              /* Database MsgQKey.         Albert   */
int  DBMsgQId;               /* Database MsgQId.          Albert   */

int _message_broadcast_flag=0;         /* MessBroadcast Sys Albert */
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
unsigned long broadcastMessDelay=2000; /*(msec) periodic mess testing. Albert */   

int _lock_flag=0;                /* Flag for locking. Albert                  */
char lockFileName[250];          /* FN for lock file. Albert                  */
int lockFileDeskriptor;          /* FD for lock file. Albert                  */
unsigned long lockDelay=1000;    /* (msec) periodical masterStatus testing.   */
int masterFlag=1;                /* am I master for write operations? Albert  */  
void masterTesting();            /* periodical calback of masterStatus testing*/
extern Widget blinkToplevel;     /* for locking status marking                */
char masterStr[64],slaveStr[64]; /* titles of Master/Slave +- printer/database*/
XtIntervalId lockTimeoutId=NULL;   
extern XFontStruct *font_info;
extern char alhVersionString[60];
 
#ifdef CMLOG
				/* CMLOG flags & variables */
int use_CMLOG_alarm = 0;
int use_CMLOG_opmod = 0;
#endif

extern int DEBUG;

extern int alarmLogFileMaxRecords;     /* alarm log file maximum # records */
extern int alarmLogFileOffsetBytes;    /* alarm log file current offset in bytes */
extern char alarmLogFileEndString[];   /* alarm log file end of data string */
extern int alarmLogFileEndStringLength;/* alarm log file end data string len*/

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

#define PARM_DEBUG					0
#define PARM_ACT					1
#define PARM_ALL_FILES_DIR			2
#define PARM_LOG_DIR				3
#define PARM_ALARM_LOG_FILE			4
#define PARM_OPMOD_LOG_FILE			5
#define PARM_ALARM_LOG_MAX			6
#define PARM_DATABASE				7
#define PARM_PRINTER				8
#define PARM_DATED					9
#define PARM_PASSIVE				10
#define PARM_READONLY				11
#define PARM_MESSAGE_BROADCAST		12
#define PARM_SILENT					13
#define PARM_LOCK					14
#define PARM_HELP					15
#define PARM_ALARM_LOG_CMLOG		16
#define PARM_OPMOD_LOG_CMLOG		17
#define PARM_GLOBAL					18
#define PARM_CAPUT_ACK_TRANSIENTS	19
#define PARM_VERSION				20
#define PARM_NO_ERROR_POPUP			21
#define PARM_MAIN_WINDOW			22
#define PARM_ALARM_FILTER			23

struct parm_data
{
	char* parm;
	int len;
	int id;
};

/* order of elements matters: long before short to prevent ambiguity */

static struct parm_data ptable[] = {
#ifdef CMLOG
		{ "-aCM", 4,			PARM_ALARM_LOG_CMLOG },
#endif
		{ "-a", 2,				PARM_ALARM_LOG_FILE },
 		{ "-B", 2,				PARM_MESSAGE_BROADCAST },	/* Albert */
		{ "-c", 2,				PARM_ACT },
 		{ "-caputackt", 10,		PARM_CAPUT_ACK_TRANSIENTS },
		{ "-D", 2,				PARM_READONLY },
		{ "-debug", 6 ,			PARM_DEBUG },
		{ "-filter", 7,			PARM_ALARM_FILTER },
		{ "-f", 2,				PARM_ALL_FILES_DIR },
 		{ "-global", 7,			PARM_GLOBAL },
		{ "-help", 5,			PARM_HELP },
		{ "-L", 2,				PARM_LOCK },				/* Albert */
		{ "-l", 2,				PARM_LOG_DIR },
		{ "-mainwindow", 11,	PARM_MAIN_WINDOW },
		{ "-m", 2,				PARM_ALARM_LOG_MAX },
		{ "-noerrorpopup", 13,	PARM_NO_ERROR_POPUP },
		{ "-O", 2,				PARM_DATABASE },			/* Albert */
#ifdef CMLOG
		{ "-oCM", 4,			PARM_OPMOD_LOG_CMLOG },
#endif
		{ "-o", 2,				PARM_OPMOD_LOG_FILE },
		{ "-P", 2,				PARM_PRINTER },
		{ "-S", 2,				PARM_PASSIVE },
		{ "-s", 2,				PARM_SILENT },
		{ "-T", 2,				PARM_DATED },
		{ "-v", 2,				PARM_VERSION },
		{ "-version", 8,		PARM_VERSION },
 		{ NULL,		-1,     -1 }};

/* forward declarations */
static void saveConfigFile_callback(Widget widget,char *filename,
XmAnyCallbackStruct *cbs);
static void printUsage(char *);
static void fileSetup(char *filename,ALINK *area,int fileType,int programId,
Widget widget);
static int checkFilename(char *filename,int fileType);
static int getCommandLineParms(int argc, char** argv);
int getUserInfo();

/***************************************************
 exit and quit application
***************************************************/
void exit_quit(Widget w, XtPointer clientdata, XtPointer calldata)
{
	struct subWindow  *subWindow;
	ALINK *area = (ALINK *)clientdata;
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
	if (area){
		if (area->treeWindow){
            subWindow=area->treeWindow;
      		if (subWindow->lines) free(subWindow->lines);
     		free(area->treeWindow);
    	}
		if (area->groupWindow){
            subWindow=area->groupWindow;
      		if (subWindow->lines) free(subWindow->lines);
      		free(area->groupWindow);
    	}
		if (area->propWindow) free(area->propWindow);
		if (area->forceMaskWindow) free(area->forceMaskWindow);
		if (area->forcePVWindow) free(area->forcePVWindow);
		if (area->maskWindow) free(area->maskWindow);
		if (area->pmainGroup) free(area->pmainGroup);
		free(area);
	}
	XtDestroyWidget(topLevelShell);
	XFreeFont(display,font_info);
#ifndef WIN32
	if(_lock_flag)  {
	  lockf(lockFileDeskriptor,F_ULOCK, 0L); /* Albert */
	  if (lockTimeoutId) {
	    XtRemoveTimeOut(lockTimeoutId);
	  }
	if(_message_broadcast_flag)  {
	  lockf(messBroadcastDeskriptor, F_ULOCK, 0L); /* Albert */
	  if (broadcastMessTimeoutId) {
	    XtRemoveTimeOut(broadcastMessTimeoutId);
	  }
	}

	} 
#endif

#ifdef CMLOG
	if (use_CMLOG_alarm || use_CMLOG_opmod)	alCMLOGdisconnect();
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
		else {
		  tt = fopen(filename,"r");
		  if(!tt) 
		    { 
		      strcpy(filename,"/tmp/AlhDisableWriting");
		      chmod("/tmp/AlhDisableWriting",0777); 
		      tt = fopen(filename,"w");
		      if (!tt) {
			fprintf(stderr,"can't read OPMOD or ALARMLOG and /tmp directory \n");
			exit(1);
		      }
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
			break;

		case 4:
			/*createDialog(fileSelectionBox,XmDIALOG_ERROR,filename," write error.");*/
			fatalErrMsg("Write error for file %s.\n",filename);
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
#ifdef CMLOG
			if (use_CMLOG_alarm || use_CMLOG_opmod) alCMLOGconnect();
#endif

			setupConfig(filename,programId,area);
			if(_lock_flag)                              /* Albert */
			  {
			    FILE *fp;
			    strcpy(lockFileName,psetup.configFile);
			    strcat(lockFileName,".LOCK");
			    if (!(fp=fopen(lockFileName,"a")))
			      {
				perror("Can't open locking file for a");
				exit(1);
			      }
                              fclose(fp);     
#ifndef WIN32
			    if((lockFileDeskriptor=open(lockFileName,O_RDWR,0644)) == 0)
			      { 
				perror("Can't open locking file for rw");
				exit(1);
			      }
#endif
			    if (DEBUG) fprintf(stderr,"INIT: deskriptor for %s=%d\n",
					       lockFileName,lockFileDeskriptor);
                            strcpy(masterStr,"Master");
                            strcpy(slaveStr, "Slave"); 
                            if(_printer_flag) {
			      strcat(masterStr,"+printer");
			      strcat(slaveStr, "+printer");
			    }
                              if(_DB_call_flag) {
			      strcat(masterStr,"+Oracle");
			      strcat(slaveStr, "+Oracle");
			    }
                              if(_message_broadcast_flag) {
			      strcat(masterStr,"+Message");
			      strcat(slaveStr, "+Message");
			    }

			    masterTesting(); /* Albert */
			  }
			if(_message_broadcast_flag) /* Albert */
			  {
			    FILE *fpL, *fpI;
			    strcpy(messBroadcastLockFileName,psetup.configFile);
			    strcat(messBroadcastLockFileName,".MESSLOCK");
 			    strcpy(messBroadcastInfoFileName,psetup.configFile);
			    strcat(messBroadcastInfoFileName,".MESS");        
			    if ( (!(fpL=fopen(messBroadcastLockFileName,"a"))) 
				 || (!(fpI=fopen(messBroadcastInfoFileName,"a"))) )
			      {
				perror("Can't open messBroadcast file for a");
				exit(1);
			      }
                              fclose(fpL);
			      fclose(fpI);
#ifndef WIN32
			    if((messBroadcastDeskriptor=
				open(messBroadcastLockFileName,O_RDWR,0644)) == 0)
			      { 
				perror("Can't open messBroadcast file for rw");
				exit(1);
			      }
#endif
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
                        if (!fl) perror("CAN'T OPEN LOG FILE"); /* Albert */
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
                        if (!fo) perror("CAN'T OPEN OP FILE"); /* Albert */
			break;

		case FILE_SAVEAS:
		case FILE_SAVE:
			filename_dup = malloc(strlen(filename)+1);
			if ( filename_dup ) strcpy(filename_dup,filename);
			saveConfigFile_callback(widget,filename_dup,(void *)NULL);
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
	/* Free the filename string copy */
	free(filename);
}

/******************************************************
  getCommandLineParms
******************************************************/
static int getCommandLineParms(int argc, char** argv)
{
	int i,j;
	int finished=0;
	int parm_error=0;

	alarmLogFileMaxRecords=commandLine.alarmLogFileMaxRecords=2000; /* Albert */
        
	for(i=1;i<argc && !parm_error;i++)
	{
		for(j=0;!finished && !parm_error && ptable[j].parm;j++)
		{
			if(strncmp(ptable[j].parm,argv[i],Mmax(ptable[j].len,strlen(argv[i])))==0)
			{
				switch(ptable[j].id)
				{

				case PARM_DEBUG:
					DEBUG = TRUE;
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
						if(argv[i][0]=='-') parm_error=2;
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
						if(argv[i][0]=='-') parm_error=2;
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
						if(argv[i][0]=='-') parm_error=2;
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
						if(argv[i][0]=='-') parm_error=2;
						else
						{
							commandLine.opModFile=argv[i];
							finished=1;
						}
					}
					break;
				case PARM_ALARM_FILTER:
					if(++i>=argc) parm_error=1;
					else
					{
					   if (argv[i][0] == 'a') {
						  default_display_filter = alFilterAlarmsOnly;
						  finished = 1;
					   } else if (argv[i][0] == 'u') {
						  default_display_filter = alFilterUnackAlarmsOnly;
						  finished = 1;
					   } else if (argv[i][0] == 'n') {
						  default_display_filter = alFilterAll;
						  finished = 1;
					   } else parm_error=2;
					}
					break;
#ifdef CMLOG
				case PARM_ALARM_LOG_CMLOG:
					use_CMLOG_alarm = 1;
					finished=1;
					break;
				case PARM_OPMOD_LOG_CMLOG:
					use_CMLOG_opmod = 1;
					finished=1;
					break;
#endif
				case PARM_ALARM_LOG_MAX:
					if(++i>=argc) parm_error=1;
					else
					{
						if(argv[i][0]=='-') parm_error=2;
						else
						{
							commandLine.alarmLogFileMaxRecords=atoi(argv[i]);
							if( (!commandLine.alarmLogFileMaxRecords)&&(strcmp(argv[i],"0") )) parm_error=2;
							alarmLogFileMaxRecords=commandLine.alarmLogFileMaxRecords;
							finished=1;
						}
					}
					break;
				case PARM_PRINTER: /* Printer parameters. Albert */
					if(++i>=argc) parm_error=1;
					else
					{
						if(argv[i][0]=='-') parm_error=2;
						else
						{
                                                        printerMsgQKey=atoi(argv[i]);
                                                        if(!printerMsgQKey) parm_error=2;
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
					/*_read_only_flag=1;*/ /* Passive-option. Albert */
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
						if(argv[i][0]=='-') parm_error=2;
						else
						{
                                                        DBMsgQKey=atoi(argv[i]);
                                                        if(!DBMsgQKey) parm_error=2;
                                                        _DB_call_flag=1;
							finished=1;
						}
					}
					break;
				case PARM_MESSAGE_BROADCAST:
					_message_broadcast_flag=1;/* Mess. Broadcast Albert */
					finished=1;
					break;
				case PARM_MAIN_WINDOW:
					_main_window_flag=1;
					finished=1;
					break;
				case PARM_NO_ERROR_POPUP:
					_no_error_popup=1;
					finished=1;
					break;
				case PARM_GLOBAL:
					_global_flag=1;
					finished=1;
					break;
				case PARM_CAPUT_ACK_TRANSIENTS:
					_transients_flag=1;
					finished=1;
					break;
				case PARM_HELP:
					printUsage(argv[0]);
					finished=1;
					break;
				case PARM_VERSION:
					fprintf(stderr,"%s\n",alhVersionString);
					exit(1);
					break;
				default:
					parm_error=1;
					break;
				}
			}
		}
		if (!finished && !parm_error)
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

	if (_lock_flag) commandLine.alarmLogFileMaxRecords = 0;

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

	if (parm_error == 1) {
  		fprintf(stderr,"\nInvalid command line option %s\n ", argv[i-1]);
		printUsage(argv[0]);
		return 1;
	}
	if (parm_error == 2) {
  		fprintf(stderr,"\nInvalid command line option %s %s\n ", argv[i-2], argv[i-1]);
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
	fprintf(stderr,"usage: %s [OPTIONS] [Xoptions] [configfile]\n", pgm);
	fprintf(stderr,"where:\n");
	fprintf(stderr,"  configfile       Alarm configuration filename ["DEFAULT_CONFIG"]\n");
	fprintf(stderr,"OPTIONS:\n");
	fprintf(stderr,"  -a alarmlogfile  Alarm log filename ["DEFAULT_ALARM"]\n");
#ifdef CMLOG		
	fprintf(stderr,"  -aCM             Alarm log using CMLOG\n");
#endif			
	fprintf(stderr,"  -B               Message Broadcast System\n");
	fprintf(stderr,"  -c               Alarm Configuration Tool mode\n");
	fprintf(stderr,"  -caputackt       Caput config file ackt settings to channels on startup\n");
	fprintf(stderr,"                     (if global and active)\n");
	fprintf(stderr,"  -D               Disable alarm and opmod log writing\n");
	fprintf(stderr,"  -debug           Debug output\n");
	fprintf(stderr,"  -f filedir       Directory for config files [.]\n");
	fprintf(stderr,"  -filter f-opt    Set alarm display filter with f-opt being one of [no]\n");
	fprintf(stderr,"                     n[o]:     no filter\n");
	fprintf(stderr,"                     a[ctive]: show only active alarms\n");
	fprintf(stderr,"                     u[nack]:  show only unacknowledged alarms\n");
	fprintf(stderr,"  -global          Global mode (acks and ackt fields) \n");
	fprintf(stderr,"  -help            Print usage\n");
	fprintf(stderr,"  -L               Locking system\n");
	fprintf(stderr,"  -l logdir        Directory for log files [.]\n");
	fprintf(stderr,"  -m maxrecords    Alarm log file max records [2000]\n");
	fprintf(stderr,"  -mainwindow      Start with main window\n");
	fprintf(stderr,"  -noerrorpopup    Do not display error popup window (errors are logged).\n");
	fprintf(stderr,"  -O key           Database call\n");
	fprintf(stderr,"  -o opmodlogfile  OpMod log filename ["DEFAULT_OPMOD"]\n");
#ifdef CMLOG		
	fprintf(stderr,"  -oCM             OpMod log using CMLOG\n");
#endif			
	fprintf(stderr,"  -P key           Print to TCP printer\n");
	fprintf(stderr,"  -S               Passive (no caputs - acks field, ackt field, sevrpv)\n");
	fprintf(stderr,"  -s               Silent (no alarm beeping)\n");
	fprintf(stderr,"  -T               AlarmLogDated\n");
	fprintf(stderr,"  -v               Print version number\n");
	fprintf(stderr,"  -version         Print version number\n");
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

	getUserInfo(); /*Get info (usr,Fullusr,host,displ) about operator  Albert*/

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
		if (programId == ALH) strcpy(configFile,DEFAULT_CONFIG); /*Albert */
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
/* *******************************new code. Albert************************************* */
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
	    perror("lockf Error!!!!"); /* Albert exit ?????? */
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
  if(DEBUG) fprintf(stderr,"notsave_time=%d\n",notsave_time);
  if( (notsave_time <= 0) || (notsave_time > max_not_save_time) ) 
    {
      createDialog(w,XmDIALOG_INFORMATION,"\nNot_save failed\n","time is so big");
      return;
    }
  alLogNotSaveStart(notsave_time);

  notsave=1;
  XtAppAddTimeOut(appContext,notsave_time*1000*60 ,(XtTimerCallbackProc) notsaveProc, w);

}
}

void notsaveProc(Widget w) 
{ 
time_t time_tmp;
notsave=0;
time_tmp=time(0L);
alLogNotSaveFinish();
createDialog(w,XmDIALOG_MESSAGE,ctime(&time_tmp),"We start save alarmLog again \n");
}

void broadcastMess_exit_quit(int unused)
{
exit_quit(NULL,NULL,NULL);
/* signal(SIGINT,broadcastMess_exit_quit); Albert */
}

int getUserInfo()
{
static char myhostname[256];
static char loginid[16];        
static char real_world_name[128]; 
static char displayName[256];
int ret=0;

#ifndef WIN32
struct passwd *pp;              
int effective_uid;

    effective_uid = geteuid();
    if ((pp = getpwuid(effective_uid))) 
      {
	strcpy(loginid,pp->pw_name);
	strcpy(real_world_name,pp->pw_gecos);
      }
    else 
      {
	strcpy(loginid,"Unknoun_user");
	strcpy(real_world_name," ");
	ret=1;
      }
    if(gethostname(myhostname,256) != 0) 
      {
	strcpy(myhostname,"unknoun_host");
	ret=1;
      }
    if(display) strcpy(displayName,DisplayString(display));
    else 
      {
	strcpy(displayName,"unknown_display");
	ret=1;
      }
    userID.loginid=loginid;
    userID.real_world_name=real_world_name;
    userID.myhostname=myhostname;
    userID.displayName=displayName;
#endif
    return(ret);
}

/* *******************************End of new code. Albert ************************* */







