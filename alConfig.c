/*
 $Log$
 Revision 1.3  1995/02/28 16:43:27  jba
 ansi c changes

 * Revision 1.2  1994/06/22  21:16:26  jba
 * Added cvs Log keyword
 *
 */

static char *sccsId = "@(#)alConfig.c	1.15\t12/15/93";

/* 
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
 * .01  06-12-91        bkc     Including an additional input of sevrPVName in 
 *				configuration file
 * .02  07-22-91        bkc     Change data type for forcePVValue & resetPVValue
 * .03  08-08-91        mrk	Major Revision
 * .04  12-10-93        jba Changes for new command line dir and file options
 */


/*
*-----------------------------------------------------------------
*    routines related to construction of alarm configuration 
*-----------------------------------------------------------------
*
-------------
|   PUBLIC  |
-------------
alGetConfig(pmainGroup, filename,caConnect)        		Get alarm system
        struct mainGroup *pmainGroup;
        char *filename;                 	input config file
        int   caConnect;                 	connect to CA? (TRUE/FALSE)
*
alWriteGroupConfig(fp,pgroup)                      Write group configuration
	FILE  *fp;
        SLIST *pgroup;
*
void alCreateConfig(pmainGroup)                 Create empty configuration
    struct mainGroup *pmainGroup;
*
void alPrintConfig(pmainGroup)
    struct mainGroup *pmainGroup;
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <sllLib.h>
#include <alLib.h>
#include <alh.h>
#include <ax.h>

#ifndef TRUE
#define TRUE 1
#endif

#define BUF_SIZE 150
#define GROUP_LINE 1
#define CHANNEL_LINE 2

extern int DEBUG;

#ifdef __STDC__

static void print_error( char *buf, char *message);
static void GetGroupLine( char *buf, GLINK **pglink, struct mainGroup *pmainGroup);
static void GetIncludeLine( char *buf, GLINK **pglink,
      int caConnect, struct mainGroup *pmainGroup);
static void GetChannelLine( char *buf, GLINK **pglink, CLINK **pclink,
      int caConnect, struct mainGroup *pmainGroup);
static void GetOptionalLine( FILE *fp, char *buf,
    GLINK *glink, CLINK *clink, int context, int caConnect);
static void alConfigTreePrint( FILE *fw, GLINK *glink, char  *treeSym);

#else

static void print_error();
static void GetGroupLine();
static void GetChannelLine();
static void GetIncludeLine();
static void GetOptionalLine();
static void alConfigTreePrint();

#endif /*__STDC__*/


/*****************************************************
   utility routine 
*******************************************************/
static void print_error(buf,message)
    char *buf;
    char *message;
{
    printf("%s\nError in previous line: %s\n",buf,message);
}

/*******************************************************************
 	get alarm  configuration 
*******************************************************************/
void alGetConfig(pmainGroup,filename,caConnect)
struct mainGroup *pmainGroup;
char *filename;
int caConnect;
{
    FILE *fp;
    char buf[BUF_SIZE];
    CLINK *clink;
    GLINK *glink;
    int context=0;
    int first_char;

    if (filename[0] == '\0') return;

    fp = fopen(filename,"r");
    if(fp==NULL) {
	perror("Could not open Alarm Configuration File");
	exit(-1);
    }

    glink = NULL;
    clink = NULL;
    while( fgets(buf,BUF_SIZE,fp) != NULL) {
	
	/* find first non blank character */

	first_char = 0;
	while( buf[first_char] == ' ' || buf[first_char] == '\t') first_char++;

	if(toupper(buf[first_char])=='G') {
	    GetGroupLine(&buf[first_char],&glink,pmainGroup);
	    context = GROUP_LINE;
	    clink = NULL;
	    }
	else if(toupper(buf[first_char])=='C') {
	    GetChannelLine(&buf[first_char],&glink,&clink,caConnect,pmainGroup);
	    context = CHANNEL_LINE;
	    }
	else if(buf[first_char]=='$') {
	    GetOptionalLine(fp,&buf[first_char],glink,clink,context,caConnect);
	    }
	else if(toupper(buf[first_char])=='I') {
	    GetIncludeLine(&buf[first_char],&glink,caConnect,pmainGroup);
	    context = GROUP_LINE;
	    clink = NULL;
	    }
	else {
	     
     	    /*printf("Illegal line: %s\n",buf);*/
    	    }
    }
    /*
     *  pendio and add change connection events
     */

    if (caConnect) alCaStartEvents((SLIST *)pmainGroup);

    fclose(fp);
    return;

}


