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
/* alAudio.c
 *
   $Id$
*/

/************************DESCRIPTION***********************************
  Routines for audio output
**********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <windows.h>
#include <mmsystem.h>

#include <Xm/Xm.h>
#include <Xm/AtomMgr.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/PanedW.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/Scale.h>

#include "alAudio.h"
#include "ax.h"
#include "beep.h"

#define AUDIO_INTERNAL_SPEAKER     0
#define AUDIO_DEFAULT_OUTPUT       1

#define BEEP_AUDIO_SOURCE_INTERNAL 0
#define BEEP_AUDIO_SOURCE_FILE     1

#define AUDIO_CHANNELS_MONO        1
#define AUDIO_CHANNELS_STEREO      2

#define AUDIO_MAX_GAIN       0xFF
#define AUDIO_MID_GAIN       0x6F
#define AUDIO_MIN_GAIN       0x00

#define WAV_PATTERN "*.wav"

extern Pixel bg_pixel[ALH_ALARM_NSEV];

static struct beepsetup {
    Widget         audioSetupDialog;
    Widget         beepSourceFrameWidget;
    short          port;
    short          channels;
    short          sampleRate;
    short          balance;
    short          beepSource;
    unsigned char *beep;
    unsigned long  beepLength;
    char          *beepFileName;
    unsigned char *beepFileData;
    unsigned long  beepFileLength;
} audioSetup={NULL,NULL,AUDIO_DEFAULT_OUTPUT,AUDIO_CHANNELS_STEREO,
              8000,50,BEEP_AUDIO_SOURCE_INTERNAL,0,0,0,0,0} ;

/* forward declarations */
static int audioSetupNewFilename(Widget textfield, char *filename);
static void audioSetupCreateDialog(Widget menuButton);
static void audioSetupDismissCallback(Widget widget,XtPointer clientdata,XtPointer cbs);
static void audioSetupOutputChangeCallback(Widget widget,XtPointer clientdata,XtPointer cbs);
static void audioSetupBeepSourceChangeCallback(Widget widget,XtPointer clientdata,XtPointer cbs);
static void audioSetupTestBeepCallback(Widget widget,XtPointer clientdata,XtPointer cbs);
static void audioSetupFilenameChangeCallback(Widget widget,XtPointer clientdata,XtPointer cbs);
static void audioSetupFilenameBrowseCallback( Widget widget,XtPointer clientdata, XtPointer cbs);


/******************************************************
  alhAudioSetupCallback
******************************************************/
void alhAudioSetupCallback( Widget menuButton, XtPointer clientdata,
XtPointer cbs)
{
    /* dismiss the dialog */
    if (audioSetup.audioSetupDialog &&
        XtIsManaged(audioSetup.audioSetupDialog)) {
        audioSetupDismissCallback(audioSetup.audioSetupDialog,
            (XtPointer)menuButton, NULL);
        if (menuButton) XtVaSetValues(menuButton, XmNset, FALSE, NULL);
        return;
    }

    /* create audioSetupWindow and Dialog Widgets if necessary */
    if (!audioSetup.audioSetupDialog)  audioSetupCreateDialog(menuButton);

    /* show Dialog */
    if (!audioSetup.audioSetupDialog) return;
    if (!XtIsManaged(audioSetup.audioSetupDialog)){
        XtManageChild(audioSetup.audioSetupDialog);
    }

    XMapWindow(XtDisplay(audioSetup.audioSetupDialog),
        XtWindow(XtParent(audioSetup.audioSetupDialog)));

    if (menuButton) XtVaSetValues(menuButton, XmNset, TRUE, NULL);
}

/******************************************************
  alAudioBeep
******************************************************/
static int alAudioBeep()
{
    if ( audioSetup.beep==0 &&
         audioSetup.beepSource == BEEP_AUDIO_SOURCE_INTERNAL) {
            audioSetup.beep = beep;
            audioSetup.beepLength = beepLength;
    }

    if ( audioSetup.beep==0 ){
        if (audioSetup.beepSource==BEEP_AUDIO_SOURCE_FILE){
            errMsg("No audio file specified\n");
        } else {
            errMsg("No audio data\n");
        }
        return(-1);
    }


    if (!PlaySound((LPCSTR)audioSetup.beep,0,SND_MEMORY|SND_ASYNC))
    {
        errMsg("Error playing WAVE.\n");
        if ( beep==audioSetup.beep || beep==0 ||
             !PlaySound((LPCSTR)beep,0,SND_MEMORY|SND_ASYNC))
        {
         return(-1);
        }
    }
    return 0;
}

/******************************************************
  alBeep
******************************************************/
void alBeep(Display *displayBB)
{
    int percent;

    percent = 100;

    if (audioSetup.port==AUDIO_INTERNAL_SPEAKER){
        XBell(displayBB,percent);
        /*MessageBeep(-1);*/
    } else {
        if (alAudioBeep()){
            XBell(displayBB,percent);
            /*MessageBeep(-1);*/
        }
    }
}

