/* alCA.c
 *
 * $Id$
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <ax.h>

#define DEBUG_CALLBACKS 0

/* global variables */
static char buff[100];
int toBeConnectedCount = 0;
unsigned long caDelay = 100;        /* ms */
extern XtIntervalId caTimeoutId = (XtIntervalId) 0;
extern XtAppContext appContext;

/* forward declarations */
static void alCaNewAlarmEvent(struct event_handler_args args);
static void alCaGroupForceEvent(struct event_handler_args args);
static void alCaChannelForceEvent(struct event_handler_args args);
static void alCaChannelConnectionEvent(struct connection_handler_args args);
static void alCaForcePVConnectionEvent(struct connection_handler_args args);
static void alCaSevrPVConnectionEvent(struct connection_handler_args args);
static void alCaChannelAccessRightsEvent(struct access_rights_handler_args args);
static void alCaForcePVAccessRightsEvent(struct access_rights_handler_args args);
static void alCaSevrPVAccessRightsEvent(struct access_rights_handler_args args);
static void alCaUpdate(XtPointer cd, XtIntervalId * id);
static void alCaException(struct exception_handler_args args);

/*********************************************************************
 alCaPend
 *********************************************************************/
void alCaPend(double waitSeconds)
{
    time_t startTime, currentTime;

    currentTime = time(&startTime);
    while (toBeConnectedCount > 0 && difftime(currentTime, startTime) < waitSeconds) {
        ca_pend_event(.1);
        time(&currentTime);
    }
}

/*********************************************************************
 alCaIsConnected
 *********************************************************************/
short alCaIsConnected(chid chid)
{
    if (!chid) return FALSE;
    if (ca_field_type(chid) == TYPENOTCONN) return FALSE;
    else return TRUE;
}

/*********************************************************************
 alCaFlushIo
 *********************************************************************/
void alCaFlushIo()
{
    ca_flush_io();
}

/*********************************************************************
 init ca, add excptn handler, reg fd, add timer proc
 *********************************************************************/
void alCaInit()
{

#if DEBUG_CALLBACKS
    {
        printf("alCaInit: caTimeoutId=%d\n", caTimeoutId);
    }
#endif

    /* Initialize channel access */
    SEVCHK(ca_task_initialize(), "alCaInit: error in ca_task_initialize");

    /* Register exception handler */
    SEVCHK(ca_add_exception_event(alCaException, NULL),
           "alCaInit: error in ca_add_exception_event");

    /* Register file descriptor callback */
    SEVCHK(ca_add_fd_registration(registerCA, NULL),
           "alCaInit: error in ca_add_fd_registration");

    /* Start the CA poll and update areas timer proc */
    caTimeoutId = XtAppAddTimeOut(appContext, caDelay, alCaUpdate, NULL);

#if DEBUG_CALLBACKS
    printf("          caTimeoutId=%d\n", caTimeoutId);
#endif
}

/*********************************************************************
 Callback when there is activity on a CA file descriptor
 *********************************************************************/
void alCaPoll()
{

#if DEBUG_CALLBACKS
    {
        static int n = 0;

        printf("alCaPoll: n=%d\n", n++);
    }
#endif

    ca_poll();
}

/*********************************************************************
 cancel timer, close channel access
 *********************************************************************/
void alCaStop()
{
    /* cancel timeout */
    if (caTimeoutId) {
        XtRemoveTimeOut(caTimeoutId);
        caTimeoutId = (XtIntervalId) 0;
    }
    /* and close channel access */
    SEVCHK(ca_task_exit(), "alCaStop: error in ca_task_exit");

}

/*********************************************************************
  create chid, start search, add connection event handler, add puser,
  add access_rights event handler
 *********************************************************************/
void alCaConnectChannel(char *name, chid * pchid, void *puser)
{
    int status;

    if (strlen(name) <= (size_t) 1) return;

    toBeConnectedCount++;
    status = ca_search_and_connect(name, pchid, alCaChannelConnectionEvent, puser);
    if (status != ECA_NORMAL) alLogConnection(ca_name(*pchid),
                        "alCaConnectChannel:ca_search_and_connect failed.");

    status = ca_replace_access_rights_event(*pchid, alCaChannelAccessRightsEvent);
    if (status != ECA_NORMAL) alLogConnection(ca_name(*pchid),
                 "alCaConnectChannel:ca_replace_access_rights_event failed.");
}

