/*
 $Log$
 Revision 1.5  1996/11/19 19:40:35  jba
 Fixed motif delete window actions, and fixed size of force PV window.

 Revision 1.4  1995/10/20 16:50:55  jba
 Modified Action menus and Action windows
 Renamed ALARMCOMMAND to SEVRCOMMAND
 Added STATCOMMAND facility
 Added ALIAS facility
 Added ALARMCOUNTFILTER facility
 Make a few bug fixes.

 * Revision 1.3  1995/02/28  16:43:53  jba
 * ansi c changes
 *
 * Revision 1.2  1994/06/22  21:17:54  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)scroll.c	1.16\t2/18/94";

/* scroll.c */
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
 * .01  09-18-91        bkc     Add file title on scroll window
 * .02  10-04-91        bkc     Add alarm log file column description on scroll window
 * .03  10-04-93        bkc     Set ShowPosition to last line of file
 * .03  mm-dd-yy        iii     Comment
 *      ...
 */

/***********************************************************
*
*    scroll.c
*
*This file contains the routines for viewing the alarm
*configuration, alarm log file and operator modification
*log file.  It uses scrolled text window to display the text.
*Opened scrolled window also displays the most current
*events recorded in log files.


Routines defined in scroll.c:
-------------
|   PUBLIC  |
-------------
void updateLog(fileIndex,string)                            Update open file view window
void fileViewWindow(w,option,menuButton)                  Open file view window callback

-------------
|  PRIVATE  |
-------------
static void closeFileViewWindow_callback(w,operandFile,call_data)     
static void closeFileViewShell(w,operandFile,call_data)     
*
**********************************************************/

#include <stdio.h>
#include <sys/stat.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/ScrollBar.h>
#include <Xm/Text.h>

#include <alh.h>
#include <ax.h>

#ifndef MAX
#  define MAX(a,b) ((a) > (b) ? a : b)
#endif


int viewFileUsedLength[N_LOG_FILES];        /* used length of file. */
int viewFileMaxLength[N_LOG_FILES];        /* max length of file. */
unsigned char *viewFileString[N_LOG_FILES];    /* contents of file. */

Widget viewTextWidget[N_LOG_FILES];        /* view text widget */
Widget viewFilenameWidget[N_LOG_FILES]; /* view filename widget */


extern int DEBUG;
extern struct setup psetup;            /* current file setup */
extern FILE *fl;                /* alarm logfile pointer*/
extern FILE *fo;                /* opmod file pointer*/


char error_file_size[] = {
    "  Sorry:  file size too big to view."
    };


#ifdef __STDC__

static void closeFileViewWindow_callback( Widget w, int operandFile, caddr_t call_data);
static void closeFileViewShell( Widget w, int operandFile, caddr_t call_data);

#else

static void closeFileViewWindow_callback();
static void closeFileViewShell();

#endif /*__STDC__*/


