ADD_ON = ../..

include $(ADD_ON)/src/config/CONFIG

EPICS_LIBS = $(EPICS_BASE_BIN/libca.a $(EPICS_BASE_BIN/libCom.a $(EPICS_BASE_BIN/libUnix.a

USR_CFLAGS = -DACCESS_SECURITY $(MOTIF_CFLAGS) $(X11_CFLAGS)
USR_LDFLAGS = $(MOTIF_LDFLAGS) $(X11_LDFLAGS) -lm

CC = acc
GCC = acc

   SRCS =  acknowledge.c  \
		alCA.c \
		alConfig.c \
		alDebug.c \
		alFilter.c \
		alInitialize.c \
		alLib.c \
		alLog.c \
		alView.c  \
		alh.c \
		awAct.c \
		awAlh.c \
		awEdit.c \
		awView.c \
		axArea.c \
		axRunW.c \
		axSubW.c \
		clipboardOps.c \
		current.c \
		dialog.c \
		file.c \
		force.c \
		guidance.c \
		help.c \
		line.c \
		mask.c \
		process.c \
		property.c \
		scroll.c \
		showmask.c \
		sllLib.c \
		testalarm.c

OBJS =  acknowledge.o  \
		alCA.o \
		alConfig.o \
		alDebug.o \
		alFilter.o \
		alInitialize.o \
		alLib.o \
		alLog.o \
		alView.o  \
		alh.o \
		awAct.o \
		awAlh.o \
		awEdit.o \
		awView.o \
		axArea.o \
		axRunW.o \
		axSubW.o \
		clipboardOps.o \
		current.o \
		dialog.o \
		file.o \
		force.o \
		guidance.o \
		help.o \
		line.o \
		mask.o \
		process.o \
		property.o \
		scroll.o \
		showmask.o \
		sllLib.o \
		testalarm.o   

PROD = alh
PROD_VERSION = $Version$

include $(ADD_ON)/src/config/RULES

alh: $(OBJS) $(DEP_LIBS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

