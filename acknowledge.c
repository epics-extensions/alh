/*
 $Id$
 */

#include <stdio.h>

#include <Xm/Xm.h>

#include <alh.h>
#include <alLib.h>
#include <line.h>
#include <ax.h>
extern int _passive_flag; /* Passive flag. Albert */

/*
******************************************************************
*   acknowledge.c 
*
*	This file contains routines for alarm acknowledgement
*
******************************************************************
*/

/***************************************************
 channel acknowledge button callback 
****************************************************/

static void ackChan(cline)
  struct chanLine *cline;
{
  CLINK *clink;
  struct chanData *cdata;

  clink = (CLINK *)cline->clink;
  cdata = clink->pchanData;
  if ( cdata->unackSevr == 0) return;
  alLogAckChan(cline);
  alCaPutGblAck(cdata->chid,&cdata->unackSevr);
  alAckChan(clink);
  clink->pmainGroup->modified = 1;
}

/***************************************************
 group acknowledge button callback 
****************************************************/

static void ackGroup(gline)
  struct groupLine *gline;
{
  GLINK *glink;


  glink = (GLINK *)gline->glink;
  if ( alHighestSeverity(glink->pgroupData->unackSev) == 0) return;
  alLogAckGroup(gline);
  alAckGroup(glink);
/*  awInvokeCallback(); */ 
  glink->pmainGroup->modified = 1;
 
}

/***************************************************
  ack_callback
****************************************************/

void ack_callback(widget,line,cbs)
     Widget widget;
     struct anyLine  *line;
     XmAnyCallbackStruct *cbs;
{
     ALINK *area; /* We need it in passive mode only; Albert */
     if (_passive_flag) {
            XtVaGetValues(widget, XmNuserData, &area, NULL);
	    createDialog(area->form_main,XmDIALOG_WARNING,
            "You can't acknowledge alarms in passive mode."," ");
	    return;
     }

     if (line->linkType == GROUP) ackGroup((struct groupLine *)line); 
     else if (line->linkType == CHANNEL) ackChan((struct chanLine *)line); 
}

