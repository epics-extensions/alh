/*
 $Log$
 Revision 1.5  1995/10/20 16:50:36  jba
 Modified Action menus and Action windows
 Renamed ALARMCOMMAND to SEVRCOMMAND
 Added STATCOMMAND facility
 Added ALIAS facility
 Added ALARMCOUNTFILTER facility
 Make a few bug fixes.

 * Revision 1.4  1995/05/30  15:58:03  jba
 * Added ALARMCOMMAND facility
 *
 * Revision 1.3  1995/02/28  16:43:46  jba
 * ansi c changes
 *
 * Revision 1.2  1994/06/22  21:17:25  jba
 * Added cvs Log keyword
 *
 */

static char *fallbackhsccsId = "@(#)fallback.h	1.5\t12/15/93";

/* fallback.h 
 *
 *      Author:		Janet Anderson
 *      Date:		02-16-93
 *
 *	Experimental Physics and Industrial Control System (EPICS)
 *
 *	Copyright 1991, the Regents of the University of California,
 *	and the University of Chicago Board of Governors.
 *
 *	This software was produced under  U.S. Government contracts:
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
 * .01	12-10-93		jba	#endif comment changed to proper c comment
 * .01	mm-dd-yy		nnn	Description
 */
#ifndef INCfallbackh
#define INCfallbackh 1

/***********************************************************
   alh fallback resource values
***********************************************************/

/* pushButtonName font needs to have 12 as height */

static String fallbackResources[] = {
         "*initialResourcesPersistent: False",
         "*nameTextW.background: green",
         "*alh*foreground: black",
         "*alh*background: #b0c3ca",
         "*act*foreground: black",
         "*act*background: #b0c3ca",
         "*form_main.width: 1000",
         "*form_main.height: 600",
         "*form_main.x: 100",
         "*form_main.x: 200",
         "*drawing_area.width: 1000",
         "*scale.value: 50",
         "*scale.highlightOnEnter: FALSE",
         "*scale.scaleMultiple: 5",
         "*treeSym.fontList: 12x24",
         "*XmCascadeButtonGadget.fontList: 8x13",
         "*XmCascadeButtonWidget.fontList: 8x13",
         "*XmPushButtonGadget.fontList: 8x13",
         "*XmPushButtonWidget.fontList: 8x13",
         "*XmToggleButtonGadget.fontList: 8x13",
         "*XmToggleButtonWidget.fontList: 8x13",
         "*ack.fontList: 7x14",
         "*sevr.fontList: 7x14",
         "*pushButtonName.fontList: -*-Helvetica-bold-r-*--12-*",
         "*pushButtonGroupName.fontList: -*-Helvetica-bold-r-*--12-*",
         (String)NULL
};

#endif /* INCfallbackh */