/*********************************************************************
  create chid, start search, add connection event handler, add puser,
  add access_rights event handler
 *********************************************************************/
void alCaConnectForcePV(char *name, chid * pchid, void *puser)
{
    int status;

    if (strlen(name) <= (size_t) 1) return;

    toBeConnectedCount++;
    status = ca_search_and_connect(name, pchid, alCaForcePVConnectionEvent, puser);
    if (status != ECA_NORMAL) alLogConnection(ca_name(*pchid),
                        "alCaConnectForcePV:ca_search_and_connect failed.");
    status = ca_replace_access_rights_event(*pchid, alCaForcePVAccessRightsEvent);
    if (status != ECA_NORMAL) alLogConnection(ca_name(*pchid),
                 "alCaConnectForcePV:ca_replace_access_rights_event failed.");
}

/*********************************************************************
  create chid, start search, add connection event handler, add puser,
  add access_rights event handler
 *********************************************************************/
void alCaConnectSevrPV(char *name, chid * pchid, void *puser)
{
    int status;

    if (strlen(name) <= (size_t) 1) return;

    toBeConnectedCount++;
    status = ca_search_and_connect(name, pchid, alCaSevrPVConnectionEvent, puser);
    if (status != ECA_NORMAL) alLogConnection(ca_name(*pchid),
                        "alCaConnectSevrPV:ca_search_and_connect failed.");
    status = ca_replace_access_rights_event(*pchid, alCaSevrPVAccessRightsEvent);
    if (status != ECA_NORMAL) alLogConnection(ca_name(*pchid),
                  "alCaConnectSevrPV:ca_replace_access_rights_event failed.");
}

/*********************************************************************
 clear a channel chid
 *********************************************************************/
void alCaClearChannel(chid * pchid)
{
    int status;

    if (!*pchid) return;

    status = ca_clear_channel(*pchid);
    if (status != ECA_NORMAL)
        alLogConnection(ca_name(*pchid), "alCaClearChannel:ca_clear_channel failed.");
    *pchid = NULL;
}


/*********************************************************************
 clear event
 *********************************************************************/
void alCaClearEvent(evid * pevid)
{
    int status;

    if (!*pevid) return;

    status = ca_clear_event(*pevid);
    if (status != ECA_NORMAL) alLogConnection(ca_name((*pevid)->chan),
                        "alCaClearEvent:ca_clear_event failed.");
    *pevid = NULL;
}

/*********************************************************************
 add channel alarm event handler
 *********************************************************************/
void alCaAddEvent(chid chid, evid * pevid, void *clink)
{
    int status;

    if (!chid) return;

    status = ca_add_masked_array_event(DBR_STSACK_STRING, 1,
                                       chid,
                                       alCaNewAlarmEvent,
                                       clink,
                                       (float) 0, (float) 0, (float) 0,
                                       pevid,
                                       DBE_ALARM);
    if (status != ECA_NORMAL) alLogConnection(ca_name(chid),
                        "alCaAddEvent:ca_add_masked_array_event failed.");
}

/*********************************************************************
 add forcePV value event handler
 *********************************************************************/
void alCaAddForcePVEvent(chid chid, void *link, evid * pevid, int type)
{
    int status;
    void (*pFunc) (struct event_handler_args);

    if (!chid) return;

    if (type == CHANNEL) pFunc = alCaChannelForceEvent;
    else if (type == GROUP) pFunc = alCaGroupForceEvent;
    else
        alLogConnection(ca_name(chid),
                        "alCaAddForcePVEvent:Invalid type.");

    status = ca_add_masked_array_event(DBR_SHORT, 1,
                                       chid,
                                       pFunc,
                                       link,
                                       (float) 0, (float) 0, (float) 0,
                                       pevid,
                                       DBE_VALUE);
    if (status != ECA_NORMAL) alLogConnection(ca_name(chid),
                     "alCaAddForcePVEvent:ca_add_masked_array_event failed.");
}

