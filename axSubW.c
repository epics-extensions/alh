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
/* axSubW.c */

/************************DESCRIPTION***********************************
  This file contains subWindow handling routines.
**********************************************************************/

/********************************************************************
  Public functions defined in axSubW.c 

static void scrollBarMovedCallback()    Adjust viewOffset when
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

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>

#include "alh.h"
#include "sllLib.h"
#include "line.h"
#include "axSubW.h"
#include "axArea.h"
#include "ax.h"

/* globals */
extern Pixel bg_pixel[ALH_ALARM_NSEV];

/***************************************************
  scrollBarMovedCallback
****************************************************/
static void scrollBarMovedCallback(Widget widget,struct subWindow *subWindow,
XmScrollBarCallbackStruct *cbs)
{
	if ((int)subWindow->viewOffset == cbs->value) return;
	subWindow->viewOffset = cbs->value;
	redraw(subWindow,0);
}

/***************************************************
  setParentLink
****************************************************/
void setParentLink(struct subWindow *subWindow,void *link)
{
	subWindow->parentLink = link;
}

/***************************************************
  setLineRoutine
****************************************************/
void setLineRoutine(void *area,struct subWindow *subWindow,int program)
{
	/* Set line widget creation/modify routines to default values */
	if (isTreeWindow(area,subWindow)) {
		subWindow->alViewNth = ( void  *(*)())alViewNthTreeW;
		subWindow->alViewNext = ( void *(*)())alViewNextTreeW;
		subWindow->alViewMaxSevrN = alViewMaxSevrNTreeW;
	} else {
		subWindow->alViewNth = (void  *(*)())alViewNthGroupW;
		subWindow->alViewNext = (void *(*)())alViewNextGroupW;
		subWindow->alViewMaxSevrN = alViewMaxSevrNGroupW;
	}
}
/***************************************************
  markSelectedWidget
****************************************************/
void markSelectedWidget(struct subWindow *subWindow,Widget newWidget)
{
	static Pixel armColor=0;
	static Pixel backgroundColor=0;
	static Pixel bottomShadowColor=0;
	static Pixel topShadowColor=0;
	Widget       oldWidget;

	oldWidget = subWindow->selectionWidget;
	if (oldWidget == newWidget ) return;

	if (oldWidget) {
		XtVaGetValues(oldWidget,
		    XmNbackground,       &backgroundColor,
		    XmNarmColor,         &armColor,
		    XmNtopShadowColor,   &topShadowColor,
		    XmNbottomShadowColor,&bottomShadowColor,
		    NULL);

		XtVaSetValues(oldWidget,
		    XmNbackground,       armColor,
		    XmNarmColor,         backgroundColor,
		    XmNtopShadowColor,   bottomShadowColor,
		    XmNbottomShadowColor,topShadowColor,
		    NULL);
	}

	if (newWidget) {
		XtVaGetValues(newWidget,
		    XmNbackground,       &backgroundColor,
		    XmNarmColor,         &armColor,
		    XmNtopShadowColor,   &topShadowColor,
		    XmNbottomShadowColor,&bottomShadowColor,
		    NULL);

		XtVaSetValues(newWidget, 
		    XmNbackground,       armColor,
		    XmNarmColor,         backgroundColor,
		    XmNtopShadowColor,   bottomShadowColor,
		    XmNbottomShadowColor,topShadowColor,
		    NULL);
	}
	subWindow->selectionWidget = newWidget;
	markActiveWidget(subWindow->area,newWidget);
}

/***************************************************
  initializeSubWindow
****************************************************/
void initializeSubWindow(struct subWindow *subWindow)
{
	subWindow->modified = FALSE;
	subWindow->viewOffset = 0;

	/* Make NULL the selected group */
	subWindow->selectionLink = NULL;
	subWindow->parentLink = NULL;

	subWindow->viewConfigCount = 0;
	subWindow->oldViewConfigCount = 0;

	initializeLines((SNODE *)subWindow->lines);
}

/***************************************************
  setViewConfigCount
****************************************************/
void setViewConfigCount(struct subWindow *subWindow,int count)
{
	subWindow->viewConfigCount = count;
	/*
	     subWindow->oldViewConfigCount = 0;
	*/
}

/***************************************************
  invokeSubWindowUpdate
****************************************************/
void invokeSubWindowUpdate(struct subWindow *subWindow)
{
	SLIST *lines;
	struct anyLine *line;
	void  *link=0;
	int linkType;

	if (subWindow->modified){
		redraw(subWindow,0);
		subWindow->modified = 0;
		return;
	}

	lines = subWindow->lines;
	if (!lines) return;

	/* update modified lines */
	line = (struct anyLine *)sllFirst(lines);
	while (line){
		invokeLinkUpdate((GCLINK *)line->link,line->linkType);
		if (line->link ){
			link = line->link;
			linkType = line->linkType;
		}
		line = (struct anyLine *)sllNext(line);
	}

	/* update the severity below indicator */
	if (link) {
		link = (subWindow->alViewNext)(link,&linkType);
		if (link) initSevrBelow(subWindow,link);
	}

	/* update the severity above indicator */
	link = (subWindow->alViewNth)(subWindow->parentLink,&linkType,0);
	initSevrAbove(subWindow,link);
}

