/*
 $Log$
 Revision 1.3  1995/03/24 16:35:49  jba
 Bug fix and reorganized some files

 * Revision 1.2  1994/06/22  21:16:59  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)awView.c	1.14\t10/22/93";

/* awView.c */
/* 
 *      Author: Janet Anderson
 *      Date:   02-16-93
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01  02-16-93        jba     initial implementation
 * .02  10-04-93        jba     Changed from XtAppAddTimeOut to fdmgr_add_timeout
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

#include <sys/time.h>

#include <Xm/Xm.h>

#include <fdmgr.h>

#include <alh.h>
#include <alLib.h>
#include <sllLib.h>
#include <axArea.h>
#include <axSubW.h>
#include <line.h>
#include <ax.h>

extern fdctx *pfdctx;

/*  structures for arrow button single vs double click action  */
struct timeoutData {
     Widget       pushButton;
     void        *glink;
     void        *timeoutId;
};

#ifdef __STDC__

static void singleClickTreeW_callback( struct timeoutData *pdata);

#else

static void singleClickTreeW_callback();

#endif /*__STDC__*/

/*
******************************************************************
	routines defined in awView.c
*******************************************************************
	awView.c 

*
*	Routines for modifying and displaying the config view
*

******************************************************************
-------------
|  PUBLIC  |
-------------
*
void
nameGroupW_callback(pushButton,line,cbs)  Group Window name button callback
     Widget pushButton;
     struct anyLine *line;
     XmPushButtonCallbackStruct *cbs;
*
void 
nameTreeW_callback(pushButton, line, cbs)  Tree Window name button callback
     Widget pushButton;
     struct anyLine   *line;
     XmPushButtonCallbackStruct *cbs;
*
void 
arrow_callback(pushButton,glink,cbs)       Arrow button callback
     Widget     pushButton;
     void     *glink;
     XmPushButtonCallbackStruct *cbs;
*
void createConfigDisplay(area, expansion)  Create initial config
     ALINK     *area;                      view and redraw subWindows
     int        expansion;
*
void displayNewViewTree(area,glink,command)Use command to change config
     ALINK            *area;               view and redraw subWindows
     GLINK            *glink;
     int               command;
*
void redraw(subWindow,rowNumber)           Redraw subW starting at rowNumber
     struct subWindow *subWindow;
     int rowNumber;
*
void invokeLinkUpdate(link,linkType)       Update lines for displayed link
     GCLINK *link;
     int linkType;
*
void markSelection(subWindow,line)         Set selection fields,subW + area
     struct subWindow  *subWindow;
     struct anyLine  *line;
*
void awViewAddNewAlarm(clink)              Modify subWindows view config
     CLINK *clink;
*
void awViewNewGroup(area, link)
     ALINK *area;
     GCLINK *link;
*
void awViewNewChan(area, link)
     ALINK *area;
     GCLINK *link;
*
******************************************************************
-------------
|  PRIVATE  |
-------------
*
static void 
singleClickTreeW_callback(pdata)       Single click timeout callback
     struct timeoutData *pdata;

******************************************************************
*/



/***************************************************
  nameGroupW_callback
****************************************************/

void nameGroupW_callback(pushButton, line, cbs)
     Widget pushButton;
     struct anyLine *line;
     XmPushButtonCallbackStruct *cbs;
{
     struct subWindow  *subWindow;

     XtVaGetValues(pushButton, XmNuserData, &subWindow, NULL);
     markSelectedWidget(subWindow,pushButton);
     markSelection(subWindow, line);

     propUpdateDialog(subWindow->area, line->link,line->linkType);

}

/***************************************************
  nameTreeW_callback
****************************************************/

