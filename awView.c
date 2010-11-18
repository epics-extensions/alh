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
/* awView.c */

/************************DESCRIPTION***********************************
  This file contains routines for altering the users view
**********************************************************************/

#define DEBUG_CALLBACKS 0

#include <time.h>

#include "alh.h"
#include "alLib.h"
#include "sllLib.h"
#include "axArea.h"
#include "axSubW.h"
#include "line.h"
#include "ax.h"

#define MINIMUM_MULTICLICKTIME 500

/*  structures for arrow button single vs double click action  */
struct timeoutData {
	Widget        pushButton;
	void         *gdata;
	XtIntervalId timeoutId;
};

/***************************************************
  doubleClickNameGroupW
****************************************************/
static void doubleClickNameGroupW(Widget pushButton,struct anyLine *line)
{
	struct subWindow  *treeWindow;
	struct subWindow  *groupWindow;
	GLINK     *parent;
	GCLINK     *link;
	WLINE     *wline;
	ALINK *area;
	int grandparentsOpen;
	GLINK     *glinkTemp;

	struct anyLine *l;

        XtVaGetValues(pushButton, XmNuserData, &l, NULL);
        groupWindow = l->pwindow;

	area = groupWindow->area;

	if (line->linkType == GROUP ) {

		link = line->link;
		parent = link->parent;

		treeWindow = area->treeWindow;

		/* update tree window  */
		grandparentsOpen = TRUE;
		glinkTemp=parent->parent;
		while (glinkTemp){
			if ( glinkTemp->viewCount <= 1 ) grandparentsOpen = FALSE;
			glinkTemp = glinkTemp->parent;
		}
		if ( grandparentsOpen ){
			if ( parent->viewCount <= 1 ){
				displayNewViewTree(area,parent,EXPANDCOLLAPSE1);
			}
		}

		/*groupWindow must now display contents of new treeWindow selection */
		markSelectedWidget(groupWindow,NULL);
		markSelection(groupWindow, NULL);
		groupWindow->parentLink = link;
		groupWindow->viewConfigCount = alViewAdjustGroupW((GLINK *)link,
		    area->viewFilter);
		groupWindow->viewOffset = 0;
		redraw(groupWindow,0);

		/* markSelection */
		area->selectionLink = link;
		area->selectionType = GROUP;
		area->selectionWindow = treeWindow;
		treeWindow->selectionLink = link;

		/* update dialog windows */
		axUpdateDialogs(area);

		/* markSelectedWidget */
		if (link->lineTreeW) {
			wline = ((struct anyLine *)link->lineTreeW)->wline;
			markSelectedWidget(treeWindow,wline->name);
		} else {
			markSelectedWidget(treeWindow,NULL);
		}
	} else {
		markSelectedWidget(groupWindow,pushButton);
		markSelection(groupWindow, line);

		/* update dialog windows if displayed */
		axUpdateDialogs(area);
	}
}

/***************************************************
  nameTreeW_callback
****************************************************/
void nameTreeW_callback(Widget pushButton,struct anyLine *line,
XmPushButtonCallbackStruct *cbs)
{
	struct subWindow  *subWindow;
	struct subWindow  *groupWindow;

	struct anyLine *l;

	if (pushButton) {
	  XtVaGetValues(pushButton, XmNuserData, &l, NULL);
	  subWindow = l->pwindow;
	}
	else {
	  return;
	}

	/* groupWindow must now display contents of new treeWindow selection */
	groupWindow = ((ALINK *)subWindow->area)->groupWindow;
	markSelectedWidget(groupWindow,NULL);
	markSelection(groupWindow, NULL);
	if (line) {
		groupWindow->parentLink = line->link;
		groupWindow->viewConfigCount = alViewAdjustGroupW((GLINK *)line->link,
		    ((ALINK *)subWindow->area)->viewFilter);
	}
	groupWindow->viewOffset = 0;
	redraw(groupWindow,0);

	markSelectedWidget(subWindow,pushButton);
	markSelection(subWindow, line);

	/* update dialog windows */
	axUpdateDialogs(subWindow->area);
}