/***************************************************
  createSubWindow
****************************************************/
struct subWindow *createSubWindow(void *area)
{
	struct subWindow  *subWindow;

	subWindow =(struct subWindow *)calloc(1,sizeof(struct subWindow));

	subWindow->area = (void *)area;

	/* allocate and initialize a list for line data */
	subWindow->lines = (SLIST *)calloc(1,sizeof(SLIST));
	sllInit(subWindow->lines);

	return(subWindow);
}

/***************************************************
  createSubWindowWidgets
****************************************************/
void createSubWindowWidgets(struct subWindow *subWindow,Widget parent)
{
	Pixel          color=0;
	Dimension      width=0;

	/* Create a Form for sevr indicators and scrollbar */
	subWindow->form_vsb = XtVaCreateWidget("form_vsb",
	    xmFormWidgetClass,         parent,
	    XmNorientation,            XmVERTICAL,
	    XmNadjustLast,             TRUE,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNrightAttachment,        XmATTACH_FORM,
	    XmNbottomAttachment,       XmATTACH_FORM,
	    NULL);

	subWindow->sevrAboveInd = XtVaCreateManagedWidget("sevrAboveInd",
	    xmLabelWidgetClass,        subWindow->form_vsb,
	    XmNlabelType,              XmPIXMAP,
	    XmNborderWidth,            1,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNrightAttachment,        XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);

	subWindow->sevrBelowInd = XtVaCreateManagedWidget(" ",
	    xmLabelWidgetClass,        subWindow->form_vsb,
	    XmNlabelType,              XmPIXMAP,
	    XmNborderWidth,            1,
	    XmNrightAttachment,        XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    XmNbottomAttachment,       XmATTACH_FORM,
	    NULL);

	/* Create vertical scrollbar for the tree area */
	subWindow->vsb = XtVaCreateManagedWidget("vsb",
	    xmScrollBarWidgetClass,    subWindow->form_vsb,
	    XmNorientation,            XmVERTICAL,
	    XmNrepeatDelay,            200,
	    XmNtopAttachment,          XmATTACH_WIDGET,
	    XmNtopWidget,              subWindow->sevrAboveInd,
	    XmNbottomAttachment,       XmATTACH_WIDGET,
	    XmNbottomWidget,           subWindow->sevrBelowInd,
	    XmNrightAttachment,        XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);


	/* manage the Form for sevr indicators and scrollbar in the sub */
	XtManageChild(subWindow->form_vsb);

	/* use same callback for all callback reasons */
	XtAddCallback(subWindow->vsb,
	    XmNvalueChangedCallback,
	    (XtCallbackProc)scrollBarMovedCallback, subWindow);
	XtAddCallback(subWindow->vsb,
	    XmNdragCallback,
	    (XtCallbackProc)scrollBarMovedCallback, subWindow);

	XtVaGetValues(subWindow->vsb,
	    XmNwidth,                  &width,
	    XmNtroughColor,            &color,
	    NULL);

	XtVaSetValues(subWindow->sevrAboveInd,
	    XmNbackground,             color,
	    XmNheight,                 width,
	    XmNwidth,                  width,
	    NULL);
	XtVaSetValues(subWindow->sevrBelowInd,
	    XmNbackground,             color,
	    XmNheight,                 width,
	    XmNwidth,                  width,
	    NULL);

	/* Create DrawingArea to hold the line widgets */
	subWindow->drawing_area = XtVaCreateManagedWidget("drawing_area",
	    xmDrawingAreaWidgetClass,  parent,
	    XmNresizePolicy,           XmRESIZE_NONE,
	    XmNmarginHeight,           3,
	    XmNspacing,                0,
	    XmNtopAttachment,          XmATTACH_FORM,
	    XmNbottomAttachment,       XmATTACH_FORM,
	    XmNleftAttachment,         XmATTACH_FORM,
	    NULL);
}

/***************************************************
  calcRowCount
****************************************************/
int calcRowCount(struct subWindow *subWindow)
{
	int viewRowCount;
	viewRowCount =(int)((int)(subWindow->viewHeight-2*subWindow->marginHeight)/
	    (int)(2+subWindow->rowHeight));
	return(viewRowCount);
}

/***************************************************
  calcRowYValue
****************************************************/
int calcRowYValue(struct subWindow *subWindow,int lineNo)
{
	int yValue;
	yValue = (int)(2+subWindow->rowHeight)*lineNo + subWindow->marginHeight;

	return(yValue);
}