/*******************************************************************
	read the Group Line from configuration file
*******************************************************************/
static void GetGroupLine(buf,pglink,pmainGroup)
    char *buf;
    GLINK **pglink;
    struct mainGroup *pmainGroup;
{
    GLINK *glink;
    char command[20];
    int  rtn;
    char parent[PVNAME_SIZE];
    char name[PVNAME_SIZE];
    struct groupData *gdata;
    GLINK *parent_link;


    parent_link = *pglink; 

    rtn = sscanf(buf,"%20s%32s%32s",command,parent,name);

    if(rtn!=3) {
	print_error(buf,"Illegal Group command");
	return;
    	}

    glink = alAllocGroup();
    glink->pmainGroup = pmainGroup;
    gdata = glink->pgroupData;
    gdata->name = (char *)calloc(1,strlen(name)+1);
    strcpy(gdata->name,name);

 
	alSetMask("-----",&(gdata->forcePVMask));
 	gdata->forcePVValue = 1;
	gdata->resetPVValue = 0;
	gdata->forcePVName = "-";
	gdata->sevrPVName = "-";


    /*parent is NULL , i. e. main group*/
    if(strcmp("NULL",parent)==0)  	{ 

/* commented out following lines to allow INCLUDE to work*/
/*
	if(pmainGroup->p1stgroup!=NULL) {
	    print_error(buf,"Missing parent");
	    return;
		}
*/

	pmainGroup->p1stgroup = glink;
	glink->parent = NULL;
  	*pglink = glink;	
	return;
    	}

    /* must find parent */

    while(parent_link!=NULL && strcmp(parent_link->pgroupData->name,parent)!=0)
	parent_link = parent_link->parent;

    	if(parent_link==NULL) {
		print_error(buf,"can not find parent");
		return;
    		}

 	glink->parent = parent_link;
    	alAddGroup(parent_link,glink);
    	*pglink = glink;

}



/*******************************************************************
	read the Include Line from configuration file
*******************************************************************/
static void GetIncludeLine(buf,pglink,caConnect,pmainGroup)
     char *buf;
     GLINK **pglink;
     int caConnect;
     struct mainGroup *pmainGroup;
{
     GLINK *glink;
     GLINK *glinkHold;
     char command[20];
     int  rtn;
     char parent[PVNAME_SIZE];
     char name[PVNAME_SIZE];
     GLINK *parent_link;
     char    filename[NAMEDEFAULT_SIZE];


     parent_link = *pglink; 

     rtn = sscanf(buf,"%20s%32s%32s",command,parent,name);

     if(rtn!=3) {
          print_error(buf,"Illegal Include command");
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
     /* read config file */
     alGetConfig(pmainGroup,filename,caConnect);

     if ( !pmainGroup->p1stgroup) {
           print_error(buf,"Ignoring Invalid Include file");
           return;
     }
     glink = pmainGroup->p1stgroup;
     pmainGroup->p1stgroup = glinkHold;

     /*parent is NULL , i. e. main group*/
 
     if(strcmp("NULL",parent)==0)       { 
          if(pmainGroup->p1stgroup!=NULL) {
               print_error(buf,"Missing parent");
               return;
          }

          pmainGroup->p1stgroup = glink;
          glink->parent = NULL;
             *pglink = glink;     
          return;
     }

     /* must find parent */

     while(parent_link!=NULL && strcmp(parent_link->pgroupData->name,parent)!=0)
      parent_link = parent_link->parent;
 
     if(parent_link==NULL) {
          print_error(buf,"can not find parent");
          return;
     }

     glink->parent = parent_link;
          alAddGroup(parent_link,glink);
          *pglink = glink;

}



/*******************************************************************
	read the Channel Line from configuration file
*******************************************************************/
static void GetChannelLine(buf,pglink,pclink,caConnect,pmainGroup)
    char *buf;
    GLINK **pglink;
    CLINK **pclink;
    int caConnect;
    struct mainGroup *pmainGroup;
{
    CLINK *clink;
    char command[20];
    int  rtn;
    char parent[PVNAME_SIZE];
    char name[PVNAME_SIZE];
    char mask[6];
    struct chanData *cdata;
    GLINK *parent_link;


    rtn = sscanf(buf,"%20s%32s%32s%6s",command,parent,name,mask);
    if(rtn<3) {
	print_error(buf,"Illegal Channel command");
	return;
    	}

