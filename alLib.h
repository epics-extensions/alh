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

static char *alLibhsccsId = "@(#) $Id$";

typedef struct mask {			/* mask bit setting */
	unsigned Cancel    : 1;
	unsigned Disable   : 1;
	unsigned Ack       : 1;
	unsigned AckT      : 1;
	unsigned Log	   : 1;
	unsigned Unused    : 11;
} MASK;

typedef struct countFilter {
	int    inputCount;
	int    inputSeconds;
	short  stat;        /* channel status from CA */
	short  sev;         /* current severity */
	char   value[MAX_STRING_SIZE];	/* channel value from CA */
	int    curCount;
	int    alarmTime;
	void  *clink;
	XtIntervalId timeoutId;
} COUNTFILTER;

/* group/channel data structure */
struct gcData {
	char *name;		/* group name */
	char *forcePVName;	/* force PV name */
	char *sevrPVName;	/* severity PV name */
	short PVValue;		/* PV value */

	short forcePVValue;	/* PV value for force mask */

	short resetPVValue;	/* PV value for reset mask */
	MASK forcePVMask;	/* force Mask */
	char *alias;	 	/* alias text */
	char *command;	 	/* command text */
	ELLLIST sevrCommandList;	/* alarm severity command list */
	short curSevr;		/* current severity */
	short unackSevr;	/* highest unack severity */
	chid forcechid;			/* forcePV channel id */
	evid forceevid;			/* forcePV channel evid */
	chid sevrchid;			/* group sevrPV channel id */
};

/* group data structure */
struct groupData {
	char *name;		/* group name */
	char *forcePVName;	/* force PV name */
	char *sevrPVName;	/* severity PV name */
	short PVValue;		/* PV value */

	short forcePVValue;		/* PV value for force mask */

	short resetPVValue;		/* PV value for reset mask */
	MASK forcePVMask;		/* force Mask */
	char *alias;	 	/* alias text */
	char *command;	 		/* command text */
	ELLLIST sevrCommandList;	/* severity command list */
	short curSevr;			/* current highestseverity from CA */
	short unackSevr;		/* highest unack severity */
	chid forcechid;			/* forcePV channel id */
	evid forceevid;			/* forcePV channel evid */
	chid sevrchid;			/* group sevrPV channel id */
	char *treeSym;                  /* tree symbols for treeWindow display */
	short mask[ALARM_NMASK];	/* no. of channels of masked types*/
	short curSev[ALH_ALARM_NSEV];  	/* channels of different severity */
	short unackSev[ALH_ALARM_NSEV];  	/* channels of unacknowledged sev */
};

/* channel data structure */
struct chanData {
	char *name; 		/* channel name, or device/attribute */
	char *forcePVName;	/* force PV name */
	char *sevrPVName;	/* severity PV name */
	short PVValue;		/* PV value */

	short forcePVValue;		/* PV value for force mask */
	short resetPVValue;		/* PV value for reset mask */
	MASK forcePVMask;		/* forced mask setting */
	char *alias;	 	/* alias text */
	char *command;			/* command text */

	ELLLIST sevrCommandList;	/* severity command list */
	short curSevr;			/* channel severity from CA */
	short unackSevr;		/* highest unack severity */
	chid forcechid;		 	/* forcePV channel id */
	evid forceevid;			/* forcePV channel evid */
	chid sevrchid;		 	/* sevrPV channel id */
	ELLLIST statCommandList;	/* alarm status command list */
	COUNTFILTER *countFilter;	/* alarm count filter */
	MASK curMask;			/* current mask setting */
	MASK defaultMask;		/* default mask setting */
	char value[MAX_STRING_SIZE];	/* channel value from CA */
	short curStat;			/* channel status from CA */
	short unackStat;		/* unack status */
	chid chid;			/* chid from CA search */
	evid evid;			/* evid from CA add event */
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
	struct groupLink *parent;	/* parent groupLinke pointer */
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