/**************************************************************************
    create scroll window for file view
**************************************************************************/
void fileViewWindow(w,option,menuButton)
Widget w;
int option;
Widget menuButton;
{
  static Widget config_shell=NULL,alarm_shell=NULL,opmod_shell=NULL;
  Widget app_shell=NULL,title,button;
  Widget previous;

  struct stat statbuf;         /* Information on a file. */
  FILE *fp = NULL;             /* Pointer to open file   */
  char filename[120];
  int operandFile=0;
  Arg al[20];
  int ac;
  XmString str=NULL;

  switch (option) {
    case CONFIG_FILE:   
        operandFile = CONFIG_FILE;
        app_shell = config_shell;
        break;
    case ALARM_FILE:   
        operandFile = ALARM_FILE;
        app_shell = alarm_shell;
        break;
    case OPMOD_FILE:   
        operandFile = OPMOD_FILE;
        app_shell = opmod_shell;
        break;
  }
  if (app_shell && XtIsManaged(app_shell)) {

    viewFileUsedLength[option] = 0;
    viewFileMaxLength[option] = 0;

    XtFree((char *)viewFileString[option]);
    viewFileString[option]=NULL;
    XtUnmanageChild(app_shell);

    XtVaSetValues(menuButton, XmNset, FALSE, NULL);

    return;
  }

  XtVaSetValues(menuButton, XmNset, TRUE, NULL);

  switch (option) {
    case CONFIG_FILE:   
        strcpy(filename,psetup.configFile);
        break;

    case ALARM_FILE:   
        fclose(fl);                /* close old file */
        fl = fopen(psetup.logFile,"a");        /* open new file */
        strcpy(filename,psetup.logFile);
        break;

    case OPMOD_FILE:   
        fclose(fo);                /* close old file */
        fo = fopen(psetup.opModFile,"a");    /* open new file */
        strcpy(filename,psetup.opModFile);
        break;
  }

  if ((fp = fopen(filename, "r+")) == NULL) {
        if ((fp = fopen(filename, "r")) != NULL) {
            fprintf(stderr, "fileViewWindow: file %s opened read only.\n",
        filename);
        } else {
           XtVaSetValues(menuButton, XmNset, FALSE, NULL);
           fprintf(stderr,"fileViewWindow: file %s not found\n",filename);
       return;             /* bail out if no file found */
        }
  }

  if (stat(filename, &statbuf) == 0)
         viewFileUsedLength[operandFile] = statbuf.st_size;
   else
         viewFileUsedLength[operandFile] = 1000000; /* arbitrary file length */

  if (statbuf.st_size > 1000000)
    {
    XtVaSetValues(menuButton, XmNset, FALSE, NULL);
    createDialog(XtParent(w),XmDIALOG_ERROR,filename, &error_file_size[0]);
    return;
    }

    
  /* read the file string */
  viewFileMaxLength[operandFile] = MAX(INITIAL_FILE_LENGTH,
        2*viewFileUsedLength[operandFile]);
  viewFileString[operandFile] = (unsigned char *) 
        XtCalloc(1,(unsigned)viewFileMaxLength[operandFile]);
  fread(viewFileString[operandFile], sizeof(char), 
        viewFileUsedLength[operandFile], fp);

  /* close up the file */
  if (fclose(fp)) 
    fprintf(stderr, "fileViewWindow: unable to close file %s.\n", 
        filename);

if (!app_shell) {
  /*  create view window dialog */
  ac = 0;
  XtSetArg (al[ac], XmNy, 47);  ac++;
  XtSetArg (al[ac], XmNx, 77);  ac++;
/*
  XtSetArg (al[ac], XmNallowShellResize, FALSE);  ac++;
*/
  XtSetArg (al[ac], XmNallowOverlap, FALSE);  ac++;
  XtSetArg(al[ac], XmNautoUnmanage, FALSE); ac++;
  switch (option) {
    case CONFIG_FILE:   
                str = XmStringLtoRCreate("Configuration File", XmSTRING_DEFAULT_CHARSET);
        break;
    case ALARM_FILE:   
                str = XmStringLtoRCreate("Alarm Log File", XmSTRING_DEFAULT_CHARSET);
        break;
    case OPMOD_FILE:   
                str = XmStringLtoRCreate("Operator Mod File", XmSTRING_DEFAULT_CHARSET);
        break;
  }
  XtSetArg(al[ac], XmNdialogTitle, str); ac++;
  app_shell = XmCreateFormDialog(XtParent(w), "SCROLL", al, ac);
  XmStringFree(str);
  XtVaSetValues(app_shell, XmNallowOverlap, FALSE, NULL);

  /* Modify the window manager menu "close" callback */
  {
     Atom         WM_DELETE_WINDOW;
     XtVaSetValues(XtParent(app_shell),
          XmNdeleteResponse, XmDO_NOTHING, NULL);
     WM_DELETE_WINDOW = XmInternAtom(XtDisplay(XtParent(app_shell)),
          "WM_DELETE_WINDOW", False);
     XmAddWMProtocolCallback(XtParent(app_shell),WM_DELETE_WINDOW,
          (XtCallbackProc)closeFileViewShell, (XtPointer)operandFile);
  }
 
  switch (option) {
    case CONFIG_FILE:   
        config_shell = app_shell;
        break;
    case ALARM_FILE:   
        alarm_shell = app_shell;
        break;
    case OPMOD_FILE:   
        opmod_shell = app_shell;
        break;
  }

/* add close button */
  ac = 0;
  XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
  XtSetArg (al[ac], XmNtopOffset, 5);  ac++;
  XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
  XtSetArg (al[ac], XmNleftOffset, 5);  ac++;
  XtSetArg (al[ac], XmNuserData, menuButton);  ac++;
  button = XtCreateManagedWidget("Close",xmPushButtonWidgetClass,
                        app_shell, al, ac);
  XtAddCallback(button, XmNactivateCallback,
                         (XtCallbackProc)closeFileViewWindow_callback,
                         (XtPointer) operandFile);

  previous = button;

/* create file name widget */
  ac = 0;
  XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  ac++;
  XtSetArg (al[ac], XmNtopOffset, 6);  ac++;
  XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);  ac++;
  XtSetArg (al[ac], XmNleftWidget, button);  ac++;
  XtSetArg (al[ac], XmNleftOffset, 6);  ac++;
  viewFilenameWidget[operandFile] = XtCreateManagedWidget(filename,xmLabelGadgetClass,
    app_shell,al, ac);

/* add titles */
  if ( option == ALARM_FILE) {
        ac = 0;
        XtSetArg (al[ac], XmNwidth,800);  ac++;
        XtSetArg (al[ac], XmNheight,30);  ac++;
        XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  ac++;
        XtSetArg (al[ac], XmNtopWidget, button);  ac++;
        XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
        title = XtCreateManagedWidget("    TIME_STAMP        PROCESS_VARIABLE_NAME        CURRENT_STATUS              HIGHEST_UNACK_STATUS              VALUE",
                xmLabelGadgetClass,app_shell,al,ac);
        previous = title;

   }


  /* create text widget */
  ac = 0;
  XtSetArg (al[ac], XmNrows, 24);  ac++;
  XtSetArg (al[ac], XmNcolumns, 80);  ac++;
  XtSetArg (al[ac], XmNscrollVertical, True);  ac++;
  XtSetArg (al[ac], XmNscrollHorizontal, True);  ac++;
  XtSetArg (al[ac], XmNeditMode, XmMULTI_LINE_EDIT);  ac++;
  XtSetArg (al[ac], XmNeditable, FALSE);  ac++;
  XtSetArg (al[ac], XmNcursorPositionVisible, FALSE);  ac++;
  XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  ac++;
  XtSetArg (al[ac], XmNtopWidget, previous);  ac++;
  XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  ac++;
  XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  ac++;
  XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  ac++;
  viewTextWidget[operandFile] = XmCreateScrolledText(app_shell, "text", al, ac);

  /* make appropriate item sensitive */
  XtSetSensitive(viewTextWidget[operandFile], True);
  XmAddTabGroup(viewTextWidget[operandFile]);

  XtManageChild(viewTextWidget[operandFile]);

}
  /* update the file name string */
  str = XmStringLtoRCreate(filename, XmSTRING_DEFAULT_CHARSET);
  XtVaSetValues(viewFilenameWidget[operandFile], XmNlabelString, str, NULL);
  XmStringFree(str);

  /* add the file string to the text widget */
  XmTextSetString(viewTextWidget[operandFile], (char *)viewFileString[operandFile]);

  XtVaSetValues(viewTextWidget[operandFile],
       XmNcursorPosition,  viewFileUsedLength[operandFile]-1,
       NULL);
  XmTextShowPosition(viewTextWidget[operandFile], viewFileUsedLength[operandFile]-1);


