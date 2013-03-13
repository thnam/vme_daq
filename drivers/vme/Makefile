EXE =   r
CC  =   gcc
COPTS   =   -fPIC -DLINUX -Wall  -lpthread
#COPTS  =   -g -fPIC -DLINUX -Wall 
FLAGS   =   -Wall -s
#FLAGS  =   -Wall
INC_DIR    = $(MIDASSYS)/include
LIB_DIR    = $(MIDASSYS)/$(OS_DIR)/lib
SRC_DIR    = $(MIDASSYS)/src
DEPLIBS =       -lCAENVME 

INCLUDEDIR =    -I. -I$(INC_DIR)
OBJS    =  v1718.o test.o v792n.o
#########################################################################
all :   $(EXE)

clean   :
	/bin/rm -f $(OBJS) $(EXE)

$(EXE)  :   $(OBJS)
	$(CC) $(FLAGS) -o $(EXE) $(OBJS) $(DEPLIBS)
$(OBJS) :   $(INCLUDES) Makefile
%.o :   %.c
	$(CC) $(COPTS) $(INCLUDEDIR) -c -o $@ $<
