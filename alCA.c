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
/* alCA.c */

/************************DESCRIPTION***********************************
  This file contains routines for channel access.
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "ax.h"

#define DEBUG_CALLBACKS 0

/* global variables */
extern int _global_flag;
extern int _passive_flag;
int toBeConnectedCount = 0;
extern XtAppContext appContext;
static XtIntervalId caTimeoutId = (XtIntervalId) 0;
static unsigned long caDelay = 100;        /* ms */

/* forward declarations */
static void alCaNewAlarmEvent(struct event_handler_args args);
static void alCaForcePVValueEvent(struct event_handler_args args);
static void alCaChannelConnectionEvent(struct connection_handler_args args);
static void alCaForcePVConnectionEvent(struct connection_handler_args args);
static void alCaSevrPVConnectionEvent(struct connection_handler_args args);
static void alCaAckPVConnectionEvent(struct connection_handler_args args);
static void alCaAckPVAccessRightsEvent(struct access_rights_handler_args args);
static void alCaChannelAccessRightsEvent(struct access_rights_handler_args args);
static void alCaForcePVAccessRightsEvent(struct access_rights_handler_args args);
static void alCaSevrPVAccessRightsEvent(struct access_rights_handler_args args);
static void alCaUpdate(XtPointer cd, XtIntervalId * id);
static void alCaException(struct exception_handler_args args);
static void alCaHeartbeatPVAccessRightsEvent(struct access_rights_handler_args args);
static void alCaHeartbeatPVConnectionEvent(struct connection_handler_args args);


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
	int status;

#if DEBUG_CALLBACKS
	{
		printf("alCaInit: caTimeoutId=%d\n", caTimeoutId);
	}