/*
  XtSetSensitive(viewTextWidget[operandFile], False);
*/


  XtManageChild(app_shell); 

}


/**************************************************************************
    close scroll window for file view
**************************************************************************/
static void closeFileViewShell(w,operandFile,call_data)
Widget w;
int operandFile;
caddr_t call_data;
{
    Widget menuButton;
    WidgetList children;

    XtVaGetValues(w, XmNchildren, &children, NULL);
    XtUnmanageChild(children[0]);
    XtVaGetValues(children[0], XmNchildren, &children, NULL);
    XtVaGetValues(children[0], XmNuserData, &menuButton, NULL);
    XtVaSetValues(menuButton, XmNset, FALSE, NULL);

    viewFileUsedLength[operandFile] = 0;
    viewFileMaxLength[operandFile] = 0;

    XtFree((char *)viewFileString[operandFile]);
    viewFileString[operandFile]=NULL;

}



/**************************************************************************
    close scroll window for file view
**************************************************************************/
static void closeFileViewWindow_callback(w,operandFile,call_data)
Widget w;
int operandFile;
caddr_t call_data;
{
    Widget menuButton;

    XtVaGetValues(w, XmNuserData, &menuButton, NULL);
    XtVaSetValues(menuButton, XmNset, FALSE, NULL);

    viewFileUsedLength[operandFile] = 0;
    viewFileMaxLength[operandFile] = 0;


    XtFree((char *)viewFileString[operandFile]);
    viewFileString[operandFile]=NULL;
    XtUnmanageChild(XtParent(w));

}





