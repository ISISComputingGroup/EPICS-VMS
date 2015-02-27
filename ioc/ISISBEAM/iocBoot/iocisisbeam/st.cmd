#!../../bin/linux-x86_64/isisbeam

## $Id: st.cmd 55 2014-06-27 14:12:54Z FreddieAkeroyd $
## $HeadURL: http://svn.isis.rl.ac.uk/freddie_test/trunk/EPICS_VMS/ioc/ISISBEAM/iocBoot/iocisisbeam/st.cmd $

< envPaths

# 130.246.39.24 is isis nagios monitoring server, 130.246.51.165 is ISIS beam gateway
epicsEnvSet("EPICS_IOC_LOG_INET", "130.246.39.24")
iocLogInit()

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/isisbeam.dbd",0,0)
isisbeam_registerRecordDeviceDriver(pdbbase)

## portName, paramFile, pollTime
isisbeamConfigure("isisbeam","${TOP}/iocBoot/${IOC}/params.txt",3.0)

## Load record instances
dbLoadRecords("$(DEVIOCSTATS)/db/iocAdminSoft.db","IOC=ICS:IOCSTATS")
dbLoadRecords("db/isisbeam.db","P=ICS:AC:")
dbLoadRecords("db/shutter_mode.db","P=ICS:AC:")
dbLoadRecords("db/shutter_status.db","P=ICS:AC:")
dbLoadRecords("db/vat.db","P=ICS:AC:")

cd ${TOP}/iocBoot/${IOC}
iocInit()

## Start any sequence programs
#seq sncxxx,"user=faa59Host"
