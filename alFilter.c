/* alFilter.c */

/************************DESCRIPTION***********************************
  Routines to filter alarm groups and channels
**********************************************************************/

static char *sccsId = "@(#) $Id$";

#include "alLib.h"

/***************************************************
  alFilterAll
****************************************************/
int alFilterAll(GCLINK *gclink)
{
	return(TRUE);
}

/***************************************************
  alFilterAlarmsOnly
****************************************************/
int alFilterAlarmsOnly(GCLINK *gclink)
{
	if ( gclink->pgcData->curSevr || gclink->pgcData->unackSevr ) return(TRUE);
	return(FALSE);
}

/***************************************************
  alFilterUnackAlarmsOnly
****************************************************/
int alFilterUnackAlarmsOnly(GCLINK *gclink)
{
	if ( gclink->pgcData->unackSevr ) return(TRUE);
	return(FALSE);
}

