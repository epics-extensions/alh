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
/* axSubW.h */

/************************DESCRIPTION***********************************
  structure for subWindows
**********************************************************************/

#ifndef INCaxSubWh
#define INCaxSubWh

static char *axSubWhsccsId = "@(#) $Id$";

#include <Xm/Xm.h>

#include "sllLib.h"

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
	int             (*alViewMaxSevrN)();
	/* ----- Selection info ----- */
	void            *selectionLink;
	Widget           selectionWidget;
	/* ----- line item info ----- */
	SLIST           *lines;
};

#endif

