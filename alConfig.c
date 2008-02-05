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
/* alConfig.c */

/************************DESCRIPTION***********************************
  Routines related to construction of alarm configuration 
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include "postfix.h"
#include "sllLib.h"
#include "alLib.h"
#include "alh.h"
#include "ax.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef UP
#define UP 1
#endif

#ifndef DOWN
#define DOWN 0
#endif

/* ALARM_ANY must not be equal to any valid alarm severity value */
#ifndef ALARM_ANY
#define ALARM_ANY ALH_ALARM_NSEV 
#endif
/* UP_ALARM must not be equal to any valid alarm severity value */
#ifndef UP_ALARM
#define UP_ALARM ALARM_ANY + 1
#endif

extern char * alhAlarmStatusString[];
extern char * alhAlarmSeverityString[];

extern int _global_flag;
extern int _passive_flag;
extern int DEBUG;
extern int _DB_call_flag;
extern int _description_field_flag;
char applicationName[64];  /* Albert1 applicationName = mainGroupName will be send to DB */
char deviceName[64]="";    /* Albert1 reserved;  will be send to DB */


/* alarm severity command list */
struct sevrCommand {
	ELLNODE node;       /* double link list node type */
	int direction;      /* alarm severity direction */
	int sev;            /* alarm severity level*/
	char *instructionString;        /* instructionstring text address */
	char *command;      /* command text address */
};

/* alarm status command list */
struct statCommand {
	ELLNODE node;       /* double link list node type */
	int stat;            /* alarm status value*/
	char *alhAlarmStatusString;        /* alarmStatusString text address */
	char *command;      /* command text address */
};

/* forward declarations */
static void print_error( char *buf, char *message);
static void GetGroupLine( char *buf, GLINK **pglink, struct mainGroup *pmainGroup);
static void GetIncludeLine( char *buf, GLINK **pglink,
int caConnect, struct mainGroup *pmainGroup);
static void GetChannelLine( char *buf, GLINK **pglink, CLINK **pclink,
int caConnect, struct mainGroup *pmainGroup);
static void GetOptionalLine( FILE *fp, char *buf,
GCLINK *gclink, int context, int caConnect, struct mainGroup *pmainGroup);
static void alWriteGroupConfig(FILE *fp,SLIST *pgroup);

static void alConfigTreePrint( FILE *fw, GLINK *glink, char  *treeSym);

/*****************************************************
   utility routine 
*******************************************************/
static void print_error(char *buf,char *message)
{
	errMsg("Input error: %s: %s\n",message,buf);
}

/*******************************************************************
 	get alarm  configuration 
*******************************************************************/
void alGetConfig(struct mainGroup *pmainGroup,char *filename,
int caConnect)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	CLINK *clink;
	GLINK *glink;
	GCLINK *gclink=0;
	int first_char;
	int context=0;

	if (filename[0] == '\0') return;

	fp = fopen(filename,"r");
	if(fp==NULL) {
		perror("Could not open Alarm Configuration File");
		exit(-1);
	}

	glink = pmainGroup->p1stgroup;
	clink = NULL;
	while( fgets(buf,MAX_STRING_LENGTH,fp) != NULL) {

		/* find first non blank character */

		first_char = 0;
		while( buf[first_char] == ' ' || buf[first_char] == '\t'
		    || buf[first_char] == '\n') first_char++;

		if (strncmp(&buf[first_char],"GROUP",5)==0) {
			GetGroupLine(&buf[first_char],&glink,pmainGroup);
			context = GROUP;
			gclink = (GCLINK *)glink;
		}
		else if (strncmp(&buf[first_char],"CHANNEL",7)==0) {
			GetChannelLine(&buf[first_char],&glink,&clink,caConnect,pmainGroup);
			context = CHANNEL;
			gclink = (GCLINK *)clink;
		}
		else if (strncmp(&buf[first_char],"$",1)==0) {
			GetOptionalLine(fp,&buf[first_char],gclink,context,caConnect,pmainGroup);
		}
		else if (strncmp(&buf[first_char],"INCLUDE",7)==0) {
			GetIncludeLine(&buf[first_char],&glink,caConnect,pmainGroup);
			context = GROUP;
			gclink = (GCLINK *)glink;
		}
		else if(buf[first_char]=='\0') {
		}
		else if(buf[first_char]=='#') {
		}
		else {
			print_error(buf,"Invalid input line");
		}
	}

	fclose(fp);
	return;

}

/*******************************************************************
	read the Group Line from configuration file
*******************************************************************/
static void GetGroupLine(char *buf,GLINK **pglink,
struct mainGroup *pmainGroup)
{
	GLINK		*glink;
	char 		command[20];
	int  		rtn;
	char 		parent[PVNAME_SIZE];
	char 		name[PVNAME_SIZE];
	struct groupData 	*gdata;
	GLINK 		*parent_link;


	parent_link = *pglink;

	rtn = sscanf(buf,"%20s%32s%32s",command,parent,name);

	if(rtn!=3) {
		print_error(buf,"Invalid GROUP command");
		return;
	}

	glink = alCreateGroup();
	glink->pmainGroup = pmainGroup;
	gdata = glink->pgroupData;
	if (gdata->name) free(gdata->name);
	gdata->name = (char *)calloc(1,strlen(name)+1);
	strcpy(gdata->name,name);
	if (_DB_call_flag) if(strlen(applicationName)==0) strncpy(applicationName,name,64);

