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
/* line.h */

/************************DESCRIPTION***********************************
  structure for displayed lines in the subWindows
**********************************************************************/

#ifndef INClineh
#define INClineh

#include "sllLib.h"
#include "alh.h"

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
	short curSevr;                    /* current severity */
	short unackSevr;                  /* highest unack severity */
	int curStat;                    /* Channel: current status */
	int curSev[ALH_ALARM_NSEV];   /* Group: current sevr channel counts*/
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