/***************************************************
  adjustScrollBar
****************************************************/
void adjustScrollBar(struct subWindow *subWindow)
{
	/* adjust scrollbar and slider if viewRowCount or viewConfigCount changed*/
	if ( !subWindow->viewRowCount ||  !subWindow->viewConfigCount)
		XtUnmanageChild(subWindow->form_vsb);

	if ( subWindow->viewRowCount != subWindow->oldViewRowCount ||
	    subWindow->viewConfigCount != subWindow->oldViewConfigCount ){

		if ( XtIsManaged(subWindow->form_vsb) == TRUE )
			XtUnmanageChild(subWindow->form_vsb);

		/* set slider values */
		XtVaSetValues(subWindow->vsb,
		    XmNvalue,         subWindow->viewOffset,
		    XmNmaximum,       Mmax(subWindow->viewRowCount,
		    subWindow->viewConfigCount),
		    XmNsliderSize,    Mmax(subWindow->viewRowCount, 1),
		    XmNpageIncrement, Mmax(subWindow->viewRowCount-1, 1),
		    NULL);

		/* manage/unmanage scrollbar */
		if ( subWindow->viewConfigCount <= subWindow->viewRowCount ){
			if ( XtIsManaged(subWindow->form_vsb) == TRUE ){
				XtUnmanageChild(subWindow->form_vsb);
			}
		} else {
			if ( XtIsManaged(subWindow->form_vsb) == FALSE ){
				XtManageChild(subWindow->form_vsb);
			}
		}

		subWindow->oldViewConfigCount = subWindow->viewConfigCount;
		subWindow->oldViewRowCount = subWindow->viewRowCount;
	}
}

/***************************************************
  exposeResizeCallback
****************************************************/
void exposeResizeCallback(Widget widget,struct subWindow *subWindow,
XEvent *cbs)
{
	Dimension oldViewHeight;

	/*
	     if (cbs->reason == XmCR_EXPOSE){
	          return;
	     }
	*/
	oldViewHeight = subWindow->viewHeight;
	XtVaGetValues(subWindow->drawing_area,
	    XmNheight,            &subWindow->viewHeight,
	    XmNmarginHeight,      &subWindow->marginHeight,
	    NULL);

	subWindow->oldViewRowCount = subWindow->viewRowCount;
	if ( subWindow->rowHeight ){
		subWindow->viewRowCount = calcRowCount(subWindow);
	}

	if (subWindow->viewRowCount <= 0) return;

	redraw(subWindow,0);

	/*  because redraw is slow we get into trouble with following stmnts
	    with many frequent calls to resize 
	*/
#if 0
	if (subWindow->viewHeight == oldViewHeight) return;

	if (subWindow->viewHeight < oldViewHeight) {
		if (subWindow->viewRowCount < subWindow->viewConfigCount){
			redraw(subWindow,subWindow->viewRowCount);
		} else return;
	} else{
		redraw(subWindow,subWindow->oldViewRowCount);
	}
#endif
}

/***************************************************
  defaultTreeSelection
****************************************************/
void defaultTreeSelection(ALINK *area)
{
	struct subWindow  *treeWindow;
	struct subWindow  *groupWindow;
	struct anyLine       *line;
	WLINE                *wline;

	treeWindow = (struct subWindow  *)area->treeWindow;
	groupWindow = (struct subWindow  *)area->groupWindow;
	if (!treeWindow->lines) return;
	line = (struct anyLine *)sllFirst(treeWindow->lines);
	if (!line) return;
	wline = (WLINE *)(line->wline);

	markSelectedWidget(treeWindow,wline->name);
	markSelection(treeWindow, line);

	/* groupWindow must now display contents of new treeWindow selection */
	groupWindow->parentLink = line->link;
	groupWindow->viewConfigCount = 
	    alViewAdjustGroupW((GLINK *)line->link,area->viewFilter);
	groupWindow->viewOffset = 0;
	redraw(groupWindow,0);
}

/***************************************************
  initSevrAbove
****************************************************/
void initSevrAbove(struct subWindow *subWindow,void *link)
{
	int    sevrAbove;

	if ( XtIsManaged(subWindow->form_vsb) == TRUE ){

		sevrAbove = (subWindow->alViewMaxSevrN)(link,subWindow->viewOffset);
#if  XmVersion && XmVersion >= 1002
		XmChangeColor(subWindow->sevrAboveInd,bg_pixel[sevrAbove]);
#else
		XtVaSetValues(subWindow->sevrAboveInd,XmNbackground,
		    bg_pixel[sevrAbove],NULL);
#endif
	}
}

/***************************************************
  initSevrBelow
****************************************************/
void initSevrBelow(struct subWindow *subWindow,void *link)
{
	int    sevrBelow,n;

	if ( XtIsManaged(subWindow->form_vsb) == TRUE ){

		n = subWindow->viewConfigCount - subWindow->viewOffset - subWindow->viewRowCount;

		sevrBelow = (subWindow->alViewMaxSevrN)(link,n);
		if (sevrBelow < 0 ) return;
#if  XmVersion && XmVersion >= 1002
		XmChangeColor(subWindow->sevrBelowInd,bg_pixel[sevrBelow]);
#else
		XtVaSetValues(subWindow->sevrBelowInd,XmNbackground,bg_pixel[sevrBelow],NULL);
#endif
	}
}

