/* alAudio.c 
 *
 * $Id$
 *
 */

#include "alh.h"

/* Audio device not implemented */

/******************************************************
  alBeep
******************************************************/
int alBeep(Display *displayBB)
{
	XBell(displayBB,0);
}


