/* line.h */

/************************DESCRIPTION***********************************
  structure for displayed lines in the subWindows
**********************************************************************/

#ifndef INClineh
#define INClineh

#include "alarm.h"

#include "sllLib.h"

static char *linehsccsId = "@(#) $Id$";

#define LINEMESSAGE_SIZE   60           /* window message line size */

typedef  void (*FUNPTR)();      /* define void function pointer */

/* group/channel Line data structure */
struct anyLine {
	SNODE node;                     /* line node type */
	int lineNo;                     /* line no in window */
	char mask[8];                   /* mask summary string */
	char *pname;                    /* line name pointer */
	char *alias;                    /* line alias pointer */
	void *pwindow;                  /* corresponding window pointer */
	void *link;                     /* corresponding glink address */
	FUNPTR cosCallback;             /* COS callback pointer */
	char message[LINEMESSAGE_SIZE]; /* summary message string */
	void *wline;                    /* address of line_widget */
	int linkType;                   /* type of gclink group or channel */
	int unackSevr;                  /* highest unack severity */
	int curSevr;                    /* current severity */

	int curStat;                    /* Channel: current status */
	int unackStat;                  /* Channel: unack status */

	short curSev[ALARM_NSEV];       /* Group: channels current sevr */
};

struct widgetLine {
	Widget row_widget;
	Widget treeSym;
	Widget ack;
	Widget sevr;
	Widget name;
	Widget arrow;
	Widget guidance;
	Widget process;
	Widget mask;
	Widget message;
};

typedef struct widgetLine WLINE;

#endif /* INClineh */

