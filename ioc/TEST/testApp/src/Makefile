TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# build an ioc application

PROD_IOC = test

DBD += test.dbd
# test.dbd will be made up from these files:
test_DBD += base.dbd
#include definitions for any other support applications needed
test_DBD += asyn.dbd
test_DBD += devIocStats.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
test_SRCS += test_registerRecordDeviceDriver.cpp
test_SRCS_DEFAULT += testMain.cpp
test_SRCS_vxWorks += -nil-

#The following adds support from base/src/vxWorks
test_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

#add a definition for each support application used by this application
#test_LIBS += xxx

#NOTES:
#      1)It is not possible to build sncExample both as a component of test
#        and standalone. You must choose only one.
#      2)To build sncExample SNCSEQ must be defined in <top>/configure/RELEASE

#The following builds sncExample as a component of test
#test_SRCS += sncExample.stt
test_LIBS += seq pv
test_LIBS += devIocStats
test_LIBS += asyn

test_LIBS += $(EPICS_BASE_IOC_LIBS)

#The following builds sncExample as a standalone application
#PROD_HOST += sncExample
#sncExample_SNCFLAGS += +m
#sncExample_SRCS += sncExample.stt
#sncExample_LIBS += seq pv
#sncExample_LIBS += $(EPICS_BASE_HOST_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

