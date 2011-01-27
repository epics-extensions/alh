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
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/audioio.h>
#include <signal.h>
#include <stropts.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

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

/* Pre solaris 8 definition */
#ifndef AUDIO_NONE
#define AUDIO_NONE      0x00    /* all ports off */
#endif

#ifndef AUDIO_CHANNELS_STEREO
#define AUDIO_CHANNELS_STEREO   (2)
#endif

#ifndef AUDIO_MID_GAIN
#define AUDIO_MID_GAIN  (AUDIO_MAX_GAIN / 2)
#endif

#ifndef AUDIO_PRECISION_8
#define AUDIO_PRECISION_8               (8)
#endif

#ifndef AUDIO_CHANNELS_MONO
#define AUDIO_CHANNELS_MONO     (1)
#endif
/* End of pre solaris 8 definition */

#define AUDIO_SOURCE_HI      0
#define AUDIO_SOURCE_LO      1
#define AUDIO_SOURCE_FILE    2

#define AU_PATTERN "*.au"

extern Pixel bg_pixel[ALH_ALARM_NSEV];

static struct audioinfo {
    char*          filename;
    unsigned char* pdata;
    unsigned long  dataSize;
    unsigned long  headerSize;
    unsigned long  audioSize;
    int            precision; /* bit-width of each sample  8,16,... */
    int            encoding;  /* data encoding method 0=none, 1=ULAW, 2=ALAW, 3=PCM */
    short          sampleRate; /* samples per second */
    short          channels;  /* number of channels 1=mono 2=stereo */
    short          littleEndian; /* not used yet */
    long           samples; 
};


static struct beepsetup {
    Widget         audioSetupDialog;
    Widget         audioSourceFrameWidget;
    short          port;        /* user selected I/O port */
    short          volume;      /* user selected gain level: 0 - 255 */
    short          balance;     /* user selected stereo channel balance - not implemented yet*/
    short          audioSource;
    struct audioinfo* paudioInfo;
    struct audioinfo* paudioInfoFile;
    struct audioinfo* paudioInfoInternalHi;
    struct audioinfo* paudioInfoInternalLo;
} audioSetup={NULL,NULL,AUDIO_NONE,AUDIO_MID_GAIN,50,0,NULL,NULL,NULL} ;

/* forward declarations */
static void initAudioSetup();
static unsigned char* readAudioData(char *filename,unsigned long *plength);
static struct audioinfo *createAudioInfo(char *filename,unsigned char *pdata,unsigned long length);
static int audioSetupNewFilename(Widget textfield, char *filename);
static int auAudioSettings(struct audioinfo *paudioInfo);
static void audioSetupCreateDialog(Widget menuButton);
static void audioSetupDismissCallback(Widget widget,XtPointer clientdata,XtPointer cbs);
static void audioSetupOutputChangeCallback(Widget widget,XtPointer clientdata,XtPointer cbs);
static void audioSetupVolumeChangeCallback(Widget widget,XtPointer clientdata,XtPointer cbs);
static void audioSetupAudioSourceChangeCallback(Widget widget,XtPointer clientdata,XtPointer cbs);
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
    static struct audio_info sound_info;
    audio_device_t dev_id;
    int i;
    int err;
    static int fd = -1;
    struct audioinfo *paudioInfo;

    if ( fd == -1 ) {
        AUDIO_INITINFO(&sound_info);
        fd = open("/dev/audio", O_WRONLY||O_NONBLOCK);
        if ( fd == -1 ) {
            errMsg("Error on open /dev/audio\n");
            return(-1);
        }
    } else {
        ioctl (fd, I_SETSIG, 0);
        ioctl (fd, I_FLUSH, FLUSHW);
    }

    if ( ioctl(fd, AUDIO_GETINFO, &sound_info) == -1 ) {
        errMsg("Error on ioctl get sound_info: %s\n",strerror(errno));
        close( fd );
        fd = -1;
        return(-1);
    }

    if (!audioSetup.paudioInfo) initAudioSetup();

    paudioInfo = audioSetup.paudioInfo;
    sound_info.play.sample_rate = paudioInfo->sampleRate; /* samples per second */
    sound_info.play.channels = paudioInfo->channels; /* number of interleaved channels */
    sound_info.play.precision = paudioInfo->precision; /* bit-width of each sample */
    sound_info.play.encoding = paudioInfo->encoding;  /* data encoding method */

    sound_info.play.port = audioSetup.port; /* user selected I/O port */
    sound_info.play.gain = audioSetup.volume; /* user selected gain level: 0 - 255 */
    /*uchar_t balance;*/        /* user selected stereo channel balance */

    if ( err=ioctl(fd, AUDIO_SETINFO, &sound_info) == -1 ) {
        errMsg("Error on ioctl set sound_info: %s \n",strerror(errno));
        close( fd );
        fd = -1;
        return(-1);
    }

    if ( paudioInfo->pdata==0 ){
        if (audioSetup.paudioInfo==audioSetup.paudioInfoFile){
            errMsg("No audio file specified\n");
        } else {
            errMsg("No audio data\n");
        }
        close( fd );
        fd = -1;
        return(-1);
    }

    if ( err=fcntl(fd, F_SETFL,O_NONBLOCK) == -1 ) {
        errMsg("Error on fcntl set O_NONBLOCK: %s \n",strerror(errno));
        close( fd );
        fd = -1;
        return(-1);
    }

    if ( write(fd,paudioInfo->pdata+paudioInfo->headerSize,
             paudioInfo->audioSize)!=paudioInfo->audioSize ){
        errMsg("Error on audio write: %s\n",strerror(errno));
        close( fd );
        fd = -1;
        return(-1);
    }

