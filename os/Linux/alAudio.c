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

#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>

#include "alh.h"

/* Audio device not implemented */

/******************************************************
  alBeep
******************************************************/
int alBeep(Display *displayBB)
{
    pid_t childPID;

    if(strlen(psetup.soundFile)>0 ) {
        childPID = fork();
        if (childPID < 0) /* fork failed */
        {
            /*printf("\nalh: Fork failed, using default beep\n");*/
            XkbBell(displayBB,None,0,None);
        }
        else /* fork success */
        {
            if (childPID == 0) /* child process */
            {
	        execlp("play","play","-q",psetup.soundFile,NULL);
            }
        }
    } else {
        XkbBell(displayBB,None,0,None);
    }
    return 0;
}
