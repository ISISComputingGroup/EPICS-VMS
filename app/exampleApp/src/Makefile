TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# Build an IOC support library

LIBRARY_IOC += xxxSupport

# xxxRecord.h will be created from xxxRecord.dbd
DBDINC += xxxRecord
# install devXxxSoft.dbd into <top>/dbd
DBD += xxxSupport.dbd

# The following are compiled and added to the Support library
xxxSupport_SRCS += xxxRecord.c
xxxSupport_SRCS += devXxxSoft.c

xxxSupport_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================
# build an ioc application

PROD_IOC = example
# example.dbd will be created and installed
DBD += example.dbd

# example.dbd will be made up from these files:
example_DBD += base.dbd
example_DBD += xxxSupport.dbd
example_DBD += dbSubExample.dbd
example_DBD += exampleHello.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
example_SRCS += example_registerRecordDeviceDriver.cpp
example_SRCS_DEFAULT += exampleMain.cpp
example_SRCS_vxWorks += -nil-

# Add locally compiled object code
example_SRCS += dbSubExample.c
example_SRCS += exampleHello.c

# The following adds support from base/src/vxWorks
example_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

example_LIBS += xxxSupport

# NOTE: To build SNL programs, SNCSEQ must be defined
# in the <top>/configure/RELEASE file

ifneq ($(SNCSEQ),)
    # This builds sncExample as a component of example
    example_SNCFLAGS += +r
    example_DBD += sncExample.dbd
    example_SRCS += sncExample.stt
    example_LIBS += seq pv

    # The following builds sncProgram as a standalone application
    PROD_HOST += sncProgram
    sncProgram_SNCFLAGS += +m
    sncProgram_SRCS += sncProgram.st
    sncProgram_LIBS += seq pv
    sncProgram_LIBS += $(EPICS_BASE_HOST_LIBS)
endif

example_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