	/*parent is NULL , i. e. main group*/
	if(strcmp("NULL",parent)==0)  	{

		/* commented out following lines to allow INCLUDE to work*/
		/*
		      if(pmainGroup->p1stgroup!=NULL) {
		      print_error(buf,"Missing parent");
		      return;
		      }
		      */

		if(pmainGroup->p1stgroup==NULL) {
		pmainGroup->p1stgroup = glink;
		glink->parent = NULL;
		*pglink = glink;
		return;
		} else {
		glink->parent = parent_link;
		alAddGroup(parent_link,glink);
		*pglink = glink;
		return;
		}
	}

	/* must find parent */

	while(parent_link!=NULL && strcmp(parent_link->pgroupData->name,parent)!=0)
		parent_link = parent_link->parent;

	if(parent_link==NULL) {
		print_error(buf,"Can not find parent");
		return;
	}

	glink->parent = parent_link;
	alAddGroup(parent_link,glink);
	*pglink = glink;

}

/*******************************************************************
	read the Include Line from configuration file
*******************************************************************/
static void GetIncludeLine(char *buf,GLINK **pglink,int caConnect,
struct mainGroup *pmainGroup)
{
	GLINK *glinkHold;
	char command[20];
	int  rtn;
	char parent[PVNAME_SIZE];
	char name[NAMEDEFAULT_SIZE];
	GLINK *parent_link=0;
	char    filename[NAMEDEFAULT_SIZE];


	if (pglink) parent_link = *pglink;

	rtn = sscanf(buf,"%20s%32s%s",command,parent,name);

	if(rtn!=3) {
		print_error(buf,"Invalid INCLUDE command");
		return;
	}

	/* use default config file directory */
	filename[0] = '\0';
	if (name[0] != '/' && psetup.configDir) {
		sprintf(filename,"%s/",psetup.configDir);
	}

	/* set filename */
	if (name[0] != '\0') strcat(filename,name);


	glinkHold = pmainGroup->p1stgroup;
   
	if(strcmp("NULL",parent)==0)       {
		if(pmainGroup->p1stgroup!=NULL) {
			print_error(buf,"Missing parent");
			return;
		}
		parent_link = NULL;
	}

	/* must find parent */
	while(parent_link!=NULL && strcmp(parent_link->pgroupData->name,parent)!=0)
		parent_link = parent_link->parent;


	pmainGroup->p1stgroup = parent_link;

	/* read config file */
	alGetConfig(pmainGroup,filename,caConnect);

	if (!pmainGroup->p1stgroup) {
		print_error(buf,"Ignoring Invalid INCLUDE file");
		return;
	}
	pmainGroup->p1stgroup = glinkHold;

	*pglink = parent_link;
}

/*******************************************************************
	read the Channel Line from configuration file
*******************************************************************/
#define TOKEN_MAXSIZE	127
static void GetChannelLine(char *buf,GLINK **pglink,CLINK **pclink,
int caConnect,struct mainGroup *pmainGroup)
{
	CLINK 		*clink;
	int  		rtn;
	char 		name[TOKEN_MAXSIZE+2];
	char 		*parent;
	char 		*mask;
	struct chanData 	*cdata;
	GLINK 		*parent_link;
	int			i;

	static char	token[5][TOKEN_MAXSIZE+1];

	for (i = 0; i < 5; i++) *token[i] = 0;
	rtn = sscanf
	    (buf, "%127s %127s %127s %127s %127s",
	    token[0], token[1], token[2], token[3], token[4]);

	/* CDEV line */
	if (strcmp (token[0], "CDEV") == 0)
	{
		if (rtn < 4) {
			print_error(buf,"Invalid CDEV command");
			return;
		}

		parent = token[1];
		/* name is "device attribte" */
		sprintf (name, "%s %s", token[2], token[3]);
		mask = token[4];
	}

	/* CHANNEL line */
	else	
	{
		if (rtn<3) {
			print_error(buf,"Invalid CHANNEL command");
			return;
		}

		parent = token[1];
		strcpy (name, token[2]);
		mask = token[3];
	}

	clink = alCreateChannel();
	clink->pmainGroup = pmainGroup;
	cdata = clink->pchanData;
	strncpy(cdata->name,name,PVNAME_SIZE);

	/* must find parent */
	parent_link = *pglink;
	while(parent_link!=NULL && strcmp(parent_link->pgroupData->name,parent)!=0)
		parent_link = parent_link->parent;

	if(parent_link==NULL) {
		print_error(buf,"Can not find parent");
		return;
	}

	alAddChan(parent_link,clink);
	clink->parent = parent_link;
	*pglink = clink->parent;
	*pclink = clink;

	if (mask) {
		alSetMask(mask,&(cdata->defaultMask));
		alSetCurChanMask(clink,cdata->defaultMask);
	}

	if (cdata->curMask.Cancel==1 || cdata->curMask.Disable==1) {
		cdata->curSevr=NO_ALARM;
	} else {
		cdata->curSevr=ERROR_STATE;
	}
	cdata->curStat=NO_ALARM;
	while(parent_link!=NULL) {
		parent_link->pgroupData->curSev[cdata->curSevr] ++;
		parent_link->pgroupData->unackSev[NO_ALARM] ++;
		parent_link->pgroupData->unackBeepSev[NO_ALARM] ++;
		parent_link = parent_link->parent;
	}

	if (caConnect && strlen(cdata->name) > (size_t) 1) {
		alCaConnectChannel(cdata->name,&cdata->chid,clink);
	}

	if (caConnect && cdata->curMask.Cancel == 0) {
		alCaAddEvent(cdata->chid,&cdata->evid,clink);
	}
	if (_description_field_flag) {
	  if( (cdata->description=malloc(128)) == NULL) {
	    fatalErrMsg("No memory ");
	  }
	  getDescriptionRecord(cdata->name,cdata->description,cdata->descriptionId);
	}
}

