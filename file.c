/*
 $Log$
 Revision 1.17  1998/07/16 18:30:56  jba
 not error to start alh with no filename on command line.

 Revision 1.16  1998/06/22 18:42:14  jba
 Merged the new alh-options created at DESY MKS group:
  -D Disable Writing, -S Passive Mode, -T AlarmLogDated, -P Printing

 Revision 1.15  1998/06/22 17:49:29  jba
 Bug fixes for command line option handling.

 Revision 1.14  1998/06/02 19:40:51  evans
 Changed from using Fgmgr to using X to manage events and file
 descriptors.  (Fdmgr didn't work on WIN32.)  Uses XtAppMainLoop,
 XtAppAddInput, and XtAppAddTimeOut instead of Fdmgr routines.
 Updating areas is now in alCaUpdate, which is called every caDelay ms
 (currently 100 ms).  Added a general error message routine (errMsg)
 and an exception handler (alCAException).  Is working on Solaris and
 WIN32.

 Revision 1.13  1998/06/01 18:33:26  evans
 Modified the icon.

 Revision 1.12  1998/05/13 19:29:49  evans
 More WIN32 changes.

 Revision 1.11  1998/05/12 18:22:48  evans
 Initial changes for WIN32.

 Revision 1.10  1997/09/12 19:38:27  jba
 Bug fixes for Save and SaveAs.

 Revision 1.9  1997/04/17 18:01:14  jba
 Added calls to free allocated memory.

 Revision 1.8  1997/01/09 14:38:19  jba
 Added alarmLog circular file facility.

 Revision 1.7  1996/08/19 13:53:39  jba
 Minor usage and mask printed output changes.

 Revision 1.6  1995/11/13 22:31:29  jba
 Added beepseverity command, ansi changes and other changes.

 * Revision 1.5  1995/10/20  16:50:38  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 *
 * Revision 1.4  1995/05/30  16:04:02  jba
 * Removed references to ? as a valid input param since they do NOT work.
 *
 * Revision 1.3  1995/01/09  19:38:29  jba
 * For loop changed to while loop wo HP compile would work.
 *
 * Revision 1.2  1994/06/22  21:17:28  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)file.c	1.14\t2/3/94";

/* file.c 
 *
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
 * .01  02-16-93        jba     Added files for new user interface
 * .02  12-10-93        jba     Added new command line options and reorganized code
 * .03  02-06-94        jba     Fixed file handling when file open errors on startup
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

#define INCerrMdefh

#include <stdlib.h>
#include <stdio.h>
#ifndef WIN32
/* WIN32 does not have dirent.h used by opendir, closedir */
#include <dirent.h>
#endif
#include <ctype.h>

#include <Xm/Xm.h>

#include <alh.h>
#include <alLib.h>
#include <axArea.h>
#include <ax.h>

int _read_only_flag=0; /* Read-only flag. Albert */
int _passive_flag=0;   /* Passive flag. Albert */
int _tm_day_old;       /* Day-variable for dated. Albert */
int _printer_flag=0;   /* Printer flag. Albert */
  char printerHostname[120];
  int  printerPort;
  char printerColorModel[120];
int _time_flag=0;      /* Dated flag. Albert */

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
static struct command_line_data commandLine = { NULL,NULL,NULL,NULL,NULL,0};

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
	{ "-D",		2,	PARM_READONLY },
	{ "-help",	5,	PARM_HELP },
        { NULL,		-1,     -1 }
};

void saveConfigFile_callback(Widget widget,char *filename,XmAnyCallbackStruct *cbs);
void printUsage(char *);


/***************************************************
 exit and quit application
***************************************************/

void exit_quit(w, area, call_data)
     Widget          w;
     ALINK          *area;
     XmAnyCallbackStruct *call_data;
{
     GLINK *proot=0;

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
          alCaStop();

          /* delete all the subgroups of proot & free proot */
          alDeleteGroup(proot);

     }

     XtDestroyWidget(topLevelShell);
     exit(0);
}


/******************************************************
 shorten file name without path
******************************************************/

char *shortfile(name)
char *name;
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

