/* scroll.c */

/************************DESCRIPTION***********************************
  This file contains the routines for viewing the alarm
  configuration, alarm log file and operator modification
  log file.  It uses scrolled text window to display the text.
  Opened scrolled window also displays the most current
  events recorded in log files.
**********************************************************************/

static char *sccsId = "@(#) $Id$";

/**********************************************************************
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/ScrollBar.h>
#include <Xm/Text.h>
#include <Xm/PanedW.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/TextF.h>
#include <X11/cursorfont.h>

#include "alh.h"
#include "ax.h"


extern Display *display;

#define MAX_NUM_OF_LOG_FILES 5000  /* # of log files for browser */
static XmTextPosition positionSearch;
static int lenSearch;
extern char FS_filename[128];   /*FileName from FileSelectBox for AlLogV. */

static Widget findShell, findText;
static Widget text_with,text_fy,text_fmo,text_fd,text_fh,text_fmi,
text_ty,text_tmo,text_td,text_th,text_tmi;

static void findForward(Widget,XtPointer,XtPointer);
static void findReverse(Widget,XtPointer,XtPointer); 
static void findDismiss(Widget,XtPointer,XtPointer);
static void searchCallback(Widget,XtPointer,XmAnyCallbackStruct *);
void allDigit(Widget,XtPointer,XmTextVerifyCallbackStruct *);
static void showAllCallback(Widget,XtPointer,XtPointer);
static void showSelectedCallback(Widget,XtPointer,XtPointer);
void compactDataAscMonth(char *,char *,char *,char *,char *,char *);
void compactData(char *,char *,char *,char *,char *,char *);
char *digitalMonth(char *);
Boolean extensionIsDate(char *);
/* End for search routines. Albert */

#ifndef MAX
#  define MAX(a,b) ((a) > (b) ? a : b)
#endif


int viewFileUsedLength[N_LOG_FILES];        /* used length of file. */
int viewFileMaxLength[N_LOG_FILES];        /* max length of file. */
unsigned char *viewFileString[N_LOG_FILES];    /* contents of file. */

Widget viewTextWidget[N_LOG_FILES];        /* view text widget */
Widget browserWidget;  /* Albert */
Widget viewFilenameWidget[N_LOG_FILES]; /* view filename widget */

extern int alarmLogFileOffsetBytes; /* alarm log file current offset in bytes */
extern int alarmLogFileStringLength;  /* alarm log file record length*/
extern int alarmLogFileMaxRecords;   /* alarm log file maximum # records */
extern char alarmLogFileEndString[];  /* alarm log file end of data string */


extern int DEBUG;
extern struct setup psetup;            /* current file setup */
extern FILE *fl;                /* alarm logfile pointer*/
extern FILE *fo;                /* opmod file pointer*/



char error_file_size[] = {
	    "  Sorry:  file size too big to view."
	    };

/* forward declarations */
static void closeFileViewWindow_callback( Widget w, int operandFile, caddr_t call_data);
static void closeFileViewShell( Widget w, int operandFile, caddr_t call_data);


/**************************************************************************
    create scroll window for file view
**************************************************************************/
void fileViewWindow(Widget w,int option,Widget menuButton)
{
	static Widget config_shell=NULL,alarm_shell=NULL,opmod_shell=NULL;
	Widget app_shell=NULL,title,button;
	Widget previous;
	struct stat statbuf;         /* Information on a file. */
	FILE *fp = NULL;             /* Pointer to open file.  */
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

		break;

	case ALARM_FILE:
			strcpy(filename,psetup.logFile);
			fp=fl;
            		fseek(fp,0,SEEK_SET);
		        break;

	case OPMOD_FILE:
		fclose(fo);                /* close old file */
		strcpy(filename,psetup.opModFile);
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

		break;
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
	if (operandFile == ALARM_FILE && alarmLogFileMaxRecords)
		viewFileMaxLength[operandFile] =  alarmLogFileStringLength*alarmLogFileMaxRecords;

	viewFileString[operandFile] = (unsigned char *)
	    XtCalloc(1,(unsigned)viewFileMaxLength[operandFile]);
	fread(viewFileString[operandFile], sizeof(char), 
	    viewFileUsedLength[operandFile], fp);

	/* close up the file */
	switch(operandFile) {
	case ALARM_FILE:
		fseek(fp,alarmLogFileOffsetBytes,SEEK_SET);
		break;
	case CONFIG_FILE:
	case OPMOD_FILE:
		if (fclose(fp)) fprintf(stderr, 
		    "fileViewWindow: unable to close file %s.\n",filename);
		break;
	}


	if (!app_shell) {
		/*  create view window dialog */
		ac = 0;
		XtSetArg (al[ac], XmNy, 47);  
		ac++;
		XtSetArg (al[ac], XmNx, 77);  
		ac++;
		/*
		  XtSetArg (al[ac], XmNallowShellResize, FALSE);  ac++;
		*/
		XtSetArg (al[ac], XmNallowOverlap, FALSE);  
		ac++;
		XtSetArg(al[ac], XmNautoUnmanage, FALSE); 
		ac++;
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
		XtSetArg(al[ac], XmNdialogTitle, str); 
		ac++;
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
		XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  
		ac++;
		XtSetArg (al[ac], XmNtopOffset, 5);  
		ac++;
		XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  
		ac++;
		XtSetArg (al[ac], XmNleftOffset, 5);  
		ac++;
		XtSetArg (al[ac], XmNuserData, menuButton);  
		ac++;
		button = XtCreateManagedWidget("Close",xmPushButtonWidgetClass,
		    app_shell, al, ac);
		XtAddCallback(button, XmNactivateCallback,
		    (XtCallbackProc)closeFileViewWindow_callback,
		    (XtPointer) operandFile);

		previous = button;
		/* create file name widget */
		ac = 0;
		XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  
		ac++;
		XtSetArg (al[ac], XmNtopOffset, 6);  
		ac++;
		XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);  
		ac++;
		XtSetArg (al[ac], XmNleftWidget, button);  
		ac++;
		XtSetArg (al[ac], XmNleftOffset, 6);  
		ac++;
		viewFilenameWidget[operandFile] = XtCreateManagedWidget(filename,xmLabelGadgetClass,
		    app_shell,al, ac);

		/* add titles */
		if ( option == ALARM_FILE) {
			ac = 0;
			XtSetArg (al[ac], XmNwidth,800);  
			ac++;
			XtSetArg (al[ac], XmNheight,30);  
			ac++;
			XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  
			ac++;
			XtSetArg (al[ac], XmNtopWidget, button);  
			ac++;
			XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  
			ac++;
			title = XtCreateManagedWidget("    TIME_STAMP        PROCESS_VARIABLE_NAME        CURRENT_STATUS              HIGHEST_UNACK_STATUS              VALUE",
			    xmLabelGadgetClass,app_shell,al,ac);
			previous = title;

		}

		/* create text widget */
		ac = 0;
		XtSetArg (al[ac], XmNrows, 24);  
		ac++; 
		XtSetArg (al[ac], XmNcolumns, 80);  
		ac++;
		XtSetArg (al[ac], XmNscrollVertical, True);  
		ac++;
		XtSetArg (al[ac], XmNscrollHorizontal, True);  
		ac++;
		XtSetArg (al[ac], XmNeditMode, XmMULTI_LINE_EDIT);  
		ac++;
		XtSetArg (al[ac], XmNeditable, FALSE);  
		ac++;
		XtSetArg (al[ac], XmNcursorPositionVisible, FALSE);  
		ac++;
		XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  
		ac++;
		XtSetArg (al[ac], XmNtopWidget, previous);  
		ac++;
		XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  
		ac++;
		XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  
		ac++;
		XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  
		ac++;
		viewTextWidget[operandFile]=XmCreateScrolledText(app_shell, "text", al, ac);

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

	switch(operandFile) {
	case ALARM_FILE:
		XtVaSetValues(viewTextWidget[operandFile],
		    XmNcursorPosition,  alarmLogFileOffsetBytes+1,
		    NULL);
		XmTextShowPosition(viewTextWidget[operandFile], alarmLogFileOffsetBytes+1);
		break;
	case OPMOD_FILE:
		XtVaSetValues(viewTextWidget[operandFile],
		    XmNcursorPosition,  viewFileUsedLength[operandFile]-1,
		    NULL);
		XmTextShowPosition(viewTextWidget[operandFile], viewFileUsedLength[operandFile]-1);
		break;
	}
	/*
	  XtSetSensitive(viewTextWidget[operandFile], False);
	*/

	XtManageChild(app_shell);
}

