/* acknowledge.c */

/************************DESCRIPTION***********************************
  This file contains routines for alarm acknowledgement
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#include <stdio.h>
#include <Xm/Xm.h>
#include "alh.h"
#include "alLib.h"
#include "line.h"
#include "ax.h"

extern int _passive_flag;	/* Passive flag. Albert */

/***************************************************
 channel acknowledge button callback
****************************************************/
static void ackChan(struct anyLine * line)
{
	CLINK *clink;
	struct chanData *cdata;
	clink = (CLINK *) line->link;
	cdata = clink->pchanData;
	if (cdata->unackSevr == 0)
		return;
	alLogAckChan(line);
	alCaPutGblAck(cdata->chid, &cdata->unackSevr);
	alAckChan(clink);
	clink->pmainGroup->modified = 1;
}
/***************************************************
 group acknowledge button callback
****************************************************/
static void ackGroup(struct anyLine * line)
{
	GLINK *glink;
	glink = (GLINK *) line->link;
	if (alHighestSeverity(glink->pgroupData->unackSev) == 0)
		return;
	alLogAckGroup(line);
	alAckGroup(glink);
	glink->pmainGroup->modified = 1;
}
/***************************************************
  ack_callback
****************************************************/
void ack_callback(Widget widget, struct anyLine * line,
XmAnyCallbackStruct * cbs)
{
	ALINK *area;	/* We need it in passive mode only; Albert */
	if (_passive_flag) {
		XtVaGetValues(widget, XmNuserData, &area, NULL);
		createDialog(area->form_main, XmDIALOG_WARNING,
		    "You can't acknowledge alarms in passive mode.", " ");
		return;
	}
	if (line->linkType == GROUP)
		ackGroup(line);
	else if (line->linkType == CHANNEL)
		ackChan(line);
}
