/*
 $Log$
 Revision 1.2  1994/06/22 21:17:57  jba
 Added cvs Log keyword

 */

static char *sccsId = "@(#)showmask.c	1.12\t9/15/93";

/* showmask.c
 *
 *      Author: Ben-chin Cha
 *      Date:   12-20-90
 * Modification Log:
 * -----------------
 * .01  07-22-91        bkc     Correct show group mask dialog 
 *                              
 * .02  mm-dd-yy        iii     Comment
 *      ...
 */
#include <X11/StringDefs.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Text.h>
#include <Xm/LabelG.h>
#include <Xm/BulletinB.h>

#include <sllLib.h>
#include <alLib.h>
#include <ax.h>

extern Dimension char_width;
extern XmStringCharSet charset;

extern void done_dialog();
extern void xs_help_callback();

/*******************************************************
 * showmask.c: a popup dialog widget 
*
*This file contains routines for creating show masks 
*dialog.  One for group masks.  One for channel masks.
*
 
Routines defined in showmask.c:
------------
|  PUBLIC  |
------------
Widget createShowGroupMasksDialog(parent,cdata)        	Show group masks dialog
Widget createShowChanMasksDialog(parent,cdata)          Show channel masks dialog

 ******************************************************/


char  editors_groupmasks[19][80];
Widget label_groupmasks[19],edit_groupmasks[19];

char  *labels_groupmasks[] = { 
	"Group Name", 
	"Current Masks",
	"No. of channels in following states : Valid Alarms",  
	"                                      Major Alarms",
	"                                      Minor Alarms", 
	"                                      NoAlarms", 
	"                                      Canceled", 
	"                                      Disabled", 
	"                                      NoAck", 
	"                                      NoAck Transient",
	"                                      NoLog",
	"Total no. of channels in group ",
	"Force Process Variable : name",
	"Force Process Variable : current value ",
	"Force Process Variable : force value ",
	"Force Process Variable : reset value",
	"Force Process Variable : force mask",
	"Sevrerity Process Variable : name",
	"Related display Command Line:"	};


char *help_str_showGroupMasks[] = {
        "This dialog window displays the summary info for the",
	"specified group.",
        " ",
        "Use the Ok button to close the dialog.",
        " ",
        "","" };

char  editors_chanmasks[11][80];
Widget label_chanmasks[11],edit_chanmasks[11];
char  *labels_chanmasks[] = { 	
	"Channel Name", 
	"Current Value",
	"Current Mask",
	"Reset Mask",
	"Force Process Variable : name",
	"Force Process Variable : curent value",
	"Force Process Variable : force value ",
	"Force Process Variable : reset value",
	"Force Process Variable : force mask",
	"Severity Process Variable : name",
	"Related display Command Line:  "	};


char *help_str_showChanMasks[] = {
        "This dialog window displays the summary info for the",
	"specified channel.",
        " ",
        "Use the Ok button to close the dialog.",
        " ",
        "","" };



/****************************************************************************
	create show group mask dialog
*****************************************************************************/
Widget createShowGroupMasksDialog(parent,cdata)
Widget parent;
struct groupData *cdata;
{
Widget bb, ok_button, help_button;
Arg     wargs[10];
int     i, n=0,len=0,xloc,tot_chan=0;
 XmString str=NULL;
Dimension width,maxWidth;


	strcpy(editors_groupmasks[0],cdata->name);
 	 awGetMaskString(cdata->mask,editors_groupmasks[1]);

	for (i=0;i<ALARM_NSEV;i++) {
		tot_chan += cdata->curSev[i]; 
		sprintf(editors_groupmasks[2+i],"%d",
			cdata->curSev[ALARM_NSEV-1-i]);
		}
	for (i=0;i<ALARM_NMASK;i++) 
		sprintf(editors_groupmasks[2+ALARM_NSEV+i],"%d",cdata->mask[i]);

	i = 2 + ALARM_NSEV + ALARM_NMASK; 
		sprintf(editors_groupmasks[i],"%d",tot_chan);

	i++; 	strcpy(editors_groupmasks[i],cdata->forcePVName);

	/* find pvforce value */
	i++; 
	strcpy(editors_groupmasks[i],"");
	if (cdata->forcechid != NULL) { 
		sprintf(editors_groupmasks[i],"%d",cdata->PVValue);
		}

