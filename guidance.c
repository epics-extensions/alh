/*
 $Id$
 */

#include <stdlib.h>
#include <Xm/Xm.h>

#include <alLib.h>
#include <ax.h>

/************************************************************************
 Guidance display callback
 ***********************************************************************/
void guidanceCallback(Widget w,GCLINK *gclink,XmAnyCallbackStruct *cbs)
{
     SNODE *pt;
     struct guideLink *guidelist;
     char *guidance_str[200];
     int i=0;

     if (guidanceExists(gclink)) {
         if (gclink->guidanceLocation ) {
              callBrowser(gclink->guidanceLocation);
         } else {
              pt = sllFirst(&(gclink->GuideList));
              i=0;
              while (pt) {
                   guidelist = (struct guideLink *)pt;
                   guidance_str[i] = guidelist->list;
                   pt = sllNext(pt);
                   i++;
              }
              guidance_str[i] = "";
              guidance_str[i+1] = "";

              xs_help_callback(w,guidance_str,cbs);
         }
      }
      else {
          if (gclink->pgcData->alias){
              createDialog(w,XmDIALOG_WARNING,"No guidance for ",
                  gclink->pgcData->alias);
          } else {
              createDialog(w,XmDIALOG_WARNING,"No guidance for ",
                  gclink->pgcData->name);
          }
      }
}

/************************************************************************
 Guidance exists test
 ***********************************************************************/
int guidanceExists(GCLINK *link)
{
     if (sllFirst(&(link->GuideList)) || link->guidanceLocation) return(TRUE);
     else return(FALSE);
}
 
/************************************************************************
 Delete guidance 
 ***********************************************************************/
void guidanceDeleteGuideList(SLIST *pGuideList)
{
    SNODE *node,*next;
    struct guideLink *guidelist;

    node = sllFirst(pGuideList);
    while (node) {
        next = sllNext(node);
        guidelist = (struct guideLink *)node;
        free(guidelist->list);
        free(guidelist);
        node = next;
    }
}

/************************************************************************
 Copy guidance 
 ***********************************************************************/
void guidanceCopyGuideList(SLIST *pToGuideList,SLIST *pFromGuideList)
{
    char *buff;
    struct guideLink *guideLink;
    SNODE *node;
    SLIST GuideList;

    node = sllFirst(pFromGuideList);
    while (node) {
        buff = ((struct guideLink *)node)->list;
        guideLink = (struct guideLink *)calloc(1,sizeof(struct guideLink));
        guideLink->list = (char *)calloc(1,strlen(buff)+1);
        strcpy(guideLink->list,buff);
        sllAdd(pToGuideList,(SNODE *)guideLink);
        node = sllNext(node);
    }

}

