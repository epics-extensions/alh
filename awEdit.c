/*
 $Log$
 Revision 1.3  1995/05/31 20:34:08  jba
 Added name selection and arrow functions to Group window

 * Revision 1.2  1994/06/22  21:16:56  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)awEdit.c	1.1\t10/22/93";

/* awEdit.c */
/*
 *      Author:		Janet Anderson
 *      Date:		09-28-92
 *
 *	Experimental Physics and Industrial Control System (EPICS)
 *
 *	Copyright 1991, the Regents of the University of California,
 *	and the University of Chicago Board of Governors.
 *
 *	This software was produced under  U.S. Government contralhs:
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
	routines defined in awEdit.c
******************************************************************
         Routines for ACT specific menu, line, and callbacks
******************************************************************
-------------
|   PUBLIC  |
-------------
*

*************************************************************************
*/

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <alarm.h>

#include <alh.h>
#include <line.h>
#include <axSubW.h>
#include <alLib.h>

#include <Xm/Xm.h>
*/

#include <axArea.h>
#include <sllLib.h>
#include <ax.h>

static struct undoInfo {
     GCLINK *link;
     int linkType;
     GCLINK *configLink;
     int command;
     int delete;
} undoData;

struct clipInfo {
     GCLINK *link;
     int linkType;
} clipData;

#define KEEP 0
#define DELETE 1



/******************************************************
  editUndoSet
******************************************************/

void editUndoSet(link, linkType, configLink, command, delete)
     GCLINK *link;
     int linkType;
     GCLINK *configLink;
     int command;
     int delete;
{
     /* delete unused configLink group or channel */
     if (delete && link && link != configLink){
          if (undoData.linkType == GROUP)
               alRemoveGroup((GLINK *)undoData.link);
          else 
               alRemoveChan((CLINK *)undoData.link);
     }

     undoData.link = link;
     undoData.linkType = linkType;
     undoData.command = command;
     undoData.configLink = configLink;
}

/******************************************************
  editUndoGet
******************************************************/

void editUndoGet(plink, plinkType, pconfigLink)
     GCLINK **plink;
     int *plinkType;
     GCLINK **pconfigLink;
{
     *plink = undoData.link;
     *plinkType = undoData.linkType;
     *pconfigLink = undoData.configLink;
}

/******************************************************
  editUndoGetCommand
******************************************************/

int editUndoGetCommand()
{
     return undoData.command;
}

/******************************************************
  editClipboardGet
******************************************************/

void editClipboardGet(plink, plinkType)
     GCLINK **plink;
     int *plinkType;
{
     if (clipData.linkType == GROUP){
         *plink = (GCLINK *)alCopyGroup((GLINK *)clipData.link);
     } else {
         *plink = (GCLINK *)alCopyChan((CLINK *)clipData.link);
     }
     *plinkType = clipData.linkType;
}

/******************************************************
  editClipboardSet
******************************************************/

void editClipboardSet(link, linkType)
     GCLINK *link;
     int linkType;
{
     if (clipData.linkType == GROUP)
          alRemoveGroup((GLINK *)clipData.link);
     else 
          alRemoveChan((CLINK *)clipData.link);

     if (linkType == GROUP){
          clipData.link = (GCLINK *)alCopyGroup((GLINK *)link);
          clipData.linkType = linkType;
     } else {
          clipData.link = (GCLINK *)alCopyChan((CLINK *)link);
          clipData.linkType = linkType;
     }
}

/******************************************************
  editCutLink
******************************************************/

