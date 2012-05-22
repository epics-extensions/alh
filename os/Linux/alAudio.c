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
/* alAudio.c 
 *
 * alAudio.c,v 1.3 2003/02/27 17:20:08 jba Exp
 *
 */

#include "alh.h"
#include <stdlib.h>

/* Audio device not implemented */

/******************************************************
  alBeep
******************************************************/
int alBeep(Display *displayBB)
{
    char cmd[NAMEDEFAULT_SIZE+9]="";
    if(strlen(psetup.soundFile)>0 ) {
        strcat(cmd,"play ");
        strcat(cmd,psetup.soundFile);
	system(cmd);
    } else {
        XBell(displayBB,0);
    }
    return 0;
}