/**************************************************************************
    close scroll window for file view
**************************************************************************/
static void closeFileViewShell(Widget w,int operandFile,caddr_t call_data)
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
static void closeFileViewWindow_callback(Widget w,int operandFile,
caddr_t call_data)
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
   updateLog in scroll window
*****************************************************************/
void updateLog(int fileIndex,char *string)
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
		case ALARM_FILE: 
			strcpy(filename,psetup.logFile);
			break;
		case OPMOD_FILE: 
			strcpy(filename,psetup.opModFile);
			break;
		}

		if ((fp = fopen(filename, "r+")) == NULL) {
			if ((fp = fopen(filename, "r")) != NULL) {
				fprintf(stderr, "updateLog: file %s opened read only.\n",filename);
			} else {
				fprintf(stderr,"updateLog: file %s not found.\n",filename);
				return;                /* bail out if no file found */
			}
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

/******************************************************************
   updateAlarmLog in scroll window
*****************************************************************/
void updateAlarmLog(int fileIndex,char *string)
{
	char   str[MAX_STRING_LENGTH];
	int stringLength = strlen(string);
	int oldUsedLength = viewFileUsedLength[fileIndex];
	int startPosition,endPosition;
	int pos=0;/* Albert1 */


if(viewTextWidget[fileIndex])
	XtVaGetValues(viewTextWidget[fileIndex],
	    XmNcursorPosition,  &pos,
	    NULL); /* Albert1 */
else return;      /* Albert1 */  

	/* simply return if the file string does not exist */
	if (viewFileString[fileIndex] == NULL) return;

	if (viewFileUsedLength[fileIndex] + stringLength  <= 
	    viewFileMaxLength[fileIndex])
	{
		/* put string at end */
		strcat((char *)viewFileString[fileIndex],string);
		viewFileUsedLength[fileIndex] = viewFileUsedLength[fileIndex] + stringLength;
		XmTextSetString(viewTextWidget[fileIndex], (char *)viewFileString[fileIndex]);
		/*
				XmTextReplace(viewTextWidget[fileIndex],oldUsedLength, oldUsedLength,string);
		*/

	} else 
	{
		/*
				strncpy((char *)(viewFileString[fileIndex]+
					alarmLogFileOffsetBytes-alarmLogFileStringLength),string,stringLength) ;
		*/
		/*                sprintf(str, "%-157s\n",alarmLogFileEndString); */
		/*
		                strncpy((char *)(viewFileString[fileIndex]+alarmLogFileOffsetBytes),
					str,alarmLogFileStringLength);
		*/
		startPosition=alarmLogFileOffsetBytes-alarmLogFileStringLength;
		endPosition=alarmLogFileOffsetBytes;
		XmTextReplace(viewTextWidget[fileIndex],startPosition, endPosition,string);
		memset(str,' ',alarmLogFileStringLength);
		startPosition=alarmLogFileOffsetBytes;
		endPosition=alarmLogFileOffsetBytes+alarmLogFileStringLength;
		XmTextReplace(viewTextWidget[fileIndex],startPosition, endPosition,str);

		/* add the file string to the text widget */
		/*
				XmTextSetString(viewTextWidget[fileIndex], (char *)viewFileString[fileIndex]);
		*/
	}

        XtVaSetValues(viewTextWidget[fileIndex],
	    XmNcursorPosition,  pos,
	    NULL);
	XmTextShowPosition(viewTextWidget[fileIndex],pos);
}

/**************************************************************************
    create scroll window for AlarmLog browser. Albert1.
**************************************************************************/
void browser_fileViewWindow(Widget w,int option,Widget menuButton)
{
	static Widget config_shell=NULL,alarm_shell=NULL,opmod_shell=NULL;
	Widget app_shell=NULL,title,button,button1;
	Widget previous;
	char sbuf[120];
	DIR *directory;
	struct stat statbuf;         /* Information on a file. */
	FILE *fp = NULL;             /* Pointer to open file.  */
	char filename[120];
	int operandFile=0;
	/* definitions for search widgets. Albert */
	Arg al[20];
	int ac;
	XmString str=NULL;
	Dimension height, margin_height;
	Widget findPane, findBox, findLabel, findButtonBox,
	    findForwardButton, findReverseButton, findDismissButton;
	Widget rowcol1,rowcol2,showButton,showAllButton;
	time_t timeofday;
	struct tm *tms;
	char buf[120];
	char defaultString_fy[5],defaultString_fmo[3],defaultString_fd[3],defaultString_fh[3]="00",defaultString_fmi[3]="00";
	char defaultString_ty[5],defaultString_tmo[3],defaultString_td[3],defaultString_th[3]="24",defaultString_tmi[3]="00";
	/* End definitions for search routins. Albert */
        option=ALARM_FILE;
	operandFile = ALARM_FILE;
	app_shell = alarm_shell;

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

	       strcpy(filename,FS_filename);       
		directory = opendir(filename);
		if (directory) {
		        closedir(directory);
			sprintf(sbuf, "%s is a directory\n",filename);
			createDialog(w,XmDIALOG_WARNING,sbuf," ");
			return;
			}
		if ((fp = fopen(filename, "r")) == NULL) {
			sprintf(sbuf, "fileViewWindow: Can't open file %s\n",filename);
			createDialog(w,XmDIALOG_WARNING,sbuf," ");
			fprintf(stderr, "Can't open file %s\n",filename);
			return;
		}
		fseek(fp,0,SEEK_SET);

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
	if (alarmLogFileMaxRecords)
	     viewFileMaxLength[operandFile] = alarmLogFileStringLength*alarmLogFileMaxRecords;

	viewFileString[operandFile] = (unsigned char *)
	    XtCalloc(1,(unsigned)viewFileMaxLength[operandFile]);
	fread(viewFileString[operandFile], sizeof(char), 
	    viewFileUsedLength[operandFile], fp);

	/* close up the file */
		fseek(fp,alarmLogFileOffsetBytes,SEEK_SET);
		fclose (fp) ; /* ????? Albert1 */

	if (!app_shell) {
		/*  create view window dialog */
		ac = 0;
		XtSetArg (al[ac], XmNy, 47);  
		ac++;
		XtSetArg (al[ac], XmNx, 77);  
		ac++;
		/*
		  XtSetArg (al[ac], XmNallowShellResize, FALSE);  ac++;
		*/
		XtSetArg (al[ac], XmNallowOverlap, FALSE);  
		ac++;
		XtSetArg(al[ac], XmNautoUnmanage, FALSE); 
		ac++;
		str = XmStringLtoRCreate("Alarm Log File", XmSTRING_DEFAULT_CHARSET);
		XtSetArg(al[ac], XmNdialogTitle, str); 
		ac++;
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
			alarm_shell = app_shell;

		/* add close button */
		ac = 0;
		XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  
		ac++;
		XtSetArg (al[ac], XmNtopOffset, 5);  
		ac++;
		XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  
		ac++;
		XtSetArg (al[ac], XmNleftOffset, 5);  
		ac++;
		XtSetArg (al[ac], XmNuserData, menuButton);  
		ac++;
		button = XtCreateManagedWidget("Close",xmPushButtonWidgetClass,
		    app_shell, al, ac);
		XtAddCallback(button, XmNactivateCallback,
		    (XtCallbackProc)closeFileViewWindow_callback,
		    (XtPointer) operandFile);

		previous = button;
		/* create file name widget */
		ac = 0;
		XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  
		ac++;
		XtSetArg (al[ac], XmNtopOffset, 6);  
		ac++;
		XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);  
		ac++;
		XtSetArg (al[ac], XmNleftWidget, button);  
		ac++;
		XtSetArg (al[ac], XmNleftOffset, 6);  
		ac++;
		viewFilenameWidget[operandFile] = XtCreateManagedWidget(filename,xmLabelGadgetClass,
		    app_shell,al, ac);

		/* For Search Widgets. 
		We define "Serch" button in AlLogViewPanel. Albert.
		*/
			ac = 0;
			XtSetArg (al[ac], XmNtopAttachment, XmATTACH_FORM);  
			ac++;
			XtSetArg (al[ac], XmNtopOffset, 5);  
			ac++;
			XtSetArg (al[ac], XmNleftAttachment, XmATTACH_WIDGET);  
			ac++;
			XtSetArg (al[ac], XmNleftWidget,viewFilenameWidget[operandFile] );  
			ac++;
			XtSetArg (al[ac], XmNleftOffset, 5);  
			ac++;
			XtSetArg (al[ac], XmNuserData, menuButton);  
			ac++;
			button1 = XtCreateManagedWidget("Search",xmPushButtonWidgetClass,
			    app_shell, al, ac);
			XtAddCallback(button1, XmNactivateCallback, searchCallback, app_shell );
			ac = 0;
			XtSetArg(al[ac], XmNallowShellResize, (XtArgVal) True); 
			ac++;
			XtSetArg(al[ac], XmNmappedWhenManaged, (XtArgVal) False); 
			ac++;
			findShell=XtVaCreatePopupShell("pshell",transientShellWidgetClass,app_shell,
			    NULL);
			findPane = XtVaCreateManagedWidget("findPane", xmPanedWindowWidgetClass, 
			    findShell);
			ac = 0;
			XtSetArg(al[ac], XmNhorizontalSpacing, (XtArgVal) 5); 
			ac++;
			XtSetArg(al[ac], XmNverticalSpacing, (XtArgVal) 5); 
			ac++;
			findBox = XtCreateManagedWidget("findBox", xmRowColumnWidgetClass, 
			    findPane, al, ac);
			ac = 0;
			XtSetArg(al[ac], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); 
			ac++;
			XtSetArg(al[ac], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); 
			ac++;
			XtSetArg(al[ac], XmNtopAttachment, (XtArgVal) XmATTACH_FORM); 
			ac++;
			XtSetArg(al[ac], XmNlabelString, XmStringCreateLtoR("Search:",
			    XmFONTLIST_DEFAULT_TAG)); 
			ac++;
			XtSetArg(al[ac], XmNalignment, (XtArgVal) XmALIGNMENT_CENTER); 
			ac++;
			findLabel = XtCreateManagedWidget("findLabel", xmLabelWidgetClass, 
			    findBox, al, ac);
			ac = 0;
			XtSetArg(al[ac], XmNleftAttachment, (XtArgVal) XmATTACH_FORM); 
			ac++;
			XtSetArg(al[ac], XmNrightAttachment, (XtArgVal) XmATTACH_FORM); 
			ac++;
			XtSetArg(al[ac], XmNtopAttachment, (XtArgVal) XmATTACH_WIDGET); 
			ac++;
			XtSetArg(al[ac], XmNtopWidget, (XtArgVal) findLabel); 
			ac++;
			XtSetArg(al[ac], XmNcolumns, (XtArgVal) 40); 
			ac++;
			findText = XtCreateManagedWidget("findText", xmTextWidgetClass, findBox, 
			    al, ac);
			XtAddCallback(findText, XmNactivateCallback, findForward, NULL);
			ac = 0;
			XtSetArg(al[ac], XmNborderWidth, (XtArgVal) 0); 
			ac++;
			XtSetArg(al[ac], XmNorientation, (XtArgVal) XmHORIZONTAL); 
			ac++;
			findButtonBox=XtCreateManagedWidget("findButtonBox", xmRowColumnWidgetClass,
			    findPane, al, ac);
			ac = 0;
			XtSetArg(al[ac], XmNlabelString,
			    XmStringCreateLtoR("Forward", XmFONTLIST_DEFAULT_TAG)); 
			ac++;
			findForwardButton = XtCreateManagedWidget("findForwardButton",
			    xmPushButtonWidgetClass, findButtonBox, al, ac);
			XtAddCallback(findForwardButton, XmNactivateCallback, findForward, 
			    app_shell);
			ac = 0;
			XtSetArg(al[ac], XmNlabelString, 
			    XmStringCreateLtoR("Reverse", XmFONTLIST_DEFAULT_TAG)); 
			ac++;
			findReverseButton = XtCreateManagedWidget("findReverseButton",
			    xmPushButtonWidgetClass, findButtonBox, al, ac);
			XtAddCallback(findReverseButton,XmNactivateCallback,findReverse,app_shell);
			ac = 0;
			XtSetArg(al[ac], XmNlabelString,
			    XmStringCreateLtoR("Dismiss", XmFONTLIST_DEFAULT_TAG)); 
			ac++;
			findDismissButton = XtCreateManagedWidget("findDismissButton",
			    xmPushButtonWidgetClass, findButtonBox, al, ac);
			XtAddCallback(findDismissButton, XmNactivateCallback, findDismiss, NULL);
			ac = 0;
			XtSetArg(al[ac], XmNmarginHeight, &margin_height); 
			ac++;
			XtGetValues(findButtonBox, al, ac);
			ac = 0;
			XtSetArg(al[ac], XmNheight, &height); 
			ac++;
			XtGetValues(findDismissButton, al, ac);
			ac = 0;
			XtSetArg(al[ac], XmNpaneMinimum, 
			    (XtArgVal)  (height + (margin_height * 2))); 
			ac++;
			XtSetArg(al[ac], XmNpaneMaximum,
			    (XtArgVal) (height + (margin_height * 2))); 
			ac++;
			XtSetValues(findButtonBox, al, ac);
			XtRealizeWidget(findShell);
			previous = button1;

			timeofday = time(0L);
			tms = localtime(&timeofday);
			sprintf(buf,"%04d%02d%02d%",
			    1900+tms->tm_year,1+tms->tm_mon,tms->tm_mday);
			sscanf(buf,"%4s%2s%2s",
			    &defaultString_fy,&defaultString_fmo,&defaultString_fd);
			sscanf(buf,"%4s%2s%2s",
			    &defaultString_ty,&defaultString_tmo,&defaultString_td);

			ac=0;
			XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  
			ac++;
			XtSetArg (al[ac], XmNtopWidget,button );  
			ac++;
			XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  
			ac++;
			XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  
			ac++;
			XtSetArg (al[ac], XmNorientation, XmHORIZONTAL); 
			ac++;
			rowcol1=
			    XtCreateManagedWidget("rowcol1",xmRowColumnWidgetClass,app_shell, al,ac);
			ac=0;
			XtSetArg (al[ac], XmNcolumns, 4); 
			ac++;
			XtSetArg (al[ac], XmNmaxLength, 4); 
			ac++;
			XtVaCreateManagedWidget("From:(YYYY-MM-DD-HH-MI)",xmLabelGadgetClass, 
			    rowcol1, NULL);
			text_fy=
			    XtCreateManagedWidget("tex_fy",xmTextFieldWidgetClass, rowcol1,al,ac);
			XtVaSetValues(text_fy, XmNvalue,defaultString_fy, NULL);
			XtAddCallback(text_fy, XmNmodifyVerifyCallback, allDigit, NULL);

			XtSetArg (al[ac], XmNcolumns, 2); 
			ac++;
			XtSetArg (al[ac], XmNmaxLength, 2); 
			ac++;
			text_fmo=
			    XtCreateManagedWidget("tex_fmo",xmTextFieldWidgetClass,rowcol1,al,ac);
			XtVaSetValues(text_fmo, XmNvalue,defaultString_fmo, NULL);
			XtAddCallback(text_fmo, XmNmodifyVerifyCallback, allDigit, NULL);

			text_fd=
			    XtCreateManagedWidget("tex_fd",xmTextFieldWidgetClass, rowcol1,al,ac);
			XtVaSetValues(text_fd, XmNvalue,defaultString_fd, NULL);
			XtAddCallback(text_fd, XmNmodifyVerifyCallback, allDigit, NULL);

			text_fh=
			    XtCreateManagedWidget("tex_fh",xmTextFieldWidgetClass,rowcol1,al,ac);
			XtVaSetValues(text_fh, XmNvalue,defaultString_fh, NULL);
			XtAddCallback(text_fh, XmNmodifyVerifyCallback, allDigit, NULL);

			text_fmi=
			    XtCreateManagedWidget("tex_fmi",xmTextFieldWidgetClass,rowcol1,al,ac);
			XtVaSetValues(text_fmi, XmNvalue,defaultString_fmi, NULL);
			XtAddCallback(text_fmi, XmNmodifyVerifyCallback, allDigit, NULL);

			XtVaCreateManagedWidget("With:",xmLabelGadgetClass, rowcol1, NULL);

			text_with=
			    XtVaCreateManagedWidget("tex_with",xmTextFieldWidgetClass, rowcol1,NULL);
			XtManageChild(rowcol1);
			ac=0;
			XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  
			ac++;
			XtSetArg (al[ac], XmNtopWidget,rowcol1 );  
			ac++;
			XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  
			ac++;
			XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  
			ac++;
			XtSetArg (al[ac], XmNorientation, XmHORIZONTAL); 
			ac++;
			rowcol2=
			    XtCreateManagedWidget("rowcol1",xmRowColumnWidgetClass,app_shell, al,ac);
			ac=0;
			XtSetArg (al[ac], XmNcolumns, 4); 
			ac++;
			XtSetArg (al[ac], XmNmaxLength, 4); 
			ac++;
			XtVaCreateManagedWidget("  To:(YYYY-MM-DD-HH-MI)",xmLabelGadgetClass, 
			    rowcol2, NULL);
			text_ty=
			    XtCreateManagedWidget("tex_ty",xmTextFieldWidgetClass, rowcol2,al,ac);
			XtVaSetValues(text_ty, XmNvalue,defaultString_ty, NULL);
			XtAddCallback(text_ty, XmNmodifyVerifyCallback, allDigit, NULL);

			XtSetArg (al[ac], XmNcolumns, 2); 
			ac++;
			XtSetArg (al[ac], XmNmaxLength, 2); 
			ac++;
			text_tmo=
			    XtCreateManagedWidget("tex_tmo",xmTextFieldWidgetClass,rowcol2,al,ac);
			XtVaSetValues(text_tmo, XmNvalue,defaultString_tmo, NULL);
			XtAddCallback(text_tmo, XmNmodifyVerifyCallback, allDigit, NULL);

			text_td=
			    XtCreateManagedWidget("tex_td",xmTextFieldWidgetClass,rowcol2,al,ac);
			XtVaSetValues(text_td, XmNvalue,defaultString_td, NULL);
			XtAddCallback(text_td, XmNmodifyVerifyCallback, allDigit, NULL);

			text_th=
			    XtCreateManagedWidget("tex_th",xmTextFieldWidgetClass,rowcol2,al,ac);
			XtVaSetValues(text_th, XmNvalue,defaultString_th, NULL);
			XtAddCallback(text_th, XmNmodifyVerifyCallback, allDigit, NULL);

			text_tmi=
			    XtCreateManagedWidget("tex_tmi",xmTextFieldWidgetClass,rowcol2,al,ac);
			XtVaSetValues(text_tmi, XmNvalue,defaultString_tmi, NULL);
			XtAddCallback(text_tmi, XmNmodifyVerifyCallback, allDigit, NULL);

			showButton=XtVaCreateManagedWidget("ShowSelected",
			    xmPushButtonWidgetClass,rowcol2, NULL);
			XtAddCallback(showButton,XmNactivateCallback,showSelectedCallback,app_shell);

			showAllButton=XtVaCreateManagedWidget("Show Current File",
			    xmPushButtonWidgetClass,rowcol2, NULL);
			XtAddCallback(showAllButton, XmNactivateCallback,showAllCallback,NULL);

			XtManageChild(rowcol2);
			previous = rowcol2;   /* Albert */
		/* End for Search Widgets. Albert_____________________________________ */


		/* add titles */
			ac = 0;
			XtSetArg (al[ac], XmNwidth,800);  
			ac++;
			XtSetArg (al[ac], XmNheight,30);  
			ac++;
			XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  
			ac++;
			XtSetArg (al[ac], XmNtopWidget, previous);  
			ac++; /* was button. Albert */
			XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  
			ac++;
			title = XtCreateManagedWidget("    TIME_STAMP        PROCESS_VARIABLE_NAME        CURRENT_STATUS              HIGHEST_UNACK_STATUS              VALUE",
			    xmLabelGadgetClass,app_shell,al,ac);
			previous = title;


		/* create text widget */
		ac = 0;
		XtSetArg (al[ac], XmNrows, 30);  
		ac++; /* Was 24. Albert*/
		XtSetArg (al[ac], XmNcolumns, 80);  
		ac++;
		XtSetArg (al[ac], XmNscrollVertical, True);  
		ac++;
		XtSetArg (al[ac], XmNscrollHorizontal, True);  
		ac++;
		XtSetArg (al[ac], XmNeditMode, XmMULTI_LINE_EDIT);  
		ac++;
		XtSetArg (al[ac], XmNeditable, FALSE);  
		ac++;
		XtSetArg (al[ac], XmNcursorPositionVisible, FALSE);  
		ac++;
		XtSetArg (al[ac], XmNtopAttachment, XmATTACH_WIDGET);  
		ac++;
		XtSetArg (al[ac], XmNtopWidget, previous);  
		ac++;
		XtSetArg (al[ac], XmNleftAttachment, XmATTACH_FORM);  
		ac++;
		XtSetArg (al[ac], XmNrightAttachment, XmATTACH_FORM);  
		ac++;
		XtSetArg (al[ac], XmNbottomAttachment, XmATTACH_FORM);  
		ac++;
		browserWidget=XmCreateScrolledText(app_shell, "text", al, ac);

		/* make appropriate item sensitive */
		XtSetSensitive(browserWidget, True);
		XmAddTabGroup(browserWidget);

		XtManageChild(browserWidget);

	}
	/* update the file name string */
	str = XmStringLtoRCreate(filename, XmSTRING_DEFAULT_CHARSET);
	XtVaSetValues(viewFilenameWidget[operandFile], XmNlabelString, str, NULL);
	XmStringFree(str);

	/* add the file string to the text widget */
	XmTextSetString(browserWidget, (char *)viewFileString[operandFile]);
		XtVaSetValues(browserWidget,
		    XmNcursorPosition,  alarmLogFileOffsetBytes+1,
		    NULL);
		XmTextShowPosition(browserWidget, alarmLogFileOffsetBytes+1);

	XtManageChild(app_shell);
}

