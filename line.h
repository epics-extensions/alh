/*
 $Log$
 Revision 1.4  1995/06/22 19:48:55  jba
 Added $ALIAS facility.

 * Revision 1.3  1995/04/13  19:20:29  jba
 * Fixed bug in line mask size and initialization.
 *
 * Revision 1.2  1994/06/22  21:17:41  jba
 * Added cvs Log keyword
 *
 */

/* line.h */
/* share/src/act	@(#)line.h	1.5	G% */
/* line.h - Alarm Handler */
/*
 *      Author:		Janet Anderson
 *      Date:		05-04-92
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
 * .01	12-10-93		jba	#endif comment changed to proper c comment
 * .01	mm-dd-yy		nnn	Description
 */

#ifndef INClineh
#define INClineh

static char *linehSccsId = "@(#)line.h	1.5\t12/15/93";

#include <alarm.h>
#include <sllLib.h>

/***********************************************************
   structure for displayed lines in the subWindows
***********************************************************/

/* group Line data structure */
struct groupLine {
        SNODE node;                     /* group line node type */
        int lineNo;                     /* line no in window */
        char mask[8];                   /* mask summary string */
        char *pname;                    /* group line name pointer */
        char *alias;                    /* group line alias pointer */
        void *pwindow;                  /* corresponding window pointer */
        void *glink;                    /* corresponding glink address */
        FUNPTR cosCallback;             /* COS callback pointer */
        char message[LINEMESSAGE_SIZE]; /* summary message string */
        void *wline;                    /* address of line_widget */
        int linkType;                   /* type of gclink group or channel */
        int unackSevr;                  /* highest unack severity */
        int curSevr;                    /* current severity */
        short curSev[ALARM_NSEV];       /* channels current sevr */
        };

/* channel Line data structure */
struct chanLine {
        SNODE node;                     /* channel line node type */
        int lineNo;                     /* line no in window */
        char mask[8];                   /* current mask */
        char *pname;                    /* channel line name pointer */
        char *alias;                    /* channel line alias pointer */
        void *pwindow;                  /* corresponding window pointer*/
        void *clink;                    /* corresponding clink address */
        FUNPTR cosCallback;             /* COS callback pointer */
        char message[LINEMESSAGE_SIZE]; /* display message string */
        void *wline;                    /* address of line_widget */
        int linkType;                   /* type of gclink group or channel */
        int unackSevr;                  /* unack severity */
        int curSevr;                    /* current severity */
        int curStat;                    /* current status */
        int unackStat;                  /* unack status */
        };


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


/********************************************************************
  BRIEF DESCRIPTIONS OF FUNCTIONS DEFINED IN line.c 

  Routines for alloc, init, and update of a displayed line

*********************************************************************
-------------            
|  PUBLIC   |
-------------            
char *awGetMaskString(mask)             		Get group mask string
awUpdateChanLine(chanLine)              		Update channel line message
awUpdateGroupLine(groupLine)            		Update groupline message
static struct chanLine *awAllocChanLine()               Allocate channel line
static struct groupLine *awAllocGroupLine()             Allocate group line
awGroupMessage(groupLine)                 		Prepare group line message
awChanMessage(pchanLine)                  		Prepare channel line message
void initLine(line)                                     Initializes all line fields
void initializeLines(lines)                             Initializes all subWindow lines

*********************************************************************/
