/*
 $Log$
 Revision 1.5  1998/05/12 18:22:46  evans
 Initial changes for WIN32.

 Revision 1.4  1995/11/13 22:31:26  jba
 Added beepseverity command, ansi changes and other changes.

 * Revision 1.3  1995/10/20  16:50:25  jba
 * Modified Action menus and Action windows
 * Renamed ALARMCOMMAND to SEVRCOMMAND
 * Added STATCOMMAND facility
 * Added ALIAS facility
 * Added ALARMCOUNTFILTER facility
 * Make a few bug fixes.
 *
 * Revision 1.2  1994/06/22  21:17:07  jba
 * Added cvs Log keyword
 *
 */

/* axArea.h */

/* share/src/act	@(#)axArea.h	1.6	G% */
/* axArea.h - Alarm Handler */
/*
 *      Author:		Janet Anderson
 *      Date:		02-16-93
 *
 *	Experimental Physics and Industrial Control System (EPICS)
 *
 *	Copyright 1991, the Regents of the University of California,
 *	and the University of Chicago Board of Governors.
 *
 *	This software was produced under  U.S. Government contracts:
 *	(W-7405-ENG-36) at the Los Alamos National Laboratory,
 *	and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *	Initial development by:
 *		The Controls and Automation Group (AT-8)
 *		Ground Test Accelerator
 *		Accelerator Technology Division
 *		Los Alamos National Laboratory
 *
 *	Co-developed with
 *		The Controls and Computing Group
 *		Accelerator Systems Division
 *		Advanced Photon Source
 *		Argonne National Laboratory
 *
 *
 * Modification Log:
 * -----------------
 * .01	12-10-93		jba	endif comment changed to proper c comment
 * .01	mm-dd-yy		nnn	Description
 */

#ifndef INCaxAreah
#define INCaxAreah

static char *axAreahSccsId ="@(#)axArea.h	1.4\t9/9/93"; 

#include <Xm/Xm.h>

#include <sllLib.h>

typedef struct  areaLink{
     SNODE            node;
     int              mapped;
     int              managed;
     int              programId;
/* area widget info */
    /* runtime window widgets */
     Widget           runtimeToplevel;
     Widget           runtimeForm;
     Widget           blinkButton;

    /* main window widgets */
     Widget           toplevel;
     Widget           icon;
     Widget           currentAlarmForm;
     Widget           currentAlarm[10];
     Widget           form_main;
     Widget           menubar;
     Widget           messageArea;
     Widget           scale;
     Widget           label_filename;
     Widget           label_groupAlarm;
     Widget           label_channelAlarm;
     Widget           label_mask;
     Widget           silenceForever;
     Widget           silenceCurrent;
     Widget           beepSeverity;
     Widget           beepSeverityLabel;
     Widget           label_filenameTitle;
     Widget           treeWindowForm;
     Widget           groupWindowForm;
/* ----- Config info ----- */
     struct mainGroup *pmainGroup;
     int              changed;
     char             *blinkString;
/* ----- setup info ----- */
     int              (* viewFilter)();
     int              beepCondition;
     char             *configFile;
     char             *alarmlogFile;
     char             *opmodFile;
     char             *histFile;
/* ----- subWindow inf ----- */
     void             *treeWindow;
     void             *groupWindow;
     void             *propWindow;
     void             *forceMaskWindow;
     void             *forcePVWindow;
     void             *maskWindow;
/* ----- Current Selection groupLine info ----- */
     void             *selectionWindow;
     void             *selectionLink;
     int              selectionType;
     Widget           selectionWidget;

} ALINK;

typedef struct _menu_item {
    char        *label;         /* the label for the item */
    WidgetClass *class;         /* pushbutton, label, separator... */
    char         mnemonic;      /* mnemonic; NULL if none */
    char        *accelerator;   /* accelerator; NULL if none */
    char        *accel_text;    /* to be converted to compound string */
    void       (*callback)(Widget, void *, void*);   /* routine to call; NULL if none */
    XtPointer    callback_data; /* client_data for callback() */
    struct _menu_item *subitems; /* pullright menu items, if not NULL */
} MenuItem;

#endif /* INCaxAreah */

/********************************************************************
  BRIEF DESCRIPTIONS OF FUNCTIONS DEFINED IN axArea.c 

  Routines related to setting up an area config and area Main Window

*********************************************************************

buildPulldownMenu()              Main Window Menu creation
setupConfig()                    Setup area for new config.file
createMainWindowWidgets()        Create area Main Window widgets
markActiveWidget()               Mark active widget border
setupArea()                      Initialize an area 
isTreeWindow()                   Returns TRUE if area treeWindow 
scale_callback()                 Scale moved callback
markSelectionArea()              Set area selection values

*********************************************************************/
