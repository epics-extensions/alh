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
/* printer.h */

/************************DESCRIPTION***********************************
  In this file we describe esq. sequense for coloring alarm
  for different type of printer. 
  See http://hpcc920.external.hp.com/cposupport/eschome.html 
  for the other HP printer 
**********************************************************************/

#ifndef INCprinterh
#define INCprinterh 1

/* ------------- Mono Bold printer ---------------------------------- */

char colorStartNoMonoBold[] ={
	0};
int colorStartNoMonoBoldLen=0;

char colorStartMinorMonoBold[] ={
	0};
int colorStartMinorMonoBoldLen=0;

char colorStartMajorMonoBold[]={
	27,'[','1','m',0};  /*BOLD letters*/
int colorStartMajorMonoBoldLen=5;

char colorStartInvalidMonoBold[]={
	27,'[','1','m',0};/*BOLD letters*/
int colorStartInvalidMonoBoldLen=5;

char colorEndMonoBold[]={
	27,'[','0','m',0};
int colorEndMonoBoldLen=5;

/* ------------- Mono OKI printer ---------------------------------- */

char colorStartNoOkiBold[] ={
	0};
int colorStartNoOkiBoldLen=0;

char colorStartMinorOkiBold[] ={
	0};
int colorStartMinorOkiBoldLen=0;

char colorStartMajorOkiBold[]={
	27,72,0};  /*BOLD letters*/
int colorStartMajorOkiBoldLen=3;

char colorStartInvalidOkiBold[]={
	27,72,0};/*BOLD letters*/
int colorStartInvalidOkiBoldLen=3;

char colorEndOkiBold[]={
	27,73,0};
int colorEndOkiBoldLen=3;

/* ------------- HP DeskJet 1200C ---------------------------------- */

char colorStartNoHpColor[] ={
	0};
int colorStartNoHpColorLen=0;

char colorStartMinorHpColor[] ={
	27,'&','v','3','S',0};
int colorStartMinorHpColorLen=6;              /*YELLOW letters*/

char colorStartMajorHpColor[]={
	27,'&','v','1','S',0};
int colorStartMajorHpColorLen=6;                 /*RED letters*/

char colorStartInvalidHpColor[]={
	27,'&','v','5','S',0};
int colorStartInvalidHpColorLen=6;             /* Magenta  letters*/

char colorEndHpColor[]={
	27,'E',0};
int colorEndHpColorLen=3;

/* ------------- Mono printer ---------------------------------- */

char colorStartNoMono[] ={
	0};
int colorStartNoMonoLen=0;

char colorStartMinorMono[] ={
	0};
int colorStartMinorMonoLen=0;

char colorStartMajorMono[]={
	0};
int colorStartMajorMonoLen=0;

char colorStartInvalidMono[]={
	0};
int colorStartInvalidMonoLen=0;

char colorEndMono[]={
	0};
int colorEndMonoLen=1;


#endif /* INCprinterh */

