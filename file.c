/*
 $Log$
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
#include <dirent.h>
#include <ctype.h>

#include <Xm/Xm.h>

#include <fdmgr.h>

#include <alh.h>
#include <alLib.h>
#include <axArea.h>
#include <ax.h>

extern int DEBUG;

extern int getopt();
extern char *optarg; /* needed for getopt() */
extern int optind;   /* needed for getopt() */

extern int alarmLogFileMaxRecords;  /* alarm log file maximum # records */
extern int alarmLogFileOffsetBytes; /* alarm log file current offset in bytes */
extern char alarmLogFileEndString[]; /* alarm log file end of data string */
extern int alarmLogFileEndStringLength; /* alarm log file end data string len*/

extern FILE *fl;		/* alarm log pointer */
extern FILE *fo;		/* opmod log pointer */
 

#define FDMGR_SEC_TIMEOUT   10
#define FDMGR_USEC_TIMEOUT  0
extern fdctx *pfdctx;

#ifdef __STDC__
void saveConfigFile_callback(Widget widget,char *filename,XmAnyCallbackStruct *cbs);
#else
void saveConfigFile_callback();
#endif

/*******************************************************************
  Routines defined in file.c

  Routine for file handling and alh exit

******************************************************************
-------------
|   PUBLIC  |
-------------
*
void exit_quit(w, flag, call_data)                Exit & quit ALH callback
  Widget          w;
  int             flag;
  XmAnyCallbackStruct *call_data;
*
void fileSetupCallback(widget, client_data, cbs)  New file,any type, handling
     Widget widget;
     int    client_data;
     XmFileSelectionBoxCallbackStruct *cbs;
*
void fileCancelCallback(widget, client_data, cbs)
     Widget widget;
     int    client_data;
     XmFileSelectionBoxCallbackStruct *cbs;
*
void fileSetup(filename,area,fileType,programId, widget)
     char *filename;
     ALINK *area;
     int    fileType;
     int    programId;
     Widget widget;
*
void fileSetupInit( widget, argc, argv)
     Widget widget;
     int argc;
     char *argv[];
*
--------------
|   PRIVATE  |
--------------
char *shortfile(name)                             Cut away the path from file name
     char *name;
*
int checkFilename(filename,fileType)              Check filename for validity
     char      *filename;
     int        fileType;
*
void saveConfigFile_callback(widget,filename,cbs) Save config file 
     Widget widget;
     char                 *filename;
     XmAnyCallbackStruct *cbs;
*
*

********************************************************************************/
 

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
     DIR *directory;

     if ( filename[0] == '\0') return 2;

     if ( DEBUG == 1 )
          printf("\nFilename is %s \n", filename);

     directory = opendir(filename);
     if (directory) {
          closedir(directory);
          return 1;
     }

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
     /* XtFree(filename); */

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

     error = checkFilename(filename,fileType);
     if (error){
          switch(fileType) {

               case FILE_CONFIG:
               case FILE_CONFIG_INSERT:
                    pattern = CONFIG_PATTERN;
                    dir = psetup.configDir;
                    if (programId == ALH) 
                         strcpy(fileTypeString,"Alarm Handler: Alarm Configuration File");
                    else
                         /* not error to start act with no filename */
                         if ( filename[0] == '\0' && fileType == FILE_CONFIG ) error = 0;
                         strcpy(fileTypeString,"Alarm Configuration Tool: Alarm Configuration File");
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
                    if (fl) fclose(fl);
                    fl = fopen(psetup.logFile,"r+");
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
                    fo = fopen(psetup.opModFile,"a");
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
  fileSetupInit
******************************************************/

void fileSetupInit( widget, argc, argv)
     Widget widget;
     int argc;
     char *argv[];
{
     int    input_error;
     int    c, len;
     char   configFile[NAMEDEFAULT_SIZE];
     char   logFile[NAMEDEFAULT_SIZE];
     char   opModFile[NAMEDEFAULT_SIZE];
     char   *name = NULL;
     static struct timeval timeout = {
          FDMGR_SEC_TIMEOUT, FDMGR_USEC_TIMEOUT};

     strcpy(configFile,"");
     strcpy(logFile,DEFAULT_ALARM);
     strcpy(opModFile,DEFAULT_OPMOD);

     programId = ALH;
     programName = (char *)calloc(1,4);
     strcpy(programName,"alh");
 
     /* set config file directory using environment variable ALARMHANDLER */
     psetup.configDir = (char *)getenv("ALARMHANDLER");
     if (psetup.configDir && !opendir(psetup.configDir)){
          createDialog(widget,XmDIALOG_WARNING,psetup.configDir,
               ": ALARMHANDLER directory not found");
     }

     /* get optional command line parameters */
     input_error = FALSE;
     while (!input_error && (c = getopt(argc, argv, "vcf:l:a:o:m:")) != -1)
     {
         switch (c) {
             case 'v': DEBUG = TRUE; break;
             case 'c':
                  strcpy(programName,"act");
                  programId = ACT;
                  break;
             case 'f':
                  psetup.configDir = optarg;
                  if (psetup.configDir && !opendir(psetup.configDir)){
                       createDialog(widget,XmDIALOG_WARNING,psetup.configDir,
                            ": Config directory not found");
                  }
                  strncpy(psetup.configFile,psetup.configDir,NAMEDEFAULT_SIZE);
                  strcat(psetup.configFile,"/");
                  if (!psetup.logDir){
                       psetup.logDir = psetup.configDir;
                       strncpy(psetup.logFile,psetup.configDir,NAMEDEFAULT_SIZE-1);
                       strcat(psetup.logFile,"/");
                       strncpy(psetup.opModFile,psetup.configDir,NAMEDEFAULT_SIZE-1);
                       strcat(psetup.opModFile,"/");
                  }
                  break;
             case 'l':
                  psetup.logDir = optarg;
                  if (psetup.logDir && !opendir(psetup.logDir)){
                       createDialog(widget,XmDIALOG_WARNING,psetup.logDir,
                            ": Config directory not found");
                  }
                  strncpy(psetup.logFile,psetup.logDir,NAMEDEFAULT_SIZE-1);
                  strcat(psetup.logFile,"/");
                  strncpy(psetup.opModFile,psetup.logDir,NAMEDEFAULT_SIZE-1);
                  strcat(psetup.opModFile,"/");
                  break;
             case 'a':
                  strncpy(logFile,optarg,NAMEDEFAULT_SIZE);
                  break;
             case 'o':
                  strncpy(opModFile,optarg,NAMEDEFAULT_SIZE);
                  break;
             case 'm':
                  sscanf(optarg,"%u",&alarmLogFileMaxRecords);
                  break;
             default: input_error = 1; break;
         }
     }
 

     /* ----- get filename from the command line ----- */
     if (optind < argc) strncpy(configFile,argv[optind],NAMEDEFAULT_SIZE);
     else if (programId == ALH) strcpy(configFile,DEFAULT_CONFIG);
     if (DEBUG == 1 && argc > 1) printf("\nConfig File is %s \n", configFile);
 
     if (optind+1 < argc) input_error = 1;

     if (input_error) {
          fprintf(stderr,
          "\nusage: %s [-c] [-f filedir] [-l logdir] [-a alarmlogfile] [-o opmodlogfile] [-m alarmlogmaxrecords [Xoptions] [configfile] \n",
               argv[0]);
          fprintf(stderr,"\n\tconfigfile\tAlarm configuration filename\n");
          fprintf(stderr,"\n\t-c\t\tAlarm Configuration Tool mode\n");
          fprintf(stderr,"\n\t-f filedir\tDirectory for all files\n");
          fprintf(stderr,"\n\t-l logdir\tDirectory for log files\n");
          fprintf(stderr,"\n\t-a alarmlogfile\tAlarm log filename\n");
          fprintf(stderr,"\n\t-o opmodlogfile\tOpMod log filename\n");
          fprintf(stderr,"\n\t-m maxrecords\talarm log file max records (default 2000)\n");
          exit(1);
     }
 
     if (DEBUG) printf("programName=%s\n",programName);

     /* ----- initialize and setup opMod file ----- */
     name = opModFile;
     if ( name[0] == '/' || (name[0] == '.' && name[1] == '.') ||
          (name[0] == '.' && name[1] == '/')) { 
          strncpy(psetup.opModFile,opModFile,NAMEDEFAULT_SIZE);
     } else {
          len = strlen(psetup.opModFile);
          strncat(psetup.opModFile,opModFile,NAMEDEFAULT_SIZE-len);
     }
     fileSetup(psetup.opModFile,NULL,FILE_OPMOD,programId,widget);

     while (!fo) { 
          fdmgr_pend_event(pfdctx,&timeout);
     }

     /* ----- initialize and setup alarm log file ----- */
     name = logFile;
     if ( name[0] == '/' || (name[0] == '.' && name[1] == '.') ||
          (name[0] == '.' && name[1] == '/')) { 
          strncpy(psetup.logFile,logFile,NAMEDEFAULT_SIZE);
     } else {
          len = strlen(psetup.logFile);
          strncat(psetup.logFile,logFile,NAMEDEFAULT_SIZE-len);
     }
     fileSetup(psetup.logFile,NULL,FILE_ALARMLOG,programId,widget);

     while (!fl) {
          fdmgr_pend_event(pfdctx,&timeout);
     }

     /* ----- initialize and setup config file ----- */
     name = configFile;
     if ( name[0] == '/' || (name[0] == '.' && name[1] == '.') ||
          (name[0] == '.' && name[1] == '/')) { 
          strncpy(psetup.configFile,configFile,NAMEDEFAULT_SIZE);
     } else {
          len = strlen(psetup.configFile);
          strncat(psetup.configFile,configFile,NAMEDEFAULT_SIZE-len);
     }
     fileSetup(psetup.configFile,NULL,FILE_CONFIG,programId,widget);
}
