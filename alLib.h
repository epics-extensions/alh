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
/* alLib.h */

/************************DESCRIPTION***********************************
  Group and channel data structure definitions
**********************************************************************/

#ifndef INCalLibh
#define INCalLibh 1

#include "cadef.h"

#include "alh.h"
#include "sllLib.h"
#include "ellLib.h"

typedef struct mask {			/* mask bit setting */
	unsigned Cancel    : 1;
	unsigned Disable   : 1;
	unsigned Ack       : 1;
	unsigned AckT      : 1;
	unsigned Log	   : 1;
	unsigned Unused    : 11;
} MASK;

typedef struct alarmEvent {
	short  stat;
	short  sevr;
	short  acks;
	short  ackt;
	char   value[MAX_STRING_SIZE];
	int    alarmTime;
	void  *clink;
} ALARMEVENT;

typedef struct countFilter {
	int    inputCount;
	int    inputSeconds;
	short  stat;        /* channel status from CA */
	short  sev;         /* current severity */
	short  acks;		/* highest unack severity */
	short  ackt;		/* acknowledge transients? */
	char   value[MAX_STRING_SIZE];	/* channel value from CA */
	int    countIndex;
	int    alarmTime;
	time_t *alarmTimeHistory;
	void  *clink;
	XtIntervalId timeoutId;
} COUNTFILTER;

typedef struct forcePVdata {
	short index;
	void *link;
	short linktype;
} FORCEPVCADATA;

#ifndef NO_OF_CALC_PVS
#define NO_OF_CALC_PVS 6
#endif

typedef struct calc {
	char *expression;				/* calc expression */
	char *rpbuf;					/* calc expression in reverse polish */
	char *name[NO_OF_CALC_PVS];		/* pv name */
	chid chid[NO_OF_CALC_PVS];		/* pv channel id */
	evid evid[NO_OF_CALC_PVS];		/* pv event id */
	FORCEPVCADATA* puser[NO_OF_CALC_PVS];	/* pv current value */
	double value[NO_OF_CALC_PVS];	/* pv current value */
} FORCEPV_CALC;

typedef struct forcePV {
    char *name;            /* pv name */
    chid chid;             /* pv channel id */
    evid evid;             /* pv channel event id */
	FORCEPVCADATA* puser;  /* pv current value */
    double currentValue;    /* pv current value */
    double forceValue;      /* pv value for force mask */
    double resetValue;      /* pv value for reset mask */
    short disabled;        /* pv disabled? TRUE/FALSE */
    MASK forceMask;        /* force mask */
    FORCEPV_CALC* pcalc;
} FORCEPV;

/* group/channel data structure */
struct gcData {
	char *name;		/* group name */
	char *sevrPVName;	/* severityPV name */
	FORCEPV *pforcePV;	/* forcePV info */
	char *alias;	 	/* alias text */
	char *command;	 	/* command text */
	ELLLIST sevrCommandList;	/* alarm severity command list */
	short curSevr;		/* current severity */
	short unackSevr;	/* highest unack severity */
	short unackBeepSevr;  	/* highest unack severity for beeping */
	short highestBeepSevr;  /* highest beep severity */
	chid sevrchid;			/* group sevrPV channel id */
	short beepSevr;		/* beep severity */
	XtIntervalId noAckTimerId;
};

/* group data structure */
struct groupData {
	char *name;		/* group name */
	char *sevrPVName;	/* severityPV name */
	FORCEPV *pforcePV;	/* forcePV info */
	char *alias;	 	/* alias text */
	char *command;	 		/* command text */
	ELLLIST sevrCommandList;	/* severity command list */
	short curSevr;			/* current highestseverity from CA */
	short unackSevr;		/* highest unack severity */
	short unackBeepSevr;  	/* highest unack severity for beeping */
	short highestBeepSevr;  /* highest beep severity */
	chid sevrchid;			/* group sevrPV channel id */
	short beepSevr;		/* beep severity */
	XtIntervalId noAckTimerId;
	char *treeSym;                  /* tree symbols for treeWindow display */
	int mask[ALARM_NMASK];	/* no. of channels of masked types*/
	int beepSev[ALH_ALARM_NSEV]; /* channels of different beep severity */
	int curSev[ALH_ALARM_NSEV];  	/* channels of different severity */
	int unackSev[ALH_ALARM_NSEV];  	/* channels of unacknowledged sevr */
	int unackBeepSev[ALH_ALARM_NSEV];  	/* channels of unacknowledged sev >= beep sevr */
};