/*********************************************************************
 send global alarm acknowledgement
 *********************************************************************/
void alCaPutGblAck(chid chid, short *psevr)
{
    int status;

    if (ca_field_type(chid) == TYPENOTCONN) return;

    status = ca_put(DBR_PUT_ACKS, chid, psevr);
    if (status != ECA_NORMAL)
        alLogConnection(ca_puser(chid), "alCaPutGblAck:ca_put failed.");
}


/*********************************************************************
 alCaPutSevValue - write severity value to sevrPV
 *********************************************************************/
void alCaPutSevrValue(chid chid, short *psevr)
{
    int status;

#if DEBUG_CALLBACKS
    {
        printf("alCaPutSevrValue: name=%s value=%d\n", ca_name(chid), *psevr);
    }
#endif

    if (!chid || ca_field_type(chid) == TYPENOTCONN) return;

    status = ca_put(DBR_SHORT, chid, psevr);
    if (status != ECA_NORMAL)
        alLogConnection(ca_name(chid), "alCaPutSevrValue:ca_put failed.");
}


/*********************************************************************
 channel alarm Event
 *********************************************************************/
static void alCaNewAlarmEvent(struct event_handler_args args)
{
    int stat, sevr;
    int ackt, acks;
    char value[MAX_STRING_SIZE];

    stat = ((struct dbr_stsack_string *) args.dbr)->status;
    sevr = ((struct dbr_stsack_string *) args.dbr)->severity;
    ackt = ((struct dbr_stsack_string *) args.dbr)->ackt;
    acks = ((struct dbr_stsack_string *) args.dbr)->acks;
    strcpy(value, ((struct dbr_stsack_string *) args.dbr)->value);

    switch (args.status) {
    case ECA_NORMAL:
        alNewEvent(stat, sevr, acks, value, args.usr);
        break;
    case ECA_NORDACCESS:
        alNewAlarm(READ_ACCESS_ALARM, INVALID_ALARM, value, args.usr);
        break;
    default:
        alNewAlarm(COMM_ALARM, INVALID_ALARM, value, args.usr);
    }
}

/*********************************************************************
 channel access_rights event handler
 *********************************************************************/
static void alCaChannelAccessRightsEvent(struct access_rights_handler_args args)
{
    if (ca_field_type(args.chid) == TYPENOTCONN) return;
    if (!ca_read_access(args.chid)) {
        alNewAlarm(READ_ACCESS_ALARM, INVALID_ALARM, "0", ca_puser(args.chid));
    }
    if (!ca_write_access(args.chid)) {
        alLogConnection(ca_name(args.chid), "No write access (Channel PVName)");
    }
}


/*********************************************************************
 forcePV access_rights event handler
 *********************************************************************/
static void alCaForcePVAccessRightsEvent(struct access_rights_handler_args args)
{
    if (ca_field_type(args.chid) == TYPENOTCONN) return;
    sprintf(buff, "%s--(%s)", ca_puser(args.chid), ca_name(args.chid));
    if (!ca_read_access(args.chid)) {
        alLogConnection(buff, "No read access (Force PVName)");
    }
}


/*********************************************************************
 sevrPV access_rights event handler
 *********************************************************************/
static void alCaSevrPVAccessRightsEvent(struct access_rights_handler_args args)
{
    if (ca_field_type(args.chid) == TYPENOTCONN) return;
    sprintf(buff, "%s--(%s)", ca_puser(args.chid), ca_name(args.chid));
    if (!ca_write_access(args.chid)) {
        alLogConnection(buff, "No write access (Sevr PVName)");
    }
}


/*********************************************************************
 channel connection event handler
 *********************************************************************/
static void alCaChannelConnectionEvent(struct connection_handler_args args)
{
    if (args.op == CA_OP_CONN_UP) {
        toBeConnectedCount--;
    } else if (args.op == CA_OP_CONN_DOWN) {
        alNewAlarm(COMM_ALARM, INVALID_ALARM, "0", ca_puser(args.chid));
    } else {
        alLogConnection(ca_name(args.chid), "Unknown Connnection Event (Channel PVName)");
    }
}


/*********************************************************************
 forcePV connection event handler
 *********************************************************************/