void nameTreeW_callback(pushButton, line, cbs)
     Widget pushButton;
     struct anyLine   *line;
     XmPushButtonCallbackStruct *cbs;
{
     struct subWindow  *subWindow;
     struct subWindow  *groupWindow;

     XtVaGetValues(pushButton, XmNuserData, &subWindow, NULL);
     groupWindow = ((ALINK *)subWindow->area)->groupWindow;

     markSelectedWidget(groupWindow,NULL);
     markSelection(groupWindow, NULL);
     markSelectedWidget(subWindow,pushButton);
     markSelection(subWindow, line);

     /* update property sheet window if it is displayed */
     propUpdateDialog(subWindow->area, line->link,line->linkType);

     /* groupWindow must now display contents of new treeWindow selection */
     groupWindow->parentLink = line->link;
     groupWindow->viewConfigCount = alViewAdjustGroupW((GLINK *)line->link,
         ((ALINK *)subWindow->area)->viewFilter);
     groupWindow->viewOffset = 0;
     redraw(groupWindow,0);

}

/******************************************************
  singleClickTreeW_callback
******************************************************/

static void singleClickTreeW_callback(pdata)
     struct timeoutData *pdata;
{
     ALINK  *area;

     XtVaGetValues(pdata->pushButton, XmNuserData, &area, NULL);

     pdata->timeoutId= 0;
     displayNewViewTree(area,(GLINK *)pdata->glink,EXPANDCOLLAPSE1);
}


/***************************************************
  arrow_callback
****************************************************/

void arrow_callback(pushButton, glink, cbs)
     Widget     pushButton;
     void     *glink;
     XmPushButtonCallbackStruct *cbs;
{
     void               *area;
     static int           interval=0;
     static struct timeoutData data;
     static struct timeval timeout;


     if (cbs->click_count == 1){
          if (!interval) interval = XtGetMultiClickTime(display);
          timeout.tv_sec = 0;
          timeout.tv_usec = 1000*interval;
          
          data.pushButton = pushButton;
          data.glink = (void *)glink;
          if ( data.timeoutId== 0 ) {
               data.timeoutId= (void *)fdmgr_add_timeout(pfdctx,
                    &timeout,singleClickTreeW_callback,&data);
          }

     }
     else if (cbs->click_count == 2) {
          if (data.timeoutId) fdmgr_clear_timeout(pfdctx,data.timeoutId);
          data.timeoutId=0;

          XtVaGetValues(pushButton, XmNuserData, &area, NULL);

          displayNewViewTree(area,glink,EXPAND);
     }
}

/***************************************************
  markSelection
****************************************************/

void markSelection(subWindow,line)
     struct subWindow  *subWindow;
     struct anyLine  *line;
{

     if (!line) subWindow->selectionLink = 0;
     else subWindow->selectionLink = line->link;

     markSelectionArea(subWindow->area,line);

     return;
}

/***************************************************
  createConfigDisplay
****************************************************/

void createConfigDisplay(area, expansion)
     ALINK     *area;
     int        expansion;
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

void displayNewViewTree(area,glink,command)
     ALINK            *area;
     GLINK            *glink;
     int               command;
{
     struct anyLine    *line;
     int viewConfigCount;

     if (!glink) return;

     viewConfigCount = alViewAdjustTreeW(glink, command,area->viewFilter);
     setViewConfigCount(area->treeWindow,viewConfigCount);
     line = (struct anyLine *)glink->lineTreeW;
     if (line) redraw(area->treeWindow, line->lineNo);
     else redraw(area->treeWindow,0 );
/*
     redraw(area->treeWindow,0 );
*/
}

/***************************************************
  redraw
****************************************************/