/* channel data structure */
struct chanData {
	char *name; 		/* channel name, or device/attribute */
	char *sevrPVName;	/* severityPV name */
	FORCEPV *pforcePV;	/* forcePV info */
	char *alias;	 	/* alias text */
	char *command;			/* command text */
	ELLLIST sevrCommandList;	/* severity command list */
	short curSevr;			/* channel severity from CA */
	short unackSevr;		/* highest unack severity */
	short unackBeepSevr;  	/* highest unack severity for beeping */
	short highestBeepSevr;  /* highest beep severity */
	chid sevrchid;		 	/* sevrPV channel id */
	short beepSevr;		/* beep severity */
	XtIntervalId noAckTimerId;
	ELLLIST statCommandList;	/* alarm status command list */
	COUNTFILTER *countFilter;	/* alarm count filter */
	MASK curMask;			/* current mask setting */
	MASK defaultMask;		/* default mask setting */
	char value[MAX_STRING_SIZE];	/* channel value from CA */
	short curStat;			/* channel status from CA */
	chid chid;			/* chid from CA search */
	evid evid;			/* evid from CA add event */
        char *description;              /* Info from PV .DESC field */
        chid descriptionId;             /* his (.DESC) chid  */
        char *ackPVName;                /* ackPV name        */
	chid ackPVId;                   /* id of prev.       */
        short ackPVValue;               /* value to ackPV    */
	ALARMEVENT caAlarmEvent;	/* saved ca alarm event data */
};

/* group link  */
struct groupLink {
	SNODE node;			/* single link list node type */
	SLIST GuideList;		/* guidance link list */
	char *guidanceLocation;		/* guidance location (url or filename)*/
	struct groupLink *parent;	/* parent groupLink pointer */
	struct groupData *pgroupData;	/* group data  pointer */
	struct mainGroup *pmainGroup;   /* mainGroup pointer */
	void *lineTreeW;		/* line address Tree Window*/
	void *lineGroupW;		/* line address Group Window*/
	int viewCount;	 		/* count of open groups */
	int modified;			/* modified indicator */
	SLIST subGroupList;		/* subgroup link list */
	SLIST chanList;			/* channel link list */
};

/* channel link */
struct chanLink {
	SNODE node;			/* single link list node type */
	SLIST GuideList;		/* guidance link list */
	char *guidanceLocation;		/* guidance location (url or filename)*/
	struct groupLink *parent;	/* parent groupLinke pointer */
	struct chanData *pchanData;	/* channel data  pointer */
	struct mainGroup *pmainGroup;   /* mainGroup pointer */
	void *lineTreeW;		/* line address Tree Window*/
	void *lineGroupW;		/* line address Group Window*/
	int viewCount;	 		/* open/closed indicator */
	int modified;			/* modified indicator */

};

/* any link */
struct anyLink {
	SNODE node;			/* single link list node type */
	SLIST GuideList;		/* guidance link list */
	char *guidanceLocation;		/* guidance location (url or filename)*/
	struct groupLink *parent;	/* parent groupLink pointer */
	struct gcData *pgcData;  	/* channel data  pointer */
	struct mainGroup *pmainGroup;   /* mainGroup pointer */
	void *lineTreeW;		/* line address Tree Window*/
	void *lineGroupW;		/* line address Group Window*/
	int viewCount;	 		/* open/closed count or indicator */
	int modified;			/* modified indicator */
};

/* guidance link */
struct guideLink {
	SNODE node;			/* single link list node type */
	char *list;			/* guidance list address */
};

typedef struct groupLink GLINK;
typedef struct chanLink CLINK;
typedef struct anyLink GCLINK;

#endif /* INCalLibh */

