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
/* sllLib.h */

/************************DESCRIPTION***********************************
  *PUBLIC* functions for single link list operations.
**********************************************************************/

#ifndef INCsllLibh
#define INCsllLibh 1

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

/********************************************************************
  sllLib.c   function prototypes
*********************************************************************/
int sllAdd ( SLIST * plist, SNODE *pnode);
SNODE *sllNth ( SLIST*plist, int n);
void sllInsert( SLIST *plist, SNODE *pprev,SNODE *pnew);
int sllPrecede( SLIST *plist, SNODE *pnext,SNODE *pnew);
void sllRemove( SLIST *plist, SNODE *pnode);
int sllFind( SLIST *plist, SNODE *pnode);

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