/**************************************************************************
 Callback for ShowAll Button
**************************************************************************/
static void showAllCallback(Widget w,XtPointer client_data,XtPointer call_data)
{
	XmTextSetString(browserWidget,
	    (char *)viewFileString[ALARM_FILE]);
	XtManageChild(browserWidget);
}

/**************************************************************************
 Callback for ShowSelected Button
**************************************************************************/
static void showSelectedCallback(Widget w,XtPointer client_data,
XtPointer call_data)
{
  FILE *ffp;
  char *fy,*fmo,*fd,*fh,*fmi,*ty,*tmo,*td,*th,*tmi;
  char *string_with;
  char month[10], day[10], hour[10], min[10], year[10], un1[10],un2[10];
  int m;
  Boolean found;
  char *p;
  DIR *directory;
  struct dirent *rdr;
  char fname[120];
  char FSnameshort[120];
  char buf1[13],  buf2[13]; 
  static Cursor waitCursor;
  XSetWindowAttributes attrs;
  char lin[250];
  char *selectedText;
  static Widget dialog;
  XmString str,str1;
  char *dirForAlLog;
  int count=0;
  register int gap, i, j;
  char **v;
  register char *temp;
  char fromStr[11];
  char toStr[11];
  int len;
  int begin=0;
  int end=0;
  int tmp=1;
  Boolean breakFlag=False;
  char *line=&lin[0];

  fy =XmTextGetString(text_fy); 
  fmo=XmTextGetString(text_fmo);
  fd =XmTextGetString(text_fd); 
  fh =XmTextGetString(text_fh);
  fmi=XmTextGetString(text_fmi);
  ty =XmTextGetString(text_ty); 
  tmo=XmTextGetString(text_tmo);
  td =XmTextGetString(text_td); 
  th =XmTextGetString(text_th);
  tmi=XmTextGetString(text_tmi);

  compactData(fy,fmo,fd,fh,fmi,&buf1[0]);
  compactData(ty,tmo,td,th,tmi,&buf2[0]);
  if(strcmp(buf1,buf2) > 0) 
    {
    createDialog((Widget) client_data, XmDIALOG_WARNING,
    "DataFrom > DataTo.\n","Please, change Data");
    XtFree(fy); 
    XtFree(fmo); 
    XtFree(fh);  
    XtFree(fd); 
    XtFree(fmi);
    XtFree(ty); 
    XtFree(tmo); 
    XtFree(th); 
    XtFree(td); 
    XtFree(tmi);
    return;
    }

  dirForAlLog=XtCalloc(1,MAX_NUM_OF_LOG_FILES);
  strcpy( FSnameshort, (const char *)shortfile(FS_filename) );
  strncpy(dirForAlLog,FS_filename,strlen(FS_filename)-strlen(FSnameshort)-1);
  if ( (directory=opendir( (char *) dirForAlLog)) == NULL)
       {
       fprintf(stderr,"%s-wrong directory\n",dirForAlLog);
       XtFree(dirForAlLog);
       XtFree(fy); 
       XtFree(fmo); 
       XtFree(fh);  
       XtFree(fd); 
       XtFree(fmi);
       XtFree(ty); 
       XtFree(tmo); 
       XtFree(th); 
       XtFree(td); 
       XtFree(tmi);
       return;
       }

  string_with=XmTextGetString(text_with);
  selectedText=XtCalloc(1,viewFileMaxLength[ALARM_FILE]);
  if (!waitCursor) waitCursor=XCreateFontCursor(display,XC_watch);
  attrs.cursor=waitCursor;
  XChangeWindowAttributes(display, XtWindow (client_data),CWCursor,&attrs);
  XFlush(display);
  
  len=strlen(FSnameshort)-10;   /*  length of name without "extension" */
  while(rdr=readdir(directory)) 
    {
    if( strncmp(rdr->d_name,FSnameshort,len )) continue;
    if(!extensionIsDate(rdr->d_name+len))      continue; 
    count++;
    }
    closedir(directory);
  if ( (directory=opendir( (char *) dirForAlLog)) == NULL)
       {
       fprintf(stderr,"%s-wrong directory\n",dirForAlLog);
       XtFree(dirForAlLog);
       XtFree(fy); 
       XtFree(fmo); 
       XtFree(fh);  
       XtFree(fd);
       XtFree(fmi);
       XtFree(ty); 
       XtFree(tmo); 
       XtFree(th);  
       XtFree(td); 
       XtFree(tmi);
       return;
       }

  v=(char **) XtCalloc(sizeof(char *),count);
  i=0;
  while(rdr=readdir(directory)) {
    if( strncmp(rdr->d_name,FSnameshort,len )) continue;
    if(!extensionIsDate(rdr->d_name+len))      continue; 
    v[i]=(char *)calloc(sizeof(char), strlen(FSnameshort)+1 );
    strcpy(v[i],rdr->d_name); 
    v[i][strlen(FSnameshort)]=0;
    i++;
  }
  closedir(directory);

  /* BinarySortAlgoritm from C&R */
  for (gap = count/2; gap > 0; gap /= 2)
    for (i = gap; i < count; i++)
    for (j=i-gap; j>=0 && (strcmp(v[j],v[j+gap])>0); j-=gap) {
      temp = v[j];
      v[j] = v[j+gap];
      v[j+gap] = temp;
    }

  memcpy(&fromStr[0],fy,4);
  fromStr[4]='-';
  memcpy(&fromStr[5],fmo,2);
  fromStr[7]='-';
  memcpy(&fromStr[8],fd,2);
  fromStr[10]=0;
  memcpy(&toStr[0],ty,4);
  toStr[4]='-';
  memcpy(&toStr[5],tmo,2);
  toStr[7]='-';
  memcpy(&toStr[8],td,2);
  toStr[10]=0;

  for(i=0;i<count;i++) if((tmp=strcmp(v[i]+len,fromStr)) >=0 ) break;
  begin=i;
  if(tmp != 0) 
    {
    fh[0]=fh[1]=fmi[0]=fmi[1]='0';  /* We must search from YYYY-MM-DD-00-00 */ 
    }                               /* in this case */ 
  
  for(i=0;i<count;i++) if((tmp=strcmp(v[i]+len,toStr))  > 0 ) break;
  end=i;
  if(i==0) breakFlag=True;
  else if( strcmp( v[i-1]+len,toStr) != 0)
    {
     th[0]='2'; th[1]='4'; tmi[0]=tmi[1]='0'; /*We must search to YYYY-MM-DD-24-00*/
    }                                         /* in this case */ 

  for( i=begin; (i<end)&&(!breakFlag) ;i++)
  {
    memset(&fname[0],0,120);
    strcat(&fname[0],dirForAlLog);
    strcat(fname,"/");
    strcat(fname,v[i]);
    if( (ffp=fopen(fname,"r")) == NULL) {
      createDialog((Widget) client_data, XmDIALOG_WARNING,fname,
                   " - can't open file!");
      continue;
    }
    XFlush(display);
    XmUpdateDisplay(client_data);
   
    while ( fgets(line,159,ffp) ) {
      if(isdigit(line[0]))                    /* new time format Albert1*/
	sscanf(line,"%3s %4s %5s %3s %3s",
	       &day, &month,&year,&hour,&min);
      else {                                   /* old time format Albert1*/
	sscanf(line,"%4s %4s %3s %3s %3s %3s %4s",
	       &un1,&month,&day,&hour,&min,&un2,&year);
      month[3]=day[2]=hour[2]=min[2]=year[4]=0;
      
      if(atoi(day)<10) 
        {
        day[1]=day[0]; /* It's leading ZERO problem, because in LogFile for day<10 */
        day[0]='0';    /* we have notation like " 3 Apr" not " 03 Apr" */
        }
      }

      month[3]=day[2]=hour[2]=min[2]=year[4]=0;

      compactDataAscMonth(year,month,day,hour,min,&buf1[0]);
      compactData(fy,fmo,fd,fh,fmi,&buf2[0]);
      if( strcmp(&buf1[0],&buf2[0]) < 0 ) continue;

      compactData(ty,tmo,td,th,tmi,&buf2[0]);
      if( strcmp(&buf1[0],&buf2[0]) > 0 ) continue;
      if (strlen(string_with)){
        found=False;
        for (p = line; p = strchr( (const char *)p, *string_with); p++){
          if (!strncmp(p,string_with,strlen(string_with))) {found=True;break;}
	}
        if (!found) continue;
      }
      if( strlen(selectedText) > (viewFileMaxLength[ALARM_FILE] -2*159) )
        {

	createDialog((Widget) w, XmDIALOG_ERROR,
        "Result of the search is so long.\nPlease, detail search context.\n",
        "I show only begin of result.");
    
         breakFlag=True;
	 break;         /* Break from while( fgets()) */
        }
      strcat(selectedText,line); 
   }
   fclose(ffp);
  }

  XtFree(v); for(i=0;i<count;i++) XtFree(v[i]);
  XtFree(dirForAlLog);
  XtFree(string_with);
  XtFree(fy); 
  XtFree(fmo); 
  XtFree(fh); 
  XtFree(fd); 
  XtFree(fmi);
  XtFree(ty); 
  XtFree(tmo); 
  XtFree(th);  
  XtFree(td); 
  XtFree(tmi);

  XmTextSetString(browserWidget,selectedText);
  attrs.cursor=None;
  XChangeWindowAttributes(display, XtWindow (client_data),CWCursor,&attrs);
  XFlush(display);
  XtManageChild(browserWidget);
  XtFree(selectedText);
}




