/*
 $Log$
 Revision 1.2  1994/06/22 21:17:15  jba
 Added cvs Log keyword

 */

/* share/src/act	@(#)axSubW.h	1.4	G% */

/* axSubW.h */
/*
 *      Author:		Ben-chin Cha
 *      Date:		12-20-90
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
 * .01	mm-dd-yy		nnn	Description
 */

#ifndef INCaxSubWh
#define INCaxSubWh

static char *axSubWhSccsId = "@(#)axSubW.h	1.3\t8/4/93";


#include <Xm/Xm.h>

#ifndef INCsllLibh
#include <sllLib.h>
#endif

#ifndef INCalarmh
#include <alarm.h>
#endif

#ifndef INCalhh
#include <alh.h>
#endif

/***********************************************************
   structures for subWindows
***********************************************************/


/* display sub window structure */
struct subWindow {
/* ----- Window info ----- */
        void            *area;
        Widget           drawing_area;
        Widget           scrolled_w;
        Widget           form_vsb;
        Widget           sevrAboveInd;
        Widget           vsb;
        Widget           sevrBelowInd;
        Dimension        viewHeight;
        Dimension        rowHeight;
        Dimension        marginHeight;
        int              viewRowCount;
        int              oldViewRowCount;
/* ----- Config info ----- */
        int              modified;
        void            *parentLink;
        int              viewConfigCount;
        int              oldViewConfigCount;
        unsigned int     viewOffset;
/* ----- view routines ----- */
        void            *(*alViewNth)(); 
        void            *(*alViewNext)(); 
        void            (*alhRowWidgets)(); 
        int             (*alViewMaxSevrN)(); 
/* ----- Selection info ----- */
        void            *selectionLink; 
        Widget           selectionWidget;
/* ----- line item info ----- */
        SLIST           *lines;
};

#endif

/********************************************************************
  BRIEF DESCRIPTIONS OF FUNCTIONS DEFINED IN axSubW.c 

  This file contains subWindow handling routines.
*********************************************************************
-------------            
|  PUBLIC   |
-------------            
void scrollBarMovedCallback()           Adjust viewOffset when
                                        subWindow scrollbar is moved
void setParentLink()                    Set parentLink for subWindow
void setLineRoutine()                   Set subWindow routine names 
void markSelectedWidget()               Mark new and unmark old subWindow widgets
void initializeSubWindow()              Initialize subWindow fields
void setViewConfigCount()               Put count in subWindow viewCount field
void invokeSubWindowUpdate()            Update each line of subWindow
struct subWindow *createSubWindow()     Alloc space for a subWindow structure
void createSubWindowWidgets()           Create all subWindow widgets
int calcRowCount()                      Calculate the number of subWindow rows(lines)
int calcRowYValue()                     Calculate the subWindow y value for row(line)
void adjustScrollBar()                  Set new scrollbar values when subWindow
                                        view has changed
void exposeResizeCallback()             Redraw subWindows if resize has occurred
void defaultTreeSelection()             Make 1st line treeWindow default selection
void initSevrAbove()                    Initialize severity above indicator
void initSevrBelow()                    Initialize severity below indicator
void adjustManagedRows()                Unmanages unused line Widgets
*******************************************************************************/
