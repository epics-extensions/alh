/*
 $Log$
 Revision 1.7  1998/05/12 18:22:47  evans
 Initial changes for WIN32.

 Revision 1.6  1997/09/12 19:37:51  jba
 Removed comments.

 Revision 1.5  1995/05/31 20:34:15  jba
 Added name selection and arrow functions to Group window

 * Revision 1.4  1995/05/30  15:58:02  jba
 * Added ALARMCOMMAND facility
 *
 * Revision 1.3  1995/03/24  16:35:53  jba
 * Bug fix and reorganized some files
 *
 * Revision 1.2  1994/06/22  21:17:13  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)axSubW.c	1.7\t10/1/93";

/* axSubW.c */
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
 * .01	mm-dd-yy		nnn	Description
 */

/*
******************************************************************
        routines defined in axSubW.c
******************************************************************
*
*       This file contains subWindow handling routines.
*
******************************************************************

-------------
|   PUBLIC  |
-------------
*
void 
scrollBarMovedCallback(widget,subWindow,cbs) Adjust viewOffset when
     Widget widget;                          subWindow scrollbar is moved
     struct subWindow *subWindow;
     XmScrollBarCallbackStruct *cbs;

*
void setParentLink(subWindow,link)           Set parentLink for subWindow
     struct subWindow *subWindow;
     void *link;

*
void setLineRoutine(area,subWindow,program)          Set subWindow routine names 
     void *area;
     struct subWindow *subWindow;
     int program;

*
void markSelectedWidget(subWindow,newWidget)  Mark new and unmark old widgets
     struct subWindow  *subWindow;         for subWindow
     Widget        newWidget;

*
void initializeSubWindow(subWindow)           Initialize subWindow fields
     struct subWindow  *subWindow;

*
void setViewConfigCount(subWindow,count)      Put count in subWindow viewCount field
     struct subWindow  *subWindow;
     int count;

*
void invokeSubWindowUpdate(subWindow)         Update each line of subWindow
     struct subWindow  *subWindow;
*
struct subWindow *createSubWindow(area)    Alloc space for a subWindow structure
     void                 *area;
*
void createSubWindowWidgets(subWindow,parent) Create all subWindow widgets
     struct subWindow  *subWindow;
     Widget                parent;
*
int calcRowCount(subWindow)                   Calculate the number of rows(lines) in 
     struct subWindow *subWindow;             the subWindow
*
int calcRowYValue(subWindow,lineNo)           Calculate the subWindow y value for the
     struct subWindow *subWindow;             row(line)
     int lineNo;
*
void adjustScrollBar(subWindow)               Set new scrollbar values when subWindow
     struct subWindow *subWindow;             view has changed
*
void 
exposeResizeCallback(widget, subWindow, cbs)  Redraw subWindows if resize has occurred
     Widget widget;
     struct subWindow *subWindow;
     XmDrawingAreaCallbackStruct *cbs;
*
void defaultTreeSelection(area)               Make 1st line treeWindow default selection
      ALINK * area;
*
void initSevrAbove(subWindow,link)            Initialize severity above indicator
     struct subWindow  *subWindow;
     void *link;
*
void initSevrBelow(subWindow,link)            Initialize severity below indicator
     struct subWindow  *subWindow;
     void *link;
*
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

#include <alh.h>
#include <sllLib.h>
#include <line.h>
#include <axSubW.h>
#include <axArea.h>
#include <ax.h>


/* globals */
extern Pixel bg_pixel[ALARM_NSEV];

#ifdef __STDC__

/* prototypes for static routines */
static void scrollBarMovedCallback( Widget widget, struct subWindow *subWindow,
           XmScrollBarCallbackStruct *cbs);

#else

static void scrollBarMovedCallback();

#endif /*__STDC__*/



/***************************************************
  scrollBarMovedCallback
****************************************************/

static void scrollBarMovedCallback(widget,subWindow,cbs)
     Widget widget;
     struct subWindow *subWindow;
     XmScrollBarCallbackStruct *cbs;
{

     if ((int)subWindow->viewOffset == cbs->value) return;

     subWindow->viewOffset = cbs->value;
     redraw(subWindow,0);
}

/***************************************************
  setParentLink
****************************************************/