	i++;    sprintf(editors_groupmasks[i],"%d",cdata->forcePVValue); 
	i++; 	sprintf(editors_groupmasks[i],"%d",cdata->resetPVValue); 
	i++; 	alGetMaskString(cdata->forcePVMask,editors_groupmasks[i]);
	i++; 	strcpy(editors_groupmasks[i],cdata->sevrPVName);
	i++; 	if (cdata->command == NULL) 
			strcpy(editors_groupmasks[i],""); 
		else
			strcpy(editors_groupmasks[i],cdata->command); 

        n =0;
        XtSetArg(wargs[n], XmNautoUnmanage, FALSE); n++;
        XtSetArg(wargs[n], XmNdialogTitle,
                XmStringLtoRCreate("Group  Masks",
                XmSTRING_DEFAULT_CHARSET)); n++;
        XtSetArg(wargs[n], XmNallowShellResize, FALSE); n++;
        bb = XmCreateBulletinBoardDialog(parent, "Group Summary", wargs, n);


        /* create text widget and label widget */

        maxWidth = 0;
        for (i=0; i<XtNumber(labels_groupmasks); i++) {
            str = XmStringCreateLtoR(labels_groupmasks[i],charset);
       		n = 0;
            XtSetArg(wargs[n], XmNlabelString, str); n++;
       		XtSetArg(wargs[n], XmNx, 10); n++;
       		XtSetArg(wargs[n], XmNy, 10 + 30*i); n++;

            label_groupmasks[i] = XtCreateManagedWidget(labels_groupmasks[i],
                 xmLabelGadgetClass, bb,
                 wargs, n);
            XmStringFree(str);
            XtVaGetValues(label_groupmasks[i],XmNwidth,&width,NULL);
            if (width > maxWidth) maxWidth = width;
         }
        
        for (i=0; i<XtNumber(labels_groupmasks); i++) {
       		n = 0;
	        XtSetArg(wargs[n], XmNwidth, maxWidth); n++;
       		XtSetArg(wargs[n], XmNalignment, XmALIGNMENT_END ); n++;
            XtSetValues(label_groupmasks[i],wargs,n);
        }
/*
 * get string width in pixel
 */
  xloc = 10 + maxWidth;


        for (i=0; i<XtNumber(editors_groupmasks); i++) {
        n = 0;
        XtSetArg(wargs[n], XmNx, 10 +xloc); n++;
        XtSetArg(wargs[n], XmNy, 10 + 30*i); n++;
if (i==18) XtSetArg(wargs[n], XmNwidth, 80*char_width);
        else
	   XtSetArg(wargs[n], XmNwidth, 30 * char_width); n++;
        XtSetArg(wargs[n],XmNcursorPositionVisible,FALSE);  n++;
        XtSetArg(wargs[n], XmNeditable, FALSE); n++;
	
        edit_groupmasks[i] = XtCreateManagedWidget(editors_groupmasks[i], xmTextWidgetClass, bb,
                wargs, n);
        XmTextSetString(edit_groupmasks[i],editors_groupmasks[i]);
        }



        /*
         * add a OK button to accept text and to pop down the widget.
         */

        n = 0;
        XtSetArg(wargs[n], XmNx, 150); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i + 2); n++;

        ok_button = XtCreateManagedWidget(" Ok ", 
                                xmPushButtonGadgetClass,
                                bb, wargs, n);


        XtAddCallback(ok_button, XmNactivateCallback, 
                                (XtCallbackProc)done_dialog, bb);


        /*
         * add a button for asking for help.
        */


        n = 0;
        XtSetArg(wargs[n], XmNx, 300); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i + 2); n++;

        help_button = XtCreateManagedWidget("Help", 
                                xmPushButtonWidgetClass,
                                bb, wargs, n);

        XtAddCallback(help_button, XmNactivateCallback, 
                                (XtCallbackProc)xs_help_callback,
                                help_str_showGroupMasks);

return bb;
}







/****************************************************************************
	create show channel mask dialog
*****************************************************************************/
Widget createShowChanMasksDialog(parent,cdata)
Widget parent;
struct chanData *cdata;
{
Widget bb, ok_button, help_button;
Arg     wargs[10];
int     i, n=0,len=0,xloc;

