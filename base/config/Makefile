#*************************************************************************
# Copyright (c) 2002 The University of Chicago, as Operator of Argonne
#     National Laboratory.
# Copyright (c) 2002 The Regents of the University of California, as
#     Operator of Los Alamos National Laboratory.
# EPICS BASE Versions 3.13.7
# and higher are distributed subject to a Software License Agreement found
# in file LICENSE that is included with this distribution. 
#*************************************************************************
#
# Makefile,v 1.12.2.2 2004/02/10 16:02:19 jba Exp
#

TOP=..

include $(TOP)/configure/CONFIG

ifeq ($(findstring YES,$(COMPAT_313) $(COMPAT_TOOLS_313)),YES)
INSTALL_CONFIG = $(INSTALL_LOCATION)/config

CONFIGS += $(subst ../,,$(wildcard ../CONFIG*))
CONFIGS += $(subst ../,,$(wildcard ../RULES*))
endif

include $(TOP)/configure/RULES

