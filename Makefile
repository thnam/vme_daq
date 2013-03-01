#####################################################################
#
#  Name:         Makefile
#  Created by:   Stefan Ritt
#  Modified by:  Jimmy Ngai
#
#  Date:         July 20, 2012
#
#  Contents:     Makefile for MIDAS example frontend and analyzer
#
#  $Id: Makefile 3203 2006-07-31 21:39:02Z ritt $
#
#####################################################################
#
#--------------------------------------------------------------------
# The MIDASSYS should be defined prior the use of this Makefile
ifndef MIDASSYS
missmidas::
	@echo "...";
	@echo "Missing definition of environment variable 'MIDASSYS' !";
	@echo "...";
endif

# get OS type from shell
OSTYPE = $(shell uname)

#--------------------------------------------------------------------
# The following lines contain specific switches for different UNIX
# systems. Find the one which matches your OS and outcomment the 
# lines below.

#-----------------------------------------
# This is for Linux
ifeq ($(OSTYPE),Linux)
OSTYPE = linux
endif

ifeq ($(OSTYPE),linux)

OS_DIR = linux
OSFLAGS = -DOS_LINUX -DLINUX -DUNIX -Dextname
CFLAGS = -g -O2 -Wall -Wno-write-strings
# add to compile in 32-bit mode
# OSFLAGS += -m32
LIBS = -lm -lz -lutil -lnsl -lpthread -lrt
endif

#-----------------------
# MacOSX/Darwin is just a funny Linux
#
ifeq ($(OSTYPE),Darwin)
OSTYPE = darwin
endif

ifeq ($(OSTYPE),darwin)
OS_DIR = darwin
FF = cc
OSFLAGS = -DOS_LINUX -DLINUX -DOS_DARWIN -DUNIX -DHAVE_STRLCPY -DAbsoftUNIXFortran -fPIC -Wno-unused-function
LIBS = -lpthread
SPECIFIC_OS_PRG = $(BIN_DIR)/mlxspeaker
NEED_STRLCPY=
NEED_RANLIB=1
NEED_SHLIB=
NEED_RPATH=

endif

#-----------------------------------------
# ROOT flags and libs
#
ifdef ROOTSYS
ROOTCFLAGS := $(shell  $(ROOTSYS)/bin/root-config --cflags)
ROOTCFLAGS += -DHAVE_ROOT -DUSE_ROOT
ROOTLIBS   := $(shell  $(ROOTSYS)/bin/root-config --libs) -Wl,-rpath,$(ROOTSYS)/lib
ROOTLIBS   += -lThread
else
missroot:
	@echo "...";
	@echo "Missing definition of environment variable 'ROOTSYS' !";
	@echo "...";
endif

#-------------------------------------------------------------------
# The following lines define directories. Adjust if necessary
#                 
INC_DIR    = $(MIDASSYS)/include
LIB_DIR    = $(MIDASSYS)/$(OS_DIR)/lib
SRC_DIR    = $(MIDASSYS)/src
DRV_DIR    = ./drivers
MOD_DIR    = ./modules

#-------------------------------------------------------------------
# List of analyzer modules
#
MODULES    = adccalib.o adcsum.o scaler.o

#-------------------------------------------------------------------
# Drivers needed by the frontend program
#
FE_DRIVERS  = v1718.o v792n.o
FE_LIBS     = -lCAENVME

#-------------------------------------------------------------------
# Frontend code name defaulted to frontend in this example.
# comment out the line and run your own frontend as follow:
# gmake UFE=my_frontend
#
FE  = frontend

####################################################################
# Lines below here should not be edited
####################################################################

# MIDAS library
LIB = $(LIB_DIR)/libmidas.a

# compiler
CC = gcc
CXX = g++
CFLAGS += -g -I. -I$(INC_DIR) -I$(DRV_DIR)
CFLAGS += -I$(DRV_DIR)/vme
CFLAGS += -I$(DRV_DIR)/vme/CAENVMElib/include
LDFLAGS +=

all: $(FE) #analyzer

noenv: all

$(FE): $(LIB) $(LIB_DIR)/mfe.o $(FE_DRIVERS) $(FE).c 
	$(CC) $(CFLAGS) $(OSFLAGS) -o $(FE) $(FE).c \
	$(FE_DRIVERS) $(LIB_DIR)/mfe.o $(LIB) \
	$(LDFEFLAGS) $(LIBS) $(FE_LIBS)

#$(FE): $(LIB) $(LIB_DIR)/mfe.o $(FE_DRIVERS) $(FE).c $(SRC_DIR)/cnaf_callback.c
	#$(CC) $(CFLAGS) $(OSFLAGS) -o $(FE) $(FE).c \
	#$(SRC_DIR)/cnaf_callback.c $(FE_DRIVERS) $(LIB_DIR)/mfe.o $(LIB) \
	#$(LDFEFLAGS) $(LIBS) $(FE_LIBS)

%.o: $(DRV_DIR)/vme/%.c
	$(CC) $(CFLAGS) $(OSFLAGS) -o $@ -c $<

analyzer: $(LIB) $(LIB_DIR)/rmana.o analyzer.o $(MODULES)
	$(CXX) $(CFLAGS) -o $@ $(LIB_DIR)/rmana.o analyzer.o $(MODULES) \
	$(LIB) $(LDFLAGS) $(ROOTLIBS) $(LIBS) $(ANA_LIBS)

%.o: %.c experim.h
	$(CXX) $(USERFLAGS) $(ROOTCFLAGS) $(CFLAGS) $(OSFLAGS) -o $@ -c $<

%.o: $(MOD_DIR)/%.c experim.h
	$(CXX) $(USERFLAGS) $(ROOTCFLAGS) $(CFLAGS) $(OSFLAGS) -o $@ -c $<

clean::
	rm -rf *.o *~ \#*

#end file
