TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# build an ioc application

## we can only build a full IOC on VMS, so defines TESTING=YES 
## when not on VMS. If you are building on an non-ISIS VMS machine you
## will also need to define TESTING=YES
ifeq ($(findstring OpenVMS,$(EPICS_HOST_ARCH)),) 
# not on VMS, so define TESTING as YES
TESTING=YES
else
# change to YES if on a non ISIS VMS machine 
TESTING=NO
endif

PROD_IOC = isisbeam

ifeq ($(TESTING),YES) 
USR_CXXFLAGS += -DTESTING
else
USR_CXXFLAGS_VMS += -Wc/warn -Wc/reentrancy=multithread -D__NEW_STARLET=1
endif

DBD += isisbeam.dbd

# isisbeam.dbd will be made from isisbeamInclude.dbd

# isisbeam.dbd will be made up from these files:
#isisbeam_DBD += base.dbd
#include definitions for any other support applications needed
#isisbeam_DBD += xxx.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
isisbeam_SRCS += isisbeam_registerRecordDeviceDriver.cpp

isisbeam_SRCS_DEFAULT += isisbeamMain.cpp isisbeamDriver.cpp

isisbeam_LIBS += asyn devIocStats
isisbeam_LIBS += $(EPICS_BASE_IOC_LIBS)

# ISIS specific linker options
ifneq ($(TESTING),YES) 
isisbeam_LDFLAGS_VMS += -input_to_ld ../isisbeam.opt
endif

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