/******************************************************
  arrowTreeW_timer_callback
******************************************************/
static void arrowTreeW_timer_callback(XtPointer cd, XtIntervalId *id)
{
	struct timeoutData *pdata = (struct timeoutData *)cd;

        arrowTreeW_callback(pdata->pushButton,pdata->gdata,NULL);
}

/******************************************************
  singleClickArrowTreeW
******************************************************/
static void singleClickArrowTreeW(Widget pushButton,void *glink)
{
	ALINK  *area;

	XtVaGetValues(pushButton, XmNuserData, &area, NULL);
	displayNewViewTree(area,(GLINK *)glink,EXPANDCOLLAPSE1);
}

/***************************************************
  doubleClickArrowTreeW
****************************************************/
static void doubleClickArrowTreeW(Widget pushButton,void *glink)
{
	ALINK  *area;

	XtVaGetValues(pushButton, XmNuserData, &area, NULL);
	displayNewViewTree(area,glink,EXPAND);
}

/***************************************************
  arrowTreeW_callback
****************************************************/
void arrowTreeW_callback(Widget pushButton,void *glink,
XmPushButtonCallbackStruct *cbs)
{
	static unsigned long interval=0;
	static unsigned int clicks=0;
	static struct timeoutData data;

	if (cbs){
	    if (clicks == 0){
		/* Get multi-click time in ms */
		if (!interval) {
                    interval = XtGetMultiClickTime(display);
                    if (interval < MINIMUM_MULTICLICKTIME ) interval=MINIMUM_MULTICLICKTIME;
		}
		data.pushButton = pushButton;
		data.gdata = glink;
		data.timeoutId= XtAppAddTimeOut(appContext,interval,
			    arrowTreeW_timer_callback,(XtPointer)&data);
                clicks=1;
	    } else if (clicks == 1) {
		doubleClickArrowTreeW(pushButton,glink);
                clicks=0;
            } else {
                    clicks++;
            }
        } else {
	    if (clicks == 1) {
		singleClickArrowTreeW(pushButton,glink);
	    }
            clicks=0;
            data.timeoutId=0;
	}
}

/******************************************************
  nameGroupW_timer_callback
******************************************************/
static void nameGroupW_timer_callback(XtPointer cd, XtIntervalId *id)
{
	struct timeoutData *pdata = (struct timeoutData *)cd;

        nameGroupW_callback(pdata->pushButton,pdata->gdata,NULL);
}

/******************************************************
  singleClickNameGroupW
******************************************************/
static void singleClickNameGroupW(Widget pushButton,struct anyLine *line)
{
	void               *area;
	struct subWindow  *groupWindow;
	struct anyLine *l;

        XtVaGetValues(pushButton, XmNuserData, &l, NULL);
        groupWindow = l->pwindow;
	markSelectedWidget(groupWindow,pushButton);
	markSelection(groupWindow,line);

	/* update dialog windows if displayed */
	area = groupWindow->area;
	axUpdateDialogs(area);

}

/***************************************************
  nameGroupW_callback
****************************************************/
void nameGroupW_callback(Widget pushButton,struct anyLine *line,
XmPushButtonCallbackStruct *cbs)
{
	static unsigned long interval=0;
	static unsigned int clicks=0;
	static struct timeoutData data;
	void *area;
  	struct subWindow  *groupWindow;
	struct anyLine *l;

	if (line->linkType != GROUP ) {
                XtVaGetValues(pushButton, XmNuserData, &l, NULL);
                groupWindow = l->pwindow;
		markSelectedWidget(groupWindow,pushButton);
		markSelection(groupWindow, line);

		/* update dialog windows if displayed */
	        area = groupWindow->area;
		axUpdateDialogs(area);
		return;
	}

	if (cbs){
	    if (clicks == 0){
		/* Get multi-click time in ms */
		if (!interval) {
                    interval = XtGetMultiClickTime(display);
                    if (interval < MINIMUM_MULTICLICKTIME ) interval=MINIMUM_MULTICLICKTIME;
		}
		data.pushButton = pushButton;
		data.gdata = line;
		data.timeoutId= XtAppAddTimeOut(appContext,interval,
			    nameGroupW_timer_callback,(XtPointer)&data);
                clicks=1;
	    } else if (clicks == 1) {
		doubleClickNameGroupW(pushButton,line);
                clicks=0;
            } else {
                    clicks++;
            }
        } else {
	    if (clicks == 1) {
		singleClickNameGroupW(pushButton,line);
	    }
            clicks=0;
            data.timeoutId=0;
	}
}