void redraw(subWindow,rowNumber)
     struct subWindow *subWindow;
     int rowNumber;
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
          subWindow->viewOffset + subWindow->viewRowCount  > subWindow->viewConfigCount ){

          subWindow->viewOffset = Mmax(subWindow->viewConfigCount - subWindow->viewRowCount,0);
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
               if (linkType == GROUP) line = (struct anyLine *)awAllocGroupLine();
               else line = (struct anyLine *)awAllocChanLine();
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
                         ((struct anyLine *)linkOld->lineGroupW)->lineNo >= row )
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
           
          if(isTreeWindow(subWindow->area,subWindow)) link->lineTreeW = (void *)line;
          else link->lineGroupW = (void *)line;
          if (linkType == GROUP){
               awUpdateGroupLine((struct groupLine *)line);
          } else if (linkType == CHANNEL){
               awUpdateChanLine((struct chanLine *)line);
          }

          /* call subwindow create/change row widgets routine */
          (subWindow->alhRowWidgets)(line);

          if (line->link == subWindow->selectionLink){
               markSelectedWidget(subWindow,wline->name);
          }


          /* determine viewRowCount if not already set */
          if ( !subWindow->rowHeight){

               XtVaGetValues(subWindow->scrolled_w,
                    XmNheight,            &subWindow->viewHeight,
                    NULL);

               XtVaGetValues(subWindow->drawing_area,
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

/******************************
          printf (" subWindow->viewHeight=%d subWindow->marginHeight=%d subWindow->rowHeight=%d \n",
          subWindow->viewHeight,subWindow->marginHeight,subWindow->rowHeight);
          line = (struct anyLine *)sllFirst(subWindow->lines);
          while (line){
          Position y;
          Dimension height;

               printf ("line->lineNo = %d\n",line->lineNo);
               if (line->link) printf ("name=%s\n", ((GCLINK *)line->link)->pgcData->name);
               wline = (WLINE *)line->wline;
               printf ("wline = %d\n",wline);
               if (wline) printf ("wline->row_widget = %d\n",wline->row_widget);
               if (wline->row_widget){
                    XtVaGetValues(wline->row_widget,XmNy,&y,NULL);
                    XtVaGetValues(subWindow->drawing_area,XmNheight,&height,NULL);
                    printf("y =%d  height=%d\n",y,height); 
                }
               if (wline->row_widget){
                    XtVaGetValues(wline->row_widget,XmNy,&y,NULL);
                    printf("y =%d\n",y); 
                }

               printf("managed =%d\n",XtIsManaged(wline->row_widget)); 
               line = (struct anyLine *)sllNext(line);
          }
***************************/
}

/***************************************************
  invokeLinkUpdate
****************************************************/

void invokeLinkUpdate(link,linkType)
     GCLINK *link;
     int linkType;
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

void awViewAddNewAlarm (clink,prevCount,count)
     CLINK *clink;
     int prevCount;
     int count;
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
          addViewLink->viewCount=1;
          if (viewParent){
               if (newLineTree) alViewAdjustTreeW(viewParent,NOCHANGE,area->viewFilter);
          }
     }
     if (!(GLINK *)subWindowGroup->parentLink ||
          (viewParent == (GLINK *)subWindowGroup->parentLink)) {
          subWindowGroup->modified =1;
          clink->pmainGroup->modified = 1;
          subWindowGroup->viewConfigCount++;
          addViewLink->viewCount=1;
     }
}

/*
printf (" awViewAddNewAlarm:area=%d prevCount=%d count=%d \n",area,prevCount,count);
printf (" awViewAddNewAlarm:tree: viewParent->pgroupData->name=%s\n",viewParent->pgroupData->name);
printf (" awViewAddNewAlarm:group: viewParent->pgroupData->name=%s\n",viewParent->pgroupData->name);
*/

/******************************************************
  awViewNewGroup
******************************************************/

void awViewNewGroup(area, link)
     ALINK *area;
     GCLINK *link;
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
          if (parent->lineTreeW) line = ((struct anyLine *)parent->lineTreeW)->lineNo;
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

void awViewNewChan(area, link)
     ALINK *area;
     GCLINK *link;
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

int awViewViewCount(gclink)
     GCLINK *gclink;
{
     int viewCount = 0;

     if (gclink->pmainGroup->area)
            viewCount = (((ALINK *)gclink->pmainGroup->area)->viewFilter)(gclink);

     return(viewCount);
}

