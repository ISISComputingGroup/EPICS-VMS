TOP = ../../base
include $(TOP)/configure/CONFIG

ACTIONS += uninstall

DIRS := $(wildcard *)
DIRS := $(filter-out Makefile,$(DIRS))

include $(TOP)/configure/RULES_DIRS