#if 0
    close( fd );
    fd = -1;
#endif
    return 0;
}

/******************************************************
  alBeep
******************************************************/
void alBeep(Display *displayBB)
{
    int percent;
    int min=-100,max=100;

/* Volume percent in call to XBell does not work */
#if 0
    percent = max-(max-min)*((float)(AUDIO_MAX_GAIN-audioSetup.volume))/
        (AUDIO_MAX_GAIN-AUDIO_MIN_GAIN);
#endif
    percent = 100;

    if (audioSetup.port==AUDIO_NONE){
        XkbBell(displayBB,None,percent,None);
    } else {
        if (alAudioBeep()){
            XkbBell(displayBB,None,percent,None);
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
    Widget scale;
    Widget button;
    Widget toggleButton;
    Widget frame, rowcol;
    Widget label;
    Pixel textBackground;
    XmString string;
    static ActionAreaItem audioSetup_items[] = {
         { "Dismiss", audioSetupDismissCallback, NULL} };
    ALINK      *area;

    if (!audioSetup.paudioInfo) initAudioSetup();

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

    string = XmStringCreateSimple("Audio Beep Output Port");
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

    toggleButton = XtVaCreateManagedWidget("None (Use keyboard speaker)",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        NULL);
    XmToggleButtonSetState(toggleButton,
        ((audioSetup.port==AUDIO_NONE)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupOutputChangeCallback, (XtPointer)AUDIO_NONE);

    toggleButton = XtVaCreateManagedWidget("Internal speaker",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        NULL);
    XmToggleButtonSetState(toggleButton,
        ((audioSetup.port==AUDIO_SPEAKER)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupOutputChangeCallback, (XtPointer)AUDIO_SPEAKER);

    toggleButton = XtVaCreateManagedWidget("Headphone",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        NULL);
    XmToggleButtonSetState(toggleButton,
        ((audioSetup.port==AUDIO_HEADPHONE)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupOutputChangeCallback, (XtPointer)AUDIO_HEADPHONE);

    toggleButton = XtVaCreateManagedWidget("Line out",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        NULL);
    XmToggleButtonSetState(toggleButton,
        ((audioSetup.port==AUDIO_LINE_OUT)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupOutputChangeCallback, (XtPointer)AUDIO_LINE_OUT);

    XtManageChild(rowcol);
    XtManageChild(frame);

    string = XmStringCreateSimple("Audio Source");
    label = XtVaCreateManagedWidget("audioAudioSourceLabel",
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

    audioSetup.audioSourceFrameWidget = frame;
    if (audioSetup.port==AUDIO_NONE) XtSetSensitive(frame, FALSE);
    else XtSetSensitive(frame, TRUE);

    rowcol = XtVaCreateWidget("rowcol",
        xmRowColumnWidgetClass, frame,
        XmNspacing,          0,
        XmNmarginWidth,      10,
        XmNmarginHeight,     10,
        XmNradioBehavior,    TRUE,
        XmNbackground,     textBackground,
        NULL);

    toggleButton = XtVaCreateManagedWidget("alh internal Hi pitch beep",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        /*1XmNalignment,XmALIGNMENT_CENTER*/
        NULL);
    XmToggleButtonSetState(toggleButton,
        ((audioSetup.audioSource==AUDIO_SOURCE_HI)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupAudioSourceChangeCallback, (XtPointer)AUDIO_SOURCE_HI);

    toggleButton = XtVaCreateManagedWidget("alh internal Lo pitch beep",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        NULL);
    XmToggleButtonSetState(toggleButton,
        ((audioSetup.audioSource==AUDIO_SOURCE_LO)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupAudioSourceChangeCallback, (XtPointer)AUDIO_SOURCE_LO);

    toggleButton = XtVaCreateManagedWidget("Au/u-law (.au) file",
        xmToggleButtonGadgetClass, rowcol,
        XmNmarginHeight,     0,
        XmNbackground,      textBackground,
        NULL);
    XmToggleButtonSetState(toggleButton,
        ((audioSetup.audioSource==AUDIO_SOURCE_FILE)?TRUE:FALSE),FALSE);
    XtAddCallback(toggleButton, XmNvalueChangedCallback,
        audioSetupAudioSourceChangeCallback, (XtPointer)AUDIO_SOURCE_FILE);

    XtManageChild(rowcol);
    XtManageChild(frame);

    string = XmStringCreateSimple("Au/u-law (.au) filename");
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

    XmTextFieldSetString(filename,audioSetup.paudioInfo->filename);

    XtAddCallback(filename, XmNactivateCallback,
        audioSetupFilenameChangeCallback, NULL);

    string = XmStringCreateSimple("Audio Beep Volume");
    label = XtVaCreateManagedWidget("audioVolumelabel",
        xmLabelGadgetClass,        form,
        XmNlabelString,            string,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              filename,
        NULL);
    XmStringFree(string);

    frame = XtVaCreateWidget("frame",
        xmFrameWidgetClass, form,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              label,
        XmNleftAttachment,         XmATTACH_FORM,
        XmNrightAttachment,        XmATTACH_FORM,
        NULL);

    scale = XtVaCreateManagedWidget("VolumeScale",
        xmScaleWidgetClass, frame,
        XmNmaximum,          AUDIO_MAX_GAIN,
        XmNminimum,          AUDIO_MIN_GAIN,
        XmNvalue,            audioSetup.volume,
        XmNshowValue,        True,
        XmNorientation,      XmHORIZONTAL,
        XmNbackground,      textBackground,
        NULL);
    XtAddCallback(scale, XmNvalueChangedCallback,
        audioSetupVolumeChangeCallback, NULL);

    XtManageChild(frame);

    string = XmStringCreateSimple("Test Beep");
    label = XtVaCreateManagedWidget("audioTestlabel",
        xmLabelGadgetClass,        form,
        XmNlabelString,            string,
        XmNtopAttachment,          XmATTACH_WIDGET,
        XmNtopWidget,              frame,
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
  audioSetupSetupVolumeChangeCallback
******************************************************/
static void audioSetupVolumeChangeCallback(Widget widget, XtPointer clientdata,
XtPointer cbs)
{
    audioSetup.volume = ((XmScaleCallbackStruct *)cbs)->value;
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
static void audioSetupTestBeepCallback( Widget widget, XtPointer clientdata,
XtPointer cbs)
{
    alBeep(XtDisplay(widget));
}

/******************************************************
  audioSetupAudioSourceChangeCallback
******************************************************/
static void audioSetupAudioSourceChangeCallback( Widget widget,
        XtPointer clientdata, XtPointer cbs)
{
    XmToggleButtonCallbackStruct *state =
        (XmToggleButtonCallbackStruct *) cbs;

    if (state->set) {
        audioSetup.audioSource = (int)clientdata;

        switch (audioSetup.audioSource) {
        case AUDIO_SOURCE_HI:
            audioSetup.paudioInfo = audioSetup.paudioInfoInternalHi;
            break;
        case AUDIO_SOURCE_LO:
            audioSetup.paudioInfo = audioSetup.paudioInfoInternalLo;
            break;
        case AUDIO_SOURCE_FILE:
            audioSetup.paudioInfo = audioSetup.paudioInfoFile;
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

        if (audioSetup.port==AUDIO_NONE){
            XtSetSensitive(audioSetup.audioSourceFrameWidget, FALSE);
        } else {
            XtSetSensitive(audioSetup.audioSourceFrameWidget, TRUE);
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
  audioSetupFilenameBrowseCallback
******************************************************/
static void audioSetupFilenameBrowseCallback( Widget widget,
        XtPointer clientdata, XtPointer cbs)
{
    createFileDialog(widget,
        (void *)audioSetupFileSelectCallback,(XtPointer)clientdata,
        (void *)XtUnmanageChild,(XtPointer)0,
        (XtPointer)0,"Au/u-law (.au) file",AU_PATTERN,0);
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
    char*             filename;
    int               filenameSize;
    char*             plast;
    unsigned char*    pdata;
    struct audioinfo* poldAudioInfoFile;
    struct audioinfo* paudioInfo;
    unsigned long     length;

    filename = string;
    /* remove leading white space */
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

    pdata = readAudioData(filename,&length);
    if (!pdata){
        return -1;
    }

    paudioInfo = createAudioInfo(filename,pdata,length);
    if (!paudioInfo){
        return -1;
    }

    poldAudioInfoFile = audioSetup.paudioInfoFile;
    audioSetup.paudioInfoFile = paudioInfo;
    if (audioSetup.audioSource==AUDIO_SOURCE_FILE) {
        audioSetup.paudioInfo = paudioInfo;
    }

    if (poldAudioInfoFile){
        if (poldAudioInfoFile->filename) free(poldAudioInfoFile->filename);
        if (poldAudioInfoFile->pdata) free(poldAudioInfoFile->pdata);
        free(poldAudioInfoFile);
    }

    return 0;
}

/******************************************************
  initAudioSetup
******************************************************/
static void initAudioSetup()
{
    if ( audioSetup.paudioInfo==0 ){
        if ( audioSetup.paudioInfoInternalHi==0 ) {
            audioSetup.paudioInfoInternalHi = createAudioInfo("internalHiBeep",
                beepHi,beepHiLength);
            audioSetup.paudioInfoInternalLo = createAudioInfo("internalLoBeep",
                beepLo,beepLoLength);
        }
        audioSetup.paudioInfo = audioSetup.paudioInfoInternalHi;
    }
}

/******************************************************
  readAudioData
******************************************************/
static unsigned char * readAudioData(char *filename,unsigned long *plength)
{
    unsigned long  dataSize;
    unsigned long  length;
    FILE*          fp;
    unsigned char* pfileData;

    fp = fopen(filename,"rb");
    if(!fp) {
        errMsg("Error: %s Could not open audio file %s.\n",
            strerror(errno),filename);
        return 0;
    }
    /* Get size of audio file */
    fseek(fp,0L,SEEK_END);
    dataSize=ftell(fp);
    if(dataSize==-1) {
        errMsg("Error determining size of audio file %s.\n",filename);
        fclose(fp);
        return 0;
    }
    fseek(fp,0L,SEEK_SET);
    pfileData = calloc(dataSize,sizeof(unsigned char));
    if(!pfileData) {
        errMsg("Error allocating memory for audio file %s.\n",filename);
        fclose(fp);
        return 0;
    }
    length = fread(pfileData,sizeof(unsigned char),dataSize,fp);
    if(feof(fp)) {
        errMsg("Error reading audio file %s. End of file encountered.\n",filename);
        fclose(fp);
        free(pfileData);
        return 0;
    }
    if(length != dataSize) {
        errMsg("Error reading audio file %s.\n",filename);
        fclose(fp);
        free(pfileData);
        return 0;
    }
    fclose(fp);
    *plength = length;
    return pfileData;
}

/******************************************************
  createAudioInfo
******************************************************/
static struct audioinfo *createAudioInfo(char *filename,unsigned char *pdata,
unsigned long length)
{
    struct audioinfo *paudioInfo;
    unsigned char hdr[16];

    paudioInfo = (struct audioinfo *)calloc(1,sizeof(struct audioinfo));

    paudioInfo->pdata = pdata;
    paudioInfo->dataSize = length;

    memcpy(hdr,paudioInfo->pdata,16);
    if (!memcmp(paudioInfo->pdata,".snd",4)){
        if( auAudioSettings(paudioInfo)){
            errMsg("Error in au audio file %s.\n",filename);
            free(paudioInfo);
            return 0;
        }
    } else {
        if (!memcmp(paudioInfo->pdata,"RIFF",4)){
/* wave files not implemented yet */
            errMsg("Error: alh cannot read wave audio file %s.\n",filename);
            free(paudioInfo);
            return 0;
#if 0
            if( waveAudioSettings(paudioInfo)){
                errMsg("Error in wave audio file %s.\n",filename);
                free(paudioInfo);
                return 0;
            }
#endif
        } else {
            /* Assume headerless ULAW file */
            paudioInfo->headerSize = 0;
            paudioInfo->audioSize = length;
            paudioInfo->precision = AUDIO_PRECISION_8;
            paudioInfo->encoding = AUDIO_ENCODING_ULAW;
            paudioInfo->sampleRate = 8000;
            paudioInfo->channels = AUDIO_CHANNELS_MONO;
            paudioInfo->littleEndian = 0;
            paudioInfo->samples = -1;
       }
    }

    paudioInfo->filename = (char *)calloc(1,strlen(filename)+1);
    strcpy(paudioInfo->filename,filename);

    return paudioInfo;
}

/******************************************************
  auAudioSettings
******************************************************/
static int auAudioSettings(struct audioinfo *paudioInfo)
{
    struct header{
        long magic;
        long headerSize;
        long audioSize;
        long encoding;
        long sampleRate;
        long channels;
    };
    struct header* phdr;

    phdr = (struct header*)paudioInfo->pdata;

#ifdef DEBUG
    printf("dataSize=%ld\n",paudioInfo->dataSize);
    printf("headerSize=%ld\n", phdr->headerSize);
    printf("audioSize=%ld\n", phdr->audioSize);
    printf("encoding=%ld\n", phdr->encoding);
    printf("sampleRate=%ld\n", phdr->sampleRate);
    printf("channels=%ld\n", phdr->channels);
#endif

    paudioInfo->headerSize = phdr->headerSize;
    paudioInfo->audioSize = phdr->audioSize;
    paudioInfo->encoding = phdr->encoding;
    paudioInfo->sampleRate = phdr->sampleRate;
    paudioInfo->channels = phdr->channels;
    paudioInfo->littleEndian = 0;

    switch (phdr->encoding)
    { 
        case 1:
            paudioInfo->precision = 8;
            break;
        case 2:
            paudioInfo->precision = 8;
            break;
        case 3:
            paudioInfo->precision = 16;
            break;
        case 4:
            paudioInfo->encoding = AUDIO_ENCODING_LINEAR;
            paudioInfo->precision = 24;
            break;
        case 5:
            paudioInfo->encoding = AUDIO_ENCODING_LINEAR;
            paudioInfo->precision = 32;
            break;
        default:
            return -1;
    }
    paudioInfo->samples = phdr->audioSize/(phdr->channels*(paudioInfo->precision/8));

#ifdef DEBUG
    printf("precision=%ld\n", paudioInfo->precision);
    printf("samples=%ld\n\n", paudioInfo->samples);
#endif

    return 0;
}

/******************************************************
  waveAudioSettings
******************************************************/
static int waveAudioSettings(struct audioinfo *paudioInfo)
{
    /* not implemented yet */
    return 0;
}