void editCutLink( area, link, linkType)
     ALINK *area;
     GCLINK *link;
     int linkType;
{
     struct subWindow  *groupWindow,*treeWindow;
     GLINK *parent;
     GCLINK *linkTemp;
     int diffCount, count=0;
     struct anyLine  *treeLine;
     struct anyLine  *groupLine;
     struct anyLine  *line;

     treeWindow = area->treeWindow;
     groupWindow = area->groupWindow;

     diffCount = link->viewCount;
     treeLine = (struct anyLine *)link->lineTreeW;
     groupLine = (struct anyLine *)link->lineGroupW;

     /* treeWindow selection */
     if (link == treeWindow->selectionLink){

          /* adjust treeWindow selection */
          markSelectedWidget(treeWindow,0);
          markSelection(treeWindow, 0);

          /* adjust groupWindow selection */
          markSelectedWidget(groupWindow,0);
          markSelection(groupWindow, 0);

          /* update property window */
          propUpdateDialog(area, 0, 0);

          /* adjust lines of treeWindow*/
          line= treeLine;
          count = diffCount;
          while (count){
               line->link = 0;
               line = (struct anyLine *)sllNext(line);
               if (!line) break;
               count--;
          }

          /* adjust treeWindow viewCount */
          linkTemp=link;
          while (TRUE){
               linkTemp = (GCLINK *)linkTemp->parent;
               if (linkTemp == NULL ) break;
               linkTemp->viewCount -= diffCount;
               count = linkTemp->viewCount;
          }
          setViewConfigCount(area->treeWindow,count);

          /* adjust groupWindow */
          groupWindow = area->groupWindow;
          groupWindow->parentLink = 0;
          groupWindow->viewConfigCount = 0;
          groupWindow->viewOffset = 0;

          /* adjust lines in groupWindow */
          line = (struct anyLine *)sllFirst(groupWindow->lines);
          while (line){
               line->link = 0;
               line = (struct anyLine *)sllNext(line);
          }

          /* update line data */
          link->lineTreeW = NULL;
          link->lineGroupW = NULL;

          /* delete selected group */
          alRemoveGroup((GLINK *)link);

          /* adjust treeSym for treeWindow */
          parent = (GLINK *)link->parent;
          alViewAdjustTreeW(parent, NOCHANGE, area->viewFilter);

          /* redraw  windows */
          line = (struct anyLine *)parent->lineTreeW;
          if (line) redraw(treeWindow, line->lineNo);
          else redraw(treeWindow,0);

          redraw(groupWindow,0);
     }

     /* groupWindow selection */
     else if (link == groupWindow->selectionLink){


          /* adjust groupWindow selection */
          markSelectedWidget(groupWindow,0);
          markSelection(groupWindow, 0);

          /* make treeWindow selection the active selection */
          area->selectionWindow = treeWindow;
          area->selectionWidget = treeWindow->selectionWidget;
          area->selectionLink = treeWindow->selectionLink;
          area->selectionType = GROUP;

          /* update property window */
          propUpdateDialog(area, area->selectionLink, area->selectionType);

          /*  adjust treeWindow  */
          if (treeLine) {

               /* adjust lines of treeWindow*/
               line = treeLine;
               count = diffCount;
               while (count){
                    line->link = 0;
                    line = (struct anyLine *)sllNext(line);
                    if (!line) break;
                    count--;
               }

               /* adjust viewCount of treeWindow*/
               linkTemp = link;
               while (TRUE){
                    linkTemp = (GCLINK *)linkTemp->parent;
                    if (linkTemp == NULL ) break;
                    linkTemp->viewCount -= diffCount;
                    count = linkTemp->viewCount;
               }
               setViewConfigCount(area->treeWindow,count);
          }

          /*  adjust line of groupWindow  */
          groupLine->link = 0;

          /* adjust viewCount of groupWindow*/
          groupWindow->viewConfigCount -= 1;

          /* delete selected group or channel */
          if (groupLine->linkType == GROUP){
               alRemoveGroup((GLINK *)link);
          } else {
               alRemoveChan((CLINK *)link);
          }

          /* adjust treeSym for treeWindow */
          parent = (GLINK *)link->parent;
          alViewAdjustTreeW(parent, NOCHANGE, area->viewFilter);

          /* redraw  windows */
          if (treeLine){
               line = (struct anyLine *)parent->lineTreeW;
               if (line) redraw(treeWindow, line->lineNo);
               else redraw(treeWindow,0);
          }

          redraw(groupWindow,groupLine->lineNo);

     }

     /* not a selection */
     else {

          /* delete selected group or channel */
          if (groupLine->linkType == GROUP){
               alRemoveGroup((GLINK *)link);
          } else {
               alRemoveChan((CLINK *)link);
          }

          /*  adjust treeWindow  */
          if (treeLine) {

               /* adjust lines of treeWindow*/
               line = treeLine;
               count = diffCount;
               while (count){
                    line->link = 0;
                    line = (struct anyLine *)sllNext(line);
                    if (!line) break;
                    count--;
               }

               /* adjust viewCount of treeWindow*/
               linkTemp = link;
               while (TRUE){
                    linkTemp = (GCLINK *)linkTemp->parent;
                    if (linkTemp == NULL ) break;
                    linkTemp->viewCount -= diffCount;
                    count = linkTemp->viewCount;
               }
               setViewConfigCount(area->treeWindow,count);

               /* adjust treeSym for treeWindow */
               parent = (GLINK *)link->parent;
               alViewAdjustTreeW(parent, NOCHANGE, area->viewFilter);

               /* redraw  windows */
               line = (struct anyLine *)parent->lineTreeW;
               if (line) redraw(treeWindow, line->lineNo);
               else redraw(treeWindow,0);

          }

          if (groupLine) {
               /*  adjust line of groupWindow  */
               groupLine->link = 0;

               /* adjust viewCount of groupWindow*/
               groupWindow->viewConfigCount -= 1;

               redraw(groupWindow,groupLine->lineNo);

          }

     }

     /* update arrow */
     if (link->parent->lineTreeW)
          awRowWidgets(link->parent->lineTreeW,area);
     if (link->parent->lineGroupW)
          awRowWidgets(link->parent->lineGroupW,area);

}

