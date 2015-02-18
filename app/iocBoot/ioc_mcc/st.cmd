dbLoadDatabase("../../dbd/example.dbd")
example_registerRecordDeviceDriver(pdbbase)
dbLoadTemplate("db/mcc.template")

iocInit()