/******************************************************
  arrowGroupW_timer_callback
******************************************************/
static void arrowGroupW_timer_callback(XtPointer cd, XtIntervalId *id)
{
	struct timeoutData *pdata = (struct timeoutData *)cd;

        arrowGroupW_callback(pdata->pushButton,pdata->gdata,NULL);
}

/******************************************************
  singleClickArrowGroupW
******************************************************/
static void singleClickArrowGroupW(Widget pushButton,void *link)
{
	GLINK     *glink=(GLINK *)link;
	ALINK     *area;
	GLINK     *parent;
	GLINK     *glinkTemp;
	int grandparentsOpen;

#if DEBUG_CALLBACKS
	static int n=0;
	printf("singleClickArrowGroupW: n=%d\n",n++);
#endif

	XtVaGetValues(pushButton, XmNuserData, &area, NULL);

	/* update tree window  */
	grandparentsOpen = TRUE;
	parent = glink->parent;
	glinkTemp=parent->parent;
	while (glinkTemp){
		if ( glinkTemp->viewCount <= 1 ) grandparentsOpen = FALSE;
		glinkTemp = glinkTemp->parent;
	}
	if ( grandparentsOpen ){
		if ( parent->viewCount <= 1 ){
			displayNewViewTree(area,parent,EXPANDCOLLAPSE1);
		}
		displayNewViewTree(area,(void*)glink,EXPANDCOLLAPSE1);
	}
}

/******************************************************
  doubleClickArrowGroupW
******************************************************/
static void doubleClickArrowGroupW(Widget pushButton,void *link)
{
	ALINK     *area;

        XtVaGetValues(pushButton, XmNuserData, &area, NULL);
        singleClickArrowGroupW(pushButton,link);
        displayNewViewTree(area,(GLINK*)link,EXPAND);
}

/***************************************************
  arrowGroupW_callback
****************************************************/
void arrowGroupW_callback(Widget pushButton,void *glink,
XmPushButtonCallbackStruct *cbs)
{
	static unsigned long interval=0;
	static unsigned int clicks=0;
	static struct timeoutData data;

	if (cbs){
	    if (clicks == 0){
		/* Get multi-click time in ms */
		if (!interval) {
                    interval = XtGetMultiClickTime(display);
                    if (interval < MINIMUM_MULTICLICKTIME ) interval=MINIMUM_MULTICLICKTIME;
		}
		data.pushButton = pushButton;
		data.gdata = glink;
		data.timeoutId= XtAppAddTimeOut(appContext,interval,
			    arrowGroupW_timer_callback,(XtPointer)&data);
                clicks=1;
	    } else if (clicks == 1) {
		doubleClickArrowGroupW(pushButton,glink);
                clicks=0;
            } else {
                    clicks++;
            }
        } else {
	    if (clicks == 1) {
		singleClickArrowGroupW(pushButton,glink);
	    }
            clicks=0;
            data.timeoutId=0;
	}
}

/***************************************************
  markSelection
****************************************************/
void markSelection(struct subWindow *subWindow,struct anyLine *line)
{
	if (!line) subWindow->selectionLink = 0;
	else subWindow->selectionLink = line->link;

	markSelectionArea(subWindow->area,line);
	return;
}

