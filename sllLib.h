/*
 $Log$
 Revision 1.3  1995/10/20 16:51:00  jba
 Modified Action menus and Action windows
 Renamed ALARMCOMMAND to SEVRCOMMAND
 Added STATCOMMAND facility
 Added ALIAS facility
 Added ALARMCOUNTFILTER facility
 Make a few bug fixes.

 * Revision 1.2  1994/06/22  21:18:02  jba
 * Added cvs Log keyword
 *
 */

#ifndef INCsllLibh
#define INCsllLibh 1

/*  sllLib.h 
	header file for single link list
 */       

static char *sllLibhSccsId = "@(#)sllLib.h	1.6\t8/4/93";

struct snode {
        struct snode *next;
        };
typedef struct snode SNODE;

struct list {
        SNODE *first;
        SNODE *last;
        int count;
        };
typedef struct list SLIST;

#define sllFirst(PSLIST) \
        ((SNODE *)(((SLIST *)(PSLIST))->first))

#define sllInit(PSLIST) \
        (((SLIST *)(PSLIST))->first = NULL,\
         ((SLIST *)(PSLIST))->last = NULL,\
         ((SLIST *)(PSLIST))->count = 0)

#define sllLast(PSLIST) \
        ((SNODE *)(((SLIST *)(PSLIST))->last))

#define sllNext(PSNODE) \
        (((PSNODE) == NULL) ? NULL : (((SNODE *)(PSNODE))->next))

SNODE *sllNth();

/********************************************************************
  sllLib.c   function prototypes
*********************************************************************/

#ifdef __STDC__

int sllAdd( SLIST *list, SNODE *new);
SNODE *sllNth( SLIST *list, int n);
void sllInsert( SLIST *list, SNODE *prev,SNODE *new);
int sllPrecede( SLIST *list, SNODE *next,SNODE *new);
void sllRemove( SLIST *list, SNODE *node);
int sllFind( SLIST *plist, SNODE *pnode);

#else

int sllAdd();
SNODE *sllNth();
void sllInsert();
int sllPrecede();
void sllRemove();
int sllFind();

#endif /*__STDC__*/

/*------------------------------------------------------------------------------------
Routines defined in sllLib.c:

sllAdd(plist,new)                        Add the new node to the plist
SNODE *sllNth(plist,n)                   Get the Nth node of the plist
int sllInsert(plist,prev,new)            Insert the new node after the prev node
int sllRemove(plist,node)                Remove the node  from the plist
int sllFind(plist,pnode)                 Find the pnode from the plist


Macros defined in sllLib.h 

SNODE *sllFirst(plist)			Return the first node of the plist
sllInit(plist) 				Initialize plist structure
SNODE *sllLast(plist)			Return the last node of the plist
SNODE *sllNext(pnode)			Return the next node pointer

----------------------------------------------------------------------------------------*/
#endif