/******************************************************************
**      updateLog in scroll window
*****************************************************************/
void updateLog(fileIndex,string)
  int fileIndex;
  char *string;
{
  struct stat statbuf;         /* Information on a file. */
  FILE *fp = NULL;             /* Pointer to open file   */
  char filename[120];

  int stringLength = strlen(string);
  int oldUsedLength = viewFileUsedLength[fileIndex];


/* simply return if the file string does not exist */
  if (viewFileString[fileIndex] == NULL) return;


/*
 *  update the scroll bar 
 */
/*
  n = 0;
  XtSetArg(args[n],XmNverticalScrollBar,&scrollBar); n++;
  XtGetValues(XtParent(viewTextWidget[fileIndex]),args,n);

  n = 0;
  XtSetArg(args[n],XmNmaximum,&xmax); n++;
  XtSetArg(args[n],XmNminimum,&xmin); n++;
  XtGetValues(scrollBar,args,n);

  xmax++;

  n = 0;
  XtSetArg(args[n],XmNmaximum,&xmax); n++;
  XtSetValues(scrollBar,args,n);
*/

  if (viewFileUsedLength[fileIndex] + stringLength  <= 
        viewFileMaxLength[fileIndex]) {

  /* string fits, insert */

     strcat((char *)viewFileString[fileIndex],string);
     viewFileUsedLength[fileIndex] = viewFileUsedLength[fileIndex] + 
        stringLength;
     XmTextReplace(viewTextWidget[fileIndex],oldUsedLength,
        oldUsedLength,string);


  } else {

  /* string doesn't fit - reallocate to get enough room */
     viewFileMaxLength[fileIndex] = MAX(INITIAL_FILE_LENGTH,
        2*viewFileMaxLength[fileIndex]);
     XtFree((char *)viewFileString[fileIndex]);
     viewFileString[fileIndex] = (unsigned char *) 
        XtCalloc(1,(unsigned)viewFileMaxLength[fileIndex]);


     switch(fileIndex) {
    case ALARM_FILE: strcpy(filename,psetup.logFile);
             break;
    case OPMOD_FILE: strcpy(filename,psetup.opModFile);
             break;
     }
    
     if ((fp = fopen(filename, "r+")) == NULL)
        if ((fp = fopen(filename, "r")) != NULL) {
            fprintf(stderr, "updateLog: file %s opened read only.\n",filename);
        } else {
           fprintf(stderr,"updateLog: file %s not found.\n",filename);
           return;                /* bail out if no file found */
        }
     if (stat(filename, &statbuf) == 0)
         viewFileUsedLength[fileIndex] = statbuf.st_size;
     else
         viewFileUsedLength[fileIndex] = 1000000; /* arbitrary file length */

  /* read the file string */
     fread(viewFileString[fileIndex], sizeof(char), 
        viewFileUsedLength[fileIndex], fp);

  /* close up the file */
     if (fclose(fp)) fprintf(stderr, 
        "updateLog: unable to close file %s.\n",filename);
  
  /* add the file string to the text widget */
     XmTextSetString(viewTextWidget[fileIndex], (char *)viewFileString[fileIndex]);

  }

    XtVaSetValues(viewTextWidget[fileIndex],
         XmNcursorPosition,  viewFileUsedLength[fileIndex]-1,
         NULL);
    XmTextShowPosition(viewTextWidget[fileIndex], viewFileUsedLength[fileIndex]-1);

}