/******************************************************
  editInsertFile
******************************************************/
void editInsertFile(filename,area)
     char *filename;
     ALINK *area;
{
     void   *selectLink;
     struct subWindow *treeWindow;
     GCLINK *newLink;
     GLINK *linkHold;
     GCLINK *parentLink;
     GCLINK *priorLink;
     SNODE *pt;


     linkHold = (GLINK *)sllFirst(area->pmainGroup);
     alGetConfig(area->pmainGroup,filename,CA_CONNECT_NO);
     if (!sllFirst(area->pmainGroup)){
          createDialog(area->form_main,XmDIALOG_ERROR,filename,
              " is not a valid alarm configuration file.");
          return;
     }
     newLink = (GCLINK *)sllFirst(area->pmainGroup);
     ((GCLINK *)newLink)->viewCount = 1;
     area->pmainGroup->p1stgroup = linkHold;

     editPasteLink(area, newLink, GROUP);

}

/******************************************************
  editPasteLink
******************************************************/
void editPasteLink(area, newLink, linkType)
     ALINK *area;
     GCLINK *newLink;
     int linkType;
{
     struct subWindow  *treeWindow;
     struct subWindow  *groupWindow;
     GCLINK *parentLink;
     GCLINK *priorLink;
     GCLINK *selectLink;
     int    selectType;
     SNODE *pt;
     SNODE *firstLink;

     selectLink = (GCLINK *)area->selectionLink;
     selectType = area->selectionType;

     treeWindow = area->treeWindow;
     groupWindow = area->groupWindow;

     editUndoSet( NULL, linkType, (GCLINK *)newLink, MENU_EDIT_UNDO_CUT_NOSELECT, DELETE);

     if (linkType == GROUP){
          
          /* add/insert group link */
          parentLink = treeWindow->selectionLink;
          if ( selectLink == parentLink || selectLink == NULL || selectType == CHANNEL ){
               alPrecedeGroup((GLINK *)parentLink, NULL, (GLINK *)newLink);
          } else {
               alPrecedeGroup((GLINK *)parentLink, (GLINK *)selectLink, (GLINK *)newLink);
          }

          awViewNewGroup(area, (GCLINK *)newLink);

     } else {

          /* add/insert channel link */
          parentLink = treeWindow->selectionLink;
          if ( selectLink == parentLink || selectLink == NULL  || selectType == GROUP){
               alPrecedeChan((GLINK *)parentLink, NULL, (CLINK *)newLink);
          } else {
               alPrecedeChan((GLINK *)parentLink, (GLINK *)selectLink, (GLINK *)newLink);
          }

          awViewNewChan(area, (GCLINK *)newLink);

     }

     /* update arrow */
     if (newLink->parent->lineTreeW)
          awRowWidgets(newLink->parent->lineTreeW,area);
     if (newLink->parent->lineGroupW)
          awRowWidgets(newLink->parent->lineGroupW,area);

}
/*
printf("editCutLink entered: link=%d linkType=%d\n",link,linkType);
printf("editCutLink treeWindow selection: link=%d linkType=%d\n",link,linkType);
printf("editCutLink groupWindow selection: link=%d linkType=%d\n",link,linkType);
printf("editCutLink not a selection: link=%d linkType=%d\n",link,linkType);
printf("************** editCutCallback:  link->pgcData->name=%s \n", link->pgcData->name);
printf ("treeLine = %d \n",treeLine);
printf ("groupLine = %d \n",groupLine);
printf ("treeWindow selection \n");
printf ("groupWindow selection \n");
printf ("adjusting lines of treeWindow:  count = %d \n", count);
printf ("adjusting lines of treeWindow:  line->link = %d \n",line->link);
if (line->link) printf ("------------------ line->link name = %s \n",((GCLINK *)line->link)->pgcData->name);
printf ("adjusting lines of treeWindow:  count = %d \n", count);
printf ("adjusting lines of treeWindow:  line->link = %d \n",line->link);
if (line->link) printf ("------------------ line->link name = %s \n",((GCLINK *)line->link)->pgcData->name);
printf("editCutCallback:  area->selectionLink=%d \n",area->selectionLink);
printf("editCutCallback:  treeWindow->selectionLink=%d \n",treeWindow->selectionLink);
printf("editCutCallback:  groupWindow->selectionLink=%d \n",groupWindow->selectionLink);
if (area->selectionLink)
printf("editCutCallback:treeSelection:  area->selectionLink->pgcData->name=%s linkType=%d\n",
 ((GCLINK *)area->selectionLink)->pgcData->name,area->selectionType);
printf("groupSelection:  area->selectionLink->pgcData->name=%s linkType=%d\n",
((GCLINK *)area->selectionLink)->pgcData->name,area->selectionType);
printf ("line = %d \n",line);
printf ("linkType = %d  GROUP=%d \n",line->linkType,GROUP);
printf (" after alRemoveGroup   link = %d \n",link);
printf ("linkType = %d  CHANNEL=%d \n",line->linkType,CHANNEL);
printf (" after alRemoveChan   link = %d \n",link);
*/