/***************************************************
  createConfigDisplay
****************************************************/
void createConfigDisplay(ALINK *area,int expansion)
{
	GLINK     *glinkTop;
	int        viewConfigCount;

	/* are the following 2 lines necessary??? */
	initializeSubWindow(area->treeWindow);
	initializeSubWindow(area->groupWindow);

	if ( !area->pmainGroup ) return;

	glinkTop = (GLINK *)sllFirst(area->pmainGroup);
	glinkTop->viewCount = 0;

	viewConfigCount = alViewAdjustTreeW(glinkTop, expansion, area->viewFilter);
	setViewConfigCount(area->treeWindow,viewConfigCount);
	setParentLink(area->treeWindow,glinkTop);

	viewConfigCount = alViewAdjustGroupW((void *)glinkTop,area->viewFilter);
	setViewConfigCount(area->groupWindow,viewConfigCount);
	setParentLink(area->groupWindow,glinkTop);

	/* Create Tree Display:  starting at top level */
	if (area->mapped){
		redraw(area->treeWindow,0);

		/* mark first line as treeWindow selection */
		/* and redraw groupWindow */
		defaultTreeSelection(area);
	}
}

/***************************************************
  displayNewViewTree
****************************************************/
void displayNewViewTree(ALINK *area,GLINK *glink,int command)
{
	struct anyLine    *line;
	int viewConfigCount;

	if (!glink) return;

	viewConfigCount = alViewAdjustTreeW(glink, command,area->viewFilter);
	setViewConfigCount(area->treeWindow,viewConfigCount);
	line = (struct anyLine *)glink->lineTreeW;
	if (line) redraw(area->treeWindow, line->lineNo);
	else redraw(area->treeWindow,0 );
}

/***************************************************
  redraw
****************************************************/
void redraw(struct subWindow *subWindow,int rowNumber)
{
	struct anyLine *line=0;
	struct anyLine *ptline;
	int  row, r, linkType;
	GCLINK *link;
	GCLINK *ptlink;
	GCLINK *linkOld;
	WLINE *wline;
	WLINE *ptwline;
	SNODE *pt;

	row = rowNumber;

	/* adjust view offset if more groups will fit on display */
	if (subWindow->viewOffset && subWindow->viewRowCount &&
	    (int)subWindow->viewOffset + subWindow->viewRowCount >
	    subWindow->viewConfigCount ){

		subWindow->viewOffset = Mmax(subWindow->viewConfigCount -
		    subWindow->viewRowCount,0);
		row = 0;
	}

	r = 0;
	if (subWindow->lines) {
		line = (struct anyLine *)sllFirst(subWindow->lines);
		while (line){
			if (line->lineNo >= row) break;
			line = (struct anyLine *)sllNext(line);
			r++;
		}
	}
	row = r;


	link = (GCLINK *)(subWindow->alViewNth)(subWindow->parentLink,&linkType,0);
	initSevrAbove(subWindow,link);

	link = (GCLINK *)(subWindow->alViewNth)(subWindow->parentLink,
	    &linkType,subWindow->viewOffset+row);


	while ( link ){

		if (!line){
			line = (struct anyLine *)awAllocLine();
			sllAdd(subWindow->lines,(SNODE *)line);
			wline = (WLINE *)XtCalloc(1 , sizeof(WLINE));
			line->wline = (void *)wline;
		} else {
			linkOld = (GCLINK *)line->link;
			if (linkOld){
				line->linkType = 0;
				line->link = NULL;
				line->cosCallback = NULL;
				linkOld->modified = 0;
				if(isTreeWindow(subWindow->area,subWindow)){
					if ( (struct anyLine *)linkOld->lineTreeW &&
					    ((struct anyLine *)linkOld->lineTreeW)->lineNo >= row)
						linkOld->lineTreeW = NULL;
				} else {
					if ( (struct anyLine *)linkOld->lineGroupW &&
					    ((struct anyLine *)linkOld->lineGroupW)->lineNo >= row)
						linkOld->lineGroupW = NULL;
				}
			}
		}
		line->pwindow= (void *)subWindow;
		line->lineNo = row;
		line->linkType = linkType;
		wline = (WLINE *)line->wline;
		if (wline->name && subWindow->selectionWidget &&
		    wline->name == subWindow->selectionWidget)
			markSelectedWidget(subWindow,NULL);
		line->link = (void *)link;
		line->pname = ((GCLINK *)link)->pgcData->name;
		if (((GCLINK *)link)->pgcData->alias){
			line->alias = ((GCLINK *)link)->pgcData->alias;
		} else {
			line->alias = ((GCLINK *)link)->pgcData->name;
		}
		if(isTreeWindow(subWindow->area,subWindow))
			link->lineTreeW = (void *)line;
		else link->lineGroupW = (void *)line;
		if (linkType == GROUP){
			awUpdateGroupLine(line);
		} else if (linkType == CHANNEL){
			awUpdateChanLine(line);
		}

		/* call subwindow create/change row widgets routine */
		awRowWidgets(line,subWindow->area);

		if (line->link == subWindow->selectionLink){
			markSelectedWidget(subWindow,wline->name);
		}


		/* determine viewRowCount if not already set */
		if ( !subWindow->rowHeight){

			XtVaGetValues(subWindow->drawing_area,
			    XmNheight,            &subWindow->viewHeight,
			    XmNmarginHeight,      &subWindow->marginHeight,
			    NULL);

			XtVaGetValues(wline->row_widget,
			    XmNheight, &subWindow->rowHeight,
			    NULL);

			/* ****** NOTE: ALL ROWS MUST BE THE SAME HEIGHT*********** */
			subWindow->viewRowCount = calcRowCount(subWindow);

		}

		if (line ) line = (struct anyLine *)sllNext(line);
		row++;
		if (row >= subWindow->viewRowCount) break;
		if (row >= subWindow->viewConfigCount) break;

		link = (GCLINK *)(subWindow->alViewNext)(link,&linkType);
	}

	/* adjustManagedRows */
	pt = (SNODE *)line;
	while (pt){
		ptline = (struct anyLine *)pt;
		ptwline = (WLINE *)ptline->wline;
		if (XtIsManaged(ptwline->row_widget) == TRUE ){
			XtUnmanageChild(ptwline->row_widget);
		}
		ptlink = (GCLINK *)ptline->link;
		if (ptlink) {
			ptlink->modified = 0;
			if (isTreeWindow(subWindow->area, subWindow) ) {
				ptlink->lineTreeW = NULL;
			} else {
				ptlink->lineGroupW = NULL;
			}
		}
		initLine(ptline);
		pt = sllNext(pt);
	}

	adjustScrollBar(subWindow);

	if (link) {
		link = (GCLINK *)(subWindow->alViewNext)(link,&linkType);
		initSevrBelow(subWindow,link);
	}
}