/**************************************************************************
    allDigit
**************************************************************************/
void allDigit(Widget text_w,XtPointer unused,XmTextVerifyCallbackStruct *cbs)
{
	char c;
	int len = XmTextGetLastPosition(text_w);
	if (cbs->text->ptr == NULL) return; /* backspace */
	/* don't allow non-digits or let the input exceed 5 chars */
	if (!isdigit(c = cbs->text->ptr[0]) || len >= 5)
		cbs->doit = False;
}

/******************************************************************
   searchCallback callback routine for Search widgets. Albert
*****************************************************************/
static void searchCallback(Widget w,XtPointer client_data,
XmAnyCallbackStruct *call_data)
{
	if (XtIsRealized(findShell))  XtMapWidget(findShell);
	XtPopup(findShell, XtGrabNone);
	/*  XMapRaised(XtDisplay(findShell), XtWindow(XtParent(findShell))); */
}

/******************************************************************
   findForward callback routine for Search widgets. Albert
*****************************************************************/
static void findForward(Widget w,XtPointer client_data,
XtPointer call_data)
{
	char *search_pat, *p, *string;
	Boolean found = False;
	Widget text_w,search_w;
	int operandFile=ALARM_FILE;
	text_w = browserWidget;
	search_w=findText;
	string = XmTextGetString(text_w);
	if (!*string) {
		createDialog((Widget)findShell,XmDIALOG_WARNING,"No text to search."," ");
		XtFree(string); 
		return;
	}
	search_pat = XmTextGetString(search_w);
	if (!*search_pat) {
		createDialog((Widget)client_data,XmDIALOG_WARNING,
		    "Specify a search pattern."," ");
		XtFree(string);
		XtFree(search_pat);  
		return;
	}
	XmTextSetHighlight(text_w,positionSearch,positionSearch+lenSearch, 
	    XmHIGHLIGHT_NORMAL);
	lenSearch=strlen(search_pat);
	positionSearch = XmTextGetCursorPosition(text_w);
	for(p=&string[positionSearch+1];p=strchr((const char *)p,*search_pat);p++)
		if (!strncmp(p, search_pat, lenSearch)) {
			found = True; 
			break;
		}
	if (!found) {
		createDialog((Widget)client_data,XmDIALOG_WARNING,
		    "Forward Pattern not found."," ");
	}
	else {
		positionSearch = (XmTextPosition)(p - string);
		XmTextSetInsertionPosition(text_w, positionSearch);
		XmTextSetHighlight(text_w,positionSearch,positionSearch+lenSearch,
		    XmHIGHLIGHT_SELECTED);
	}
	XtFree(string);
	XtFree(search_pat);
}