	strcpy(editors_chanmasks[0],cdata->name);
	strcpy(editors_chanmasks[1],cdata->value);
 	 alGetMaskString(cdata->curMask,editors_chanmasks[2]);
 	 alGetMaskString(cdata->defaultMask,editors_chanmasks[3]);

	i=4; 	strcpy(editors_chanmasks[i],cdata->forcePVName);

	/* find pvforce value */
	i++;  
	strcpy(editors_groupmasks[i],"");
	strcpy(editors_groupmasks[i],"");
	if (cdata->forcechid != NULL) { 
		sprintf(editors_chanmasks[i],"%d",cdata->PVValue);
		}

	i++;    sprintf(editors_chanmasks[i],"%d",cdata->forcePVValue); 
	i++; 	sprintf(editors_chanmasks[i],"%d",cdata->resetPVValue); 
	i++; 	alGetMaskString(cdata->forcePVMask,editors_chanmasks[i]);
	i++; 	strcpy(editors_chanmasks[i],cdata->sevrPVName);
	i++; 	if (cdata->command == NULL) 
			strcpy(editors_chanmasks[i],""); 
		else
			strcpy(editors_chanmasks[i],cdata->command); 

        
        n =0;
        XtSetArg(wargs[n], XmNautoUnmanage, FALSE); n++;
        XtSetArg(wargs[n], XmNdialogTitle,
                XmStringLtoRCreate("Channel  Summary",
                XmSTRING_DEFAULT_CHARSET)); n++;
        XtSetArg(wargs[n], XmNallowShellResize, FALSE); n++;
        bb = XmCreateBulletinBoardDialog(parent, "Channel Summary", wargs, n);


        /* create text widget and label widget */

        for (i=0; i<XtNumber(labels_chanmasks); i++) {
        n = 0;
	if (len < strlen(labels_chanmasks[i]))
		len = strlen(labels_chanmasks[i]);
        XtSetArg(wargs[n], XmNlabelString, 
                XmStringCreateLtoR(labels_chanmasks[i],charset)); n++;
        XtSetArg(wargs[n], XmNx, 10); n++;
        XtSetArg(wargs[n], XmNy, 10 + 30*i); n++;

         label_chanmasks[i] = XtCreateManagedWidget(labels_chanmasks[i],
                 xmLabelGadgetClass, bb,
                 wargs, n);
         }

/*
 * get string width in pixel
 */
  xloc = 10 + len * char_width;

        for (i=0; i<XtNumber(editors_chanmasks); i++) {
        n = 0;
        XtSetArg(wargs[n], XmNx, 10+xloc); n++;
        XtSetArg(wargs[n], XmNy, 10 + 30*i); n++;	
if (i==10) XtSetArg(wargs[n], XmNwidth, 80*char_width);
        else
	   XtSetArg(wargs[n], XmNwidth, 30 * char_width); n++;

        XtSetArg(wargs[n], XmNeditable, FALSE); n++;
        XtSetArg(wargs[n],XmNcursorPositionVisible,FALSE);  n++;
	edit_chanmasks[i] = XtCreateManagedWidget(editors_chanmasks[i], xmTextWidgetClass, bb,
                wargs, n);
        XmTextSetString(edit_chanmasks[i],editors_chanmasks[i]);
        }



        /*
         * add a OK button to accept text and to pop down the widget.
         */

        n = 0;
        XtSetArg(wargs[n], XmNx, 100); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i+2); n++;

        ok_button = XtCreateManagedWidget(" Ok ", 
                                xmPushButtonGadgetClass,
                                bb, wargs, n);


        XtAddCallback(ok_button, XmNactivateCallback, 
                                (XtCallbackProc)done_dialog, bb);


        /*
         * add a button for asking for help.
         */


        n = 0;
        XtSetArg(wargs[n], XmNx, 250); n++;
        XtSetArg(wargs[n], XmNy, 10+30*i+2); n++;

        help_button = XtCreateManagedWidget("Help", 
                                xmPushButtonWidgetClass,
                                bb, wargs, n);

        XtAddCallback(help_button, XmNactivateCallback, 
                                (XtCallbackProc)xs_help_callback,
                                help_str_showChanMasks);


        return bb;

}





