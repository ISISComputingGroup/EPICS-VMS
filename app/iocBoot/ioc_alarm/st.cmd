dbLoadDatabase("../../dbd/example.dbd")
example_registerRecordDeviceDriver(pdbbase)
dbLoadTemplate("db/monitor.template")

iocInit()