/*******************************************************************
	read the Optional Line from configuration file
*******************************************************************/
static void GetOptionalLine(FILE *fp,char *buf,GCLINK *gclink,
int context,int caConnect,struct mainGroup *pmainGroup)
{
	struct gcData *gcdata;
	struct chanData *cdata=0;
	char command[20];
	char name[PVNAME_SIZE];
	char buf2[MAX_STRING_LENGTH];
	char mask[6];
	char string[10];
	double dbl;
	short value = 1;
	int valueIn;
	float rate =1.0;
	float rateIn;
	char *str;
	int i,len;
	int rtn;

	/* config optional lines */
	if (strncmp(&buf[1],"BEEPSEVERITY",12)==0) { /*BEEPSEVERITY*/

		sscanf(buf,"%20s",command);
		len = strlen(command);
		while( buf[len] == ' ' || buf[len] == '\t') len++;
		for (i=1; i<ALH_ALARM_NSEV; i++) {
			if (strncmp(&buf[len],alhAlarmSeverityString[i],
			    strlen(alhAlarmSeverityString[i]))==0){
                psetup.beepSevr = i;
			}
		}
		return;
	}

    if (strncmp(&buf[1],"HEARTBEATPV",11)==0) { /*HEARTBEATPV*/

		if (pmainGroup->heartbeatPV.name) return;
        rtn = sscanf(buf,"%20s%32s%f%d",command,name,&rateIn,&valueIn);
        if(rtn>=2) {
        	if(rtn>=3) rate = rateIn;
	        if(rtn>=4) value = valueIn;
			alHeartbeatPVAdd(pmainGroup,name,rate,value);
            if (caConnect && strlen(pmainGroup->heartbeatPV.name)) {
                alCaConnectHeartbeatPV(pmainGroup->heartbeatPV.name,
					&(pmainGroup->heartbeatPV.chid),pmainGroup);
            }
		} else {
	        print_error(buf,"Invalid $HEARTBEATPV Line");
        }
        return;
    }


	/* group/channel optional lines */

	if(gclink==NULL) {
		print_error(buf,"Logic error: glink is NULL");
		return;
	}
	gcdata = gclink->pgcData;
	if(gcdata==NULL) {
		print_error(buf,"Logic error: gcdata is NULL");
		return;
	}

	if (strncmp(&buf[1],"BEEPSEVR",8)==0) { /*BEEPSEVR*/

		sscanf(buf,"%20s",command);
		len = strlen(command);
		while( buf[len] == ' ' || buf[len] == '\t') len++;
		for (i=1; i<ALH_ALARM_NSEV; i++) {
			if (strncmp(&buf[len],alhAlarmSeverityString[i],
			    strlen(alhAlarmSeverityString[i]))==0){
				gcdata->beepSevr = i;
			}
		}
		return;
	}

	if (strncmp(&buf[1],"FORCEPV ",8)==0) { /*FORCEPV*/

		if (gcdata->pforcePV && gcdata->pforcePV->name) return;
		if (!gcdata->pforcePV) gcdata->pforcePV=(FORCEPV*)calloc(1,sizeof(FORCEPV));
		dbl=0.0;
		rtn = sscanf(buf,"%20s%32s%6s%lf%9s",command,name,mask,&dbl,string);
		if(rtn>=3) alSetMask(mask,&(gcdata->pforcePV->forceMask));
		if (rtn >= 4) gcdata->pforcePV->forceValue = dbl;
		else gcdata->pforcePV->forceValue=1;
		if (rtn == 5) {
			if (strncmp(string,"NE",2)==0 || strncmp(string,"ne",2)==0) {
				gcdata->pforcePV->resetValue = gcdata->pforcePV->forceValue;
			} else {
				dbl=0.0;
				if (sscanf(string,"%lf",&dbl) >= 1) gcdata->pforcePV->resetValue = dbl;
				else gcdata->pforcePV->resetValue = 0;
			}
		} else {
				gcdata->pforcePV->resetValue = 0;
		}
		if(rtn>=2) {
			FORCEPVCADATA *puser;

			gcdata->pforcePV->name = (char *)calloc(1,strlen(name)+1);
			strcpy(gcdata->pforcePV->name,name);
			if (caConnect && strlen(gcdata->pforcePV->name)
				&&  strcmp(gcdata->pforcePV->name,"CALC") !=0 ) {
				gcdata->pforcePV->currentValue = -999;
				alCaConnectForcePV(gcdata->pforcePV->name,&gcdata->pforcePV->chid,gcdata->name);
				puser=(FORCEPVCADATA *)calloc(1,sizeof(FORCEPVCADATA));
				puser->index=-1;
				puser->link=gclink;
				puser->linktype=context;
				gcdata->pforcePV->puser = puser;
				alCaAddForcePVEvent (gcdata->pforcePV->chid,puser,&gcdata->pforcePV->evid);
			}
		}
		return;
	}

	if (strncmp(&buf[1],"FORCEPV_CALC ",13)==0) { /*FORCEPV_CALC*/
		FORCEPV_CALC* pcalc;
		long status=0;
		short err=0;

		if (!gcdata->pforcePV) gcdata->pforcePV=(FORCEPV*)calloc(1,sizeof(FORCEPV));
		if (!gcdata->pforcePV->pcalc)
			gcdata->pforcePV->pcalc=(FORCEPV_CALC*)calloc(1,sizeof(FORCEPV_CALC));
		pcalc=gcdata->pforcePV->pcalc;
		if (pcalc->expression) free(pcalc->expression);
		rtn = sscanf(buf,"%20s%s",command,buf2);
		pcalc->expression=(char*)calloc(strlen(buf2)+1,sizeof(char));
		strcpy(pcalc->expression,buf2);
		/* convert an algebraic expression to symbolic postfix */
		status=postfix(pcalc->expression,buf2,&err);
		pcalc->rpbuf=(char*)calloc(strlen(buf2)+1,sizeof(char));
		strcpy(pcalc->rpbuf,buf2);
		if(status || err) {
			errMsg("FORCEPV_CALC error converting '%s' to symbolic postfix: status=%ld err=%d\n",
				pcalc->expression,status,err);
		}
		return;
	}

	if (strncmp(&buf[1],"FORCEPV_CALC_",13)==0) { /*FORCEPV_CALC_A, FORCEPV_CALC_B, ...*/
		rtn = sscanf(buf,"%20s%s",command,name);
		if(rtn>=2) {
			FORCEPV_CALC* pcalc;
			FORCEPVCADATA *puser;
			short index;
			double dbl;
			const char letter[]={"ABCDEF"};

			for (index=0;index<NO_OF_CALC_PVS;index++)
				if (command[14]==letter[index]) break;
			if (index>NO_OF_CALC_PVS -1 || index<0) return;

			if (!gcdata->pforcePV)
				gcdata->pforcePV=(FORCEPV*)calloc(1,sizeof(FORCEPV));
			if (!gcdata->pforcePV->pcalc)
				gcdata->pforcePV->pcalc=
					(FORCEPV_CALC*)calloc(1,sizeof(FORCEPV_CALC));
			pcalc=gcdata->pforcePV->pcalc;

			if (pcalc->name[index]) return;
			pcalc->name[index]=(char*)calloc(strlen(name)+1,sizeof(char));
			strcpy(pcalc->name[index],name);
			dbl= atof(name);
/*####### errno needs work #######*/
			if (dbl!=0) {
				pcalc->value[index] = dbl;
			}else {
				pcalc->value[index] = -999;
				alCaConnectForcePV(pcalc->name[index],&pcalc->chid[index],gcdata->name);
				if (pcalc->puser[index]) free(pcalc->puser[index]);
				puser=(FORCEPVCADATA *)calloc(1,sizeof(FORCEPVCADATA));
				puser->index=index;
				puser->link=gclink;
				puser->linktype=context;
				pcalc->puser[index] = puser;
				alCaAddForcePVEvent (pcalc->chid[index],puser,&pcalc->evid[index]);
			}

		} else {
	        print_error(buf,"Invalid $FORCEPV_CALC Line");
        }
		return;
	}

	if (strncmp(&buf[1],"SEVRPV",6)==0) { /*SEVRPV*/

		if(strcmp(gcdata->sevrPVName,"-") != 0) return;
		rtn = sscanf(buf,"%20s%32s",command,name);
		if(rtn>=2) {
			gcdata->sevrPVName = (char *)calloc(1,strlen(name)+1);
			strcpy(gcdata->sevrPVName,name);
			if (caConnect && strlen(gcdata->sevrPVName) > (size_t) 1) {
				alCaConnectSevrPV(gcdata->sevrPVName,&gcdata->sevrchid,gcdata->name);
			}

		} else {
	        print_error(buf,"Invalid $SEVRPV Line");
        }

		return;
	}

	if (strncmp(&buf[1],"COMMAND",7)==0) { /*COMMAND*/
		int len;

		if (gcdata->command) return;
		sscanf(buf,"%20s",command);
		len = strlen(command);
		while( buf[len] == ' ' || buf[len] == '\t') len++;
		gcdata->command = (char *)calloc(1,strlen(&buf[len])+1);
		strcpy(gcdata->command,&buf[len]);
		if(gcdata->command[strlen(gcdata->command)-1] == '\n')
			gcdata->command[strlen(gcdata->command)-1] = '\0';
		return;
	}

	if (strncmp(&buf[1],"SEVRCOMMAND",11)==0) { /*SEVRCOMMAND*/
		int len;

		sscanf(buf,"%20s",command);
		len = strlen(command);
		while( buf[len] == ' ' || buf[len] == '\t') len++;
		str = (char *)calloc(1,strlen(&buf[len])+1);
		strcpy(str,&buf[len]);
		if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
		addNewSevrCommand(&gcdata->sevrCommandList,str);
		return;
	}

	if (strncmp(&buf[1],"STATCOMMAND",11)==0) { /*STATCOMMAND*/
		int len;

		if(context!=CHANNEL) {
			print_error(buf,"Logic error: STATCOMMAND: Context not a channel");
			return;
		}

		sscanf(buf,"%20s",command);
		len = strlen(command);
		while( buf[len] == ' ' || buf[len] == '\t') len++;
		str = (char *)calloc(1,strlen(&buf[len])+1);
		strcpy(str,&buf[len]);
		if(str[strlen(str)-1] == '\n') str[strlen(str)-1] = '\0';
		cdata=(struct chanData *)gcdata;
		addNewStatCommand(&cdata->statCommandList,str);
		return;
	}

	if (strncmp(&buf[1],"ALIAS",5)==0) { /*ALIAS*/
		int len;

		if (gcdata->alias) return;
		sscanf(buf,"%20s",command);
		len = strlen(command);
		while( buf[len] == ' ' || buf[len] == '\t') len++;
		gcdata->alias = (char *)calloc(1,strlen(&buf[len])+1);
		strcpy(gcdata->alias,&buf[len]);
		if(gcdata->alias[strlen(gcdata->alias)-1] == '\n')
			gcdata->alias[strlen(gcdata->alias)-1] = '\0';
		return;
	}

	if (strncmp(&buf[1],"ALARMCOUNTFILTER",16)==0) { /*ALARMCOUNTFILTER*/
		int count=1;
		int seconds=1;

		if(context!=CHANNEL) {
			print_error(buf,"Logic error: ALARMCOUNTFILTER: Context not a channel");
			return;
		}

		rtn = sscanf(buf,"%20s%i%i",command,&count,&seconds);
		if(rtn>=1) {
			cdata=(struct chanData *)gcdata;
			if (cdata->countFilter) return;
			cdata->countFilter = (COUNTFILTER *)calloc(1,sizeof(COUNTFILTER));
			cdata->countFilter->inputCount=1;
			cdata->countFilter->inputSeconds=1;
			cdata->countFilter->clink=gclink;
			cdata->countFilter->stat=NO_ALARM;
			cdata->countFilter->sev=ERROR_STATE;
		    cdata->countFilter->alarmTimeHistory = 0;
		    cdata->countFilter->countIndex = 0;
		}
		if(rtn>=2) cdata->countFilter->inputCount=count;
		if(rtn>=3) cdata->countFilter->inputSeconds=seconds;
		if (cdata->countFilter->inputCount) cdata->countFilter->alarmTimeHistory =
            (time_t *)calloc(2*(cdata->countFilter->inputCount),sizeof(time_t));
		return;
	}

	if (strncmp(&buf[1],"GUIDANCE",8)==0) { /*GUIDANCE*/
		int len;

		sscanf(buf,"%20s",command);
		len = strlen(command);
		while( buf[len] == ' ' || buf[len] == '\t' || buf[len] == '\n') len++;

		if (strlen(&buf[len])) {
			if (gclink->guidanceLocation) return;
			gclink->guidanceLocation = (char *)calloc(1,strlen(&buf[len])+1);
			strcpy(gclink->guidanceLocation,&buf[len]);
			if(gclink->guidanceLocation[strlen(gclink->guidanceLocation)-1] == '\n')
				gclink->guidanceLocation[strlen(gclink->guidanceLocation)-1] = '\0';
			return;
		} else {
			struct guideLink *pgl;

			while( fgets(buf,MAX_STRING_LENGTH,fp) != NULL) {

				/*change return to null for x message box*/

				if(buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0';

				/* find the first non blank character */
				i = 0;
				while( buf[i] == ' ' || buf[i] == '\t') i++;

				if(buf[i]=='$') {
					if(strncmp("$END",&buf[i],4) == 0 ||
					    strncmp("$End",&buf[i],4) == 0)
						return;
					else {
						print_error(buf,"Invalid End");
						return;
					}
				}
				pgl = (struct guideLink *)calloc(1,sizeof(struct guideLink));
				pgl->list=(char *)calloc(1,strlen(buf)+1);
				strcpy(pgl->list,buf);
				sllAdd(&(gclink->GuideList),(SNODE *)pgl);
			}

		}

		return;
	}
	if (strncmp(&buf[1],"ACKPV",5)==0) { /*ACKPV  Albert*/
		int rtn;
		rtn = sscanf(&buf[7],"%30s%32s",name,command);

		if(rtn>=2) {
            cdata=(struct chanData *)gcdata;
			cdata->ackPVName = (char *)calloc(1,strlen(name)+1);
			strcpy(cdata->ackPVName,name);
			cdata->ackPVValue=(short) atoi(command);
			if (caConnect && strlen(cdata->ackPVName) > (size_t) 1) {
				 alCaConnectAckPV(cdata->ackPVName,&cdata->ackPVId,cdata->name);
			}

		} else {
	        print_error(buf,"Invalid $ACKPV Line");
        }
		return;
	}
	print_error(buf,"Invalid Optional Line");
}

/*******************************************************************
	write system configuration file
*******************************************************************/
void alWriteConfig(char *filename,struct mainGroup *pmainGroup)
{
	FILE *fw;
	fw = fopen(filename,"w");
	if (!fw) return;
	if (psetup.beepSevr > 1)
		fprintf(fw,"$BEEPSEVERITY  %s\n",alhAlarmSeverityString[psetup.beepSevr]);
	/*alWriteGroupConfig(fw,(SLIST *)&(pmainGroup->p1stgroup));*/
	alWriteGroupConfig(fw,(SLIST *)pmainGroup);
	fclose(fw);
}

/*******************************************************************
	write system configuration file
*******************************************************************/
static void alWriteGroupConfig(FILE * fw,SLIST *pgroup)
{
	CLINK *clink;
	GLINK *glink,*parent;
	struct groupData *gdata;
	struct chanData *cdata;
	char pvmask[6],curmask[6];

	SNODE *pt,*cpt,*cl,*gl;
	struct guideLink *guidelist;
	struct sevrCommand *sevrCommand;
	struct statCommand *statCommand;

	pt = sllFirst(pgroup);
	while (pt) {
		glink = (GLINK *)pt;
		gdata = glink->pgroupData;
		parent = glink->parent;

		if (parent == NULL) {
			fprintf(fw,"GROUP    %-28s %-28s\n",
			    "NULL",gdata->name);
		} else {
			fprintf(fw,"GROUP    %-28s %-28s\n",
			    parent->pgroupData->name,
			    gdata->name);
		}

		if (gdata->beepSevr > 1)
			fprintf(fw,"$BEEPSEVR  %s\n",alhAlarmSeverityString[gdata->beepSevr]);

		if(gdata->pforcePV) {
			alGetMaskString(gdata->pforcePV->forceMask,pvmask);
			fprintf(fw,"$FORCEPV  %-28s %6s %g ",
			    gdata->pforcePV->name,
			    pvmask,
			    gdata->pforcePV->forceValue);
			if ((float)gdata->pforcePV->resetValue==(float)gdata->pforcePV->forceValue )
				fprintf(fw,"%s","NE\n");
			else fprintf(fw,"%g\n",gdata->pforcePV->resetValue);
			if (strcmp(gdata->pforcePV->name,"CALC")==0 && gdata->pforcePV->pcalc) {
				int i;
				const char *letter[]={"A","B","C","D","E","F"};

				fprintf(fw,"$FORCEPV_CALC  %s\n",gdata->pforcePV->pcalc->expression);
				for (i=0;i<NO_OF_CALC_PVS;i++) {
					if(gdata->pforcePV->pcalc->name[i])
					fprintf(fw,"$FORCEPV_CALC_%s  %s\n",letter[i],
							gdata->pforcePV->pcalc->name[i]);

				}
			}
		}

		if(strcmp(gdata->sevrPVName,"-") != 0)
			fprintf(fw,"$SEVRPV   %-28s\n",
			    gdata->sevrPVName);

		if (gdata->command!=NULL)
			fprintf(fw,"$COMMAND  %s\n",gdata->command);

		if (gdata->alias != NULL)
			fprintf(fw,"$ALIAS  %s\n",gdata->alias);

		sevrCommand=(struct sevrCommand *)ellFirst(&gdata->sevrCommandList);
		while (sevrCommand) {
			fprintf(fw,"$SEVRCOMMAND  %s\n",sevrCommand->instructionString);
			sevrCommand=(struct sevrCommand *)ellNext((ELLNODE *)sevrCommand);
		}

		if (glink->guidanceLocation!=NULL)
			fprintf(fw,"$GUIDANCE  %s\n",glink->guidanceLocation);

		gl = sllFirst(&(glink->GuideList));
		if (gl) fprintf(fw,"$GUIDANCE\n");
		while (gl) {
			guidelist = (struct guideLink *)gl;
			fprintf(fw,"%s\n",guidelist->list);
			gl = sllNext(gl);
			if (gl == NULL) fprintf(fw,"$END\n");
		}

		cpt = sllFirst(&(glink->chanList));
		while (cpt) {
			clink = (CLINK *)cpt;

			cdata = clink->pchanData;

			alGetMaskString(cdata->curMask,curmask);

			if (strcmp(curmask,"-----") != 0)
				fprintf(fw,"CHANNEL  %-28s %-28s %6s\n",
				    glink->pgroupData->name,
				    cdata->name,
				    curmask);
				else
				fprintf(fw,"CHANNEL  %-28s %-28s\n",
				    glink->pgroupData->name,
				    cdata->name);

			if (cdata->beepSevr > 1)
				fprintf(fw,"$BEEPSEVR  %s\n",alhAlarmSeverityString[cdata->beepSevr]);

			if(cdata->pforcePV) {
				alGetMaskString(cdata->pforcePV->forceMask,pvmask);
				fprintf(fw,"$FORCEPV  %-28s %6s %g ",
			    	cdata->pforcePV->name,
			    	pvmask,
			    	cdata->pforcePV->forceValue);
				if ((float)cdata->pforcePV->resetValue==(float)cdata->pforcePV->forceValue )
					fprintf(fw,"%s","NE\n");
				else fprintf(fw,"%g\n",cdata->pforcePV->resetValue);
				if (strcmp(cdata->pforcePV->name,"CALC")==0 && cdata->pforcePV->pcalc) {
					int i;
					const char *letter[]={"A","B","C","D","E","F"};
	
					fprintf(fw,"$FORCEPV_CALC  %s\n",cdata->pforcePV->pcalc->expression);
					for (i=0;i<NO_OF_CALC_PVS;i++) {
						if(cdata->pforcePV->pcalc->name[i])
							fprintf(fw,"$FORCEPV_CALC_%s  %s\n",letter[i],
								cdata->pforcePV->pcalc->name[i]);
					}
				}
			}
	
			if(strcmp(cdata->sevrPVName,"-") != 0)
				fprintf(fw,"$SEVRPV   %-28s\n",cdata->sevrPVName);

			if (cdata->command != NULL)
				fprintf(fw,"$COMMAND  %s\n",cdata->command);

			if (cdata->alias != NULL)
				fprintf(fw,"$ALIAS  %s\n",cdata->alias);

			if (cdata->countFilter != NULL)
				fprintf(fw,"$ALARMCOUNTFILTER  %i %i\n",cdata->countFilter->inputCount,\
               
				    cdata->countFilter->inputSeconds);

			sevrCommand=(struct sevrCommand *)ellFirst(&cdata->sevrCommandList);
			while (sevrCommand) {
				fprintf(fw,"$SEVRCOMMAND  %s\n",sevrCommand->instructionString);
				sevrCommand=(struct sevrCommand *)ellNext((ELLNODE *)sevrCommand);
			}

			statCommand=(struct statCommand *)ellFirst(&cdata->statCommandList);
			while (statCommand) {
				fprintf(fw,"$STATCOMMAND  %s\n",statCommand->alhAlarmStatusString);
				statCommand=(struct statCommand *)ellNext((ELLNODE *)statCommand);
			}
			cpt = sllNext(cpt);

			if (clink->guidanceLocation != NULL)
				fprintf(fw,"$GUIDANCE  %s\n",clink->guidanceLocation);

			cl = sllFirst(&(clink->GuideList));
			if (cl)  fprintf(fw,"$GUIDANCE\n");
			while (cl) {
				guidelist = (struct guideLink *)cl;
				fprintf(fw,"%s\n",guidelist->list);
				cl = sllNext(cl);
				if (cl == NULL) fprintf(fw,"$END\n");
			}



		}
		alWriteGroupConfig(fw,&(glink->subGroupList));
		pt = sllNext(pt);
	}

}

/*******************************************************************
	create a config with a single unnamed main Group 
*******************************************************************/
void alCreateConfig(struct mainGroup *pmainGroup)
{
	GLINK *glink;

	glink = alCreateGroup();

	pmainGroup->p1stgroup = glink;

	return;
}

/***************************************************
  treePrint
****************************************************/
static void alConfigTreePrint(FILE *fw,GLINK *glink,char *treeSym)
{
	CLINK *clink;
	struct groupData *gdata;
	struct chanData *cdata;
	SNODE	*pt;
	int length;

	int symSize = 3;
	static char symMiddle[]="+--";
	static char symContinue[]="|  ";
	static char symEnd[]="+--";
	static char symBlank[]="   ";
	static char symNull[]="\0\0\0";

	if (glink == NULL) return;
	gdata = glink->pgroupData;

	length = strlen(treeSym);
	if (length >= MAX_TREE_DEPTH) return;


	/* find next view sibling */
	pt = sllNext(glink);

	if (length){
		if (pt) strncpy(&treeSym[length-symSize],symMiddle,symSize);
		else strncpy(&treeSym[length-symSize],symEnd,symSize);
		strncpy(&treeSym[length],symNull,symSize);
	}

		fprintf(fw,"%s%-28s\n",
		    treeSym,
		    gdata->name);

	if (length){
		if (pt) strncpy(&treeSym[length-symSize],symContinue,symSize);
		else strncpy(&treeSym[length-symSize],symBlank,symSize);
	}

	pt = sllFirst(&(glink->subGroupList));
	if (pt) strncpy(&treeSym[length],symContinue,symSize);
	else strncpy(&treeSym[length],symBlank,symSize);

	pt = sllFirst(&(glink->chanList));
	while (pt){

		clink = (CLINK *)pt;
		cdata = clink->pchanData;

		fprintf(fw,"%s  %-28s\n",
		    treeSym,
		    cdata->name);

		pt = sllNext(pt);
	}

	strncpy(&treeSym[length],symBlank,symSize);

	pt = sllFirst(&(glink->subGroupList));
	while (pt){
		alConfigTreePrint(fw, (GLINK *)pt, treeSym);
		pt = sllNext(pt);
	}

	strncpy(&treeSym[length],symNull,symSize);
	return;
}

/*******************************************************************
  print config tree structure
  *******************************************************************/
void alPrintConfig(FILE *fw,struct mainGroup *pmainGroup)
{
	GLINK *glinkTop;
	char    treeSym[MAX_TREE_DEPTH+1];

	memset(treeSym,'\0',MAX_TREE_DEPTH);
	glinkTop = (GLINK *)sllFirst(pmainGroup);

	alConfigTreePrint(fw,glinkTop,treeSym);
}

/*******************************************************************
    addNewSevrCommand
*******************************************************************/
void addNewSevrCommand(ELLLIST *pList,char *str)
{
	struct sevrCommand *sevrCommand;
	int len=0;
	int i=0;

	sevrCommand = (struct sevrCommand *)calloc(1, sizeof(struct sevrCommand));
	sevrCommand->instructionString = str;
	while( str[len] != ' ' && str[len] != '\t' && str[len] != '\0') len++;
	while( str[len] == ' ' || str[len] == '\t') len++;
	sevrCommand->command = &str[len];
	if(str[0]=='D') {
		sevrCommand->direction = DOWN;
		len = 5;
	} else {
		sevrCommand->direction = UP;
		len = 3;
	}
	for (i=0; i<ALH_ALARM_NSEV; i++) {
		if (strncmp(&str[len],alhAlarmSeverityString[i],
		    strlen(alhAlarmSeverityString[i]))==0) break;
	}
	if (strncmp(&str[len],"ALARM",5)==0) sevrCommand->sev = UP_ALARM;
	else sevrCommand->sev = i;
	ellAdd(pList,(void *)sevrCommand);

}

/*******************************************************************
    removeSevrCommandList
*******************************************************************/
void removeSevrCommandList(ELLLIST *pList)
{
	struct sevrCommand *sevrCommand;
	ELLNODE *pt;

	pt=ellFirst(pList);
	while (pt) {
		sevrCommand=(struct sevrCommand *)pt;
		ellDelete(pList,pt);
		pt=ellNext(pt);
		free(sevrCommand->instructionString);
		free(sevrCommand);
	}
}

/*******************************************************************
    copySevrCommandList
*******************************************************************/
void copySevrCommandList(ELLLIST *pListOld,ELLLIST *pListNew)
{
	struct sevrCommand *ptOld, *ptNew;

	ptOld=(struct sevrCommand *)ellFirst(pListOld);
	while (ptOld) {
		ptNew = (struct sevrCommand *)calloc(1, sizeof(struct sevrCommand));
		ptNew->instructionString = (char *)calloc(1,strlen(ptOld->instructionString)+1);
		strcpy(ptNew->instructionString,ptOld->instructionString);
		ptNew->command =  ptNew->instructionString + (ptOld->command - ptOld->instructionString);
		ptNew->direction = ptOld->direction;
		ptNew->sev = ptOld->sev;
		ellAdd(pListNew,(void *)ptNew);
		ptOld=(struct sevrCommand *)ellNext((ELLNODE *)ptOld);
	}
}

/*******************************************************************
    spawnSevrCommandList
*******************************************************************/
void spawnSevrCommandList(ELLLIST *pList,int sev,int sevr_prev)
{
	struct sevrCommand *sevrCommand;
	int direction;

	sevrCommand=(struct sevrCommand *)ellFirst(pList);
	if (sevrCommand) {
		if ( sev > sevr_prev ) direction=UP;
		else direction=DOWN;
		while (sevrCommand) {
			if (sevrCommand->direction==direction) {
				if (sevrCommand->sev==ALARM_ANY || sevrCommand->sev==sev )
					processSpawn_callback(NULL,sevrCommand->command,NULL);
				else if (direction==UP && sevrCommand->sev==UP_ALARM &&
				    sevr_prev==0)
					processSpawn_callback(NULL,sevrCommand->command,NULL);
			}
			sevrCommand=(struct sevrCommand *)ellNext((ELLNODE *)sevrCommand);
		}
	}
}

/*******************************************************************
    getStringSevrCommandList
*******************************************************************/
void getStringSevrCommandList(ELLLIST *pList,char **pstr)
{
	char *str;
	struct sevrCommand *sevrCommand;
	ELLNODE *pt;
	int i;

	pt = ellFirst(pList);
	i=0;
	while (pt) {
		sevrCommand = (struct sevrCommand *)pt;
		i += strlen(sevrCommand->instructionString);
		i += 1;
		pt = ellNext(pt);
	}
	str = (char*)calloc(1,i+1);
	pt = ellFirst(pList);
	i=0;
	while (pt) {
		sevrCommand = (struct sevrCommand *)pt;
		strcat(str,sevrCommand->instructionString);
		pt = ellNext(pt);
		if (pt) strcat(str,"\n");
		i++;
	}
	*pstr = str;
}

/*******************************************************************
    addNewStatCommand
*******************************************************************/
void addNewStatCommand(ELLLIST *pList,char *str)
{
	struct statCommand *statCommand;
	int len=0;
	int i=0;

	statCommand = (struct statCommand *)calloc(1, sizeof(struct statCommand));
	statCommand->alhAlarmStatusString = str;
	while( str[len] != ' ' && str[len] != '\t' && str[len] != '\0') len++;
	while( str[len] == ' ' || str[len] == '\t') len++;
	statCommand->command = &str[len];
	for (i=0; i<ALH_ALARM_NSTATUS; i++) {
		if (strncmp(str,alhAlarmStatusString[i],
		    strlen(alhAlarmStatusString[i]))==0) break;
	}
	statCommand->stat = i;
	ellAdd(pList,(void *)statCommand);

}

/*******************************************************************
    removeStatCommandList
*******************************************************************/
void removeStatCommandList(ELLLIST *pList)
{
	struct statCommand *statCommand;
	ELLNODE *pt;

	pt=ellFirst(pList);
	while (pt) {
		statCommand=(struct statCommand *)pt;
		ellDelete(pList,pt);
		pt=ellNext(pt);
		free(statCommand->alhAlarmStatusString);
		/* ?????????
		       free(statCommand->command);
		       */
		free(statCommand);
	}
}

/*******************************************************************
    copyStatCommandList
*******************************************************************/
void copyStatCommandList(ELLLIST *pListOld,ELLLIST *pListNew)
{
	struct statCommand *ptOld, *ptNew;

	ptOld=(struct statCommand *)ellFirst(pListOld);
	while (ptOld) {
		ptNew = (struct statCommand *)calloc(1, sizeof(struct statCommand));
		ptNew->alhAlarmStatusString = (char *)calloc(1,strlen(ptOld->alhAlarmStatusString)+1);
		strcpy(ptNew->alhAlarmStatusString,ptOld->alhAlarmStatusString);
		ptNew->command =  ptNew->alhAlarmStatusString + (ptOld->command - ptOld->alhAlarmStatusString);
		ptNew->stat = ptOld->stat;
		ellAdd(pListNew,(void *)ptNew);
		ptOld=(struct statCommand *)ellNext((ELLNODE *)ptOld);
	}
}

/*******************************************************************
  spawnStatCommandList
*******************************************************************/
void spawnStatCommandList(ELLLIST *pList,int stat,int stat_prev)
{
	struct statCommand *statCommand;

	statCommand=(struct statCommand *)ellFirst(pList);
	if ( statCommand && stat != stat_prev) {
		while (statCommand) {
			if (statCommand->stat == stat  )
				processSpawn_callback(NULL,statCommand->command,NULL);
			statCommand=(struct statCommand *)ellNext((ELLNODE *)statCommand);
		}
	}
}

/*******************************************************************
  getStringStatCommandList
*******************************************************************/
void getStringStatCommandList(ELLLIST *pList,char **pstr)
{
	char *str;
	struct statCommand *statCommand;
	ELLNODE *pt;
	int i;

	pt = ellFirst(pList);
	i=0;
	while (pt) {
		statCommand = (struct statCommand *)pt;
		i += strlen(statCommand->alhAlarmStatusString);
		i += 1;
		pt = ellNext(pt);
	}
	str = (char*)calloc(1,i+1);
	pt = ellFirst(pList);
	i=0;
	while (pt) {
		statCommand = (struct statCommand *)pt;
		strcat(str,statCommand->alhAlarmStatusString);
		pt = ellNext(pt);
		if (pt) strcat(str,"\n");
		i++;
	}
	*pstr = str;
}


  