static void alCaForcePVConnectionEvent(struct connection_handler_args args)
{
    sprintf(buff, "%s--(%s)", ca_puser(args.chid), ca_name(args.chid));
    if (args.op == CA_OP_CONN_UP) {
        toBeConnectedCount--;
    } else if (args.op == CA_OP_CONN_DOWN) {
        alLogConnection(buff, "Not Connected (Force PVName)");
    } else {
        alLogConnection(buff, "Unknown Connnection Event (Force PVName)");
    }
}


/*********************************************************************
 sevrPV connection event handler
 *********************************************************************/
static void alCaSevrPVConnectionEvent(struct connection_handler_args args)
{
    sprintf(buff, "%s--(%s)", ca_puser(args.chid), ca_name(args.chid));
    if (args.op == CA_OP_CONN_UP) {
        toBeConnectedCount--;
    } else if (args.op == CA_OP_CONN_DOWN) {
        alLogConnection(buff, "Not Connected (Sevr PVName)");
    } else {
        alLogConnection(buff, "Unknown Connnection Event (Sevr PVName)");
    }
}


/*********************************************************************
 group forcePV value event handler
 *********************************************************************/
static void alCaGroupForceEvent(struct event_handler_args args)
{
    if (args.status == ECA_NORMAL) {
        alGroupForceEvent(args.usr, *(short *) args.dbr);
    } else {
        alLogConnection(ca_name(args.chid), "alCaGroupForceEvent invalid args.status.");
    }
}

/*********************************************************************
 channel forcePV value event handler
 *********************************************************************/
static void alCaChannelForceEvent(struct event_handler_args args)
{
    if (args.status == ECA_NORMAL) {
        alChannelForceEvent(args.usr, *(short *) args.dbr);
    } else {
        alLogConnection(ca_name(args.chid), "alCaChannelForceEvent invalid args.status.");
    }
}

/*********************************************************************
 alCaUpdate -- Timer proc to update screen
 *********************************************************************/
static void alCaUpdate(XtPointer cd, XtIntervalId * id)
{

#if DEBUG_CALLBACKS
    {
        static int n = 0;

        printf("alCaUpdate: n=%d\n", n++);
    }
#endif

    ca_poll();
    alUpdateAreas();
    caTimeoutId = XtAppAddTimeOut(appContext, caDelay, alCaUpdate, NULL);
}

/*********************************************************************
 ca exception messages
 *********************************************************************/
static void alCaException(struct exception_handler_args args)
{
#define MAX_EXCEPTIONS 25
    static int nexceptions = 0;
    static int ended = 0;

    if (ended) return;
    if (nexceptions++ > MAX_EXCEPTIONS) {
        ended = 1;
        errMsg("alCaException: Channel Access Exception:\n"
               "Too many exceptions [%d]\n"
               "No more will be handled\n"
               "Please fix the problem and restart ALH",
               MAX_EXCEPTIONS);
        ca_add_exception_event(NULL, NULL);
        return;
    }
    errMsg("alCaException: Channel Access Exception:\n"
           "  Channel Name: %s\n"
           "  Native Type: %s\n"
           "  Native Count: %hu\n"
           "  Access: %s%s\n"
           "  IOC: %s\n"
           "  Message: %s\n"
           "  Context: %s\n"
           "  Requested Type: %s\n"
           "  Requested Count: %ld\n"
           "  Source File: %s\n"
           "  Line number: %u",
           args.chid ? ca_name(args.chid) : "Unavailable",
       args.chid ? dbf_type_to_text(ca_field_type(args.chid)) : "Unavailable",
           args.chid ? ca_element_count(args.chid) : 0,
           args.chid ? (ca_read_access(args.chid) ? "R" : "") : "Unavailable",
           args.chid ? (ca_write_access(args.chid) ? "W" : "") : "",
           args.chid ? ca_host_name(args.chid) : "Unavailable",
           ca_message(args.stat) ? ca_message(args.stat) : "Unavailable",
           args.ctx ? args.ctx : "Unavailable",
           dbf_type_to_text(args.type),
           args.count,
           args.pFile ? args.pFile : "Unavailable",
           args.pFile ? args.lineNo : 0);
}
