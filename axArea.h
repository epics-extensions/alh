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
/* axArea.h */

/************************DESCRIPTION***********************************
  Area structure definitions
**********************************************************************/

#ifndef INCaxAreah
#define INCaxAreah

#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include "sllLib.h"

#define PushButtonGadgetClass 0
#define SeparatorGadgetClass 1
#define ToggleButtonGadgetClass 2

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
	Widget           form_main;
	Widget           menubar;
	Widget           messageArea;
	Widget           scale;
	Widget           label_filename;
	Widget           label_groupAlarm;
	Widget           label_channelAlarm;
	Widget           label_mask;
	Widget           silenceOneHour;
	Widget           silenceCurrent;
	Widget           silenceForever;
	Widget           silenceForeverLabel;
	Widget           beepSeverity;
	Widget           beepSeverityLabel;
	Widget           disabledForcePVCountLabel;
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
	int              disabledForcePVCount;
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
	void             *beepSevrWindow;
	void             *noAckWindow;
	/* ----- Current Selection groupLine info ----- */
	void             *selectionWindow;
	void             *selectionLink;
	int              selectionType;
	Widget           selectionWidget;
	/* ----- Current alarm window info ----- */
	int              currentAlarmIndex;
	Widget           currentAlarmForm;
	Widget           currentAlarm[10];
	char             currentAlarmString[10][128];
} ALINK;

typedef struct _menu_item {
	char        *label;         /* the label for the item */
	int          class;         /* pushbutton, label, separator... */
	char         mnemonic;      /* mnemonic; NULL if none */
	char        *accelerator;   /* accelerator; NULL if none */
	char        *accel_text;    /* to be converted to compound string */
	void (*callback)( Widget,XtPointer,XtPointer);  /* routine to call; NULL if note */
	XtPointer    callback_data; /* client_data for callback() */
	struct _menu_item *subitems; /* pullright menu items, if not NULL */
	short  initial_state; /* initial state of toggleButton/toggleButtonGadget menu items */
} MenuItem;

#endif /* INCaxAreah */

