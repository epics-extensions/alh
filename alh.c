/* alh.c */

/************************DESCRIPTION***********************************
  Alarm Handler
**********************************************************************/

static char *sccsId = "@@(#) $Id$";

#define DEBUG_CALLBACKS 0

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
int DEBUG = 0;
int ALARM_COUNTER = 0;
SLIST *areaList;
char alhVersionString[60];
extern XtAppContext appContext;
extern Display *display;
XFontStruct *font_info;

/* externals */
extern Widget createAndPopupProductDescriptionShell();


/******************************************************
  main
******************************************************/

void main(int argc,char *argv[])
{

	/* WIN32 initialization */
#ifdef WIN32	
	HCLXmInit();
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

	XtAppSetWarningMsgHandler(appContext,
	    (XtErrorMsgHandler)trapExtraneousWarningsHandler);

	/* load fixed font */
	font_info = XLoadQueryFont(display,"fixed");

	/* setup area and configuration */
	fileSetupInit(topLevelShell,argc,argv);

	sprintf(alhVersionString,"ALH Version %d.%d.%d  (%s)",
	    ALH_VERSION,ALH_REVISION,ALH_MODIFICATION,EPICS_VERSION_STRING);

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
}