void setParentLink(subWindow,link)
     struct subWindow *subWindow;
     void *link;
{
     subWindow->parentLink = link;
}

/***************************************************
  setLineRoutine
****************************************************/

void setLineRoutine(area,subWindow,program)
     void *area;
     struct subWindow *subWindow;
     int program;
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

void markSelectedWidget(subWindow,newWidget)
     struct subWindow  *subWindow;
     Widget        newWidget;
{
     static Pixel armColor=0;
     static Pixel backgroundColor;
     static Pixel bottomShadowColor;
     static Pixel topShadowColor;
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

void initializeSubWindow(subWindow)
     struct subWindow  *subWindow;
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

void setViewConfigCount(subWindow,count)
     struct subWindow  *subWindow;
     int count;
{

     subWindow->viewConfigCount = count;
/*
     subWindow->oldViewConfigCount = 0;
*/
 
}

/***************************************************
  invokeSubWindowUpdate
****************************************************/

void invokeSubWindowUpdate(subWindow)
     struct subWindow  *subWindow;
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

struct subWindow *createSubWindow(area)
     void                 *area;
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

void createSubWindowWidgets(subWindow,parent)
     struct subWindow  *subWindow;
     Widget                parent;
{
     Pixel          color;
     Dimension      width;


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
          XmNvalueChangedCallback, (XtCallbackProc)scrollBarMovedCallback, subWindow);
     XtAddCallback(subWindow->vsb,
          XmNdragCallback,         (XtCallbackProc)scrollBarMovedCallback, subWindow);

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

int calcRowCount(subWindow)
     struct subWindow *subWindow;
{
     int viewRowCount;
     viewRowCount =(int) ((int)(subWindow->viewHeight-2*subWindow->marginHeight)/
          (int)(2+subWindow->rowHeight));
     return(viewRowCount);
}

/***************************************************
  calcRowYValue
****************************************************/

int calcRowYValue(subWindow,lineNo)
     struct subWindow *subWindow;
     int lineNo;
{
     int yValue;
     yValue = (int)(2+subWindow->rowHeight)*lineNo + subWindow->marginHeight;

     return(yValue);
}

/***************************************************
  adjustScrollBar
****************************************************/

void adjustScrollBar(subWindow)
     struct subWindow *subWindow;
{

     /* adjust scrollbar and slider if viewRowCount or viewConfigCount changed */
     if ( !subWindow->viewRowCount ||  !subWindow->viewConfigCount)
          XtUnmanageChild(subWindow->form_vsb);

     if ( subWindow->viewRowCount != subWindow->oldViewRowCount ||
          subWindow->viewConfigCount != subWindow->oldViewConfigCount ){

          if ( XtIsManaged(subWindow->form_vsb) == TRUE )
               XtUnmanageChild(subWindow->form_vsb);
          
          /* set slider values */
          XtVaSetValues(subWindow->vsb,
               XmNvalue,         subWindow->viewOffset,
               XmNmaximum,       Mmax(subWindow->viewRowCount, subWindow->viewConfigCount),
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

void exposeResizeCallback(widget, subWindow, cbs)
     Widget widget;
     struct subWindow *subWindow;
     XEvent *cbs;
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

void defaultTreeSelection(area)
      ALINK *area;
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
     groupWindow->viewConfigCount = alViewAdjustGroupW((GLINK *)line->link,area->viewFilter);
     groupWindow->viewOffset = 0;
     redraw(groupWindow,0);
}

/***************************************************
  initSevrAbove
****************************************************/

void initSevrAbove(subWindow,link)
     struct subWindow  *subWindow;
     void *link;
{
     int    sevrAbove;

     if ( XtIsManaged(subWindow->form_vsb) == TRUE ){

          sevrAbove = (subWindow->alViewMaxSevrN)(link,subWindow->viewOffset);
#if  XmVersion && XmVersion >= 1002
          XmChangeColor(subWindow->sevrAboveInd,bg_pixel[sevrAbove]);
#else
          XtVaSetValues(subWindow->sevrAboveInd,XmNbackground,bg_pixel[sevrAbove],NULL);
#endif

     }
}

/***************************************************
  initSevrBelow
****************************************************/

void initSevrBelow(subWindow,link)
     struct subWindow  *subWindow;
     void *link;
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