/******************************************************************
   findReverse callback routine for Search widgets. Albert
*****************************************************************/
static void findReverse(Widget w,XtPointer client_data,
XtPointer call_data)
{
	char *search_pat, *p, *string;
	Boolean found = False;
	Widget text_w,search_w;
	int operandFile=ALARM_FILE;
	text_w = browserWidget;
	search_w=findText;
	string = XmTextGetString(text_w);
	if (!*string) {
		createDialog((Widget)client_data,XmDIALOG_WARNING,"No text to search."," ");
		XtFree(string); 
		return;
	}
	search_pat = XmTextGetString(search_w);
	if (!*search_pat) {
		createDialog((Widget)client_data,XmDIALOG_WARNING,
		    "Specify a search pattern."," ");
		XtFree(string);
		XtFree(search_pat); 
		return;
	}
	XmTextSetHighlight(text_w,positionSearch,positionSearch+lenSearch, 
	    XmHIGHLIGHT_NORMAL);
	lenSearch=strlen(search_pat);
	if((positionSearch=XmTextGetCursorPosition(text_w))>0)
	{
		for (p = &string[positionSearch-1]; p >= string; p--)
			if (!strncmp(p, search_pat, lenSearch)) {
				found = True; 
				break;
			}
	}
	if (!found) {
		createDialog((Widget)client_data,XmDIALOG_WARNING,
		    "Reverse pattern not found."," ");
	}
	else {
		positionSearch = (XmTextPosition)(p - string);
		XmTextSetInsertionPosition(text_w, positionSearch);
		XmTextSetHighlight(text_w,positionSearch,positionSearch+lenSearch,
		    XmHIGHLIGHT_SELECTED);
	}
	XtFree(string);
	XtFree(search_pat);
}

