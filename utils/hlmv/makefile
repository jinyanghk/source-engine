# simple Makefile for Half-Life Model Viewer

CC = g++
CFLAGS = -O2
IFLAGS =
LFLAGS = -s
LIBS = -lmxtk-qt -lqt -lqgl -lGL -lGLU -lXaw

HLMV = ../bin/hlmv
OBJS = \
	controlpanel.o \
	fileAssociation.o \
	glwindow.o \
	viewersettings.o \
	mathlib.o \
	mdlviewer.o \
	pakviewer.o \
	studio_render.o \
	studio_utils.o \

all: $(HLMV)

$(HLMV): $(OBJS)
	$(CC) $(LFLAGS) -o $(HLMV) $(OBJS) $(LIBS) 

controlpanel.o: controlpanel.cpp controlpanel.h viewersettings.h studiomodel.h glwindow.h
	$(CC) -c $(CFLAGS) $(IFLAGS) controlpanel.cpp

fileassociation.o: filesssociation.cpp fileassociation.h
	$(CC) -c $(CFLAGS) $(IFLAGS) fileassociation.cpp

glwindow.o: glwindow.cpp glwindow.h studiomodel.h viewersettings.h
	$(CC) -c $(CFLAGS) $(IFLAGS) glWindow.cpp

viewersettings.o: viewersettings.cpp viewersettings.h
	$(CC) -c $(CFLAGS) $(IFLAGS) viewersettings.cpp

mathlib.o: mathlib.c mathlib.h
	$(CC) -c $(CFLAGS) $(IFLAGS) mathlib.c

mdlviewer.o: mdlviewer.cpp mdlviewer.h glwindow.h controlpanel.h studiomodel.h pakviewer.h fileassociation.h
	$(CC) -c $(CFLAGS) $(IFLAGS) mdlviewer.cpp

pakviewer.o: pakviewer.cpp pakviewer.h mdlviewer.h glwindow.h controlpanel.h studiomodel.h fileassociation.h
	$(CC) -c $(CFLAGS) $(IFLAGS) pakviewer.cpp


studio_render.o: studio_render.cpp studiomodel.h viewersettings.h
	$(CC) -c $(CFLAGS) $(IFLAGS) studio_render.cpp

studio_utils.o: studio_utils.cpp studiomodel.h
	$(CC) -c $(CFLAGS) $(IFLAGS) studio_utils.cpp



# clean

clean:
	rm *.o