/******************************************************
  audioSetupCreateDialog
******************************************************/
static void audioSetupCreateDialog(Widget menuButton)
{
    Widget audioSetupDialogShell;
    Widget form,form1;
    Widget filename;
    Widget button;
    Widget toggleButton;
    Widget frame, rowcol;
    Widget label;
    Pixel textBackground;
    XmString string;
    static ActionAreaItem audioSetup_items[] = {
         { "Dismiss", audioSetupDismissCallback, NULL} };
    ALINK      *area;

    textBackground = bg_pixel[3];

    XtVaGetValues(menuButton, XmNuserData, &area, NULL);

    if (audioSetup.audioSetupDialog){
        if (XtIsManaged(audioSetup.audioSetupDialog)) return;
        else XtManageChild(audioSetup.audioSetupDialog);
    }

    audioSetupDialogShell = XtVaCreatePopupShell("ALH Audio Setup",
        transientShellWidgetClass, area->toplevel, NULL, 0);

    /* Modify the window manager menu "close" callback */
    {
        Atom         WM_DELETE_WINDOW;
        XtVaSetValues(audioSetupDialogShell,
            XmNdeleteResponse, XmDO_NOTHING, NULL);
        WM_DELETE_WINDOW = XmInternAtom(XtDisplay(audioSetupDialogShell),
            "WM_DELETE_WINDOW", False);
        XmAddWMProtocolCallback(audioSetupDialogShell,WM_DELETE_WINDOW,
            (XtCallbackProc)audioSetupDismissCallback, (XtPointer)menuButton);
    }

    form = XtVaCreateWidget("audioSetupDialog",
        xmFormWidgetClass, audioSetupDialogShell,
        NULL);
    audioSetup.audioSetupDialog = form;

    string = XmStringCreateSimple("Audio Beep Output");
    label = XtVaCreateManagedWidget("audioOutputLabel",
        xmLabelGadgetClass,        form,
        XmNlabelString,            string,
        XmNtopAttachment,          XmATTACH_FORM,
        XmNtopWidget,              form,
        NULL);
    XmStringFree(string);

    frame = XtVaCreateWidget("frame",
        xmFrameWidgetClass, form,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              label,
        XmNleftAttachment,         XmATTACH_FORM,
        XmNrightAttachment,        XmATTACH_FORM,
        NULL);

    rowcol = XtVaCreateWidget("rowcol",
        xmRowColumnWidgetClass, frame,
        XmNspacing,         0,
        XmNmarginWidth,     10,
        XmNmarginHeight,    10,
        XmNradioBehavior,    TRUE,
        XmNbackground,      textBackground,
        NULL);

    toggleButton = XtVaCreateManagedWidget("Internal speaker",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        NULL);
    XmToggleButtonSetState(toggleButton,
        (Boolean)((audioSetup.port==AUDIO_INTERNAL_SPEAKER)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupOutputChangeCallback, (XtPointer)AUDIO_INTERNAL_SPEAKER);

    toggleButton = XtVaCreateManagedWidget("Default digital audio output",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        NULL);
    XmToggleButtonSetState(toggleButton,
        (Boolean)((audioSetup.port==AUDIO_DEFAULT_OUTPUT)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupOutputChangeCallback, (XtPointer)AUDIO_DEFAULT_OUTPUT);

    XtManageChild(rowcol);
    XtManageChild(frame);

    string = XmStringCreateSimple("Audio Beep Source");
    label = XtVaCreateManagedWidget("audioBeepSourceLabel",
        xmLabelGadgetClass,        form,
        XmNlabelString,            string,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              frame,
        NULL);
    XmStringFree(string);

    frame = XtVaCreateWidget("frame",
        xmFrameWidgetClass, form,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              label,
        XmNleftAttachment,         XmATTACH_FORM,
        XmNrightAttachment,        XmATTACH_FORM,
        NULL);

    audioSetup.beepSourceFrameWidget = frame;
    if (audioSetup.port==AUDIO_INTERNAL_SPEAKER) XtSetSensitive(frame, FALSE);
    else XtSetSensitive(frame, TRUE);

    rowcol = XtVaCreateWidget("rowcol",
        xmRowColumnWidgetClass, frame,
        XmNspacing,          0,
        XmNmarginWidth,      10,
        XmNmarginHeight,     10,
        XmNradioBehavior,    TRUE,
        XmNbackground,     textBackground,
        NULL);

    toggleButton = XtVaCreateManagedWidget("alh internal beep",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        /*1XmNalignment,XmALIGNMENT_CENTER*/
        NULL);
    XmToggleButtonSetState(toggleButton,
        (Boolean)((audioSetup.beepSource==BEEP_AUDIO_SOURCE_INTERNAL)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupBeepSourceChangeCallback,
        (XtPointer)BEEP_AUDIO_SOURCE_INTERNAL);

    if ( audioSetup.beep==0 ) {
        audioSetup.beep = beep;
        audioSetup.beepLength = beepLength;
    }

    toggleButton = XtVaCreateManagedWidget("WAV (.wav) file",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        NULL);
    XmToggleButtonSetState(toggleButton,
        (Boolean)((audioSetup.beepSource==BEEP_AUDIO_SOURCE_FILE)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupBeepSourceChangeCallback, (XtPointer)BEEP_AUDIO_SOURCE_FILE);

    XtManageChild(rowcol);
    XtManageChild(frame);

    string = XmStringCreateSimple("WAVE (.wav) filename");
    label = XtVaCreateManagedWidget("audioFilenamelabel",
        xmLabelGadgetClass,        form,
        XmNlabelString,            string,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              frame,
        NULL);
    XmStringFree(string);

    button = XtVaCreateManagedWidget("Browse",
        xmPushButtonWidgetClass, form,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              frame,
        XmNleftAttachment,         XmATTACH_WIDGET,
        XmNleftWidget,             label,
        XmNrightAttachment,        XmATTACH_FORM,
        XmNshadowThickness,        1,
        NULL);

    filename = XtVaCreateManagedWidget("filename",
        xmTextFieldWidgetClass, form,
        XmNbackground,      textBackground,
        XmNtopAttachment,   XmATTACH_WIDGET,
        XmNtopWidget,       button,
        XmNleftAttachment,  XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

    XtAddCallback(button, XmNactivateCallback,
        audioSetupFilenameBrowseCallback, filename);

    XmTextFieldSetString(filename,audioSetup.beepFileName);

    XtAddCallback(filename, XmNactivateCallback,
        audioSetupFilenameChangeCallback, NULL);

    string = XmStringCreateSimple("Test Beep");
    label = XtVaCreateManagedWidget("audioTestlabel",
        xmLabelGadgetClass,        form,
        XmNlabelString,            string,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              filename,
        XmNleftAttachment,         XmATTACH_FORM,
        NULL);
    XmStringFree(string);

    button = XtVaCreateManagedWidget("Beep",
        xmPushButtonWidgetClass, form,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              label,
        XmNleftAttachment,         XmATTACH_FORM,
        XmNrightAttachment,        XmATTACH_FORM,
        XmNshadowThickness,        2,
        NULL);
    XtAddCallback(button, XmNactivateCallback,
        audioSetupTestBeepCallback, NULL);

    /* Set the client data "Dismiss" button's callbacks. */
    audioSetup_items[0].data = (XtPointer)menuButton;

    form1 = createActionButtons(form, audioSetup_items,
        XtNumber(audioSetup_items));
    if (form1) XtVaSetValues(form1,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              button,
        XmNleftAttachment,         XmATTACH_FORM,
        XmNrightAttachment,        XmATTACH_FORM,
        NULL);

    XtManageChild(form);
    XtRealizeWidget(audioSetupDialogShell);
}

/******************************************************
  audioSetupDismissCallback
******************************************************/
static void audioSetupDismissCallback(Widget widget, XtPointer clientdata,
XtPointer cbs)
{
    Widget menuButton=(Widget)clientdata;
    Widget dialog;

    dialog=audioSetup.audioSetupDialog;
    XtUnmanageChild(dialog);
    XUnmapWindow(XtDisplay(dialog), XtWindow(XtParent(dialog)));
    if (menuButton) XtVaSetValues(menuButton, XmNset, FALSE, NULL);
}

/******************************************************
  audioSetupTestBeepCallback
******************************************************/
static void audioSetupTestBeepCallback( Widget widget, XtPointer clientdata, XtPointer cbs)
{
    alBeep(XtDisplay(widget));
}

/******************************************************
  audioSetupBeepSourceChangeCallback
******************************************************/
static void audioSetupBeepSourceChangeCallback( Widget widget,
        XtPointer clientdata, XtPointer cbs)
{
    XmToggleButtonCallbackStruct *state =
        (XmToggleButtonCallbackStruct *) cbs;

    if (state->set) {
        audioSetup.beepSource = (int)clientdata;

        switch (audioSetup.beepSource) {
        case BEEP_AUDIO_SOURCE_INTERNAL:
            audioSetup.beep = beep;
            audioSetup.beepLength = beepLength;
            break;
        case BEEP_AUDIO_SOURCE_FILE:
            audioSetup.beep = audioSetup.beepFileData;
            audioSetup.beepLength = audioSetup.beepFileLength;
            break;
        }
    }
}

/******************************************************
  audioSetupOutputChangeCallback
******************************************************/
static void audioSetupOutputChangeCallback( Widget widget,
        XtPointer clientdata, XtPointer cbs)
{
    XmToggleButtonCallbackStruct *state =
        (XmToggleButtonCallbackStruct *) cbs;

    if (state->set) {
        audioSetup.port = (int)clientdata;

        if (audioSetup.port==AUDIO_INTERNAL_SPEAKER){
            XtSetSensitive(audioSetup.beepSourceFrameWidget, FALSE);
        } else {
            XtSetSensitive(audioSetup.beepSourceFrameWidget, TRUE);
        }
    }
}

/******************************************************
  audioSetupFileSelectCallback
******************************************************/
void audioSetupFileSelectCallback(Widget widget, XtPointer clientdata,
XtPointer *cbs)
{
    char *string;

    /* get the filename string */
    XmStringGetLtoR(((XmFileSelectionBoxCallbackStruct *)cbs)->value,
         XmSTRING_DEFAULT_CHARSET, &string);

    if (strlen(string) &&
        !audioSetupNewFilename((Widget)clientdata,string))
        XtUnmanageChild(widget);
    XtFree(string);
}

/******************************************************
  audioSetupFilenameChangeCallback
******************************************************/
static void audioSetupFilenameBrowseCallback( Widget widget,
        XtPointer clientdata, XtPointer cbs)
{
    createFileDialog(widget,
        (void *)audioSetupFileSelectCallback,(XtPointer)clientdata,
        (void *)XtUnmanageChild,(XtPointer)0,
        (XtPointer)0,"Au/u-law (.au) file",WAV_PATTERN,(char *)0);
}

/******************************************************
  audioSetupFilenameChangeCallback
******************************************************/
static void audioSetupFilenameChangeCallback( Widget widget,
        XtPointer clientdata, XtPointer cbs)
{
    char * string;

    string = XmTextFieldGetString(widget);
    audioSetupNewFilename(widget,string);
    XtFree(string);
}

/******************************************************
  audioSetupNewFilename
******************************************************/
static int audioSetupNewFilename(Widget widget, char *string)
{
    char         * filename;
    char         * plast;
    int            filenameSize;
    unsigned long  fileSize;
    unsigned long  beepLength;
    FILE         * fp;
    unsigned char* beep;

    filename = string;

    /* remove leabeep white space */
    while (isspace(*filename)) {
            filename++;
            continue;
    }

    /* remove trailing white space */
    plast=filename + strlen(filename)-1;
    while (isspace(*plast)) {
        *plast=0;
        plast--;
    }

    filenameSize = strlen(filename);
    if (!filenameSize) return 0;
    if (filename[filenameSize-1]=='/'||filename[filenameSize-1]=='\\') return 0;

    XmTextFieldSetString(widget,filename);

    if (audioSetup.beepFileName) free(audioSetup.beepFileName);
    audioSetup.beepFileName=(char *)calloc(1,strlen(filename)+1);
    strcpy(audioSetup.beepFileName,filename);

    audioSetup.beepFileLength = 0;
    if (audioSetup.beepFileData) free(audioSetup.beepFileData);
    audioSetup.beepFileData = 0;

    if (audioSetup.beepSource==BEEP_AUDIO_SOURCE_FILE){
        audioSetup.beep = audioSetup.beepFileData;
        audioSetup.beepLength = audioSetup.beepFileLength;
    }

    fp = fopen(filename,"rb");
    if(!fp) {
        errMsg("Error: %s Could not open audio file %s.\n",
            strerror(errno),filename);
        return -1;
    }

    /* Get size of audio file */
    fseek(fp,0L,SEEK_END);
    fileSize=ftell(fp);
    if(fileSize==-1) {
        errMsg("Error determining size of audio file %s.\n",filename);
        fclose( fp );
        return -1;
    }
    fseek(fp,0L,SEEK_SET);

    beep = calloc(fileSize,sizeof(char));
    if(!beep) {
        errMsg("Error allocating memory for audio file %s.\n",filename);
        fclose( fp );
        return -1;
    }

    beepLength = fread(beep,sizeof(char),fileSize,fp);
    if(feof(fp)) {
        errMsg("Error reabeep audio file %s. End of file encountered.\n",filename);
        fclose( fp );
        return -1;
    }
    if(beepLength != fileSize) {
        errMsg("Error reabeep audio file %s.\n",filename);
        fclose( fp );
        return -1;
    }
    fclose( fp );

    audioSetup.beepFileLength = beepLength;
    if (audioSetup.beepFileData) free(audioSetup.beepFileData);
    audioSetup.beepFileData = beep;

    if (audioSetup.beepSource==BEEP_AUDIO_SOURCE_FILE){
        audioSetup.beep = audioSetup.beepFileData;
        audioSetup.beepLength = audioSetup.beepFileLength;
    }
    return 0;
}