/******************************************************************
   findDismiss callback routine for Search widgets. Albert
*****************************************************************/
static void findDismiss(Widget w,XtPointer client_data,
XtPointer call_data)
{
	XtUnmapWidget(findShell);
}

/******************************************************************
   Compact presentation of YYY-MM-DD. Albert. 
*****************************************************************/
void compactData(char *year,char *month,char *day,char *hour,
char *min,char *presentation)
{
	memcpy(&presentation[0],year,4);
	memcpy(&presentation[4],month,2);
	memcpy(&presentation[6],day,2);
	memcpy(&presentation[8],hour,2);
	memcpy(&presentation[10],min,2);
	presentation[12]=0;
}

/******************************************************************
   Compact presentation of YYY-MMM-DD. Albert. 
*****************************************************************/
void compactDataAscMonth(char *year,char *month,char *day,char *hour,
char *min,char *presentation)
{
	compactData(year,digitalMonth(month),day,hour,min,presentation);
}

/******************************************************************
   Month to Digit. Albert
*****************************************************************/
char *digitalMonth(char *strMonth)
{
	if(!strcmp(strMonth,"Jan")) return("01");
	if(!strcmp(strMonth,"Feb")) return("02");
	if(!strcmp(strMonth,"Mar")) return("03");
	if(!strcmp(strMonth,"Apr")) return("04");
	if(!strcmp(strMonth,"May")) return("05");
	if(!strcmp(strMonth,"Jun")) return("06");
	if(!strcmp(strMonth,"Jul")) return("07");
	if(!strcmp(strMonth,"Aug")) return("08");
	if(!strcmp(strMonth,"Sep")) return("09");
	if(!strcmp(strMonth,"Oct")) return("10");
	if(!strcmp(strMonth,"Nov")) return("11");
	if(!strcmp(strMonth,"Dec")) return("12");
	fprintf(stderr,"%s --- NOT A MONTH !!!!!\n",strMonth);
	return("00");
}

/******************************************************************
 Cheking that extension looks like YYYY-MM-DD. Albert
*****************************************************************/
Boolean extensionIsDate(char *ext)
{
  if(*(ext+10) != 0)     return False;
  if(!isdigit(*ext))     return False;
  if(!isdigit(*(ext+1))) return False;
  if(!isdigit(*(ext+2))) return False;
  if(!isdigit(*(ext+3))) return False;
  if(*(ext+4) != '-')    return False;
  if(!isdigit(*(ext+5))) return False;
  if(!isdigit(*(ext+6))) return False;
  if(*(ext+7) != '-')    return False;
  if(!isdigit(*(ext+8))) return False;
  if(!isdigit(*(ext+9))) return False;
return True;
}