    clink = alAllocChan();
    clink->pmainGroup = pmainGroup;
    cdata = clink->pchanData;
    cdata->name = (char *)calloc(1,strlen(name)+1);
    strcpy(cdata->name,name);

 
    if(rtn==4) alSetMask(mask,&(cdata->curMask));
	else
		alSetMask("-----",&(cdata->curMask));

	cdata->defaultMask = cdata->curMask;
	cdata->forcePVMask = cdata->curMask;
	cdata->forcePVValue = 1;
	cdata->resetPVValue = 0;
	cdata->forcePVName = "-";
	cdata->sevrPVName = "-";

    /* must find parent */

    parent_link = *pglink;
    while(parent_link!=NULL && strcmp(parent_link->pgroupData->name,parent)!=0)
	parent_link = parent_link->parent;

    if(parent_link==NULL) {
	print_error(buf,"can not find parent");
	return;
    	}

    alAddChan(parent_link,clink);
    clink->parent = parent_link;
    *pglink = clink->parent;
    *pclink = clink;

    if (caConnect && strlen(cdata->name) > (size_t) 1)
         alCaSearchName(cdata->name,&(cdata->chid));

}


/*******************************************************************
	read the Optional Line from configuration file
*******************************************************************/
static void GetOptionalLine(fp,buf,glink,clink,context,caConnect)
    FILE *fp;
    char *buf;
    GLINK *glink;
    CLINK *clink;
    int context;
    int caConnect;
{
    struct groupData *gdata=NULL;
    struct chanData *cdata=NULL;
    char command[20];
    char name[PVNAME_SIZE];
    char mask[6];
    short f1,f2;

    if(context==GROUP_LINE) {
	if(glink==NULL) {
	    print_error(buf,"Logic error: glink is NULL");
	    return;
	}
	gdata = glink->pgroupData;
    } else if(context==CHANNEL_LINE) {
	if(clink==NULL) {
	    print_error(buf,"Logic error: clink is NULL");
	    return;
	}
	cdata = clink->pchanData;
    } else {
	print_error(buf,"No GROUP has been defined");
	return;
    	}

    if(buf[1]=='F') { /*FORCEPV*/
	int rtn;

	if(context==GROUP_LINE) {
	    rtn = sscanf(buf,"%20s%32s%6s%hd%hd",command,name,
		mask,&f1,&f2);
	    if(rtn>=2) {
		gdata->forcePVName = (char *)calloc(1,strlen(name)+1);
		strcpy(gdata->forcePVName,name);
        if (caConnect && strlen(gdata->forcePVName) > (size_t) 1)
            alCaSearchName(gdata->forcePVName,&(gdata->forcechid));
	    	}
	    if(rtn>=3) alSetMask(mask,&(gdata->forcePVMask));
	    if (rtn >= 4) gdata->forcePVValue = f1;
	    if (rtn == 5) gdata->resetPVValue = f2;

	    }

	else {   /*must be CHANNEL_LINE*/
	    rtn = sscanf(buf,"%20s%32s%6s%hd%hd",command,name,
		mask,&f1,&f2);
	    if(rtn>=2) {
		cdata->forcePVName = (char *)calloc(1,strlen(name)+1);
		strcpy(cdata->forcePVName,name);
        if (caConnect && strlen(cdata->forcePVName) > (size_t) 1)
            alCaSearchName(cdata->forcePVName,&(cdata->forcechid));
	        }
	    if(rtn>=3) alSetMask(mask,&cdata->forcePVMask);
	    if (rtn >= 4) cdata->forcePVValue = f1;
	    if (rtn == 5) cdata->resetPVValue = f2;
	    }
	return;
    }

    if(buf[1]=='S') { /*SEVRPV*/
	int rtn;

	if(context==GROUP_LINE) {
	    rtn = sscanf(buf,"%20s%32s",command,name);
	    if(rtn>=2) {
		gdata->sevrPVName = (char *)calloc(1,strlen(name)+1);
		strcpy(gdata->sevrPVName,name);
        if (caConnect && strlen(gdata->sevrPVName) > (size_t) 1)
            alCaSearchName(gdata->sevrPVName,&(gdata->sevrchid));
	    	}
	    }

	else {/*must be CHANNEL_LINE*/
	    rtn = sscanf(buf,"%20s%32s",command,name);
	    if(rtn>=2) {
		cdata->sevrPVName = (char *)calloc(1,strlen(name)+1);
		strcpy(cdata->sevrPVName,name);
        if (caConnect && strlen(cdata->sevrPVName) > (size_t) 1)
            alCaSearchName(cdata->sevrPVName,&(cdata->sevrchid));
	    	}

	    }
	return;
    }

    if(buf[1]=='C') { /*COMMAND*/
	int len;

	sscanf(buf,"%20s",command);
	len = strlen(command);
	while( buf[len] == ' ' || buf[len] == '\t') len++;
	if(context==GROUP_LINE) {
	    gdata->command = (char *)calloc(1,strlen(&buf[len])+1);
	    strcpy(gdata->command,&buf[len]);
	    } 

	else { /*must be CHANNEL_LINE*/
	    cdata->command = (char *)calloc(1,strlen(&buf[len])+1);
	    strcpy(cdata->command,&buf[len]);
 	    }
	return;
    }

    if(buf[1]=='G') { /*GUIDANCE*/
 	struct guideLink *pgl;
	int first_char;

    	while( fgets(buf,BUF_SIZE,fp) != NULL) {

	    /*change return to blank for x message box*/

	    if(buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = '\0'; 

	   /* find the first non blank character */

	first_char = 0;
	while( buf[first_char] == ' ' || buf[first_char] == '\t') first_char++;


	    if(buf[first_char]=='$') {
	        if(strncmp("$END",&buf[first_char],4) == 0 || 
			strncmp("$End",&buf[first_char],4) == 0)
			return;
		else {
		 print_error(buf,"Illegal End");
		 return;
			}
	        }


	    pgl = (struct guideLink *)calloc(1,sizeof(struct guideLink));
	    pgl->list=(char *)calloc(1,strlen(buf)+1-first_char);
	    strcpy(pgl->list,&buf[first_char]);

	    if(context==GROUP_LINE) sllAdd(&(glink->GuideList),(SNODE *)pgl);
	    else sllAdd(&(clink->GuideList),(SNODE *)pgl);

	}
	
    }
    print_error(buf,"Illegal Optional Line");
}



/*******************************************************************
	write system configuration file
*******************************************************************/  
void alWriteConfig(filename,pmainGroup)
char *filename;
struct mainGroup *pmainGroup;
{
FILE *fw;
           fw = fopen(filename,"w");
           alWriteGroupConfig(fw,(SLIST *)&(pmainGroup->p1stgroup));
           fclose(fw);
}



/*******************************************************************
	write system configuration file
*******************************************************************/
void alWriteGroupConfig(fw,pgroup)
FILE * fw;
SLIST *pgroup;
{
CLINK *clink;
GLINK *glink,*parent;
struct groupData *gdata;
struct chanData *cdata;
char pvmask[6],curmask[6];

SNODE *pt,*cpt,*cl,*gl;
struct guideLink *guidelist;

        pt = sllFirst(pgroup);
        while (pt) {
                glink = (GLINK *)pt;

		gdata = glink->pgroupData;

		parent = glink->parent;

                alGetMaskString(gdata->forcePVMask,pvmask);

		if (parent == NULL) {

 
			fprintf(fw,"GROUP    %-28s %-28s\n",
				 "NULL",gdata->name);
			if(strcmp(gdata->forcePVName,"-") != 0)
				fprintf(fw,"$FORCEPV  %-28s %6s %3d %3d\n",
				gdata->forcePVName,
				pvmask,
				gdata->forcePVValue,
				gdata->resetPVValue);

			if(strcmp(gdata->sevrPVName,"-") != 0)
				fprintf(fw,"$SEVRPV   %-28s\n",
				gdata->sevrPVName);

			if (gdata->command != NULL)
			fprintf(fw,"$COMMAND  %s",gdata->command);


			}

		else {
 			fprintf(fw,"GROUP    %-28s %-28s\n",
				parent->pgroupData->name,
				gdata->name);

			if(strcmp(gdata->forcePVName,"-") != 0)
				fprintf(fw,"$FORCEPV  %-28s %6s %3d %3d\n",
				gdata->forcePVName,
				pvmask,
				gdata->forcePVValue,
				gdata->resetPVValue);

			if(strcmp(gdata->sevrPVName,"-") != 0)
				fprintf(fw,"$SEVRPV   %-28s\n",
				gdata->sevrPVName);

			if (gdata->command!=NULL)
			fprintf(fw,"$COMMAND  %s\n",gdata->command);



			}

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
			alGetMaskString(cdata->forcePVMask,pvmask);

			if (strcmp(curmask,"-----") != 0) 
			    fprintf(fw,"CHANNEL  %-28s %-28s %6s\n",
				glink->pgroupData->name,
				cdata->name,
				curmask);
			else
			    fprintf(fw,"CHANNEL  %-28s %-28s\n",
				glink->pgroupData->name,
				cdata->name);

			if(strcmp(cdata->forcePVName,"-") != 0)
			fprintf(fw,"$FORCEPV  %-28s %6s %3d %3d\n",
				cdata->forcePVName,
				pvmask,
				cdata->forcePVValue,
				cdata->resetPVValue);

			if(strcmp(cdata->sevrPVName,"-") != 0)
			fprintf(fw,"$SEVRPV   %-28s\n",cdata->sevrPVName);
			if (cdata->command != NULL)
			fprintf(fw,"$COMMAND  %s",cdata->command);

                        cpt = sllNext(cpt);


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
void alCreateConfig(pmainGroup)
    struct mainGroup *pmainGroup;
{
    GLINK *glink;

    glink = alCreateGroup();

	pmainGroup->p1stgroup = glink;

	return;
}


/***************************************************
  treePrint
****************************************************/

static void alConfigTreePrint(fw,glink,treeSym)
     FILE *fw;
     GLINK *glink;
     char  *treeSym;
{
     CLINK *clink;
     struct groupData *gdata;
     struct chanData *cdata;
     char pvmask[6],curmask[6];

     SNODE *cl,*gl;
     struct guideLink *guidelist;
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

     awGetMaskString(gdata->mask,curmask);

#if 0
     if (strcmp(curmask,"-----") != 0)
         fprintf(fw,"%s%-28s %6s\n",
         treeSym,
         gdata->name,
         curmask);
     else
#endif
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

#if 0
            alGetMaskString(gdata->forcePVMask,pvmask);

            if(strcmp(gdata->forcePVName,"-") != 0)
                fprintf(fw,"%s        FORCEPV  %-28s %6s %3d %3d\n",
                treeSym,
                gdata->forcePVName,
                pvmask,
                gdata->forcePVValue,
                gdata->resetPVValue);

            if(strcmp(gdata->sevrPVName,"-") != 0)
                fprintf(fw,"%s        SEVRPV   %-28s\n",
                treeSym,
                gdata->sevrPVName);

            if (gdata->command != NULL)
            fprintf(fw,"%s        COMMAND  %s",treeSym,gdata->command);

            gl = sllFirst(&(glink->GuideList));
            if (gl) fprintf(fw,"%s        GUIDANCE\n",treeSym);
            while (gl) {
                guidelist = (struct guideLink *)gl;
                fprintf(fw,"%s        %s\n",treeSym,guidelist->list);
                gl = sllNext(gl);
                if (gl == NULL) fprintf(fw,"%s        END\n",treeSym);
                }

#endif

     pt = sllFirst(&(glink->chanList));
     while (pt){

            clink = (CLINK *)pt;
            cdata = clink->pchanData;

            alGetMaskString(cdata->curMask,curmask);
            alGetMaskString(cdata->forcePVMask,pvmask);

#if 0

            if (strcmp(curmask,"-----") != 0)
                fprintf(fw,"%s  CHANNEL  %-28s %6s\n",
                treeSym,
                cdata->name,
                curmask);
            else
#endif
                fprintf(fw,"%s  %-28s\n",
                treeSym,
                cdata->name);

#if 0
            if(strcmp(cdata->forcePVName,"-") != 0)
            fprintf(fw,"%s        FORCEPV  %-28s %6s %3d %3d\n",
                treeSym,
                cdata->forcePVName,
                pvmask,
                cdata->forcePVValue,
                cdata->resetPVValue);

            if(strcmp(cdata->sevrPVName,"-") != 0)
            fprintf(fw,"%s        SEVRPV   %-28s\n",treeSym,cdata->sevrPVName);
            if (cdata->command != NULL)
            fprintf(fw,"%s        COMMAND  %s",treeSym,cdata->command);

            cl = sllFirst(&(clink->GuideList));
            if (cl)  fprintf(fw,"%s        GUIDANCE\n",treeSym);
            while (cl) {
                guidelist = (struct guideLink *)cl;
                fprintf(fw,"%s        %s\n",treeSym,guidelist->list);
                cl = sllNext(cl);
                if (cl == NULL) fprintf(fw,"%s        END\n",treeSym);
                }

#endif

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
void alPrintConfig(fw,pmainGroup)
    FILE *fw;
    struct mainGroup *pmainGroup;
{
     GLINK *glinkTop;
     char    treeSym[MAX_TREE_DEPTH+1];

     memset(treeSym,'\0',MAX_TREE_DEPTH);
     glinkTop = (GLINK *)sllFirst(pmainGroup); 

     alConfigTreePrint(fw,glinkTop,treeSym);

}