#endif

	/* Initialize channel access */
	status = ca_task_initialize();
	if (status != ECA_NORMAL){
		errMsg("ca_task_initialize failed: Return status: %s\n",ca_message(status));
	}

	/* Register exception handler */
	status = ca_add_exception_event(alCaException, NULL);
	if (status != ECA_NORMAL){
		errMsg("ca_add_exception_event failed: Return status: %s\n",ca_message(status));
	}

	/* Register file descriptor callback */
	status = ca_add_fd_registration(registerCA, NULL);
	if (status != ECA_NORMAL){
		errMsg("ca_add_fd_registration failed: Return status: %s\n",ca_message(status));
	}

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
	int status;

	/* cancel timeout */
	if (caTimeoutId) {
		XtRemoveTimeOut(caTimeoutId);
		caTimeoutId = (XtIntervalId) 0;
	}
	/* and close channel access */
	status = ca_task_exit();
	if (status != ECA_NORMAL){
		errMsg("ca_task_exit failed: Return status: %s\n",ca_message(status));
	}
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
	if (status != ECA_NORMAL) {
		errMsg("ca_search_and_connect failed for PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}

	status = ca_replace_access_rights_event(*pchid, alCaChannelAccessRightsEvent);
	if (status != ECA_NORMAL) {
		errMsg("ca_replace_access_rights_event failed for PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}
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
	if (status != ECA_NORMAL){
		errMsg("ca_search_and_connect failed for Force PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}
	status = ca_replace_access_rights_event(*pchid, alCaForcePVAccessRightsEvent);
	if (status != ECA_NORMAL){
		errMsg("ca_replace_access_rights_event failed for Force PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}
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
	if (status != ECA_NORMAL){
		errMsg("ca_search_and_connect failed for Severity PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}
	status = ca_replace_access_rights_event(*pchid, alCaSevrPVAccessRightsEvent);
	if (status != ECA_NORMAL){
		errMsg("ca_replace_access_rights_event failed for Severity PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}
}

/*********************************************************************
  create chid, start search, add connection event handler, add puser,
  add access_rights event handler
 *********************************************************************/
void alCaConnectHeartbeatPV(char *name, chid * pchid, void *puser)
{
	int status;

	if (strlen(name) <= (size_t) 1) return;

	toBeConnectedCount++;
	status = ca_search_and_connect(name,pchid,alCaHeartbeatPVConnectionEvent,puser);
	if (status != ECA_NORMAL){
		errMsg("ca_search_and_connect failed for Heartbeat PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}
	status = ca_replace_access_rights_event(*pchid, alCaHeartbeatPVAccessRightsEvent);
	if (status != ECA_NORMAL){
		errMsg("ca_replace_access_rights_event failed for Heartbeat PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}
}
/*
This is similar to previous function alCaConnectSevrPV
 */
void alCaConnectAckPV(char *name, chid * pchid, void *puser) 
{
	int status;
	if (strlen(name) <= (size_t) 1) return;

	toBeConnectedCount++;
	status = ca_search_and_connect(name, pchid, alCaAckPVConnectionEvent, puser);
	if (status != ECA_NORMAL){
		errMsg("ca_search_and_connect failed for Severity PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}
	status = ca_replace_access_rights_event(*pchid, alCaSevrPVAccessRightsEvent);
	if (status != ECA_NORMAL){
		errMsg("ca_replace_access_rights_event failed for Severity PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}
}

/*********************************************************************
 clear a channel chid
 *********************************************************************/
void alCaClearChannel(chid * pchid)
{
	int status;

	if (!*pchid) return;

	status = ca_clear_channel(*pchid);
	if (status != ECA_NORMAL){
		errMsg("ca_clear_channel failed for PV %s "
			"Return status: %s\n",ca_name(*pchid),ca_message(status));
	}
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
	if (status != ECA_NORMAL){
		errMsg("ca_clear_event failed for PV "
			"Return status: %s\n",ca_message(status));
	}
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
	if (status != ECA_NORMAL){
		errMsg("ca_add_masked_array_event failed for PV %s "
			"Return status: %s\n",ca_name(chid),ca_message(status));
	}
}

/*********************************************************************
 add forcePV value event handler
 *********************************************************************/
void alCaAddForcePVEvent(chid chid, void *userdata, evid * pevid)
{
	int status;

	if (!chid) return;

	status = ca_add_masked_array_event(DBR_DOUBLE, 1,
	    chid,
	    alCaForcePVValueEvent,
	    userdata,
	    (float) 0, (float) 0, (float) 0,
	    pevid,
	    DBE_VALUE);
	if (status != ECA_NORMAL){
		errMsg("alCaAddForcePVEvent: ca_add_masked_array_event failed for Force PV %s "
			"Return status: %s\n",ca_name(chid),ca_message(status));
	}
}

/*********************************************************************
 send global alarm acknowledgement
 *********************************************************************/
void alCaPutGblAck(chid chid, short *psevr)
{
	int status;

	if (!chid || ca_field_type(chid) == TYPENOTCONN) return;

	if (!_global_flag || _passive_flag)  return;

	status = ca_put(DBR_PUT_ACKS, chid, psevr);
	if (status != ECA_NORMAL) {
		errMsg("alCaPutGblAck: ca_put alarm acknowledgement failed for PV %s "
			"Return status: %s\n",ca_name(chid),ca_message(status));
	}
}


/*********************************************************************
 global modify acknowledge transients fields
 *********************************************************************/
void alCaPutGblAckT(chid chid, short *pstate)
{
	int status;

    if ( !_global_flag ||_passive_flag) return;

	if (!chid || ca_field_type(chid) == TYPENOTCONN) return;

	status = ca_put(DBR_PUT_ACKT, chid, pstate);
	if (status != ECA_NORMAL) {
		errMsg("alCaPutGblAckT: ca_put acknowledge transients failed for PV %s "
			"Return status: %s\n",ca_name(chid),ca_message(status));
	}
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

	if (!_global_flag || _passive_flag)  return;

	if (!chid || ca_field_type(chid) == TYPENOTCONN) return;

	status = ca_put(DBR_SHORT, chid, psevr);
	if (status != ECA_NORMAL) {
		errMsg("alCaPutSevrValue: ca_put failed for Severity PV %s "
			"Return status: %s\n",ca_name(chid),ca_message(status));
	}
}
/*
  This function is the same as prev. but for ackPV record. 
 */
void alCaPutAckValue(chid chid, short *psevr)
{
	int status;
#if DEBUG_CALLBACKS
	{
		printf("alCaPutAckValue: name=%s value=%d\n", ca_name(chid), *psevr);
	}
#endif
	if (!chid || ca_field_type(chid) == TYPENOTCONN) return;

/*	status = ca_put(ca_field_type(chid), chid, psevr);   */ 
	status = ca_put(DBR_ENUM, chid, psevr);

	if (status != ECA_NORMAL) {
		errMsg("alCaPutAckValue: ca_put failed for Severity PV %s "
			"Return status: %s\n",ca_name(chid),ca_message(status));
	}
}


/*********************************************************************
 alCaPutHeartbeatValue - write to heartbeatPV
 *********************************************************************/
void alCaPutHeartbeatValue(chid chid, short *pvalue)
{
	int status;

	if (!_global_flag || _passive_flag)  return;

	if (!chid || ca_field_type(chid) == TYPENOTCONN) return;

	status = ca_put(DBR_SHORT, chid, pvalue);
	if (status != ECA_NORMAL) {
		errMsg("alCaPutHeartbeatValue: ca_put failed for Heartbeat PV %s "
			"Return status: %s\n",ca_name(chid),ca_message(status));
	}
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
	acks = ((struct dbr_stsack_string *) args.dbr)->acks;
	ackt = ((struct dbr_stsack_string *) args.dbr)->ackt;

	if (stat >= ALARM_NSTATUS) stat = ALARM_NSTATUS-1;
	if (stat < 0) stat = 0;

	if (sevr >= ALARM_NSEV) sevr = ALARM_NSEV-1;
	if (sevr < 0) sevr = 0;

	if (acks >= ALARM_NSEV) acks = ALARM_NSEV-1;
	if (acks < 0) acks = 0;

	if (ackt != TRUE && ackt != FALSE) ackt = TRUE;

	strcpy(value, ((struct dbr_stsack_string *) args.dbr)->value);

	switch (args.status) {
	case ECA_NORMAL:
		alSaveAlarmEvent(stat, sevr, acks, ackt, value, args.usr);
		alNewEvent(stat, sevr, acks, ackt, value, args.usr);
		break;
	default:
		errMsg("alCaNewAlarmEvent failed: Return status: %s "
		" for PV %s\n",ca_message(args.status),ca_name(args.chid));
		break;
	}
}

/*********************************************************************
 channel access_rights event handler
 *********************************************************************/
static void alCaChannelAccessRightsEvent(struct access_rights_handler_args args)
{
	if (ca_field_type(args.chid) == TYPENOTCONN) return;
	if (!ca_read_access(args.chid)) {
		alNewEvent(NO_READ_ACCESS, ERROR_STATE, 0, -1, "0", ca_puser(args.chid));
	}
	if (!ca_write_access(args.chid) && _global_flag && !_passive_flag) {
		alNewEvent(NO_WRITE_ACCESS, ERROR_STATE, 0, -1, "0", ca_puser(args.chid));
	}
}


/*********************************************************************
 forcePV access_rights event handler
 *********************************************************************/
static void alCaForcePVAccessRightsEvent(struct access_rights_handler_args args)
{
	if (ca_field_type(args.chid) == TYPENOTCONN) return;
	if (!ca_read_access(args.chid)) {
		errMsg("No read access for Force PV %s\n",ca_name(args.chid));
	}
}


/*********************************************************************
 sevrPV access_rights event handler
 *********************************************************************/
static void alCaSevrPVAccessRightsEvent(struct access_rights_handler_args args)
{
	if (ca_field_type(args.chid) == TYPENOTCONN) return;
	if (!ca_write_access(args.chid) && _global_flag && !_passive_flag) {
		errMsg("No write access for Severity PV %s\n",ca_name(args.chid));
	}
}
/*
   this is the same as previous function but for .DESC-field.
 */
static void alCaAckPVAccessRightsEvent(struct access_rights_handler_args args) 
{
	if (ca_field_type(args.chid) == TYPENOTCONN) return;
	if (!ca_write_access(args.chid) && _global_flag && !_passive_flag) {
		errMsg("No write access for Ack PV %s\n",ca_name(args.chid));
	}
}


/*********************************************************************
 heartbeatPV access_rights event handler
 *********************************************************************/
static void alCaHeartbeatPVAccessRightsEvent(struct access_rights_handler_args args)
{
	if (ca_field_type(args.chid) == TYPENOTCONN) return;
	if (!ca_write_access(args.chid) && _global_flag && !_passive_flag) {
		errMsg("No write access for Heartbeat PV %s\n",ca_name(args.chid));
	}
}


/*********************************************************************
 channel connection event handler
 *********************************************************************/
static void alCaChannelConnectionEvent(struct connection_handler_args args)
{
	if (args.op == CA_OP_CONN_UP) {
		toBeConnectedCount--;
		alConnectEvent(ca_puser(args.chid));
	} else if (args.op == CA_OP_CONN_DOWN) {
		alNewEvent(NOT_CONNECTED, ERROR_STATE, 0, -1, "0", ca_puser(args.chid));
	} else {
		errMsg("Unknown Connnection Event for PV %s\n",ca_name(args.chid));
	}
}


/*********************************************************************
 forcePV connection event handler
 *********************************************************************/
static void alCaForcePVConnectionEvent(struct connection_handler_args args)
{
	if (args.op == CA_OP_CONN_UP) {
		toBeConnectedCount--;
	} else if (args.op == CA_OP_CONN_DOWN) {
		errMsg("Not Connected: Force PV %s for %s\n",
			ca_name(args.chid),(char *)ca_puser(args.chid));
	} else {
		errMsg("Unknown Connection Event Force PV %s for %s\n",
			ca_name(args.chid),(char *)ca_puser(args.chid));
	}
}


/*********************************************************************
 sevrPV connection event handler
 *********************************************************************/
static void alCaSevrPVConnectionEvent(struct connection_handler_args args)
{
	if (args.op == CA_OP_CONN_UP) {
		toBeConnectedCount--;
	} else if (args.op == CA_OP_CONN_DOWN) {
		errMsg("Not Connected: Severity PV %s for %s\n",
			ca_name(args.chid),(char *)ca_puser(args.chid));
	} else {
		errMsg("Unknown Connection Event Severity PV %s for %s\n",
			ca_name(args.chid),(char *)ca_puser(args.chid));
	}
}

/*********************************************************************
 ackPV connection event handler
 *********************************************************************/
static void alCaAckPVConnectionEvent(struct connection_handler_args args) 
{
	if (args.op == CA_OP_CONN_UP) {
		toBeConnectedCount--;
	} else if (args.op == CA_OP_CONN_DOWN) {
		errMsg("Not Connected: Severity PV %s for %s\n",
			ca_name(args.chid),(char *)ca_puser(args.chid));
	} else {
		errMsg("Unknown Connection Event Severity PV %s for %s\n",
			ca_name(args.chid),(char *)ca_puser(args.chid));
	}
}


/*********************************************************************
 heartbeatPV connection event handler
 *********************************************************************/
static void alCaHeartbeatPVConnectionEvent(struct connection_handler_args args)
{
	if (args.op == CA_OP_CONN_UP) {
		toBeConnectedCount--;
		alHeartbeatStart(ca_puser(args.chid));
	} else if (args.op == CA_OP_CONN_DOWN) {
		errMsg("Not Connected: Heartbeat PV %s\n",ca_name(args.chid));
		alHeartbeatStop(ca_puser(args.chid));
	} else {
		errMsg("Unknown Connection Event Heartbeat PV %s\n",ca_name(args.chid));
	}
}


/*********************************************************************
 group forcePV value event handler
 *********************************************************************/
static void alCaForcePVValueEvent(struct event_handler_args args)
{
	if (args.status == ECA_NORMAL) {
		alForcePVValueEvent(args.usr, *(double *) args.dbr);
	} else {
		errMsg("alCaForcePVValueEvent failed: Return status: %s "
		" for PV %s\n",ca_message(args.status),ca_name(args.chid));
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
		errMsg("Channel Access Exception: Too many exceptions [%d]\n",
		    MAX_EXCEPTIONS);
		ca_add_exception_event(NULL, NULL);
		return;
	}
	errMsg("Channel Access Exception: %s  Context: %s\n",
	    ca_message(args.stat) ? ca_message(args.stat) : "Unavailable",
	    args.ctx ? args.ctx : "Unavailable");
}
static void description_callback (struct event_handler_args args);

getDescriptionRecord(char *name,char *description,chid descriptionFieldCaId)
{
  int status;
  static char desc_name[64];
  memset(desc_name,0,64);
  strcpy(desc_name,name);
  strcat(desc_name,".DESC");

  status = ca_search(desc_name, &descriptionFieldCaId);
  if (status != ECA_NORMAL) {
    SEVCHK(status,"     CAN'T search description field\n");
    fprintf(stderr,"%s: CAN'T search description field\n",desc_name);
    *description=0;
    return;      
  }
  
  status = ca_pend_io(0.5);	
  if (status != ECA_NORMAL) 
    {
      SEVCHK(status,"     CAN'T pend description field\n");
      fprintf(stderr,"%s: CAN'T pend description field\n",desc_name);
      *description=0;
      return;          
    }	
  
  status = ca_array_get (DBR_STRING,1,descriptionFieldCaId,description);
  if (status != ECA_NORMAL) 
  {
    SEVCHK(status,"     CAN'T get description field\n");
    fprintf(stderr,"%s: CAN'T get description field\n",desc_name);
    *description=0;
    return;           
  }
  
  status= ca_pend_io(0.5);
  if (status != ECA_NORMAL)  
    {
      SEVCHK(status,"     CAN'T pend description field again \n");
      fprintf(stderr,"%s: CAN'T pend description field again \n",name);
      *description=0;
      return;     
    }
  
  status=ca_add_event
    (DBR_STRING,descriptionFieldCaId,description_callback,description,NULL);
  status= ca_pend_io(0.5);

}

static void description_callback (struct event_handler_args args)
{
  char *string= (char *)args.dbr;
  char * descr = (char *)args.usr;
  if(strlen(string) > 127) return; /* May be garbage. Avoid overwriting*/
  strcpy(descr,string);
}
