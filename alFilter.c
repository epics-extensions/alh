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

