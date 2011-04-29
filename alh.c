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
/* alh.c */

/************************DESCRIPTION***********************************
  Alarm Handler
**********************************************************************/

#define DEBUG_CALLBACKS 0
#define MIN_MULTICLICK_INTERVAL 500

#include <stdio.h>
#include <stdlib.h>

#include "alh.h"
#include "epicsVersion.h"
#include "version.h"
#include "fallback.h"
#include "sllLib.h"
#include "axArea.h"
#include "ax.h"

/* global variables */
const char *executionModeString[] = {"Local","Global"};
const char *ackTransientsString[] = {"ackT","noackT"};
Widget productDescriptionShell;
XtAppContext appContext;
Widget topLevelShell;
Display *display;



int DEBUG = 0;
SLIST *areaList;
char alhVersionString[100];
XFontStruct *font_info;

/******************************************************
  main
******************************************************/

int main(int argc,char *argv[])
{

	int interval=0;
	/* OS specific initialization */
#ifdef HCLXMINIT
#ifdef WIN32
	HCLXmInit();
#endif
#endif
#ifdef HP_UX
        _main();
#endif
 
	/*  Xt initialize the application */
	topLevelShell = XtAppInitialize(&appContext, "Alarm", NULL, 0, &argc, argv,
	    fallbackResources, NULL, 0);

	/*  check display  */
	display = XtDisplay(topLevelShell);
	if (display == NULL) {
		XtWarning("cannot open display");
		exit(-1);
	}

        interval = XtGetMultiClickTime(display);
        if (interval < MIN_MULTICLICK_INTERVAL) XtSetMultiClickTime(display,MIN_MULTICLICK_INTERVAL);
        interval = XtGetMultiClickTime(display);


	XtAppSetWarningMsgHandler(appContext,
	    (XtErrorMsgHandler)trapExtraneousWarningsHandler);

	/* load fixed font */
	font_info = XLoadQueryFont(display,"fixed");

	/* initialize alh status and severity values */
	alhAlarmStringInit();

	sprintf(alhVersionString,"ALH Version %d.%d.%d  (%s)",
	    ALH_VERSION,ALH_REVISION,ALH_MODIFICATION,EPICS_VERSION_STRING);

	/* setup area and configuration */
	fileSetupInit(topLevelShell,argc,argv);

	/* display alh credits window */
#if  IWantGreetings
	productDescriptionShell = createAndPopupProductDescriptionShell( appContext,
	    topLevelShell,
	    "  ALH  ", NULL, ALH_pixmap,
	    "\nAlarm Handler\n Alarm Configuration Tool\n",NULL,
	    ALH_CREDITS_STRING ,
	    alhVersionString,
	    NULL, -1,-1,3);
#else
	productDescriptionShell = 0;
#endif

	/* Start main loop */
	XtAppMainLoop(appContext);
	return 0;
}

