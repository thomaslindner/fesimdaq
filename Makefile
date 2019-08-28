# Makefile
#
# $Id$
#

OSFLAGS  = -DOS_LINUX -Dextname
CFLAGS   = -g -O2 -fPIC -Wall -Wuninitialized -I. -I$(MIDASSYS)/include
CXXFLAGS = $(CFLAGS)

LIBS = -lm -lz -lutil   -lpthread -lssl -ldl  
LIB_DIR         = $(MIDASSYS)/lib

# MIDAS library
MIDASLIBS = $(MIDASSYS)/lib/libmidas.a

# fix these for MacOS
UNAME=$(shell uname)
ifeq ($(UNAME),Darwin)
MIDASLIBS = $(MIDASSYS)/lib/libmidas.a  $(MIDASSYS)/lib/libmfe.a
LIB_DIR         = $(MIDASSYS)/darwin/lib
endif

# ROOT library
ifdef ROOTSYS
CXXFLAGS += -DHAVE_ROOT -I$(ROOTSYS)/include -I$(ROOTSYS)/include/root
ROOTGLIBS = $(shell $(ROOTSYS)/bin/root-config --glibs) -lThread -Wl,-rpath,$(ROOTSYS)/lib
LIBS += $(ROOTGLIBS)
endif

all:: fesimdaq.exe 


fesimdaq.exe: %.exe:   %.o 
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS)  $(MIDASLIBS) $(LIBS)

fesimdaq_v2.exe: %.exe:   %.o 
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIB_DIR)/mfe.o $(MIDASLIBS) $(LIBS)

struct_history_example.exe: %.exe:   %.o 
	$(CXX) -o $@ $(CFLAGS) $(OSFLAGS) $^ $(MIDASLIBS) $(LIB_DIR)/mfe.o $(MIDASLIBS) $(LIBS)




%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -c $<

%.o: %.c
	$(CXX) $(CXXFLAGS) $(OSFLAGS) -c $<

clean::
	-rm -f *.o *.exe

# end
