ADD_ON = ../..

include $(ADD_ON)/src/admin/CONFIG

USR_CFLAGS = $(MOTIF_CFLAGS) $(X11_CFLAGS)
USR_LDFLAGS = $(MOTIF_LDFLAGS) $(X11_LDFLAGS) -lm

SRC = .
 
CC = acc
GCC = acc

   SRCS =  $(SRC)/acknowledge.c  \
		$(SRC)/alCA.c \
		$(SRC)/alConfig.c \
		$(SRC)/alDebug.c \
		$(SRC)/alFilter.c \
		$(SRC)/alInitialize.c \
		$(SRC)/alLib.c \
		$(SRC)/alLog.c \
		$(SRC)/alView.c  \
		$(SRC)/alh.c \
		$(SRC)/awAct.c \
		$(SRC)/awAlh.c \
		$(SRC)/awEdit.c \
		$(SRC)/awView.c \
		$(SRC)/axArea.c \
		$(SRC)/axRunW.c \
		$(SRC)/axSubW.c \
		$(SRC)/clipboardOps.c \
		$(SRC)/current.c \
		$(SRC)/dialog.c \
		$(SRC)/file.c \
		$(SRC)/force.c \
		$(SRC)/guidance.c \
		$(SRC)/help.c \
		$(SRC)/line.c \
		$(SRC)/mask.c \
		$(SRC)/process.c \
		$(SRC)/property.c \
		$(SRC)/scroll.c \
		$(SRC)/showmask.c \
		$(SRC)/sllLib.c \
		$(SRC)/testalarm.c

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

include $(ADD_ON)/src/admin/RULES

alh: $(OBJS) $(DEP_LIBS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