int checkFilename(filename,fileType)
     char      *filename;
     int        fileType;
{
     FILE *tt;
     
#ifndef WIN32
     DIR *directory;
#endif
     
     if ( filename[0] == '\0') return 2;

     if ( DEBUG == 1 )
          printf("\nFilename is %s \n", filename);

#ifndef WIN32
     directory = opendir(filename);
     if (directory) {
          closedir(directory);
          return 1;
     }
#endif     

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
               tt = fopen(filename,"a");
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

void fileCancelCallback(widget, area, cbs)
     Widget widget;
     ALINK    *area;
     XmFileSelectionBoxCallbackStruct *cbs;
{
     area->managed = TRUE;
     XtUnmanageChild(widget);
}

/******************************************************
  fileSetupCallback
******************************************************/

void fileSetupCallback(widget, client_data, cbs)
     Widget widget;
     int    client_data;
     XmFileSelectionBoxCallbackStruct *cbs;
{
     char *filename;
     ALINK *area;
     int pgm;


     /* get the filename string */
     XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &filename);
     if ( DEBUG == 1 ) printf("\nFilename is %s \n", filename);

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

void fileSetup(filename,area,fileType,programId, widget)
     char *filename;
     ALINK *area;
     int    fileType;
     int    programId;
     Widget widget;
{
     int    error;
     char   fileTypeString[NAMEDEFAULT_SIZE];
     char   str[MAX_STRING_LENGTH];
     char  *dir=0;
     char  *pattern=0;
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
     _tm_day_old = tms->tm_mday;
     if ((fileType == FILE_ALARMLOG)&&(_time_flag)) 
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
                    createActionDialog(fileSelectionBox,XmDIALOG_WARNING, str ,
                         (XtCallbackProc)saveConfigFile_callback,
                         (XtPointer)filename,(XtPointer)area);
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
                    break;

               case FILE_ALARMLOG:
                    if (fo) alLogSetupAlarmFile(filename);
                    strcpy(psetup.logFile,filename);
                    if (fl) fclose(fl); /* RO-flag. Albert */
		    if(_read_only_flag)  fl = fopen(psetup.logFile,"r"); 
                    else if(_time_flag)  fl = fopen(psetup.logFile,"a+");
                    else                 fl = fopen(psetup.logFile,"r+");
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
		    else  fo=fopen(psetup.opModFile,"r");
                    break;

               case FILE_SAVEAS:
               case FILE_SAVE:
                    saveConfigFile_callback(widget,filename,(void *)NULL);
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

void saveConfigFile_callback(widget,filename,cbs)
     Widget widget;
     char                 *filename;
     XmAnyCallbackStruct *cbs;
{
     ALINK   *area;

     XtVaGetValues(widget, XmNuserData, &area, NULL);

     alLogSetupSaveConfigFile(filename);
/*
     strcpy(psetup.saveFile,filename);
*/
     alWriteConfig(filename,area->pmainGroup);
     /* unmanage the warning dialog */
     XtUnmanageChild(widget);
     /* unmanage the fileSelection dialog */
     createFileDialog(0,0,0,0,0,0,0,0,0);
}

/******************************************************
  getCommandLineParms
******************************************************/

int getCommandLineParms(int argc, char** argv)
{
	int i,j;
	int finished=0;
	int parm_error=0;
	char *p,*q;  /* for printer parameters. Albert */

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
							sscanf(argv[i],"%u",&commandLine.alarmLogFileMaxRecords);
							finished=1;
						}
					}
					break;
				case PARM_PRINTER: /* Printer parameters. Albert */
					if(++i>=argc) {
						parm_error=1;
						break;
					}
					if(argv[i][0]=='-') {
						parm_error=1;
						break;
					}
					if ( (p= strchr(argv[i],':')) == NULL) {
						fprintf(stderr,"%s - you must specify <printerHostname>:<portNumber>:[printerColorModel] for printer.\nPrinting will be disable\n",argv[i]); 
						parm_error=1;
						break;
					}
					*p=0; p++;
					strcpy(printerHostname,argv[i]);
					if ( (q= strchr(p,':')) != NULL) {
						*q=0;q++;
						printerPort=atoi(p);
						strcpy(printerColorModel,q);     
					}
					else {
						printerPort=atoi(p);
						strcpy(printerColorModel,"mono"); 
					}    
					if (printerPort== 0) {
						fprintf(stderr,"%s - is not number.\nPrinting will be disable\n",p);
						parm_error=1;
						break;
					}
					_printer_flag=1;   
					finished=1;
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
		}else {
	 		finished=0;
		}
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

void printUsage(char *pgm)
{
          fprintf(stderr,
          "\nusage: %s [-cdst] [-f filedir] [-l logdir] [-a alarmlogfile] [-o opmodlogfile] [-m alarmlogmaxrecords] [-P printerName:portNumber:<printerColorModel>] [Xoptions] [configfile] \n",
               pgm);
          fprintf(stderr,"\n\tconfigfile\tAlarm configuration filename\n");
          fprintf(stderr,"\n\t-c\t\tAlarm Configuration Tool mode\n");
          fprintf(stderr,"\n\t-f filedir\tDirectory for all files\n");
          fprintf(stderr,"\n\t-l logdir\tDirectory for log files\n");
          fprintf(stderr,"\n\t-a alarmlogfile\tAlarm log filename\n");
          fprintf(stderr,"\n\t-o opmodlogfile\tOpMod log filename\n");
          fprintf(stderr,"\n\t-m maxrecords\talarm log file max records (default 2000)\n");
          fprintf(stderr,"\n\t-D\t\tDisable Writing\n");
          fprintf(stderr,"\n\t-S\t\tPassive Mode\n");
          fprintf(stderr,"\n\t-T\t\tAlarmLogDated\n");
          fprintf(stderr,"\n\t-P Name:port:colorModel\tPrint to TCP printer(colorMod={mono,hp_color,...})\n");
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
     char *p,*q;  /* for printer parameters. Albert */

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