/***************************************************
  invokeLinkUpdate
****************************************************/
void invokeLinkUpdate(GCLINK *link,int linkType)
{
	void *line;

	if (link && link->modified ){
		line = link->lineGroupW;
		if (line) {
			if (linkType == GROUP) awUpdateGroupLine(line);
			else if (linkType == CHANNEL) awUpdateChanLine(line);
			awUpdateRowWidgets(line);
		}
		line  = link->lineTreeW;
		if (line) {
			if (linkType == GROUP) awUpdateGroupLine(line);
			else if (linkType == CHANNEL) awUpdateChanLine(line);
			awUpdateRowWidgets(line);
		}
		link->modified = 0;
	}
}

/***************************************************
  awViewAddNewAlarm
****************************************************/
void awViewAddNewAlarm (CLINK *clink,int prevCount,int count)
{
	ALINK *area;
	int prevViewCount;
	int newLineTree;
	GLINK *glink;
	GLINK *viewParent;
	GLINK *prevLink;
	GLINK *addViewLink=NULL;
	struct subWindow *subWindowTree;
	struct subWindow *subWindowGroup;

	area = clink->pmainGroup->area;

	if (!area || prevCount || !count) return;

	newLineTree = FALSE;
	viewParent = NULL;
	prevViewCount = 1;
	prevLink = 0;

	glink = clink->parent;
	if (!glink) return;
	while (glink) {
		/* view parent open */
		if (glink->viewCount >1 && !prevViewCount) {
			viewParent = glink;
			addViewLink = prevLink;
			if (prevLink) newLineTree = TRUE;
		}
		/* view parent NOT open */
		if (glink->viewCount == 1) {
			viewParent = glink;
			addViewLink = prevLink;
			newLineTree = FALSE;
		}
		prevLink = glink;
		prevViewCount = glink->viewCount;
		if (newLineTree) glink->viewCount++;
		glink = glink->parent;

	}

	subWindowTree = (struct subWindow *)area->treeWindow;
	subWindowGroup = (struct subWindow *)area->groupWindow;

	if (!viewParent || !prevViewCount) {
		addViewLink = prevLink;
		viewParent = prevLink;
		newLineTree = TRUE;
	}

	if (newLineTree) {
		subWindowTree->modified = 1;
		clink->pmainGroup->modified = 1;
		subWindowTree->viewConfigCount++;
		if (addViewLink) addViewLink->viewCount=1;
		if (viewParent){
			if (newLineTree)
				alViewAdjustTreeW(viewParent,NOCHANGE,area->viewFilter);
		}
	}
	if (!(GLINK *)subWindowGroup->parentLink ||
	    (viewParent == (GLINK *)subWindowGroup->parentLink)) {
		subWindowGroup->modified =1;
		clink->pmainGroup->modified = 1;
		subWindowGroup->viewConfigCount++;
		if (addViewLink) addViewLink->viewCount=1;
	}
}

