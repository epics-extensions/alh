/*
 $Log$
 Revision 1.3  1997/09/12 19:28:35  jba
 Removed comments.

 Revision 1.2  1994/06/22 21:16:30  jba
 Added cvs Log keyword

 */

static char *sccsId = "@(#)alFilter.c	1.1\t8/4/93";

/* alFilter.c */
/*
 *      Author:		Janet Anderson
 *      Date:		06-22-93
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

/*****************************************************************
	routines defined in alFilter.c
******************************************************************
         Routines to filter alarm groups and channels
******************************************************************
-------------
|   PUBLIC  |
-------------
*
int alFilterAll(gclink)        group/channel filter - NULL filter
     GCLINK  *gclink;
*
int alFilterAlarms(gclink)     group/channel filter - Alarms Only filter
     GCLINK  *gclink;
*
*******************************************************************/

#include <alLib.h>

/***************************************************
  alFilterAll
****************************************************/

int alFilterAll(gclink)
     GCLINK  *gclink;
{
     return(TRUE);
}

/***************************************************
  alFilterAlarmsOnly
****************************************************/


int alFilterAlarmsOnly(gclink)
     GCLINK  *gclink;
{

     if ( gclink->pgcData->curSevr || gclink->pgcData->unackSevr ) return(TRUE);
     return(FALSE);
}

