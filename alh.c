/*
 $Log$
 Revision 1.5  1998/05/13 19:29:48  evans
 More WIN32 changes.

 Revision 1.4  1995/10/20 16:50:05  jba
 Modified Action menus and Action windows
 Renamed ALARMCOMMAND to SEVRCOMMAND
 Added STATCOMMAND facility
 Added ALIAS facility
 Added ALARMCOUNTFILTER facility
 Make a few bug fixes.

 * Revision 1.3  1994/09/14  21:11:13  jba
 * Modified  to work with new CONFIG files
 *
 * Revision 1.2  1994/06/22  21:16:45  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)alh.c	1.23\t12/15/93";

/* alh.c */
/*  alh  -  Alarm Handler
 *
 *      Author: Ben-chin Cha 
 *      Date:   12-20-90
 * 
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 *              Argonne National Laboratory
 *
 * Modification Log:
 * -----------------
 * .01  08-22-91        bkc     Change XtMainLoop to call awInvokeCallback
 *				when there is no Xevent or CA event 
 * .02  mm-dd-yy        bkc	Add the option of printing # of alarms detected  
 * .03  02-16-93        jba     Rewrote alh.c for new user interface
 * .04  12-10-93        jba     Moved commandline options handling to file.c
 * .nn  mm-dd-yy        iii     Comment
 *      ...
 */

#include <stdio.h>
#include <stdlib.h>

#include <fdmgr.h>

#include <alh.h>
#include <epicsVersion.h>
#include <fallback.h>
#include <sllLib.h>
#include <axArea.h>
#include <ax.h>

#define FDMGR_SEC_TIMEOUT   10
#define FDMGR_USEC_TIMEOUT  0

/* global variables */
int DEBUG = 0;
int ALARM_COUNTER = 0;
SLIST *areaList;
extern fdctx *pfdctx;
extern XtAppContext appContext;
extern Display *display;


extern Widget createAndPopupProductDescriptionShell();

/******************************************************
  main
******************************************************/

void main(argc, argv)
     int argc;
     char *argv[];
{
     ALINK        *area;
     static struct timeval timeout = {
          FDMGR_SEC_TIMEOUT, FDMGR_USEC_TIMEOUT};
     Widget        topLevelShell;

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

     /* initialize fdmgr */
     alFdmgrInit(display);

     XtAppSetWarningMsgHandler(appContext,
           (XtErrorMsgHandler)trapExtraneousWarningsHandler);

     /* setup area and configuration */
     fileSetupInit(topLevelShell,argc,argv);

     /* display alh credits window */
#if  IWantGreetings &&  XmVersion && XmVersion >= 1002
     productDescriptionShell = createAndPopupProductDescriptionShell(appContext,
          topLevelShell,
          "  ALH  ", NULL, ALH_pixmap,
          "\nAlarm Handler\n Alarm Configuration Tool\n",NULL,
          EPICS_VERSION_STRING,
          "\nDeveloped at Argonne National Laboratory\nAuthors: Ben-Chin Cha, Janet Anderson,\n         Mark Anderson, and Marty Kraimer\n",
          NULL, -1,-1,3);
#else
     productDescriptionShell = 0;
#endif

#if 1

     alProcessCA();
     XFlush(display);
     /* start alh Process events loop */
     while(TRUE) {
          fdmgr_pend_event(pfdctx,&timeout);
          area = 0;
          if (areaList) area = (ALINK *)sllFirst(areaList);
          while (area) {
               if (area->pmainGroup && area->pmainGroup->p1stgroup){
                    alHighestSystemSeverity(area->pmainGroup->p1stgroup);

                    if ( area->pmainGroup->modified ){
                         if ( area->mapped && area->managed){
                               invokeDialogUpdate(area);
                               invokeSubWindowUpdate(area->treeWindow);
                               invokeSubWindowUpdate(area->groupWindow);
                         }
                         updateCurrentAlarmWindow(area);
                         area->pmainGroup->modified = 0;
                    }
               }
               area = (ALINK *)sllNext(area);
          }
     }
#else
     XtAppMainLoop(appContext);
#endif
}
