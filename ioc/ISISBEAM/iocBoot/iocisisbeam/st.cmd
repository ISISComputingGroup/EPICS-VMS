#!../../bin/linux-x86_64/isisbeam

## $Id: st.cmd 55 2014-06-27 14:12:54Z FreddieAkeroyd $
## $HeadURL: http://svn.isis.rl.ac.uk/freddie_test/trunk/EPICS_VMS/ioc/ISISBEAM/iocBoot/iocisisbeam/st.cmd $

< envPaths

# 130.246.39.152 is ISIS beam gateway
epicsEnvSet("EPICS_IOC_LOG_PORT", "7006")
epicsEnvSet("EPICS_IOC_LOG_INET", "130.246.39.152")
#epicsEnvSet("SIM_ISISBEAM", "1")
epicsEnvSet("RESET_ERROR_COUNT", "200")
epicsEnvSet("RESET_TIME_DIFF", "300")
iocLogInit()

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/isisbeam.dbd",0,0)
isisbeam_registerRecordDeviceDriver(pdbbase)

## portName, paramFile, pollTime
isisbeamConfigure("isisbeam","${TOP}/iocBoot/${IOC}/params.txt",3.0,$(SIM_ISISBEAM=0))

#epicsEnvSet("MYPVPREFIX", "f:")
epicsEnvSet("MYPVPREFIX", "")

## Load record instances
dbLoadRecords("$(DEVIOCSTATS)/db/iocAdminSoft.db","IOC=$(MYPVPREFIX)ICS:IB:IOCSTATS")
dbLoadRecords("db/isisbeam.db","P=$(MYPVPREFIX)")
dbLoadRecords("db/beam.db","P=$(MYPVPREFIX),IOC=$(MYPVPREFIX)ICS:IB:IOCSTATS")
dbLoadRecords("db/shutter_mode.db","P=$(MYPVPREFIX)")
dbLoadRecords("db/shutter_mode_ts1.db","P=$(MYPVPREFIX)")
dbLoadRecords("db/shutter_status.db","P=$(MYPVPREFIX)")
dbLoadRecords("db/shutter_status_ts1.db","P=$(MYPVPREFIX)")
dbLoadRecords("db/vat.db","P=$(MYPVPREFIX)")
dbLoadRecords("db/vat_ts1.db","P=$(MYPVPREFIX)")

cd ${TOP}/iocBoot/${IOC}
asSetFilename("default.acf")
iocInit()

## list out PVs
#dbl

## Start any sequence programs
#seq sncxxx,"user=faa59Host"