/******************************************************
  awViewNewGroup
******************************************************/
void awViewNewGroup(ALINK *area,GCLINK *link)
{
	struct subWindow  *groupWindow,*treeWindow;
	GLINK *parent, *glink;
	GCLINK *gclink;
	int diffCount, line;

	treeWindow = area->treeWindow;
	groupWindow = area->groupWindow;
	parent = (GLINK *)link->parent;

	diffCount = link->viewCount;
	/* update changed treeWindow */
	if (parent && parent->viewCount > 1) {

		/* adjust viewCounts of open parents */
		glink=parent;
		while (glink){
			glink->viewCount += diffCount;
			glink = (GLINK *)glink->parent;
		}

		/* adjust treeWindow viewCount */
		treeWindow->viewConfigCount += diffCount;

		/* update treeSym for treeWindow */
		alViewAdjustTreeW(parent, NOCHANGE, area->viewFilter);

		/* redraw  treeWindow */
		line = 0;
		if (parent->lineTreeW) {
			line = ((struct anyLine *)parent->lineTreeW)->lineNo;
		}
		redraw(treeWindow,line);

	}

	/* update groupWindow  configCount*/
	if (parent == groupWindow->parentLink){
		groupWindow->viewConfigCount++;

		gclink = groupWindow->selectionLink;
		if (gclink) {
			line = ((struct anyLine *)gclink->lineGroupW)->lineNo;
			if ( link && (line == groupWindow->viewRowCount) ){
				groupWindow->viewOffset++;
			}
		}
		redraw(groupWindow,0);
	}
}

/******************************************************
  awViewNewChan
******************************************************/
void awViewNewChan(ALINK *area,GCLINK *link)
{
	struct subWindow  *groupWindow;
	int line;
	GCLINK *gclink;

	groupWindow = area->groupWindow;

	/* adjust groupWindow  configCount*/
	groupWindow->viewConfigCount++;

	/* redraw  groupWindow */
	gclink = groupWindow->selectionLink;
	if (gclink){
		line = ((struct anyLine *)gclink->lineGroupW)->lineNo;
		if ( link && (line == groupWindow->viewRowCount) ){
			groupWindow->viewOffset++;
		}
	}
	redraw(groupWindow,0);
}

/******************************************************
  awViewViewCount
******************************************************/
int awViewViewCount(GCLINK *gclink)
{
	int viewCount = 0;

	if (gclink->pmainGroup->area)
		viewCount = (((ALINK *)gclink->pmainGroup->area)->viewFilter)(gclink);

	return(viewCount);
}

/******************************************************
  invokeDialogUpdate
******************************************************/

void invokeDialogUpdate(ALINK *area)
{
	GCLINK *gclink;

	gclink = area->selectionLink;
	if (gclink && gclink->modified) axUpdateDialogs(area);
}

