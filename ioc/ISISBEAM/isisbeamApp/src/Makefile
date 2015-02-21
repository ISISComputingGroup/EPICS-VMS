TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# build an ioc application

PROD_IOC = isisbeam

# we can only build a full IOC on VMS, so define TESTING elsewhere
USR_CXXFLAGS_DEFAULT += -DTESTING
USR_CXXFLAGS_VMS += -Wc/warn -Wc/reentrancy=multithread

DBD += isisbeam.dbd

# isisbeam.dbd will be made from isisbeamInclude.dbd

# isisbeam.dbd will be made up from these files:
#isisbeam_DBD += base.dbd
#include definitions for any other support applications needed
#isisbeam_DBD += xxx.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
isisbeam_SRCS += isisbeam_registerRecordDeviceDriver.cpp

isisbeam_SRCS_DEFAULT += isisbeamMain.cpp isisbeamDriver.cpp

#add a definition for each support application used by this application
isisbeam_LIBS += asyn devIocStats

isisbeam_LIBS += $(EPICS_BASE_IOC_LIBS)
isisbeam_LDFLAGS_VMS += -input_to_ld ../isisbeam.opt -Wl/threads=upcalls

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

