static char *sccsId = "@(#)sllLib.c	1.7\t10/15/93";

/*********************************************************
*	 sllLib.c 
*
*This file contains   *PUBLIC*   functions for single link list
*operation.
*
**********************************************************/




#include <stdio.h>
#include "sllLib.h"


/*********************************************************
	add a node at the end of the list
**********************************************************/
int sllAdd(list,new)
SNODE *new;
SLIST *list;
{
        if (list->count > 0) {
                list->last->next = new;
                
                }
        else
                list->first = new;
	new->next = NULL;
        list->last = new;
        list->count++;
        return(0);
}

/*********************************************************
	find the N th node of the list
**********************************************************/
SNODE *sllNth(list,n)
int n;
SLIST *list;
{
SNODE *pt;
int i,count;
        count = list->count;
        if (n >= count) return(list->last);
        if ( n <= 0) return(list->first);
        pt = list->first;
        for (i=1;i<n;i++) 
                pt = pt->next;
        return(pt);
}


/*********************************************************
	insert a node after prev node of the list
**********************************************************/
void sllInsert(list,prev,new)
SLIST *list;
SNODE *prev,*new;
{
        if (prev == NULL) {
                new->next = list->first;
                list->first = new;
                }
        else if (prev == list->last) {
                prev->next = new;
                list->last = new;
		new->next = NULL;
                }
        else {
                new->next = prev->next;
                prev->next = new;
                }
	list->count++;
	return;

}


/*********************************************************
	insert a new node before next node of the list
**********************************************************/
int sllPrecede(list,next,new)
     SLIST *list;
     SNODE *next,*new;
{
     SNODE *prev,*last,*now;

        if (next == NULL) {
                if (list->count == 0){
                        list->first = new;
                        list->last = new;
                }
                else {
                        last = list->last;
                        last->next = new;
                        list->last = new;
                }
        }
        else {
                prev = NULL;
                now = list->first;

                while (now ) {
                        if (now == next) break;
                        prev = now;
                        now = now->next;
                }
                if (now) {
                        new->next = now;
                        if (prev == NULL)
                             list->first = new;
                        else prev->next = new;
                }
                else return(-1);
        }
        list->count++;
        return(0);
}

/*********************************************************
	remove the node from the list
**********************************************************/
void sllRemove(list,node)
SLIST *list;
SNODE *node;
{

SNODE *prev,*now,*last;
        prev = NULL;
        now = list->first;
        last = list->last;
        
        while (now ) {
                if (now == node) break;
                else {
                        prev = now;
                        now = now->next;
                        }
                }
        if (now) {
        if (prev == NULL){
             list->first = now->next;
             prev = node;
        }
        else    prev->next = now->next;
        if (node == last) list->last = prev;
        list->count--;
        }
}


/*********************************************************
	find the node from the list
**********************************************************/
int sllFind(plist,pnode)
SLIST *plist;
SNODE *pnode;
{
int i;
SNODE *pt;
        pt = plist->first;
        for (i=1;i<=plist->count;i++) {
                if (pt == pnode) {
                return(i);
                }
        pt = sllNext(pt);
        }
        return(-1);             
}


