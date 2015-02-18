To get a new support module to compile on VMS, you will need to edit
some files in "configure" and possibly elsewhere. The intermediate
build directories, usually of the form O.* need to be named differently
on VMS and so any reference to these in either a configure/* or Makefile
needs to be replaced by $(BUILD_PREFIX) Often only configure/CONFIG will
need editing for this, but sometimes it may be configure/CONFIG_APP

For example, in asyn/configure/CONFIG_APP the line:

    -include $(TOP)/configure/O.$(T_A)/CONFIG_APP_INCLUDE

was changed to:

    -include $(TOP)/configure/$(BUILD_PREFIX)$(T_A)/CONFIG_APP_INCLUDE
