/* awEdit.c */

/************************DESCRIPTION***********************************
  This file contains routines for edit functions
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Xm/Xm.h>

#include "alarm.h"

#include "alh.h"
#include "line.h"
#include "axSubW.h"
#include "alLib.h"
#include "axArea.h"
#include "sllLib.h"
#include "ax.h"

#define KEEP 0
#define ALH_DELETE 1

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
} clipData = { 
	0, 0 };



/******************************************************
  editUndoSet
******************************************************/
void editUndoSet(GCLINK *link,int linkType,GCLINK *configLink,
int command,int delete)
{
	/* delete unused configLink group or channel */
	/*
	     if (delete && link && link != configLink){
	          if (undoData.linkType == GROUP)
	               alRemoveGroup((GLINK *)undoData.link);
	          else 
	               alRemoveChan((CLINK *)undoData.link);
	     }
	*/
	undoData.link = link;
	undoData.linkType = linkType;
	undoData.configLink = configLink;
	undoData.command = command;
}

/******************************************************
  editUndoGet
******************************************************/
void editUndoGet(GCLINK **plink,int *plinkType,GCLINK **pconfigLink)
{
	static int undoInit=1;

	if (undoInit) {
		undoData.link=NULL;
		undoData.linkType=0;
		undoData.configLink=NULL;
		undoInit =0;
	}
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
void editClipboardGet(GCLINK **plink,int *plinkType)
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
void editClipboardSet(GCLINK *link,int linkType)
{
	if (clipData.linkType == GROUP)
		alDeleteGroup((GLINK *)clipData.link);
	else 
		alDeleteChan((CLINK *)clipData.link);

	if (linkType == GROUP){
		clipData.link = (GCLINK *)alCopyGroup((GLINK *)link);
	} else {
		clipData.link = (GCLINK *)alCopyChan((CLINK *)link);
	}
	clipData.linkType = linkType;
}

/******************************************************
  editCutLink
******************************************************/
void editCutLink(ALINK *area,GCLINK *link,int linkType)
{
	struct subWindow  *groupWindow,*treeWindow;
	GLINK *parent;
	GCLINK *linkTemp;
	int diffCount, count=0;
	struct anyLine  *treeLine;
	struct anyLine  *groupLine;
	struct anyLine  *line;
	WLINE *wline=0;
	Widget pushButton = 0;
	XmPushButtonCallbackStruct *cbs=NULL;

	if (!link) return;

	treeWindow = area->treeWindow;
	groupWindow = area->groupWindow;

	diffCount = link->viewCount;
	treeLine = (struct anyLine *)link->lineTreeW;
	groupLine = (struct anyLine *)link->lineGroupW;

	/* treeWindow selection */
	if (link == treeWindow->selectionLink){

		/* adjust lines of treeWindow*/
		/*
		          line= treeLine;
		          count = diffCount;
		          while (count){
		               line->link = 0;
		               line = (struct anyLine *)sllNext(line);
		               if (!line) break;
		               count--;
		          }
		*/

		/* adjust lines in groupWindow */
		/*
		          line = (struct anyLine *)sllFirst(groupWindow->lines);
		          while (line){
		               line->link = 0;
		               line = (struct anyLine *)sllNext(line);
		            }
		*/

		/* adjust treeWindow viewCount */
		linkTemp=link;
		while (TRUE){
			linkTemp = (GCLINK *)linkTemp->parent;
			if (linkTemp == NULL ) break;
			linkTemp->viewCount -= diffCount;
			count = linkTemp->viewCount;
		}
		setViewConfigCount(area->treeWindow,count);

		/* update line data */
		/*
		          link->lineTreeW = NULL;
		          link->lineGroupW = NULL;
		*/

		/* remove selected group from parent's childlist*/
		alRemoveGroup((GLINK *)link);

		/* adjust treeSym for treeWindow */
		parent = (GLINK *)link->parent;
		alViewAdjustTreeW(parent, NOCHANGE, area->viewFilter);

		/* redraw tree window */
		line = (struct anyLine *)parent->lineTreeW;
		if (line) {
			redraw(treeWindow, line->lineNo);
			wline = (WLINE *)line->wline;
			if (wline) pushButton = wline->name;
		}
		else redraw(treeWindow,0);

		/* adjust groupWindow */
		groupWindow = area->groupWindow;
		groupWindow->parentLink = parent;
		groupWindow->viewConfigCount = alViewAdjustGroupW((GLINK *)parent,
		    area->viewFilter);

		/* redraw  Group window */
		if (pushButton) nameTreeW_callback(pushButton, line, cbs);

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

		/* update dialog windows */
		axUpdateDialogs(area);

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
void editInsertFile(char *filename,ALINK *area)
{
	GCLINK *newLink;
	GLINK *linkHold;

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
void editPasteLink(ALINK *area,GCLINK *newLink,int linkType)
{
	struct subWindow  *treeWindow;
	GCLINK *parentLink;
	GCLINK *selectLink;
	int    selectType;

	if (!newLink) return;

	selectLink = (GCLINK *)area->selectionLink;
	selectType = area->selectionType;

	treeWindow = area->treeWindow;

	editUndoSet( NULL, linkType, (GCLINK *)newLink,
	    MENU_EDIT_UNDO_CUT_NOSELECT, ALH_DELETE);

	if (linkType == GROUP){

		/* add/insert group link */
		parentLink = treeWindow->selectionLink;
		if ( selectLink == parentLink || selectLink == NULL ||
		    selectType == CHANNEL ){
			alPrecedeGroup((GLINK *)parentLink, NULL, (GLINK *)newLink);
		} else {
			alPrecedeGroup((GLINK *)parentLink,(GLINK *)selectLink,
			    (GLINK *)newLink);
		}

		awViewNewGroup(area, (GCLINK *)newLink);

	} else {

		/* add/insert channel link */
		parentLink = treeWindow->selectionLink;
		if ( selectLink == parentLink || selectLink == NULL  ||
		    selectType == GROUP){
			alPrecedeChan((GLINK *)parentLink, NULL, (CLINK *)newLink);
		} else {
			alPrecedeChan((GLINK *)parentLink,(CLINK *)selectLink,
			    (CLINK *)newLink);
		}

		awViewNewChan(area, (GCLINK *)newLink);

	}

	/* update arrow */
	if (newLink->parent->lineTreeW)
		awRowWidgets(newLink->parent->lineTreeW,area);
	if (newLink->parent->lineGroupW)
		awRowWidgets(newLink->parent->lineGroupW,area);

}

