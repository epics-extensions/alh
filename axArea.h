/* axArea.h */

/************************DESCRIPTION***********************************
  Area structure definitions
**********************************************************************/

#ifndef INCaxAreah
#define INCaxAreah

static char *axAreahsccsId = "@(#) $Id$";

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
	Widget           silenceOneHour;
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

