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

