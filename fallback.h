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
/* fallback.h */

/************************DESCRIPTION***********************************
  alh fallback resource values
**********************************************************************/

#ifndef INCfallbackh
#define INCfallbackh 1

static char *fallbackhSccsId = "@(#) $Id$";

/* pushButtonName font needs to have 12 as height */

static String fallbackResources[] = {
	         "*initialResourcesPersistent: False",
	         "*nameTextW.background: green",
	         "*alh*foreground: black",
	         "*alh*background: #b0c3ca",
	         "*act*foreground: black",
	         "*act*background: #b0c3ca",
	         "*productDescriptionShell*background: #b0c3ca",
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
	         "*warningMessage*background: Red",
	         "*warningMessage*foreground: White",
	         (String)NULL
};

#endif /* INCfallbackh */

